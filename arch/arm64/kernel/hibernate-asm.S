/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Hibernate low-level support
 *
 * Copyright (C) 2016 ARM Ltd.
 * Author:	James Morse <james.morse@arm.com>
 */
#include <linux/linkage.h>
#include <linux/errno.h>

#include <asm/asm-offsets.h>
#include <asm/assembler.h>
#include <asm/cputype.h>
#include <asm/memory.h>
#include <asm/page.h>
#include <asm/virt.h>

/*
 * Resume from hibernate
 *
 * Loads temporary page tables then restores the memory image.
 * Finally branches to cpu_resume() to restore the state saved by
 * swsusp_arch_suspend().
 *
 * Because this code has to be copied to a 'safe' page, it can't call out to
 * other functions by PC-relative address. Also remember that it may be
 * mid-way through over-writing other functions. For this reason it contains
 * code from caches_clean_inval_pou() and uses the copy_page() macro.
 *
 * This 'safe' page is mapped via ttbr0, and executed from there. This function
 * switches to a copy of the linear map in ttbr1, performs the restore, then
 * reenters the original kernel with the MMU off.
 *
 * All of memory gets written to, including code. We need to clean the kernel
 * text to the Point of Coherence (PoC) before reentering it.
 * Because the kernel modules and executable pages mapped to user space are
 * also written as data, we clean all pages we touch to the Point of
 * Unification (PoU).
 *
 * x0: physical address of temporary zero page
 * x1: physical address of temporary page tables
 * x2: linear map address of restore_pblist in the current kernel
 * x3: linear map address of the start of the resumed kernel image
 * x4: linear map address of the end of the resumed kernel image
 * x5: physical address to reenter the resumed kernel
 * x6: whether to return to el2
 */
.pushsection    ".hibernate_exit.text", "ax"
SYM_CODE_START(swsusp_arch_suspend_exit)
	/*
	 * Save the arguments so that we can clobber low numbered registers
	 */
	mov	x22, x2
	mov	x23, x3
	mov	x24, x4
	mov	x25, x5
	mov	x26, x6

	/*
	 * This function executes from an idmap in TTBR0.
	 * Install the temporary linear map in TTBR1, replacing the kernel page
	 * tables.
	 */
	break_before_make_ttbr_switch	x0, x1, x3, x4

	/*
	 * Walk the restore_pblist and restore each page for the kernel session
	 * being resumed. All pages are cleaned to the PoU in case they contain
	 * userspace or module code.
	 *
	 * The main kernel image is cleaned to the PoC to as it contains
	 * portions of code and data that need to be used with the MMU disabled.
	 */
.L__next_pbe:
	ldr	x0,  [x22, #HIBERN_PBE_ORIG]
	ldr	x1,  [x22, #HIBERN_PBE_ADDR]

	mov	x10, x0

	copy_page	x0, x1, x2, x3, x4, x5, x6, x7, x8, x9

	/* Clean the copied page to PoU */
	mov	x0, x10
	add	x1, x10, #PAGE_SIZE
	raw_dcache_line_size x2, x3
	dcache_by_myline_op cvau, ish, x0, x1, x2, x3

	ldr	x22, [x22, #HIBERN_PBE_NEXT]
	cbnz	x22, .L__next_pbe

	/* Clean the main kernel image to the PoC */
	dcache_by_myline_op cvac, ish, x23, x24, x2, x3

	/* Complete all D-cache maintenance above */
	dsb	ish

	ic	ialluis
	dsb	ish
	isb

	cbz	x26, .L__no_el2

	/* Enter the resumed kernel at EL2 with the MMU disabled. */
	mov	x0, #HVC_SOFT_RESTART
	mov	x1, x25
	hvc	#0

	/* HVC_SOFT_RESTART should never return */
	b	.

.L__no_el2:
	/* Enter the resumed kernel at EL1 with the MMU disabled. */
	mov_q	x0, INIT_SCTLR_EL1_MMU_OFF
	pre_disable_mmu_workaround
	msr	sctlr_el1, x0
	isb
	br	x25
SYM_CODE_END(swsusp_arch_suspend_exit)
.popsection
