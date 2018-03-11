#include "tests.h"

/** Init/Exit should work for just one thread (namely, the main thread). */
void init_exit_one() {
	MyInitThreads();
	TEST_CHECK(MyGetThread() == 0);
	MyExitThread();
}
