/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2017 ARM Ltd.
 */
#ifndef __ASM_DAIFFLAGS_H
#define __ASM_DAIFFLAGS_H

#include <linux/irqflags.h>

#include <asm/arch_gicv3.h>
#include <asm/barrier.h>
#include <asm/cpufeature.h>
#include <asm/ptrace.h>
#include <asm/sysreg.h>

#define DAIF_MASK		(PSR_D_BIT | PSR_A_BIT | PSR_I_BIT | PSR_F_BIT)


/* mask/save/unmask/restore all exceptions, including interrupts. */
static inline void local_daif_mask(void)
{
	__daif_imm_set(DAIF_IMM_DAIF);
	trace_hardirqs_off();
}

static inline unsigned long local_daif_save_flags(void)
{
	return arch_local_save_flags();
}

static inline unsigned long local_daif_save(void)
{
	unsigned long flags;

	flags = local_daif_save_flags();
	local_daif_mask();

	return flags;
}

static inline void local_daif_restore(unsigned long flags)
{
	if (!arch_irqs_disabled_flags(flags))
		trace_hardirqs_on();

	arch_local_irq_restore(flags);
}

/*
 * Called by synchronous exception handlers to restore the DAIF bits that were
 * modified by taking an exception.
 *
 * This does not need to update the PMR, so we don't use local_daif_restore().
 */
static inline void local_daif_inherit(struct pt_regs *regs)
{
	unsigned long flags = regs->pstate & DAIF_MASK;

	write_sysreg(flags, daif);
}

/*
 * Enter a process context with all exceptions unmasked, starting from a
 * context with all exceptions masked in DAIF (and IRQ unmasked in PMR).
 *
 * Unmasks: Debug, SError, IRQ, FIQ, NMI
 */
static inline void local_daif_unmask_procctx(void)
{
	trace_hardirqs_on();
	__daif_imm_clear(DAIF_IMM_DAIF);
}

/*
 * Enter a process context for the first time, starting from a context with all
 * exceptions masked in DAIF (and PMR in an UNKNOWN state).
 *
 * Unmasks: Debug, SError, IRQ, FIQ, NMI
 */
static inline void local_daif_init_procctx(void)
{
	if (system_uses_irq_prio_masking()) {
		gic_write_pmr(GIC_PRIO_IRQON);
		pmr_sync();
	}

	local_daif_unmask_procctx();
}

/*
 * Enter a process context with only (regular) IRQ masked for the first time,
 * starting from a context with all exceptions masked in DAIF (and PMR in an
 * UNKNOWN state).
 *
 * Unmasks: Debug, SError, FIQ, NMI
 */
static inline void local_daif_init_procctx_noirq(void)
{
	if (system_uses_irq_prio_masking()) {
		gic_write_pmr(GIC_PRIO_IRQOFF);
		__daif_imm_clear(DAIF_IMM_DAIF);
	} else {
		__daif_imm_clear(DAIF_IMM_DA_F);
	}

	trace_hardirqs_off();
}

/*
 * Enter a process context with only (regular) IRQ masked, starting from a
 * context with all exceptions masked in DAIF (and IRQ unmasked in PMR).
 *
 * Unmasks: Debug, SError, FIQ, NMI
 */
static inline void local_daif_unmask_procctx_noirq(void)
{
	local_daif_init_procctx_noirq();
}

/*
 * Enter an error context with SError masked, from an arbitrary context where
 * higher priority exceptions may or may not be masked.
 *
 * Masks: SError, IRQ, NMI
 * Unchanged: Debug, FIQ
 */
static inline void local_daif_mask_errctx(void)
{
	__daif_imm_set(DAIF_IMM_A | DAIF_IMM_I);
	trace_hardirqs_off();
}
#endif
