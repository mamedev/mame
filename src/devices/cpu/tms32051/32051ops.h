// license:BSD-3-Clause
// copyright-holders:Ville Linde
const tms32051_device::opcode_func tms32051_device::s_opcode_table[256] =
{
	/* 0x00 - 0x0f */
	&tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,
	&tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,     &tms32051_device::op_lar_mem,
	&tms32051_device::op_lamm,        &tms32051_device::op_smmr,        &tms32051_device::op_subc,        &tms32051_device::op_rpt_mem,
	&tms32051_device::op_out,         &tms32051_device::op_ldp_mem,     &tms32051_device::op_lst_st0,     &tms32051_device::op_lst_st1,
	/* 0x10 - 0x1f */
	&tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,
	&tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,
	&tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,
	&tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,    &tms32051_device::op_lacc_mem,
	/* 0x20 - 0x2f */
	&tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,
	&tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,
	&tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,
	&tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,     &tms32051_device::op_add_mem,
	/* 0x30 - 0x3f */
	&tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,
	&tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,
	&tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,
	&tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,     &tms32051_device::op_sub_mem,
	/* 0x40 - 0x4f */
	&tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,
	&tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,
	&tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,
	&tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,         &tms32051_device::op_bit,
	/* 0x50 - 0x5f */
	&tms32051_device::op_mpya,        &tms32051_device::op_mpys,        &tms32051_device::op_sqra,        &tms32051_device::op_sqrs,
	&tms32051_device::op_mpy_mem,     &tms32051_device::op_mpyu,        &tms32051_device::op_invalid,     &tms32051_device::op_bldp,
	&tms32051_device::op_xpl_dbmr,    &tms32051_device::op_opl_dbmr,    &tms32051_device::op_apl_dbmr,    &tms32051_device::op_cpl_dbmr,
	&tms32051_device::op_xpl_imm,     &tms32051_device::op_opl_imm,     &tms32051_device::op_apl_imm,     &tms32051_device::op_cpl_imm,
	/* 0x60 - 0x6f */
	&tms32051_device::op_addc,        &tms32051_device::op_add_s16_mem, &tms32051_device::op_adds,        &tms32051_device::op_addt,
	&tms32051_device::op_subb,        &tms32051_device::op_sub_s16_mem, &tms32051_device::op_subs,        &tms32051_device::op_subt,
	&tms32051_device::op_zalr,        &tms32051_device::op_lacl_mem,    &tms32051_device::op_lacc_s16_mem,&tms32051_device::op_lact,
	&tms32051_device::op_xor_mem,     &tms32051_device::op_or_mem,      &tms32051_device::op_and_mem,     &tms32051_device::op_bitt,
	/* 0x70 - 0x7f */
	&tms32051_device::op_lta,         &tms32051_device::op_ltp,         &tms32051_device::op_ltd,         &tms32051_device::op_lt,
	&tms32051_device::op_lts,         &tms32051_device::op_lph,         &tms32051_device::op_pshd,        &tms32051_device::op_dmov,
	&tms32051_device::op_adrk,        &tms32051_device::op_b,           &tms32051_device::op_call,        &tms32051_device::op_banz,
	&tms32051_device::op_sbrk,        &tms32051_device::op_bd,          &tms32051_device::op_calld,       &tms32051_device::op_banzd,
	/* 0x80 - 0x8f */
	&tms32051_device::op_sar,         &tms32051_device::op_sar,         &tms32051_device::op_sar,         &tms32051_device::op_sar,
	&tms32051_device::op_sar,         &tms32051_device::op_sar,         &tms32051_device::op_sar,         &tms32051_device::op_sar,
	&tms32051_device::op_samm,        &tms32051_device::op_lmmr,        &tms32051_device::op_popd,        &tms32051_device::op_mar,
	&tms32051_device::op_spl,         &tms32051_device::op_sph,         &tms32051_device::op_sst_st0,     &tms32051_device::op_sst_st1,
	/* 0x90 - 0x9f */
	&tms32051_device::op_sacl,        &tms32051_device::op_sacl,        &tms32051_device::op_sacl,        &tms32051_device::op_sacl,
	&tms32051_device::op_sacl,        &tms32051_device::op_sacl,        &tms32051_device::op_sacl,        &tms32051_device::op_sacl,
	&tms32051_device::op_sach,        &tms32051_device::op_sach,        &tms32051_device::op_sach,        &tms32051_device::op_sach,
	&tms32051_device::op_sach,        &tms32051_device::op_sach,        &tms32051_device::op_sach,        &tms32051_device::op_sach,
	/* 0xa0 - 0xaf */
	&tms32051_device::op_norm,        &tms32051_device::op_invalid,     &tms32051_device::op_mac,         &tms32051_device::op_macd,
	&tms32051_device::op_blpd_bmar,   &tms32051_device::op_blpd_imm,    &tms32051_device::op_tblr,        &tms32051_device::op_tblw,
	&tms32051_device::op_bldd_slimm,  &tms32051_device::op_bldd_dlimm,  &tms32051_device::op_mads,        &tms32051_device::op_madd,
	&tms32051_device::op_bldd_sbmar,  &tms32051_device::op_bldd_dbmar,  &tms32051_device::op_splk,        &tms32051_device::op_in,
	/* 0xb0 - 0xbf */
	&tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,
	&tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,    &tms32051_device::op_lar_simm,
	&tms32051_device::op_add_simm,    &tms32051_device::op_lacl_simm,   &tms32051_device::op_sub_simm,    &tms32051_device::op_rpt_simm,
	&tms32051_device::op_ldp_imm,     &tms32051_device::op_ldp_imm,     &tms32051_device::op_group_be,    &tms32051_device::op_group_bf,
	/* 0xc0 - 0xcf */
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	/* 0xd0 - 0xdf */
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	&tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,    &tms32051_device::op_mpy_simm,
	/* 0xe0 - 0xef */
	&tms32051_device::op_bcnd,        &tms32051_device::op_bcnd,        &tms32051_device::op_bcnd,        &tms32051_device::op_bcnd,
	&tms32051_device::op_xc,          &tms32051_device::op_xc,          &tms32051_device::op_xc,          &tms32051_device::op_xc,
	&tms32051_device::op_cc,          &tms32051_device::op_cc,          &tms32051_device::op_cc,          &tms32051_device::op_cc,
	&tms32051_device::op_retc,        &tms32051_device::op_retc,        &tms32051_device::op_retc,        &tms32051_device::op_retc,
	/* 0xf0 - 0xff */
	&tms32051_device::op_bcndd,       &tms32051_device::op_bcndd,       &tms32051_device::op_bcndd,       &tms32051_device::op_bcndd,
	&tms32051_device::op_xc,          &tms32051_device::op_xc,          &tms32051_device::op_xc,          &tms32051_device::op_xc,
	&tms32051_device::op_ccd,         &tms32051_device::op_ccd,         &tms32051_device::op_ccd,         &tms32051_device::op_ccd,
	&tms32051_device::op_retcd,       &tms32051_device::op_retcd,       &tms32051_device::op_retcd,       &tms32051_device::op_retcd
};

const tms32051_device::opcode_func tms32051_device::s_opcode_table_be[256] =
{
	/* 0x00 - 0x0f */
	&tms32051_device::op_abs,         &tms32051_device::op_cmpl,        &tms32051_device::op_neg,         &tms32051_device::op_pac,
	&tms32051_device::op_apac,        &tms32051_device::op_spac,        &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_sfl,         &tms32051_device::op_sfr,         &tms32051_device::op_invalid,
	&tms32051_device::op_rol,         &tms32051_device::op_ror,         &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x10 - 0x1f */
	&tms32051_device::op_addb,        &tms32051_device::op_adcb,        &tms32051_device::op_andb,        &tms32051_device::op_orb,
	&tms32051_device::op_rolb,        &tms32051_device::op_rorb,        &tms32051_device::op_sflb,        &tms32051_device::op_sfrb,
	&tms32051_device::op_sbb,         &tms32051_device::op_sbbb,        &tms32051_device::op_xorb,        &tms32051_device::op_crgt,
	&tms32051_device::op_crlt,        &tms32051_device::op_exar,        &tms32051_device::op_sacb,        &tms32051_device::op_lacb,
	/* 0x20 - 0x2f */
	&tms32051_device::op_bacc,        &tms32051_device::op_baccd,       &tms32051_device::op_idle,        &tms32051_device::op_idle2,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x30 - 0x3f */
	&tms32051_device::op_cala,        &tms32051_device::op_invalid,     &tms32051_device::op_pop,         &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_reti,        &tms32051_device::op_invalid,     &tms32051_device::op_rete,        &tms32051_device::op_invalid,
	&tms32051_device::op_push,        &tms32051_device::op_calad,       &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x40 - 0x4f */
	&tms32051_device::op_clrc_intm,   &tms32051_device::op_setc_intm,   &tms32051_device::op_clrc_ov,     &tms32051_device::op_setc_ov,
	&tms32051_device::op_clrc_cnf,    &tms32051_device::op_setc_cnf,    &tms32051_device::op_clrc_ext,    &tms32051_device::op_setc_ext,
	&tms32051_device::op_clrc_hold,   &tms32051_device::op_setc_hold,   &tms32051_device::op_clrc_tc,     &tms32051_device::op_setc_tc,
	&tms32051_device::op_clrc_xf,     &tms32051_device::op_setc_xf,     &tms32051_device::op_clrc_carry,  &tms32051_device::op_setc_carry,
	/* 0x50 - 0x5f */
	&tms32051_device::op_invalid,     &tms32051_device::op_trap,        &tms32051_device::op_nmi,         &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_zpr,         &tms32051_device::op_zap,         &tms32051_device::op_sath,        &tms32051_device::op_satl,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x60 - 0x6f */
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	/* 0x70 - 0x7f */
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	&tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,        &tms32051_device::op_intr,
	/* 0x80 - 0x8f */
	&tms32051_device::op_mpy_limm,    &tms32051_device::op_and_s16_limm,&tms32051_device::op_or_s16_limm, &tms32051_device::op_xor_s16_limm,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x90 - 0x9f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xa0 - 0xaf */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xb0 - 0xbf */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xc0 - 0xcf */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_rpt_limm,    &tms32051_device::op_rptz,        &tms32051_device::op_rptb,        &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xd0 - 0xdf */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xe0 - 0xef */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0xf0 - 0xff */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
};

const tms32051_device::opcode_func tms32051_device::s_opcode_table_bf[256] =
{
	/* 0x00 - 0x0f */
	&tms32051_device::op_spm,         &tms32051_device::op_spm,         &tms32051_device::op_spm,         &tms32051_device::op_spm,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,
	&tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,    &tms32051_device::op_lar_limm,
	/* 0x10 - 0x1f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x20 - 0x2f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x30 - 0x3f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x40 - 0x4f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_cmpr,        &tms32051_device::op_cmpr,        &tms32051_device::op_cmpr,        &tms32051_device::op_cmpr,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x50 - 0x5f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x60 - 0x6f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x70 - 0x7f */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	/* 0x80 - 0x8f */
	&tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,
	&tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,
	&tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,
	&tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,   &tms32051_device::op_lacc_limm,
	/* 0x90 - 0x9f */
	&tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,
	&tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,
	&tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,
	&tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,    &tms32051_device::op_add_limm,
	/* 0xa0 - 0xaf */
	&tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,
	&tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,
	&tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,
	&tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,    &tms32051_device::op_sub_limm,
	/* 0xb0 - 0xbf */
	&tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,
	&tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,
	&tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,
	&tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,    &tms32051_device::op_and_limm,
	/* 0xc0 - 0xcf */
	&tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,
	&tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,
	&tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,
	&tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,     &tms32051_device::op_or_limm,
	/* 0xd0 - 0xdf */
	&tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,
	&tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,
	&tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,
	&tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,    &tms32051_device::op_xor_limm,
	/* 0xe0 - 0xef */
	&tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,
	&tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,
	&tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,
	&tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,        &tms32051_device::op_bsar,
	/* 0xf0 - 0xff */
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
	&tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,     &tms32051_device::op_invalid,
};
