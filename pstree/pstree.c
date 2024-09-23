#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#define MAX_LEN 1024

struct process{
  int pid;
  int ppid;
  int depth;
  int childNum;
  char* name;
  struct process *child;
  struct process *next;
};

struct process* nodes[100 * MAX_LEN];
char path[MAX_LEN];
char line[MAX_LEN];
int printOp = 0;
int sortOp = 0;
int version = 0;

struct process* createProcess(int pid, int ppid, char* name){
  struct process* newProcess = (struct process*)malloc(sizeof(struct process));
  newProcess->pid = pid;
  newProcess->ppid = ppid;
  newProcess->depth = 0;
  newProcess->childNum = 0;
  newProcess->name = name;
  newProcess->child = NULL;
  newProcess->next = NULL;

  return newProcess;
}

void addChild(struct process* parent, struct process* child){
  if (parent->child == NULL) parent->child = child;
  else{
	struct process* brother = parent->child;
	while (brother->next != NULL) brother = brother->next;
	brother->next = child;
  }
  parent->childNum++;
}

void getDepth(struct process* root){
  if(root == NULL) return;
  
  struct process* q[100 * MAX_LEN];
  int head = 0;
  int tail = 0;
  int depth = 0;
  q[tail++] = root;

  while (head < tail){
    struct process* node = q[head];
	head++;
    struct process* child = node->child;

	while(child != NULL){
	  child->depth = node->depth + 1;
	  q[tail++] = child;
	  child = child->next;
	}
  }
}
/*
void bubbleSort(struct process *root, int n){
  if(!root || !root->next) return;
  for(int i = 0; i < n; i++){
    struct process* pre = root;
    struct process* cur = root->next;
    int flag = 0;
	for(int j = 0; j < n - i - 1; j++){
	  if(cur != NULL && pre->pid > cur->pid){
		 // swap(pre->pid, cur->pid);
		 int tmp = pre->pid;
		 pre->pid = cur->pid;
		 cur->pid = tmp;
		 flag = 1;
	  }
	  pre = pre->next;
	  cur = cur->next;
	}
	if(!flag) break;
  }
}
struct process* sortList(struct process* head) {
	struct process* ptr = head;
	int n = 0;
	while(ptr != NULL){
		n++;
		ptr = ptr->next;
	}
	bubbleSort(head, n);
	return head;
}
*/


struct process* sortLinkedList(struct process* root) {
    if (root == NULL || root->next == NULL)
        return root;

    struct process* dummy = (struct process*)malloc(sizeof(struct process));
    struct process* pre;
    struct process* cur;
	  struct process* nxt;
    struct process* tail;
    
	  dummy->next = root;
    tail = NULL;
    while (root != tail)
    {
				pre = dummy;
				cur = pre->next;
				nxt = cur->next;
				while (nxt != tail)
				{
						if(sortOp == 1)
						{
								if (cur->pid > nxt->pid)
								{
										pre->next = nxt;
										cur->next = nxt->next;
										nxt->next = cur;
								}
								else cur = cur->next;
						}
						else if (sortOp == 0)
						{
								if (strcmp(cur->name, nxt->name) > 0)
								{
										// printf("%s > %s\n", cur->name, nxt->name);
										pre->next = nxt;
										cur->next = nxt->next;
										nxt->next = cur;
								}
								else cur = cur->next;
						}
						pre = pre->next;
						nxt = cur->next;

				}
				tail = cur;
    }

    root = dummy->next;
	dummy = NULL;
	free(dummy);
    return root;
}

void sortPstree(struct process* node) {
  node->child = sortLinkedList(node->child);
  struct process* child = node->child;
  while(child != NULL) {
    sortPstree(child);
	child = child->next;
  }
}

/*
int compare(const void *a, const void *b) {
    struct process *processA = (struct process *)a;
    struct process *processB = (struct process *)b;
    return strcmp(processA->name, processB->name);
}
*/


void printPstree(struct process* node) {
  for(int i = 0; i < node->depth; i++) printf("  ");
  if (!printOp) printf("%s\n", node->name);
  else if(printOp == 1) printf("%s(%d)\n", node->name, node->pid);

  struct process* child = node->child;
  while (child != NULL) {
    printPstree(child);
    child = child->next;
  }
}

int main(int argc, char *argv[]) {
  DIR *dir = opendir("/proc");
  if(dir == NULL) assert(0);  
  else{
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL){
	  if(entry->d_type == DT_DIR){
		int pid = atoi(entry->d_name);
		// printf("%d: %s\n", pid, entry->d_name);
		if (pid){
		  char *name = (char*)malloc(sizeof(char) * MAX_LEN);
		  int ppid = -1;

		  snprintf(path, MAX_LEN, "/proc/%d/status", pid);
		  FILE* fp = fopen(path, "r");

		  assert(fp);

		  while (fgets(line, sizeof(line), fp) != NULL) {
    	    if (strncmp(line, "Name:", 5) == 0) sscanf(line + 6, "%[^\n]", name); 
    	    else if (strncmp(line, "PPid:", 5) == 0) ppid = atoi(line + 5);
		  }
		  fclose(fp); 
		  // printf("pid: %d, name: %s\n", pid, name);

		  nodes[pid] = createProcess(pid, ppid, name);
		  if (ppid) addChild(nodes[ppid], nodes[pid]);
		  
		  // printf("pid: %d, ppid: %d, name: %s\n", pid, ppid, name);
		}
	  }
	}
  }
  closedir(dir);

  getDepth(nodes[1]);
  // printPstree(nodes[1]);

  for (int i = 1; i < argc; i++) {
    assert(argv[i]);
	if(!strcmp(argv[i], "-V") || !strcmp(argv[i], "--version")){
	  // only print version
	  version = 1;
	  fprintf(stderr, "pstree (PSmisc) 23.4\nCopyright (C) 1993-2020 Werner Almesberger and Craig Small\n\nPSmisc comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it under\nthe terms of the GNU General Public License.\nFor more information about these matters, see the files named COPYING.\n");
	  break; 
	}
	else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--numeric-sort")){
	  // sort with pid
	  // sortPstree(nodes[1]);
	  sortOp = 1;
	}
	else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--show-pids")){
	  // print with pid
	  printOp = 1;
	}
  }
  
  sortPstree(nodes[1]);
  if(!version) printPstree(nodes[1]);
  // assert(!argv[argc]);
  return 0;
}
