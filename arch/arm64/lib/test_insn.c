// SPDX-License-Identifier: GPL-2.0-only
/*
 * KUnit tests for arm64 instruction manipulation code.
 *
 * Copyright (C) 2022 Arm Ltd.
 */

#include <kunit/test.h>

#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/types.h>

#include <asm/byteorder.h>
#include <asm/insn.h>

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
	enum aarch64_insn_register rd;
	s64 offset;

	u32 insn = aarch64_insn_gen_adr(0, params->offset, params->rd,
					params->type);

	if (params->fail) {
		KUNIT_EXPECT_EQ(test, insn, AARCH64_BREAK_FAULT);
		return;
	}

	KUNIT_EXPECT_EQ(test, insn, params->insn);

	KUNIT_EXPECT_TRUE(test, params->is(insn));

	rd = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RD, insn);
	KUNIT_EXPECT_EQ(test, rd, params->rd);

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
	enum aarch64_insn_register rt, rt2, rn, rs;

	u32 insn = aarch64_insn_gen_load_store_ex(params->rt, params->rn,
						  params->rs, params->size,
						  params->type);

	if (params->fail) {
		KUNIT_EXPECT_EQ(test, insn, AARCH64_BREAK_FAULT);
		return;
	}

	KUNIT_EXPECT_EQ(test, insn, params->insn);

	KUNIT_EXPECT_TRUE(test, params->is(insn));

	rt = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RT, insn);
	KUNIT_EXPECT_EQ(test, rt, params->rt);

	rt2 = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RT2, insn);
	KUNIT_EXPECT_EQ(test, rt2, AARCH64_INSN_REG_ZR);

	rn = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RN, insn);
	KUNIT_EXPECT_EQ(test, rn, params->rn);

	rs = aarch64_insn_decode_register(AARCH64_INSN_REGTYPE_RS, insn);
	KUNIT_EXPECT_EQ(test, rs, params->rs);
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

static struct kunit_case aarch64_insn_insn_test_cases[] = {
	KUNIT_CASE(test_insn_adr),
	KUNIT_CASE(test_insn_adrp),
	KUNIT_CASE(test_insn_ldxr),
	KUNIT_CASE(test_insn_stxr),
	KUNIT_CASE(test_insn_ldxp),
	KUNIT_CASE(test_insn_stxp),
	{ /* sentinel */ }
};

static struct kunit_suite test_aarch64_insn_insn_suite = {
	.name = "aarch64_insn_instruction",
	.test_cases = aarch64_insn_insn_test_cases,
};

kunit_test_suites(
	&test_aarch64_insn_insn_suite
);

MODULE_LICENSE("GPL");
