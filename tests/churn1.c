#include "tests.h"

/**
 * Stress test under thread churn, but with at most two active threads at a
 * time (T0 spawns T1, T0 exits so T1 gets scheduled, T1 spawns T2, etc.).
 */

#define T8_ROUNDS 1000

static struct {
	unsigned int failed;
} d8;

static void t8_func(int round) {
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

void churn1() {
	MyInitThreads();
	d8.failed = 0;
	t8_func(1);
	MyExitThread();
}
