#include "tests.h"

/**
 * Check that after T1 and T9 run and exit, the next-created thread has the
 * correct thread ID.
 */

static void t12_func(int t){
  TEST_CHECK_(t == MyGetThread(),
		  "thread should have ID %d, actually has ID %d", t, MyGetThread());
}

void double_jmp_buf(){
  MyInitThreads();
  int i;
  for (i = 1; i < MAXTHREADS; i++){
    TEST_CHECK(MyCreateThread(t12_func, i) == i);
    TEST_CHECK(MyYieldThread(i) == i);
  }
  TEST_CHECK(MyCreateThread(t12_func, 1) == 1);
  TEST_CHECK(MyYieldThread(1) == 1);
  MyExitThread();
}
