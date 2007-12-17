/*****************************************************************************
 *
 *   tms70tb.c (function table)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright (c) 2001 tim lindner, all rights reserved.
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

static void (*opfn[0x100])(void) = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  nop,     idle,    illegal, illegal, illegal, eint,    dint,    setc,
            pop_st,  stsp,    rets,    reti,    illegal, ldsp,    push_st, illegal,

/* 0x1X */  illegal, illegal, mov_r2a, and_r2a, or_r2a,  xor_r2a, btjo_r2a,btjz_r2a,
            add_r2a, adc_r2a, sub_ra,  sbb_ra,  mpy_ra,  cmp_ra,  dac_r2a, dsb_r2a,

/* 0x2X */  illegal, illegal, mov_i2a, and_i2a, or_i2a,  xor_i2a, btjo_i2a,btjz_i2a,
            add_i2a, adc_i2a, sub_ia,  sbb_ia,  mpy_ia,  cmp_ia,  dac_i2a, dsb_i2a,

/* 0x3X */  illegal, illegal, mov_r2b, and_r2b, or_r2b,  xor_r2b, btjo_r2b,btjz_r2b,
            add_r2b, adc_r2b, sub_rb,  sbb_rb,  mpy_rb,  cmp_rb,  dac_r2b, dsb_r2b,

/* 0x4X */  illegal, illegal, mov_r2r, and_r2r, or_r2r,  xor_r2r, btjo_r2r,btjz_r2r,
            add_r2r, adc_r2r, sub_rr,  sbb_rr,  mpy_rr,  cmp_rr,  dac_r2r, dsb_r2r,

/* 0x5X */  illegal, illegal, mov_i2b, and_i2b, or_i2b,  xor_i2b, btjo_i2b,btjz_i2b,
            add_i2b, adc_i2b, sub_ib,  sbb_ib,  mpy_ib,  cmp_ib,  dac_i2b, dsb_i2b,

/* 0x6X */  illegal, illegal, mov_b2a, and_b2a, or_b2a,  xor_b2a, btjo_b2a,btjz_b2a,
            add_b2a, adc_b2a, sub_ba,  sbb_ba,  mpy_ba,  cmp_ba,  dac_b2a, dsb_b2a,

/* 0x7X */  illegal, illegal, mov_i2r, and_i2r, or_i2r,  xor_i2r, btjo_i2r,btjz_i2r,
            add_i2r, adc_i2r, sub_ir,  sbb_ir,  mpy_ir,  cmp_ir,  dac_i2r, dsb_i2r,

/* 0x8X */  movp_p2a,illegal, movp_a2p,andp_a2p,orp_a2p, xorp_a2p,btjop_ap,btjzp_ap,
            movd_imm,illegal, lda_dir, sta_dir, br_dir,  cmpa_dir,call_dir,illegal,

/* 0x9X */  illegal, movp_p2b,movp_b2p,andp_b2p,orp_b2p, xorp_b2p,btjop_bp,btjzp_bp,
            movd_r,  illegal, lda_ind, sta_ind, br_ind,  cmpa_ind,call_ind,illegal,

/* 0xAX */  illegal, illegal, movp_i2p,andp_i2p,orp_i2p, xorp_i2p,btjop_ip,btjzp_ip,
            movd_inx,illegal, lda_inx, sta_inx, br_inx,  cmpa_inx,call_inx,illegal,

/* 0xBX */  clrc,    illegal, dec_a,   inc_a,   inv_a,   clr_a,   xchb_a,  swap_a,
            push_a,  pop_a,   djnz_a,  decd_a,  rr_a,    rrc_a,   rl_a,    rlc_a,

/* 0xCX */  mov_a2b, tstb,    dec_b,   inc_b,   inv_b,   clr_b,   xchb_b,  swap_b,
            push_b,  pop_b,   djnz_b,  decd_b,  rr_b,    rrc_b,   rl_b,    rlc_b,

/* 0xDX */  mov_a2r, mov_b2r, dec_r,   inc_r,   inv_r,   clr_r,   xchb_r,  swap_r,
            push_r,  pop_r,   djnz_r,  decd_r,  rr_r,    rrc_r,   rl_r,    rlc_r,

/* 0xEX */  jmp,     j_jn,    jeq,     jc,      jp,      jpz,     jne,     jl,
            trap_23, trap_22, trap_21, trap_20, trap_19, trap_18, trap_17, trap_16,

/* 0xFX */  trap_15, trap_14, trap_13, trap_12, trap_11, trap_10, trap_9,  trap_8,
            trap_7,  trap_6,  trap_5,  trap_4,  trap_3,  trap_2,  trap_1,  trap_0
};

static void (*opfn_exl[0x100])(void) = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */  nop,     idle,    illegal, illegal, illegal, eint,    dint,    setc,
            pop_st,  stsp,    rets,    reti,    illegal, ldsp,    push_st, illegal,

/* 0x1X */  illegal, illegal, mov_r2a, and_r2a, or_r2a,  xor_r2a, btjo_r2a,btjz_r2a,
            add_r2a, adc_r2a, sub_ra,  sbb_ra,  mpy_ra,  cmp_ra,  dac_r2a, dsb_r2a,

/* 0x2X */  illegal, illegal, mov_i2a, and_i2a, or_i2a,  xor_i2a, btjo_i2a,btjz_i2a,
            add_i2a, adc_i2a, sub_ia,  sbb_ia,  mpy_ia,  cmp_ia,  dac_i2a, dsb_i2a,

/* 0x3X */  illegal, illegal, mov_r2b, and_r2b, or_r2b,  xor_r2b, btjo_r2b,btjz_r2b,
            add_r2b, adc_r2b, sub_rb,  sbb_rb,  mpy_rb,  cmp_rb,  dac_r2b, dsb_r2b,

/* 0x4X */  illegal, illegal, mov_r2r, and_r2r, or_r2r,  xor_r2r, btjo_r2r,btjz_r2r,
            add_r2r, adc_r2r, sub_rr,  sbb_rr,  mpy_rr,  cmp_rr,  dac_r2r, dsb_r2r,

/* 0x5X */  illegal, illegal, mov_i2b, and_i2b, or_i2b,  xor_i2b, btjo_i2b,btjz_i2b,
            add_i2b, adc_i2b, sub_ib,  sbb_ib,  mpy_ib,  cmp_ib,  dac_i2b, dsb_i2b,

/* 0x6X */  illegal, illegal, mov_b2a, and_b2a, or_b2a,  xor_b2a, btjo_b2a,btjz_b2a,
            add_b2a, adc_b2a, sub_ba,  sbb_ba,  mpy_ba,  cmp_ba,  dac_b2a, dsb_b2a,

/* 0x7X */  illegal, illegal, mov_i2r, and_i2r, or_i2r,  xor_i2r, btjo_i2r,btjz_i2r,
            add_i2r, adc_i2r, sub_ir,  sbb_ir,  mpy_ir,  cmp_ir,  dac_i2r, dsb_i2r,

/* 0x8X */  movp_p2a,illegal, movp_a2p,andp_a2p,orp_a2p, xorp_a2p,btjop_ap,btjzp_ap,
            movd_imm,illegal, lda_dir, sta_dir, br_dir,  cmpa_dir,call_dir,illegal,

/* 0x9X */  illegal, movp_p2b,movp_b2p,andp_b2p,orp_b2p, xorp_b2p,btjop_bp,btjzp_bp,
            movd_r,  illegal, lda_ind, sta_ind, br_ind,  cmpa_ind,call_ind,illegal,

/* 0xAX */  illegal, illegal, movp_i2p,andp_i2p,orp_i2p, xorp_i2p,btjop_ip,btjzp_ip,
            movd_inx,illegal, lda_inx, sta_inx, br_inx,  cmpa_inx,call_inx,illegal,

/* 0xBX */  clrc,    illegal, dec_a,   inc_a,   inv_a,   clr_a,   xchb_a,  swap_a,
            push_a,  pop_a,   djnz_a,  decd_a,  rr_a,    rrc_a,   rl_a,    rlc_a,

/* 0xCX */  mov_a2b, tstb,    dec_b,   inc_b,   inv_b,   clr_b,   xchb_b,  swap_b,
            push_b,  pop_b,   djnz_b,  decd_b,  rr_b,    rrc_b,   rl_b,    rlc_b,

/* 0xDX */  mov_a2r, mov_b2r, dec_r,   inc_r,   inv_r,   clr_r,   xchb_r,  swap_r_exl,
            push_r,  pop_r,   djnz_r,  decd_r,  rr_r,    rrc_r,   rl_r,    rlc_r,

/* 0xEX */  jmp,     j_jn,    jeq,     jc,      jp,      jpz,     jne,     jl,
            trap_23, trap_22, trap_21, trap_20, trap_19, trap_18, trap_17, trap_16,

/* 0xFX */  trap_15, trap_14, trap_13, trap_12, trap_11, trap_10, trap_9,  trap_8,
            trap_7,  trap_6,  trap_5,  trap_4,  trap_3,  trap_2,  trap_1,  trap_0
};
