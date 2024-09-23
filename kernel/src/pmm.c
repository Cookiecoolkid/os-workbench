#include <common.h>

#ifdef TEST
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#endif

#define MIN_BLOCK_SIZE 32
#define PG_SIZE 64 * 1024

pagesAllCPU_t pagesAllCPU[MAX_CPU]; // bug! Don't use pagesAllCPU_t*
																		// otherwise will dereference NULL
heapBlock_t* heapHead;
lock_t heapLock;

void lock(lock_t *locked) {
    while (atomic_xchg(locked, UNLOCKED) != UNLOCKED) { ;}
    assert(*locked == UNLOCKED);
}
void unlock(lock_t *locked) {
    atomic_xchg(locked, LOCKED);
}
void lock_init(lock_t *locked) {
    *locked = LOCKED;
}

size_t ROUND(size_t size) {
		size_t ret = 1;
		while (ret < size) ret <<= 1;
		return ret < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : ret;
}

void* allocPage() {
		lock(&heapLock);

		size_t size = PG_SIZE;
		heapBlock_t* pre = NULL;
		heapBlock_t* p = heapHead;
		while(p) {
				if (p->size > size) break;
				pre = p;
				p = p->next;
		}
		if (p) {
				assert(p->size > size); 
				if (pre) {
						assert(pre->next == p);
						heapBlock_t* block = (heapBlock_t*)((uintptr_t)p + size);
						block->next = p->next;
						block->size = p->size - size;
						p->size = size;
						pre->next = block;
				} else {
						assert(heapHead == p);
						heapBlock_t* block = (heapBlock_t*)((uintptr_t)p + size);
						block->next = p->next;
						block->size = p->size - size;
						p->size = size;
						heapHead = block;
				}
				unlock(&heapLock);
				return (void*)p;				
		}
		unlock(&heapLock);
		return NULL;
}

void* allocHeap(size_t size) {
		lock(&heapLock);

		heapBlock_t* p = heapHead;
		heapBlock_t* lastBlock = NULL;
		while(p) {
				if (p->size >= 2 * size) lastBlock = p;
				p = p->next;
		}
		p = lastBlock;
		
		if (p == NULL) {
				unlock(&heapLock);
				return NULL;
		}
		// alloc from heapBlock.end and add header
		// actually this alloc may not be aligned
		
		heapBlock_t* block;
		if (p->size >= 2 * size) {
				block = (heapBlock_t*)((uintptr_t)p + p->size - size);
				uintptr_t misalign = (uintptr_t)block & (size - 1);
				if (misalign) {
						block = (heapBlock_t*)((uintptr_t)block - misalign);
				}	
					
				header_t* header = (header_t*)((uintptr_t)block - sizeof(header_t));
				p->size = (uintptr_t)header - (uintptr_t)p;
				header->size = size;
				header->magic = 7777777;
				unlock(&heapLock);
				return (void*)block;
		}
		else {
				unlock(&heapLock);
				return NULL;
		}
}

void* slabAlloc(size_t size) {
		int cpu = cpu_current();
		lock(&pagesAllCPU[cpu].lock);

		pagePerCPU_t* p = pagesAllCPU[cpu].header;
		while (p) {
				if (p->size == size && p->freeHead) {
						// page still has freeblock of this size
						break;
				}
				p = p->next;
		}	
		if (p) {
				assert(p->size == size && p->freeHead);
				freeBlock_t* ret = p->freeHead;
				p->freeHead = p->freeHead->next;
				unlock(&pagesAllCPU[cpu].lock);
				return (void*) ret;
		}
		// need new page
		
		pagePerCPU_t* page = (pagePerCPU_t*)allocPage();
		if (page == NULL) {
				unlock(&pagesAllCPU[cpu].lock);
				return NULL;
		}
		
		page->size = size;
		// remain "one size" for pageheader
		freeBlock_t* ph = (freeBlock_t*)((uintptr_t)page + size); 
		page->freeHead = ph;
		for (int i = 1; i < (PG_SIZE) / size - 1; i++) {	
				ph->next = (freeBlock_t*)((uintptr_t)ph + size);
				ph = ph->next;
		}
		// insert page
		page->next = pagesAllCPU[cpu].header;
		pagesAllCPU[cpu].header = page;

		freeBlock_t* ret = page->freeHead;
		page->freeHead = page->freeHead->next;
		unlock(&pagesAllCPU[cpu].lock);
		return (void*) ret;
}

static void *kalloc(size_t size) {
    // TODO
	  size_t aligned_size = ROUND(size);
		// printf("size roundup to %p\n", roundup);

		if (aligned_size > 16 * 1024 * 1024) return NULL;

		if (aligned_size <= 8 * 1024) return slabAlloc(aligned_size);
		else return allocHeap(aligned_size);
}


// may free a page's freeBlock or a huge heapBlock (or page?)
static void kfree(void *ptr) {
    // TODO
		// lock(&pagesAllCPU[cpu_current()].lock);
	
		/*

		pagePerCPU_t* p;
		int pageBlock = 0;
		int i;
		for (i = 0; i < cpu_count(); i++) {
				lock(&pagesAllCPU[i].lock);
				p = pagesAllCPU[i].header;
				while (p) {
						if ((uintptr_t)p <= (uintptr_t)ptr 
										&& (uintptr_t)ptr < (uintptr_t)p + 64 * 1024) {
								pageBlock = 1;
								break;
						}
						p = p->next;
				}
				if (pageBlock) break;
				unlock(&pagesAllCPU[i].lock);
		}
		if (pageBlock) {
				freeBlock_t* fptr = (freeBlock_t*)ptr;
				fptr->next = p->freeHead;
				p->freeHead = fptr;
				unlock(&pagesAllCPU[i].lock);
				return;
		} 
		*/
		header_t* header = (header_t*)((uintptr_t)ptr - sizeof(header_t));
    if (header->magic != 7777777) return;

		lock(&heapLock);

    size_t size = header->size;

    // Convert the block back to a heapBlock
    heapBlock_t* block = (heapBlock_t*)header;
    block->size = size + sizeof(header_t);


    // Find the correct position to insert the block into the free list
    heapBlock_t* prev = NULL;
    heapBlock_t* curr = heapHead;
    while (curr && (uintptr_t)curr < (uintptr_t)block) {
        prev = curr;
        curr = curr->next;
    }

    // Insert the block into the free list
    if (prev) {
        prev->next = block;
    } else {
        heapHead = block;
    }
    block->next = curr;

    if (prev && (uintptr_t)prev + prev->size == (uintptr_t)block) {
        prev->size += block->size;
        prev->next = block->next;
        block = prev;
    }

    if (curr && (uintptr_t)block + block->size == (uintptr_t)curr) {
        block->size += curr->size;
        block->next = curr->next;
    }

    unlock(&heapLock);
}

#ifndef TEST
static void pmm_init() {
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );
				
		for (int i = 0; i < cpu_count(); i++) {
				pagesAllCPU[i].header = NULL;
				lock_init(&(pagesAllCPU[i].lock));
		}
		lock_init(&heapLock);
		heapHead = (heapBlock_t*)heap.start;
		heapHead->next = NULL;
		heapHead->size = (size_t)((uintptr_t)heap.end - (uintptr_t)heap.start);

		printf(
						"Got %d MiB heap: [%p, %p)\n",
						pmsize >> 20, heap.start, heap.end
					);
}
#else
static void pmm_init() {
		char *ptr  = malloc(HEAP_SIZE);
		heap.start = ptr;
		heap.end   = ptr + HEAP_SIZE;
		printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};
