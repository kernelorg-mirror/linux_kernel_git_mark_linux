/*
 * Low-level exception handling code
 *
 * Copyright (C) 2012 ARM Ltd.
 * Authors:	Catalin Marinas <catalin.marinas@arm.com>
 *		Will Deacon <will.deacon@arm.com>
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

#include <linux/init.h>
#include <linux/linkage.h>

#include <asm/alternative.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/cpufeature.h>
#include <asm/errno.h>
#include <asm/esr.h>
#include <asm/irq.h>
#include <asm/memory.h>
#include <asm/thread_info.h>
#include <asm/unistd.h>


/*
 * Bad Abort numbers
 *-----------------
 */
#define BAD_SYNC	0
#define BAD_IRQ		1
#define BAD_FIQ		2
#define BAD_ERROR	3

	.macro	kernel_entry, el, regsize = 64
	sub	sp, sp, #S_FRAME_SIZE
	.if	\regsize == 32
	mov	w0, w0				// zero upper 32 bits of x0
	.endif
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	.if	\el == 0
	mrs	x21, sp_el0
	mov	tsk, sp
	and	tsk, tsk, #~(THREAD_SIZE - 1)

	mov	x29, xzr			// fp pointed to user-space
	.else
	add	x21, sp, #S_FRAME_SIZE
	get_thread_info tsk
	/* Save the task's original addr_limit and set USER_DS (TASK_SIZE_64) */
	ldr	x20, [tsk, #TI_ADDR_LIMIT]
	str	x20, [sp, #S_ORIG_ADDR_LIMIT]
	mov	x20, #TASK_SIZE_64
	str	x20, [tsk, #TI_ADDR_LIMIT]
	ALTERNATIVE(nop, SET_PSTATE_UAO(0), ARM64_HAS_UAO, CONFIG_ARM64_UAO)
	.endif /* \el == 0 */
	mrs	x22, elr_el1
	mrs	x23, spsr_el1
	stp	lr, x21, [sp, #S_LR]
	stp	x22, x23, [sp, #S_PC]

	/*
	 * Set syscallno to -1 by default (overridden later if real syscall).
	 */
	.if	\el == 0
	mvn	x21, xzr
	str	x21, [sp, #S_SYSCALLNO]
	.endif

	/*
	 * Set sp_el0 to current thread_info.
	 */
	.if	\el == 0
	msr	sp_el0, tsk
	.endif

	/*
	 * Registers that may be useful after this macro is invoked:
	 *
	 * x21 - aborted SP
	 * x22 - aborted PC
	 * x23 - aborted PSTATE
	*/
	.endm

	.macro	kernel_exit, el
	.if	\el != 0
	/* Restore the task's original addr_limit. */
	ldr	x20, [sp, #S_ORIG_ADDR_LIMIT]
	str	x20, [tsk, #TI_ADDR_LIMIT]

	/* No need to restore UAO, it will be restored from SPSR_EL1 */
	.endif

	ldp	x21, x22, [sp, #S_PC]		// load ELR, SPSR
	.if	\el == 0
	ldr	x23, [sp, #S_SP]		// load return stack pointer
	msr	sp_el0, x23
	.endif
	msr	elr_el1, x21			// set up the return data
	msr	spsr_el1, x22
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	ldr	lr, [sp, #S_LR]
	add	sp, sp, #S_FRAME_SIZE		// restore sp
	eret					// return to kernel
	.endm

	.macro	get_thread_info, rd
	mrs	\rd, sp_el0
	.endm

/*
 * These are the registers used in the syscall handler, and allow us to
 * have in theory up to 7 arguments to a function - x0 to x6.
 *
 * x7 is reserved for the system call number in 32-bit mode.
 */
tsk	.req	x28		// current thread_info

	.text

/*
 * Exception vectors.
 */

	.align	11
ENTRY(vectors)
	ventry	el1_sync_invalid		// Synchronous EL1t
	ventry	el1_irq_invalid			// IRQ EL1t
	ventry	el1_fiq_invalid			// FIQ EL1t
	ventry	el1_error_invalid		// Error EL1t

	ventry	el1_sync			// Synchronous EL1h
	ventry	el1_irq				// IRQ EL1h
	ventry	el1_fiq_invalid			// FIQ EL1h
	ventry	el1_error_invalid		// Error EL1h

	ventry	el0_sync			// Synchronous 64-bit EL0
	ventry	el0_irq				// IRQ 64-bit EL0
	ventry	el0_fiq_invalid			// FIQ 64-bit EL0
	ventry	el0_error_invalid		// Error 64-bit EL0

#ifdef CONFIG_COMPAT
	ventry	el0_sync_compat			// Synchronous 32-bit EL0
	ventry	el0_irq_compat			// IRQ 32-bit EL0
	ventry	el0_fiq_invalid_compat		// FIQ 32-bit EL0
	ventry	el0_error_invalid_compat	// Error 32-bit EL0
#else
	ventry	el0_sync_invalid		// Synchronous 32-bit EL0
	ventry	el0_irq_invalid			// IRQ 32-bit EL0
	ventry	el0_fiq_invalid			// FIQ 32-bit EL0
	ventry	el0_error_invalid		// Error 32-bit EL0
#endif
END(vectors)

/*
 * Invalid mode handlers
 */
	.macro	inv_entry, el, reason, regsize = 64
	kernel_entry \el, \regsize
	mov	x0, sp
	mov	x1, #\reason
	mrs	x2, esr_el1
	b	bad_mode
	.endm

el0_sync_invalid:
	inv_entry 0, BAD_SYNC
ENDPROC(el0_sync_invalid)

el0_irq_invalid:
	inv_entry 0, BAD_IRQ
ENDPROC(el0_irq_invalid)

el0_fiq_invalid:
	inv_entry 0, BAD_FIQ
ENDPROC(el0_fiq_invalid)

el0_error_invalid:
	inv_entry 0, BAD_ERROR
ENDPROC(el0_error_invalid)

#ifdef CONFIG_COMPAT
el0_fiq_invalid_compat:
	inv_entry 0, BAD_FIQ, 32
ENDPROC(el0_fiq_invalid_compat)

el0_error_invalid_compat:
	inv_entry 0, BAD_ERROR, 32
ENDPROC(el0_error_invalid_compat)
#endif

el1_sync_invalid:
	inv_entry 1, BAD_SYNC
ENDPROC(el1_sync_invalid)

el1_irq_invalid:
	inv_entry 1, BAD_IRQ
ENDPROC(el1_irq_invalid)

el1_fiq_invalid:
	inv_entry 1, BAD_FIQ
ENDPROC(el1_fiq_invalid)

el1_error_invalid:
	inv_entry 1, BAD_ERROR
ENDPROC(el1_error_invalid)

/*
 * EL1 mode handlers.
 */
	.align	6
el1_sync:
	kernel_entry 1
	mov	x0, sp
	bl	__el1_sync
	kernel_exit 1
ENDPROC(el1_sync)

	.align	6
el1_irq:
	kernel_entry 1
	mov	x0, sp
	bl	__el1_irq
	kernel_exit 1
ENDPROC(el1_irq)

/*
 * EL0 mode handlers.
 */
	.align	6
el0_sync:
	kernel_entry 0
	mov	x0, sp
	bl	__el0_sync
	kernel_exit 0
ENDPROC(el0_sync)

	.align	6
el0_irq:
	kernel_entry 0
	mov	x0, sp
	bl	__el0_irq_naked
	kernel_exit 0
ENDPROC(el0_irq)

#ifdef CONFIG_COMPAT
	.align	6
el0_sync_compat:
	kernel_entry 0, 32
	mov	x0, sp
	bl	__el0_sync
	kernel_exit 0
ENDPROC(el0_sync_compat)

	.align	6
el0_irq_compat:
	kernel_entry 0, 32
	mov	x0, sp
	bl	__el0_irq_naked
	kernel_exit 0
ENDPROC(el0_irq_compat)
#endif

/*
 * Register switch for AArch64. The callee-saved registers need to be saved
 * and restored. On entry:
 *   x0 = previous task_struct (must be preserved across the switch)
 *   x1 = next task_struct
 * Previous and next are guaranteed not to be the same.
 *
 */
ENTRY(cpu_switch_to)
	mov	x10, #THREAD_CPU_CONTEXT
	add	x8, x0, x10
	mov	x9, sp
	stp	x19, x20, [x8], #16		// store callee-saved registers
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	lr, [x8]
	add	x8, x1, x10
	ldp	x19, x20, [x8], #16		// restore callee-saved registers
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	lr, [x8]
	mov	sp, x9
	and	x9, x9, #~(THREAD_SIZE - 1)
	msr	sp_el0, x9
	ret
ENDPROC(cpu_switch_to)

/*
 * This is how we return from a fork.
 */
ENTRY(ret_from_fork)
	bl	schedule_tail
	cbz	x19, 1f				// not a kernel thread
	mov	x0, x20
	blr	x19
1:	mov	x0, sp
	bl	el0_ret_from_fork
	kernel_exit 0
ENDPROC(ret_from_fork)

/*
 * x0 - arg
 * x1 - func
 * x2 - stack
 */
ENTRY(call_on_stack)
	/*
	 * We need to save our LR somewhere, so we may as well make a frame
	 * record.
	 */
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp

	mov	sp, x2

	/*
	 * For the first frame on the IRQ stack, we use a special format
	 * (handled by unwind_frame), where we store our previous SP rather the
	 * than LR. Since we just made a frame, our FP is the same as our
	 * original SP.
	 */
	stp	x29, x29, [sp, #-16]!
	mov	x29, sp

	blr	x1

	ldp	x29, xzr, [sp], #16
	mov	sp, x29

	ldp	x29, x30, [sp], #16

	ret
ENDPROC(call_on_stack)
