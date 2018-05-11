// SPDX-License-Identifier: GPL-2.0
#ifndef __ASM_PTRAUTH_ASM_H
#define __ASM_PTRAUTH_ASM_H

#include <asm/asm-offsets.h>
#include <asm/sysreg.h>

#ifdef CONFIG_ARM64_PTR_AUTH

	.macro ptrauth_keys_install_user tsk, tmp
	ldr	\tmp, [\tsk, #(TSK_TI_KEYS_USER + PTRAUTH_KEY_APIALO)]
	msr_s	SYS_APIAKEYLO_EL1, \tmp
	ldr	\tmp, [\tsk, #(TSK_TI_KEYS_USER + PTRAUTH_KEY_APIAHI)]
	msr_s	SYS_APIAKEYHI_EL1, \tmp
	.endm

#else /* CONFIG_ARM64_PTR_AUTH */

	.macro ptrauth_keys_install_user keys, tmp
	.endm

#endif /* CONFIG_ARM64_PTR_AUTH */

#endif /* __ASM_PTRAUTH_ASM_H */
