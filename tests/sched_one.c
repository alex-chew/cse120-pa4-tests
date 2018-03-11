#include "tests.h"

/** MySchedThread should work for just the main thread. */
void sched_one() {
	MyInitThreads();
	TEST_CHECK(MyGetThread() == 0);
	MySchedThread();
	TEST_CHECK(MyGetThread() == 0);
	MySchedThread();
	TEST_CHECK(MyGetThread() == 0);
	MySchedThread();
	TEST_CHECK(MyGetThread() == 0);
	MyExitThread();
}
