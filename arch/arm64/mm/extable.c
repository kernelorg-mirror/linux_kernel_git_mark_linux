// SPDX-License-Identifier: GPL-2.0
/*
 * Based on arch/arm/mm/extable.c
 */

#include <linux/extable.h>
#include <linux/uaccess.h>

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
