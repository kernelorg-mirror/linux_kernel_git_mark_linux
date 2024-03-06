// SPDX-License-Identifier: GPL-2.0-only

#define pr_fmt(fmt)	"fstress: " fmt

#include <linux/bug.h>
#include <linux/errno.h>
#include <linux/ftrace.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>

static atomic_t fstress_panic = ATOMIC_INIT(0);

static void dump_ftrace_ops(struct ftrace_ops *op)
{
	pr_warn("%ps = {\n"
		"  .func = %pS\n"
		"  .flags = 0x%lx\n"
		"  .saved_func = %pS\n"
		"  .trampoline = %pS\n"
		"  .ops_func = %pS\n"
		"  .managed = %pS\n"
		"}\n",
		op,
		op->func,
		op->flags,
		op->saved_func,
		(void *)op->trampoline,
		op->ops_func,
		op->managed);
}

static void report_bad_ops(const char *func,
			   unsigned long ip,
			   unsigned long parent_ip,
			   struct ftrace_ops *op,
			   struct ftrace_regs *fregs,
			   struct ftrace_ops *expected)
{
	pr_warn("%s(%pS, %pS, %pS, %pS) expected ops %pS\n",
		func, (void *)ip, (void *)parent_ip,
		op, fregs, expected);
	dump_ftrace_ops(op);
	dump_ftrace_ops(expected);
}

#define DEFINE_OPS(x)							\
	static void trace_func_##x(unsigned long ip,			\
				   unsigned long parent_ip,		\
				   struct ftrace_ops *op,		\
				   struct ftrace_regs *fregs);		\
									\
	struct ftrace_ops ops_##x =  {					\
		.func = trace_func_##x,					\
	};								\
									\
	static noinstr void trace_func_##x(unsigned long ip,		\
					   unsigned long parent_ip,	\
					   struct ftrace_ops *op,	\
					   struct ftrace_regs *fregs)	\
	{								\
		struct ftrace_ops *expected = &ops_##x;			\
		if (op == expected)					\
			return;						\
		if (raw_atomic_cmpxchg(&fstress_panic, 0, 1))		\
			return;						\
		instrumentation_begin();				\
		report_bad_ops(__func__, ip, parent_ip, op, fregs,	\
			       expected);				\
		instrumentation_end();					\
	}

DEFINE_OPS(a);
DEFINE_OPS(b);
DEFINE_OPS(c);
DEFINE_OPS(d);
DEFINE_OPS(e);
DEFINE_OPS(f);
DEFINE_OPS(g);
DEFINE_OPS(h);

static struct ftrace_ops *fstress_ops[] = {
	&ops_a,
	&ops_b,
	&ops_c,
	&ops_d,
	&ops_e,
	&ops_f,
	&ops_g,
	&ops_h,
};

static noinline void tracee(void)
{
	/*
	 * Do nothing, but prevent compiler from eliding calls to this
	 * function.
	 */
	barrier();
}

static noinline int fstress_tracee_thread(void *unused)
{
	while (!kthread_should_stop()) {
		for (int i = 0; i < 100000; i++)
			tracee();
		schedule();
	}

	return 0;
}

static int fstress_manager_thread(void *unused)
{
	const int nr_ops = ARRAY_SIZE(fstress_ops);

	for (int i = 0; i < nr_ops; i++) {
		ftrace_set_filter_ip(fstress_ops[i], (unsigned long)tracee, 0, 0);
	}

	while (!kthread_should_stop()) {
		for (int i = 0; i <= nr_ops; i++) {
			if (i < nr_ops)
				WARN_ON_ONCE(register_ftrace_function(fstress_ops[i]));
			if (i > 0)
				WARN_ON_ONCE(unregister_ftrace_function(fstress_ops[i - 1]));

			/*
			 * Try to schedule while one trace function is active
			 * (rather than the list func) to maximize the
			 * possibility that threads are preemted while a single
			 * trace function is active.
			 */
			schedule();
		}
	}

	for (int i = 0; i < nr_ops; i++) {
		WARN_ON_ONCE(unregister_ftrace_function(fstress_ops[i]));
		ftrace_free_filter(fstress_ops[i]);
	}

	return 0;
}

#define NR_TRACEE_THREADS	100

struct task_struct *tracee_threads[NR_TRACEE_THREADS];
struct task_struct *manager_thread;

static __init int fstress_init(void)
{
	pr_info("Creating tracee threads\n");
	for (int i = 0; i < NR_TRACEE_THREADS; i++) {
		tracee_threads[i] = kthread_create(fstress_tracee_thread, NULL, "fstress_%d", i);
		if (!tracee_threads[i]) {
			pr_err("Cannot create tracee thread %d\n", i);
			goto out_free_tracees;
		}
	}

	pr_info("Creating manager thread\n");
	manager_thread = kthread_create(fstress_manager_thread, NULL, "fstress_manager");
	if (!manager_thread) {
		pr_err("Cannot create manager thread\n");
		goto out_free_tracees;
	}

	pr_info("Starting threads\n");
	for (int i = 0; i < NR_TRACEE_THREADS; i++)
		wake_up_process(tracee_threads[i]);
	wake_up_process(manager_thread);

	return 0;

out_free_tracees:
	for (int i = 0; i < NR_TRACEE_THREADS; i++) {
		if (tracee_threads[i])
			kthread_stop(tracee_threads[i]);
	}

	return -EAGAIN;
}

late_initcall(fstress_init);
