/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_STACKTRACE_H
#define __ASM_STACKTRACE_H

#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/types.h>
#include <linux/llist.h>

#include <asm/memory.h>
#include <asm/ptrace.h>
#include <asm/sdei.h>

enum stack_type {
	STACK_TYPE_UNKNOWN,
	STACK_TYPE_TASK,
	STACK_TYPE_IRQ,
	STACK_TYPE_OVERFLOW,
	STACK_TYPE_SDEI_NORMAL,
	STACK_TYPE_SDEI_CRITICAL,
	__NR_STACK_TYPES
};

struct stack_info {
	unsigned long low;
	unsigned long high;
	enum stack_type type;
};

extern void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk,
			   const char *loglvl);

DECLARE_PER_CPU(unsigned long *, irq_stack_ptr);

static inline bool stackinfo_on_stack(const struct stack_info *info,
				      unsigned long sp, unsigned long size)
{
	if (!info->low)
		return false;

	if (sp < info->low || sp + size < sp || sp + size > info->high)
		return false;

	return true;
}

static struct stack_info stackinfo_get_unknown(void)
{
	return (struct stack_info) {
		.low = 0,
		.high = 0,
		.type = STACK_TYPE_UNKNOWN,
	};
}

static struct stack_info stackinfo_get_irq(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(irq_stack_ptr);
	unsigned long high = low + IRQ_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
		.type = STACK_TYPE_IRQ,
	};
}

static inline bool on_irq_stack(unsigned long sp, unsigned long size)
{
	const struct stack_info irq_info = stackinfo_get_irq();
	return stackinfo_on_stack(&irq_info, sp, size);
}

static struct stack_info stackinfo_get_task(const struct task_struct *tsk)
{
	unsigned long low = (unsigned long)task_stack_page(tsk);
	unsigned long high = low + THREAD_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
		.type = STACK_TYPE_TASK,
	};
}

static inline bool on_task_stack(const struct task_struct *tsk,
				 unsigned long sp, unsigned long size)
{
	const struct stack_info tsk_info = stackinfo_get_task(tsk);
	return stackinfo_on_stack(&tsk_info, sp, size);
}

#ifdef CONFIG_VMAP_STACK
DECLARE_PER_CPU(unsigned long [OVERFLOW_STACK_SIZE/sizeof(long)], overflow_stack);

static struct stack_info stackinfo_get_overflow(void)
{
	unsigned long low = (unsigned long)raw_cpu_ptr(overflow_stack);
	unsigned long high = low + OVERFLOW_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
		.type = STACK_TYPE_OVERFLOW,
	};
}

#else
#define stackinfo_get_overflow()	stackinfo_get_unknown()
#endif

#if defined(CONFIG_ARM_SDE_INTERFACE) && defined(CONFIG_VMAP_STACK)
DECLARE_PER_CPU(unsigned long *, sdei_shadow_call_stack_normal_ptr);
DECLARE_PER_CPU(unsigned long *, sdei_shadow_call_stack_critical_ptr);

static struct stack_info stackinfo_get_sdei_normal(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(sdei_stack_normal_ptr);
	unsigned long high = low + SDEI_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
		.type = STACK_TYPE_SDEI_NORMAL,
	};
}

static struct stack_info stackinfo_get_sdei_critical(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(sdei_stack_critical_ptr);
	unsigned long high = low + SDEI_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
		.type = STACK_TYPE_SDEI_CRITICAL,
	};
}
#else
#define stackinfo_get_sdei_normal()	stackinfo_get_unknown()
#define stackinfo_get_sdei_critical()	stackinfo_get_unknown()
#endif

struct stack_context {
	struct stack_info stacks[__NR_STACK_TYPES];
};

static __always_inline struct stack_context
stackinfo_get_ctx_accessible(const struct task_struct *tsk)
{
	struct stack_context ctx = {
		.stacks[0 ... __NR_STACK_TYPES - 1] = stackinfo_get_unknown(),
	};

	ctx.stacks[STACK_TYPE_TASK] = stackinfo_get_task(tsk);

	if (tsk != current || preemptible())
		goto out;

	ctx.stacks[STACK_TYPE_IRQ] = stackinfo_get_irq();
	ctx.stacks[STACK_TYPE_OVERFLOW] = stackinfo_get_overflow();

	if (!IS_ENABLED(CONFIG_VMAP_STACK) ||
	    !IS_ENABLED(CONFIG_ARM_SDE_INTERFACE) ||
	    !in_nmi())
	    goto out;

	ctx.stacks[STACK_TYPE_SDEI_NORMAL] = stackinfo_get_sdei_normal();
	ctx.stacks[STACK_TYPE_SDEI_CRITICAL] = stackinfo_get_sdei_critical();

out:
	return ctx;
}

/*
 * We can only safely access per-cpu stacks from current in a non-preemptible
 * context.
 */
static inline bool on_accessible_stack(const struct task_struct *tsk,
				       unsigned long sp, unsigned long size,
				       struct stack_info *info)
{
	struct stack_context accessible = stackinfo_get_ctx_accessible(tsk);

	for (int i = 0; i < __NR_STACK_TYPES; i++) {
		struct stack_info tmp = accessible.stacks[i];
		if (!stackinfo_on_stack(&tmp, sp, size))
			continue;

		*info = tmp;
		return true;
	}

	*info = stackinfo_get_unknown();
	return false;
}
#endif	/* __ASM_STACKTRACE_H */
