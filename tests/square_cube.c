#include "tests.h"

/**
 * Thread chaining: T2 (cube thread) yields to T1 (square thread); T1 yields
 * to T0 (main thread), T0 yields back to T2, etc.
 *
 * From Exercise C.
 */

static struct {
	int square, cube;
} d1;

static void t1_printSquares(int t) {
	for (int i = 0; i < 5; i++) {
		d1.square = i * i;
		TEST_MSG("T%d: %d squared = %d\n", MyGetThread(), i, d1.square);
		// T1 should only be yielded to from T2
		TEST_CHECK(MyYieldThread(t) == 2);
	}
}

static void t1_printCubes(int t) {
	for (int i = 0; i < 5; i++) {
		d1.cube = i * i * i;
		TEST_MSG("T%d: %d cubed = %d\n", MyGetThread(), i, d1.cube);
		// T1 should only be yielded to from T0
		TEST_CHECK(MyYieldThread(t) == 0);
	}
}

void square_cube() {
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
