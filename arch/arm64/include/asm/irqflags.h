/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_IRQFLAGS_H
#define __ASM_IRQFLAGS_H

#include <asm/alternative.h>
#include <asm/barrier.h>
#include <asm/ptrace.h>
#include <asm/sysreg.h>

/*
 * Aarch64 has flags for masking: Debug, Asynchronous (serror), Interrupts and
 * FIQ exceptions, in the 'daif' register. We mask and unmask them in 'dai'
 * order:
 * Masking debug exceptions causes all other exceptions to be masked too/
 * Masking SError masks irq, but not debug exceptions. Masking irqs has no
 * side effects for other flags. Keeping to this order makes it easier for
 * entry.S to know which exceptions should be unmasked.
 *
 * FIQ is never expected, but we mask it when we disable debug exceptions, and
 * unmask it at all other times.
 */

#define daif_to_flags(daif)	(daif)
#define flags_to_daif(flags)	((u32)(flags))

#define pmr_to_flags(pmr)	((u64)(pmr) << 32)
#define flags_to_pmr(flags)	((flags) >> 32)

static inline bool __system_uses_irq_prio_masking(void)
{
	return IS_ENABLED(CONFIG_ARM64_PSEUDO_NMI) &&
	       __cpus_have_const_cap(ARM64_HAS_IRQ_PRIO_MASKING);
}

static inline void arch_local_irq_enable(void)
{
	barrier();

	if (__system_uses_irq_prio_masking()) {
		write_sysreg_s(GIC_PRIO_IRQON, SYS_ICC_PMR_EL1);
		pmr_sync();
	} else {
		__daif_imm_clear(DAIF_IMM_I);
	}

	barrier();
}

static inline void arch_local_irq_disable(void)
{
	barrier();

	if (__system_uses_irq_prio_masking())
		write_sysreg_s(GIC_PRIO_IRQOFF, SYS_ICC_PMR_EL1);
	else
		__daif_imm_set(DAIF_IMM_I);

	barrier();
}

/*
 * Save the current interrupt enable state.
 */
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags = daif_to_flags(read_sysreg(daif));

	if (__system_uses_irq_prio_masking())
		flags |= pmr_to_flags(read_sysreg_s(SYS_ICC_PMR_EL1));

	return flags;
}

static inline bool arch_irqs_disabled_flags(unsigned long flags)
{
	unsigned long daif = flags_to_daif(flags);

	/*
	 * If the PMR masks IRQs, set DAIF.I (bit 7). Bit 7 happens to be the
	 * most significant bit of ICC_PMR_EL1.Priority, and is clear when IRQs
	 * are priority masked.
	 */
	BUILD_BUG_ON(~GIC_PRIO_IRQON & PSR_I_BIT);
	BUILD_BUG_ON(!(~GIC_PRIO_IRQOFF & PSR_I_BIT));
	if (__system_uses_irq_prio_masking())
		daif |= ~flags_to_pmr(flags);

	return daif & PSR_I_BIT;
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	flags = arch_local_save_flags();

	arch_local_irq_disable();

	return flags;
}

/*
 * restore saved IRQ state
 */
static inline void arch_local_irq_restore(unsigned long flags)
{
	barrier();

	if (__system_uses_irq_prio_masking()) {
		write_sysreg_s(flags_to_pmr(flags), SYS_ICC_PMR_EL1);
		pmr_sync();
	}

	write_sysreg(flags_to_daif(flags), daif);

	barrier();
}

#endif /* __ASM_IRQFLAGS_H */
