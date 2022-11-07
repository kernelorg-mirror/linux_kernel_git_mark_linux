/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */

#ifndef __ASM_BRK_IMM_H
#define __ASM_BRK_IMM_H

/*
 * #imm16 values used for BRK instruction generation
 * 0x004: for installing kprobes
 * 0x005: for installing uprobes
 * 0x006: for kprobe software single-step
 * Allowed values for kgdb are 0x400 - 0x7ff
 * 0x100: for triggering a fault on purpose (reserved)
 * 0x400: for dynamic BRK instruction
 * 0x401: for compile time BRK instruction
 * 0x800: kernel-mode BUG() and WARN() traps
 * 0x9xx: tag-based KASAN trap (allowed values 0x900 - 0x9ff)
 * 0x55xx: Undefined Behavior Sanitizer traps ('U' << 8)
 * 0x8xxx: Control-Flow Integrity traps
 */
#define BRK_IMM_KPROBES			0x004
#define BRK_IMM_UPROBES			0x005
#define BRK_IMM_KPROBES_SS		0x006
#define BRK_IMM_FAULT			0x100
#define BRK_IMM_KGDB_DYNAMIC		0x400
#define BRK_IMM_KGDB_COMPILED		0x401
#define BRK_IMM_BUG			0x800

#define BRK_IMM_KASAN_BASE		0x900
#define BRK_IMM_KASAN_MASK		0x0ff

#define BRK_IMM_UBSAN_BASE		0x5500
#define BRK_IMM_UBSAN_MASK		0x00ff

#define BRK_IMM_CFI_TARGET		GENMASK(4, 0)
#define BRK_IMM_CFI_TYPE		GENMASK(9, 5)
#define BRK_IMM_CFI_BASE		0x8000
#define BRK_IMM_CFI_MASK		(BRK_IMM_CFI_TARGET | BRK_IMM_CFI_TYPE)

#endif
