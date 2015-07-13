#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/kconfig.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/thread_info.h>

#include <asm/irqflags.h>
#include <asm/preempt.h>

asmlinkage __visible void __sched preempt_schedule_irq(void);
extern void (*handle_arch_irq)(struct pt_regs *);

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

