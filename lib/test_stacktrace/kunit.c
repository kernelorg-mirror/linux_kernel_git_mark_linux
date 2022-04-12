// SPDX-License-Identifier: GPL-2.0
/*
 * Stacktrace KUnit tests
 */

#include <kunit/test.h>
#include <linux/kallsyms.h>
#include <linux/stacktrace.h>

#include "funcs.h"

#define MAX_STACK_ENTRIES 100

static void *callback_return_arg(void *arg)
{
	return arg;
}

static void test_stacktrace_caller_funcs(struct kunit *test)
{
	void *result = test_stacktrace_caller_3(callback_return_arg, NULL);

	KUNIT_EXPECT_PTR_EQ(test, result, NULL);
}

struct st_data {
	unsigned long store[MAX_STACK_ENTRIES];
	int nr_entries;
};

static void *callback_st_save(void *arg)
{
	struct st_data *st = arg;

	st->nr_entries = stack_trace_save(st->store, MAX_STACK_ENTRIES, 0);

	return st;
}

static void *callback_st_save_reliable(void *arg)
{
	struct st_data *st = arg;

	st->nr_entries = stack_trace_save_tsk_reliable(current, st->store,
						       MAX_STACK_ENTRIES);

	return st;
}

/*
 * Find the index of the first instance of `func` within a trace, starting from
 * `start`.
 */
int st_find_func_idx(struct st_data *st, void *func, int start)
{
	unsigned long size;
	unsigned long offset;
	unsigned long addr = (unsigned long)func;

	if (!kallsyms_lookup_size_offset(addr, &size, &offset))
		return st->nr_entries;

	for (int i = start; i < st->nr_entries; i++) {
		unsigned long elem = st->store[i];

		if (elem < addr)
			continue;
		if (elem >= addr + size)
			continue;

		return i;
	}

	return st->nr_entries;
}

static bool st_contains_func(struct st_data *st, void *func)
{
	return st_find_func_idx(st, func, 0) < st->nr_entries;
}

static void test_stacktrace_no_uncalled(struct kunit *test)
{
	struct st_data st;

	test_stacktrace_caller_3(callback_st_save, &st);

	KUNIT_ASSERT_GE(test, st.nr_entries, 0);

	KUNIT_EXPECT_FALSE(test, st_contains_func(&st, test_stacktrace_not_called));
}

#define KUNIT_EXPECT_CALLERS_INCLUDED(test, st)						\
do {											\
	KUNIT_EXPECT_TRUE(test, st_contains_func(&st, test_stacktrace_caller_0));	\
	KUNIT_EXPECT_TRUE(test, st_contains_func(&st, test_stacktrace_caller_1));	\
	KUNIT_EXPECT_TRUE(test, st_contains_func(&st, test_stacktrace_caller_2));	\
	KUNIT_EXPECT_TRUE(test, st_contains_func(&st, test_stacktrace_caller_3));	\
} while (0)


#define KUNIT_ASSERT_CALLERS_ORDERED(test, st)						\
do {											\
	int idx = 0;									\
											\
	idx = st_find_func_idx(&st, test_stacktrace_caller_0, idx);			\
	KUNIT_ASSERT_TRUE(test, idx < st.nr_entries);					\
											\
	idx = st_find_func_idx(&st, test_stacktrace_caller_1, idx);			\
	KUNIT_ASSERT_TRUE(test, idx < st.nr_entries);					\
											\
	idx = st_find_func_idx(&st, test_stacktrace_caller_2, idx);			\
	KUNIT_ASSERT_TRUE(test, idx < st.nr_entries);					\
											\
	idx = st_find_func_idx(&st, test_stacktrace_caller_3, idx);			\
	KUNIT_ASSERT_TRUE(test, idx < st.nr_entries);					\
} while (0)

static void test_stacktrace_callers_included(struct kunit *test)
{
	struct st_data st;

	test_stacktrace_caller_3(callback_st_save, &st);

	KUNIT_ASSERT_GE(test, st.nr_entries, 0);

	KUNIT_EXPECT_CALLERS_INCLUDED(test, st);
}

static void test_stacktrace_reliable_callers_included(struct kunit *test)
{
	struct st_data st;

	if (!IS_ENABLED(CONFIG_HAVE_RELIABLE_STACKTRACE))
		kunit_skip(test, "CONFIG_HAVE_RELIABLE_STACKTRACE not available");

	test_stacktrace_caller_3(callback_st_save_reliable, &st);

	KUNIT_ASSERT_GE(test, st.nr_entries, 0);

	KUNIT_EXPECT_CALLERS_INCLUDED(test, st);
}


static void test_stacktrace_callers_ordered(struct kunit *test)
{
	struct st_data st;

	test_stacktrace_caller_3(callback_st_save, &st);

	KUNIT_ASSERT_GE(test, st.nr_entries, 0);

	KUNIT_ASSERT_CALLERS_ORDERED(test, st);
}

static void test_stacktrace_reliable_callers_ordered(struct kunit *test)
{
	struct st_data st;

	if (!IS_ENABLED(CONFIG_HAVE_RELIABLE_STACKTRACE))
		kunit_skip(test, "CONFIG_HAVE_RELIABLE_STACKTRACE not available");

	test_stacktrace_caller_3(callback_st_save_reliable, &st);

	KUNIT_ASSERT_GE(test, st.nr_entries, 0);

	KUNIT_ASSERT_CALLERS_ORDERED(test, st);
}

static struct kunit_case test_stacktrace_cases[] = {
	KUNIT_CASE(test_stacktrace_caller_funcs),
	KUNIT_CASE(test_stacktrace_no_uncalled),
	KUNIT_CASE(test_stacktrace_callers_included),
	KUNIT_CASE(test_stacktrace_reliable_callers_included),
	KUNIT_CASE(test_stacktrace_callers_ordered),
	KUNIT_CASE(test_stacktrace_reliable_callers_ordered),
	{}
};

static struct kunit_suite test_stacktrace_suite = {
	.name = "stacktrace",
	.test_cases = test_stacktrace_cases,
};

kunit_test_suites(&test_stacktrace_suite);

MODULE_LICENSE("GPL v2");
