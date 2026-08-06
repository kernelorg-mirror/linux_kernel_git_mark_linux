/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_THUNK_H
#define __ASM_THUNK_H

#define __THUNK_CALL(name)						\
do {									\
	asm volatile(							\
	"	bl	" #name "\n"					\
	:								\
	: "r" (__builtin_frame_address(0))				\
	: "memory", "x16", "x17", "x30");				\
} while(0)

static __always_inline void thunk_preempt_schedule(void)
{
	__THUNK_CALL(__thunk_preempt_schedule);
}

static __always_inline void thunk_preempt_schedule_notrace(void)
{
	__THUNK_CALL(__thunk_preempt_schedule_notrace);
}

#endif /* __ASM_THUNK_H */
