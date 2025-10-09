/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_CPUTYPE_H
#define __ASM_CPUTYPE_H

#define INVALID_HWID		ULONG_MAX

#define MPIDR_UP_BITMASK	(0x1 << 30)
#define MPIDR_MT_BITMASK	(0x1 << 24)
#define MPIDR_HWID_BITMASK	UL(0xff00ffffff)

#define MPIDR_LEVEL_BITS_SHIFT	3
#define MPIDR_LEVEL_BITS	(1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK	((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
	(((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
	((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define MIDR_REVISION_MASK	0xf
#define MIDR_REVISION(midr)	((midr) & MIDR_REVISION_MASK)
#define MIDR_PARTNUM_SHIFT	4
#define MIDR_PARTNUM_MASK	(0xfff << MIDR_PARTNUM_SHIFT)
#define MIDR_PARTNUM(midr)	\
	(((midr) & MIDR_PARTNUM_MASK) >> MIDR_PARTNUM_SHIFT)
#define MIDR_ARCHITECTURE_SHIFT	16
#define MIDR_ARCHITECTURE_MASK	(0xf << MIDR_ARCHITECTURE_SHIFT)
#define MIDR_ARCHITECTURE(midr)	\
	(((midr) & MIDR_ARCHITECTURE_MASK) >> MIDR_ARCHITECTURE_SHIFT)
#define MIDR_VARIANT_SHIFT	20
#define MIDR_VARIANT_MASK	(0xf << MIDR_VARIANT_SHIFT)
#define MIDR_VARIANT(midr)	\
	(((midr) & MIDR_VARIANT_MASK) >> MIDR_VARIANT_SHIFT)
#define MIDR_IMPLEMENTOR_SHIFT	24
#define MIDR_IMPLEMENTOR_MASK	(0xffU << MIDR_IMPLEMENTOR_SHIFT)
#define MIDR_IMPLEMENTOR(midr)	\
	(((midr) & MIDR_IMPLEMENTOR_MASK) >> MIDR_IMPLEMENTOR_SHIFT)

#define MIDR_CPU_MODEL(imp, partnum) \
	((_AT(u32, imp)		<< MIDR_IMPLEMENTOR_SHIFT) | \
	(0xf			<< MIDR_ARCHITECTURE_SHIFT) | \
	((partnum)		<< MIDR_PARTNUM_SHIFT))

#define MIDR_MODEL(imp, partnum) \
	MIDR_CPU_MODEL(ARM_CPU_IMP_##imp, imp##_CPU_PART_##partnum)

#define MIDR_CPU_VAR_REV(var, rev) \
	(((var)	<< MIDR_VARIANT_SHIFT) | (rev))

#define MIDR_CPU_MODEL_MASK (MIDR_IMPLEMENTOR_MASK | MIDR_PARTNUM_MASK | \
			     MIDR_ARCHITECTURE_MASK)

#include <asm/midr-defs.h>

/* Fujitsu Erratum 010001 affects A64FX 1.0 and 1.1, (v0r0 and v1r0) */
#define MIDR_FUJITSU_ERRATUM_010001		MIDR_MODEL(FUJITSU, A64FX)
#define MIDR_FUJITSU_ERRATUM_010001_MASK	(~MIDR_CPU_VAR_REV(1, 0))
#define TCR_CLEAR_FUJITSU_ERRATUM_010001	(TCR_NFD1 | TCR_NFD0)

#ifndef __ASSEMBLY__

#include <asm/sysreg.h>

#define read_cpuid(reg)			read_sysreg_s(SYS_ ## reg)

/*
 * The CPU ID never changes at run time, so we might as well tell the
 * compiler that it's constant.  Use this function to read the CPU ID
 * rather than directly reading processor_id or read_cpuid() directly.
 */
static inline u32 __attribute_const__ read_cpuid_id(void)
{
	return read_cpuid(MIDR_EL1);
}

/*
 * Represent a range of MIDR values for a given CPU model and a
 * range of variant/revision values.
 *
 * @model	- CPU model as defined by MIDR_CPU_MODEL
 * @rv_min	- Minimum value for the revision/variant as defined by
 *		  MIDR_CPU_VAR_REV
 * @rv_max	- Maximum value for the variant/revision for the range.
 */
struct midr_range {
	u32 model;
	u32 rv_min;
	u32 rv_max;
};

#define MIDR_RANGE(imp, partnum, v_min, r_min, v_max, r_max)	\
	{							\
		.model = MIDR_MODEL(imp, partnum),		\
		.rv_min = MIDR_CPU_VAR_REV(v_min, r_min),	\
		.rv_max = MIDR_CPU_VAR_REV(v_max, r_max),	\
	}

#define MIDR_RANGE_SINGLE(imp, partnum, v, r)			\
	MIDR_RANGE(imp, partnum, v, r, v, r)

#define MIDR_RANGE_ALL(imp, partnum) 				\
	MIDR_RANGE(imp, partnum, 0, 0, 0xf, 0xf)

static inline bool midr_is_cpu_model_range(u32 midr, u32 model, u32 rv_min,
					   u32 rv_max)
{
	u32 _model = midr & MIDR_CPU_MODEL_MASK;
	u32 rv = midr & (MIDR_REVISION_MASK | MIDR_VARIANT_MASK);

	return _model == model && rv >= rv_min && rv <= rv_max;
}

struct target_impl_cpu {
	u64 midr;
	u64 revidr;
	u64 aidr;
};

bool cpu_errata_set_target_impl(u64 num, void *impl_cpus);
bool is_midr_in_range_list(struct midr_range const *ranges);

static inline u64 __attribute_const__ read_cpuid_mpidr(void)
{
	return read_cpuid(MPIDR_EL1);
}

static inline unsigned int __attribute_const__ read_cpuid_implementor(void)
{
	return MIDR_IMPLEMENTOR(read_cpuid_id());
}

static inline unsigned int __attribute_const__ read_cpuid_part_number(void)
{
	return MIDR_PARTNUM(read_cpuid_id());
}

static inline u32 __attribute_const__ read_cpuid_cachetype(void)
{
	return read_cpuid(CTR_EL0);
}
#endif /* __ASSEMBLY__ */

#endif
