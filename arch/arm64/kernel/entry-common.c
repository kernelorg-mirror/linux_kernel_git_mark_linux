// SPDX-License-Identifier: GPL-2.0
/*
 * Exception handling code
 *
 * Copyright (C) 2019 ARM Ltd.
 */

#include <linux/context_tracking.h>
#include <linux/hardirq.h>
#include <linux/irq.h>
#include <linux/irqflags.h>
#include <linux/linkage.h>
#include <linux/lockdep.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
#include <linux/ptrace.h>
#include <linux/sched/debug.h>
#include <linux/thread_info.h>

#include <asm/cpufeature.h>
#include <asm/daifflags.h>
#include <asm/esr.h>
#include <asm/exception.h>
#include <asm/kprobes.h>
#include <asm/mmu.h>
#include <asm/stacktrace.h>
#include <asm/sysreg.h>

static void notrace el1_abort(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	local_daif_inherit(regs);
	far = untagged_addr(far);
	do_mem_abort(far, esr, regs);
}
NOKPROBE_SYMBOL(el1_abort);

static void notrace el1_pc(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	local_daif_inherit(regs);
	do_sp_pc_abort(far, esr, regs);
}
NOKPROBE_SYMBOL(el1_pc);

static void notrace el1_undef(struct pt_regs *regs)
{
	local_daif_inherit(regs);
	do_undefinstr(regs);
}
NOKPROBE_SYMBOL(el1_undef);

static void notrace el1_inv(struct pt_regs *regs, unsigned long esr)
{
	local_daif_inherit(regs);
	bad_mode(regs, 0, esr);
}
NOKPROBE_SYMBOL(el1_inv);

static void notrace el1_dbg(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	/*
	 * The CPU masked interrupts, and we are leaving them masked during
	 * do_debug_exception(). Update PMR as if we had called
	 * local_mask_daif().
	 */
	if (system_uses_irq_prio_masking())
		gic_write_pmr(GIC_PRIO_IRQON | GIC_PRIO_PSR_I_SET);

	do_debug_exception(far, esr, regs);
}
NOKPROBE_SYMBOL(el1_dbg);

asmlinkage void notrace el1_sync_handler(struct pt_regs *regs)
{
	unsigned long esr = read_sysreg(esr_el1);

	switch (ESR_ELx_EC(esr)) {
	case ESR_ELx_EC_DABT_CUR:
	case ESR_ELx_EC_IABT_CUR:
		el1_abort(regs, esr);
		break;
	/*
	 * We don't handle ESR_ELx_EC_SP_ALIGN, since we will have hit a
	 * recursive exception when trying to push the initial pt_regs.
	 */
	case ESR_ELx_EC_PC_ALIGN:
		el1_pc(regs, esr);
		break;
	case ESR_ELx_EC_SYS64:
	case ESR_ELx_EC_UNKNOWN:
		el1_undef(regs);
		break;
	case ESR_ELx_EC_BREAKPT_CUR:
	case ESR_ELx_EC_SOFTSTP_CUR:
	case ESR_ELx_EC_WATCHPT_CUR:
	case ESR_ELx_EC_BRK64:
		el1_dbg(regs, esr);
		break;
	default:
		el1_inv(regs, esr);
	};
}
NOKPROBE_SYMBOL(el1_sync_handler);

static void notrace el0_da(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	far = untagged_addr(far);
	do_mem_abort(far, esr, regs);
}
NOKPROBE_SYMBOL(el0_da);

static void notrace el0_ia(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	/*
	 * We've taken an instruction abort from userspace and not yet
	 * re-enabled IRQs. If the address is a kernel address, apply
	 * BP hardening prior to enabling IRQs and pre-emption.
	 */
	if (!is_ttbr0_addr(far))
		arm64_apply_bp_hardening();

	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_mem_abort(far, esr, regs);
}
NOKPROBE_SYMBOL(el0_ia);

static void notrace el0_fpsimd_acc(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_fpsimd_acc(esr, regs);
}
NOKPROBE_SYMBOL(el0_fpsimd_acc);

static void notrace el0_sve_acc(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_sve_acc(esr, regs);
}
NOKPROBE_SYMBOL(el0_sve_acc);

static void notrace el0_fpsimd_exc(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_fpsimd_exc(esr, regs);
}
NOKPROBE_SYMBOL(el0_fpsimd_exc);

static void notrace el0_sys(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_sysinstr(esr, regs);
}
NOKPROBE_SYMBOL(el0_sys);

static void notrace el0_pc(struct pt_regs *regs, unsigned long esr)
{
	unsigned long far = read_sysreg(far_el1);

	if (!is_ttbr0_addr(instruction_pointer(regs)))
		arm64_apply_bp_hardening();

	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_sp_pc_abort(far, esr, regs);
}
NOKPROBE_SYMBOL(el0_pc);

static void notrace el0_sp(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX_NOIRQ);
	do_sp_pc_abort(regs->sp, esr, regs);
}
NOKPROBE_SYMBOL(el0_sp);

static void notrace el0_undef(struct pt_regs *regs)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_undefinstr(regs);
}
NOKPROBE_SYMBOL(el0_undef);

static void notrace el0_inv(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	bad_el0_sync(regs, 0, esr);
}
NOKPROBE_SYMBOL(el0_inv);

static void notrace el0_dbg(struct pt_regs *regs, unsigned long esr)
{
	/* Only watchpoints write FAR_EL1, otherwise its UNKNOWN */
	unsigned long far = read_sysreg(far_el1);

	if (system_uses_irq_prio_masking())
		gic_write_pmr(GIC_PRIO_IRQON | GIC_PRIO_PSR_I_SET);

	user_exit_irqoff();
	do_debug_exception(far, esr, regs);
	local_daif_restore(DAIF_PROCCTX_NOIRQ);
}
NOKPROBE_SYMBOL(el0_dbg);

static void notrace el0_svc(struct pt_regs *regs)
{
	if (system_uses_irq_prio_masking())
		gic_write_pmr(GIC_PRIO_IRQON | GIC_PRIO_PSR_I_SET);

	do_el0_svc(regs);
}
NOKPROBE_SYMBOL(el0_svc);

asmlinkage void notrace el0_sync_handler(struct pt_regs *regs)
{
	unsigned long esr = read_sysreg(esr_el1);

	switch (ESR_ELx_EC(esr)) {
	case ESR_ELx_EC_SVC64:
		el0_svc(regs);
		break;
	case ESR_ELx_EC_DABT_LOW:
		el0_da(regs, esr);
		break;
	case ESR_ELx_EC_IABT_LOW:
		el0_ia(regs, esr);
		break;
	case ESR_ELx_EC_FP_ASIMD:
		el0_fpsimd_acc(regs, esr);
		break;
	case ESR_ELx_EC_SVE:
		el0_sve_acc(regs, esr);
		break;
	case ESR_ELx_EC_FP_EXC64:
		el0_fpsimd_exc(regs, esr);
		break;
	case ESR_ELx_EC_SYS64:
	case ESR_ELx_EC_WFx:
		el0_sys(regs, esr);
		break;
	case ESR_ELx_EC_SP_ALIGN:
		el0_sp(regs, esr);
		break;
	case ESR_ELx_EC_PC_ALIGN:
		el0_pc(regs, esr);
		break;
	case ESR_ELx_EC_UNKNOWN:
		el0_undef(regs);
		break;
	case ESR_ELx_EC_BREAKPT_LOW:
	case ESR_ELx_EC_SOFTSTP_LOW:
	case ESR_ELx_EC_WATCHPT_LOW:
	case ESR_ELx_EC_BRK64:
		el0_dbg(regs, esr);
		break;
	default:
		el0_inv(regs, esr);
	}
}
NOKPROBE_SYMBOL(el0_sync_handler);

#ifdef CONFIG_COMPAT
static void notrace el0_cp15(struct pt_regs *regs, unsigned long esr)
{
	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX);
	do_cp15instr(esr, regs);
}
NOKPROBE_SYMBOL(el0_cp15);

static void notrace el0_svc_compat(struct pt_regs *regs)
{
	if (system_uses_irq_prio_masking())
		gic_write_pmr(GIC_PRIO_IRQON | GIC_PRIO_PSR_I_SET);

	do_el0_svc_compat(regs);
}
NOKPROBE_SYMBOL(el0_svc_compat);

asmlinkage void notrace el0_sync_compat_handler(struct pt_regs *regs)
{
	unsigned long esr = read_sysreg(esr_el1);

	switch (ESR_ELx_EC(esr)) {
	case ESR_ELx_EC_SVC32:
		el0_svc_compat(regs);
		break;
	case ESR_ELx_EC_DABT_LOW:
		el0_da(regs, esr);
		break;
	case ESR_ELx_EC_IABT_LOW:
		el0_ia(regs, esr);
		break;
	case ESR_ELx_EC_FP_ASIMD:
		el0_fpsimd_acc(regs, esr);
		break;
	case ESR_ELx_EC_FP_EXC32:
		el0_fpsimd_exc(regs, esr);
		break;
	case ESR_ELx_EC_PC_ALIGN:
		el0_pc(regs, esr);
		break;
	case ESR_ELx_EC_UNKNOWN:
	case ESR_ELx_EC_CP14_MR:
	case ESR_ELx_EC_CP14_LS:
	case ESR_ELx_EC_CP14_64:
		el0_undef(regs);
		break;
	case ESR_ELx_EC_CP15_32:
	case ESR_ELx_EC_CP15_64:
		el0_cp15(regs, esr);
		break;
	case ESR_ELx_EC_BREAKPT_LOW:
	case ESR_ELx_EC_SOFTSTP_LOW:
	case ESR_ELx_EC_WATCHPT_LOW:
	case ESR_ELx_EC_BKPT32:
		el0_dbg(regs, esr);
		break;
	default:
		el0_inv(regs, esr);
	}
}
NOKPROBE_SYMBOL(el0_sync_compat_handler);
#endif /* CONFIG_COMPAT */

static void __sched el1_preempt(void)
{
	if (!IS_ENABLED(CONFIG_PREEMPT) || preempt_count())
		return;

	/*
	 * To avoid nesting NMIs and overflowing the stack, we must leave NMIs
	 * masked until the exception return. We want to context-switch with
	 * IRQs masked but NMIs enabled, so cannot preempt an NMI.
	 *
	 * PSTATE.{D,A,F} are cleared for IRQ and NMI by el1_irq_handler().
	 * When gic_handle_irq() handles an NMI, it leaves PSTATE.I set.
	 * If anything is set in DAIF, this is an NMI.
	 */
	if (system_uses_irq_prio_masking() && read_sysreg(daif) != 0)
		return;

	lockdep_assert_irqs_disabled();

	/*
	 * Preempting a task from an IRQ means we leave copies of PSTATE
	 * on the stack. cpufeature's enable calls may modify PSTATE, but
	 * resuming one of these preempted tasks would undo those changes.
	 *
	 * Only allow a task to be preempted once cpufeatures have been
	 * enabled.
	 */
	if (static_branch_likely(&arm64_const_caps_ready))
		preempt_schedule_irq();
}

static void notrace invoke_irq_handler(struct pt_regs *regs)
{
	unsigned long irq_stack = (unsigned long)raw_cpu_read(irq_stack_ptr);

	irq_stack += IRQ_STACK_SIZE;

	if (on_thread_stack())
		call_on_stack(regs, handle_arch_irq, irq_stack);
	else
		handle_arch_irq(regs);
}
NOKPROBE_SYMBOL(invoke_irq_handler);

asmlinkage void notrace el1_irq_handler(struct pt_regs *regs)
{
	bool masked;

	if (system_uses_irq_prio_masking())
		gic_write_pmr(regs->pmr_save | GIC_PRIO_PSR_I_SET);

	/*
	 * We can't use local_daif_restore(DAIF_PROCCTX_NOIRQ) here as it will
	 * see the A flag is clear and try to unmask NMIs.
	 */
	write_sysreg(DAIF_PROCCTX_NOIRQ, daif);

	/*
	 * If IRQs were masked, this is definitely an NMI. If IRQs were
	 * unmasked, this may be an IRQ or an NMI, and gic_handle_nmi() will
	 * handle nmi_{enter,exit} as necessary.
	 */
	masked = !irqs_priority_unmasked(regs);

	if (masked)
		nmi_enter();
	else
		trace_hardirqs_off();

	invoke_irq_handler(regs);

	if (masked) {
		nmi_exit();
	} else {
		el1_preempt();
		trace_hardirqs_on();
	}
}
NOKPROBE_SYMBOL(el1_irq_handler);

static inline void notrace do_el0_irq_bp_hardening(struct pt_regs *regs)
{
	if (!IS_ENABLED(CONFIG_HARDEN_BRANCH_PREDICTOR))
		return;
	if (regs->pc & BIT(55))
		arm64_apply_bp_hardening();
}
NOKPROBE_SYMBOL(do_el0_irq_bp_hardening);

asmlinkage void notrace el0_irq_handler(struct pt_regs *regs)
{
	if (system_uses_irq_prio_masking())
		gic_write_pmr(GIC_PRIO_IRQON | GIC_PRIO_PSR_I_SET);

	user_exit_irqoff();
	local_daif_restore(DAIF_PROCCTX_NOIRQ);
	trace_hardirqs_off();
	do_el0_irq_bp_hardening(regs);
	invoke_irq_handler(regs);
	trace_hardirqs_on();
}
NOKPROBE_SYMBOL(el0_irq_handler);
