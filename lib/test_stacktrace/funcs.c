// SPDX-License-Identifier: GPL-2.0
/*
 * Functions to be unwound.
 */
#include "funcs.h"

noinline void *test_stacktrace_caller_0(st_callback_t callback, void *arg)
{
	return callback(arg);
}

noinline void *test_stacktrace_caller_1(st_callback_t callback, void *arg)
{
	return test_stacktrace_caller_0(callback, arg);
}

noinline void *test_stacktrace_caller_2(st_callback_t callback, void *arg)
{
	return test_stacktrace_caller_1(callback, arg);
}

noinline void *test_stacktrace_caller_3(st_callback_t callback, void *arg)
{
	return test_stacktrace_caller_2(callback, arg);
}

void test_stacktrace_not_called(void)
{
}
