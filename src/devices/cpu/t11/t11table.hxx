// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** t11: Portable DEC T-11 emulator ******************************************

    Opcode table

*****************************************************************************/

/*

modes:
  rg = register
  rgd = register deferred
  in = increment
  ind = increment deferred
  de = decrement
  ded = decrement deferred
  ix = index
  ixd = index deferred

*/

#define OP(x)  &t11_device::x

const t11_device::opcode_func t11_device::s_opcode_table[65536 >> 3] =
{
	/* 0x0000 */
	OP(op_0000),    OP(halt),       OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal4),   OP(jmp_rgd),    OP(jmp_in),     OP(jmp_ind),    OP(jmp_de),     OP(jmp_ded),    OP(jmp_ix),     OP(jmp_ixd),
	OP(rts),        OP(illegal),    OP(illegal),    OP(illegal),    OP(ccc),        OP(ccc),        OP(scc),        OP(scc),
	OP(swab_rg),    OP(swab_rgd),   OP(swab_in),    OP(swab_ind),   OP(swab_de),    OP(swab_ded),   OP(swab_ix),    OP(swab_ixd),
	/* 0x0100 */
	OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),
	OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),
	OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),
	OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),         OP(br),
	/* 0x0200 */
	OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),
	OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),
	OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),
	OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),        OP(bne),
	/* 0x0300 */
	OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),
	OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),
	OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),
	OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),        OP(beq),
	/* 0x0400 */
	OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),
	OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),
	OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),
	OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),        OP(bge),
	/* 0x0500 */
	OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),
	OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),
	OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),
	OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),        OP(blt),
	/* 0x0600 */
	OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),
	OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),
	OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),
	OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),        OP(bgt),
	/* 0x0700 */
	OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),
	OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),
	OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),
	OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),        OP(ble),
	/* 0x0800 */
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	/* 0x0900 */
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	OP(illegal4),   OP(jsr_rgd),    OP(jsr_in),     OP(jsr_ind),    OP(jsr_de),     OP(jsr_ded),    OP(jsr_ix),     OP(jsr_ixd),
	/* 0x0a00 */
	OP(clr_rg),     OP(clr_rgd),    OP(clr_in),     OP(clr_ind),    OP(clr_de),     OP(clr_ded),    OP(clr_ix),     OP(clr_ixd),
	OP(com_rg),     OP(com_rgd),    OP(com_in),     OP(com_ind),    OP(com_de),     OP(com_ded),    OP(com_ix),     OP(com_ixd),
	OP(inc_rg),     OP(inc_rgd),    OP(inc_in),     OP(inc_ind),    OP(inc_de),     OP(inc_ded),    OP(inc_ix),     OP(inc_ixd),
	OP(dec_rg),     OP(dec_rgd),    OP(dec_in),     OP(dec_ind),    OP(dec_de),     OP(dec_ded),    OP(dec_ix),     OP(dec_ixd),
	/* 0x0b00 */
	OP(neg_rg),     OP(neg_rgd),    OP(neg_in),     OP(neg_ind),    OP(neg_de),     OP(neg_ded),    OP(neg_ix),     OP(neg_ixd),
	OP(adc_rg),     OP(adc_rgd),    OP(adc_in),     OP(adc_ind),    OP(adc_de),     OP(adc_ded),    OP(adc_ix),     OP(adc_ixd),
	OP(sbc_rg),     OP(sbc_rgd),    OP(sbc_in),     OP(sbc_ind),    OP(sbc_de),     OP(sbc_ded),    OP(sbc_ix),     OP(sbc_ixd),
	OP(tst_rg),     OP(tst_rgd),    OP(tst_in),     OP(tst_ind),    OP(tst_de),     OP(tst_ded),    OP(tst_ix),     OP(tst_ixd),
	/* 0x0c00 */
	OP(ror_rg),     OP(ror_rgd),    OP(ror_in),     OP(ror_ind),    OP(ror_de),     OP(ror_ded),    OP(ror_ix),     OP(ror_ixd),
	OP(rol_rg),     OP(rol_rgd),    OP(rol_in),     OP(rol_ind),    OP(rol_de),     OP(rol_ded),    OP(rol_ix),     OP(rol_ixd),
	OP(asr_rg),     OP(asr_rgd),    OP(asr_in),     OP(asr_ind),    OP(asr_de),     OP(asr_ded),    OP(asr_ix),     OP(asr_ixd),
	OP(asl_rg),     OP(asl_rgd),    OP(asl_in),     OP(asl_ind),    OP(asl_de),     OP(asl_ded),    OP(asl_ix),     OP(asl_ixd),
	/* 0x0d00 */
	OP(mark),       OP(mark),       OP(mark),       OP(mark),       OP(mark),       OP(mark),       OP(mark),       OP(mark),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(sxt_rg),     OP(sxt_rgd),    OP(sxt_in),     OP(sxt_ind),    OP(sxt_de),     OP(sxt_ded),    OP(sxt_ix),     OP(sxt_ixd),
	/* 0x0e00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x0f00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),

	/* 0x1000 */
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	/* 0x1100 */
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	OP(mov_rg_rg),  OP(mov_rg_rgd), OP(mov_rg_in),  OP(mov_rg_ind), OP(mov_rg_de),  OP(mov_rg_ded), OP(mov_rg_ix),  OP(mov_rg_ixd),
	/* 0x1200 */
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	/* 0x1300 */
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	OP(mov_rgd_rg), OP(mov_rgd_rgd), OP(mov_rgd_in), OP(mov_rgd_ind), OP(mov_rgd_de), OP(mov_rgd_ded), OP(mov_rgd_ix), OP(mov_rgd_ixd),
	/* 0x1400 */
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	/* 0x1500 */
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	OP(mov_in_rg),  OP(mov_in_rgd), OP(mov_in_in),  OP(mov_in_ind), OP(mov_in_de),  OP(mov_in_ded), OP(mov_in_ix),  OP(mov_in_ixd),
	/* 0x1600 */
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	/* 0x1700 */
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	OP(mov_ind_rg), OP(mov_ind_rgd), OP(mov_ind_in), OP(mov_ind_ind), OP(mov_ind_de), OP(mov_ind_ded), OP(mov_ind_ix), OP(mov_ind_ixd),
	/* 0x1800 */
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	/* 0x1900 */
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	OP(mov_de_rg),  OP(mov_de_rgd), OP(mov_de_in),  OP(mov_de_ind), OP(mov_de_de),  OP(mov_de_ded), OP(mov_de_ix),  OP(mov_de_ixd),
	/* 0x1a00 */
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	/* 0x1b00 */
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	OP(mov_ded_rg), OP(mov_ded_rgd), OP(mov_ded_in), OP(mov_ded_ind), OP(mov_ded_de), OP(mov_ded_ded), OP(mov_ded_ix), OP(mov_ded_ixd),
	/* 0x1c00 */
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	/* 0x1d00 */
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	OP(mov_ix_rg),  OP(mov_ix_rgd), OP(mov_ix_in),  OP(mov_ix_ind), OP(mov_ix_de),  OP(mov_ix_ded), OP(mov_ix_ix),  OP(mov_ix_ixd),
	/* 0x1e00 */
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	/* 0x1f00 */
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),
	OP(mov_ixd_rg), OP(mov_ixd_rgd), OP(mov_ixd_in), OP(mov_ixd_ind), OP(mov_ixd_de), OP(mov_ixd_ded), OP(mov_ixd_ix), OP(mov_ixd_ixd),

	/* 0x2000 */
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	/* 0x2100 */
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	OP(cmp_rg_rg),  OP(cmp_rg_rgd), OP(cmp_rg_in),  OP(cmp_rg_ind), OP(cmp_rg_de),  OP(cmp_rg_ded), OP(cmp_rg_ix),  OP(cmp_rg_ixd),
	/* 0x2200 */
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	/* 0x2300 */
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	OP(cmp_rgd_rg), OP(cmp_rgd_rgd), OP(cmp_rgd_in), OP(cmp_rgd_ind), OP(cmp_rgd_de), OP(cmp_rgd_ded), OP(cmp_rgd_ix), OP(cmp_rgd_ixd),
	/* 0x2400 */
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	/* 0x2500 */
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	OP(cmp_in_rg),  OP(cmp_in_rgd), OP(cmp_in_in),  OP(cmp_in_ind), OP(cmp_in_de),  OP(cmp_in_ded), OP(cmp_in_ix),  OP(cmp_in_ixd),
	/* 0x2600 */
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	/* 0x2700 */
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	OP(cmp_ind_rg), OP(cmp_ind_rgd), OP(cmp_ind_in), OP(cmp_ind_ind), OP(cmp_ind_de), OP(cmp_ind_ded), OP(cmp_ind_ix), OP(cmp_ind_ixd),
	/* 0x2800 */
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	/* 0x2900 */
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	OP(cmp_de_rg),  OP(cmp_de_rgd), OP(cmp_de_in),  OP(cmp_de_ind), OP(cmp_de_de),  OP(cmp_de_ded), OP(cmp_de_ix),  OP(cmp_de_ixd),
	/* 0x2a00 */
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	/* 0x2b00 */
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	OP(cmp_ded_rg), OP(cmp_ded_rgd), OP(cmp_ded_in), OP(cmp_ded_ind), OP(cmp_ded_de), OP(cmp_ded_ded), OP(cmp_ded_ix), OP(cmp_ded_ixd),
	/* 0x2c00 */
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	/* 0x2d00 */
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	OP(cmp_ix_rg),  OP(cmp_ix_rgd), OP(cmp_ix_in),  OP(cmp_ix_ind), OP(cmp_ix_de),  OP(cmp_ix_ded), OP(cmp_ix_ix),  OP(cmp_ix_ixd),
	/* 0x2e00 */
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	/* 0x2f00 */
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),
	OP(cmp_ixd_rg), OP(cmp_ixd_rgd), OP(cmp_ixd_in), OP(cmp_ixd_ind), OP(cmp_ixd_de), OP(cmp_ixd_ded), OP(cmp_ixd_ix), OP(cmp_ixd_ixd),

	/* 0x3000 */
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	/* 0x3100 */
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	OP(bit_rg_rg),  OP(bit_rg_rgd), OP(bit_rg_in),  OP(bit_rg_ind), OP(bit_rg_de),  OP(bit_rg_ded), OP(bit_rg_ix),  OP(bit_rg_ixd),
	/* 0x3200 */
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	/* 0x3300 */
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	OP(bit_rgd_rg), OP(bit_rgd_rgd), OP(bit_rgd_in), OP(bit_rgd_ind), OP(bit_rgd_de), OP(bit_rgd_ded), OP(bit_rgd_ix), OP(bit_rgd_ixd),
	/* 0x3400 */
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	/* 0x3500 */
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	OP(bit_in_rg),  OP(bit_in_rgd), OP(bit_in_in),  OP(bit_in_ind), OP(bit_in_de),  OP(bit_in_ded), OP(bit_in_ix),  OP(bit_in_ixd),
	/* 0x3600 */
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	/* 0x3700 */
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	OP(bit_ind_rg), OP(bit_ind_rgd), OP(bit_ind_in), OP(bit_ind_ind), OP(bit_ind_de), OP(bit_ind_ded), OP(bit_ind_ix), OP(bit_ind_ixd),
	/* 0x3800 */
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	/* 0x3900 */
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	OP(bit_de_rg),  OP(bit_de_rgd), OP(bit_de_in),  OP(bit_de_ind), OP(bit_de_de),  OP(bit_de_ded), OP(bit_de_ix),  OP(bit_de_ixd),
	/* 0x3a00 */
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	/* 0x3b00 */
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	OP(bit_ded_rg), OP(bit_ded_rgd), OP(bit_ded_in), OP(bit_ded_ind), OP(bit_ded_de), OP(bit_ded_ded), OP(bit_ded_ix), OP(bit_ded_ixd),
	/* 0x3c00 */
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	/* 0x3d00 */
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	OP(bit_ix_rg),  OP(bit_ix_rgd), OP(bit_ix_in),  OP(bit_ix_ind), OP(bit_ix_de),  OP(bit_ix_ded), OP(bit_ix_ix),  OP(bit_ix_ixd),
	/* 0x3e00 */
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	/* 0x3f00 */
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),
	OP(bit_ixd_rg), OP(bit_ixd_rgd), OP(bit_ixd_in), OP(bit_ixd_ind), OP(bit_ixd_de), OP(bit_ixd_ded), OP(bit_ixd_ix), OP(bit_ixd_ixd),

	/* 0x4000 */
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	/* 0x4100 */
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	OP(bic_rg_rg),  OP(bic_rg_rgd), OP(bic_rg_in),  OP(bic_rg_ind), OP(bic_rg_de),  OP(bic_rg_ded), OP(bic_rg_ix),  OP(bic_rg_ixd),
	/* 0x4200 */
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	/* 0x4300 */
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	OP(bic_rgd_rg), OP(bic_rgd_rgd), OP(bic_rgd_in), OP(bic_rgd_ind), OP(bic_rgd_de), OP(bic_rgd_ded), OP(bic_rgd_ix), OP(bic_rgd_ixd),
	/* 0x4400 */
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	/* 0x4500 */
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	OP(bic_in_rg),  OP(bic_in_rgd), OP(bic_in_in),  OP(bic_in_ind), OP(bic_in_de),  OP(bic_in_ded), OP(bic_in_ix),  OP(bic_in_ixd),
	/* 0x4600 */
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	/* 0x4700 */
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	OP(bic_ind_rg), OP(bic_ind_rgd), OP(bic_ind_in), OP(bic_ind_ind), OP(bic_ind_de), OP(bic_ind_ded), OP(bic_ind_ix), OP(bic_ind_ixd),
	/* 0x4800 */
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	/* 0x4900 */
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	OP(bic_de_rg),  OP(bic_de_rgd), OP(bic_de_in),  OP(bic_de_ind), OP(bic_de_de),  OP(bic_de_ded), OP(bic_de_ix),  OP(bic_de_ixd),
	/* 0x4a00 */
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	/* 0x4b00 */
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	OP(bic_ded_rg), OP(bic_ded_rgd), OP(bic_ded_in), OP(bic_ded_ind), OP(bic_ded_de), OP(bic_ded_ded), OP(bic_ded_ix), OP(bic_ded_ixd),
	/* 0x4c00 */
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	/* 0x4d00 */
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	OP(bic_ix_rg),  OP(bic_ix_rgd), OP(bic_ix_in),  OP(bic_ix_ind), OP(bic_ix_de),  OP(bic_ix_ded), OP(bic_ix_ix),  OP(bic_ix_ixd),
	/* 0x4e00 */
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	/* 0x4f00 */
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),
	OP(bic_ixd_rg), OP(bic_ixd_rgd), OP(bic_ixd_in), OP(bic_ixd_ind), OP(bic_ixd_de), OP(bic_ixd_ded), OP(bic_ixd_ix), OP(bic_ixd_ixd),

	/* 0x5000 */
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	/* 0x5100 */
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	OP(bis_rg_rg),  OP(bis_rg_rgd), OP(bis_rg_in),  OP(bis_rg_ind), OP(bis_rg_de),  OP(bis_rg_ded), OP(bis_rg_ix),  OP(bis_rg_ixd),
	/* 0x5200 */
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	/* 0x5300 */
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	OP(bis_rgd_rg), OP(bis_rgd_rgd), OP(bis_rgd_in), OP(bis_rgd_ind), OP(bis_rgd_de), OP(bis_rgd_ded), OP(bis_rgd_ix), OP(bis_rgd_ixd),
	/* 0x5400 */
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	/* 0x5500 */
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	OP(bis_in_rg),  OP(bis_in_rgd), OP(bis_in_in),  OP(bis_in_ind), OP(bis_in_de),  OP(bis_in_ded), OP(bis_in_ix),  OP(bis_in_ixd),
	/* 0x5600 */
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	/* 0x5700 */
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	OP(bis_ind_rg), OP(bis_ind_rgd), OP(bis_ind_in), OP(bis_ind_ind), OP(bis_ind_de), OP(bis_ind_ded), OP(bis_ind_ix), OP(bis_ind_ixd),
	/* 0x5800 */
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	/* 0x5900 */
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	OP(bis_de_rg),  OP(bis_de_rgd), OP(bis_de_in),  OP(bis_de_ind), OP(bis_de_de),  OP(bis_de_ded), OP(bis_de_ix),  OP(bis_de_ixd),
	/* 0x5a00 */
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	/* 0x5b00 */
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	OP(bis_ded_rg), OP(bis_ded_rgd), OP(bis_ded_in), OP(bis_ded_ind), OP(bis_ded_de), OP(bis_ded_ded), OP(bis_ded_ix), OP(bis_ded_ixd),
	/* 0x5c00 */
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	/* 0x5d00 */
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	OP(bis_ix_rg),  OP(bis_ix_rgd), OP(bis_ix_in),  OP(bis_ix_ind), OP(bis_ix_de),  OP(bis_ix_ded), OP(bis_ix_ix),  OP(bis_ix_ixd),
	/* 0x5e00 */
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	/* 0x5f00 */
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),
	OP(bis_ixd_rg), OP(bis_ixd_rgd), OP(bis_ixd_in), OP(bis_ixd_ind), OP(bis_ixd_de), OP(bis_ixd_ded), OP(bis_ixd_ix), OP(bis_ixd_ixd),

	/* 0x6000 */
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	/* 0x6100 */
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	OP(add_rg_rg),  OP(add_rg_rgd), OP(add_rg_in),  OP(add_rg_ind), OP(add_rg_de),  OP(add_rg_ded), OP(add_rg_ix),  OP(add_rg_ixd),
	/* 0x6200 */
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	/* 0x6300 */
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	OP(add_rgd_rg), OP(add_rgd_rgd), OP(add_rgd_in), OP(add_rgd_ind), OP(add_rgd_de), OP(add_rgd_ded), OP(add_rgd_ix), OP(add_rgd_ixd),
	/* 0x6400 */
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	/* 0x6500 */
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	OP(add_in_rg),  OP(add_in_rgd), OP(add_in_in),  OP(add_in_ind), OP(add_in_de),  OP(add_in_ded), OP(add_in_ix),  OP(add_in_ixd),
	/* 0x6600 */
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	/* 0x6700 */
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	OP(add_ind_rg), OP(add_ind_rgd), OP(add_ind_in), OP(add_ind_ind), OP(add_ind_de), OP(add_ind_ded), OP(add_ind_ix), OP(add_ind_ixd),
	/* 0x6800 */
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	/* 0x6900 */
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	OP(add_de_rg),  OP(add_de_rgd), OP(add_de_in),  OP(add_de_ind), OP(add_de_de),  OP(add_de_ded), OP(add_de_ix),  OP(add_de_ixd),
	/* 0x6a00 */
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	/* 0x6b00 */
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	OP(add_ded_rg), OP(add_ded_rgd), OP(add_ded_in), OP(add_ded_ind), OP(add_ded_de), OP(add_ded_ded), OP(add_ded_ix), OP(add_ded_ixd),
	/* 0x6c00 */
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	/* 0x6d00 */
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	OP(add_ix_rg),  OP(add_ix_rgd), OP(add_ix_in),  OP(add_ix_ind), OP(add_ix_de),  OP(add_ix_ded), OP(add_ix_ix),  OP(add_ix_ixd),
	/* 0x6e00 */
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	/* 0x6f00 */
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),
	OP(add_ixd_rg), OP(add_ixd_rgd), OP(add_ixd_in), OP(add_ixd_ind), OP(add_ixd_de), OP(add_ixd_ded), OP(add_ixd_ix), OP(add_ixd_ixd),

	/* 0x7000 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7100 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7200 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7300 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7400 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7500 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7600 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7700 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7800 */
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	/* 0x7900 */
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	OP(xor_rg),     OP(xor_rgd),    OP(xor_in),     OP(xor_ind),    OP(xor_de),     OP(xor_ded),    OP(xor_ix),     OP(xor_ixd),
	/* 0x7a00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7b00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7c00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7d00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x7e00 */
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	/* 0x7f00 */
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),
	OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),        OP(sob),

	/* 0x8000 */
	OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),
	OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),
	OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),
	OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),        OP(bpl),
	/* 0x8100 */
	OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),
	OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),
	OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),
	OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),        OP(bmi),
	/* 0x8200 */
	OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),
	OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),
	OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),
	OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),        OP(bhi),
	/* 0x8300 */
	OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),
	OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),
	OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),
	OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),       OP(blos),
	/* 0x8400 */
	OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),
	OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),
	OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),
	OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),        OP(bvc),
	/* 0x8500 */
	OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),
	OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),
	OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),
	OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),        OP(bvs),
	/* 0x8600 */
	OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),
	OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),
	OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),
	OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),        OP(bcc),
	/* 0x8700 */
	OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),
	OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),
	OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),
	OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),        OP(bcs),
	/* 0x8800 */
	OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),
	OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),
	OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),
	OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),        OP(emt),
	/* 0x8900 */
	OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),
	OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),
	OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),
	OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),       OP(trap),
	/* 0x8a00 */
	OP(clrb_rg),    OP(clrb_rgd),   OP(clrb_in),    OP(clrb_ind),   OP(clrb_de),    OP(clrb_ded),   OP(clrb_ix),    OP(clrb_ixd),
	OP(comb_rg),    OP(comb_rgd),   OP(comb_in),    OP(comb_ind),   OP(comb_de),    OP(comb_ded),   OP(comb_ix),    OP(comb_ixd),
	OP(incb_rg),    OP(incb_rgd),   OP(incb_in),    OP(incb_ind),   OP(incb_de),    OP(incb_ded),   OP(incb_ix),    OP(incb_ixd),
	OP(decb_rg),    OP(decb_rgd),   OP(decb_in),    OP(decb_ind),   OP(decb_de),    OP(decb_ded),   OP(decb_ix),    OP(decb_ixd),
	/* 0x8b00 */
	OP(negb_rg),    OP(negb_rgd),   OP(negb_in),    OP(negb_ind),   OP(negb_de),    OP(negb_ded),   OP(negb_ix),    OP(negb_ixd),
	OP(adcb_rg),    OP(adcb_rgd),   OP(adcb_in),    OP(adcb_ind),   OP(adcb_de),    OP(adcb_ded),   OP(adcb_ix),    OP(adcb_ixd),
	OP(sbcb_rg),    OP(sbcb_rgd),   OP(sbcb_in),    OP(sbcb_ind),   OP(sbcb_de),    OP(sbcb_ded),   OP(sbcb_ix),    OP(sbcb_ixd),
	OP(tstb_rg),    OP(tstb_rgd),   OP(tstb_in),    OP(tstb_ind),   OP(tstb_de),    OP(tstb_ded),   OP(tstb_ix),    OP(tstb_ixd),
	/* 0x8c00 */
	OP(rorb_rg),    OP(rorb_rgd),   OP(rorb_in),    OP(rorb_ind),   OP(rorb_de),    OP(rorb_ded),   OP(rorb_ix),    OP(rorb_ixd),
	OP(rolb_rg),    OP(rolb_rgd),   OP(rolb_in),    OP(rolb_ind),   OP(rolb_de),    OP(rolb_ded),   OP(rolb_ix),    OP(rolb_ixd),
	OP(asrb_rg),    OP(asrb_rgd),   OP(asrb_in),    OP(asrb_ind),   OP(asrb_de),    OP(asrb_ded),   OP(asrb_ix),    OP(asrb_ixd),
	OP(aslb_rg),    OP(aslb_rgd),   OP(aslb_in),    OP(aslb_ind),   OP(aslb_de),    OP(aslb_ded),   OP(aslb_ix),    OP(aslb_ixd),
	/* 0x8d00 */
	OP(mtps_rg),    OP(mtps_rgd),   OP(mtps_in),    OP(mtps_ind),   OP(mtps_de),    OP(mtps_ded),   OP(mtps_ix),    OP(mtps_ixd),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(mfps_rg),    OP(mfps_rgd),   OP(mfps_in),    OP(mfps_ind),   OP(mfps_de),    OP(mfps_ded),   OP(mfps_ix),    OP(mfps_ixd),
	/* 0x8e00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0x8f00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),

	/* 0x9000 */
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	/* 0x9100 */
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	OP(movb_rg_rg), OP(movb_rg_rgd), OP(movb_rg_in), OP(movb_rg_ind), OP(movb_rg_de), OP(movb_rg_ded), OP(movb_rg_ix), OP(movb_rg_ixd),
	/* 0x9200 */
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	/* 0x9300 */
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	OP(movb_rgd_rg), OP(movb_rgd_rgd), OP(movb_rgd_in), OP(movb_rgd_ind), OP(movb_rgd_de), OP(movb_rgd_ded), OP(movb_rgd_ix), OP(movb_rgd_ixd),
	/* 0x9400 */
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	/* 0x9500 */
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	OP(movb_in_rg), OP(movb_in_rgd), OP(movb_in_in), OP(movb_in_ind), OP(movb_in_de), OP(movb_in_ded), OP(movb_in_ix), OP(movb_in_ixd),
	/* 0x9600 */
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	/* 0x9700 */
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	OP(movb_ind_rg), OP(movb_ind_rgd), OP(movb_ind_in), OP(movb_ind_ind), OP(movb_ind_de), OP(movb_ind_ded), OP(movb_ind_ix), OP(movb_ind_ixd),
	/* 0x9800 */
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	/* 0x9900 */
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	OP(movb_de_rg), OP(movb_de_rgd), OP(movb_de_in), OP(movb_de_ind), OP(movb_de_de), OP(movb_de_ded), OP(movb_de_ix), OP(movb_de_ixd),
	/* 0x9a00 */
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	/* 0x9b00 */
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	OP(movb_ded_rg), OP(movb_ded_rgd), OP(movb_ded_in), OP(movb_ded_ind), OP(movb_ded_de), OP(movb_ded_ded), OP(movb_ded_ix), OP(movb_ded_ixd),
	/* 0x9c00 */
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	/* 0x9d00 */
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	OP(movb_ix_rg), OP(movb_ix_rgd), OP(movb_ix_in), OP(movb_ix_ind), OP(movb_ix_de), OP(movb_ix_ded), OP(movb_ix_ix), OP(movb_ix_ixd),
	/* 0x9e00 */
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	/* 0x9f00 */
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),
	OP(movb_ixd_rg), OP(movb_ixd_rgd), OP(movb_ixd_in), OP(movb_ixd_ind), OP(movb_ixd_de), OP(movb_ixd_ded), OP(movb_ixd_ix), OP(movb_ixd_ixd),

	/* 0xa000 */
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	/* 0xa100 */
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	OP(cmpb_rg_rg), OP(cmpb_rg_rgd), OP(cmpb_rg_in), OP(cmpb_rg_ind), OP(cmpb_rg_de), OP(cmpb_rg_ded), OP(cmpb_rg_ix), OP(cmpb_rg_ixd),
	/* 0xa200 */
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	/* 0xa300 */
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	OP(cmpb_rgd_rg), OP(cmpb_rgd_rgd), OP(cmpb_rgd_in), OP(cmpb_rgd_ind), OP(cmpb_rgd_de), OP(cmpb_rgd_ded), OP(cmpb_rgd_ix), OP(cmpb_rgd_ixd),
	/* 0xa400 */
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	/* 0xa500 */
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	OP(cmpb_in_rg), OP(cmpb_in_rgd), OP(cmpb_in_in), OP(cmpb_in_ind), OP(cmpb_in_de), OP(cmpb_in_ded), OP(cmpb_in_ix), OP(cmpb_in_ixd),
	/* 0xa600 */
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	/* 0xa700 */
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	OP(cmpb_ind_rg), OP(cmpb_ind_rgd), OP(cmpb_ind_in), OP(cmpb_ind_ind), OP(cmpb_ind_de), OP(cmpb_ind_ded), OP(cmpb_ind_ix), OP(cmpb_ind_ixd),
	/* 0xa800 */
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	/* 0xa900 */
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	OP(cmpb_de_rg), OP(cmpb_de_rgd), OP(cmpb_de_in), OP(cmpb_de_ind), OP(cmpb_de_de), OP(cmpb_de_ded), OP(cmpb_de_ix), OP(cmpb_de_ixd),
	/* 0xaa00 */
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	/* 0xab00 */
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	OP(cmpb_ded_rg), OP(cmpb_ded_rgd), OP(cmpb_ded_in), OP(cmpb_ded_ind), OP(cmpb_ded_de), OP(cmpb_ded_ded), OP(cmpb_ded_ix), OP(cmpb_ded_ixd),
	/* 0xac00 */
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	/* 0xad00 */
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	OP(cmpb_ix_rg), OP(cmpb_ix_rgd), OP(cmpb_ix_in), OP(cmpb_ix_ind), OP(cmpb_ix_de), OP(cmpb_ix_ded), OP(cmpb_ix_ix), OP(cmpb_ix_ixd),
	/* 0xae00 */
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	/* 0xaf00 */
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),
	OP(cmpb_ixd_rg), OP(cmpb_ixd_rgd), OP(cmpb_ixd_in), OP(cmpb_ixd_ind), OP(cmpb_ixd_de), OP(cmpb_ixd_ded), OP(cmpb_ixd_ix), OP(cmpb_ixd_ixd),

	/* 0xb000 */
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	/* 0xb100 */
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	OP(bitb_rg_rg), OP(bitb_rg_rgd), OP(bitb_rg_in), OP(bitb_rg_ind), OP(bitb_rg_de), OP(bitb_rg_ded), OP(bitb_rg_ix), OP(bitb_rg_ixd),
	/* 0xb200 */
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	/* 0xb300 */
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	OP(bitb_rgd_rg), OP(bitb_rgd_rgd), OP(bitb_rgd_in), OP(bitb_rgd_ind), OP(bitb_rgd_de), OP(bitb_rgd_ded), OP(bitb_rgd_ix), OP(bitb_rgd_ixd),
	/* 0xb400 */
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	/* 0xb500 */
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	OP(bitb_in_rg), OP(bitb_in_rgd), OP(bitb_in_in), OP(bitb_in_ind), OP(bitb_in_de), OP(bitb_in_ded), OP(bitb_in_ix), OP(bitb_in_ixd),
	/* 0xb600 */
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	/* 0xb700 */
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	OP(bitb_ind_rg), OP(bitb_ind_rgd), OP(bitb_ind_in), OP(bitb_ind_ind), OP(bitb_ind_de), OP(bitb_ind_ded), OP(bitb_ind_ix), OP(bitb_ind_ixd),
	/* 0xb800 */
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	/* 0xb900 */
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	OP(bitb_de_rg), OP(bitb_de_rgd), OP(bitb_de_in), OP(bitb_de_ind), OP(bitb_de_de), OP(bitb_de_ded), OP(bitb_de_ix), OP(bitb_de_ixd),
	/* 0xba00 */
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	/* 0xbb00 */
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	OP(bitb_ded_rg), OP(bitb_ded_rgd), OP(bitb_ded_in), OP(bitb_ded_ind), OP(bitb_ded_de), OP(bitb_ded_ded), OP(bitb_ded_ix), OP(bitb_ded_ixd),
	/* 0xbc00 */
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	/* 0xbd00 */
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	OP(bitb_ix_rg), OP(bitb_ix_rgd), OP(bitb_ix_in), OP(bitb_ix_ind), OP(bitb_ix_de), OP(bitb_ix_ded), OP(bitb_ix_ix), OP(bitb_ix_ixd),
	/* 0xbe00 */
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	/* 0xbf00 */
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),
	OP(bitb_ixd_rg), OP(bitb_ixd_rgd), OP(bitb_ixd_in), OP(bitb_ixd_ind), OP(bitb_ixd_de), OP(bitb_ixd_ded), OP(bitb_ixd_ix), OP(bitb_ixd_ixd),

	/* 0xc000 */
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	/* 0xc100 */
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	OP(bicb_rg_rg), OP(bicb_rg_rgd), OP(bicb_rg_in), OP(bicb_rg_ind), OP(bicb_rg_de), OP(bicb_rg_ded), OP(bicb_rg_ix), OP(bicb_rg_ixd),
	/* 0xc200 */
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	/* 0xc300 */
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	OP(bicb_rgd_rg), OP(bicb_rgd_rgd), OP(bicb_rgd_in), OP(bicb_rgd_ind), OP(bicb_rgd_de), OP(bicb_rgd_ded), OP(bicb_rgd_ix), OP(bicb_rgd_ixd),
	/* 0xc400 */
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	/* 0xc500 */
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	OP(bicb_in_rg), OP(bicb_in_rgd), OP(bicb_in_in), OP(bicb_in_ind), OP(bicb_in_de), OP(bicb_in_ded), OP(bicb_in_ix), OP(bicb_in_ixd),
	/* 0xc600 */
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	/* 0xc700 */
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	OP(bicb_ind_rg), OP(bicb_ind_rgd), OP(bicb_ind_in), OP(bicb_ind_ind), OP(bicb_ind_de), OP(bicb_ind_ded), OP(bicb_ind_ix), OP(bicb_ind_ixd),
	/* 0xc800 */
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	/* 0xc900 */
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	OP(bicb_de_rg), OP(bicb_de_rgd), OP(bicb_de_in), OP(bicb_de_ind), OP(bicb_de_de), OP(bicb_de_ded), OP(bicb_de_ix), OP(bicb_de_ixd),
	/* 0xca00 */
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	/* 0xcb00 */
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	OP(bicb_ded_rg), OP(bicb_ded_rgd), OP(bicb_ded_in), OP(bicb_ded_ind), OP(bicb_ded_de), OP(bicb_ded_ded), OP(bicb_ded_ix), OP(bicb_ded_ixd),
	/* 0xcc00 */
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	/* 0xcd00 */
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	OP(bicb_ix_rg), OP(bicb_ix_rgd), OP(bicb_ix_in), OP(bicb_ix_ind), OP(bicb_ix_de), OP(bicb_ix_ded), OP(bicb_ix_ix), OP(bicb_ix_ixd),
	/* 0xce00 */
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	/* 0xcf00 */
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),
	OP(bicb_ixd_rg), OP(bicb_ixd_rgd), OP(bicb_ixd_in), OP(bicb_ixd_ind), OP(bicb_ixd_de), OP(bicb_ixd_ded), OP(bicb_ixd_ix), OP(bicb_ixd_ixd),

	/* 0xd000 */
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	/* 0xd100 */
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	OP(bisb_rg_rg), OP(bisb_rg_rgd), OP(bisb_rg_in), OP(bisb_rg_ind), OP(bisb_rg_de), OP(bisb_rg_ded), OP(bisb_rg_ix), OP(bisb_rg_ixd),
	/* 0xd200 */
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	/* 0xd300 */
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	OP(bisb_rgd_rg), OP(bisb_rgd_rgd), OP(bisb_rgd_in), OP(bisb_rgd_ind), OP(bisb_rgd_de), OP(bisb_rgd_ded), OP(bisb_rgd_ix), OP(bisb_rgd_ixd),
	/* 0xd400 */
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	/* 0xd500 */
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	OP(bisb_in_rg), OP(bisb_in_rgd), OP(bisb_in_in), OP(bisb_in_ind), OP(bisb_in_de), OP(bisb_in_ded), OP(bisb_in_ix), OP(bisb_in_ixd),
	/* 0xd600 */
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	/* 0xd700 */
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	OP(bisb_ind_rg), OP(bisb_ind_rgd), OP(bisb_ind_in), OP(bisb_ind_ind), OP(bisb_ind_de), OP(bisb_ind_ded), OP(bisb_ind_ix), OP(bisb_ind_ixd),
	/* 0xd800 */
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	/* 0xd900 */
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	OP(bisb_de_rg), OP(bisb_de_rgd), OP(bisb_de_in), OP(bisb_de_ind), OP(bisb_de_de), OP(bisb_de_ded), OP(bisb_de_ix), OP(bisb_de_ixd),
	/* 0xda00 */
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	/* 0xdb00 */
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	OP(bisb_ded_rg), OP(bisb_ded_rgd), OP(bisb_ded_in), OP(bisb_ded_ind), OP(bisb_ded_de), OP(bisb_ded_ded), OP(bisb_ded_ix), OP(bisb_ded_ixd),
	/* 0xdc00 */
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	/* 0xdd00 */
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	OP(bisb_ix_rg), OP(bisb_ix_rgd), OP(bisb_ix_in), OP(bisb_ix_ind), OP(bisb_ix_de), OP(bisb_ix_ded), OP(bisb_ix_ix), OP(bisb_ix_ixd),
	/* 0xde00 */
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	/* 0xdf00 */
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),
	OP(bisb_ixd_rg), OP(bisb_ixd_rgd), OP(bisb_ixd_in), OP(bisb_ixd_ind), OP(bisb_ixd_de), OP(bisb_ixd_ded), OP(bisb_ixd_ix), OP(bisb_ixd_ixd),

	/* 0xe000 */
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	/* 0xe100 */
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	OP(sub_rg_rg),  OP(sub_rg_rgd), OP(sub_rg_in),  OP(sub_rg_ind), OP(sub_rg_de),  OP(sub_rg_ded), OP(sub_rg_ix),  OP(sub_rg_ixd),
	/* 0xe200 */
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	/* 0xe300 */
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	OP(sub_rgd_rg), OP(sub_rgd_rgd), OP(sub_rgd_in), OP(sub_rgd_ind), OP(sub_rgd_de), OP(sub_rgd_ded), OP(sub_rgd_ix), OP(sub_rgd_ixd),
	/* 0xe400 */
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	/* 0xe500 */
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	OP(sub_in_rg),  OP(sub_in_rgd), OP(sub_in_in),  OP(sub_in_ind), OP(sub_in_de),  OP(sub_in_ded), OP(sub_in_ix),  OP(sub_in_ixd),
	/* 0xe600 */
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	/* 0xe700 */
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	OP(sub_ind_rg), OP(sub_ind_rgd), OP(sub_ind_in), OP(sub_ind_ind), OP(sub_ind_de), OP(sub_ind_ded), OP(sub_ind_ix), OP(sub_ind_ixd),
	/* 0xe800 */
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	/* 0xe900 */
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	OP(sub_de_rg),  OP(sub_de_rgd), OP(sub_de_in),  OP(sub_de_ind), OP(sub_de_de),  OP(sub_de_ded), OP(sub_de_ix),  OP(sub_de_ixd),
	/* 0xea00 */
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	/* 0xeb00 */
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	OP(sub_ded_rg), OP(sub_ded_rgd), OP(sub_ded_in), OP(sub_ded_ind), OP(sub_ded_de), OP(sub_ded_ded), OP(sub_ded_ix), OP(sub_ded_ixd),
	/* 0xec00 */
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	/* 0xed00 */
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	OP(sub_ix_rg),  OP(sub_ix_rgd), OP(sub_ix_in),  OP(sub_ix_ind), OP(sub_ix_de),  OP(sub_ix_ded), OP(sub_ix_ix),  OP(sub_ix_ixd),
	/* 0xee00 */
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	/* 0xef00 */
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),
	OP(sub_ixd_rg), OP(sub_ixd_rgd), OP(sub_ixd_in), OP(sub_ixd_ind), OP(sub_ixd_de), OP(sub_ixd_ded), OP(sub_ixd_ix), OP(sub_ixd_ixd),

	/* 0xf000 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf100 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf200 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf300 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf400 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf500 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf600 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf700 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf800 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xf900 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xfa00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xfb00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xfc00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xfd00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xfe00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	/* 0xff00 */
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),
	OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal),    OP(illegal)
};
