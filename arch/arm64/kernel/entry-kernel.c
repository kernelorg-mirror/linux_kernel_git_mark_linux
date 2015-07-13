#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/kconfig.h>
#include <linux/linkage.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/thread_info.h>
#include <linux/types.h>

#include <asm/cputype.h>
#include <asm/esr.h>
#include <asm/exception.h>
#include <asm/irqflags.h>
#include <asm/preempt.h>
#include <asm/processor.h>
#include <asm/sysreg.h>

/*
 * Bad Abort numbers
 *-----------------
 */
#define BAD_SYNC	0
#define BAD_IRQ		1
#define BAD_FIQ		2
#define BAD_ERROR	3

asmlinkage void __exception do_mem_abort(unsigned long addr, unsigned int esr,
					 struct pt_regs *regs);

asmlinkage void __exception do_sp_pc_abort(unsigned long addr,
					   unsigned int esr,
					   struct pt_regs *regs);

asmlinkage void __exception do_undefinstr(struct pt_regs *regs);

asmlinkage int __exception do_debug_exception(unsigned long addr,
					      unsigned int esr,
					      struct pt_regs *regs);

asmlinkage void bad_mode(struct pt_regs *regs, int reason, unsigned int esr);

asmlinkage __visible void __sched preempt_schedule_irq(void);

extern void (*handle_arch_irq)(struct pt_regs *);

typedef void (*exception_handler)(struct pt_regs *regs, u32 esr);

static void el1_da(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);

	local_dbg_enable();
	local_irq_enable();

	do_mem_abort(addr, esr, regs);

	local_irq_disable();
}

static void el1_sp_pc(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);
	local_dbg_enable();
	do_sp_pc_abort(addr, esr, regs);
}

static void el1_undef(struct pt_regs *regs, u32 esr)
{
	local_dbg_enable();
	do_undefinstr(regs);
}

static void el1_dbg(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);
	do_debug_exception(addr, esr, regs);
}


static void el1_inv(struct pt_regs *regs, u32 esr)
{
	local_dbg_enable();
	bad_mode(regs, BAD_SYNC, esr);
}

static const exception_handler el1_sync_handlers[] = {
	[0 ... ESR_ELx_EC_MAX]		= el1_inv,
	[ESR_ELx_EC_DABT_CUR]		= el1_da,
	[ESR_ELx_EC_SYS64]		= el1_undef,
	[ESR_ELx_EC_SP_ALIGN]		= el1_sp_pc,
	[ESR_ELx_EC_PC_ALIGN]		= el1_sp_pc,
	[ESR_ELx_EC_UNKNOWN]		= el1_undef,
	[ESR_ELx_EC_BREAKPT_CUR]	= el1_dbg,
	[ESR_ELx_EC_SOFTSTP_CUR]	= el1_dbg,
	[ESR_ELx_EC_WATCHPT_CUR]	= el1_dbg,
	[ESR_ELx_EC_BRK64]		= el1_dbg,
};

asmlinkage void __el1_sync(struct pt_regs *regs)
{
	u32 esr = read_sysreg(ESR_EL1);
	trace_hardirqs_off();
	el1_sync_handlers[ESR_ELx_EC(esr)](regs, esr);
	trace_hardirqs_on();
}

static void el1_preempt(void)
{
	if (!IS_ENABLED(CONFIG_PREEMPT) || preempt_count())
		return;

	while (tif_need_resched())
		preempt_schedule_irq();
}

asmlinkage void __el1_irq(struct pt_regs *regs)
{
	local_dbg_enable();
	trace_hardirqs_off();
	handle_arch_irq(regs);
	el1_preempt();
	trace_hardirqs_on();
}

