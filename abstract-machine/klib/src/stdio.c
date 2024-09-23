#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


typedef enum {
		TYPE_NONE = 0,
		TYPE_INT_PADDED_1 = 1,
		TYPE_INT_PADDED_2 = 2,
		TYPE_INT_PADDED_3 = 3,
		TYPE_INT_PADDED_4 = 4,
		TYPE_INT_PADDED_5 = 5,
		TYPE_INT_PADDED_6 = 6,
		TYPE_INT_PADDED_7 = 7,
		TYPE_INT_PADDED_8 = 8,
		TYPE_INT_PADDED_9 = 9,
		TYPE_STRING = 10,
		TYPE_INT = 11,
		TYPE_CHAR = 12,
		TYPE_POINTER = 13,
} FormatType;

FormatType format_type(const char *fmt, int index) {
		if(fmt[index] != '%') return TYPE_NONE;
		if(fmt[index + 1] == 's') return TYPE_STRING;
		else if (fmt[index + 1] == 'd') return TYPE_INT;
		else if (fmt[index + 1] == 'c') return TYPE_CHAR;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '1' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_1;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '2' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_2;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '3' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_3;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '4' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_4;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '5' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_5;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '6' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_6;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '7' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_7;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '8' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_8;
		else if (fmt[index + 1] == '0' && fmt[index + 2] == '9' && fmt[index + 3] == 'd') return TYPE_INT_PADDED_9;

		else if (fmt[index + 1] == 'p') return TYPE_POINTER;
		else return TYPE_NONE;
}

void ull_to_hex(char* out, unsigned long long value) {
    int i;
    for(i = 0; value != 0; i++) {
        int temp = value % 16;
        if(temp < 10) {
            out[i] = temp + 48;
        } else {
            out[i] = temp + 87;
        }
        value = value / 16;
    }
    out[i] = '\0';
    // reverse the string
    for(int j = 0; j < i / 2; j++) {
        char temp = out[j];
        out[j] = out[i - j - 1];
        out[i - j - 1] = temp;
    }
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    int i = 0;
    int j = 0;
    char* s;
    int d;
    int ch;
    void* p;
    va_start(ap, fmt);
		while(fmt[i] != '\0')
    {
        int type = format_type(fmt, i);
        if(type)
        {
            if(type == TYPE_STRING)
            {
								s = va_arg(ap, char*);                                                                                 
								int k = 0;
								while(s[k] != '\0')
										out[j++] = s[k++];
								i += 2;
						}
						else if (type == TYPE_INT || (type >= TYPE_INT_PADDED_1 && type <= TYPE_INT_PADDED_9))
						{
								d = va_arg(ap, int);
								int st = j;
								int ed;
								bool isneg = false;
								int digit = 0;
								if (d < 0)
								{
										isneg = true;
										d = -d;
								}

								do{
										out[j++] = d % 10 + '0';
										d /= 10;
										digit++;
								}while(d > 0);

								if(type >= TYPE_INT_PADDED_1 && type <= TYPE_INT_PADDED_9)
								{
										while(digit < type)
										{
												out[j++] = '0';
												digit++;
										}
								}
								if(isneg) out[j++] = '-';
								ed = j - 1;

								while(st < ed)
								{
										char tmp = out[st];
										out[st] = out[ed];
										out[ed] = tmp;
										st++, ed--;
								}

								if(type == TYPE_INT) i += 2;
								else if(type >= TYPE_INT_PADDED_1 && type <= TYPE_INT_PADDED_9) i += 4;
						}
						else if (type == TYPE_CHAR)
						{
								ch = va_arg(ap, int);
								out[j++] = ch;
								i += 2;
						}
						else if (type == TYPE_POINTER)
						{
								p = va_arg(ap, void*);
								if (p != NULL) {
										out[j++] = '0';
										out[j++] = 'x';
										char buffer[64];
										ull_to_hex(buffer, (unsigned long long)p);
										for (int k = 0; buffer[k] != '\0'; ++k)
										{
												out[j++] = buffer[k];
										}
								} else {
										out[j++] = 'N';
										out[j++] = 'U';
										out[j++] = 'L';
										out[j++] = 'L';
								}
                i += 2;
            }
        }
        else out[j++] = fmt[i++];
    }
    va_end(ap);
    out[j] = '\0';
    return j;
}


int printf(const char *fmt, ...) {
		va_list ap;
		int i = 0;
		int j = 0;
		char* s;
		int d;
		int ch;
		void *p;
		char buf[4096];

		char arg[64];

		va_start(ap, fmt);

		// while(fmt[i] != '\0') putch(fmt[i++]);	
		// i = 0;

		while(fmt[i] != '\0')
		{
				FormatType type = format_type(fmt, i);
				if(type)
				{
						if(type == TYPE_STRING)
						{
								s = va_arg(ap, char*);
								j += sprintf(buf + j, "%s", s);
								i += 2;
						}
						else if(type == TYPE_INT || (type >= TYPE_INT_PADDED_1 && type <= TYPE_INT_PADDED_9))
						{
								d = va_arg(ap, int);
								arg[0] = '%';
								if (type >= TYPE_INT_PADDED_1 && type <= TYPE_INT_PADDED_9)
								{
										arg[1] = '0'; 
										arg[2] = type + '0';
										arg[3] = 'd';
										arg[4] = '\0';
										i += 4;
								}
								else if (type == TYPE_INT)
								{
										arg[1] = 'd';
										arg[2] = '\0';
										i += 2;
								}
								j += sprintf(buf + j, arg, d);
						}
						else if (type == TYPE_CHAR)
						{
								ch = va_arg(ap, int);
								j += sprintf(buf + j, "%c", ch);
								i += 2;
						}
						else if (type == TYPE_POINTER)
						{
								p = va_arg(ap, void*);
								j += sprintf(buf + j, "%p", p);
								i += 2;
						}
				}
				else
						buf[j++] = fmt[i++];
		}
		buf[j] = '\0';
		j = 0;
		while(buf[j] != '\0')
		{
				putch(buf[j]);
				j++;
		}

		va_end(ap);
		return j;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
