#include "tests.h"

/**
 * Check that arguments to MyYieldThread are validated (within the range [0,
 * MAXTHREADS - 1], and are active).
 */

static void t10_func(int source) {
	int tid = MyGetThread();

	// Reject thread IDs outside the range [0, MAXTHREADS - 1]
	TEST_CHECK(MyYieldThread(-1) == -1);
	TEST_CHECK(MyYieldThread(MAXTHREADS) == -1);

	// Spawning thread exited already
	TEST_CHECK(MyYieldThread(0) == -1);
	TEST_CHECK(MyYieldThread(source) == -1);

	// T1 yields to T2, then T2 yields to T1
	TEST_CHECK(MyYieldThread(3 - tid) == 3 - tid);

	// Reject inactive threads
	TEST_CHECK(MyYieldThread(3) == -1);
	TEST_CHECK(MyYieldThread(MAXTHREADS - 1) == -1);
}

void valid_yield() {
	int tid;
	MyInitThreads();
	tid = MyGetThread();

	// Create T1 and T2, then exit
	MyCreateThread(t10_func, tid);
	MyCreateThread(t10_func, tid);
	MyExitThread();
}
