// SPDX-License-Identifier: GPL-2.0-only
/*
 * brbe_test - Tests the AArch64 Branch Record Buffer Extension (BRBE)
 *
 * As BRBE is not currently exposed to guests, this only checks that usage of
 * BRBE registers and instructions raise an UNDEFINED exception. This will need
 * to be updated when BRBE is exposed to guests.
 *
 * Copyright (c) 2024 Arm Limited.
 */

#include <stdint.h>

#include "kvm_util.h"
#include "processor.h"
#include "test_util.h"
#include <linux/bitfield.h>

#include <asm/sysreg.h>

static volatile uint64_t undef_expected_pc;
static volatile uint64_t undef_taken;

static void guest_undef_handler(struct ex_regs *regs)
{
	GUEST_ASSERT_EQ(undef_expected_pc, regs->pc);
	undef_taken += 1;
	regs->pc += 4;
}

#define __TEST_UNDEF_INSN(name, lbl, insn...)				\
do {									\
	extern unsigned char lbl;					\
									\
	undef_taken = 0;						\
	undef_expected_pc = (uint64_t)&(lbl);				\
									\
	asm volatile(#lbl ": " insn);					\
									\
	undef_expected_pc = 0;						\
	GUEST_ASSERT_EQ(undef_taken, 1);				\
} while (0)

#define TEST_UNDEF_INSN(name, insn)					\
	__TEST_UNDEF_INSN(name, name##_lbl, insn)

#define TEST_UNDEF_SYSREG_R(name, _Op0, _Op1, _CRn, _CRm, _Op2)		\
do {									\
	__TEST_UNDEF_INSN(name, name##_read_lbl,			\
	"	mrs xzr, S%[op0]_%[op1]_C%[crn]_C%[crm]_%[op2]\n"	\
	:								\
	: [op0] "i" (_Op0), [op1] "i" (_Op1),				\
	  [crn] "i" (_CRn), [crm] "i" (_CRm),				\
	  [op2] "i" (_Op2)						\
	);								\
} while (0)

#define TEST_UNDEF_SYSREG_W(name, _Op0, _Op1, _CRn, _CRm, _Op2)	\
do {									\
	__TEST_UNDEF_INSN(name, name##_write_lbl,			\
	"	msr S%[op0]_%[op1]_C%[crn]_C%[crm]_%[op2], xzr\n"	\
	:								\
	: [op0] "i" (_Op0), [op1] "i" (_Op1),				\
	  [crn] "i" (_CRn), [crm] "i" (_CRm),				\
	  [op2] "i" (_Op2)						\
	);								\
} while (0)

#define TEST_UNDEF_SYSREG_RW(name, _Op0, _Op1, _CRn, _CRm, _Op2)	\
do {									\
	TEST_UNDEF_SYSREG_R(name, _Op0, _Op1, _CRn, _CRm, _Op2);	\
	TEST_UNDEF_SYSREG_W(name, _Op0, _Op1, _CRn, _CRm, _Op2);	\
} while (0)

/*
 * To generate the encodings for the BRBE record registers we need the indices
 * (0 to 31 inclusive) to be compile-time constants, and hence cannot use a
 * loop to iterate over them.
 *
 * This code is really good.
 */
#define FOR_EACH_BRBN(__do)						\
do {									\
	__do( 0); __do( 1); __do( 2); __do( 3);				\
	__do( 4); __do( 5); __do( 6); __do( 7);				\
	__do( 8); __do( 9); __do(10); __do(11);				\
	__do(12); __do(13); __do(14); __do(15);				\
	__do(16); __do(17); __do(18); __do(19);				\
	__do(20); __do(21); __do(22); __do(23);				\
	__do(24); __do(25); __do(26); __do(27);				\
	__do(28); __do(29); __do(30); __do(31);				\
} while (0);

#define __test_undef_brbinfn(n)						\
do {									\
	const int CRm = n & GENMASK(3, 0);				\
	const int Op2 = n & GENMASK(4, 4) << 2;				\
	TEST_UNDEF_SYSREG_RW(brbinf##n##_el1, 2, 1, 8, CRm, Op2);	\
} while (0)

#define __test_undef_brbsrcn(n)						\
do {									\
	const int CRm = n & GENMASK(3, 0);				\
	const int Op2 = (n & GENMASK(4, 4) << 2) | 1;			\
	TEST_UNDEF_SYSREG_RW(brbsrc##n##_el1, 2, 1, 8, CRm, Op2);	\
} while (0)

#define __test_undef_brbtgtn(n)						\
do {									\
	const int CRm = n & GENMASK(3, 0);				\
	const int Op2 = (n & GENMASK(4, 4) << 2) | 2;			\
	TEST_UNDEF_SYSREG_RW(brbtgt##n##_el1, 2, 1, 8, CRm, Op2);	\
} while (0)


static void guest_main(void)
{
	TEST_UNDEF_SYSREG_RW(brbcr_el1,		2, 1, 9, 0, 0);
	TEST_UNDEF_SYSREG_RW(brbfcr_el1,	2, 1, 9, 0, 1);
	TEST_UNDEF_SYSREG_RW(brbidr_el1,	2, 1, 9, 2, 0);

	FOR_EACH_BRBN(__test_undef_brbinfn);
	TEST_UNDEF_SYSREG_RW(brbinfinj_el1,	2, 1, 9, 1, 0);

	FOR_EACH_BRBN(__test_undef_brbsrcn);
	TEST_UNDEF_SYSREG_RW(brbsrcinj_el1,	2, 1, 9, 1, 1);

	FOR_EACH_BRBN(__test_undef_brbtgtn);
	TEST_UNDEF_SYSREG_RW(brbtgtinj_el1,	2, 1, 9, 1, 2);

	TEST_UNDEF_SYSREG_RW(brbts_el1,		2, 1, 9, 0, 2);

	TEST_UNDEF_INSN(brb_iall,	"sys #1, C7, C2, 4, xzr");
	TEST_UNDEF_INSN(brb_inj,	"sys #1, C7, C2, 5, xzr");

	GUEST_DONE();
}

static void test_guest_brbe_undef(void)
{
	struct kvm_vcpu *vcpu;
	struct kvm_vm *vm;
	struct ucall uc;

	vm = vm_create_with_one_vcpu(&vcpu, guest_main);

	vm_init_descriptor_tables(vm);
	vcpu_init_descriptor_tables(vcpu);

	vm_install_sync_handler(vm, VECTOR_SYNC_CURRENT,
				ESR_EC_UNKNOWN, guest_undef_handler);

	vcpu_run(vcpu);

	switch (get_ucall(vcpu, &uc)) {
	case UCALL_ABORT:
		REPORT_GUEST_ASSERT(uc);
		break;
	case UCALL_DONE:
		break;
	default:
		TEST_FAIL("Unexpected ucall: %lu", uc.cmd);
	}

	kvm_vm_free(vm);
}

int main(void)
{
	test_guest_brbe_undef();
}
