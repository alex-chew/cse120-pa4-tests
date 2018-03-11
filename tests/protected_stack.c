#include "tests.h"

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

static void t11_func(int tid) {
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

void protected_stack() {
	MyInitThreads();
	t11_func(0);
}
