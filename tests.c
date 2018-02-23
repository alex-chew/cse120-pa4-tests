#include "acutest.h"
#include "aux.h"
#include "mykernel4.h"
#include "umix.h"

#ifdef USE_REFERENCE_KERNEL
#define MyInitThreads  InitThreads
#define MyCreateThread CreateThread
#define MyGetThread    GetThread
#define MyYieldThread  YieldThread
#define MySchedThread  SchedThread
#define MyExitThread   ExitThread
#endif


/**
 * Thread chaining: T2 (cube thread) yields to T1 (square thread); T1 yields
 * to T0 (main thread), T0 yields back to T2, etc.
 *
 * From Exercise C.
 */

static struct {
	int square, cube;
} d1;

void t1_printSquares(int t) {
	for (int i = 0; i < 5; i++) {
		d1.square = i * i;
		TEST_MSG("T%d: %d squared = %d\n", MyGetThread(), i, d1.square);
		// T1 should only be yielded to from T2
		TEST_CHECK(MyYieldThread(t) == 2);
	}
}

void t1_printCubes(int t) {
	for (int i = 0; i < 5; i++) {
		d1.cube = i * i * i;
		TEST_MSG("T%d: %d cubed = %d\n", MyGetThread(), i, d1.cube);
		// T1 should only be yielded to from T0
		TEST_CHECK(MyYieldThread(t) == 0);
	}
}

void t1() {
	MyInitThreads();
	int me = MyGetThread();
	int t = MyCreateThread(t1_printSquares, me);
	t = MyCreateThread(t1_printCubes, t);

	for (int i = 0; i < 5; i++) {
		// T1 should only be yielded to from T1
		TEST_CHECK(MyYieldThread(t) == 1);
		TEST_MSG("T%d: square = %d, cube = %d\n", me, d1.square, d1.cube);
	}
	TEST_CHECK(d1.square == 16);
	TEST_CHECK(d1.cube == 64);
	MyExitThread();
}


/** Init/Exit should work for just one thread (namely, the main thread). */
void t2() {
	MyInitThreads();
	TEST_CHECK(MyGetThread() == 0);
	MyExitThread();
}


/** MySchedThread should work for just the main thread. */
void t3() {
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

void t4_func(int param) {
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

void t4() {
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


/**
 * Yielding to and from all threads (including main thread).
 */

void t5_func(int source) {
	MyYieldThread(source);
}

void t5() {
	MyInitThreads();
	int me = MyGetThread();
	for (int i = 1; i <= 9; ++i) {
		TEST_CHECK(MyYieldThread(MyCreateThread(t5_func, me)) == i);
		TEST_CHECK(MyYieldThread(me) == me);
	}
	MyExitThread();
}


/**
 * Stress test under thread churn. For each of 1000 rounds, 5 threads are
 * created running func(round). Each of these 5 threads creates a new thread
 * running func(round + 1), then exits.
 *
 * We expect that each thread ID appears 500 times, since T0 through T4 are
 * run on odd rounds, and T5 through T9 are run on even rounds.
 */

#define T6_ROUNDS 1000

static struct {
	int tid_counts[MAXTHREADS];
} d6;

void t6_func(int round) {
	int tid = MyGetThread();
	++d6.tid_counts[tid];
	if (round < T6_ROUNDS) {
		MyCreateThread(t6_func, round + 1);
	} else {
		TEST_CHECK(d6.tid_counts[tid] == T6_ROUNDS / 2);
		TEST_CHECK(d6.tid_counts[(tid + 5) % MAXTHREADS] == T6_ROUNDS / 2);
	}
	MyExitThread();
}

void t6() {
	MyInitThreads();
	for (int t = 0; t < MAXTHREADS; ++t) { d6.tid_counts[t] = 0; }

	// Run t6_func(0) in T0 to T4
	for (int i = 1; i < 5; ++i) {
		MyCreateThread(t6_func, 1);
	}
	t6_func(1);
}


/**
 * Stress test under thread churn, but with at most 1 available thread ID at
 * any time (i.e. either 9 or 10 threads are active).
 */

#define T7_ROUNDS 1000

void t7_func(int round) {
	if (round < T7_ROUNDS) {
		int created = MyCreateThread(t7_func, round + 9);
		int expected = (round + 8) % MAXTHREADS;
		if (created != expected) {
			TEST_CHECK_(0, "Round %d created thread ID %d, expected ID %d",
					round, created, expected);
		}
	}
}

void t7() {
	MyInitThreads();
	for (int i = 1; i <= 8; ++i) {
		MyCreateThread(t7_func, i + 1);
	}
	t7_func(1);
	MyExitThread();
}


/**
 * Stress test under thread churn, but with at most two active threads at a
 * time (T0 spawns T1, T0 exits so T1 gets scheduled, T1 spawns T2, etc.).
 */

#define T8_ROUNDS 1000

void t8_func(int round) {
	if (round < T8_ROUNDS) {
		int created = MyCreateThread(t8_func, round + 1);
		int expected = round % MAXTHREADS;
		if (created != expected) {
			TEST_CHECK_(0, "Round %d created thread ID %d, expected ID %d",
					round, created, expected);
		}
	}
}

void t8() {
	MyInitThreads();
	t8_func(1);
	MyExitThread();
}


/**
 * Check that thread creation can be requested even when MAXTHREADS threads
 * are active, but (-1) will be returned each time.
 */

static struct {
	int alive[3];
} d9;

void t9_func(int _) {
	(void) _;
	int tid = MyGetThread();
	if (tid <= 2) {
		d9.alive[tid] = 0;
		MyExitThread();
	}
	if (d9.alive[0]) { MyYieldThread(0); }
}

void t9() {
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


/**
 * Check that arguments to MyYieldThread are validated (within the range [0,
 * MAXTHREADS - 1], and are active).
 */

void t10_func(int source) {
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

void t10() {
	int tid;
	MyInitThreads();
	tid = MyGetThread();

	// Create T1 and T2, then exit
	MyCreateThread(t10_func, tid);
	MyCreateThread(t10_func, tid);
	MyExitThread();
}


// Required for acutest to work
TEST_LIST = {
	{"square_cube", t1},
	{"init_exit_one", t2},
	{"sched_one", t3},
	{"increase_ids", t4},
	{"yield_everywhere", t5},
	{"churn5", t6},
	{"churn9", t7},
	{"churn1", t8},
	{"create_max", t9},
	{"valid_yield", t10},
	{0}
};
