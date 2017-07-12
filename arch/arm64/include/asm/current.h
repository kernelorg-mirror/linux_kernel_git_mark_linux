#ifndef __ASM_CURRENT_H
#define __ASM_CURRENT_H

#include <linux/compiler.h>

#ifndef __ASSEMBLY__

struct task_struct;

/*
 * We don't use read_sysreg() as we want the compiler to cache the value where
 * possible.
 */
static __always_inline struct task_struct *get_current(void)
{
	unsigned long cur;

	asm ("mrs %0, tpidr_el1" : "=r" (cur));

	return (struct task_struct *)cur;
}

#define current get_current()

#endif /* __ASSEMBLY__ */

#endif /* __ASM_CURRENT_H */

