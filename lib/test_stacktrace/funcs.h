// SPDX-License-Identifier: GPL-2.0
#ifndef TEST_STACKTRACE_FUNCS_H
#define TEST_STACKTRACE_FUNCS_H

typedef void *(*st_callback_t)(void *arg);

void *test_stacktrace_caller_0(st_callback_t callback, void *arg);
void *test_stacktrace_caller_1(st_callback_t callback, void *arg);
void *test_stacktrace_caller_2(st_callback_t callback, void *arg);
void *test_stacktrace_caller_3(st_callback_t callback, void *arg);

void test_stacktrace_not_called(void);

#endif /* TEST_STACKTRACE_FUNCS_H */

