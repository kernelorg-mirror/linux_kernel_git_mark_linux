#include <linux/compiler.h>
#include <linux/context_tracking.h>
#include <linux/irqflags.h>
#include <linux/kconfig.h>
#include <linux/linkage.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/thread_info.h>
#include <linux/types.h>

#include <asm/alternative.h>
#include <asm/cpufeature.h>
#include <asm/cputype.h>
#include <asm/debug-monitors.h>
#include <asm/esr.h>
#include <asm/exception.h>
#include <asm/irqflags.h>
#include <asm/preempt.h>
#include <asm/processor.h>
#include <asm/syscall.h>
#include <asm/sysreg.h>
#include <asm/unistd.h>

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

asmlinkage long do_ni_syscall(struct pt_regs *regs);

asmlinkage int syscall_trace_enter(struct pt_regs *regs);

asmlinkage void syscall_trace_exit(struct pt_regs *regs);

asmlinkage void bad_mode(struct pt_regs *regs, int reason, unsigned int esr);

asmlinkage __visible void __sched preempt_schedule_irq(void);

extern void (*handle_arch_irq)(struct pt_regs *);

typedef void (*exception_handler)(struct pt_regs *regs, u32 esr);

void do_fpsimd_acc(unsigned int esr, struct pt_regs *regs);

void do_fpsimd_exc(unsigned int esr, struct pt_regs *regs);

asmlinkage void do_notify_resume(struct pt_regs *regs,
				 unsigned int thread_flags);

static unsigned long get_ti_flags(void)
{
	return READ_ONCE(current_thread_info()->flags);
}

static void singlestep_disable(void)
{
	unsigned long mdscr;

	if (!(get_ti_flags() & TIF_SINGLESTEP))
		return;

	mdscr = read_sysreg(MDSCR_EL1);
	mdscr &= ~DBG_MDSCR_SS;
	write_sysreg(mdscr, MDSCR_EL1);
}

static void singlestep_enable(void)
{
	unsigned long mdscr;

	if (!(get_ti_flags() & TIF_SINGLESTEP))
		return;

	local_dbg_disable();

	mdscr = read_sysreg(MDSCR_EL1);
	mdscr |= DBG_MDSCR_SS;
	write_sysreg(mdscr, MDSCR_EL1);
	isb();
}

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

static void workaround_cortex_a53_845719(void)
{
	unsigned long tmp;

	if (!IS_ENABLED(CONFIG_ARM64_ERRATUM_845719))
		return;

	if (IS_ENABLED(CONFIG_PID_IN_CONTEXTIDR)) {
		asm volatile(
			ALTERNATIVE(
				"nop; nop",
				"mrs %0, contextidr_el1; msr contextidr_el1, %0",
				ARM64_WORKAROUND_845719
			)
		: "=r" (tmp));
	} else {
		asm volatile(
			ALTERNATIVE(
				"nop",
				"msr contextidr_el1, xzr",
				ARM64_WORKAROUND_845719
			)
		);
	}
}

static void el0_prepare_entry(void)
{
	singlestep_disable();
}

static void el0_prepare_return(struct pt_regs *regs)
{
	unsigned long flags;

	for (;;) {
		raw_local_irq_disable();

		flags = get_ti_flags();

		if (!(flags & _TIF_WORK_MASK))
			break;

		if (flags & _TIF_NEED_RESCHED) {
			raw_local_irq_enable();
			schedule();
			continue;
		}

		raw_local_irq_enable();
		do_notify_resume(regs, flags);
	}

	singlestep_enable();
	user_enter();
	workaround_cortex_a53_845719();
}

void el0_ret_from_fork(struct pt_regs *regs)
{
	el0_prepare_return(regs);
}

#if 0
static void __handle_irq_onstack(struct pt_regs *regs)
{
	register struct pt_regs *arg asm("x0") = regs;
	unsigned long irqsp = IRQ_STACK_PTR(smp_processor_id());
	unsigned long tmp;

	/*
	 * The first item on the irq stack is a non-standard frame record,
	 * consisting of the saved FP and SP rather than FP and LR. In
	 * unwind_frame we have a special case to use this when walking form
	 * the irq stack to the task stack, as otherwise we cannot determine
	 * the SP of the previous task stack frame.
	 *
	 * Note that irqsp and tmp must be callee-saved registers due to our
	 * other clobbers.
	 */
	asm volatile(
	"	mov	%[tmp], sp\n"
	"	mov	sp, %[irqsp]\n"
	"	stp	x29, %[tmp], [sp, #-16]!\n"
	"	mov	x29, sp\n"
	"	blr	%[func]\n"
	"	ldr	x29, [sp]\n"
	"	mov	sp, %[tmp]\n"
	: [regs] "=&r" (arg), /* x0 argument to func */
	  [tmp] "=&r" (tmp)
	: [irqsp] "r" (irqsp),
	  [func] "r" (handle_arch_irq)
	: "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
	  "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x30",
	  "cc", "memory"
	);
}
#else
void call_on_stack(void *arg, void *func, unsigned long stack);

static void __handle_irq_onstack(struct pt_regs *regs)
{
	call_on_stack(regs, handle_arch_irq, IRQ_STACK_PTR(smp_processor_id()));
}
#endif

static void handle_irq_onstack(struct pt_regs *regs)
{
	unsigned long sp = current_stack_pointer;

	if (on_irq_stack(sp, smp_processor_id()))
		handle_arch_irq(regs);
	else
		__handle_irq_onstack(regs);
}


asmlinkage void __el1_irq(struct pt_regs *regs)
{
	local_dbg_enable();
	trace_hardirqs_off();
	handle_irq_onstack(regs);
	el1_preempt();
	trace_hardirqs_on();
}

asmlinkage void __el0_irq_naked(struct pt_regs *regs)
{
	el0_prepare_entry();
	local_dbg_enable();
	trace_hardirqs_off();
	user_exit(); /* balanced in kernel_exit ??? */
	handle_irq_onstack(regs);
	el1_preempt();
	trace_hardirqs_on();
	el0_prepare_return(regs);
}

static void el0_inv(struct pt_regs *regs, u32 esr)
{
	el0_prepare_entry();
	local_dbg_enable();
	user_exit();
	bad_mode(regs, BAD_SYNC, esr);
}

typedef long (*syscall_func)(unsigned long, unsigned long, unsigned long,
			     unsigned long, unsigned long, unsigned long,
			     unsigned long);

static void el0_syscall_raw(struct pt_regs *regs, const void *table[], int sc_nr)
{
	u32 scno = regs->syscallno;
	syscall_func syscall;

	if (scno >= sc_nr) {
		regs->regs[0] = do_ni_syscall(regs);
		return;
	}

	/*
	 * Invoke an arbitrary function that has up to 7 arguments by relying
	 * on the AAPCS. This is not portable.
	 */
	syscall = table[scno];
	regs->regs[0] = syscall(regs->regs[0], regs->regs[1], regs->regs[2],
				regs->regs[3], regs->regs[4], regs->regs[5],
				regs->regs[6]);
}

static void el0_syscall_trace(struct pt_regs *regs, const void *table[], int sc_nr)
{
	int ret;

	/* Set default errno for user-issued syscall(-1) */
	if (syscall_get_nr(current, regs) == -1)
		syscall_set_return_value(current, regs, -ENOSYS, 0);

	ret = syscall_trace_enter(regs);
	if (ret == -1)
		goto out_trace_exit;

	/* TODO: consider syscall_set_nr() */
	regs->syscallno = ret;

	el0_syscall_raw(regs, table, sc_nr);

out_trace_exit:
	syscall_trace_exit(regs);
}

static void el0_svc_naked(struct pt_regs *regs, const void *table[], int sc_nr)
{
	regs->orig_x0 = regs->regs[0];

	local_dbg_enable();
	local_irq_enable();

	user_exit();

	if (current_thread_info()->flags & _TIF_SYSCALL_WORK)
		el0_syscall_trace(regs, table, sc_nr);
	else
		el0_syscall_raw(regs, table, sc_nr);
}

static void el0_svc(struct pt_regs *regs, u32 esr)
{
	regs->syscallno = regs->regs[8];
	el0_svc_naked(regs, sys_call_table, __NR_syscalls);
}

static void el0_da(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);

	local_dbg_enable();
	local_irq_enable();

	user_exit();

	/* Clear any tagged address bits */
	addr &= ~(0xffUL << 56);

	do_mem_abort(addr, esr, regs);
}

static void el0_ia(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);

	local_dbg_enable();
	local_irq_enable();

	user_exit();

	do_mem_abort(addr, esr, regs);
}

static void el0_fpsimd_acc(struct pt_regs *regs, u32 esr)
{
	local_dbg_enable();

	user_exit();

	do_fpsimd_acc(esr, regs);
}

static void el0_fpsimd_exc(struct pt_regs *regs, u32 esr)
{
	local_dbg_enable();

	user_exit();

	do_fpsimd_exc(esr, regs);
}

static void el0_undef(struct pt_regs *regs, u32 esr)
{
	local_dbg_enable();
	local_irq_enable();

	user_exit();

	do_undefinstr(regs);
}

static void el0_sp_pc(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);

	local_dbg_enable();
	local_irq_enable();

	do_sp_pc_abort(addr, esr, regs);
}

static void el0_dbg(struct pt_regs *regs, u32 esr)
{
	unsigned long addr = read_sysreg(FAR_EL1);
	do_debug_exception(addr, esr, regs);

	local_dbg_enable();
	user_exit();
}

static const exception_handler el0_sync_handlers[] = {
	[0 ... ESR_ELx_EC_MAX]		= el0_inv,
	[ESR_ELx_EC_SVC64]		= el0_svc,
	[ESR_ELx_EC_DABT_LOW]		= el0_da,
	[ESR_ELx_EC_IABT_LOW]		= el0_ia,
	[ESR_ELx_EC_FP_ASIMD]		= el0_fpsimd_acc,
	[ESR_ELx_EC_FP_EXC64]		= el0_fpsimd_exc,
	[ESR_ELx_EC_SYS64]		= el0_undef,
	[ESR_ELx_EC_SP_ALIGN]		= el0_sp_pc,
	[ESR_ELx_EC_PC_ALIGN]		= el0_sp_pc,
	[ESR_ELx_EC_UNKNOWN]		= el0_undef,
	[ESR_ELx_EC_BREAKPT_LOW]	= el0_dbg,
	[ESR_ELx_EC_SOFTSTP_LOW]	= el0_dbg,
	[ESR_ELx_EC_WATCHPT_LOW]	= el0_dbg,
	[ESR_ELx_EC_BRK64]		= el0_dbg,
};

static inline void __el0_sync_naked(struct pt_regs *regs,
				    const exception_handler *handlers)
{
	u32 esr = read_sysreg(ESR_EL1);
	el0_prepare_entry();
	handlers[ESR_ELx_EC(esr)](regs, esr);
	el0_prepare_return(regs);
}

asmlinkage void __el0_sync(struct pt_regs *regs)
{
	__el0_sync_naked(regs, el0_sync_handlers);
}

#ifdef CONFIG_COMPAT

extern const void * compat_sys_call_table[];

static void el0_svc_compat(struct pt_regs *regs, u32 esr)
{
	regs->syscallno = regs->regs[7];
	el0_svc_naked(regs, compat_sys_call_table, __NR_compat_syscalls);
}

static const exception_handler el0_sync_compat_handlers[] = {
	[0 ... ESR_ELx_EC_MAX]		= el0_inv,
	[ESR_ELx_EC_SVC32]		= el0_svc_compat,
	[ESR_ELx_EC_DABT_LOW]		= el0_da,
	[ESR_ELx_EC_IABT_LOW]		= el0_ia,
	[ESR_ELx_EC_FP_ASIMD]		= el0_fpsimd_acc,
	[ESR_ELx_EC_FP_EXC32]		= el0_fpsimd_exc,
	[ESR_ELx_EC_PC_ALIGN]		= el0_sp_pc,
	[ESR_ELx_EC_UNKNOWN]		= el0_undef,
	[ESR_ELx_EC_CP15_32]		= el0_undef,
	[ESR_ELx_EC_CP15_64]		= el0_undef,
	[ESR_ELx_EC_CP14_MR]		= el0_undef,
	[ESR_ELx_EC_CP14_LS]		= el0_undef,
	[ESR_ELx_EC_CP14_64]		= el0_undef,
	[ESR_ELx_EC_BREAKPT_LOW]	= el0_dbg,
	[ESR_ELx_EC_SOFTSTP_LOW]	= el0_dbg,
	[ESR_ELx_EC_WATCHPT_LOW]	= el0_dbg,
	[ESR_ELx_EC_BKPT32]		= el0_dbg,
	[ESR_ELx_EC_VECTOR32]		= el0_dbg,
};

asmlinkage void __el0_sync_compat(struct pt_regs *regs)
{
	__el0_sync_naked(regs, el0_sync_compat_handlers);
}

#endif /* CONFIG_COMPAT */
