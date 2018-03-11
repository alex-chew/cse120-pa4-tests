#include "tests.h"

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

static void t13_func(int _) {
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

void all75() {
	MyInitThreads();
	t13_func(0);
}
