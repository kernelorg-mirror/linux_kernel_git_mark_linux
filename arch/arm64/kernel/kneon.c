// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024 ARM Ltd.
 */

#define pr_fmt(fmt) "kneon: " fmt

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/printk.h>

#include <asm/asm-bug.h>
#include <asm/neon.h>
#include <asm/simd.h>
#include <asm/sysreg.h>

static struct task_struct *neon_tsk;

static int kneon_thread(void *arg)
{
	unsigned long out;
	unsigned long in = ~0UL;

	if (!may_use_simd()) {
		pr_info("SIMD unavailable\n");
		return -ENOENT;
	}

	kernel_neon_begin();

	asm volatile(
	"	mov	v0.d[0], %[in]\n"
	".Lloop:\n"
	"	mov	%[out], v0.d[0]\n"
	"	cmp	%[in], %[out]\n"
	"	b.eq	.Lloop\n"
	: [out] "=r" (out)
	: [in] "r" (in)
	: "cc"
	);

	pr_info("clobbered from 0x%016lx -> 0x%016lx\n", in, out);

	kernel_neon_end();

	return 0;
}

static __init int kneon_init(void)
{
	neon_tsk = kthread_run(kneon_thread, NULL, "kneon");
	if (IS_ERR(neon_tsk))
		return PTR_ERR(neon_tsk);

	pr_info("Initialized\n");
	return 0;
}

static __exit void kneon_exit(void)
{
	kthread_stop(neon_tsk);
	pr_info("Stopped\n");
}

module_init(kneon_init);
module_exit(kneon_exit);

MODULE_DESCRIPTION("Kernel mode neon test");
MODULE_LICENSE("GPL");
