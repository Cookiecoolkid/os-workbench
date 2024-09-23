#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>  
#include <sys/wait.h>  
#include <fcntl.h>  
#include <regex.h>
#include <time.h>

#define BUFFER_SIZE 65536
#define CMD_SIZE 4096
#define SYSCALL_SIZE 4096
#define NAME_SIZE 4096
typedef struct {
		char name[NAME_SIZE];
		float time;
}syscallTime;

syscallTime syscall_time[SYSCALL_SIZE];

char *cmd[CMD_SIZE];
int _index = 0;
int index_ = 0;
float total_time = 0.0;
clock_t st, ed;
int t = 1;

/*
int compare_syscalls(const void *a, const void *b) {
    return ((syscallTime*)a)->time < ((syscallTime*)b)->time ? 1 : -1;
}
*/

int compare_syscalls(const void *a, const void *b) {
    float diff = ((syscallTime*)a)->time - ((syscallTime*)b)->time;
    return (diff < 0) - (diff > 0);
}

void print_top_syscalls() {
    qsort(syscall_time, index_, sizeof(syscallTime), compare_syscalls);
		printf("Time: %.1fs\n", 0.1 * t++);
    for (int i = 0; i < index_ && i < 5; i++) {
				int ratio = (int)((syscall_time[i].time / total_time) * 100);
    		printf("%s (%d%%)\n", syscall_time[i].name, ratio);
				fflush(stdout);
		}
		printf("====================\n");
		for (int i = 0; i < 80; i++) printf("%c",'\0');
		fflush(stdout);
}

void add_time(char* name, float time) {
		int i;
		for (i = 0; i < index_; i++) {
				if (strcmp(name, syscall_time[i].name) == 0) {
						syscall_time[i].time += time;
						break;
				}
		}
		if (i == index_) {
				strncpy(syscall_time[index_].name, name, NAME_SIZE);
				syscall_time[index_].time = time;
				index_++;
				assert(index_ <= SYSCALL_SIZE);
		}
		total_time += time;
}

float parse_regex(char* str) {
		const char* pattern = "<([0-9]+\\.[0-9]+)>$";

		regex_t regex;
		int ret = regcomp(&regex, pattern, REG_EXTENDED);
		if (ret != 0) {
				printf("Failed to compile regex\n");
				perror("regex");
		}

		regmatch_t matches[2];
		ret = regexec(&regex, str, 2, matches, 0);
		float floatValue = -1.0;

		if (ret == 0 && matches[1].rm_so != -1) {
				int start = matches[1].rm_so;
				int end = matches[1].rm_eo;
				int len = end - start;
				char* matchedStr = malloc(len + 1);
				memcpy(matchedStr, str + start, len);
				matchedStr[len] = '\0';

				floatValue = atof(matchedStr);
				free(matchedStr);
		}
		regfree(&regex);    
		return floatValue;
}

char* parse_name(const char* str) {
		const char* pos = strchr(str, '(');
		if (pos == NULL) {
				return NULL;
		} else {
				size_t length = pos - str;
				char* substring = malloc(length + 1);
				strncpy(substring, str, length);
				substring[length] = '\0';
				return substring;
		}
}

void parse_args(int argc, char *argv[]) {
		_index = 0;
		cmd[_index++] = "strace";
		cmd[_index++] = "-T";
		for (int i = 1; i < argc; i++) {
        assert(argv[i]);
				cmd[_index++] = argv[i];
    }
    assert(!argv[argc]);
		cmd[_index++] = NULL;
		assert(_index <= CMD_SIZE);
}

void exec_strace(char **envp) {
    st = clock();
		int pipefd[2];  
    pid_t pid;  
  
    if (pipe(pipefd) == -1) {  
        perror("pipe");  
        exit(EXIT_FAILURE);  
    }  
  
    pid = fork();  
    if (pid == -1) {  
        perror("fork");  
        exit(EXIT_FAILURE);  
    }  
  
		// child
    if (pid == 0) { 
        // close(STDOUT_FILENO); 
			 	close(STDERR_FILENO);	
				dup2(pipefd[1], STDERR_FILENO);  
				close(pipefd[0]); 

				char **exec_argv = cmd;
				char **exec_envp = envp;
				execve("strace",          exec_argv, exec_envp);
				execve("/bin/strace",     exec_argv, exec_envp);
				execve("/usr/bin/strace", exec_argv, exec_envp);
				perror("execve");
				exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);
        char buffer[BUFFER_SIZE];
        ssize_t nread;
        
				ssize_t total_read = 0;
				while ((nread = read(pipefd[0], buffer + total_read, sizeof(buffer) - total_read - 1)) > 0) {
            total_read += nread;
            buffer[total_read] = '\0';

						char *newline = strchr(buffer, '\n');
						if (newline != NULL) {
								*newline = '\0';
								// printf("Received: %s\n", buffer);
								float floatValue = parse_regex(buffer);
								// printf("Name: %s\n", parse_name(buffer));
								char* name = parse_name(buffer);
								if (floatValue > 0 && name) add_time(name, floatValue);

								free(name);

								size_t remaining = total_read - (newline + 1 - buffer);
								memmove(buffer, newline + 1, remaining);
								total_read = remaining;
						}
						ed = clock();
						if (((ed - st) * 1000) / CLOCKS_PER_SEC >= 10) {
								st = ed;
								print_top_syscalls();
						}
        }
				print_top_syscalls();
        waitpid(pid, NULL, 0);  
    }  
}

int main(int argc, char *argv[], char *envp[]) {
		parse_args(argc, argv);
		exec_strace(envp);
		return 0;
}
