#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define LOCKED 1
#define UNLOCKED 0
#define MAX_CPU 16
#define PAGES_PER_CPU 10

typedef int lock_t;

void lock(lock_t *locked);

void unlock(lock_t *locked);

void lock_init(lock_t *locked);

// header store the size of heapBlock
typedef struct header_t {
		size_t size;
		size_t magic;
}header_t;

// Record free block of this page
typedef struct freeBlock_t {
		struct freeBlock_t* next;
		// size_t size;
}freeBlock_t;

// Per CPU has 16B, 32B, 64B, ... , 4KB page (10 pages in total)
typedef struct pagePerCPU_t {
		struct pagePerCPU_t* next;
		freeBlock_t* freeHead;
		size_t size; // size of this page's block
}pagePerCPU_t;

// For all CPU pages
typedef struct pagesAllCPU_t {
		pagePerCPU_t* header;
		lock_t lock;
}pagesAllCPU_t;

// For >= 8kiB alloc
typedef struct heapBlock_t {
		struct heapBlock_t* next;
		size_t size;
}heapBlock_t;
