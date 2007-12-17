static void (* tms32051_opcode_table[256])(void) =
{
	/* 0x00 - 0x0f */
	op_lar_mem,		op_lar_mem,		op_lar_mem,		op_lar_mem,
	op_lar_mem,		op_lar_mem,		op_lar_mem,		op_lar_mem,
	op_lamm,		op_smmr,		op_subc,		op_rpt_mem,
	op_out,			op_ldp_mem,		op_lst_st0,		op_lst_st1,
	/* 0x10 - 0x1f */
	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,
	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,
	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,
	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,	op_lacc_mem,
	/* 0x20 - 0x2f */
	op_add_mem,		op_add_mem,		op_add_mem,		op_add_mem,
	op_add_mem,		op_add_mem,		op_add_mem,		op_add_mem,
	op_add_mem,		op_add_mem,		op_add_mem,		op_add_mem,
	op_add_mem,		op_add_mem,		op_add_mem,		op_add_mem,
	/* 0x30 - 0x3f */
	op_sub_mem,		op_sub_mem,		op_sub_mem,		op_sub_mem,
	op_sub_mem,		op_sub_mem,		op_sub_mem,		op_sub_mem,
	op_sub_mem,		op_sub_mem,		op_sub_mem,		op_sub_mem,
	op_sub_mem,		op_sub_mem,		op_sub_mem,		op_sub_mem,
	/* 0x40 - 0x4f */
	op_bit,			op_bit,			op_bit,			op_bit,
	op_bit,			op_bit,			op_bit,			op_bit,
	op_bit,			op_bit,			op_bit,			op_bit,
	op_bit,			op_bit,			op_bit,			op_bit,
	/* 0x50 - 0x5f */
	op_mpya,		op_mpys,		op_sqra,		op_sqrs,
	op_mpy_mem,		op_mpyu,		op_invalid,		op_bldp,
	op_xpl_dbmr,	op_opl_dbmr,	op_apl_dbmr,	op_cpl_dbmr,
	op_xpl_imm,		op_opl_imm,		op_apl_imm,		op_cpl_imm,
	/* 0x60 - 0x6f */
	op_addc,		op_add_s16_mem,	op_adds,		op_addt,
	op_subb,		op_sub_s16_mem,	op_subs,		op_subt,
	op_zalr,		op_lacl_mem,	op_lacc_s16_mem,op_lact,
	op_xor_mem,		op_or_mem,		op_and_mem,		op_bitt,
	/* 0x70 - 0x7f */
	op_lta,			op_ltp,			op_ltd,			op_lt,
	op_lts,			op_lph,			op_pshd,		op_dmov,
	op_adrk,		op_b,			op_call,		op_banz,
	op_sbrk,		op_bd,			op_calld,		op_banzd,
	/* 0x80 - 0x8f */
	op_sar,			op_sar,			op_sar,			op_sar,
	op_sar,			op_sar,			op_sar,			op_sar,
	op_samm,		op_lmmr,		op_popd,		op_mar,
	op_spl,			op_sph,			op_sst_st0,		op_sst_st1,
	/* 0x90 - 0x9f */
	op_sacl,		op_sacl,		op_sacl,		op_sacl,
	op_sacl,		op_sacl,		op_sacl,		op_sacl,
	op_sach,		op_sach,		op_sach,		op_sach,
	op_sach,		op_sach,		op_sach,		op_sach,
	/* 0xa0 - 0xaf */
	op_norm,		op_invalid,		op_mac,			op_macd,
	op_blpd_bmar,	op_blpd_imm,	op_tblr,		op_tblw,
	op_bldd_slimm,	op_bldd_dlimm,	op_mads,		op_madd,
	op_bldd_sbmar,	op_bldd_dbmar,	op_splk,		op_in,
	/* 0xb0 - 0xbf */
	op_lar_simm,	op_lar_simm,	op_lar_simm,	op_lar_simm,
	op_lar_simm,	op_lar_simm,	op_lar_simm,	op_lar_simm,
	op_add_simm,	op_lacl_simm,	op_sub_simm,	op_rpt_simm,
	op_ldp_imm,		op_ldp_imm,		op_group_be,	op_group_bf,
	/* 0xc0 - 0xcf */
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	/* 0xd0 - 0xdf */
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,	op_mpy_simm,
	/* 0xe0 - 0xef */
	op_bcnd,		op_bcnd,		op_bcnd,		op_bcnd,
	op_xc,			op_xc,			op_xc,			op_xc,
	op_cc,			op_cc,			op_cc,			op_cc,
	op_retc,		op_retc,		op_retc,		op_retc,
	/* 0xf0 - 0xff */
	op_bcndd,		op_bcndd,		op_bcndd,		op_bcndd,
	op_xc,			op_xc,			op_xc,			op_xc,
	op_ccd,			op_ccd,			op_ccd,			op_ccd,
	op_retcd,		op_retcd,		op_retcd,		op_retcd
};

static void (* tms32051_opcode_table_be[256])(void) =
{
	/* 0x00 - 0x0f */
	op_abs,			op_cmpl,		op_neg,			op_pac,
	op_apac,		op_spac,		op_invalid,		op_invalid,
	op_invalid,		op_sfl,			op_sfr,			op_invalid,
	op_rol,			op_ror,			op_invalid,		op_invalid,
	/* 0x10 - 0x1f */
	op_addb,		op_adcb,		op_andb,		op_orb,
	op_rolb,		op_rorb,		op_sflb,		op_sfrb,
	op_sbb,			op_sbbb,		op_xorb,		op_crgt,
	op_crlt,		op_exar,		op_sacb,		op_lacb,
	/* 0x20 - 0x2f */
	op_bacc,		op_baccd,		op_idle,		op_idle2,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x30 - 0x3f */
	op_cala,		op_invalid,		op_pop,			op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_reti,		op_invalid,		op_rete,		op_invalid,
	op_push,		op_calad,		op_invalid,		op_invalid,
	/* 0x40 - 0x4f */
	op_clrc_intm,	op_setc_intm,	op_clrc_ov,		op_setc_ov,
	op_clrc_cnf,	op_setc_cnf,	op_clrc_ext,	op_setc_ext,
	op_clrc_hold,	op_setc_hold,	op_clrc_tc,		op_setc_tc,
	op_clrc_xf,		op_setc_xf,		op_clrc_carry,	op_setc_carry,
	/* 0x50 - 0x5f */
	op_invalid,		op_trap,		op_nmi,			op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_zpr,			op_zap,			op_sath,		op_satl,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x60 - 0x6f */
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	/* 0x70 - 0x7f */
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	op_intr,		op_intr,		op_intr,		op_intr,
	/* 0x80 - 0x8f */
	op_mpy_limm,	op_and_s16_limm,op_or_s16_limm,	op_xor_s16_limm,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x90 - 0x9f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xa0 - 0xaf */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xb0 - 0xbf */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xc0 - 0xcf */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_rpt_limm,	op_rptz,		op_rptb,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xd0 - 0xdf */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xe0 - 0xef */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0xf0 - 0xff */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
};

static void (* tms32051_opcode_table_bf[256])(void) =
{
	/* 0x00 - 0x0f */
	op_spm,			op_spm,			op_spm,			op_spm,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_lar_limm,	op_lar_limm,	op_lar_limm,	op_lar_limm,
	op_lar_limm,	op_lar_limm,	op_lar_limm,	op_lar_limm,
	/* 0x10 - 0x1f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x20 - 0x2f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x30 - 0x3f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x40 - 0x4f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_cmpr,		op_cmpr,		op_cmpr,		op_cmpr,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x50 - 0x5f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x60 - 0x6f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x70 - 0x7f */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	/* 0x80 - 0x8f */
	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,
	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,
	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,
	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,	op_lacc_limm,
	/* 0x90 - 0x9f */
	op_add_limm,	op_add_limm,	op_add_limm,	op_add_limm,
	op_add_limm,	op_add_limm,	op_add_limm,	op_add_limm,
	op_add_limm,	op_add_limm,	op_add_limm,	op_add_limm,
	op_add_limm,	op_add_limm,	op_add_limm,	op_add_limm,
	/* 0xa0 - 0xaf */
	op_sub_limm,	op_sub_limm,	op_sub_limm,	op_sub_limm,
	op_sub_limm,	op_sub_limm,	op_sub_limm,	op_sub_limm,
	op_sub_limm,	op_sub_limm,	op_sub_limm,	op_sub_limm,
	op_sub_limm,	op_sub_limm,	op_sub_limm,	op_sub_limm,
	/* 0xb0 - 0xbf */
	op_and_limm,	op_and_limm,	op_and_limm,	op_and_limm,
	op_and_limm,	op_and_limm,	op_and_limm,	op_and_limm,
	op_and_limm,	op_and_limm,	op_and_limm,	op_and_limm,
	op_and_limm,	op_and_limm,	op_and_limm,	op_and_limm,
	/* 0xc0 - 0xcf */
	op_or_limm,		op_or_limm,		op_or_limm,		op_or_limm,
	op_or_limm,		op_or_limm,		op_or_limm,		op_or_limm,
	op_or_limm,		op_or_limm,		op_or_limm,		op_or_limm,
	op_or_limm,		op_or_limm,		op_or_limm,		op_or_limm,
	/* 0xd0 - 0xdf */
	op_xor_limm,	op_xor_limm,	op_xor_limm,	op_xor_limm,
	op_xor_limm,	op_xor_limm,	op_xor_limm,	op_xor_limm,
	op_xor_limm,	op_xor_limm,	op_xor_limm,	op_xor_limm,
	op_xor_limm,	op_xor_limm,	op_xor_limm,	op_xor_limm,
	/* 0xe0 - 0xef */
	op_bsar,		op_bsar,		op_bsar,		op_bsar,
	op_bsar,		op_bsar,		op_bsar,		op_bsar,
	op_bsar,		op_bsar,		op_bsar,		op_bsar,
	op_bsar,		op_bsar,		op_bsar,		op_bsar,
	/* 0xf0 - 0xff */
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
	op_invalid,		op_invalid,		op_invalid,		op_invalid,
};
