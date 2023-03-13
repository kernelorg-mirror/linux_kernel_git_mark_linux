/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 Arm Ltd.
 */

#include <linux/linkage.h>

#include <asm/asm-uaccess.h>
#include <asm/assembler.h>

#define USER_OFF(off, x...)	USER(fixup_offset_##off, x)

#define FIXUP_OFFSET(n)							\
fixup_offset_##n:							\
	sub	x0, x3, x0;						\
	sub	x0, x0, n;						\
	ret

/*
 * Write zeroes to a user space buffer
 *
 * Parameters:
 *	x0 - to
 *	x1 - n
 * Returns:
 *	x0 - bytes not copied
 *
 * Unlike a memset(to, 0, n), we need to handle faults on user addresses, and
 * we need to precisely report the number of bytes (not) written. We must only
 * use aligned single-copy-atomic stores to write to user memory, as stores
 * which are not single-copy-atomic (e.g. unaligned stores, STP, ASMID stores)
 * can be split into separate byte accesses (per ARM DDI 0487I.a, Section
 * B2.2.1) and some arbitatrary set of those byte accesses might occur prior to
 * a fault being raised (per per ARM DDI 0487I.a, Section B2.7.1).
 *
 * We use STTR to write to user memory, which has 1/2/4/8 byte forms, and the
 * user address ('to') might have arbitrary alignment, so we must handle
 * misalignment up to 8 bytes.
 */
SYM_FUNC_START(__arch_clear_user)
		/*
		 * The end address. Fixup handlers will use this to calculate
		 * the number of bytes cleared.
		 */
		add	x3, x0, x1

		/*
		 * Tracing of a kernel build indicates that for the vast
		 * majority of calls to copy_to_user(), 'to' is aligned to 8
		 * bytes. When this is the case, we want to skip to the bulk
		 * copy as soon as possible.
		 */
		ands	x4, x0, 0x7
		b.eq	body

		/*
		 * For small unaligned copies, it's not worth trying to be
		 * optimal.
		 */
		cmp	x1, #8
		b.lo	bytewise_loop

		/*
		 * Calculate the distance to the next 8-byte boundary.
		 */
		mov	x5, #8
		sub	x4, x5, x4

SYM_INNER_LABEL(head_realign_1b, SYM_L_LOCAL)
		tbz	x4, #0, head_realign_2b

USER_OFF(0,	sttrb	wzr, [x0])
		add	x0, x0, #1

SYM_INNER_LABEL(head_realign_2b, SYM_L_LOCAL)
		tbz	x4, #1, head_realign_4b

USER_OFF(0,	sttrh	wzr, [x0])
		add	x0, x0, #2

SYM_INNER_LABEL(head_realign_4b, SYM_L_LOCAL)
		tbz	x4, #2, head_realigned

USER_OFF(0,	sttr	wzr, [x0])
		add	x0, x0, #4

SYM_INNER_LABEL(head_realigned, SYM_L_LOCAL)
		/*
		 * Any 1-7 byte misalignment has now been fixed; subtract this
		 * misalignment from the remaining size.
		 */
		sub	x1, x1, x4

SYM_INNER_LABEL(body, SYM_L_LOCAL)
		cmp	x1, #64
		b.lo	tail_32b

SYM_INNER_LABEL(body_64b_loop, SYM_L_LOCAL)
USER_OFF(0,	sttr	xzr, [x0, #0])
USER_OFF(8,	sttr	xzr, [x0, #8])
USER_OFF(16,	sttr	xzr, [x0, #16])
USER_OFF(24,	sttr	xzr, [x0, #24])
USER_OFF(32,	sttr	xzr, [x0, #32])
USER_OFF(40,	sttr	xzr, [x0, #40])
USER_OFF(48,	sttr	xzr, [x0, #48])
USER_OFF(56,	sttr	xzr, [x0, #56])
		add	x0, x0, #64
		sub	x1, x1, #64

		cmp	x1, #64
		b.hs	body_64b_loop

SYM_INNER_LABEL(tail_32b, SYM_L_LOCAL)
		tbz	x1, #5, tail_16b

USER_OFF(0,	sttr	xzr, [x0, #0])
USER_OFF(8,	sttr	xzr, [x0, #8])
USER_OFF(16,	sttr	xzr, [x0, #16])
USER_OFF(24,	sttr	xzr, [x0, #24])
		add	x0, x0, #32

SYM_INNER_LABEL(tail_16b, SYM_L_LOCAL)
		tbz	x1, #4, tail_8b

USER_OFF(0,	sttr	xzr, [x0, #0])
USER_OFF(8,	sttr	xzr, [x0, #8])
		add	x0, x0, #16

SYM_INNER_LABEL(tail_8b, SYM_L_LOCAL)
		tbz	x1, #3, tail_4b

USER_OFF(0,	sttr	xzr, [x0])
		add	x0, x0, #8

SYM_INNER_LABEL(tail_4b, SYM_L_LOCAL)
		tbz	x1, #2, tail_2b

USER_OFF(0,	sttr	wzr, [x0])
		add	x0, x0, #4

SYM_INNER_LABEL(tail_2b, SYM_L_LOCAL)
		tbz	x1, #1, tail_1b

USER_OFF(0,	sttrh	wzr, [x0])
		add	x0, x0, #2

SYM_INNER_LABEL(tail_1b, SYM_L_LOCAL)
		tbz	x1, #0, done

USER_OFF(0,	sttrb	wzr, [x0])

SYM_INNER_LABEL(done, SYM_L_LOCAL)
		mov	x0, xzr
		ret

SYM_INNER_LABEL(bytewise_loop, SYM_L_LOCAL)
		cbz	x1, done

USER_OFF(0,	sttrb	wzr, [x0])
		add	x0, x0, #1
		sub	x1, x1, #1

		b	bytewise_loop

FIXUP_OFFSET(0)
FIXUP_OFFSET(8)
FIXUP_OFFSET(16)
FIXUP_OFFSET(24)
FIXUP_OFFSET(32)
FIXUP_OFFSET(40)
FIXUP_OFFSET(48)
FIXUP_OFFSET(56)
SYM_FUNC_END(__arch_clear_user)
EXPORT_SYMBOL(__arch_clear_user)
