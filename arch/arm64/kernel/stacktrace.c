// SPDX-License-Identifier: GPL-2.0-only
/*
 * Stack tracing support
 *
 * Copyright (C) 2012 ARM Ltd.
 */
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/task_stack.h>
#include <linux/stacktrace.h>

#include <asm/irq.h>
#include <asm/pointer_auth.h>
#include <asm/stack_pointer.h>
#include <asm/stacktrace.h>

/*
 * A snapshot of a frame record or fp/lr register values, along with some
 * accounting information necessary for robust unwinding.
 *
 * @fp:          The fp value in the frame record (or the real fp)
 * @pc:          The lr value in the frame record (or the real lr)
 *
 * @kr_cur:      When KRETPROBES is selected, holds the kretprobe instance
 *               associated with the most recently encountered replacement lr
 *               value.
 */
struct unwind_state {
	unsigned long fp;
	unsigned long pc;
#ifdef CONFIG_KRETPROBES
	struct llist_node *kr_cur;
#endif
};

static notrace void unwind_init(struct unwind_state *state, unsigned long fp,
				unsigned long pc)
{
	state->fp = fp;
	state->pc = pc;
#ifdef CONFIG_KRETPROBES
	state->kr_cur = NULL;
#endif
}
NOKPROBE_SYMBOL(unwind_init);

/*
 * Unwind from one frame record (A) to the next frame record (B).
 *
 * We terminate early if the location of B indicates a malformed chain of frame
 * records (e.g. a cycle), determined based on the location and fp value of A
 * and the location (but not the fp value) of B.
 */


static notrace int unwind_frame_record(struct unwind_state *state,
				       stack_trace_consume_fn consume_entry, void *cookie)
{
	state->pc = ptrauth_strip_insn_pac(state->pc);

#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	if (tsk->ret_stack &&
		(state->pc == (unsigned long)return_to_handler)) {
		unsigned long orig_pc;
		/*
		 * This is a case where function graph tracer has
		 * modified a return address (LR) in a stack frame
		 * to hook a function return.
		 * So replace it to an original value.
		 */
		orig_pc = ftrace_graph_ret_addr(tsk, NULL, state->pc,
						(void *)state->fp);
		if (WARN_ON_ONCE(state->pc == orig_pc))
			return -EINVAL;
		state->pc = orig_pc;
	}
#endif /* CONFIG_FUNCTION_GRAPH_TRACER */
#ifdef CONFIG_KRETPROBES
	if (is_kretprobe_trampoline(state->pc))
		state->pc = kretprobe_find_ret_addr(tsk, (void *)state->fp, &state->kr_cur);
#endif

	if (!consume_entry(cookie, state->pc))
		return -ENOENT;

	return 0;
}
NOKPROBE_SYMBOL(unwind_frame_record);

/*
 * Unwind frames within a single stack.
 */
static notrace int unwind_one_stack(const struct task_struct *tsk,
				    const struct stack_info *stack,
				    struct unwind_state *state,
				    stack_trace_consume_fn consume_entry, void *cookie)
{
	unsigned long fp = state->fp;
	unsigned long prev_fp = 0;
	int ret;

	while (stackinfo_on_stack(stack, fp, 16)) {

		/* Final frame; nothing to unwind */
		if (fp == (unsigned long)task_pt_regs(tsk)->stackframe)
			return -ENOENT;

		if (fp & 0x7)
			return -EINVAL;

		if (fp <= prev_fp)
			return -EINVAL;

		/*
		 * Read this frame record
		 */
		state->fp = READ_ONCE_NOCHECK(*(unsigned long *)(fp));
		state->pc = READ_ONCE_NOCHECK(*(unsigned long *)(fp + 8));

		prev_fp = fp;
		fp = state->fp;

		ret = unwind_frame_record(state, consume_entry, cookie);
		if (ret)
			return ret;
	}

	return 0;
}
NOKPROBE_SYMBOL(unwind_one_stack);

/*
 * Stacks can nest in several valid orders, e.g.
 *
 * TASK -> IRQ -> OVERFLOW -> SDEI_NORMAL
 * TASK -> SDEI_NORMAL -> SDEI_CRITICAL -> OVERFLOW
 *
 * ... but the nesting itself is strict. Once we transition from one
 * stack to another, it's never valid to unwind back to that first
 * stack.
 */
static int unwind_stacks(struct task_struct *tsk,
			 struct unwind_state *state,
			 stack_trace_consume_fn consume_entry, void *cookie)
{
	struct stack_context accessible = stackinfo_get_ctx_accessible(tsk);

	if (!consume_entry(cookie, state->pc))
		return -ENOENT;

next_stack:
	for (int i = 0; i < __NR_STACK_TYPES; i++) {
		struct stack_info stack = accessible.stacks[i];
		int ret;

		if (!stackinfo_on_stack(&stack, state->fp, 16))
			continue;

		accessible.stacks[i] = stackinfo_get_unknown();

		ret = unwind_one_stack(tsk, &stack, state, consume_entry,
				       cookie);
		if (ret)
			return ret;

		goto next_stack;
	}

	return -EINVAL;
}

static void notrace unwind(struct task_struct *tsk,
			   struct unwind_state *state,
			   stack_trace_consume_fn consume_entry, void *cookie)
{
	unwind_stacks(tsk, state, consume_entry, cookie);
}

static bool dump_backtrace_entry(void *arg, unsigned long where)
{
	char *loglvl = arg;
	printk("%s %pSb\n", loglvl, (void *)where);
	return true;
}

void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk,
		    const char *loglvl)
{
	pr_debug("%s(regs = %p tsk = %p)\n", __func__, regs, tsk);

	if (regs && user_mode(regs))
		return;

	if (!tsk)
		tsk = current;

	if (!try_get_task_stack(tsk))
		return;

	printk("%sCall trace:\n", loglvl);
	arch_stack_walk(dump_backtrace_entry, (void *)loglvl, tsk, regs);

	put_task_stack(tsk);
}

void show_stack(struct task_struct *tsk, unsigned long *sp, const char *loglvl)
{
	dump_backtrace(NULL, tsk, loglvl);
	barrier();
}

noinline notrace void arch_stack_walk(stack_trace_consume_fn consume_entry,
			      void *cookie, struct task_struct *task,
			      struct pt_regs *regs)
{
	struct unwind_state state;

	if (regs)
		unwind_init(&state, regs->regs[29], regs->pc);
	else if (task == current)
		unwind_init(&state,
				(unsigned long)__builtin_frame_address(1),
				(unsigned long)__builtin_return_address(0));
	else
		unwind_init(&state, thread_saved_fp(task),
				thread_saved_pc(task));

	unwind(task, &state, consume_entry, cookie);
}
