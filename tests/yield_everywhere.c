#include "tests.h"

/**
 * Yielding to and from all threads (including main thread).
 */

static void t5_func(int source) {
	MyYieldThread(source);
}

void yield_everywhere() {
	MyInitThreads();
	int me = MyGetThread();
	for (int i = 1; i <= 9; ++i) {
		TEST_CHECK(MyYieldThread(MyCreateThread(t5_func, me)) == i);
		TEST_CHECK(MyYieldThread(me) == me);
	}
	MyExitThread();
}
