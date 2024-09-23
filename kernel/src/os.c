#include <common.h>

#define test_3_rounds 500000
static void os_init() {
    pmm->init();
}

void test_0() {
		// while (1) {
				void *test1;
				test1 = pmm->alloc(64);
				if (test1 == NULL) {
						printf("CPU[%d] - Fail\n", cpu_current());
				}
				else printf("CPU[%d] - test0-1 alloc: %p\n", cpu_current(), test1);
				
				void *test2;
				test2 = pmm->alloc(64);
				if (test2 == NULL) {
						printf("CPU[%d] - Fail\n", cpu_current());
				}
				else printf("CPU[%d] - test0-2 alloc: %p\n", cpu_current(), test2);

				
				for (size_t i = 0; i < 100000; i++) { ; }

				if (test1) {
						printf("CPU[%d] - test0-1 free: %p\n", cpu_current(), test1);
						pmm->free(test1);
				}
				if (test2) {
						printf("CPU[%d] - test0-2 free: %p\n", cpu_current(), test2);
						pmm->free(test2);
				}
				// }
}

void test_1() {
		// while (1) {
				void *test1;
				test1 = pmm->alloc(64 * 4096);
				if (test1 == NULL) {
						printf("CPU[%d] - Fail\n", cpu_current());
				}
				else printf("CPU[%d] - test1-1 alloc: %p\n",cpu_current(),  test1);
				
				void *test2;
				test2 = pmm->alloc(64 * 4096);
				if (test2 == NULL) {
						printf("CPU[%d] - Fail\n", cpu_current());
				}
				else printf("CPU[%d] - test1-2 alloc: %p\n", cpu_current(), test2);

				
				for (size_t i = 0; i < 100000000; i++) { ; }
				
				if (test1) {
						printf("CPU[%d] - test1-1 free: %p\n", cpu_current(), test1);
						pmm->free(test1);
				}

				if (test2) {
						printf("CPU[%d] - test1-2 free: %p\n", cpu_current(), test2);
						pmm->free(test2);
				}
				// }
				//
				void *test3;
				test3 = pmm->alloc(128 * 4096);
				if (test3 == NULL) {
						printf("CPU[%d] - Fail\n", cpu_current());
				}
				else printf("CPU[%d] - test1-3 alloc: %p\n", cpu_current(), test3);

				
				for (size_t i = 0; i < 100000000; i++) { ; }
				
				if (test3) {
						printf("CPU[%d] - test1-3 free: %p\n", cpu_current(), test3);
						pmm->free(test3);
				}	
}

void test_2() {
		// while (1) {
				void *test1;
				void *test2;
				void *test3;
				test1 = pmm->alloc(16);
				test2 = pmm->alloc(64 * 4096);
				test3 = pmm->alloc(64);
				if (test1 == NULL) {
						printf("CPU[%d] - Fail alloc test_2-1\n", cpu_current());
				}
				if (test2 == NULL) {
						printf("CPU[%d] - Fail alloc test_2-2\n", cpu_current());
				}
				if (test3 == NULL) {
						printf("CPU[%d] - Fail alloc test_2-3\n", cpu_current());
				}

				if (test1 || test2 || test3) printf("CPU[%d] - test_2-1 alloc: %p, test_2-2 alloc: %p, test_2-3 alloc: %p\n",
							 cpu_current(), test1, test2, test3);
				for (size_t i = 0; i < 100000; i++) { ; }

				if (test1) {
						printf("CPU[%d] - test2-1 free: %p\n", cpu_current(), test1);	
						pmm->free(test1);
				}
				if (test3) {
						printf("CPU[%d] - test2-3 free: %p\n", cpu_current(), test3);	
						pmm->free(test1);
				}
				for (size_t i = 0; i < 100000000; i++) { ; }

				if (test2) {
						printf("CPU[%d] - test2-2 free: %p\n", cpu_current(), test2);
						pmm->free(test2);
				}
				// }
}

void test_3()
{
		void *test1[test_3_rounds];
		void *test2[test_3_rounds];
		for (size_t i = 0; i < test_3_rounds; i++)
		{
				test1[i] = pmm->alloc(32);
				if (test1[i] == NULL)
				{
						printf("test is NULL");
				}
				else
						printf("test1[%d]: %p\n",i, test1[i]);
				int *test1_int=(int *)test1[i];
				*test1_int = 9876;
				*(test1_int+(24 - 4) / 4) = 7777777;
				test2[i] = pmm->alloc(64);
				if (test2[i] == NULL)
				{
						printf("test2[%d] is NULL",i);
				}
				else
						printf("test2[%d]: %p\n",i, test2[i]);
		}
		for (size_t i = 0; i < test_3_rounds; i++)
		{
				int *test1_int=(int *)test1[i];
				assert(*test1_int == 9876);
				assert(*(test1_int+(24-4) / 4) == 7777777);
				pmm->free(test1[i]);
				pmm->free(test2[i]);
		}
}

static void os_run() {
		for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
				putch(*s == '*' ? '0' + cpu_current() : *s);
		}

		// printf("cpu_count() = %d\n", cpu_count());

		test_0();
		test_1();
		test_2();
		// test_3();

		while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};
