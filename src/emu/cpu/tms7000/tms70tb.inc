/*****************************************************************************
 *
 *   tms70tb.inc (function table)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

const tms7000_device::opcode_func tms7000_device::s_opfn[0x100] = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  &tms7000_device::nop,     &tms7000_device::idle,    &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::eint,    &tms7000_device::dint,    &tms7000_device::setc,
			&tms7000_device::pop_st,  &tms7000_device::stsp,    &tms7000_device::rets,    &tms7000_device::reti,    &tms7000_device::illegal, &tms7000_device::ldsp,    &tms7000_device::push_st, &tms7000_device::illegal,

/* 0x1X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2a, &tms7000_device::and_r2a, &tms7000_device::or_r2a,  &tms7000_device::xor_r2a, &tms7000_device::btjo_r2a,&tms7000_device::btjz_r2a,
			&tms7000_device::add_r2a, &tms7000_device::adc_r2a, &tms7000_device::sub_ra,  &tms7000_device::sbb_ra,  &tms7000_device::mpy_ra,  &tms7000_device::cmp_ra,  &tms7000_device::dac_r2a, &tms7000_device::dsb_r2a,

/* 0x2X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2a, &tms7000_device::and_i2a, &tms7000_device::or_i2a,  &tms7000_device::xor_i2a, &tms7000_device::btjo_i2a,&tms7000_device::btjz_i2a,
			&tms7000_device::add_i2a, &tms7000_device::adc_i2a, &tms7000_device::sub_ia,  &tms7000_device::sbb_ia,  &tms7000_device::mpy_ia,  &tms7000_device::cmp_ia,  &tms7000_device::dac_i2a, &tms7000_device::dsb_i2a,

/* 0x3X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2b, &tms7000_device::and_r2b, &tms7000_device::or_r2b,  &tms7000_device::xor_r2b, &tms7000_device::btjo_r2b,&tms7000_device::btjz_r2b,
			&tms7000_device::add_r2b, &tms7000_device::adc_r2b, &tms7000_device::sub_rb,  &tms7000_device::sbb_rb,  &tms7000_device::mpy_rb,  &tms7000_device::cmp_rb,  &tms7000_device::dac_r2b, &tms7000_device::dsb_r2b,

/* 0x4X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2r, &tms7000_device::and_r2r, &tms7000_device::or_r2r,  &tms7000_device::xor_r2r, &tms7000_device::btjo_r2r,&tms7000_device::btjz_r2r,
			&tms7000_device::add_r2r, &tms7000_device::adc_r2r, &tms7000_device::sub_rr,  &tms7000_device::sbb_rr,  &tms7000_device::mpy_rr,  &tms7000_device::cmp_rr,  &tms7000_device::dac_r2r, &tms7000_device::dsb_r2r,

/* 0x5X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2b, &tms7000_device::and_i2b, &tms7000_device::or_i2b,  &tms7000_device::xor_i2b, &tms7000_device::btjo_i2b,&tms7000_device::btjz_i2b,
			&tms7000_device::add_i2b, &tms7000_device::adc_i2b, &tms7000_device::sub_ib,  &tms7000_device::sbb_ib,  &tms7000_device::mpy_ib,  &tms7000_device::cmp_ib,  &tms7000_device::dac_i2b, &tms7000_device::dsb_i2b,

/* 0x6X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_b2a, &tms7000_device::and_b2a, &tms7000_device::or_b2a,  &tms7000_device::xor_b2a, &tms7000_device::btjo_b2a,&tms7000_device::btjz_b2a,
			&tms7000_device::add_b2a, &tms7000_device::adc_b2a, &tms7000_device::sub_ba,  &tms7000_device::sbb_ba,  &tms7000_device::mpy_ba,  &tms7000_device::cmp_ba,  &tms7000_device::dac_b2a, &tms7000_device::dsb_b2a,

/* 0x7X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2r, &tms7000_device::and_i2r, &tms7000_device::or_i2r,  &tms7000_device::xor_i2r, &tms7000_device::btjo_i2r,&tms7000_device::btjz_i2r,
			&tms7000_device::add_i2r, &tms7000_device::adc_i2r, &tms7000_device::sub_ir,  &tms7000_device::sbb_ir,  &tms7000_device::mpy_ir,  &tms7000_device::cmp_ir,  &tms7000_device::dac_i2r, &tms7000_device::dsb_i2r,

/* 0x8X */  &tms7000_device::movp_p2a,&tms7000_device::illegal, &tms7000_device::movp_a2p,&tms7000_device::andp_a2p,&tms7000_device::orp_a2p, &tms7000_device::xorp_a2p,&tms7000_device::btjop_ap,&tms7000_device::btjzp_ap,
			&tms7000_device::movd_imm,&tms7000_device::illegal, &tms7000_device::lda_dir, &tms7000_device::sta_dir, &tms7000_device::br_dir,  &tms7000_device::cmpa_dir,&tms7000_device::call_dir,&tms7000_device::illegal,

/* 0x9X */  &tms7000_device::illegal, &tms7000_device::movp_p2b,&tms7000_device::movp_b2p,&tms7000_device::andp_b2p,&tms7000_device::orp_b2p, &tms7000_device::xorp_b2p,&tms7000_device::btjop_bp,&tms7000_device::btjzp_bp,
			&tms7000_device::movd_r,  &tms7000_device::illegal, &tms7000_device::lda_ind, &tms7000_device::sta_ind, &tms7000_device::br_ind,  &tms7000_device::cmpa_ind,&tms7000_device::call_ind,&tms7000_device::illegal,

/* 0xAX */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::movp_i2p,&tms7000_device::andp_i2p,&tms7000_device::orp_i2p, &tms7000_device::xorp_i2p,&tms7000_device::btjop_ip,&tms7000_device::btjzp_ip,
			&tms7000_device::movd_inx,&tms7000_device::illegal, &tms7000_device::lda_inx, &tms7000_device::sta_inx, &tms7000_device::br_inx,  &tms7000_device::cmpa_inx,&tms7000_device::call_inx,&tms7000_device::illegal,

/* 0xBX */  &tms7000_device::clrc,    &tms7000_device::illegal, &tms7000_device::dec_a,   &tms7000_device::inc_a,   &tms7000_device::inv_a,   &tms7000_device::clr_a,   &tms7000_device::xchb_a,  &tms7000_device::swap_a,
			&tms7000_device::push_a,  &tms7000_device::pop_a,   &tms7000_device::djnz_a,  &tms7000_device::decd_a,  &tms7000_device::rr_a,    &tms7000_device::rrc_a,   &tms7000_device::rl_a,    &tms7000_device::rlc_a,

/* 0xCX */  &tms7000_device::mov_a2b, &tms7000_device::tstb,    &tms7000_device::dec_b,   &tms7000_device::inc_b,   &tms7000_device::inv_b,   &tms7000_device::clr_b,   &tms7000_device::xchb_b,  &tms7000_device::swap_b,
			&tms7000_device::push_b,  &tms7000_device::pop_b,   &tms7000_device::djnz_b,  &tms7000_device::decd_b,  &tms7000_device::rr_b,    &tms7000_device::rrc_b,   &tms7000_device::rl_b,    &tms7000_device::rlc_b,

/* 0xDX */  &tms7000_device::mov_a2r, &tms7000_device::mov_b2r, &tms7000_device::dec_r,   &tms7000_device::inc_r,   &tms7000_device::inv_r,   &tms7000_device::clr_r,   &tms7000_device::xchb_r,  &tms7000_device::swap_r,
			&tms7000_device::push_r,  &tms7000_device::pop_r,   &tms7000_device::djnz_r,  &tms7000_device::decd_r,  &tms7000_device::rr_r,    &tms7000_device::rrc_r,   &tms7000_device::rl_r,    &tms7000_device::rlc_r,

/* 0xEX */  &tms7000_device::jmp,     &tms7000_device::j_jn,    &tms7000_device::jeq,     &tms7000_device::jc,      &tms7000_device::jp,      &tms7000_device::jpz,     &tms7000_device::jne,     &tms7000_device::jl,
			&tms7000_device::trap_23, &tms7000_device::trap_22, &tms7000_device::trap_21, &tms7000_device::trap_20, &tms7000_device::trap_19, &tms7000_device::trap_18, &tms7000_device::trap_17, &tms7000_device::trap_16,

/* 0xFX */  &tms7000_device::trap_15, &tms7000_device::trap_14, &tms7000_device::trap_13, &tms7000_device::trap_12, &tms7000_device::trap_11, &tms7000_device::trap_10, &tms7000_device::trap_9,  &tms7000_device::trap_8,
			&tms7000_device::trap_7,  &tms7000_device::trap_6,  &tms7000_device::trap_5,  &tms7000_device::trap_4,  &tms7000_device::trap_3,  &tms7000_device::trap_2,  &tms7000_device::trap_1,  &tms7000_device::trap_0
};

const tms7000_device::opcode_func tms7000_device::s_opfn_exl[0x100] = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  &tms7000_device::nop,     &tms7000_device::idle,    &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::eint,    &tms7000_device::dint,    &tms7000_device::setc,
			&tms7000_device::pop_st,  &tms7000_device::stsp,    &tms7000_device::rets,    &tms7000_device::reti,    &tms7000_device::illegal, &tms7000_device::ldsp,    &tms7000_device::push_st, &tms7000_device::illegal,

/* 0x1X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2a, &tms7000_device::and_r2a, &tms7000_device::or_r2a,  &tms7000_device::xor_r2a, &tms7000_device::btjo_r2a,&tms7000_device::btjz_r2a,
			&tms7000_device::add_r2a, &tms7000_device::adc_r2a, &tms7000_device::sub_ra,  &tms7000_device::sbb_ra,  &tms7000_device::mpy_ra,  &tms7000_device::cmp_ra,  &tms7000_device::dac_r2a, &tms7000_device::dsb_r2a,

/* 0x2X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2a, &tms7000_device::and_i2a, &tms7000_device::or_i2a,  &tms7000_device::xor_i2a, &tms7000_device::btjo_i2a,&tms7000_device::btjz_i2a,
			&tms7000_device::add_i2a, &tms7000_device::adc_i2a, &tms7000_device::sub_ia,  &tms7000_device::sbb_ia,  &tms7000_device::mpy_ia,  &tms7000_device::cmp_ia,  &tms7000_device::dac_i2a, &tms7000_device::dsb_i2a,

/* 0x3X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2b, &tms7000_device::and_r2b, &tms7000_device::or_r2b,  &tms7000_device::xor_r2b, &tms7000_device::btjo_r2b,&tms7000_device::btjz_r2b,
			&tms7000_device::add_r2b, &tms7000_device::adc_r2b, &tms7000_device::sub_rb,  &tms7000_device::sbb_rb,  &tms7000_device::mpy_rb,  &tms7000_device::cmp_rb,  &tms7000_device::dac_r2b, &tms7000_device::dsb_r2b,

/* 0x4X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_r2r, &tms7000_device::and_r2r, &tms7000_device::or_r2r,  &tms7000_device::xor_r2r, &tms7000_device::btjo_r2r,&tms7000_device::btjz_r2r,
			&tms7000_device::add_r2r, &tms7000_device::adc_r2r, &tms7000_device::sub_rr,  &tms7000_device::sbb_rr,  &tms7000_device::mpy_rr,  &tms7000_device::cmp_rr,  &tms7000_device::dac_r2r, &tms7000_device::dsb_r2r,

/* 0x5X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2b, &tms7000_device::and_i2b, &tms7000_device::or_i2b,  &tms7000_device::xor_i2b, &tms7000_device::btjo_i2b,&tms7000_device::btjz_i2b,
			&tms7000_device::add_i2b, &tms7000_device::adc_i2b, &tms7000_device::sub_ib,  &tms7000_device::sbb_ib,  &tms7000_device::mpy_ib,  &tms7000_device::cmp_ib,  &tms7000_device::dac_i2b, &tms7000_device::dsb_i2b,

/* 0x6X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_b2a, &tms7000_device::and_b2a, &tms7000_device::or_b2a,  &tms7000_device::xor_b2a, &tms7000_device::btjo_b2a,&tms7000_device::btjz_b2a,
			&tms7000_device::add_b2a, &tms7000_device::adc_b2a, &tms7000_device::sub_ba,  &tms7000_device::sbb_ba,  &tms7000_device::mpy_ba,  &tms7000_device::cmp_ba,  &tms7000_device::dac_b2a, &tms7000_device::dsb_b2a,

/* 0x7X */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::mov_i2r, &tms7000_device::and_i2r, &tms7000_device::or_i2r,  &tms7000_device::xor_i2r, &tms7000_device::btjo_i2r,&tms7000_device::btjz_i2r,
			&tms7000_device::add_i2r, &tms7000_device::adc_i2r, &tms7000_device::sub_ir,  &tms7000_device::sbb_ir,  &tms7000_device::mpy_ir,  &tms7000_device::cmp_ir,  &tms7000_device::dac_i2r, &tms7000_device::dsb_i2r,

/* 0x8X */  &tms7000_device::movp_p2a,&tms7000_device::illegal, &tms7000_device::movp_a2p,&tms7000_device::andp_a2p,&tms7000_device::orp_a2p, &tms7000_device::xorp_a2p,&tms7000_device::btjop_ap,&tms7000_device::btjzp_ap,
			&tms7000_device::movd_imm,&tms7000_device::illegal, &tms7000_device::lda_dir, &tms7000_device::sta_dir, &tms7000_device::br_dir,  &tms7000_device::cmpa_dir,&tms7000_device::call_dir,&tms7000_device::illegal,

/* 0x9X */  &tms7000_device::illegal, &tms7000_device::movp_p2b,&tms7000_device::movp_b2p,&tms7000_device::andp_b2p,&tms7000_device::orp_b2p, &tms7000_device::xorp_b2p,&tms7000_device::btjop_bp,&tms7000_device::btjzp_bp,
			&tms7000_device::movd_r,  &tms7000_device::illegal, &tms7000_device::lda_ind, &tms7000_device::sta_ind, &tms7000_device::br_ind,  &tms7000_device::cmpa_ind,&tms7000_device::call_ind,&tms7000_device::illegal,

/* 0xAX */  &tms7000_device::illegal, &tms7000_device::illegal, &tms7000_device::movp_i2p,&tms7000_device::andp_i2p,&tms7000_device::orp_i2p, &tms7000_device::xorp_i2p,&tms7000_device::btjop_ip,&tms7000_device::btjzp_ip,
			&tms7000_device::movd_inx,&tms7000_device::illegal, &tms7000_device::lda_inx, &tms7000_device::sta_inx, &tms7000_device::br_inx,  &tms7000_device::cmpa_inx,&tms7000_device::call_inx,&tms7000_device::illegal,

/* 0xBX */  &tms7000_device::clrc,    &tms7000_device::illegal, &tms7000_device::dec_a,   &tms7000_device::inc_a,   &tms7000_device::inv_a,   &tms7000_device::clr_a,   &tms7000_device::xchb_a,  &tms7000_device::swap_a,
			&tms7000_device::push_a,  &tms7000_device::pop_a,   &tms7000_device::djnz_a,  &tms7000_device::decd_a,  &tms7000_device::rr_a,    &tms7000_device::rrc_a,   &tms7000_device::rl_a,    &tms7000_device::rlc_a,

/* 0xCX */  &tms7000_device::mov_a2b, &tms7000_device::tstb,    &tms7000_device::dec_b,   &tms7000_device::inc_b,   &tms7000_device::inv_b,   &tms7000_device::clr_b,   &tms7000_device::xchb_b,  &tms7000_device::swap_b,
			&tms7000_device::push_b,  &tms7000_device::pop_b,   &tms7000_device::djnz_b,  &tms7000_device::decd_b,  &tms7000_device::rr_b,    &tms7000_device::rrc_b,   &tms7000_device::rl_b,    &tms7000_device::rlc_b,

/* 0xDX */  &tms7000_device::mov_a2r, &tms7000_device::mov_b2r, &tms7000_device::dec_r,   &tms7000_device::inc_r,   &tms7000_device::inv_r,   &tms7000_device::clr_r,   &tms7000_device::xchb_r,  &tms7000_device::swap_r_exl,
			&tms7000_device::push_r,  &tms7000_device::pop_r,   &tms7000_device::djnz_r,  &tms7000_device::decd_r,  &tms7000_device::rr_r,    &tms7000_device::rrc_r,   &tms7000_device::rl_r,    &tms7000_device::rlc_r,

/* 0xEX */  &tms7000_device::jmp,     &tms7000_device::j_jn,    &tms7000_device::jeq,     &tms7000_device::jc,      &tms7000_device::jp,      &tms7000_device::jpz,     &tms7000_device::jne,     &tms7000_device::jl,
			&tms7000_device::trap_23, &tms7000_device::trap_22, &tms7000_device::trap_21, &tms7000_device::trap_20, &tms7000_device::trap_19, &tms7000_device::trap_18, &tms7000_device::trap_17, &tms7000_device::trap_16,

/* 0xFX */  &tms7000_device::trap_15, &tms7000_device::trap_14, &tms7000_device::trap_13, &tms7000_device::trap_12, &tms7000_device::trap_11, &tms7000_device::trap_10, &tms7000_device::trap_9,  &tms7000_device::trap_8,
			&tms7000_device::trap_7,  &tms7000_device::trap_6,  &tms7000_device::trap_5,  &tms7000_device::trap_4,  &tms7000_device::trap_3,  &tms7000_device::trap_2,  &tms7000_device::trap_1,  &tms7000_device::trap_0
};
