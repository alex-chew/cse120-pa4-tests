#include "tests.h"

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

static void t6_func(int round) {
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

void churn5() {
	MyInitThreads();
	for (int t = 0; t < MAXTHREADS; ++t) { d6.tid_counts[t] = 0; }

	// Run t6_func(0) in T0 to T4
	for (int i = 1; i < 5; ++i) {
		MyCreateThread(t6_func, 1);
	}
	t6_func(1);
}
