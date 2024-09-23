#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


size_t strlen(const char *s) {
		size_t len = 0;
		while(s[len] != '\0') len++;
		return len;
}

char *strcpy(char *dst, const char *src) {
		size_t i;
		for(i = 0; src[i] != '\0'; i++)
				dst[i] = src[i];
		dst[i] = '\0';  // Add null terminator
		return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
		size_t i;
		for(i = 0; i < n && src[i] != '\0'; i++)
				dst[i] = src[i];
		for(; i < n; i++)
				dst[i] = '\0';
		return dst;
}

char *strcat(char *dst, const char *src) {
		size_t dst_len = strlen(dst);
		size_t src_len = strlen(src);
		size_t i;
		for(i = 0; i < src_len; i++)
				dst[dst_len + i] = src[i];
		dst[dst_len + i] = '\0';
		return dst;
}
int strcmp(const char *s1, const char *s2) {                                                                                                                                
    size_t i = 0;
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    if(s1_len == 0 && s2_len == 0) return 0;
    else if (s1_len == 0) return -1;
    else if (s2_len == 0) return 1;
    else 
    {
        while(i < s1_len && i < s2_len)
        {
            if(s1[i] > s2[i]) return 1;
            else if (s1[i] < s2[i]) return -1;
            i++;
        }
        if(i == s1_len && i != s2_len) return -1;
        else if (i != s1_len && i == s2_len) return 1; 
    }
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    if(s1_len == 0 && s2_len == 0) return 0;
    else if (s1_len == 0) return -1;
    else if (s2_len == 0) return 1;
    else 
    {
        while(i < s1_len && i < s2_len && i < n)
        {
            if(s1[i] > s2[i]) return 1;
            else if (s1[i] < s2[i]) return -1;
            i++;
        }
        if(i == n) return 0;
        else assert(0);
    }
}

void *memset(void *s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    for(size_t i = 0; i < n; i++)
    {
        *p = c;
        p++;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    unsigned char* d = dst;
    const unsigned char*s = (const unsigned char*)src;
    if(d <= s)
        for(size_t i = 0; i < n; i++)
            d[i] = s[i];
    else
        for(size_t i = n - 1; i >= 0; i--)
            d[i] = s[i];
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
    unsigned char* o = (unsigned char*)out;
    const unsigned char* i = (const unsigned char*)in;              
    for(size_t k = 0; k < n; k++)
        o[k] = i[k];
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for(size_t i = 0; i < n; i++)
    {
        if(p1[i] < p2[i]) return -1;
        else if (p1[i] > p2[i]) return 1;
    }
    return 0;
}

#endif
