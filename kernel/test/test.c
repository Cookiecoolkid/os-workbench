#include "common.h"
#include "thread.h"
#include "assert.h"
#include <sys/syscall.h>
#include <stddef.h>
#include <unistd.h>

static void entry(int tid) { 
		void* block = pmm->alloc(128); 
		printf("Thread %d allocated block at address %p\n", tid, block);
    pmm->free(block);
    printf("Thread %d freed block at address %p\n", tid, block);
}
static void goodbye()      { printf("End.\n"); }
int main() {
		pmm->init();
		for (int i = 0; i < 4; i++)
				create(entry);
		join(goodbye);
}
