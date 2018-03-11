#include "tests.h"

/**
 * Stress test under thread churn, but with at most 1 available thread ID at
 * any time (i.e. either 9 or 10 threads are active).
 */

#define T7_ROUNDS 1000

static struct {
	unsigned int failed;
} d7;

static void t7_func(int round) {
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

void churn9() {
	MyInitThreads();
	d7.failed = 0;
	for (int i = 1; i <= 8; ++i) {
		MyCreateThread(t7_func, i + 1);
	}
	t7_func(1);
	MyExitThread();
}
