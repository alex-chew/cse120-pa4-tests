#ifndef TESTS_H
#define TESTS_H

#define TEST_NO_MAIN

#include <string.h>
#include "../aux.h"
#include "../umix.h"
#include "../mykernel4.h"
#include "../acutest.h"

#ifdef USE_REFERENCE_KERNEL
#define MyInitThreads  InitThreads
#define MyCreateThread CreateThread
#define MyGetThread    GetThread
#define MyYieldThread  YieldThread
#define MySchedThread  SchedThread
#define MyExitThread   ExitThread
#endif

#define STACKSIZE	65536		// maximum size of thread stack

#endif
