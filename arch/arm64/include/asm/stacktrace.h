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

struct stack_info {
	unsigned long low;
	unsigned long high;
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

static inline struct stack_info stackinfo_get_unknown(void)
{
	return (struct stack_info) {
		.low = 0,
		.high = 0,
	};
}

static inline struct stack_info stackinfo_get_irq(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(irq_stack_ptr);
	unsigned long high = low + IRQ_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
	};
}

static inline bool on_irq_stack(unsigned long sp, unsigned long size)
{
	const struct stack_info irq_info = stackinfo_get_irq();
	return stackinfo_on_stack(&irq_info, sp, size);
}

static inline struct stack_info stackinfo_get_task(const struct task_struct *tsk)
{
	unsigned long low = (unsigned long)task_stack_page(tsk);
	unsigned long high = low + THREAD_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
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

static inline struct stack_info stackinfo_get_overflow(void)
{
	unsigned long low = (unsigned long)raw_cpu_ptr(overflow_stack);
	unsigned long high = low + OVERFLOW_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
	};
}

#else
#define stackinfo_get_overflow()	stackinfo_get_unknown()
#endif

#if defined(CONFIG_ARM_SDE_INTERFACE) && defined(CONFIG_VMAP_STACK)
DECLARE_PER_CPU(unsigned long *, sdei_shadow_call_stack_normal_ptr);
DECLARE_PER_CPU(unsigned long *, sdei_shadow_call_stack_critical_ptr);

static inline struct stack_info stackinfo_get_sdei_normal(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(sdei_stack_normal_ptr);
	unsigned long high = low + SDEI_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
	};
}

static inline struct stack_info stackinfo_get_sdei_critical(void)
{
	unsigned long low = (unsigned long)raw_cpu_read(sdei_stack_critical_ptr);
	unsigned long high = low + SDEI_STACK_SIZE;

	return (struct stack_info) {
		.low = low,
		.high = high,
	};
}
#else
#define stackinfo_get_sdei_normal()	stackinfo_get_unknown()
#define stackinfo_get_sdei_critical()	stackinfo_get_unknown()
#endif

enum kernel_stack_type {
	STACK_TYPE_TASK,
	STACK_TYPE_IRQ,
	STACK_TYPE_OVERFLOW,
	STACK_TYPE_SDEI_NORMAL,
	STACK_TYPE_SDEI_CRITICAL,
	__NR_STACK_TYPES
};

struct kernel_stack_info {
	struct stack_info stacks[__NR_STACK_TYPES];
};

/*
 * We can only safely access per-cpu stacks from current in a non-preemptible
 * context.
 */

static __always_inline struct kernel_stack_info
get_accessible_kernel_stacks(const struct task_struct *tsk)
{
	struct kernel_stack_info info = {
		.stacks = {
			[0 ... __NR_STACK_TYPES - 1] = stackinfo_get_unknown(),
		},
	};

	info.stacks[STACK_TYPE_TASK] = stackinfo_get_task(tsk);

	if (tsk != current || preemptible())
		goto out;

	info.stacks[STACK_TYPE_IRQ] = stackinfo_get_irq();
	info.stacks[STACK_TYPE_OVERFLOW] = stackinfo_get_overflow();

	if (!IS_ENABLED(CONFIG_VMAP_STACK) ||
	    !IS_ENABLED(CONFIG_ARM_SDE_INTERFACE) ||
	    !in_nmi())
		goto out;

	info.stacks[STACK_TYPE_SDEI_NORMAL] = stackinfo_get_sdei_normal();
	info.stacks[STACK_TYPE_SDEI_CRITICAL] = stackinfo_get_sdei_critical();

out:
	return info;
}
#endif	/* __ASM_STACKTRACE_H */
