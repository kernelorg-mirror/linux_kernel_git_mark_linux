/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * TODO: explain what this is for.
 */
#include <linux/export.h>
#include <linux/linkage.h>

/*
 * Thunk will preserve:
 * - X0 to X15 (explicit below)
 * - X18 (explicit below, only needed for !SCS builds)
 * - X19 to X28 (callee-saved)
 * - X29 (explicit below)
 * - SP (implicit)
 *
 * Caller must preserve:
 * - X16 to X17 (may be clobbered by veneer on call to thunk)
 * - X30 (clobbered by call to thunk)
 *
 * Caller must ensure:
 * - SP is aligned to 16 bytes (TODO: how?)
 * - FP is initalized to caller's frame record.
 *
 * The thunk must create a frame record.
 *
 * TODO: How to handle wait_chan unwinds? Thunk must be skipped.
 *
 * TODO: is it necessary to disable kprobes?
 */
#define THUNK_X(n)		(16 + 8 * (n))
#define THUNK_X18		(THUNK_X(16))
#define THUNK_FRAME_SIZE	(THUNK_X18 + 16)

	.macro THUNK callee:req
SYM_FUNC_START(__thunk_\callee)
	stp	x29, x30, [sp, #-THUNK_FRAME_SIZE]
	mov	x29, sp

	stp	x0,  x1,  [sp, THUNK_X(0)]
	stp	x2,  x3,  [sp, THUNK_X(2)]
	stp	x4,  x5,  [sp, THUNK_X(4)]
	stp	x6,  x7,  [sp, THUNK_X(6)]
	stp	x8,  x9,  [sp, THUNK_X(8)]
	stp	x10, x11, [sp, THUNK_X(10)]
	stp	x12, x13, [sp, THUNK_X(12)]
	stp	x14, x15, [sp, THUNK_X(14)]
	str	x18,      [sp, THUNK_X18]

	bl	\callee

	ldp	x0,  x1,  [sp, THUNK_X(0)]
	ldp	x2,  x3,  [sp, THUNK_X(2)]
	ldp	x4,  x5,  [sp, THUNK_X(4)]
	ldp	x6,  x7,  [sp, THUNK_X(6)]
	ldp	x8,  x9,  [sp, THUNK_X(8)]
	ldp	x10, x11, [sp, THUNK_X(10)]
	ldp	x12, x13, [sp, THUNK_X(12)]
	ldp	x14, x15, [sp, THUNK_X(14)]
	ldr	x18,      [sp, THUNK_X18]

	ldp	x29, x30, [sp], #THUNK_FRAME_SIZE
	ret
SYM_FUNC_END(__thunk_\callee)
EXPORT_SYMBOL(__thunk_\callee)
	.endm

THUNK preempt_schedule
THUNK preempt_schedule_notrace
