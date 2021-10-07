// SPDX-License-Identifier: GPL-2.0
/*
 * Based on arch/arm/mm/extable.c
 */

#include <linux/bitfield.h>
#include <linux/extable.h>
#include <linux/uaccess.h>

#include <asm/ptrace.h>

typedef bool (*ex_handler_t)(const struct exception_table_entry *,
			     struct pt_regs *);

static inline ex_handler_t
get_ex_handler(const struct exception_table_entry *ex)
{
	return (ex_handler_t)((unsigned long)&ex->handler + ex->handler);
}

static inline unsigned long
get_ex_fixup(const struct exception_table_entry *ex)
{
	return ((unsigned long)&ex->fixup + ex->fixup);
}

bool fixup_exception(struct pt_regs *regs)
{
	const struct exception_table_entry *ex;
	ex_handler_t handler;

	ex = search_exception_tables(instruction_pointer(regs));
	if (!ex)
		return false;

	if (in_bpf_jit(regs))
		return arm64_bpf_fixup_exception(ex, regs);

	handler = get_ex_handler(ex);
	return handler(ex, regs);
}

bool ex_handler_fixup(const struct exception_table_entry *ex,
		      struct pt_regs *regs)
{
	regs->pc = get_ex_fixup(ex);
	return true;
}
EXPORT_SYMBOL(ex_handler_fixup);

bool ex_handler_efault_zero(const struct exception_table_entry *ex,
			   struct pt_regs *regs)
{
	int reg_err = FIELD_GET(EX_DATA_REG_ERR, ex->data);
	int reg_zero = FIELD_GET(EX_DATA_REG_ZERO, ex->data);

	pt_regs_write_reg(regs, reg_err, -EFAULT);
	pt_regs_write_reg(regs, reg_zero, 0);

	regs->pc = get_ex_fixup(ex);
	return true;
}
EXPORT_SYMBOL(ex_handler_efault_zero);

bool ex_handler_luz(const struct exception_table_entry *ex,
		    struct pt_regs *regs)
{
	int reg_data = FIELD_GET(EX_DATA_REG_DATA, ex->data);
	int reg_addr = FIELD_GET(EX_DATA_REG_ADDR, ex->data);
	unsigned long data, addr, offset;

	addr = pt_regs_read_reg(regs, reg_addr);

	offset = addr & 0x7UL;
	addr &= ~0x7UL;

	data = *(unsigned long*)addr;

#ifndef __AARCH64EB__
	data >>= 8 * offset;
#else
	data <<= 8 * offset;
#endif

	pt_regs_write_reg(regs, reg_data, data);

	regs->pc = get_ex_fixup(ex);
	return true;
}
EXPORT_SYMBOL(ex_handler_luz);
