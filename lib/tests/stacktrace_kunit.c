// SPDX-License-Identifier: GPL-2.0-only
/*
 * Tests for reliable stacktrace
 */

#include <linux/compiler.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>

#include <kunit/test.h>

/*
 * The async tests simulate taking an interrupt from regular C code, and
 * unwinding within the interrupt context.
 */
#if defined(CONFIG_KPROBES)
#define HAVE_ASYNC_TESTS
#endif

/*
 * The async_fgraph tests simulate taking an interrupt from the fgraph return
 * trampoline (return_to_handler) and unwinding within the interrupt context.
 */
#if defined(CONFIG_KPROBES) && defined(CONFIG_FUNCTION_GRAPH_TRACER)
#define HAVE_ASYNC_FGRAPH_TESTS
#endif

#define NR_STACKTRACE_ELEMS	100

struct stacktrace_test_data {
	unsigned long stack_trace[NR_STACKTRACE_ELEMS];
	int len;
	bool hit;
};

static void std_dump(struct kunit *test, struct stacktrace_test_data *std)
{
	kunit_info(test, "stacktrace hit: %s, length: %d\n",
		   std->hit ? "YES" : "NO",
		   std->len);

	for (int i = 0; i < std->len; i++) {
		kunit_info(test, "  %3d => %pS\n", i,
			   (void *)std->stack_trace[i]);
	}
}

static int std_test_init(struct kunit *test)
{
	struct stacktrace_test_data *std = kunit_kzalloc(test, sizeof(*std), GFP_KERNEL);
	if (!std)
		return -ENOMEM;

	test->priv = std;
	return 0;
}

static void std_test_exit(struct kunit *test)
{
	if (test->status == KUNIT_FAILURE)
		std_dump(test, test->priv);

	kunit_kfree(test, test->priv);
}

static void std_save_stacktrace_reliable(struct stacktrace_test_data *std)
{
	std->len = stack_trace_save_tsk_reliable(current,
						 std->stack_trace,
						 NR_STACKTRACE_ELEMS);
	std->hit = true;
}

typedef void *(*callee_t)(void *arg);
typedef void *(*caller_t)(callee_t callee, void *arg);

static noinline void *callee_nop(void *arg)
{
	/* Inhibit dead code elimination */
	return OPTIMIZER_HIDE_EXPR(arg);
}

static noinline void *callee_unwind_reliable(void *arg)
{
	std_save_stacktrace_reliable(current->kunit_test->priv);
	return OPTIMIZER_HIDE_EXPR(arg);
}

static noinline void *callee_tail_nop(void *arg)
{
	/* Permit tail call optimization */
	return callee_nop(arg);
}

static noinline void *callee_notail_nop(void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(callee_nop(arg));
}

static noinline void *caller_tail_unwind_reliable(void *arg)
{
	/* Permit tail call optimization */
	return callee_unwind_reliable(arg);
}

static noinline void *caller_notail_unwind_reliable(void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(callee_unwind_reliable(arg));
}

static noinline void *caller_1(callee_t callee, void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(callee(arg));
}

static noinline void *caller_2(callee_t callee, void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(caller_1(callee, arg));
}

static noinline void *caller_3(callee_t callee, void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(caller_2(callee, arg));
}

static int lookup_sym_bounds(void *sym, unsigned long *start,
			     unsigned long *end)
{
	unsigned long offset, size;

	if (!kallsyms_lookup_size_offset((unsigned long)sym, &size, &offset))
		return -ENOENT;

	if (offset) {
		WARN_ONCE(1, "%pS has non zero offset: %ld\n", sym, offset);
		return -EINVAL;
	}

	*start = (unsigned long)sym;
	*end = (unsigned long)sym + size;

	return 0;
}

static int stacktrace_find_sym(struct stacktrace_test_data *std,
			       void *sym)
{
	unsigned long start, end;
	int ret;

	if (std->len < 0)
		return std->len;

	ret = lookup_sym_bounds(sym, &start, &end);
	if (ret)
		return ret;

	for (int i = 0; i < std->len; i++) {
		unsigned long pc = std->stack_trace[i];
		if (start <= pc && pc < end)
			return i;
	}

	return -ENOENT;
}

/*
 * Generate each possible instruction pointer value within a given function.
 */
static inline const void *
__gen_ips_symbol(void *symbol, const void *prev, char *desc)
{
	unsigned long cur = (unsigned long)prev;
	unsigned long start, end;
	int ret;

	ret = lookup_sym_bounds(symbol, &start, &end);
	if (WARN_ON_ONCE(ret))
		return NULL;

	/*
	 * TODO: find the next instruction boundary better.
	 */
	cur = cur ? cur + 1 : start;
	if (cur == end)
		cur = 0;

	sprintf(desc, "%pS", (void *)cur);

	return (void *)cur;
}

typedef void (*std_expect_t)(struct kunit *test,
			     struct stacktrace_test_data *std);

static void std_expect_empty(struct kunit *test,
			     struct stacktrace_test_data *std)
{
	KUNIT_EXPECT_EQ(test, std->hit, false);
	KUNIT_EXPECT_EQ(test, std->len, 0);
}

struct stacktrace_test_params {
	void **trace;
};

struct stacktrace_test_params basic_scenario = {
	.trace = (void *[]) { caller_1, caller_2, caller_3, NULL },
};

/*
 * Check that we have recorded the expected callchain.
 *
 * Every function in the expected trace for the scenario must exist in the real
 * trace, and must be ordered as with the expected trace.
 *
 * Other calls may exist in the trace, so we only check that the expected
 * functions exist and are ordered relative to one another, and do not check
 * their absolute positions in the trace, nor do we check their absolute
 * offsets from one another.
 */
static void std_expect_trace(struct kunit *test,
			     struct stacktrace_test_data *std,
			     void **expected_trace)
{
	unsigned int idx_min = 0;

	for (void **trace = expected_trace; *trace != NULL; trace++) {
		int idx_caller = stacktrace_find_sym(std, *trace);

		if (idx_caller < 0) {
			KUNIT_FAIL(test, "Stacktrace misses %ps\n", *trace);
			return;
		}

		if (idx_caller < idx_min) {
			KUNIT_FAIL(test, "Stacktrace ordered incorrectly\n");
			return;
		}

		idx_min = idx_caller + 1;
	}
}

static void std_expect_callers_present(struct kunit *test,
				       struct stacktrace_test_data *std)
{

	std_expect_trace(test, std, basic_scenario.trace);
}

static void std_expect_callers_present_reliable(struct kunit *test,
						struct stacktrace_test_data *std)
{
	if (std->len < 0) {
		kunit_info(test, "marked unreliable: %d", std->len);
		return;
	}

	std_expect_callers_present(test, std);
}

static void __do_test_basic(struct kunit *test,
			    callee_t callee,
			    std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;

	void *ret = caller_3(callee, NULL);
	KUNIT_EXPECT_PTR_EQ(test, ret, NULL);

	expect(test, std);
}

static void test_nounwind(struct kunit *test)
{
	__do_test_basic(test, callee_nop, std_expect_empty);
}

static void test_nounwind_tail(struct kunit *test)
{
	__do_test_basic(test, callee_tail_nop, std_expect_empty);
}

static void test_nounwind_notail(struct kunit *test)
{
	__do_test_basic(test, callee_notail_nop, std_expect_empty);
}

static void test_unwind_reliable(struct kunit *test)
{
	__do_test_basic(test,
			callee_unwind_reliable,
			std_expect_callers_present_reliable);
}

static void test_unwind_reliable_tail(struct kunit *test)
{
	__do_test_basic(test,
			caller_tail_unwind_reliable,
			std_expect_callers_present_reliable);
}

static void test_unwind_reliable_notail(struct kunit *test)
{
	__do_test_basic(test,
			caller_notail_unwind_reliable,
			std_expect_callers_present_reliable);
}

#ifdef CONFIG_FUNCTION_TRACER
static void fops_nop(unsigned long ip,
		     unsigned long parent_ip,
		     struct ftrace_ops *op,
		     struct ftrace_regs *fregs)
{
	/* do nothing */
}

static void fops_unwind_reliable(unsigned long ip,
				 unsigned long parent_ip,
				 struct ftrace_ops *op,
				 struct ftrace_regs *fregs)
{
	std_save_stacktrace_reliable(current->kunit_test->priv);
}

static int __do_ftrace_unwind(callee_t callee,
			      const void *tracee,
			      ftrace_func_t fops_func,
			      unsigned long fops_flags)
{
	struct ftrace_ops ops = {
		.func = fops_func,
		.flags = fops_flags,
	};
	int ret;

	ret = ftrace_set_filter_ip(&ops, (unsigned long)tracee,
				   0, 0);
	if (ret)
		goto out;

	ret = register_ftrace_function(&ops);
	if (ret)
		goto out_free_filter;

	caller_3(callee, NULL);

	unregister_ftrace_function(&ops);

out_free_filter:
	ftrace_free_filter(&ops);
out:
	return ret;
}

static void __do_test_ftrace(struct kunit *test,
			     callee_t callee,
			     const void *tracee,
			     ftrace_func_t fops_func,
			     unsigned long fops_flags,
			     std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;
	int ret;

	ret = __do_ftrace_unwind(callee, tracee,
				 fops_func, fops_flags);
	KUNIT_ASSERT_EQ(test, ret, 0);

	expect(test, std);
}

static void test_nounwind_with_ftrace(struct kunit *test)
{
	__do_test_ftrace(test,
			 callee_nop, callee_nop,
			 fops_nop, 0,
			 std_expect_empty);
}

static void test_unwind_reliable_with_ftrace(struct kunit *test)
{
	__do_test_ftrace(test,
			 callee_unwind_reliable, callee_unwind_reliable,
			 fops_nop, 0,
			 std_expect_callers_present_reliable);
}

static void test_ftrace_unwind_reliable(struct kunit *test)
{
	__do_test_ftrace(test,
			 callee_nop, callee_nop,
			 fops_unwind_reliable, 0,
			 std_expect_callers_present_reliable);
}

#endif /* CONFIG_FUNCTION_TRACER */

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
static int gops_entry_func_nop(struct ftrace_graph_ent *trace,
			       struct fgraph_ops *gops,
			       struct ftrace_regs *regs)
{
	return 1;
}

static int gops_entry_func_unwind_reliable(struct ftrace_graph_ent *trace,
					   struct fgraph_ops *gops,
					   struct ftrace_regs *regs)
{
	std_save_stacktrace_reliable(current->kunit_test->priv);
	return 1;
}

static void gops_return_func_nop(struct ftrace_graph_ret *trace,
				 struct fgraph_ops *gops,
				 struct ftrace_regs *regs)
{
	/* do nothing */
}

static void gops_return_func_unwind_reliable(struct ftrace_graph_ret *trace,
					     struct fgraph_ops *gops,
					     struct ftrace_regs *regs)
{
	std_save_stacktrace_reliable(current->kunit_test->priv);
}

static int __do_fgraph_unwind(callee_t callee,
			      unsigned long *tracees, int cnt,
			      trace_func_graph_ent_t entry_func,
			      trace_func_graph_ret_t return_func)
{
	struct fgraph_ops gops = {
		.entryfunc = entry_func,
		.retfunc = return_func,
	};
	int ret;

	ret = ftrace_set_filter_ips(&gops.ops, tracees, cnt, 0, 1);
	if (ret)
		goto out;

	ret = register_ftrace_graph(&gops);
	if (ret)
		goto out_free_filter;

	caller_3(callee, NULL);

	unregister_ftrace_graph(&gops);
out_free_filter:
	ftrace_free_filter(&gops.ops);
out:
	return ret;
}

static void __do_test_fgraph_many(struct kunit *test,
				  callee_t callee,
				  unsigned long *tracees, unsigned int cnt,
				  trace_func_graph_ent_t entry_func,
				  trace_func_graph_ret_t return_func,
				  std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;
	int ret;

	ret = __do_fgraph_unwind(callee, tracees, cnt,
				 entry_func, return_func);
	KUNIT_ASSERT_EQ(test, ret, 0);

	expect(test, std);
}

static void __do_test_fgraph(struct kunit *test,
			     callee_t callee,
			     const void *tracee,
			     trace_func_graph_ent_t entry_func,
			     trace_func_graph_ret_t return_func,
			     std_expect_t expect)
{
	unsigned long tracees[] = { (unsigned long)tracee };

	__do_test_fgraph_many(test, callee,
			      tracees, ARRAY_SIZE(tracees),
			      entry_func, return_func,
			      expect);
}

static void test_nounwind_with_fgraph(struct kunit *test)
{
	__do_test_fgraph(test,
			 callee_nop, callee_nop,
			 gops_entry_func_nop,
			 gops_return_func_nop,
			 std_expect_empty);
}

static void test_nounwind_with_fgraph_tail(struct kunit *test)
{
	unsigned long tracees[] = {
		(unsigned long)callee_nop,
		(unsigned long)callee_tail_nop,
	};

	__do_test_fgraph_many(test,
			      callee_tail_nop,
			      tracees, ARRAY_SIZE(tracees),
			      gops_entry_func_nop,
			      gops_return_func_nop,
			      std_expect_empty);
}

static void test_nounwind_with_fgraph_notail(struct kunit *test)
{
	unsigned long tracees[] = {
		(unsigned long)callee_nop,
		(unsigned long)callee_notail_nop,
	};

	__do_test_fgraph_many(test,
			      callee_notail_nop,
			      tracees, ARRAY_SIZE(tracees),
			      gops_entry_func_nop,
			      gops_return_func_nop,
			      std_expect_empty);
}

static void test_unwind_reliable_with_fgraph(struct kunit *test)
{
	__do_test_fgraph(test,
			 callee_unwind_reliable, callee_unwind_reliable,
			 gops_entry_func_nop,
			 gops_return_func_nop,
			 std_expect_callers_present_reliable);
}

static void test_unwind_reliable_with_fgraph_tail(struct kunit *test)
{
	unsigned long tracees[] = {
		(unsigned long)callee_unwind_reliable,
		(unsigned long)caller_tail_unwind_reliable,
	};

	__do_test_fgraph_many(test,
			      caller_tail_unwind_reliable,
			      tracees, ARRAY_SIZE(tracees),
			      gops_entry_func_nop,
			      gops_return_func_nop,
			      std_expect_callers_present_reliable);
}

static void test_unwind_reliable_with_fgraph_notail(struct kunit *test)
{
	unsigned long tracees[] = {
		(unsigned long)callee_unwind_reliable,
		(unsigned long)caller_notail_unwind_reliable,
	};

	__do_test_fgraph_many(test,
			      caller_notail_unwind_reliable,
			      tracees, ARRAY_SIZE(tracees),
			      gops_entry_func_nop,
			      gops_return_func_nop,
			      std_expect_callers_present_reliable);
}

static void test_fgraph_entry_unwind_reliable(struct kunit *test)
{
	__do_test_fgraph(test,
			 callee_nop, callee_nop,
			 gops_entry_func_unwind_reliable,
			 gops_return_func_nop,
			 std_expect_callers_present_reliable);
}

static void test_fgraph_exit_unwind_reliable(struct kunit *test)
{
	__do_test_fgraph(test,
			 callee_nop, callee_nop,
			 gops_entry_func_nop,
			 gops_return_func_unwind_reliable,
			 std_expect_callers_present_reliable);
}

#endif /* CONFIG_FUNCTION_GRAPH_TRACER */

#ifdef CONFIG_KPROBES
static int kp_nop(struct kprobe *kp, struct pt_regs *regs)
{
	return 0;
}

static int kp_unwind_reliable(struct kprobe *kp, struct pt_regs *regs)
{
	std_save_stacktrace_reliable(current->kunit_test->priv);

	return 0;
}

static bool __do_kprobe_unwind(struct kunit *test,
			       callee_t callee,
			       const void *kp_addr,
			       kprobe_pre_handler_t handler)
{
	struct kprobe kp = {
		.addr = (kprobe_opcode_t *)kp_addr,
		.pre_handler = handler,
	};
	int ret;

	if (within_kprobe_blacklist((unsigned long)kp_addr)) {
		kunit_info(test, "cannot kprobe %pS (blacklist)", kp_addr);
		return false;
	}

	ret = register_kprobe(&kp);
	if (ret) {
		kunit_info(test, "cannot kprobe %pS (error %d)", kp_addr, ret);
		return false;
	}
	caller_3(callee, NULL);

	unregister_kprobe(&kp);

	return true;
}

static void __do_test_kprobe(struct kunit *test,
			     callee_t callee,
			     const void *tracee,
			     kprobe_pre_handler_t kp_handler,
			     std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;

	if (!__do_kprobe_unwind(test, callee, tracee, kp_handler)) {
		KUNIT_FAIL(test, "Unable to kprobe %pS\n", tracee);
		return;
	}

	expect(test, std);
}

static void test_nounwind_with_kprobe(struct kunit *test)
{
	__do_test_kprobe(test,
			 callee_nop, callee_nop,
			 kp_nop,
			 std_expect_empty);
}

static void test_unwind_reliable_with_kprobe(struct kunit *test)
{
	__do_test_kprobe(test,
			 callee_unwind_reliable, callee_unwind_reliable,
			 kp_nop,
			 std_expect_callers_present_reliable);
}

static void test_kprobe_unwind_reliable(struct kunit *test)
{
	__do_test_kprobe(test,
			 callee_nop, callee_nop,
			 kp_unwind_reliable,
			 std_expect_callers_present_reliable);
}

#endif /* CONFIG_KPROBES */

#ifdef HAVE_ASYNC_TESTS
static noinline void *callee_nop_caller(void *arg)
{
	/* Inhibit tail call optimization */
	return OPTIMIZER_HIDE_EXPR(callee_nop(arg));
}

static inline const void *
gen_ips_nop_caller(struct kunit *test, const void *prev, char *desc)
{
	return __gen_ips_symbol(callee_nop_caller, prev, desc);
}

static void __do_test_one_async_kp(struct kunit *test,
				  callee_t callee,
				  const void *tracee,
				  kprobe_pre_handler_t kp_handler,
				  std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;

	if (!__do_kprobe_unwind(test, callee, tracee, kp_handler))
		return;

	if (!std->hit) {
		kunit_info(test, "did not hit kprobe at %pS", tracee);
		return;
	}

	kunit_info(test, "hit kprobe at %pS", tracee);

	expect(test, std);
}

static void test_async_unwind_reliable(struct kunit *test)
{
	__do_test_one_async_kp(test,
			       callee_nop_caller,
			       test->param_value,
			       kp_unwind_reliable,
			       std_expect_callers_present_reliable);
}
#endif /* HAVE_ASYNC_TESTS */

#ifdef HAVE_ASYNC_FGRAPH_TESTS
static inline const void *
gen_ips_return_to_handler(struct kunit *test, const void *prev, char *desc)
{
	return __gen_ips_symbol(return_to_handler, prev, desc);
}

static int __do_async_fgraph_unwind(struct kunit *test,
				    const void *tracee,
				    const void *kp_addr,
				    kprobe_pre_handler_t handler)
{
	struct fgraph_ops gops = {
		.entryfunc = gops_entry_func_nop,
		.retfunc = gops_return_func_nop,
	};
	int ret;

	ret = ftrace_set_filter_ip(&gops.ops, (unsigned long)tracee, 0, 1);
	if (ret)
		goto out;

	ret = register_ftrace_graph(&gops);
	if (ret)
		goto out_free_filter;

	__do_kprobe_unwind(test, callee_nop, kp_addr, handler);

	unregister_ftrace_graph(&gops);
out_free_filter:
	ftrace_free_filter(&gops.ops);
out:
	return ret;
}

static void __do_test_fgraph_async(struct kunit *test,
				   kprobe_pre_handler_t kp_handler,
				   std_expect_t expect)
{
	struct stacktrace_test_data *std = test->priv;
	const void *kp_addr = test->param_value;
	const void *tracee = callee_nop;
	int ret;

	ret = __do_async_fgraph_unwind(test, tracee, kp_addr, kp_handler);
	if (ret) {
		KUNIT_FAIL(test, "cannot hook %pS with fgraph\n", tracee);
		return;
	}

	if (!std->hit) {
		kunit_info(test, "did not hit kprobe at %pS", kp_addr);
		return;
	}

	kunit_info(test, "hit kprobe at %pS", kp_addr);

	expect(test, std);
}

static void test_fgraph_async_unwind_reliable(struct kunit *test)
{
	__do_test_fgraph_async(test, kp_unwind_reliable,
			       std_expect_callers_present_reliable);
}

#endif /* HAVE_ASYNC_FGRAPH_TESTS */

static struct kunit_case reliable_stacktrace_test_cases[] = {
	/*
	 * Test that our callchain scenarios work correctly in the absence of
	 * testing. If these fail, the rest of the tests are useless.
	 */
	KUNIT_CASE(test_nounwind),
	KUNIT_CASE(test_nounwind_tail),
	KUNIT_CASE(test_nounwind_notail),

	/*
	 * Test that direct unwinding works for a variety of scenarios.
	 */
	KUNIT_CASE(test_unwind_reliable),
	KUNIT_CASE(test_unwind_reliable_tail),
	KUNIT_CASE(test_unwind_reliable_notail),

#ifdef CONFIG_FUNCTION_TRACER
	/*
	 * Test that ftrace doesn't break regular code, including unwinding in
	 * regular code.
	 */
	KUNIT_CASE(test_nounwind_with_ftrace),
	KUNIT_CASE(test_unwind_reliable_with_ftrace),

	/*
	 * Test that unwinding works within ftrace tracers.
	 */
	KUNIT_CASE(test_ftrace_unwind_reliable),
#endif /* CONFIG_FUNCTION_TRACER */

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	/*
	 * Test that fgraph doesn't break regular code, including unwinding in
	 * regular code.
	 */
	KUNIT_CASE(test_nounwind_with_fgraph),
	KUNIT_CASE(test_nounwind_with_fgraph_tail),
	KUNIT_CASE(test_nounwind_with_fgraph_notail),
	KUNIT_CASE(test_unwind_reliable_with_fgraph),
	KUNIT_CASE(test_unwind_reliable_with_fgraph_tail),
	KUNIT_CASE(test_unwind_reliable_with_fgraph_notail),

	/*
	 * Test that unwinding works within fgraph handlers.
	 */
	KUNIT_CASE(test_fgraph_entry_unwind_reliable),
	KUNIT_CASE(test_fgraph_exit_unwind_reliable),
#endif /* CONFIG_FUNCTION_GRAPH_TRACER */

#ifdef CONFIG_KPROBES
	/*
	 * Test that kprobes doesn't break regular code, including unwinding in
	 * regular code.
	 */
	KUNIT_CASE(test_nounwind_with_kprobe),
	KUNIT_CASE(test_unwind_reliable_with_kprobe),

	/*
	 * Test that unwindinw works within kprobe handlers.
	 */
	KUNIT_CASE(test_kprobe_unwind_reliable),
#endif /* CONFIG_KPROBES */

#ifdef HAVE_ASYNC_TESTS
	KUNIT_CASE_PARAM(test_async_unwind_reliable,
			 gen_ips_nop_caller),
#endif /* HAVE_ASYNC_TESTS */

#ifdef HAVE_ASYNC_FGRAPH_TESTS
	KUNIT_CASE_PARAM(test_fgraph_async_unwind_reliable,
			 gen_ips_return_to_handler),
#endif /* HAVE_ASYNC_FGRAPH_TESTS */

	{ /* sentinel */ }
};

static struct kunit_suite reliable_stacktrace_test_suite = {
	.name = "reliable_stacktrace",
	.test_cases = reliable_stacktrace_test_cases,
	.init = std_test_init,
	.exit = std_test_exit,
};
kunit_test_suites(&reliable_stacktrace_test_suite);
