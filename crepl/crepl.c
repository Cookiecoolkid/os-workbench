#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include <dlfcn.h>

#define NAME_MAX_LEN 256
#define FUNC_MAX_LEN 512

char func_lib_path[FUNC_MAX_LEN][NAME_MAX_LEN + 256];
static int func_index = 0;
static int expr_index = 0;
bool wrong = false;

bool check_function(char* line) {
		int pivot = 0;
		while (line[pivot] == ' ') pivot++;
		if(line[pivot] == 'i' && line[pivot + 1] == 'n' && line[pivot + 2] == 't')
				return true;
		return false;
}

void get_function_name(char* line, char* name) {
    char* start = strchr(line, '(');
    if (start == NULL) {
				wrong = true;
				return;
//        fprintf(stderr, "Invalid function definition\n");
//        exit(EXIT_FAILURE);
    }
    char* end = start;
    while (end > line && *(end - 1) == ' ') end--;

    start = end;
    while (start > line && *(start - 1) != ' ') start--;

		assert(end - start <= NAME_MAX_LEN);
    strncpy(name, start, end - start);
    name[end - start] = '\0';
}

void exec_command(char* command, char* arg1, char* arg2, char* arg3, 
				char* arg4, char* arg5, char* arg6, char* arg7) {
		pid_t pid = fork();
		if (pid < 0) {
				perror("fork");
				exit(EXIT_FAILURE);
		} else if (pid == 0) {
				execlp(command, command, arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL);
				perror("execlp");
				exit(EXIT_FAILURE);
		} else {
				wait(NULL);
		}
}

void create_C_file(char* line, char* name) {
		char path[256];

		snprintf(path, sizeof(path), "/tmp/lib%s.c", name);	
		FILE *file = fopen(path, "w");
		if (file == NULL) {
				printf("Wrong Expr\n");
				fflush(stdout);
				perror("fopen");
				exit(EXIT_FAILURE);
		}

		if (fputs(line, file) == EOF) {
				printf("Wrong Expr\n");
				fflush(stdout);
				perror("fputs");
				exit(EXIT_FAILURE);
		}

		if (fclose(file) != 0) {
				printf("Wrong Expr\n");
				fflush(stdout);
				perror("fclose");
				exit(EXIT_FAILURE);
		}
}

int main(int argc, char *argv[]) {
		static char line[4096];

    while (1) {
        printf("crepl> ");
        fflush(stdout);
				
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        // To be implemented.
    		
				wrong = false;

				char name[NAME_MAX_LEN];
				char src_path[512], lib_path[512];
				char expr_line[8192];
				char dl_option[512];
				bool get_function = check_function(line);
				if (get_function) { 
						get_function_name(line, name);
						if (wrong) continue;
						create_C_file(line, name);
				} else {
						snprintf(name, sizeof(name), "__expr_wrapper_%d", expr_index++);
						snprintf(expr_line, sizeof(expr_line), "int %s() { return %s;}", name, line);
						create_C_file(expr_line, name);
				}


				snprintf(src_path, sizeof(src_path), "/tmp/lib%s.c", name);
				snprintf(lib_path, sizeof(lib_path), "/tmp/lib%s.so", name);
				snprintf(dl_option, sizeof(dl_option), "-l%s", name);
#ifdef __x86_64__ 
				exec_command("gcc", "-shared", "-o", lib_path, src_path, "-L/tmp", dl_option, NULL);
#else
				exec_command("gcc", "-m32", "-shared", "-o", lib_path, src_path, "-L/tmp", dl_option);
#endif
				if (get_function) {
						// load function.so
						snprintf(func_lib_path[func_index], sizeof(func_lib_path[func_index]),"%s", lib_path);
						func_index++;

						void *handle = dlopen(lib_path, RTLD_NOW | RTLD_GLOBAL);
						if (handle == NULL) {
								printf("handle NULL\n");
								fflush(stdout);
								continue;
//								fprintf(stderr, "dlopen: %s\n", dlerror());
//								exit(EXIT_FAILURE);
						}

						printf("OK!\n");
						fflush(stdout);
				} else {
						/*
						for (int i = 0; i < func_index; i++) {
								void *handle = dlopen(func_lib_path[i], RTLD_NOW | RTLD_GLOBAL);
								if (handle == NULL) {
										fprintf(stderr, "dlopen: %s\n", dlerror());
										exit(EXIT_FAILURE);
								}
						}
						*/
   					void *handle = dlopen(lib_path, RTLD_NOW | RTLD_GLOBAL);
						if (handle == NULL) {
								continue;
//								fprintf(stderr, "dlopen: %s\n", dlerror());
//								exit(EXIT_FAILURE);
						}

						int (*func)() = dlsym(handle, name);
						if (func == NULL) {
								continue;
//								fprintf(stderr, "dlsym: %s\n", dlerror());
//								exit(EXIT_FAILURE);
						}

						int res = func();

						if (dlclose(handle) != 0) {
								continue;
//								fprintf(stderr, "dlclose: %s\n", dlerror());
//								exit(EXIT_FAILURE);
						}
						printf("= %d\n", res);
						fflush(stdout);
				}
		}
}
