#include "co.h"
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>


#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#define STKSIZE 65536
#define COSIZE 128

struct co {
		char stk[STKSIZE];
		struct ucontext_t uctx;
		int idx;
		// void (*func)(void *);
		// void *arg;
};

struct co* co_threads[COSIZE];
struct co* cur;
ucontext_t main_ctx;
int main_ctx_init = 0; 

struct co* co_start(const char *name, void (*func)(void *), void *arg) {
		struct co* co_t = (struct co*)malloc(sizeof(struct co));
		assert(co_t);

		int idx = 0;
		while (idx < 128 && co_threads[idx] != NULL) idx++;
		assert(idx != 128);

		co_t->idx = idx;
		co_threads[idx] = co_t;
		
		debug("co_threads[%d] = %p\n", idx, co_threads[idx]);

		getcontext(&(co_t->uctx));
		co_t->uctx.uc_stack.ss_sp = co_t->stk;
		co_t->uctx.uc_stack.ss_size = sizeof(co_t->stk);
		
    if (!main_ctx_init) {
        getcontext(&main_ctx);
        main_ctx_init = 1;
    }
		if (cur) co_t->uctx.uc_link = &main_ctx; // (cur->ctx)
		else 
		{
				co_t->uctx.uc_link = &main_ctx;  // NULL
		}
		int argc = 0;
		char* args = (char*)arg;
		
		while(args[argc]) 
		{
				debug("args[%d] = %c\n", argc, args[argc]);
				argc++;
		}
		debug("argc:%d\n", argc);

		makecontext(&(co_t->uctx), (void*)func, argc, args);

		return co_t;
}

void co_wait(struct co *co) {
		cur = co;
		// setcontext(&co->uctx);
		swapcontext(&main_ctx, &co->uctx);
		co_threads[co->idx] = NULL;
		free(co);
}

void co_yield() {
		int idx = (cur->idx + 1) % COSIZE; 

		while(idx != cur->idx && co_threads[idx] == NULL) 
				idx = (idx + 1) % COSIZE;

		if (co_threads[idx] == NULL) return;


		// debug("Switch to %d\n", idx);
		
		struct co* prev = cur; 
		// getcontext(&(cur->uctx));	
		assert(co_threads[idx]);	
	
		// co_threads[idx]->uctx.uc_link = &(cur->uctx);
		cur = co_threads[idx];
		swapcontext(&(prev->uctx), &(cur->uctx));
		// setcontext(&(co_threads[idx]->uctx));
}
