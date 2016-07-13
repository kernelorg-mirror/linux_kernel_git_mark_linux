/*
 * Copyright (C) 2013 ARM Ltd.
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
#ifndef __ASM_PERCPU_H
#define __ASM_PERCPU_H

static inline void set_my_cpu_offset(unsigned long off)
{
	asm volatile("msr tpidr_el1, %0; mov x28, %0" :: "r" (off) : "memory");
}

static inline unsigned long __my_cpu_offset(void)
{
	unsigned long off;

	/*
	 * We want to allow caching the value, so avoid using volatile and
	 * instead use a fake stack read to hazard against barrier().
	 */
	asm("mov %0, x28" : "=r" (off) :
		"Q" (*(const unsigned long *)current_stack_pointer));

	return off;
}
#define __my_cpu_offset __my_cpu_offset()

#define __PCP_W_1	"w"
#define __PCP_W_2	"w"
#define __PCP_W_4	"w"
#define __PCP_W_8	""
#define __PCP_W(n)	__PCP_W_ ## n

#define __PCP_S_1	"b"
#define __PCP_S_2	"h"
#define __PCP_S_4	""
#define __PCP_S_8	""
#define __PCP_S(n)	__PCP_S_ ## n

#define ____PCP_READ(n, __pcp)						\
({									\
	typeof(__pcp) __ret;						\
	asm("ldr" __PCP_S(n) " %" __PCP_W(n) "[ret], [%[ptr], x28]"	\
	    : [ret] "=r" (__ret)					\
	    : [ptr] "r" (&__pcp), [force_hazard] "Q" (__pcp));		\
	__ret;								\
})

#define this_cpu_read_1(pcp)		____PCP_READ(1, pcp)
#define this_cpu_read_2(pcp)		____PCP_READ(2, pcp)
#define this_cpu_read_4(pcp)		____PCP_READ(4, pcp)
#define this_cpu_read_8(pcp)		____PCP_READ(8, pcp)

#define ____PCP_WRITE(n, __pcp, __val)					\
({									\
	asm("str" __PCP_S(n) " %" __PCP_W(n) "[val], [%[pcp], x28]"	\
	    : [force_hazard] "=Q" (__pcp)				\
	    : [val] "r" (__val), [pcp] "r" (&__pcp));			\
})

#define this_cpu_write_1(pcp, val)	____PCP_WRITE(1, pcp, val)
#define this_cpu_write_2(pcp, val)	____PCP_WRITE(2, pcp, val)
#define this_cpu_write_4(pcp, val)	____PCP_WRITE(4, pcp, val)
#define this_cpu_write_8(pcp, val)	____PCP_WRITE(8, pcp, val)

#define __PCP_OP(n, __pcp, __i, __op, __cl)					\
({										\
	unsigned long __ret, __tmp, __off, __ptr;				\
										\
	asm volatile(								\
	"1:	mov	%[off], x28\n"						\
	"	add	%[ptr], %[pcp], %[off]\n"				\
	"	ldxr" __PCP_S(n) "\t%" __PCP_W(n) "[ret], [%[ptr]]\n"		\
	"	eor	%[tmp], %[off], x28\n"					\
	"	cbnz	%[tmp], 1b\n"						\
	"	" #__op " %[ret], %[ret], %[i]\n"				\
	"	stxr" __PCP_S(n) "\t%w[tmp], %" __PCP_W(n) "[ret], [%[ptr]]\n"	\
	"	cbnz	%[tmp], 1b\n"						\
	: [ret] "=&r" (__ret), [off] "=&r" (__off), [tmp] "=&r" (__tmp),	\
	  [ptr] "=&r" (__ptr), [force_hazard] "+Q" (__pcp)			\
	: [pcp] "r" (&__pcp), [i] "r" (__i)					\
	: __cl									\
	);									\
										\
	(typeof(__pcp))__ret;							\
})

#define this_cpu_add_1(pcp, val)		__PCP_OP(1, pcp, val, add, )
#define this_cpu_add_2(pcp, val)		__PCP_OP(2, pcp, val, add, )
#define this_cpu_add_4(pcp, val)		__PCP_OP(4, pcp, val, add, )
#define this_cpu_add_8(pcp, val)		__PCP_OP(8, pcp, val, add, )

#define this_cpu_add_return_1(pcp, val)		__PCP_OP(1, pcp, val, add, "memory")
#define this_cpu_add_return_2(pcp, val)		__PCP_OP(2, pcp, val, add, "memory")
#define this_cpu_add_return_4(pcp, val)		__PCP_OP(4, pcp, val, add, "memory")
#define this_cpu_add_return_8(pcp, val)		__PCP_OP(8, pcp, val, add, "memory")

#define this_cpu_and_1(pcp, val)		__PCP_OP(1, pcp, val, and, )
#define this_cpu_and_2(pcp, val)		__PCP_OP(2, pcp, val, and, )
#define this_cpu_and_4(pcp, val)		__PCP_OP(4, pcp, val, and, )
#define this_cpu_and_8(pcp, val)		__PCP_OP(8, pcp, val, and, )

#define this_cpu_or_1(pcp, val)			__PCP_OP(1, pcp, val, orr, )
#define this_cpu_or_2(pcp, val)			__PCP_OP(2, pcp, val, orr, )
#define this_cpu_or_4(pcp, val)			__PCP_OP(4, pcp, val, orr, )
#define this_cpu_or_8(pcp, val)			__PCP_OP(8, pcp, val, orr, )

#define __PCP_XCHG(n, __pcp, __new)						\
({										\
	unsigned long __ret, __tmp, __off, __ptr;				\
										\
	asm volatile(								\
	"1:	mov	%[off], x28\n"						\
	"	add	%[ptr], %[pcp], %[off]\n"				\
	"	ldxr" __PCP_S(n) "\t%" __PCP_W(n) "[ret], [%[ptr]]\n"		\
	"	eor	%[tmp], %[off], x28\n"					\
	"	cbnz	%[tmp], 1b\n"						\
	"	stxr" __PCP_S(n) "\t%w[tmp], %" __PCP_W(n) "[new], [%[ptr]]\n"	\
	"	cbnz	%[tmp], 1b\n"						\
	: [ret] "=&r" (__ret), [off] "=&r" (__off), [tmp] "=&r" (__tmp),	\
	  [ptr] "=&r" (__ptr), [force_hazard] "+Q" (__pcp)			\
	: [pcp] "r" (&__pcp), [new] "r" (__new)					\
	: "memory"								\
	);									\
										\
	(typeof(__pcp))__ret;							\
})

#define this_cpu_xchg_1(pcp, val)		__PCP_XCHG(1, pcp, val)
#define this_cpu_xchg_2(pcp, val)		__PCP_XCHG(2, pcp, val)
#define this_cpu_xchg_4(pcp, val)		__PCP_XCHG(4, pcp, val)
#define this_cpu_xchg_8(pcp, val)		__PCP_XCHG(8, pcp, val)

#define __PCP_CMPXCHG(n, __pcp, __old, __new)					\
({										\
	unsigned long __ret, __tmp, __off, __ptr;				\
										\
	asm volatile(								\
	"1:	mov	%[off], x28\n"						\
	"	add	%[ptr], %[pcp], %[off]\n"				\
	"	ldxr" __PCP_S(n) "\t%" __PCP_W(n) "[ret], [%[ptr]]\n"		\
	"	eor	%[tmp], %[off], x28\n"					\
	"	cbnz	%[tmp], 1b\n"						\
	"	eor	%[tmp], %[ret], %[old]\n"				\
	"	cbnz	%[tmp], 2f\n"						\
	"	stxr" __PCP_S(n) "\t%w[tmp], %" __PCP_W(n) "[new], [%[ptr]]\n"	\
	"	cbnz	%[tmp], 1b\n"						\
	"2:"									\
	: [ret] "=&r" (__ret), [off] "=&r" (__off), [tmp] "=&r" (__tmp),	\
	  [ptr] "=&r" (__ptr), [force_hazard] "+Q" (__pcp)			\
	: [pcp] "r" (&__pcp), [old] "r" (__old), [new] "r" (__new)		\
	: "memory"								\
	);									\
										\
	(typeof(__pcp))__ret;							\
})

#define this_cpu_cmpxhg_1(pcp, o, n)	__PCP_CMPXCHG(1, pcp, o, n)
#define this_cpu_cmpxhg_2(pcp, o, n)	__PCP_CMPXCHG(2, pcp, o, n)
#define this_cpu_cmpxhg_4(pcp, o, n)	__PCP_CMPXCHG(4, pcp, o, n)
#define this_cpu_cmpxhg_8(pcp, o, n)	__PCP_CMPXCHG(8, pcp, o, n)

#define this_cpu_cmpxchg_double_8(ptr1, ptr2, o1, o2, n1, n2)		\
({									\
	unsigned long __ret, __tmp, __off, __ptr;			\
									\
	asm volatile(							\
	"1:	mov 	%[off], x28\n"					\
	"	add	%[ptr], %[pcp], %[off]\n"			\
	"	ldxp	%[ret], %[tmp], [%[ptr]]\n"			\
	"	cmp	%[off], x28\n"					\
	"	b.ne	1b\n"						\
	"	eor	%[ret], %[ret], %[old1]\n"			\
	"	eor	%[tmp], %[tmp], %[old2]\n"			\
	"	orr	%[ret], %[ret], %[tmp]\n"			\
	"	cbnz	%[ret], 2f\n"					\
	"	stxp	%w[ret], %[new1], %[new2], [%[ptr]]\n"		\
	"	cbnz	%[ret], 1b\n"					\
	"2:"								\
	: [ret] "=&r" (__ret), [tmp] "=&r" (__tmp),			\
	  [off] "=&r" (__off), [ptr] "=&r" (__ptr),			\
	  [hazard1] "+Q" (ptr1), [hazard2] "+Q" (ptr2)			\
	: [pcp] "r" (&ptr1), [old1] "r" (o1), [old2] "r" (o2),		\
	  [new1] "r" (n1), [new2] "r" (n2)				\
	: "memory", "cc"						\
	);								\
									\
	!__ret;								\
})

#include <asm-generic/percpu.h>

#endif /* __ASM_PERCPU_H */
