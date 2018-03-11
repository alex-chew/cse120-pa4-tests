#include "tests.h"

/**
 * Check that thread creation can be requested even when MAXTHREADS threads
 * are active, but (-1) will be returned each time.
 */

static struct {
	int alive[3];
} d9;

static void t9_func(int _) {
	(void) _;
	int tid = MyGetThread();
	if (tid <= 2) {
		d9.alive[tid] = 0;
		MyExitThread();
	}
	if (d9.alive[0]) { MyYieldThread(0); }
}

void create_max() {
	MyInitThreads();
	d9.alive[0] = d9.alive[1] = d9.alive[2] = 1;

	// Max out threads, so that further creation requests return -1
	for (int i = 1; i < MAXTHREADS; ++i) {
		TEST_CHECK(MyCreateThread(t9_func, 0) == i);
	}
	TEST_CHECK(MyCreateThread(t9_func, 0) == -1);
	TEST_CHECK(MyCreateThread(t9_func, 0) == -1);
	TEST_CHECK(MyCreateThread(t9_func, 0) == -1);

	// Kill T1 and T2, then ensure that those IDs are assigned to new threads
	if (d9.alive[1]) { MyYieldThread(1); }
	if (d9.alive[2]) { MyYieldThread(2); }
	TEST_CHECK(MyCreateThread(t9_func, 0) == 1);
	TEST_CHECK(MyCreateThread(t9_func, 0) == 2);

	// Now that MAXTHREADS threads are active again, creation is denied
	TEST_CHECK(MyCreateThread(t9_func, 0) == -1);
	TEST_CHECK(MyCreateThread(t9_func, 0) == -1);

	d9.alive[0] = 0;
	MyExitThread();
}
