#include <string.h>

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

#define STACKSIZE	65536		// maximum size of thread stack


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
		// T1 should only be yielded to from T2, except last iteration
		if (i < 4) { TEST_CHECK(MyYieldThread(t) == 2); }
	}
}

void t1_printCubes(int t) {
	for (int i = 0; i < 5; i++) {
		d1.cube = i * i * i;
		TEST_MSG("T%d: %d cubed = %d\n", MyGetThread(), i, d1.cube);
		// T1 should only be yielded to from T0, except last iteration
		if (i < 4) { TEST_CHECK(MyYieldThread(t) == 0); }
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

static struct {
	unsigned int failed;
} d7;

void t7_func(int round) {
	if (round < T7_ROUNDS) {
		int expected = (round + 8) % MAXTHREADS;
		int created = MyCreateThread(t7_func, round + 9);
		if (created != expected) {
			TEST_CHECK_(0,
					"round %d should create thread %d, but got %d",
					round, expected, created);
			++d7.failed;
		}
	} else if (round == T7_ROUNDS + 8) {
		TEST_CHECK_(d7.failed == 0,
				"each round created correct thread IDs, but %d rounds failed",
				d7.failed);
	}
}

void t7() {
	MyInitThreads();
	d7.failed = 0;
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

static struct {
	unsigned int failed;
} d8;

void t8_func(int round) {
	if (round < T8_ROUNDS) {
		int expected = round % MAXTHREADS;
		int created = MyCreateThread(t8_func, round + 1);
		if (created != expected) {
			TEST_CHECK_(0,
					"round %d should create thread %d, but got %d",
					round, expected, created);
			++d8.failed;
		}
	} else if (round == T8_ROUNDS) {
		TEST_CHECK_(d8.failed == 0,
				"each round created correct thread IDs, but %d rounds failed",
				d8.failed);
	}
}

void t8() {
	MyInitThreads();
	d8.failed = 0;
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
	int other;

	// Reject thread IDs outside the range [0, MAXTHREADS - 1]
	TEST_CHECK(MyYieldThread(-1) == -1);
	TEST_CHECK(MyYieldThread(MAXTHREADS) == -1);

	// Spawning thread exited already
	TEST_CHECK(MyYieldThread(0) == -1);
	TEST_CHECK(MyYieldThread(source) == -1);

	// T1 yields to T2, then T2 yields to T1
	other = MyYieldThread(3 - tid);
	if (tid == 1) { TEST_CHECK(other == 2); }

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


/**
 * Check that threads' stacks are protected.
 *
 * Each thread should be guaranteed (approximately) STACKSIZE bytes of
 * protected stack space; if we fill T0's stack with 0xf0, T1's stack with
 * 0xf1, etc. up to T9 with 0xf9, then when T0 resumes, its stack should be
 * intact (i.e. not modified by T1).
 */

// Why 196? For some reason if we allocate any more, then the reference kernel
// fails to protect our `stack` local variable. For the sake of testing, if our
// kernel can handle anything that the reference kernel can handle, we're good.
#define T11_BYTES (STACKSIZE - sizeof(int) - 196)
#define T11_MARKER() ((unsigned char) '\xf0' | (MyGetThread() & 0xf))

void t11_func(int tid) {
	unsigned char stack[T11_BYTES];

	// First, create all threads
	if (tid + 1 < MAXTHREADS) { MyCreateThread(t11_func, tid + 1); }
	TEST_MSG("Thread %d stack = %p to %p", tid,
			(void *) &stack[T11_BYTES - 1], (void *) stack);
	MyYieldThread((MyGetThread() + 1) % MAXTHREADS);

	// Fill each threads' stack with a corresponding marker value. We avoid
	// using the `tid` argument from here on out, since an incorrect
	// implementation will overwrite the value and wreak havoc.
	memset(stack, T11_MARKER(), T11_BYTES);
	MyYieldThread((MyGetThread() + 1) % MAXTHREADS);

	// Ensure that first and last bytes in `stack` are intact
	TEST_CHECK_(stack[0] == T11_MARKER(),
			"T%d stack[ 0] = %x, got %x",
			MyGetThread(), T11_MARKER(), stack[0]);
	TEST_CHECK_(stack[T11_BYTES - 1] == T11_MARKER(),
			"T%d stack[-1] = %x, got %x",
			MyGetThread(), T11_MARKER(), stack[T11_BYTES - 1]);
	MyExitThread();
}

void t11() {
	MyInitThreads();
	t11_func(0);
}


/**
 * Check that after T1 and T9 run and exit, the next-created thread has the
 * correct thread ID.
 */

void t12_func(int t){
  TEST_CHECK_(t == MyGetThread(),
		  "thread should have ID %d, actually has ID %d", t, MyGetThread());
}

void t12(){
  MyInitThreads();
  int i;
  for (i = 1; i < MAXTHREADS; i++){
    TEST_CHECK(MyCreateThread(t12_func, i) == i);
    MyYieldThread(i);
  }
  TEST_CHECK(MyCreateThread(t12_func, 1) == 1);
  MyYieldThread(1);
  MyExitThread();
}


/**
 * Tests all functionality of the kernel, including a number of edge cases.
 */

#define T13_ROUNDS 75
#define T13_ORDER() (t13_order_cmd[d13.round][0])
#define T13_CMD()   (t13_order_cmd[d13.round][1])
#define T13_CREATE     (0x100 << 1)
#define T13_CREATE_ERR (0x100 << 2)
#define T13_EXIT       (0x100 << 3)
#define T13_SCHED      (0x100 << 4)
#define T13_YIELD      (0x100 << 5)
#define T13_YIELD_ERR  (0x100 << 6)
#define T13_TARGET(cmd) ((cmd) & 0xff)

// Array of [thread id, command].
// Commands use the flags above to signal behavior, and are read by t13_func.
static const int t13_order_cmd[T13_ROUNDS][2] = {
	[ 0] = {0, T13_CREATE | T13_YIELD | 1},  // Yield to new thread
	[ 1] = {1, T13_CREATE | T13_SCHED},  // Yield to old before new
	[ 2] = {0, T13_CREATE | T13_YIELD | 0},
	[ 3] = {0, T13_CREATE | T13_SCHED},
	[ 4] = {2, T13_CREATE | T13_SCHED},
	[ 5] = {1, T13_SCHED},
	[ 6] = {3, T13_CREATE | T13_EXIT},
	[ 7] = {4, T13_YIELD | 5},
	[ 8] = {5, T13_YIELD | 5},  // Yield to self
	[ 9] = {5, T13_CREATE | T13_YIELD | 5},  // Create, then yield to self
	[10] = {5, T13_CREATE | T13_SCHED},
	[11] = {0, T13_YIELD | 2},
	[12] = {2, T13_EXIT},
	[13] = {1, T13_EXIT},
	[14] = {6, T13_CREATE | T13_EXIT},  // Create and exit
	[15] = {4, T13_CREATE | T13_EXIT},
	[16] = {7, T13_CREATE},
	[17] = {7, T13_CREATE},
	[18] = {7, T13_CREATE | T13_YIELD | 7},
	[19] = {7, T13_CREATE | T13_SCHED},
	[20] = {8, T13_YIELD | 8},
	[21] = {8, T13_YIELD | 3},
	[22] = {3, T13_CREATE | T13_CREATE_ERR | T13_YIELD | 1},  // No IDs left
	[23] = {1, T13_YIELD | 4},
	[24] = {4, T13_EXIT},
	[25] = {5, T13_EXIT},
	[26] = {0, T13_EXIT},
	[27] = {9, T13_EXIT},
	[28] = {2, T13_EXIT},
	[29] = {6, T13_EXIT},
	[30] = {7, T13_CREATE | T13_YIELD | 6 | T13_YIELD_ERR},  // Invalid target
	[31] = {7, T13_YIELD | 2 | T13_YIELD_ERR},  // Invalid target
	[32] = {7, T13_YIELD | 6 | T13_YIELD_ERR},  // Invalid target
	[33] = {7, T13_YIELD | 7},
	[34] = {7, T13_CREATE | T13_EXIT},
	[35] = {8, T13_YIELD | 7 | T13_YIELD_ERR},
	[36] = {8, T13_SCHED},
	[37] = {3, T13_CREATE},  // Create 5 in a row
	[38] = {3, T13_CREATE},
	[39] = {3, T13_CREATE},
	[40] = {3, T13_CREATE},
	[41] = {3, T13_CREATE},
	[42] = {3, T13_CREATE | T13_CREATE_ERR},  // No IDs left, three in a row
	[43] = {3, T13_CREATE | T13_CREATE_ERR},
	[44] = {3, T13_CREATE | T13_CREATE_ERR},
	[45] = {3, T13_EXIT},
	[46] = {1, T13_EXIT},
	[47] = {9, T13_SCHED},
	[48] = {0, T13_YIELD | 1 | T13_YIELD_ERR},  // Yield to recently-exited
	[49] = {0, T13_SCHED},
	[50] = {8, T13_EXIT},
	[51] = {2, T13_EXIT},
	[52] = {4, T13_YIELD | 7},
	[53] = {7, T13_YIELD | 9},
	[54] = {9, T13_EXIT},
	[55] = {5, T13_EXIT},
	[56] = {6, T13_EXIT},
	[57] = {0, T13_EXIT},
	[58] = {4, T13_EXIT},
	[59] = {7, T13_SCHED},  // Sched with no other threads
	[60] = {7, T13_YIELD | 1 | T13_YIELD_ERR},  // Yield to long-since exited
	[61] = {7, T13_SCHED},  // Sched with no other threads
	[62] = {7, T13_CREATE | T13_SCHED},  // Sched-yield to new thread
	[63] = {8, T13_YIELD | 8},  // New thread yield to self
	[64] = {8, T13_SCHED},  // Sched cycle between two threads
	[65] = {7, T13_SCHED},
	[66] = {8, T13_SCHED},
	[67] = {7, T13_EXIT},  // Leaves only one thread left
	[68] = {8, T13_CREATE | T13_SCHED},  // Sched-yield to new thread
	[69] = {9, T13_SCHED},  // Sched cycle between two threads
	[70] = {8, T13_SCHED},
	[71] = {9, T13_SCHED},
	[72] = {8, T13_SCHED},
	[73] = {9, T13_EXIT},
	[74] = {8, T13_EXIT},
};

static struct {
	int round;
} d13;

void t13_func(int _) {
	(void) _;
	int cmd, created;
	// In each round, exactly one thread reads and runs the associated command
	while (d13.round < T13_ROUNDS) {
		TEST_CHECK_(T13_ORDER() == MyGetThread(),
				"round %d thread is T%d, got T%d",
				d13.round, T13_ORDER(), MyGetThread());

		cmd = T13_CMD();
		++d13.round;
		if (cmd & T13_CREATE) {
			created = MyCreateThread(t13_func, _);
			if (cmd & T13_CREATE_ERR) {
				TEST_CHECK(created == -1);
			} else {
				TEST_CHECK(created != -1);
			}
		}
		if (cmd & T13_EXIT) { MyExitThread(); }
		if (cmd & T13_SCHED) {
			MySchedThread();
			continue;
		}
		if (cmd & T13_YIELD) {
			if (cmd & T13_YIELD_ERR) {
				TEST_MSG("Yield error expected here:");
			}
			MyYieldThread(T13_TARGET(cmd));
		}
	}
	MyExitThread();
}

void t13() {
	MyInitThreads();
	t13_func(0);
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
	{"protected_stack", t11},
	{"double_jmp_buf", t12},
	{"all75", t13},
	{0}
};
