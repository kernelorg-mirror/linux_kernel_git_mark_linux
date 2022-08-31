// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2013 Huawei Ltd.
 * Author: Jiang Liu <liuj97@gmail.com>
 *
 * Copyright (C) 2014-2016 Zi Shen Lim <zlim.lnx@gmail.com>
 */
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/printk.h>
#include <linux/sizes.h>
#include <linux/types.h>

#include <asm/debug-monitors.h>
#include <asm/errno.h>
#include <asm/insn.h>
#include <asm/kprobes.h>

static int __kprobes aarch64_get_imm_shift_mask(enum aarch64_insn_imm_type type,
						u32 *maskp, int *shiftp)
{
	u32 mask;
	int shift;

	switch (type) {
	case AARCH64_INSN_IMM_26:
		mask = BIT(26) - 1;
		shift = 0;
		break;
	case AARCH64_INSN_IMM_19:
		mask = BIT(19) - 1;
		shift = 5;
		break;
	case AARCH64_INSN_IMM_16:
		mask = BIT(16) - 1;
		shift = 5;
		break;
	case AARCH64_INSN_IMM_14:
		mask = BIT(14) - 1;
		shift = 5;
		break;
	case AARCH64_INSN_IMM_12:
		mask = BIT(12) - 1;
		shift = 10;
		break;
	case AARCH64_INSN_IMM_9:
		mask = BIT(9) - 1;
		shift = 12;
		break;
	case AARCH64_INSN_IMM_7:
		mask = BIT(7) - 1;
		shift = 15;
		break;
	case AARCH64_INSN_IMM_6:
	case AARCH64_INSN_IMM_S:
		mask = BIT(6) - 1;
		shift = 10;
		break;
	case AARCH64_INSN_IMM_R:
		mask = BIT(6) - 1;
		shift = 16;
		break;
	case AARCH64_INSN_IMM_N:
		mask = 1;
		shift = 22;
		break;
	default:
		return -EINVAL;
	}

	*maskp = mask;
	*shiftp = shift;

	return 0;
}

#define ADR_IMM_HILOSPLIT	2
#define ADR_IMM_SIZE		SZ_2M
#define ADR_IMM_LOMASK		((1 << ADR_IMM_HILOSPLIT) - 1)
#define ADR_IMM_HIMASK		((ADR_IMM_SIZE >> ADR_IMM_HILOSPLIT) - 1)
#define ADR_IMM_LOSHIFT		29
#define ADR_IMM_HISHIFT		5

u32 __kprobes aarch64_insn_encode_immediate(enum aarch64_insn_imm_type type,
				  u32 insn, u64 imm)
{
	u32 immlo, immhi, mask;
	int shift;

	if (insn == AARCH64_BREAK_FAULT)
		return AARCH64_BREAK_FAULT;

	switch (type) {
	case AARCH64_INSN_IMM_ADR:
		shift = 0;
		immlo = (imm & ADR_IMM_LOMASK) << ADR_IMM_LOSHIFT;
		imm >>= ADR_IMM_HILOSPLIT;
		immhi = (imm & ADR_IMM_HIMASK) << ADR_IMM_HISHIFT;
		imm = immlo | immhi;
		mask = ((ADR_IMM_LOMASK << ADR_IMM_LOSHIFT) |
			(ADR_IMM_HIMASK << ADR_IMM_HISHIFT));
		break;
	default:
		if (aarch64_get_imm_shift_mask(type, &mask, &shift) < 0) {
			pr_err("%s: unknown immediate encoding %d\n", __func__,
			       type);
			return AARCH64_BREAK_FAULT;
		}
	}

	/* Update the immediate field. */
	insn &= ~(mask << shift);
	insn |= (imm & mask) << shift;

	return insn;
}

u32 aarch64_insn_decode_register(enum aarch64_insn_register_type type,
					u32 insn)
{
	int shift;

	switch (type) {
	case AARCH64_INSN_REGTYPE_RT:
	case AARCH64_INSN_REGTYPE_RD:
		shift = 0;
		break;
	case AARCH64_INSN_REGTYPE_RN:
		shift = 5;
		break;
	case AARCH64_INSN_REGTYPE_RT2:
	case AARCH64_INSN_REGTYPE_RA:
		shift = 10;
		break;
	case AARCH64_INSN_REGTYPE_RM:
	case AARCH64_INSN_REGTYPE_RS:
		shift = 16;
		break;
	default:
		pr_err("%s: unknown register type encoding %d\n", __func__,
		       type);
		return 0;
	}

	return (insn >> shift) & GENMASK(4, 0);
}

u32 aarch64_insn_encode_register(enum aarch64_insn_register_type type,
				 u32 insn,
				 enum aarch64_insn_register reg)
{
	int shift;

	if (insn == AARCH64_BREAK_FAULT)
		return AARCH64_BREAK_FAULT;

	if (reg < AARCH64_INSN_REG_0 || reg > AARCH64_INSN_REG_SP) {
		pr_err("%s: unknown register encoding %d\n", __func__, reg);
		return AARCH64_BREAK_FAULT;
	}

	switch (type) {
	case AARCH64_INSN_REGTYPE_RT:
	case AARCH64_INSN_REGTYPE_RD:
		shift = 0;
		break;
	case AARCH64_INSN_REGTYPE_RN:
		shift = 5;
		break;
	case AARCH64_INSN_REGTYPE_RT2:
	case AARCH64_INSN_REGTYPE_RA:
		shift = 10;
		break;
	case AARCH64_INSN_REGTYPE_RM:
	case AARCH64_INSN_REGTYPE_RS:
		shift = 16;
		break;
	default:
		pr_err("%s: unknown register type encoding %d\n", __func__,
		       type);
		return AARCH64_BREAK_FAULT;
	}

	insn &= ~(GENMASK(4, 0) << shift);
	insn |= reg << shift;

	return insn;
}

u32 __kprobes aarch64_insn_gen_branch_imm(unsigned long pc, unsigned long addr,
					  enum aarch64_insn_branch_type type)
{
	u32 insn;
	long offset = addr - pc;

	switch (type) {
	case AARCH64_INSN_BRANCH_LINK:
		insn = aarch64_insn_get_bl_value();
		break;
	case AARCH64_INSN_BRANCH_NOLINK:
		insn = aarch64_insn_get_b_value();
		break;
	default:
		pr_err("%s: unknown branch encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_scaled_signed_imm26(&insn, offset, 4))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_comp_branch_imm(unsigned long pc, unsigned long addr,
				     enum aarch64_insn_register reg,
				     enum aarch64_insn_variant variant,
				     enum aarch64_insn_branch_type type)
{
	u32 insn;
	long offset = addr - pc;
	bool sf;

	switch (type) {
	case AARCH64_INSN_BRANCH_COMP_ZERO:
		insn = aarch64_insn_get_cbz_value();
		break;
	case AARCH64_INSN_BRANCH_COMP_NONZERO:
		insn = aarch64_insn_get_cbnz_value();
		break;
	default:
		pr_err("%s: unknown branch encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, reg) ||
	    !aarch64_insn_try_encode_scaled_signed_imm19(&insn, offset, 4))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_cond_branch_imm(unsigned long pc, unsigned long addr,
				     enum aarch64_insn_condition cond)
{
	u32 insn;
	long offset = addr - pc;

	insn = aarch64_insn_get_bcond_value();

	if (cond < AARCH64_INSN_COND_EQ || cond > AARCH64_INSN_COND_AL) {
		pr_err("%s: unknown condition encoding %d\n", __func__, cond);
		return AARCH64_BREAK_FAULT;
	}
	insn |= cond;

	if (!aarch64_insn_try_encode_scaled_signed_imm19(&insn, offset, 4))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_branch_reg(enum aarch64_insn_register reg,
				enum aarch64_insn_branch_type type)
{
	u32 insn;

	switch (type) {
	case AARCH64_INSN_BRANCH_NOLINK:
		insn = aarch64_insn_get_br_value();
		break;
	case AARCH64_INSN_BRANCH_LINK:
		insn = aarch64_insn_get_blr_value();
		break;
	case AARCH64_INSN_BRANCH_RETURN:
		insn = aarch64_insn_get_ret_value();
		break;
	default:
		pr_err("%s: unknown branch encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_reg_rn(&insn, reg))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_load_store_reg(enum aarch64_insn_register reg,
				    enum aarch64_insn_register base,
				    enum aarch64_insn_register offset,
				    enum aarch64_insn_size_type size,
				    enum aarch64_insn_ldst_type type)
{
	u32 insn;

	switch (type) {
	case AARCH64_INSN_LDST_LOAD_REG_OFFSET:
		insn = aarch64_insn_get_ldr_reg_value();
		break;
	case AARCH64_INSN_LDST_SIGNED_LOAD_REG_OFFSET:
		insn = aarch64_insn_get_signed_ldr_reg_value();
		break;
	case AARCH64_INSN_LDST_STORE_REG_OFFSET:
		insn = aarch64_insn_get_str_reg_value();
		break;
	default:
		pr_err("%s: unknown load/store encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_ldst_size(&insn, size) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, reg) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, base) ||
	    !aarch64_insn_try_encode_reg_rm(&insn, offset))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_load_store_imm(enum aarch64_insn_register reg,
				    enum aarch64_insn_register base,
				    unsigned int imm,
				    enum aarch64_insn_size_type size,
				    enum aarch64_insn_ldst_type type)
{
	u32 insn;

	switch (type) {
	case AARCH64_INSN_LDST_LOAD_IMM_OFFSET:
		insn = aarch64_insn_get_ldr_imm_value();
		break;
	case AARCH64_INSN_LDST_SIGNED_LOAD_IMM_OFFSET:
		insn = aarch64_insn_get_signed_load_imm_value();
		break;
	case AARCH64_INSN_LDST_STORE_IMM_OFFSET:
		insn = aarch64_insn_get_str_imm_value();
		break;
	default:
		pr_err("%s: unknown load/store encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_ldst_size(&insn, size) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, reg) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, base) ||
	    !aarch64_insn_try_encode_scaled_unsigned_imm12(&insn, imm, 1 << size))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_load_literal(unsigned long pc, unsigned long addr,
				  enum aarch64_insn_register reg,
				  bool is64bit)
{
	u32 insn = aarch64_insn_get_ldr_lit_value();
	s64 offset = addr - pc;

	if (is64bit)
		insn |= BIT(30);

	if (!aarch64_insn_try_encode_reg_rt(&insn, reg) ||
	    !aarch64_insn_try_encode_scaled_signed_imm19(&insn, offset, 4))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_load_store_pair(enum aarch64_insn_register reg1,
				     enum aarch64_insn_register reg2,
				     enum aarch64_insn_register base,
				     int offset,
				     enum aarch64_insn_variant variant,
				     enum aarch64_insn_ldst_type type)
{
	u32 insn;
	int scale;
	bool sf;

	switch (type) {
	case AARCH64_INSN_LDST_LOAD_PAIR_PRE_INDEX:
		insn = aarch64_insn_get_ldp_pre_value();
		break;
	case AARCH64_INSN_LDST_STORE_PAIR_PRE_INDEX:
		insn = aarch64_insn_get_stp_pre_value();
		break;
	case AARCH64_INSN_LDST_LOAD_PAIR_POST_INDEX:
		insn = aarch64_insn_get_ldp_post_value();
		break;
	case AARCH64_INSN_LDST_STORE_PAIR_POST_INDEX:
		insn = aarch64_insn_get_stp_post_value();
		break;
	default:
		pr_err("%s: unknown load/store encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		scale = 4;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		scale = 8;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, reg1) ||
	    !aarch64_insn_try_encode_reg_rt2(&insn, reg2) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, base) ||
	    !aarch64_insn_try_encode_scaled_signed_imm7_15(&insn, offset, scale))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_load_store_ex(enum aarch64_insn_register reg,
				   enum aarch64_insn_register base,
				   enum aarch64_insn_register state,
				   enum aarch64_insn_size_type size,
				   enum aarch64_insn_ldst_type type)
{
	u32 insn;
	bool order;

	switch (type) {
	case AARCH64_INSN_LDST_LOAD_EX:
	case AARCH64_INSN_LDST_LOAD_ACQ_EX:
		insn = aarch64_insn_get_ldxr_value();
		order = (type == AARCH64_INSN_LDST_LOAD_ACQ_EX);
		break;
	case AARCH64_INSN_LDST_STORE_EX:
	case AARCH64_INSN_LDST_STORE_REL_EX:
		insn = aarch64_insn_get_stxr_value();
		order = (type == AARCH64_INSN_LDST_STORE_REL_EX);
		break;
	default:
		pr_err("%s: unknown load/store exclusive encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_ldst_size(&insn, size) ||
	    !aarch64_insn_try_encode_unsigned_ldst_o0(&insn, order) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, reg) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, base) ||
	    !aarch64_insn_try_encode_reg_rt2(&insn, AARCH64_INSN_REG_ZR) ||
	    !aarch64_insn_try_encode_reg_rs(&insn, state))
		return AARCH64_BREAK_FAULT;

	return insn;
}

#ifdef CONFIG_ARM64_LSE_ATOMICS
u32 aarch64_insn_gen_atomic_ld_op(enum aarch64_insn_register result,
				  enum aarch64_insn_register address,
				  enum aarch64_insn_register value,
				  enum aarch64_insn_size_type size,
				  enum aarch64_insn_mem_atomic_op op,
				  enum aarch64_insn_mem_order_type order)
{
	u32 insn;
	bool acquire = false;
	bool release = false;

	switch (op) {
	case AARCH64_INSN_MEM_ATOMIC_ADD:
		insn = aarch64_insn_get_ldadd_value();
		break;
	case AARCH64_INSN_MEM_ATOMIC_CLR:
		insn = aarch64_insn_get_ldclr_value();
		break;
	case AARCH64_INSN_MEM_ATOMIC_EOR:
		insn = aarch64_insn_get_ldeor_value();
		break;
	case AARCH64_INSN_MEM_ATOMIC_SET:
		insn = aarch64_insn_get_ldset_value();
		break;
	case AARCH64_INSN_MEM_ATOMIC_SWP:
		insn = aarch64_insn_get_swp_value();
		break;
	default:
		pr_err("%s: unimplemented mem atomic op %d\n", __func__, op);
		return AARCH64_BREAK_FAULT;
	}

	switch (size) {
	case AARCH64_INSN_SIZE_32:
	case AARCH64_INSN_SIZE_64:
		break;
	default:
		pr_err("%s: unimplemented size encoding %d\n", __func__, size);
		return AARCH64_BREAK_FAULT;
	}

	switch (order) {
	case AARCH64_INSN_MEM_ORDER_NONE:
		break;
	case AARCH64_INSN_MEM_ORDER_ACQ:
		acquire = true;
		break;
	case AARCH64_INSN_MEM_ORDER_REL:
		release = true;
		break;
	case AARCH64_INSN_MEM_ORDER_ACQREL:
		acquire = true;
		release = true;
		break;
	default:
		pr_err("%s: unknown mem order %d\n", __func__, order);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_ldst_size(&insn, size) ||
	    !aarch64_insn_try_encode_unsigned_amo_a(&insn, acquire) ||
	    !aarch64_insn_try_encode_unsigned_amo_r(&insn, release) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, result) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, address) ||
	    !aarch64_insn_try_encode_reg_rs(&insn, value))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_cas(enum aarch64_insn_register result,
			 enum aarch64_insn_register address,
			 enum aarch64_insn_register value,
			 enum aarch64_insn_size_type size,
			 enum aarch64_insn_mem_order_type order)
{
	u32 insn = aarch64_insn_get_cas_value();
	bool acquire = false;
	bool release = false;

	switch (size) {
	case AARCH64_INSN_SIZE_32:
	case AARCH64_INSN_SIZE_64:
		break;
	default:
		pr_err("%s: unimplemented size encoding %d\n", __func__, size);
		return AARCH64_BREAK_FAULT;
	}

	switch (order) {
	case AARCH64_INSN_MEM_ORDER_NONE:
		break;
	case AARCH64_INSN_MEM_ORDER_ACQ:
		acquire = true;
		break;
	case AARCH64_INSN_MEM_ORDER_REL:
		release = true;
		break;
	case AARCH64_INSN_MEM_ORDER_ACQREL:
		acquire = true;
		release = true;
		break;
	default:
		pr_err("%s: unknown mem order %d\n", __func__, order);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_ldst_size(&insn, size) ||
	    !aarch64_insn_try_encode_unsigned_ldst_L(&insn, acquire) ||
	    !aarch64_insn_try_encode_unsigned_ldst_o0(&insn, release) ||
	    !aarch64_insn_try_encode_reg_rt(&insn, result) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, address) ||
	    !aarch64_insn_try_encode_reg_rs(&insn, value))
		return AARCH64_BREAK_FAULT;

	return insn;
}
#endif

u32 aarch64_insn_gen_add_sub_imm(enum aarch64_insn_register dst,
				 enum aarch64_insn_register src,
				 int imm, enum aarch64_insn_variant variant,
				 enum aarch64_insn_adsb_type type)
{
	u32 insn;
	bool sf;
	bool sh;

	switch (type) {
	case AARCH64_INSN_ADSB_ADD:
		insn = aarch64_insn_get_add_imm_value();
		break;
	case AARCH64_INSN_ADSB_SUB:
		insn = aarch64_insn_get_sub_imm_value();
		break;
	case AARCH64_INSN_ADSB_ADD_SETFLAGS:
		insn = aarch64_insn_get_adds_imm_value();
		break;
	case AARCH64_INSN_ADSB_SUB_SETFLAGS:
		insn = aarch64_insn_get_subs_imm_value();
		break;
	default:
		pr_err("%s: unknown add/sub encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!(imm & ~GENMASK(11, 0))) {
		sh = false;
	} else if (!(imm & ~GENMASK(23, 12))) {
		sh = true;
		imm >>= 12;
	} else {
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_unsigned_sh(&insn, sh) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, src) ||
	    !aarch64_insn_try_encode_unsigned_imm12(&insn, imm))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_bitfield(enum aarch64_insn_register dst,
			      enum aarch64_insn_register src,
			      int immr, int imms,
			      enum aarch64_insn_variant variant,
			      enum aarch64_insn_bitfield_type type)
{
	u32 insn;
	u32 mask;
	bool sf;

	switch (type) {
	case AARCH64_INSN_BITFIELD_MOVE:
		insn = aarch64_insn_get_bfm_value();
		break;
	case AARCH64_INSN_BITFIELD_MOVE_UNSIGNED:
		insn = aarch64_insn_get_ubfm_value();
		break;
	case AARCH64_INSN_BITFIELD_MOVE_SIGNED:
		insn = aarch64_insn_get_sbfm_value();
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		mask = GENMASK(4, 0);
		sf = false;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		mask = GENMASK(5, 0);
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	if (immr & ~mask)
		return AARCH64_BREAK_FAULT;
	if (imms & ~mask)
		return AARCH64_BREAK_FAULT;

	/*
	 * N == sf for all valid encodings
	 */
	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_unsigned_N(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, src) ||
	    !aarch64_insn_try_encode_unsigned_immr(&insn, immr) ||
	    !aarch64_insn_try_encode_unsigned_imms(&insn, imms))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_movewide(enum aarch64_insn_register dst,
			      int imm, int shift,
			      enum aarch64_insn_variant variant,
			      enum aarch64_insn_movewide_type type)
{
	u32 insn;
	bool sf;

	switch (type) {
	case AARCH64_INSN_MOVEWIDE_ZERO:
		insn = aarch64_insn_get_movz_value();
		break;
	case AARCH64_INSN_MOVEWIDE_KEEP:
		insn = aarch64_insn_get_movk_value();
		break;
	case AARCH64_INSN_MOVEWIDE_INVERSE:
		insn = aarch64_insn_get_movn_value();
		break;
	default:
		pr_err("%s: unknown movewide encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		if (shift > 16)
			return AARCH64_BREAK_FAULT;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		if (shift > 48)
			return AARCH64_BREAK_FAULT;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_scaled_unsigned_hw(&insn, shift, 16) ||
	    !aarch64_insn_try_encode_unsigned_imm16(&insn, imm))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_add_sub_shifted_reg(enum aarch64_insn_register dst,
					 enum aarch64_insn_register src,
					 enum aarch64_insn_register reg,
					 enum aarch64_insn_reg_shift_type shift_type,
					 int shift,
					 enum aarch64_insn_variant variant,
					 enum aarch64_insn_adsb_type type)
{
	u32 insn;
	bool sf;

	switch (type) {
	case AARCH64_INSN_ADSB_ADD:
		insn = aarch64_insn_get_add_value();
		break;
	case AARCH64_INSN_ADSB_SUB:
		insn = aarch64_insn_get_sub_value();
		break;
	case AARCH64_INSN_ADSB_ADD_SETFLAGS:
		insn = aarch64_insn_get_adds_value();
		break;
	case AARCH64_INSN_ADSB_SUB_SETFLAGS:
		insn = aarch64_insn_get_subs_value();
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	switch (shift_type) {
	case AARCH64_INSN_REG_SHIFT_LSL:
	case AARCH64_INSN_REG_SHIFT_LSR:
	case AARCH64_INSN_REG_SHIFT_ASR:
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		if (shift > 31)
			return AARCH64_BREAK_FAULT;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		if (shift > 63)
			return AARCH64_BREAK_FAULT;
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, src) ||
	    !aarch64_insn_try_encode_reg_rm(&insn, reg) ||
	    !aarch64_insn_try_encode_unsigned_reg_shift(&insn, shift_type) ||
	    !aarch64_insn_try_encode_unsigned_imm6_10(&insn, shift))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_data1(enum aarch64_insn_register dst,
			   enum aarch64_insn_register src,
			   enum aarch64_insn_variant variant,
			   enum aarch64_insn_data1_type type)
{
	u32 insn;
	bool sf;

	switch (type) {
	case AARCH64_INSN_DATA1_REVERSE_16:
		insn = aarch64_insn_get_rev16_value();
		break;
	case AARCH64_INSN_DATA1_REVERSE_32:
		insn = aarch64_insn_get_rev32_value();
		break;
	case AARCH64_INSN_DATA1_REVERSE_64:
		if (variant != AARCH64_INSN_VARIANT_64BIT) {
			pr_err("%s: invalid variant for reverse64 %d\n",
			       __func__, variant);
			return AARCH64_BREAK_FAULT;
		}
		insn = aarch64_insn_get_rev64_value();
		break;
	default:
		pr_err("%s: unknown data1 encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, src))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_data2(enum aarch64_insn_register dst,
			   enum aarch64_insn_register src,
			   enum aarch64_insn_register reg,
			   enum aarch64_insn_variant variant,
			   enum aarch64_insn_data2_type type)
{
	u32 insn;
	bool sf = false;

	switch (type) {
	case AARCH64_INSN_DATA2_UDIV:
		insn = aarch64_insn_get_udiv_value();
		break;
	case AARCH64_INSN_DATA2_SDIV:
		insn = aarch64_insn_get_sdiv_value();
		break;
	case AARCH64_INSN_DATA2_LSLV:
		insn = aarch64_insn_get_lslv_value();
		break;
	case AARCH64_INSN_DATA2_LSRV:
		insn = aarch64_insn_get_lsrv_value();
		break;
	case AARCH64_INSN_DATA2_ASRV:
		insn = aarch64_insn_get_asrv_value();
		break;
	case AARCH64_INSN_DATA2_RORV:
		insn = aarch64_insn_get_rorv_value();
		break;
	default:
		pr_err("%s: unknown data2 encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf))
		return AARCH64_BREAK_FAULT;

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RD, insn, dst);

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RN, insn, src);

	return aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RM, insn, reg);
}

u32 aarch64_insn_gen_data3(enum aarch64_insn_register dst,
			   enum aarch64_insn_register src,
			   enum aarch64_insn_register reg1,
			   enum aarch64_insn_register reg2,
			   enum aarch64_insn_variant variant,
			   enum aarch64_insn_data3_type type)
{
	u32 insn;
	bool sf = false;

	switch (type) {
	case AARCH64_INSN_DATA3_MADD:
		insn = aarch64_insn_get_madd_value();
		break;
	case AARCH64_INSN_DATA3_MSUB:
		insn = aarch64_insn_get_msub_value();
		break;
	default:
		pr_err("%s: unknown data3 encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf))
		return AARCH64_BREAK_FAULT;

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RD, insn, dst);

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RA, insn, src);

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RN, insn,
					    reg1);

	return aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RM, insn,
					    reg2);
}

u32 aarch64_insn_gen_logical_shifted_reg(enum aarch64_insn_register dst,
					 enum aarch64_insn_register src,
					 enum aarch64_insn_register reg,
					 enum aarch64_insn_reg_shift_type shift_type,
					 int shift,
					 enum aarch64_insn_variant variant,
					 enum aarch64_insn_logic_type type)
{
	u32 insn;
	bool sf;

	switch (type) {
	case AARCH64_INSN_LOGIC_AND:
		insn = aarch64_insn_get_and_value();
		break;
	case AARCH64_INSN_LOGIC_BIC:
		insn = aarch64_insn_get_bic_value();
		break;
	case AARCH64_INSN_LOGIC_ORR:
		insn = aarch64_insn_get_orr_value();
		break;
	case AARCH64_INSN_LOGIC_ORN:
		insn = aarch64_insn_get_orn_value();
		break;
	case AARCH64_INSN_LOGIC_EOR:
		insn = aarch64_insn_get_eor_value();
		break;
	case AARCH64_INSN_LOGIC_EON:
		insn = aarch64_insn_get_eon_value();
		break;
	case AARCH64_INSN_LOGIC_AND_SETFLAGS:
		insn = aarch64_insn_get_ands_value();
		break;
	case AARCH64_INSN_LOGIC_BIC_SETFLAGS:
		insn = aarch64_insn_get_bics_value();
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	switch (shift_type) {
	case AARCH64_INSN_REG_SHIFT_LSL:
	case AARCH64_INSN_REG_SHIFT_LSR:
	case AARCH64_INSN_REG_SHIFT_ASR:
	case AARCH64_INSN_REG_SHIFT_ROR:
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		if (shift > 31)
			return AARCH64_BREAK_FAULT;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		if (shift > 63)
			return AARCH64_BREAK_FAULT;
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, dst) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, src) ||
	    !aarch64_insn_try_encode_reg_rm(&insn, reg) ||
	    !aarch64_insn_try_encode_unsigned_reg_shift(&insn, shift_type) ||
	    !aarch64_insn_try_encode_unsigned_imm6_10(&insn, shift))
		return AARCH64_BREAK_FAULT;

	return insn;
}

/*
 * MOV (register) is architecturally an alias of ORR (shifted register) where
 * MOV <*d>, <*m> is equivalent to ORR <*d>, <*ZR>, <*m>
 */
u32 aarch64_insn_gen_move_reg(enum aarch64_insn_register dst,
			      enum aarch64_insn_register src,
			      enum aarch64_insn_variant variant)
{
	return aarch64_insn_gen_logical_shifted_reg(dst, AARCH64_INSN_REG_ZR, src,
						    AARCH64_INSN_REG_SHIFT_LSL, 0,
						    variant,
						    AARCH64_INSN_LOGIC_ORR);
}

u32 aarch64_insn_gen_adr(unsigned long pc, unsigned long addr,
			 enum aarch64_insn_register reg,
			 enum aarch64_insn_adr_type type)
{
	u32 insn;
	s64 offset;
	int scale;

	switch (type) {
	case AARCH64_INSN_ADR_TYPE_ADR:
		insn = aarch64_insn_get_adr_value();
		offset = addr - pc;
		scale = 1;
		break;
	case AARCH64_INSN_ADR_TYPE_ADRP:
		insn = aarch64_insn_get_adrp_value();
		offset = addr - ALIGN_DOWN(pc, SZ_4K);
		scale = SZ_4K;
		break;
	default:
		return AARCH64_BREAK_FAULT;
	}

	if (!aarch64_insn_try_encode_reg_rd(&insn, reg) ||
	    !aarch64_insn_try_encode_scaled_signed_adr_imm(&insn, offset, scale))
		return AARCH64_BREAK_FAULT;

	return insn;
}

/*
 * Decode the imm field of a branch, and return the byte offset as a
 * signed value (so it can be used when computing a new branch
 * target).
 */
s32 aarch64_get_branch_offset(u32 insn)
{
	if (aarch64_insn_is_b(insn) || aarch64_insn_is_bl(insn)) {
		return aarch64_insn_decode_scaled_signed_imm26(insn, 4);
	}

	if (aarch64_insn_is_cbz(insn) || aarch64_insn_is_cbnz(insn) ||
	    aarch64_insn_is_bcond(insn)) {
		return aarch64_insn_decode_scaled_signed_imm19(insn, 4);
	}

	if (aarch64_insn_is_tbz(insn) || aarch64_insn_is_tbnz(insn)) {
		return aarch64_insn_decode_scaled_signed_imm14(insn, 4);
	}

	/* Unhandled instruction */
	BUG();
}

/*
 * Encode the displacement of a branch in the imm field and return the
 * updated instruction.
 */
u32 aarch64_set_branch_offset(u32 insn, s32 offset)
{
	if (aarch64_insn_is_b(insn) || aarch64_insn_is_bl(insn)) {
		if (!aarch64_insn_try_encode_scaled_signed_imm26(&insn, offset, 4))
			return AARCH64_BREAK_FAULT;
		return insn;
	}

	if (aarch64_insn_is_cbz(insn) || aarch64_insn_is_cbnz(insn) ||
	    aarch64_insn_is_bcond(insn)) {
		if (!aarch64_insn_try_encode_scaled_signed_imm19(&insn, offset, 4))
			return AARCH64_BREAK_FAULT;
		return insn;
	}
	if (aarch64_insn_is_tbz(insn) || aarch64_insn_is_tbnz(insn)) {
		if (!aarch64_insn_try_encode_scaled_signed_imm14(&insn, offset, 4))
			return AARCH64_BREAK_FAULT;
		return insn;
	}

	/* Unhandled instruction */
	BUG();
}

s64 aarch64_insn_adr_get_offset(u32 insn)
{
	return aarch64_insn_decode_signed_adr_imm(insn);
}

u32 aarch64_insn_adr_set_offset(u32 insn, s64 offset)
{
	if (!aarch64_insn_try_encode_signed_adr_imm(&insn, offset))
		return AARCH64_BREAK_FAULT;
	return insn;
}

s64 aarch64_insn_adrp_get_offset(u32 insn)
{
	return aarch64_insn_decode_scaled_signed_adr_imm(insn, SZ_4K);
}

u32 aarch64_insn_adrp_set_offset(u32 insn, s64 offset)
{
	if (!aarch64_insn_try_encode_scaled_signed_adr_imm(&insn, offset, SZ_4K))
		return AARCH64_BREAK_FAULT;
	return insn;
}

/*
 * Extract the Op/CR data from a msr/mrs instruction.
 */
u32 aarch64_insn_extract_system_reg(u32 insn)
{
	return (insn & 0x1FFFE0) >> 5;
}

bool aarch32_insn_is_wide(u32 insn)
{
	return insn >= 0xe800;
}

/*
 * Macros/defines for extracting register numbers from instruction.
 */
u32 aarch32_insn_extract_reg_num(u32 insn, int offset)
{
	return (insn & (0xf << offset)) >> offset;
}

#define OPC2_MASK	0x7
#define OPC2_OFFSET	5
u32 aarch32_insn_mcr_extract_opc2(u32 insn)
{
	return (insn & (OPC2_MASK << OPC2_OFFSET)) >> OPC2_OFFSET;
}

#define CRM_MASK	0xf
u32 aarch32_insn_mcr_extract_crm(u32 insn)
{
	return insn & CRM_MASK;
}

static bool range_of_ones(u64 val)
{
	/* Doesn't handle full ones or full zeroes */
	u64 sval = val >> __ffs64(val);

	/* One of Sean Eron Anderson's bithack tricks */
	return ((sval + 1) & (sval)) == 0;
}

static u32 aarch64_encode_bitmask_immediate(u64 imm,
					    enum aarch64_insn_variant variant,
					    u32 insn)
{
	unsigned int immr, imms, n, ones, ror, esz, tmp;
	u64 mask;
	bool sf = false;

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		esz = 32;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		esz = 64;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	mask = GENMASK(esz - 1, 0);

	/* Can't encode full zeroes, full ones, or value wider than the mask */
	if (!imm || imm == mask || imm & ~mask)
		return AARCH64_BREAK_FAULT;

	/*
	 * Inverse of Replicate(). Try to spot a repeating pattern
	 * with a pow2 stride.
	 */
	for (tmp = esz / 2; tmp >= 2; tmp /= 2) {
		u64 emask = BIT(tmp) - 1;

		if ((imm & emask) != ((imm >> tmp) & emask))
			break;

		esz = tmp;
		mask = emask;
	}

	/* N is only set if we're encoding a 64bit value */
	n = esz == 64;

	/* Trim imm to the element size */
	imm &= mask;

	/* That's how many ones we need to encode */
	ones = hweight64(imm);

	/*
	 * imms is set to (ones - 1), prefixed with a string of ones
	 * and a zero if they fit. Cap it to 6 bits.
	 */
	imms  = ones - 1;
	imms |= 0xf << ffs(esz);
	imms &= BIT(6) - 1;

	/* Compute the rotation */
	if (range_of_ones(imm)) {
		/*
		 * Pattern: 0..01..10..0
		 *
		 * Compute how many rotate we need to align it right
		 */
		ror = __ffs64(imm);
	} else {
		/*
		 * Pattern: 0..01..10..01..1
		 *
		 * Fill the unused top bits with ones, and check if
		 * the result is a valid immediate (all ones with a
		 * contiguous ranges of zeroes).
		 */
		imm |= ~mask;
		if (!range_of_ones(~imm))
			return AARCH64_BREAK_FAULT;

		/*
		 * Compute the rotation to get a continuous set of
		 * ones, with the first bit set at position 0
		 */
		ror = fls64(~imm);
	}

	/*
	 * immr is the number of bits we need to rotate back to the
	 * original set of ones. Note that this is relative to the
	 * element size...
	 */
	immr = (esz - ror) % esz;

	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_unsigned_N(&insn, n) ||
	    !aarch64_insn_try_encode_unsigned_immr(&insn, immr) ||
	    !aarch64_insn_try_encode_unsigned_imms(&insn, imms))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_logical_immediate(enum aarch64_insn_logic_type type,
				       enum aarch64_insn_variant variant,
				       enum aarch64_insn_register Rn,
				       enum aarch64_insn_register Rd,
				       u64 imm)
{
	u32 insn;

	switch (type) {
	case AARCH64_INSN_LOGIC_AND:
		insn = aarch64_insn_get_and_imm_value();
		break;
	case AARCH64_INSN_LOGIC_ORR:
		insn = aarch64_insn_get_orr_imm_value();
		break;
	case AARCH64_INSN_LOGIC_EOR:
		insn = aarch64_insn_get_eor_imm_value();
		break;
	case AARCH64_INSN_LOGIC_AND_SETFLAGS:
		insn = aarch64_insn_get_ands_imm_value();
		break;
	default:
		pr_err("%s: unknown logical encoding %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RD, insn, Rd);
	insn = aarch64_insn_encode_register(AARCH64_INSN_REGTYPE_RN, insn, Rn);
	return aarch64_encode_bitmask_immediate(imm, variant, insn);
}

u32 aarch64_insn_gen_extr(enum aarch64_insn_variant variant,
			  enum aarch64_insn_register Rm,
			  enum aarch64_insn_register Rn,
			  enum aarch64_insn_register Rd,
			  u8 lsb)
{
	u32 insn;
	bool sf;

	insn = aarch64_insn_get_extr_value();

	switch (variant) {
	case AARCH64_INSN_VARIANT_32BIT:
		sf = false;
		if (lsb > 31)
			return AARCH64_BREAK_FAULT;
		break;
	case AARCH64_INSN_VARIANT_64BIT:
		sf = true;
		if (lsb > 63)
			return AARCH64_BREAK_FAULT;
		break;
	default:
		pr_err("%s: unknown variant encoding %d\n", __func__, variant);
		return AARCH64_BREAK_FAULT;
	}

	/*
	 * N == sf for all valid encodings
	 */
	if (!aarch64_insn_try_encode_unsigned_sf(&insn, sf) ||
	    !aarch64_insn_try_encode_unsigned_N(&insn, sf) ||
	    !aarch64_insn_try_encode_unsigned_imms(&insn, lsb) ||
	    !aarch64_insn_try_encode_reg_rd(&insn, Rd) ||
	    !aarch64_insn_try_encode_reg_rn(&insn, Rn) ||
	    !aarch64_insn_try_encode_reg_rm(&insn, Rm))
		return AARCH64_BREAK_FAULT;

	return insn;
}

u32 aarch64_insn_gen_dmb(enum aarch64_insn_mb_type type)
{
	u32 opt;
	u32 insn;

	switch (type) {
	case AARCH64_INSN_MB_SY:
		opt = 0xf;
		break;
	case AARCH64_INSN_MB_ST:
		opt = 0xe;
		break;
	case AARCH64_INSN_MB_LD:
		opt = 0xd;
		break;
	case AARCH64_INSN_MB_ISH:
		opt = 0xb;
		break;
	case AARCH64_INSN_MB_ISHST:
		opt = 0xa;
		break;
	case AARCH64_INSN_MB_ISHLD:
		opt = 0x9;
		break;
	case AARCH64_INSN_MB_NSH:
		opt = 0x7;
		break;
	case AARCH64_INSN_MB_NSHST:
		opt = 0x6;
		break;
	case AARCH64_INSN_MB_NSHLD:
		opt = 0x5;
		break;
	default:
		pr_err("%s: unknown dmb type %d\n", __func__, type);
		return AARCH64_BREAK_FAULT;
	}

	insn = aarch64_insn_get_dmb_value();
	insn &= ~GENMASK(11, 8);
	insn |= (opt << 8);

	return insn;
}
