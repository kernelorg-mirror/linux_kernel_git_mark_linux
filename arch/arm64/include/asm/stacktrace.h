/*
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_STACKTRACE_H
#define __ASM_STACKTRACE_H

#include <linux/percpu.h>
#include <linux/sched.h>

#include <asm/memory.h>
#include <asm/ptrace.h>

struct stackframe {
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	unsigned int graph;
#endif
};

extern int unwind_frame(struct task_struct *tsk, struct stackframe *frame);
extern void walk_stackframe(struct task_struct *tsk, struct stackframe *frame,
			    int (*fn)(struct stackframe *, void *), void *data);
extern void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk);

DECLARE_PER_CPU(unsigned long [IRQ_STACK_SIZE/sizeof(long)], irq_stack);

/*
 * The highest address on the stack, and the first to be used. Used to
 * find the dummy-stack frame put down by el?_irq() in entry.S, which
 * is structured as follows:
 *
 *       ------------
 *       |          |  <- irq_stack_ptr
 *   top ------------
 *       |   x19    | <- irq_stack_ptr - 0x08
 *       ------------
 *       |   x29    | <- irq_stack_ptr - 0x10
 *       ------------
 *
 * where x19 holds a copy of the task stack pointer where the struct pt_regs
 * from kernel_entry can be found.
 *
 */
#define IRQ_STACK_PTR() ((unsigned long)raw_cpu_ptr(irq_stack) + IRQ_STACK_SIZE)

/*
 * The offset from irq_stack_ptr where entry.S will store the original
 * stack pointer. Used by unwind_frame() and dump_backtrace().
 */
#define IRQ_STACK_TO_TASK_STACK(ptr) (*((unsigned long *)((ptr) - 0x08)))

static inline bool on_irq_stack(unsigned long sp)
{
	/* variable names the same as kernel/stacktrace.c */
	unsigned long low = (unsigned long)raw_cpu_ptr(irq_stack);
	unsigned long high = low + IRQ_STACK_SIZE;

	return (low <= sp && sp < high);
}

#endif	/* __ASM_STACKTRACE_H */
