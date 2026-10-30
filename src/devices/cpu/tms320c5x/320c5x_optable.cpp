// license:BSD-3-Clause
// copyright-holders:Ville Linde
#include "emu.h"
#include "tms320c5x.h"

const tms320c51_device::opcode_func tms320c51_device::s_opcode_table[256] =
{
	/* 0x00 - 0x0f */
	&tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,
	&tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,     &tms320c51_device::op_lar_mem,
	&tms320c51_device::op_lamm,        &tms320c51_device::op_smmr,        &tms320c51_device::op_subc,        &tms320c51_device::op_rpt_mem,
	&tms320c51_device::op_out,         &tms320c51_device::op_ldp_mem,     &tms320c51_device::op_lst_st0,     &tms320c51_device::op_lst_st1,
	/* 0x10 - 0x1f */
	&tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,
	&tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,
	&tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,
	&tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,    &tms320c51_device::op_lacc_mem,
	/* 0x20 - 0x2f */
	&tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,
	&tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,
	&tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,
	&tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,     &tms320c51_device::op_add_mem,
	/* 0x30 - 0x3f */
	&tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,
	&tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,
	&tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,
	&tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,     &tms320c51_device::op_sub_mem,
	/* 0x40 - 0x4f */
	&tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,
	&tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,
	&tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,
	&tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,         &tms320c51_device::op_bit,
	/* 0x50 - 0x5f */
	&tms320c51_device::op_mpya,        &tms320c51_device::op_mpys,        &tms320c51_device::op_sqra,        &tms320c51_device::op_sqrs,
	&tms320c51_device::op_mpy_mem,     &tms320c51_device::op_mpyu,        &tms320c51_device::op_invalid,     &tms320c51_device::op_bldp,
	&tms320c51_device::op_xpl_dbmr,    &tms320c51_device::op_opl_dbmr,    &tms320c51_device::op_apl_dbmr,    &tms320c51_device::op_cpl_dbmr,
	&tms320c51_device::op_xpl_imm,     &tms320c51_device::op_opl_imm,     &tms320c51_device::op_apl_imm,     &tms320c51_device::op_cpl_imm,
	/* 0x60 - 0x6f */
	&tms320c51_device::op_addc,        &tms320c51_device::op_add_s16_mem, &tms320c51_device::op_adds,        &tms320c51_device::op_addt,
	&tms320c51_device::op_subb,        &tms320c51_device::op_sub_s16_mem, &tms320c51_device::op_subs,        &tms320c51_device::op_subt,
	&tms320c51_device::op_zalr,        &tms320c51_device::op_lacl_mem,    &tms320c51_device::op_lacc_s16_mem,&tms320c51_device::op_lact,
	&tms320c51_device::op_xor_mem,     &tms320c51_device::op_or_mem,      &tms320c51_device::op_and_mem,     &tms320c51_device::op_bitt,
	/* 0x70 - 0x7f */
	&tms320c51_device::op_lta,         &tms320c51_device::op_ltp,         &tms320c51_device::op_ltd,         &tms320c51_device::op_lt,
	&tms320c51_device::op_lts,         &tms320c51_device::op_lph,         &tms320c51_device::op_pshd,        &tms320c51_device::op_dmov,
	&tms320c51_device::op_adrk,        &tms320c51_device::op_b,           &tms320c51_device::op_call,        &tms320c51_device::op_banz,
	&tms320c51_device::op_sbrk,        &tms320c51_device::op_bd,          &tms320c51_device::op_calld,       &tms320c51_device::op_banzd,
	/* 0x80 - 0x8f */
	&tms320c51_device::op_sar,         &tms320c51_device::op_sar,         &tms320c51_device::op_sar,         &tms320c51_device::op_sar,
	&tms320c51_device::op_sar,         &tms320c51_device::op_sar,         &tms320c51_device::op_sar,         &tms320c51_device::op_sar,
	&tms320c51_device::op_samm,        &tms320c51_device::op_lmmr,        &tms320c51_device::op_popd,        &tms320c51_device::op_mar,
	&tms320c51_device::op_spl,         &tms320c51_device::op_sph,         &tms320c51_device::op_sst_st0,     &tms320c51_device::op_sst_st1,
	/* 0x90 - 0x9f */
	&tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,
	&tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,        &tms320c51_device::op_sacl,
	&tms320c51_device::op_sach,        &tms320c51_device::op_sach,        &tms320c51_device::op_sach,        &tms320c51_device::op_sach,
	&tms320c51_device::op_sach,        &tms320c51_device::op_sach,        &tms320c51_device::op_sach,        &tms320c51_device::op_sach,
	/* 0xa0 - 0xaf */
	&tms320c51_device::op_norm,        &tms320c51_device::op_invalid,     &tms320c51_device::op_mac,         &tms320c51_device::op_macd,
	&tms320c51_device::op_blpd_bmar,   &tms320c51_device::op_blpd_imm,    &tms320c51_device::op_tblr,        &tms320c51_device::op_tblw,
	&tms320c51_device::op_bldd_slimm,  &tms320c51_device::op_bldd_dlimm,  &tms320c51_device::op_mads,        &tms320c51_device::op_madd,
	&tms320c51_device::op_bldd_sbmar,  &tms320c51_device::op_bldd_dbmar,  &tms320c51_device::op_splk,        &tms320c51_device::op_in,
	/* 0xb0 - 0xbf */
	&tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,
	&tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,    &tms320c51_device::op_lar_simm,
	&tms320c51_device::op_add_simm,    &tms320c51_device::op_lacl_simm,   &tms320c51_device::op_sub_simm,    &tms320c51_device::op_rpt_simm,
	&tms320c51_device::op_ldp_imm,     &tms320c51_device::op_ldp_imm,     &tms320c51_device::op_group_be,    &tms320c51_device::op_group_bf,
	/* 0xc0 - 0xcf */
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	/* 0xd0 - 0xdf */
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	&tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,    &tms320c51_device::op_mpy_simm,
	/* 0xe0 - 0xef */
	&tms320c51_device::op_bcnd,        &tms320c51_device::op_bcnd,        &tms320c51_device::op_bcnd,        &tms320c51_device::op_bcnd,
	&tms320c51_device::op_xc,          &tms320c51_device::op_xc,          &tms320c51_device::op_xc,          &tms320c51_device::op_xc,
	&tms320c51_device::op_cc,          &tms320c51_device::op_cc,          &tms320c51_device::op_cc,          &tms320c51_device::op_cc,
	&tms320c51_device::op_retc,        &tms320c51_device::op_retc,        &tms320c51_device::op_retc,        &tms320c51_device::op_retc,
	/* 0xf0 - 0xff */
	&tms320c51_device::op_bcndd,       &tms320c51_device::op_bcndd,       &tms320c51_device::op_bcndd,       &tms320c51_device::op_bcndd,
	&tms320c51_device::op_xc,          &tms320c51_device::op_xc,          &tms320c51_device::op_xc,          &tms320c51_device::op_xc,
	&tms320c51_device::op_ccd,         &tms320c51_device::op_ccd,         &tms320c51_device::op_ccd,         &tms320c51_device::op_ccd,
	&tms320c51_device::op_retcd,       &tms320c51_device::op_retcd,       &tms320c51_device::op_retcd,       &tms320c51_device::op_retcd
};

const tms320c51_device::opcode_func tms320c51_device::s_opcode_table_be[256] =
{
	/* 0x00 - 0x0f */
	&tms320c51_device::op_abs,         &tms320c51_device::op_cmpl,        &tms320c51_device::op_neg,         &tms320c51_device::op_pac,
	&tms320c51_device::op_apac,        &tms320c51_device::op_spac,        &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_sfl,         &tms320c51_device::op_sfr,         &tms320c51_device::op_invalid,
	&tms320c51_device::op_rol,         &tms320c51_device::op_ror,         &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x10 - 0x1f */
	&tms320c51_device::op_addb,        &tms320c51_device::op_adcb,        &tms320c51_device::op_andb,        &tms320c51_device::op_orb,
	&tms320c51_device::op_rolb,        &tms320c51_device::op_rorb,        &tms320c51_device::op_sflb,        &tms320c51_device::op_sfrb,
	&tms320c51_device::op_sbb,         &tms320c51_device::op_sbbb,        &tms320c51_device::op_xorb,        &tms320c51_device::op_crgt,
	&tms320c51_device::op_crlt,        &tms320c51_device::op_exar,        &tms320c51_device::op_sacb,        &tms320c51_device::op_lacb,
	/* 0x20 - 0x2f */
	&tms320c51_device::op_bacc,        &tms320c51_device::op_baccd,       &tms320c51_device::op_idle,        &tms320c51_device::op_idle2,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x30 - 0x3f */
	&tms320c51_device::op_cala,        &tms320c51_device::op_invalid,     &tms320c51_device::op_pop,         &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_reti,        &tms320c51_device::op_invalid,     &tms320c51_device::op_rete,        &tms320c51_device::op_invalid,
	&tms320c51_device::op_push,        &tms320c51_device::op_calad,       &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x40 - 0x4f */
	&tms320c51_device::op_clrc_intm,   &tms320c51_device::op_setc_intm,   &tms320c51_device::op_clrc_ov,     &tms320c51_device::op_setc_ov,
	&tms320c51_device::op_clrc_cnf,    &tms320c51_device::op_setc_cnf,    &tms320c51_device::op_clrc_ext,    &tms320c51_device::op_setc_ext,
	&tms320c51_device::op_clrc_hold,   &tms320c51_device::op_setc_hold,   &tms320c51_device::op_clrc_tc,     &tms320c51_device::op_setc_tc,
	&tms320c51_device::op_clrc_xf,     &tms320c51_device::op_setc_xf,     &tms320c51_device::op_clrc_carry,  &tms320c51_device::op_setc_carry,
	/* 0x50 - 0x5f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_trap,        &tms320c51_device::op_nmi,         &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_zpr,         &tms320c51_device::op_zap,         &tms320c51_device::op_sath,        &tms320c51_device::op_satl,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x60 - 0x6f */
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	/* 0x70 - 0x7f */
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	&tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,        &tms320c51_device::op_intr,
	/* 0x80 - 0x8f */
	&tms320c51_device::op_mpy_limm,    &tms320c51_device::op_and_s16_limm,&tms320c51_device::op_or_s16_limm, &tms320c51_device::op_xor_s16_limm,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x90 - 0x9f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xa0 - 0xaf */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xb0 - 0xbf */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xc0 - 0xcf */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_rpt_limm,    &tms320c51_device::op_rptz,        &tms320c51_device::op_rptb,        &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xd0 - 0xdf */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xe0 - 0xef */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0xf0 - 0xff */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
};

const tms320c51_device::opcode_func tms320c51_device::s_opcode_table_bf[256] =
{
	/* 0x00 - 0x0f */
	&tms320c51_device::op_spm,         &tms320c51_device::op_spm,         &tms320c51_device::op_spm,         &tms320c51_device::op_spm,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,
	&tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,    &tms320c51_device::op_lar_limm,
	/* 0x10 - 0x1f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x20 - 0x2f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x30 - 0x3f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x40 - 0x4f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_cmpr,        &tms320c51_device::op_cmpr,        &tms320c51_device::op_cmpr,        &tms320c51_device::op_cmpr,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x50 - 0x5f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x60 - 0x6f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x70 - 0x7f */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	/* 0x80 - 0x8f */
	&tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,
	&tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,
	&tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,
	&tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,   &tms320c51_device::op_lacc_limm,
	/* 0x90 - 0x9f */
	&tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,
	&tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,
	&tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,
	&tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,    &tms320c51_device::op_add_limm,
	/* 0xa0 - 0xaf */
	&tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,
	&tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,
	&tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,
	&tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,    &tms320c51_device::op_sub_limm,
	/* 0xb0 - 0xbf */
	&tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,
	&tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,
	&tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,
	&tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,    &tms320c51_device::op_and_limm,
	/* 0xc0 - 0xcf */
	&tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,
	&tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,
	&tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,
	&tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,     &tms320c51_device::op_or_limm,
	/* 0xd0 - 0xdf */
	&tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,
	&tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,
	&tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,
	&tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,    &tms320c51_device::op_xor_limm,
	/* 0xe0 - 0xef */
	&tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,
	&tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,
	&tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,
	&tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,        &tms320c51_device::op_bsar,
	/* 0xf0 - 0xff */
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
	&tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,     &tms320c51_device::op_invalid,
};
