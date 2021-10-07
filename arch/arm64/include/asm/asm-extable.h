/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __ASM_ASM_EXTABLE_H
#define __ASM_ASM_EXTABLE_H

#ifdef __ASSEMBLY__

#define __ASM_EXTABLE_RAW(insn, fixup, handler, data)	\
	.pushsection	__ex_table, "a";		\
	.align		3;				\
	.long		((insn) - .);			\
	.long		((fixup) - .);			\
	.long		((handler) - .);		\
	.long		(data);				\
	.popsection;

/*
 * Create an exception table entry for `insn`, which will branch to `fixup`
 * when an unhandled fault is taken.
 */
	.macro		_asm_extable, insn, fixup
	__ASM_EXTABLE_RAW(\insn, \fixup, ex_handler_fixup, 0)
	.endm

/*
 * Create an exception table entry for `insn` if `fixup` is provided. Otherwise
 * do nothing.
 */
	.macro		_cond_extable, insn, fixup
	.ifnc		\fixup,
	_asm_extable	\insn, \fixup
	.endif
	.endm

#else /* __ASSEMBLY__ */

#include <linux/bits.h>
#include <linux/stringify.h>

#include <asm/gpr-num.h>

#define __ASM_EXTABLE_RAW(insn, fixup, handler, data)	\
	".pushsection	__ex_table, \"a\"\n"		\
	".align		3\n"				\
	".long		((" insn ") - .)\n"		\
	".long		((" fixup ") - .)\n"		\
	".long		((" handler ") - .)\n"		\
	".long		(" data ")\n"			\
	".popsection\n"

#define _ASM_EXTABLE(insn, fixup)					\
	__ASM_EXTABLE_RAW(#insn, #fixup, "ex_handler_fixup", "0")

#define EX_DATA_REG_ERR_SHIFT	0
#define EX_DATA_REG_ERR		GENMASK(4, 0)
#define EX_DATA_REG_ZERO_SHIFT	5
#define EX_DATA_REG_ZERO	GENMASK(9, 5)

#define EX_DATA_REG(reg, gpr)						\
	"((.L__gpr_num_" #gpr ") << " __stringify(EX_DATA_REG_##reg##_SHIFT) ")"

#define _ASM_EXTABLE_EFAULT_ZERO(insn, fixup, err, zero)		\
	__DEFINE_ASM_GPR_NUMS						\
	__ASM_EXTABLE_RAW(#insn, #fixup, "ex_handler_efault_zero",	\
			  "(" EX_DATA_REG(ERR, err) " | " EX_DATA_REG(ZERO, zero) ")")

#define _ASM_EXTABLE_EFAULT(insn, fixup, err)				\
	_ASM_EXTABLE_EFAULT_ZERO(insn, fixup, err, xzr)

#define EX_DATA_REG_DATA_SHIFT	0
#define EX_DATA_REG_DATA	GENMASK(4, 0)
#define EX_DATA_REG_ADDR_SHIFT	5
#define EX_DATA_REG_ADDR	GENMASK(9, 5)

#define _ASM_EXTABLE_LUZ(insn, fixup, data, addr)		\
	__DEFINE_ASM_GPR_NUMS					\
	__ASM_EXTABLE_RAW(#insn, #fixup, "ex_handler_luz",	\
			  "(" EX_DATA_REG(DATA, data) " | " EX_DATA_REG(ADDR, addr) ")")

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ASM_EXTABLE_H */
