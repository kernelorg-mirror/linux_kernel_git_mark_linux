// SPDX-License-Identifier: GPL-2.0-only
/*
 * KUnit tests for arm64 instruction manipulation code.
 *
 * Copyright (C) 2022 Arm Ltd.
 */

#include <kunit/test.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/types.h>

#include <asm/byteorder.h>
#include <asm/insn.h>
#include <asm/lse.h>

#define __ASM_U32(opcode, insn, inputs...)				\
({									\
	extern __le32 opcode;						\
	asm(								\
	"	.subsection 1\n"					\
	"	.globl " __stringify(opcode) "\n"			\
	__stringify(opcode) ":\n"					\
		insn "\n"						\
	"	.previous\n"						\
	:								\
	: inputs							\
	);								\
	le32_to_cpu(opcode);						\
})

#define ASM_U32(insn, inputs...)					\
	__ASM_U32(__UNIQUE_ID(opcode), insn, inputs)

/*
 * Map from x<N> or w<N> to AARCH64_INSN_REG_<n>
 *
 * This code is really good
 */
#define AARCH64_REG_IDX_x0	AARCH64_INSN_REG_0
#define AARCH64_REG_IDX_x1	AARCH64_INSN_REG_1
#define AARCH64_REG_IDX_x2	AARCH64_INSN_REG_2
#define AARCH64_REG_IDX_x3	AARCH64_INSN_REG_3
#define AARCH64_REG_IDX_x4	AARCH64_INSN_REG_4
#define AARCH64_REG_IDX_x5	AARCH64_INSN_REG_5
#define AARCH64_REG_IDX_x6	AARCH64_INSN_REG_6
#define AARCH64_REG_IDX_x7	AARCH64_INSN_REG_7
#define AARCH64_REG_IDX_x8	AARCH64_INSN_REG_8
#define AARCH64_REG_IDX_x9	AARCH64_INSN_REG_9
#define AARCH64_REG_IDX_x10	AARCH64_INSN_REG_10
#define AARCH64_REG_IDX_x11	AARCH64_INSN_REG_11
#define AARCH64_REG_IDX_x12	AARCH64_INSN_REG_12
#define AARCH64_REG_IDX_x13	AARCH64_INSN_REG_13
#define AARCH64_REG_IDX_x14	AARCH64_INSN_REG_14
#define AARCH64_REG_IDX_x15	AARCH64_INSN_REG_15
#define AARCH64_REG_IDX_x16	AARCH64_INSN_REG_16
#define AARCH64_REG_IDX_x17	AARCH64_INSN_REG_17
#define AARCH64_REG_IDX_x18	AARCH64_INSN_REG_18
#define AARCH64_REG_IDX_x19	AARCH64_INSN_REG_19
#define AARCH64_REG_IDX_x20	AARCH64_INSN_REG_20
#define AARCH64_REG_IDX_x21	AARCH64_INSN_REG_21
#define AARCH64_REG_IDX_x22	AARCH64_INSN_REG_22
#define AARCH64_REG_IDX_x23	AARCH64_INSN_REG_23
#define AARCH64_REG_IDX_x24	AARCH64_INSN_REG_24
#define AARCH64_REG_IDX_x25	AARCH64_INSN_REG_25
#define AARCH64_REG_IDX_x26	AARCH64_INSN_REG_26
#define AARCH64_REG_IDX_x27	AARCH64_INSN_REG_27
#define AARCH64_REG_IDX_x28	AARCH64_INSN_REG_28
#define AARCH64_REG_IDX_x29	AARCH64_INSN_REG_29
#define AARCH64_REG_IDX_x30	AARCH64_INSN_REG_30
#define AARCH64_REG_IDX_xzr	AARCH64_INSN_REG_ZR

#define AARCH64_REG_IDX_sp	AARCH64_INSN_REG_SP

#define AARCH64_REG_IDX_w0	AARCH64_INSN_REG_0
#define AARCH64_REG_IDX_w1	AARCH64_INSN_REG_1
#define AARCH64_REG_IDX_w2	AARCH64_INSN_REG_2
#define AARCH64_REG_IDX_w3	AARCH64_INSN_REG_3
#define AARCH64_REG_IDX_w4	AARCH64_INSN_REG_4
#define AARCH64_REG_IDX_w5	AARCH64_INSN_REG_5
#define AARCH64_REG_IDX_w6	AARCH64_INSN_REG_6
#define AARCH64_REG_IDX_w7	AARCH64_INSN_REG_7
#define AARCH64_REG_IDX_w8	AARCH64_INSN_REG_8
#define AARCH64_REG_IDX_w9	AARCH64_INSN_REG_9
#define AARCH64_REG_IDX_w10	AARCH64_INSN_REG_10
#define AARCH64_REG_IDX_w11	AARCH64_INSN_REG_11
#define AARCH64_REG_IDX_w12	AARCH64_INSN_REG_12
#define AARCH64_REG_IDX_w13	AARCH64_INSN_REG_13
#define AARCH64_REG_IDX_w14	AARCH64_INSN_REG_14
#define AARCH64_REG_IDX_w15	AARCH64_INSN_REG_15
#define AARCH64_REG_IDX_w16	AARCH64_INSN_REG_16
#define AARCH64_REG_IDX_w17	AARCH64_INSN_REG_17
#define AARCH64_REG_IDX_w18	AARCH64_INSN_REG_18
#define AARCH64_REG_IDX_w19	AARCH64_INSN_REG_19
#define AARCH64_REG_IDX_w20	AARCH64_INSN_REG_20
#define AARCH64_REG_IDX_w21	AARCH64_INSN_REG_21
#define AARCH64_REG_IDX_w22	AARCH64_INSN_REG_22
#define AARCH64_REG_IDX_w23	AARCH64_INSN_REG_23
#define AARCH64_REG_IDX_w24	AARCH64_INSN_REG_24
#define AARCH64_REG_IDX_w25	AARCH64_INSN_REG_25
#define AARCH64_REG_IDX_w26	AARCH64_INSN_REG_26
#define AARCH64_REG_IDX_w27	AARCH64_INSN_REG_27
#define AARCH64_REG_IDX_w28	AARCH64_INSN_REG_28
#define AARCH64_REG_IDX_w29	AARCH64_INSN_REG_29
#define AARCH64_REG_IDX_w30	AARCH64_INSN_REG_30
#define AARCH64_REG_IDX_wzr	AARCH64_INSN_REG_ZR

#define REG_IDX(r)	(AARCH64_REG_IDX_##r)

#define INSN_MSG(insn)							\
	KUNIT_SUBSUBTEST_INDENT "'" #insn "' is 0x%08x", insn

#define INSN_EXPECT_IS(test, type, insn)				\
	KUNIT_EXPECT_TRUE_MSG(test, aarch64_insn_is_##type(insn), 	\
			      INSN_MSG(insn))

#define INSN_EXPECT_IS_NOT(test, type, insn)				\
	KUNIT_EXPECT_FALSE_MSG(test, aarch64_insn_is_##type(insn), 	\
			       INSN_MSG(insn))

#define INSN_EXPECT_IMM_EQ(test, insn, immname, val, scale)				\
do {											\
	KUNIT_EXPECT_EQ_MSG(test,							\
			    val,							\
			    aarch64_insn_decode_scaled_##immname(insn, scale),		\
			    KUNIT_SUBSUBTEST_INDENT "'" #insn "' is 0x%08x\n"		\
			    KUNIT_SUBSUBTEST_INDENT "'" #scale "' is %lld\n",		\
			    insn, scale);						\
} while (0)

#define IMM_MSG(val, scale)								\
	KUNIT_SUBSUBTEST_INDENT "'" #val "' is %lld (0x%llx)\n"				\
	KUNIT_SUBSUBTEST_INDENT "'" #scale "' is %lld\n",				\
	val, val, scale

#define INSN_EXPECT_CAN_ENCODE_TRUE(test, name, val, scale)				\
	KUNIT_EXPECT_TRUE_MSG(test,							\
			      aarch64_insn_can_encode_scaled_##name(val, scale),	\
			      IMM_MSG(val, scale))

#define INSN_EXPECT_CAN_ENCODE_FALSE(test, name, val, scale)				\
	KUNIT_EXPECT_FALSE_MSG(test,							\
			       aarch64_insn_can_encode_scaled_##name(val, scale),	\
			       IMM_MSG(val, scale))

#define INSN_EXPECT_TRY_ENCODE_TRUE(test, insnp, name, val, scale)			\
	KUNIT_EXPECT_TRUE_MSG(test,							\
			      aarch64_insn_try_encode_scaled_##name(insnp, val, scale),	\
			      IMM_MSG(val, scale))

#define INSN_EXPECT_TRY_ENCODE_FALSE(test, insnp, name, val, scale)			\
	KUNIT_EXPECT_FALSE_MSG(test,							\
			       aarch64_insn_try_encode_scaled_##name(insnp, val, scale),\
			       IMM_MSG(val, scale))

#define INSN_EXPECT_REG_EQ(test, insn, regname, reg)					\
do {											\
	KUNIT_EXPECT_EQ_MSG(test,							\
			    reg,							\
			    aarch64_insn_decode_reg_##regname(insn),			\
			    INSN_MSG(insn));						\
} while (0)

#define INSN_EXPECT_TRY_ENCODE_REG_TRUE(test, insnp, regname, reg)			\
do {											\
	KUNIT_EXPECT_TRUE_MSG(test,							\
			      aarch64_insn_try_encode_reg_##regname(insnp, reg),	\
			      KUNIT_SUBSUBTEST_INDENT "'" #reg "' is 0x%x\n",		\
			      reg);							\
} while (0)

#define TEST_IMM_MIN_MAX(test, immname, min, max, scale)			\
do {										\
	INSN_EXPECT_CAN_ENCODE_TRUE(test, immname, min, scale);			\
	INSN_EXPECT_CAN_ENCODE_TRUE(test, immname, max, scale);			\
										\
	INSN_EXPECT_CAN_ENCODE_FALSE(test, immname, min - 1, scale);		\
	INSN_EXPECT_CAN_ENCODE_FALSE(test, immname, min - scale, scale);	\
										\
	INSN_EXPECT_CAN_ENCODE_FALSE(test, immname, max + 1, scale);		\
	INSN_EXPECT_CAN_ENCODE_FALSE(test, immname, max + scale, scale);	\
} while (0)

#define TEST_IMM_VALUE(test, immname, val, scale)				\
do {										\
	u32 insn = 0;								\
	INSN_EXPECT_CAN_ENCODE_TRUE(test, immname, val, scale);			\
	INSN_EXPECT_TRY_ENCODE_TRUE(test, &insn, immname, val, scale);		\
	INSN_EXPECT_IMM_EQ(test, insn, immname, val, scale);			\
} while (0)

#define TEST_IMM_RANGE(test, immname, _min, _max, _scale)			\
do {										\
	typeof(aarch64_insn_decode_##immname(0))				\
		min = _min,							\
		max = _max,							\
		scale = _scale;							\
										\
	TEST_IMM_MIN_MAX(test, immname, min, max, scale);			\
										\
	for (typeof(min) val = min; val < max; val += scale) {			\
		TEST_IMM_VALUE(test, immname, val, scale);			\
	}									\
} while (0)

#define TEST_UNSCALED_IMM_RANGE(test, immname, min, max)			\
	TEST_IMM_RANGE(test, immname, min, max, 1)

#define TEST_SCALED_IMM_RANGE(test, immname, scale, min, max)			\
	TEST_IMM_RANGE(test, immname,  min, max, scale)

#define TEST_IMM_MATCHES_LEGACY(test, bits, immname, immtype)			\
do {										\
	for (unsigned long val = 0;						\
	     val < BIT(bits);							\
	     val++) {								\
		u32 old = 0;							\
		u32 new = 0;							\
		old = aarch64_insn_encode_immediate(immtype, 0, val);		\
		INSN_EXPECT_TRY_ENCODE_TRUE(test, &new, immname, val, 1);	\
		INSN_EXPECT_IMM_EQ(test, old, immname, val, 1);			\
		INSN_EXPECT_IMM_EQ(test, new, immname, val, 1);			\
	}									\
} while (0)

#define TEST_IMM_CASE(test, asm_insn, immname, gen_imm, scale)			\
do {										\
	u32 obj_insn = ASM_U32(asm_insn, [imm] "i" (gen_imm));			\
	INSN_EXPECT_IMM_EQ(test, obj_insn, immname, gen_imm, scale);		\
} while (0)

#define TEST_UNSCALED_IMM_CASE(test, immname, asm_insn, gen_imm)		\
	TEST_IMM_CASE(test, asm_insn, immname, gen_imm, 1)

#define TEST_SCALED_IMM_CASE(test, immname, scale, asm_insn, gen_imm)		\
	TEST_IMM_CASE(test, asm_insn, immname, gen_imm, scale)

static void test_imm_adr(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 19,
				unsigned_adr_imm, AARCH64_INSN_IMM_ADR);

	/*
	 * As used by ADR
	 */
	TEST_UNSCALED_IMM_RANGE(test, signed_adr_imm,
				-SZ_1M, SZ_1M - 1);

	TEST_UNSCALED_IMM_CASE(test, signed_adr_imm,
			       "adr x0, . + %[imm]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, signed_adr_imm,
			       "adr x0, . + %[imm]",
			       257);

	TEST_UNSCALED_IMM_CASE(test, signed_adr_imm,
			       "adr x0, . + %[imm]",
			       -43);

	/*
	 * As used by ADRP
	 *
	 * See TEST_ADRP_CASE() for why we use a local labels for these tests.
	 */
	TEST_SCALED_IMM_RANGE(test, signed_adr_imm, SZ_4K,
			      -SZ_4G, SZ_4G - SZ_4K);

	TEST_SCALED_IMM_CASE(test, signed_adr_imm, SZ_4K,
			     "1: adrp x0, 1b + %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_adr_imm, SZ_4K,
			     "1: adrp x0, 1b + %[imm]",
			     8192);

	TEST_SCALED_IMM_CASE(test, signed_adr_imm, SZ_4K,
			     "1: adrp x0, 1b + %[imm]",
			     -16384);
}

static void test_imm_b50(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by TBZ
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_b50,
				0, 63);

	TEST_UNSCALED_IMM_CASE(test, unsigned_b50,
			       "tbz x0, %[imm], .",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_b50,
			       "tbz x0, %[imm], .",
			       22);

	TEST_UNSCALED_IMM_CASE(test, unsigned_b50,
			       "tbz x0, %[imm], .",
			       47);
}

static void test_imm_imm26(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 26,
				unsigned_imm26, AARCH64_INSN_IMM_26);

	/*
	 * As used by Branch (immediate)
	 */
	TEST_SCALED_IMM_RANGE(test, signed_imm26, 4,
			      -SZ_128M, SZ_128M - 4);

	TEST_SCALED_IMM_CASE(test, signed_imm26, 4,
			     "b . + %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_imm26, 4,
			     "b . + %[imm]",
			     36);

	TEST_SCALED_IMM_CASE(test, signed_imm26, 4,
			     "b . + %[imm]",
			     -200);
}

static void test_imm_imm19(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 19,
				unsigned_imm19, AARCH64_INSN_IMM_19);

	/*
	 * As used by LDR (literal)
	 */
	TEST_SCALED_IMM_RANGE(test, signed_imm19, 4,
			      -SZ_1M, SZ_1M - 4);

	TEST_SCALED_IMM_CASE(test, signed_imm19, 4,
			     "ldr x0, . + %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_imm19, 4,
			     "ldr x0, . + %[imm]",
			     208);

	TEST_SCALED_IMM_CASE(test, signed_imm19, 4,
			     "ldr x0, . + %[imm]",
			     -16384);
}

static void test_imm_imm16(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 16,
				unsigned_imm16, AARCH64_INSN_IMM_16);

	/*
	 * As used by SVC
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_imm16,
				0, 65535);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm16,
			       "svc %[imm]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm16,
			       "svc %[imm]",
			       425);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm16,
			       "svc %[imm]",
			       16384);
}

static void test_imm_imm14(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 14,
				unsigned_imm14, AARCH64_INSN_IMM_14);

	/*
	 * As used by TBZ
	 */
	TEST_SCALED_IMM_RANGE(test, signed_imm14, 4,
			      -SZ_32K, SZ_32K - 4);

	TEST_SCALED_IMM_CASE(test, signed_imm14, 4,
			     "tbz x0, 0, . + %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_imm14, 4,
			     "tbz x0, 0, . + %[imm]",
			     36);

	TEST_SCALED_IMM_CASE(test, signed_imm14, 4,
			     "tbz x0, 0, . + %[imm]",
			     -200);
}

static void test_imm_imm12(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 12,
				unsigned_imm12, AARCH64_INSN_IMM_12);

	/*
	 * As used by LDR (immediate, unsigned offset)
	 */
	TEST_SCALED_IMM_RANGE(test, unsigned_imm12, 8,
			      0, 32760);

	TEST_SCALED_IMM_RANGE(test, unsigned_imm12, 4,
			      0, 16380);

	TEST_SCALED_IMM_CASE(test, unsigned_imm12, 8,
			     "ldr x0, [x1, %[imm]]",
			     0);

	TEST_SCALED_IMM_CASE(test, unsigned_imm12, 8,
			     "ldr x0, [x1, %[imm]]",
			     24);

	TEST_SCALED_IMM_CASE(test, unsigned_imm12, 4,
			     "ldr w0, [x1, %[imm]]",
			     0);

	TEST_SCALED_IMM_CASE(test, unsigned_imm12, 4,
			     "ldr w0, [x1, %[imm]]",
			     12);
}

static void test_imm_imm9(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 9,
				unsigned_imm9, AARCH64_INSN_IMM_9);

	/*
	 * As used by LDR (immedate, pre-index)
	 */
	TEST_UNSCALED_IMM_RANGE(test, signed_imm9,
				-256, 255);

	TEST_UNSCALED_IMM_CASE(test, signed_imm9,
			       "ldr x0, [x1, %[imm]]!",
			       0);

	TEST_UNSCALED_IMM_CASE(test, signed_imm9,
			       "ldr x0, [x1, %[imm]]!",
			       13);

	TEST_UNSCALED_IMM_CASE(test, signed_imm9,
			       "ldr x0, [x1, %[imm]]!",
			       -37);
}

static void test_imm_imm7_15(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 7,
				unsigned_imm7_15, AARCH64_INSN_IMM_7);

	/*
	 * As used by LDP (64-bit)
	 */
	TEST_SCALED_IMM_RANGE(test, signed_imm7_15, 8,
			      -512, 504);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 8,
			     "ldp x0, x1, [x2, %[imm]]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 8,
			     "ldp x0, x1, [x2, %[imm]]",
			     40);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 8,
			     "ldp x0, x1, [x2, %[imm]]",
			     -56);

	/*
	 * As used by LDP (32-bit)
	 */
	TEST_SCALED_IMM_RANGE(test, signed_imm7_15, 4,
			      -256, 252);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 4,
			     "ldp w0, w1, [x2, %[imm]]",
			     0);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 4,
			     "ldp w0, w1, [x2, %[imm]]",
			     28);

	TEST_SCALED_IMM_CASE(test, signed_imm7_15, 4,
			     "ldp w0, w1, [x2, %[imm]]",
			     -60);
}

static void test_imm_imm6_10(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 6,
				unsigned_imm6_10, AARCH64_INSN_IMM_6);

	/*
	 * As used by ADD (shifted register)
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_imm6_10,
				0, 63);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm6_10,
			       "add x0, x1, x2, lsl %[imm]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm6_10,
			       "add x0, x1, x2, lsl %[imm]",
			       63);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm6_10,
			       "add x0, x1, x2, lsl %[imm]",
			       32);
}

static void test_imm_imm3_10(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by ADD (extended register)
	 * Note that ADD treats values 5-7 as UNALLOCATED
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_imm3_10,
				0, 7);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm3_10,
			       "add x0, x1, w2, sxtw %[imm]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm3_10,
			       "add x0, x1, w2, sxtw %[imm]",
			       3);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imm3_10,
			       "add x0, x1, w2, sxtw %[imm]",
			       4);
}

static void test_imm_immr(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 6,
				unsigned_immr, AARCH64_INSN_IMM_R);

	/*
	 * As used by SBFM
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_immr,
				0, 63);

	TEST_UNSCALED_IMM_CASE(test, unsigned_immr,
			       "sbfm x0, x1, %[imm], 0",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_immr,
			       "sbfm x0, x1, %[imm], 0",
			       27);

	TEST_UNSCALED_IMM_CASE(test, unsigned_immr,
			       "sbfm x0, x1, %[imm], 0",
			       53);
}

static void test_imm_imms(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 6,
				unsigned_imms, AARCH64_INSN_IMM_S);

	/*
	 * As used by SBFM
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_imms,
				0, 63);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imms,
			       "sbfm x0, x1, 0, %[imm]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imms,
			       "sbfm x0, x1, 0, %[imm]",
			       27);

	TEST_UNSCALED_IMM_CASE(test, unsigned_imms,
			       "sbfm x0, x1, 0, %[imm]",
			       53);
}

static void test_imm_N(struct kunit *test)
{
	TEST_IMM_MATCHES_LEGACY(test, 1,
				unsigned_N, AARCH64_INSN_IMM_N);

	/*
	 * The 'N' bit doesn't directly correspond to an assembly parameter for
	 * any instruction. It is typically used to encode bitmask immediates
	 * as part of 'N:imms:immr'.
	 *
	 * It is always set for 64-bit UBFM instructions, and always clear for
	 * 32-bit UBFM instructions.
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_N,
				0, 1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_N,
			       "ubfm x0, x1, #0, #0",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_N,
			       "ubfm w0, w1, #0, #0",
			       0);
}

static void test_imm_hw(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by MOVZ (64-bit)
	 */
	TEST_SCALED_IMM_RANGE(test, unsigned_hw, 16,
			      0, 48);

	TEST_SCALED_IMM_CASE(test, unsigned_hw, 16,
			     "movz x0, #0, lsl %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, unsigned_hw, 16,
			     "movz x0, #0, lsl %[imm]",
			     32);

	TEST_SCALED_IMM_CASE(test, unsigned_hw, 16,
			     "movz x0, #0, lsl %[imm]",
			     48);
}

static void test_imm_sf(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by MOV (register)
	 *
	 * The 'sf' bit doesn't directly correspond to an assembly immediate.
	 * It is always set for 64-bit instructions, and always clear for
	 * 32-bit instructions.
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_sf,
				0, 1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_sf,
			       "mov x0, x1",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_sf,
			       "mov w0, w1",
			       0);
}

static void test_imm_sh(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by ADD (immediate)
	 */
	TEST_SCALED_IMM_RANGE(test, unsigned_sh, 12,
			      0, 12);

	TEST_SCALED_IMM_CASE(test, unsigned_sh, 12,
			     "add x0, x0, #0, lsl %[imm]",
			     0);

	TEST_SCALED_IMM_CASE(test, unsigned_sh, 12,
			     "add x0, x0, #0, lsl %[imm]",
			     12);
}

static void test_imm_ldst_size(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by LDR (immediate)
	 *
	 * The 'size' field doesn't directly correspond to an assembly
	 * immediate. It is used to encode the size of the memory access
	 * (encoded as log2(size)).
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_ldst_size,
				0, 3);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_size,
			       "ldr x0, [x1]",
			       AARCH64_INSN_SIZE_64);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_size,
			       "ldr w0, [x1]",
			       AARCH64_INSN_SIZE_32);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_size,
			       "ldrh w0, [x1]",
			       AARCH64_INSN_SIZE_16);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_size,
			       "ldrb w0, [x1]",
			       AARCH64_INSN_SIZE_8);
}

static void test_imm_ldst_L(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by LDXR / STXR
	 *
	 * The 'L' field doesn't directly correspond to an assembly immediate.
	 * Its interpretation varies by instruction, but it is always set for
	 * LDXR variants and always clear for STXR variants.
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_ldst_L,
				0, 1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_L,
			       "ldxr x0, [x1]",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_L,
			       "ldxrb w2, [x3]",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_L,
			       "stxr w0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_L,
			       "stxr w3, x4, [x5]",
			       0);
}

static void test_imm_ldst_o0(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by LDXR/LDAXR/STXR/STLXR
	 *
	 * The 'o0' field doesn't directly correspond to an assembly immediate.
	 * It is used for some load/store encodings to indicate acquire/release
	 * ordering.
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_ldst_o0,
				0, 1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_o0,
			       "ldxr x0, [x1]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_o0,
			       "ldaxr x0, [x1]",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_o0,
			       "stxr w0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_ldst_o0,
			       "stlxr w0, x1, [x2]",
			       1);
}

static void test_imm_amo_a(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by SWP / SWPA / SWPAL / SWPL
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_amo_a,
				0, 1);

	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_a,
			       __LSE_PREAMBLE
			       "swp x0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_a,
			       __LSE_PREAMBLE
			       "swpa x0, x1, [x2]",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_a,
			       __LSE_PREAMBLE
			       "swpl x0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_a,
			       __LSE_PREAMBLE
			       "swpal x0, x1, [x2]",
			       1);
}

static void test_imm_amo_r(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by SWP / SWPA / SWPAL / SWPL
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_amo_r,
				0, 1);

	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_r,
			       __LSE_PREAMBLE
			       "swp x0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_r,
			       __LSE_PREAMBLE
			       "swpa x0, x1, [x2]",
			       0);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_r,
			       __LSE_PREAMBLE
			       "swpl x0, x1, [x2]",
			       1);

	TEST_UNSCALED_IMM_CASE(test, unsigned_amo_r,
			       __LSE_PREAMBLE
			       "swpal x0, x1, [x2]",
			       1);
}

static void test_imm_reg_shift(struct kunit *test)
{
	/* No legacy encoder exists */

	/*
	 * As used by AND (shifted register)
	 *
	 * The 'shift' field is used to encode the shift type to apply to a
	 * register argument: LSL / LSR / ASR / ROR
	 */
	TEST_UNSCALED_IMM_RANGE(test, unsigned_reg_shift,
				0, 3);

	TEST_UNSCALED_IMM_CASE(test, unsigned_reg_shift,
			       "and x0, x1, x2, lsl #1",
			       AARCH64_INSN_REG_SHIFT_LSL);

	TEST_UNSCALED_IMM_CASE(test, unsigned_reg_shift,
			       "and x0, x1, x2, lsr #1",
			       AARCH64_INSN_REG_SHIFT_LSR);

	TEST_UNSCALED_IMM_CASE(test, unsigned_reg_shift,
			       "and x0, x1, x2, asr #1",
			       AARCH64_INSN_REG_SHIFT_ASR);

	TEST_UNSCALED_IMM_CASE(test, unsigned_reg_shift,
			       "and x0, x1, x2, ror #1",
			       AARCH64_INSN_REG_SHIFT_ROR);
}

static struct kunit_case aarch64_insn_imm_test_cases[] = {
	KUNIT_CASE(test_imm_adr),
	KUNIT_CASE(test_imm_b50),
	KUNIT_CASE(test_imm_imm3_10),
	KUNIT_CASE(test_imm_imm6_10),
	KUNIT_CASE(test_imm_imm7_15),
	KUNIT_CASE(test_imm_imm9),
	KUNIT_CASE(test_imm_imm12),
	KUNIT_CASE(test_imm_imm14),
	KUNIT_CASE(test_imm_imm16),
	KUNIT_CASE(test_imm_imm19),
	KUNIT_CASE(test_imm_imm26),
	KUNIT_CASE(test_imm_immr),
	KUNIT_CASE(test_imm_imms),
	KUNIT_CASE(test_imm_N),
	KUNIT_CASE(test_imm_hw),
	KUNIT_CASE(test_imm_sf),
	KUNIT_CASE(test_imm_sh),
	KUNIT_CASE(test_imm_ldst_size),
	KUNIT_CASE(test_imm_ldst_L),
	KUNIT_CASE(test_imm_ldst_o0),
	KUNIT_CASE(test_imm_amo_a),
	KUNIT_CASE(test_imm_amo_r),
	KUNIT_CASE(test_imm_reg_shift),
	{ /* sentinel */ }
};

static struct kunit_suite test_aarch64_insn_imm_suite = {
	.name = "aarch64_insn_immediate",
	.test_cases = aarch64_insn_imm_test_cases,
};

#define TEST_REG_VALUE(test, regname, val)				\
do {									\
	u32 insn = 0;							\
	KUNIT_EXPECT_TRUE(test, aarch64_insn_reg_is_valid(reg));	\
	INSN_EXPECT_TRY_ENCODE_REG_TRUE(test, &insn, regname, val);	\
	KUNIT_EXPECT_EQ(test,						\
			val,						\
			aarch64_insn_decode_reg_##regname(insn));	\
} while (0)

#define TEST_REG_RANGE(test, regname)					\
do {									\
	enum aarch64_insn_register reg;					\
	for (reg = AARCH64_INSN_REG_0;					\
	     reg <= AARCH64_INSN_REG_SP;				\
	     reg++) {							\
		TEST_REG_VALUE(test, regname, reg);			\
	}								\
} while (0)

#define TEST_REG_MATCHES_LEGACY(test, regname, regtype)				\
do {										\
	enum aarch64_insn_register reg;						\
	for (reg = AARCH64_INSN_REG_0;						\
	     reg <= AARCH64_INSN_REG_SP;					\
	     reg++) {								\
		u32 old = 0;							\
		u32 new = 0;							\
		old = aarch64_insn_encode_register(regtype, 0, reg);		\
		INSN_EXPECT_TRY_ENCODE_REG_TRUE(test, &new, regname, reg);	\
		KUNIT_EXPECT_EQ(test, old, new);				\
		INSN_EXPECT_REG_EQ(test, old, regname, reg);			\
		INSN_EXPECT_REG_EQ(test, new, regname, reg);			\
	}									\
} while (0)

#define TEST_REG_CASE(test, regname, asm_insn, arg_reg)			\
do {									\
	enum aarch64_insn_register reg = REG_IDX(arg_reg);		\
	register long reg_var asm(#arg_reg) = 0;			\
	u32 obj_insn = ASM_U32(asm_insn, [reg] "r" (reg_var));		\
	INSN_EXPECT_REG_EQ(test, obj_insn, regname, reg);		\
} while (0)

static void test_reg_rt(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rt, AARCH64_INSN_REGTYPE_RT);

	TEST_REG_RANGE(test, rt);

	TEST_REG_CASE(test, rt,
		      "ldr %[reg], [x1]",
		      x5);

	TEST_REG_CASE(test, rt,
		      "ldr %[reg], [x1]",
		      x17);
}

static void test_reg_rd(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rd, AARCH64_INSN_REGTYPE_RD);

	TEST_REG_RANGE(test, rd);

	TEST_REG_CASE(test, rd,
		      "mov %[reg], x0",
		      x3);

	TEST_REG_CASE(test, rd,
		      "mov %[reg], x0",
		      x22);
}

static void test_reg_rn(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rn, AARCH64_INSN_REGTYPE_RN);

	TEST_REG_RANGE(test, rn);

	TEST_REG_CASE(test, rn,
		      "orr x0, %[reg], x0",
		      x9);

	TEST_REG_CASE(test, rn,
		      "orr x0, %[reg], x0",
		      x27);
}

static void test_reg_rt2(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rt2, AARCH64_INSN_REGTYPE_RT2);

	TEST_REG_RANGE(test, rt2);

	TEST_REG_CASE(test, rt2,
		      "ldp x0, %[reg], [x1]",
		      x14);

	TEST_REG_CASE(test, rt2,
		      "ldp x0, %[reg], [x1]",
		      x25);
}

static void test_reg_ra(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, ra, AARCH64_INSN_REGTYPE_RA);

	TEST_REG_RANGE(test, ra);

	TEST_REG_CASE(test, ra,
		      "madd x0, x1, x2, %[reg]",
		      x5);

	TEST_REG_CASE(test, ra,
		      "madd x0, x1, x2, %[reg]",
		      x26);
}

static void test_reg_rm(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rm, AARCH64_INSN_REGTYPE_RM);

	TEST_REG_RANGE(test, rm);

	TEST_REG_CASE(test, rm,
		      "orr x0, x1, %[reg]",
		      x6);

	TEST_REG_CASE(test, rm,
		      "orr x0, x1, %[reg]",
		      x19);
}

static void test_reg_rs(struct kunit *test)
{
	TEST_REG_MATCHES_LEGACY(test, rs, AARCH64_INSN_REGTYPE_RS);

	TEST_REG_RANGE(test, rs);

	TEST_REG_CASE(test, rs,
		      "stxr %w[reg], x0, [x1]",
		      x2);

	TEST_REG_CASE(test, rs,
		      "stxr %w[reg], x0, [x1]",
		      x16);
}

static struct kunit_case aarch64_insn_reg_test_cases[] = {
	KUNIT_CASE(test_reg_rt),
	KUNIT_CASE(test_reg_rd),
	KUNIT_CASE(test_reg_rn),
	KUNIT_CASE(test_reg_rt2),
	KUNIT_CASE(test_reg_ra),
	KUNIT_CASE(test_reg_rm),
	KUNIT_CASE(test_reg_rs),
	{ /* sentinel */ }
};

static struct kunit_suite test_aarch64_insn_reg_suite = {
	.name = "aarch64_insn_register",
	.test_cases = aarch64_insn_reg_test_cases,
};

#define TEST_ADR_CASE(test, arg_rd, arg_offset)						\
do {											\
	enum aarch64_insn_register rd = REG_IDX(arg_rd);				\
	s64 offset = (arg_offset);							\
											\
	u32 obj_insn = ASM_U32("adr " #arg_rd ", . + %0", "i" (offset));		\
	u32 gen_insn = aarch64_insn_gen_adr(0, offset, rd, AARCH64_INSN_ADR_TYPE_ADR);	\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, adr, obj_insn);						\
	INSN_EXPECT_IS(test, adr, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rd, rd);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rd, rd);					\
											\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_adr_imm, offset, 1);			\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_adr_imm, offset, 1);			\
} while (0)

struct test_insn_adr_adrp_params {
	enum aarch64_insn_adr_type type;
	enum aarch64_insn_register rd;
	s64 offset;

	bool fail;
	u32 insn;

	bool (*is)(u32);
	s64 (*get_offset)(u32);
};

static void test_insn_adr_adrp_case(struct kunit *test,
				    struct test_insn_adr_adrp_params *params)
{
	s64 offset;

	u32 insn = aarch64_insn_gen_adr(0, params->offset, params->rd,
					params->type);

	if (params->fail) {
		KUNIT_EXPECT_EQ(test, insn, AARCH64_BREAK_FAULT);
		return;
	}

	KUNIT_EXPECT_EQ(test, insn, params->insn);

	KUNIT_EXPECT_TRUE(test, params->is(insn));

	INSN_EXPECT_REG_EQ(test, insn, rd, params->rd);

	offset = params->get_offset(insn);
	KUNIT_EXPECT_EQ(test, offset, params->offset);
}

#define __CASE_ADR(_rd, _offset, _fail, _insn)				\
	{								\
		.type = AARCH64_INSN_ADR_TYPE_ADR,			\
		.is = aarch64_insn_is_adr,				\
		.get_offset = aarch64_insn_adr_get_offset,		\
		.rd = REG_IDX(_rd),					\
		.offset = (_offset),					\
		.fail = (_fail),					\
		.insn = (_insn),					\
	}

#define CASE_ADR_GOOD(_rd, _offset)					\
	__CASE_ADR(_rd, _offset, false,					\
		   ASM_U32("adr " #_rd ", %0", "i" (_offset)))

#define CASE_ADR_FAIL(_rd, _offset)					\
	__CASE_ADR(_rd, _offset, true, 0)

static void test_insn_adr(struct kunit *test)
{
	struct test_insn_adr_adrp_params params[] = {
		CASE_ADR_GOOD(x0, 0),
		CASE_ADR_GOOD(x1, 0),
		CASE_ADR_GOOD(x2, 257),
		CASE_ADR_GOOD(x3, -923),
		CASE_ADR_GOOD(x4, SZ_512K),
		CASE_ADR_GOOD(x5, -SZ_512K),
		CASE_ADR_GOOD(x6, SZ_1M - 1),
		CASE_ADR_GOOD(x7, -SZ_1M),

		/* out-of-range immediates */
		CASE_ADR_FAIL(x0, SZ_1M),
		CASE_ADR_FAIL(x0, -SZ_1M - 1),
	};

	for (int i = 0; i < ARRAY_SIZE(params); i++) {
		test_insn_adr_adrp_case(test, &params[i]);
	}
}

/*
 * For the asm, Ideally we'd use ". + %[imm]", but unfortuantely this or
 * ":pghi_21_nc: . + %[imm]" causes binutils 2.38 to generate unexpected ELF
 * notes, causing the build to fail with warnings of the form:
 *
 * | `.note.gnu.property' referenced in section `.text' of
 * | arch/arm64/lib/test_insn.o: defined in discarded section
 * | `.note.gnu.property' of arch/arm64/lib/test_insn.o
 *
 * Using a local label happens to inhibit this, and is functionally equivalent.
 */
#define CASE_ADRP(_rd, _offset, _fail, _insn)				\
	{								\
		.type = AARCH64_INSN_ADR_TYPE_ADRP,			\
		.is = aarch64_insn_is_adrp,				\
		.get_offset = aarch64_insn_adrp_get_offset,		\
		.rd = REG_IDX(_rd),					\
		.offset = (_offset),					\
		.fail = (_fail),					\
		.insn = (_insn),					\
	}

#define CASE_ADRP_GOOD(_rd, _offset)					\
	CASE_ADRP(_rd, _offset, false,					\
		  ASM_U32("1: adrp " #_rd ", 1b + %0",			\
			  "i" ((s64)(_offset))))

#define CASE_ADRP_FAIL(_rd, _offset)					\
	CASE_ADRP(_rd, _offset, true, 0)

static void test_insn_adrp(struct kunit *test)
{
	struct test_insn_adr_adrp_params params[] = {
		CASE_ADRP_GOOD(x0,  0),
		CASE_ADRP_GOOD(x1,  0),
		CASE_ADRP_GOOD(x2,  SZ_4K),
		CASE_ADRP_GOOD(x3,  -SZ_4K),
		CASE_ADRP_GOOD(x4,  SZ_1G),
		CASE_ADRP_GOOD(x5,  -SZ_1G),
		CASE_ADRP_GOOD(x6,  SZ_2G),
		CASE_ADRP_GOOD(x7,  -SZ_2G),
		CASE_ADRP_GOOD(x8,  SZ_2G + SZ_1G),
		CASE_ADRP_GOOD(x9,  -SZ_2G + -SZ_1G),
		CASE_ADRP_GOOD(x10, SZ_4G - SZ_4K),
		CASE_ADRP_GOOD(x11, -SZ_4G),

		/* out-of-range immediates */
		CASE_ADRP_FAIL(x0, SZ_4G),
		CASE_ADRP_FAIL(x0, -SZ_4G - SZ_4K),
	};

	for (int i = 0; i < ARRAY_SIZE(params); i++) {
		test_insn_adr_adrp_case(test, &params[i]);
	}
}

#define TEST_BRANCH_IMM_CASE(test, insn, arg_type, arg_offset)			\
do {										\
	enum aarch64_insn_branch_type type = (arg_type);			\
	s64 offset = (arg_offset);						\
										\
	u32 obj_insn = ASM_U32(#insn " . + %0", "i" (offset));			\
	u32 gen_insn = aarch64_insn_gen_branch_imm(0, offset, type);		\
										\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);				\
	INSN_EXPECT_IS(test, insn, obj_insn);					\
	INSN_EXPECT_IS(test, insn, gen_insn);					\
										\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm26, offset, 4);		\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm26, offset, 4);		\
} while (0)

static void test_insn_b(struct kunit *test)
{
	TEST_BRANCH_IMM_CASE(test, b, AARCH64_INSN_BRANCH_NOLINK, 0);
	TEST_BRANCH_IMM_CASE(test, b, AARCH64_INSN_BRANCH_NOLINK, 36);
	TEST_BRANCH_IMM_CASE(test, b, AARCH64_INSN_BRANCH_NOLINK, -24);
	TEST_BRANCH_IMM_CASE(test, b, AARCH64_INSN_BRANCH_NOLINK, SZ_128M - 4);
	TEST_BRANCH_IMM_CASE(test, b, AARCH64_INSN_BRANCH_NOLINK, -SZ_128M);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_branch_imm(0, SZ_128M,
						    AARCH64_INSN_BRANCH_NOLINK));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_branch_imm(0, -SZ_128M + -4,
						    AARCH64_INSN_BRANCH_NOLINK));
}

static void test_insn_bl(struct kunit *test)
{
	TEST_BRANCH_IMM_CASE(test, bl, AARCH64_INSN_BRANCH_LINK, 0);
	TEST_BRANCH_IMM_CASE(test, bl, AARCH64_INSN_BRANCH_LINK, 36);
	TEST_BRANCH_IMM_CASE(test, bl, AARCH64_INSN_BRANCH_LINK, -24);
	TEST_BRANCH_IMM_CASE(test, bl, AARCH64_INSN_BRANCH_LINK, SZ_128M - 4);
	TEST_BRANCH_IMM_CASE(test, bl, AARCH64_INSN_BRANCH_LINK, -SZ_128M);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_branch_imm(0, SZ_128M,
						    AARCH64_INSN_BRANCH_LINK));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_branch_imm(0, -SZ_128M + -4,
						    AARCH64_INSN_BRANCH_LINK));
}

#define TEST_BCOND_CASE(test, asm_cond, arg_cond, arg_offset)			\
do {										\
	enum aarch64_insn_condition cond = (arg_cond);				\
	s64 offset = (arg_offset);						\
										\
	u32 obj_insn = ASM_U32("b." asm_cond " . + %0", "i" (offset));		\
	u32 gen_insn = aarch64_insn_gen_cond_branch_imm(0, offset, cond);	\
										\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);				\
	INSN_EXPECT_IS(test, bcond, obj_insn);					\
	INSN_EXPECT_IS(test, bcond, gen_insn);					\
										\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm19, offset, 4);		\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm19, offset, 4);		\
} while (0)

static void test_insn_bcond(struct kunit *test)
{
	TEST_BCOND_CASE(test, "eq", AARCH64_INSN_COND_EQ, 0);
	TEST_BCOND_CASE(test, "ne", AARCH64_INSN_COND_NE, 0);
	TEST_BCOND_CASE(test, "mi", AARCH64_INSN_COND_MI, 32);
	TEST_BCOND_CASE(test, "hi", AARCH64_INSN_COND_HI, -56);
	TEST_BCOND_CASE(test, "gt", AARCH64_INSN_COND_GT, SZ_1M - 4);
	TEST_BCOND_CASE(test, "al", AARCH64_INSN_COND_AL, -SZ_1M);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_cond_branch_imm(0, SZ_1M,
							 AARCH64_INSN_COND_EQ));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_cond_branch_imm(0, -SZ_1M + -4,
							 AARCH64_INSN_COND_EQ));
}

#define TEST_COMP_BRANCH_CASE(test, insn, arg_rt, arg_offset, arg_variant, arg_type)	\
do {											\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	s64 offset = (arg_offset);							\
	enum aarch64_insn_variant variant = (arg_variant);				\
	enum aarch64_insn_branch_type type = (arg_type);				\
											\
	u32 obj_insn = ASM_U32(#insn " " #arg_rt ", %0", "i" (offset));			\
	u32 gen_insn = aarch64_insn_gen_comp_branch_imm(0, offset, rt, variant, type);	\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, insn, obj_insn);						\
	INSN_EXPECT_IS(test, insn, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm19, offset, 4);			\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm19, offset, 4);			\
} while (0)

static void test_insn_cbnz(struct kunit *test)
{
	TEST_COMP_BRANCH_CASE(test,
			      cbnz, x0, 0,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_NONZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbnz, x1, 56,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_NONZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbnz, x2, -200,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_NONZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbnz, w0, 64,
			      AARCH64_INSN_VARIANT_32BIT,
			      AARCH64_INSN_BRANCH_COMP_NONZERO);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_comp_branch_imm(0, SZ_1M,
							 AARCH64_INSN_REG_0,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_BRANCH_COMP_NONZERO));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_comp_branch_imm(0, -SZ_1M + -1,
							 AARCH64_INSN_REG_0,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_BRANCH_COMP_NONZERO));
}

static void test_insn_cbz(struct kunit *test)
{
	TEST_COMP_BRANCH_CASE(test,
			      cbz, x0, 0,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_ZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbz, x1, 56,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_ZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbz, x2, -200,
			      AARCH64_INSN_VARIANT_64BIT,
			      AARCH64_INSN_BRANCH_COMP_ZERO);

	TEST_COMP_BRANCH_CASE(test,
			      cbz, w0, 64,
			      AARCH64_INSN_VARIANT_32BIT,
			      AARCH64_INSN_BRANCH_COMP_ZERO);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_comp_branch_imm(0, SZ_1M,
							 AARCH64_INSN_REG_0,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_BRANCH_COMP_ZERO));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_comp_branch_imm(0, -SZ_1M + -1,
							 AARCH64_INSN_REG_0,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_BRANCH_COMP_ZERO));
}

#define TEST_LDST_REG_CASE(test, class, insn, arg_rt, arg_rn, arg_rm, arg_size,		\
			   arg_type)							\
do {											\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);				\
	enum aarch64_insn_register rm = REG_IDX(arg_rm);				\
	enum aarch64_insn_size_type size = (arg_size);					\
	enum aarch64_insn_ldst_type type = (arg_type);					\
											\
	u32 obj_insn = ASM_U32(#insn " " #arg_rt ", [" #arg_rn "," #arg_rm "]");	\
	u32 gen_insn = aarch64_insn_gen_load_store_reg(rt, rn, rm, size, type);		\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, class, obj_insn);						\
	INSN_EXPECT_IS(test, class, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rm, rm);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rm, rm);					\
} while (0)

static void test_insn_ldr_reg(struct kunit *test)
{
	TEST_LDST_REG_CASE(test, ldr_reg,
			   ldr, x0, x1, x2,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_LOAD_REG_OFFSET);

	TEST_LDST_REG_CASE(test, ldr_reg,
			   ldr, w3, x4, x5,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_LOAD_REG_OFFSET);

	TEST_LDST_REG_CASE(test, ldr_reg,
			   ldrh, w6, x7, x8,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_LOAD_REG_OFFSET);

	TEST_LDST_REG_CASE(test, ldr_reg,
			   ldrb, w9, x10, x11,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_LOAD_REG_OFFSET);
}

static void test_insn_str_reg(struct kunit *test)
{
	TEST_LDST_REG_CASE(test, str_reg,
			   str, x0, x1, x2,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_STORE_REG_OFFSET);

	TEST_LDST_REG_CASE(test, str_reg,
			   str, w3, x4, x5,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_STORE_REG_OFFSET);

	TEST_LDST_REG_CASE(test, str_reg,
			   strh, w6, x7, x8,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_STORE_REG_OFFSET);

	TEST_LDST_REG_CASE(test, str_reg,
			   strb, w9, x10, x11,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_STORE_REG_OFFSET);
}

#define TEST_LDST_IMM_CASE(test, class, insn, arg_rt, arg_rn, arg_offset,		\
			   arg_size, arg_type)						\
do {											\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);				\
	s64 offset = (arg_offset);							\
	enum aarch64_insn_size_type size = (arg_size);					\
	enum aarch64_insn_ldst_type type = (arg_type);					\
	int scale = 1 << size;								\
											\
	u32 obj_insn = ASM_U32(#insn " " #arg_rt ", [" #arg_rn ", %0]",	"i" (offset));	\
	u32 gen_insn = aarch64_insn_gen_load_store_imm(rt, rn, offset, size, type);	\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, class, obj_insn);						\
	INSN_EXPECT_IS(test, class, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);					\
											\
	INSN_EXPECT_IMM_EQ(test, obj_insn, unsigned_imm12, offset, scale);		\
	INSN_EXPECT_IMM_EQ(test, gen_insn, unsigned_imm12, offset, scale);		\
} while (0)

static void test_insn_ldr_imm(struct kunit *test)
{
	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldr, x0, x1, 0,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldr, x3, x4, 24,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldr, w0, x1, 0,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldr, w3, x4, 28,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldrh, w0, x1, 0,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldrh, w3, x4, 26,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldrb, w0, x1, 0,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, ldr_imm,
			   ldrb, w3, x4, 27,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_LOAD_IMM_OFFSET);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							32768,
							AARCH64_INSN_SIZE_64,
							AARCH64_INSN_LDST_LOAD_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							16384,
							AARCH64_INSN_SIZE_32,
							AARCH64_INSN_LDST_LOAD_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							8192,
							AARCH64_INSN_SIZE_16,
							AARCH64_INSN_LDST_LOAD_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							4096,
							AARCH64_INSN_SIZE_8,
							AARCH64_INSN_LDST_LOAD_IMM_OFFSET));
}

static void test_insn_str_imm(struct kunit *test)
{
	TEST_LDST_IMM_CASE(test, str_imm,
			   str, x0, x1, 0,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   str, x3, x4, 24,
			   AARCH64_INSN_SIZE_64,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   str, w0, x1, 0,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   str, w3, x4, 28,
			   AARCH64_INSN_SIZE_32,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   strh, w0, x1, 0,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   strh, w3, x4, 26,
			   AARCH64_INSN_SIZE_16,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   strb, w0, x1, 0,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	TEST_LDST_IMM_CASE(test, str_imm,
			   strb, w3, x4, 27,
			   AARCH64_INSN_SIZE_8,
			   AARCH64_INSN_LDST_STORE_IMM_OFFSET);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							32768,
							AARCH64_INSN_SIZE_64,
							AARCH64_INSN_LDST_STORE_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							16384,
							AARCH64_INSN_SIZE_32,
							AARCH64_INSN_LDST_STORE_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							8192,
							AARCH64_INSN_SIZE_16,
							AARCH64_INSN_LDST_STORE_IMM_OFFSET));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_imm(AARCH64_INSN_REG_0,
							AARCH64_INSN_REG_1,
							4096,
							AARCH64_INSN_SIZE_8,
							AARCH64_INSN_LDST_STORE_IMM_OFFSET));
}

#define TEST_LDR_LIT_CASE(test, arg_rt, is64, arg_offset)				\
do {											\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	s64 offset = (arg_offset);							\
											\
	u32 obj_insn = ASM_U32("ldr " #arg_rt ", . + %0", "i" (offset));		\
	u32 gen_insn = aarch64_insn_gen_load_literal(0, offset, rt, is64);		\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, ldr_lit, obj_insn);					\
	INSN_EXPECT_IS(test, ldr_lit, gen_insn);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm19, offset, 4);			\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm19, offset, 4);			\
} while (0)

static void test_insn_ldr_lit(struct kunit *test)
{
	TEST_LDR_LIT_CASE(test, x0, true, 0);
	TEST_LDR_LIT_CASE(test, x1, true, 0);
	TEST_LDR_LIT_CASE(test, x2, true, 16);
	TEST_LDR_LIT_CASE(test, x3, true, -200);
	TEST_LDR_LIT_CASE(test, w4, false, 0);
	TEST_LDR_LIT_CASE(test, w5, false, 32);
	TEST_LDR_LIT_CASE(test, w6, false, -96);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_literal(0, SZ_1M,
						      AARCH64_INSN_REG_0, true));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_literal(0, -SZ_1M - 4,
						      AARCH64_INSN_REG_0, true));
}

#define TEST_LDST_PRE_CASE(test, arg_insn, arg_rt, arg_rt2, arg_rn, arg_offset,			\
			   arg_variant, arg_type)						\
do {												\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);					\
	enum aarch64_insn_register rt2 = REG_IDX(arg_rt2);					\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);					\
	s64 offset = (arg_offset);								\
	enum aarch64_insn_variant variant = (arg_variant);					\
	enum aarch64_insn_ldst_type type = (arg_type);						\
	int scale = (variant == AARCH64_INSN_VARIANT_64BIT) ? 8 : 4;				\
												\
	u32 obj_insn = ASM_U32(#arg_insn " " #arg_rt ", " #arg_rt2 ", [" #arg_rn ", %0]!",	\
			       "i" (offset));							\
	u32 gen_insn = aarch64_insn_gen_load_store_pair(rt, rt2, rn, offset, variant, type);	\
												\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);						\
	INSN_EXPECT_IS(test, arg_insn##_pre, obj_insn);						\
	INSN_EXPECT_IS(test, arg_insn##_pre, gen_insn);						\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);						\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt2, rt2);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt2, rt2);						\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);						\
												\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm7_15, offset, scale);			\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm7_15, offset, scale);			\
} while (0)

void test_insn_ldp_pre(struct kunit *test)
{
	TEST_LDST_PRE_CASE(test,
			   ldp, x0, x1, x2, 0,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   ldp, x5, x19, x28, 48,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   ldp, x2, x7, x13, -56,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   ldp, w0, w1, x2, 0,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   ldp, w5, w19, x28, 12,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   ldp, w2, w7, x13, -52,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -520,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 512,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -260,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 256,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX));
}

void test_insn_stp_pre(struct kunit *test)
{
	TEST_LDST_PRE_CASE(test,
			   stp, x0, x1, x2, 0,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   stp, x5, x19, x28, 48,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   stp, x2, x7, x13, -56,
			   AARCH64_INSN_VARIANT_64BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   stp, w0, w1, x2, 0,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   stp, w5, w19, x28, 12,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	TEST_LDST_PRE_CASE(test,
			   stp, w2, w7, x13, -52,
			   AARCH64_INSN_VARIANT_32BIT,
			   AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -520,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 512,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -260,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 256,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX));
}

#define TEST_LDST_POST_CASE(test, arg_insn, arg_rt, arg_rt2, arg_rn, arg_offset,		\
			   arg_variant, arg_type)						\
do {												\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);					\
	enum aarch64_insn_register rt2 = REG_IDX(arg_rt2);					\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);					\
	s64 offset = (arg_offset);								\
	enum aarch64_insn_variant variant = (arg_variant);					\
	enum aarch64_insn_ldst_type type = (arg_type);						\
	int scale = (variant == AARCH64_INSN_VARIANT_64BIT) ? 8 : 4;				\
												\
	u32 obj_insn = ASM_U32(#arg_insn " " #arg_rt ", " #arg_rt2 ", [" #arg_rn "], %0",	\
			       "i" (offset));							\
	u32 gen_insn = aarch64_insn_gen_load_store_pair(rt, rt2, rn, offset, variant, type);	\
												\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);						\
	INSN_EXPECT_IS(test, arg_insn##_post, obj_insn);					\
	INSN_EXPECT_IS(test, arg_insn##_post, gen_insn);					\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);						\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt2, rt2);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt2, rt2);						\
												\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);						\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);						\
												\
	INSN_EXPECT_IMM_EQ(test, obj_insn, signed_imm7_15, offset, scale);			\
	INSN_EXPECT_IMM_EQ(test, gen_insn, signed_imm7_15, offset, scale);			\
} while (0)

void test_insn_ldp_post(struct kunit *test)
{
	TEST_LDST_POST_CASE(test, ldp, x0, x1, x2, 0,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, ldp, x5, x19, x28, 48,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, ldp, x2, x7, x13, -56,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, ldp, w0, w1, x2, 0,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, ldp, w5, w19, x28, 12,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, ldp, w2, w7, x13, -52,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -520,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 512,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -260,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 256,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX));
}

void test_insn_stp_post(struct kunit *test)
{
	TEST_LDST_POST_CASE(test, stp, x0, x1, x2, 0,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, stp, x5, x19, x28, 48,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, stp, x2, x7, x13, -56,
			  AARCH64_INSN_VARIANT_64BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, stp, w0, w1, x2, 0,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, stp, w5, w19, x28, 12,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	TEST_LDST_POST_CASE(test, stp, w2, w7, x13, -52,
			  AARCH64_INSN_VARIANT_32BIT,
			  AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX);

	/*
	 * Out-of-range immediates
	 */
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -520,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 512,
							 AARCH64_INSN_VARIANT_64BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 -260,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX));
	KUNIT_EXPECT_EQ(test,
			AARCH64_BREAK_FAULT,
			aarch64_insn_gen_load_store_pair(AARCH64_INSN_REG_0,
							 AARCH64_INSN_REG_1,
							 AARCH64_INSN_REG_2,
							 256,
							 AARCH64_INSN_VARIANT_32BIT,
							 AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX));
}

struct test_insn_exclusive_params {
	enum aarch64_insn_register rt, rn, rs;
	enum aarch64_insn_size_type size;
	enum aarch64_insn_ldst_type type;

	bool fail;
	u32 insn;

	bool (*is)(u32);
};

static void test_insn_exclusive_case(struct kunit *test,
				     struct test_insn_exclusive_params *params)
{
	u32 insn = aarch64_insn_gen_load_store_ex(params->rt, params->rn,
						  params->rs, params->size,
						  params->type);

	if (params->fail) {
		KUNIT_EXPECT_EQ(test, insn, AARCH64_BREAK_FAULT);
		return;
	}

	KUNIT_EXPECT_EQ(test, insn, params->insn);

	KUNIT_EXPECT_TRUE(test, params->is(insn));

	INSN_EXPECT_REG_EQ(test, insn, rt, params->rt);
	INSN_EXPECT_REG_EQ(test, insn, rt2, AARCH64_INSN_REG_ZR);
	INSN_EXPECT_REG_EQ(test, insn, rn, params->rn);
	INSN_EXPECT_REG_EQ(test, insn, rs, params->rs);
}

#define __CASE_EXCLUSIVE(_rt, _rs, _rn, _size, _type, _fail, _insn, _is)	\
	{									\
		.rt = REG_IDX(_rt),						\
		.rs = REG_IDX(_rs),						\
		.rn = REG_IDX(_rn),						\
		.size = AARCH64_INSN_SIZE_##_size,				\
		.type = AARCH64_INSN_LDST_##_type,				\
		.fail = (_fail),						\
		.insn = (_insn),						\
		.is = (_is),							\
	}

#define CASE_LDXR_GOOD(_asm, _rt, _rn, _size, _type)			\
	__CASE_EXCLUSIVE(_rt, wzr, _rn, _size, _type, false, 		\
			  ASM_U32(_asm), aarch64_insn_is_ldxr)

static void test_insn_ldxr(struct kunit *test)
{
	struct test_insn_exclusive_params params[] = {
		CASE_LDXR_GOOD("ldaxr  x0, [x1]", x0, x1, 64, LOAD_ACQ_EX),
		CASE_LDXR_GOOD("ldaxr  w0, [x1]", w0, x1, 32, LOAD_ACQ_EX),
		CASE_LDXR_GOOD("ldaxrh w0, [x1]", w0, x1, 16, LOAD_ACQ_EX),
		CASE_LDXR_GOOD("ldaxrb w0, [x1]", w0, x1, 8,  LOAD_ACQ_EX),

		CASE_LDXR_GOOD("ldxr   x0, [x1]", x0, x1, 64, LOAD_EX),
		CASE_LDXR_GOOD("ldxr   w0, [x1]", w0, x1, 32, LOAD_EX),
		CASE_LDXR_GOOD("ldxrh  w0, [x1]", w0, x1, 16, LOAD_EX),
		CASE_LDXR_GOOD("ldxrb  w0, [x1]", w0, x1, 8,  LOAD_EX),
	};

	for (int i = 0; i < ARRAY_SIZE(params); i++) {
		test_insn_exclusive_case(test, &params[i]);
	}

	/*
	 * Historically mis-idenfied as an exclusive by aarch64_insn_is_*()
	 */
	INSN_EXPECT_IS_NOT(test, ldxr, 0x08200000);
	INSN_EXPECT_IS_NOT(test, ldxr, 0x085f0000);
	INSN_EXPECT_IS_NOT(test, ldxr, 0x08407c00);
}

#define CASE_STXR_GOOD(_asm, _rs, _rt, _rn, _size, _type)		\
	__CASE_EXCLUSIVE(_rt, _rs, _rn, _size, _type, false, 		\
			  ASM_U32(_asm), aarch64_insn_is_stxr)

static void test_insn_stxr(struct kunit *test)
{
	struct test_insn_exclusive_params params[] = {
		CASE_STXR_GOOD("stlxr  w0, x1, [x2]", w0, x1, x2, 64, STORE_REL_EX),
		CASE_STXR_GOOD("stlxr  w0, w1, [x2]", w0, x1, x2, 32, STORE_REL_EX),
		CASE_STXR_GOOD("stlxrh w0, w1, [x2]", w0, x1, x2, 16, STORE_REL_EX),
		CASE_STXR_GOOD("stlxrb w0, w1, [x2]", w0, x1, x2, 8,  STORE_REL_EX),

		CASE_STXR_GOOD("stxr   w0, x1, [x2]", w0, x1, x2, 64, STORE_EX),
		CASE_STXR_GOOD("stxr   w0, w1, [x2]", w0, x1, x2, 32, STORE_EX),
		CASE_STXR_GOOD("stxrh  w0, w1, [x2]", w0, x1, x2, 16, STORE_EX),
		CASE_STXR_GOOD("stxrb  w0, w1, [x2]", w0, x1, x2, 8,  STORE_EX),
	};

	for (int i = 0; i < ARRAY_SIZE(params); i++) {
		test_insn_exclusive_case(test, &params[i]);
	}

	/*
	 * Historically mis-idenfied as an exclusive by aarch64_insn_is_*()
	 */
	INSN_EXPECT_IS_NOT(test, stxr, 0x08200000);
	INSN_EXPECT_IS_NOT(test, stxr, 0x08000000);
}

static void test_insn_ldxp(struct kunit *test)
{
	u32 insn = ASM_U32("ldxp x1, x2, [x3]");
	INSN_EXPECT_IS(test, ldxp, insn);

	/*
	 * Historically mis-idenfied as an exclusive by aarch64_insn_is_*()
	 */
	INSN_EXPECT_IS_NOT(test, ldxp, 0x08200000);
	INSN_EXPECT_IS_NOT(test, ldxp, 0x88200000);
	INSN_EXPECT_IS_NOT(test, ldxp, 0x88600000);
}

static void test_insn_stxp(struct kunit *test)
{
	u32 insn = ASM_U32("stxp w0, x1, x2, [x3]");
	INSN_EXPECT_IS(test, stxp, insn);

	/*
	 * Historically mis-idenfied as an exclusive by aarch64_insn_is_*()
	 */
	INSN_EXPECT_IS_NOT(test, stxp, 0x08200000);
}

#define TEST_LSE_LD_CASE(test, class, insn, arg_rs, arg_rt, arg_rn, arg_size,		\
			 arg_op, arg_order)						\
do {											\
	enum aarch64_insn_register rs = REG_IDX(arg_rs);				\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);				\
	enum aarch64_insn_size_type size = (arg_size);					\
	enum aarch64_insn_mem_atomic_op op = (arg_op);					\
	enum aarch64_insn_mem_order_type order = (arg_order);				\
											\
	u32 obj_insn = ASM_U32(#insn " " #arg_rs ", " #arg_rt ", [" #arg_rn "]");	\
	u32 gen_insn = aarch64_insn_gen_atomic_ld_op(rt, rn, rs, size, op, order);	\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, class, obj_insn);						\
	INSN_EXPECT_IS(test, class, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rs, rs);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rs, rs);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);					\
} while (0)

#define TEST_LSE_LD_ORDER_CASES(test, insn, xw, size, op)				\
do {											\
	TEST_LSE_LD_CASE(test, insn,							\
			 insn, xw##0, xw##1, x2,					\
			 size, op, AARCH64_INSN_MEM_ORDER_NONE);			\
											\
	TEST_LSE_LD_CASE(test, insn,							\
			 insn##a, xw##3, xw##4, x5,					\
			 size, op, AARCH64_INSN_MEM_ORDER_ACQ);				\
											\
	TEST_LSE_LD_CASE(test, insn,							\
			 insn##l, xw##6, xw##7, x8,					\
			 size, op, AARCH64_INSN_MEM_ORDER_REL);				\
											\
	TEST_LSE_LD_CASE(test, insn,							\
			 insn##al, xw##9, xw##10, x11,					\
			 size, op, AARCH64_INSN_MEM_ORDER_ACQREL);			\
} while (0)

#define TEST_LSE_LD_CASES(test, insn, op)						\
do {											\
	TEST_LSE_LD_ORDER_CASES(test, insn, x, AARCH64_INSN_SIZE_64, op);		\
	TEST_LSE_LD_ORDER_CASES(test, insn, w, AARCH64_INSN_SIZE_32, op);		\
} while (0)

static void test_insn_ldadd(struct kunit *test)
{
	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_LD_CASES(test, ldadd, AARCH64_INSN_MEM_ATOMIC_ADD);
}

static void test_insn_ldclr(struct kunit *test)
{
	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_LD_CASES(test, ldclr, AARCH64_INSN_MEM_ATOMIC_CLR);
}

static void test_insn_ldeor(struct kunit *test)
{
	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_LD_CASES(test, ldeor, AARCH64_INSN_MEM_ATOMIC_EOR);
}

static void test_insn_ldset(struct kunit *test)
{
	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_LD_CASES(test, ldset, AARCH64_INSN_MEM_ATOMIC_SET);
}

static void test_insn_swp(struct kunit *test)
{
	if (!IS_ENABLED(CONFIG_AS_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_LD_CASES(test, swp, AARCH64_INSN_MEM_ATOMIC_SWP);
}

#define TEST_LSE_CAS_CASE(test, insn, arg_rs, arg_rt, arg_rn, arg_size, arg_order)	\
do {											\
	enum aarch64_insn_register rs = REG_IDX(arg_rs);				\
	enum aarch64_insn_register rt = REG_IDX(arg_rt);				\
	enum aarch64_insn_register rn = REG_IDX(arg_rn);				\
	enum aarch64_insn_size_type size = (arg_size);					\
	enum aarch64_insn_mem_order_type order = (arg_order);				\
											\
	u32 obj_insn = ASM_U32(#insn " " #arg_rs ", " #arg_rt ", [" #arg_rn "]");	\
	u32 gen_insn = aarch64_insn_gen_cas(rt, rn, rs, size, order);			\
											\
	KUNIT_EXPECT_EQ(test, obj_insn, gen_insn);					\
	INSN_EXPECT_IS(test, cas, obj_insn);						\
	INSN_EXPECT_IS(test, cas, gen_insn);						\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rs, rs);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rs, rs);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rt, rt);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rt, rt);					\
											\
	INSN_EXPECT_REG_EQ(test, obj_insn, rn, rn);					\
	INSN_EXPECT_REG_EQ(test, gen_insn, rn, rn);					\
} while (0)

static void test_insn_cas(struct kunit *test)
{
	if (IS_ENABLED(CONFIG_CC_HAS_LSE_ATOMICS))
		kunit_skip(test, "Missing toolchain support for LSE atomics");

	TEST_LSE_CAS_CASE(test,
			  cas, x0, x1, x2,
			  AARCH64_INSN_SIZE_64,
			  AARCH64_INSN_MEM_ORDER_NONE);

	TEST_LSE_CAS_CASE(test,
			  casa, x1, x2, x3,
			  AARCH64_INSN_SIZE_64,
			  AARCH64_INSN_MEM_ORDER_ACQ);

	TEST_LSE_CAS_CASE(test,
			  casl, x4, x5, x6,
			  AARCH64_INSN_SIZE_64,
			  AARCH64_INSN_MEM_ORDER_REL);

	TEST_LSE_CAS_CASE(test,
			  casal, x7, x8, x9,
			  AARCH64_INSN_SIZE_64,
			  AARCH64_INSN_MEM_ORDER_ACQREL);

	TEST_LSE_CAS_CASE(test,
			  cas, w0, w1, x2,
			  AARCH64_INSN_SIZE_32,
			  AARCH64_INSN_MEM_ORDER_NONE);

	TEST_LSE_CAS_CASE(test,
			  casa, w1, w2, x3,
			  AARCH64_INSN_SIZE_32,
			  AARCH64_INSN_MEM_ORDER_ACQ);

	TEST_LSE_CAS_CASE(test,
			  casl, w4, w5, x6,
			  AARCH64_INSN_SIZE_32,
			  AARCH64_INSN_MEM_ORDER_REL);

	TEST_LSE_CAS_CASE(test,
			  casal, w7, w8, x9,
			  AARCH64_INSN_SIZE_32,
			  AARCH64_INSN_MEM_ORDER_ACQREL);
}

static struct kunit_case aarch64_insn_insn_test_cases[] = {
	KUNIT_CASE(test_insn_adr),
	KUNIT_CASE(test_insn_adrp),
	KUNIT_CASE(test_insn_b),
	KUNIT_CASE(test_insn_bl),
	KUNIT_CASE(test_insn_bcond),
	KUNIT_CASE(test_insn_cbnz),
	KUNIT_CASE(test_insn_cbz),
	KUNIT_CASE(test_insn_ldr_reg),
	KUNIT_CASE(test_insn_str_reg),
	KUNIT_CASE(test_insn_ldr_imm),
	KUNIT_CASE(test_insn_str_imm),
	KUNIT_CASE(test_insn_ldr_lit),
	KUNIT_CASE(test_insn_ldp_pre),
	KUNIT_CASE(test_insn_stp_pre),
	KUNIT_CASE(test_insn_ldp_post),
	KUNIT_CASE(test_insn_stp_post),
	KUNIT_CASE(test_insn_ldxr),
	KUNIT_CASE(test_insn_stxr),
	KUNIT_CASE(test_insn_ldxp),
	KUNIT_CASE(test_insn_stxp),
	KUNIT_CASE(test_insn_ldadd),
	KUNIT_CASE(test_insn_ldclr),
	KUNIT_CASE(test_insn_ldeor),
	KUNIT_CASE(test_insn_ldset),
	KUNIT_CASE(test_insn_swp),
	KUNIT_CASE(test_insn_cas),
	{ /* sentinel */ }
};

static struct kunit_suite test_aarch64_insn_insn_suite = {
	.name = "aarch64_insn_instruction",
	.test_cases = aarch64_insn_insn_test_cases,
};

kunit_test_suites(
	&test_aarch64_insn_imm_suite,
	&test_aarch64_insn_reg_suite,
	&test_aarch64_insn_insn_suite
);

MODULE_LICENSE("GPL");
