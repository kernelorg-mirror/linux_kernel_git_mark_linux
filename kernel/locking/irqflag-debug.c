// SPDX-License-Identifier: GPL-2.0-only

#include <linux/bug.h>
#include <linux/irqflags.h>

void warn_bogus_irq_restore(void)
{
	WARN_ONCE(1, "local_irq_restore() called with IRQs enabled\n");
}
