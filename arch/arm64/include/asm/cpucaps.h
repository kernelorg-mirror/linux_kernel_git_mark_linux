/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Low-level cpucap definitions with minimal header dependencies
 */
#ifndef __ASM_CPUCAPS_H
#define __ASM_CPUCAPS_H

#include <asm/cpucap-defs.h>

#ifndef __ASSEMBLY__

#include <linux/types.h>

/*
 * Determine (based on compile-time knowledge) whether a cpucap is possible or
 * is known to never be set at runtime.
 *
 * This is intended to allow code to be elided when CONFIG_* options neccessary
 * for a to a cpucap are not selected. This must only depend on compile-time
 * constants and must not read from global variables or execute dynamic tests
 * that the compiler cannot reduce to a compile-time constant.
 */
static __always_inline bool
cpucap_is_compiletime_possible(unsigned int cap)
{
	if (cap >= ARM64_NCAPS)
		return false;

	switch (cap) {
	case ARM64_WORKAROUND_NVIDIA_CARMEL_CNP:
		return IS_ENABLED(CONFIG_NVIDIA_CARMEL_CNP_ERRATUM);
	default:
		return true;
	}
}

#endif /* __ASSEMBLY__ */

#endif /* __ASM_CPUCAPS_H */
