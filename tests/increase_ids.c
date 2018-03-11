#include "tests.h"

/**
 * IDs must be assigned in increasing order.
 *
 * From notes in Exercise C:
 * "Threads IDs should be integers that are assigned in increasing order. The
 * initial thread (that exists by default) is thread 0."
 */

static struct {
	int round;
} d4;

static void t4_func(int param) {
	int tid = MyGetThread();
	TEST_CHECK(param == tid);

	if (d4.round == 0) {
		if (tid == 2 || tid == 5) { MyExitThread(); }
		else { MyYieldThread(0); }
	}

	if (d4.round == 1) {
		// T1 becomes "leader" thread once T0 exits
		if (tid == 1) {
			MyYieldThread(3);
			d4.round = 2;
		}
		else if (tid == 3) { MyExitThread(); }
		else { MyYieldThread(1); }
	}

	// Next three threads are created. Since the last ID assigned was 2, and
	// since 3 is the next available, the three threads are assigned the
	// following IDs: 3, 5, and 0.
	if (d4.round == 2) {
		if (tid == 1) {
			TEST_CHECK(MyCreateThread(t4_func, 3) == 3);
			TEST_CHECK(MyCreateThread(t4_func, 5) == 5);
			TEST_CHECK(MyCreateThread(t4_func, 0) == 0);
		}
	}
}

void increase_ids() {
	MyInitThreads();

	// Seven threads are created: 0, 1, 2, ..., 6.
	TEST_CHECK(MyGetThread() == 0); // T0 will be "leader" thread until exit
	TEST_CHECK(MyCreateThread(t4_func, 1) == 1);
	TEST_CHECK(MyCreateThread(t4_func, 2) == 2);
	TEST_CHECK(MyCreateThread(t4_func, 3) == 3);
	TEST_CHECK(MyCreateThread(t4_func, 4) == 4);
	TEST_CHECK(MyCreateThread(t4_func, 5) == 5);
	TEST_CHECK(MyCreateThread(t4_func, 6) == 6);

	// Next, threads 2 and 5 exit.
	d4.round = 0;
	MyYieldThread(2);
	MyYieldThread(5);

	// Next four threads are created: 7, 8, 9, and 2. Since 0 and 1 still
	// exist, those IDs are skipped over.
	TEST_CHECK(MyCreateThread(t4_func, 7) == 7);
	TEST_CHECK(MyCreateThread(t4_func, 8) == 8);
	TEST_CHECK(MyCreateThread(t4_func, 9) == 9);
	TEST_CHECK(MyCreateThread(t4_func, 2) == 2);

	// Next 0 and 3 exit.
	d4.round = 1;
	MyExitThread();
	// (This continues in t4_func)
}
