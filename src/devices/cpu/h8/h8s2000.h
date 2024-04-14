// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2000.h

    H8S-2000 base cpu emulation

    Adds the exr register, the move multiple instructions, the shifts by
    two, tas and optionally the trace mode.


***************************************************************************/

#ifndef MAME_CPU_H8_H8S2000_H
#define MAME_CPU_H8_H8S2000_H

#pragma once

#include "h8h.h"
#include "h8_dtc.h"

class h8s2000_device : public h8h_device {
protected:
	h8s2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate);

	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 19; }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	O(andc_imm8_exr);
	O(band_imm3_abs16); O(band_imm3_abs32);
	O(bclr_imm3_abs16); O(bclr_r8h_abs16); O(bclr_imm3_abs32); O(bclr_r8h_abs32);
	O(biand_imm3_abs16); O(biand_imm3_abs32);
	O(bild_imm3_abs16); O(bild_imm3_abs32);
	O(bior_imm3_abs16); O(bior_imm3_abs32);
	O(bist_imm3_abs16); O(bist_imm3_abs32);
	O(bixor_imm3_abs16); O(bixor_imm3_abs32);
	O(bld_imm3_abs16); O(bld_imm3_abs32);
	O(bnot_imm3_abs16); O(bnot_r8h_abs16); O(bnot_imm3_abs32); O(bnot_r8h_abs32);
	O(bor_imm3_abs16); O(bor_imm3_abs32);
	O(bset_imm3_abs16); O(bset_r8h_abs16); O(bset_imm3_abs32); O(bset_r8h_abs32);
	O(bst_imm3_abs16); O(bst_imm3_abs32);
	O(btst_imm3_abs16); O(btst_r8h_abs16); O(btst_imm3_abs32); O(btst_r8h_abs32);
	O(bxor_imm3_abs16); O(bxor_imm3_abs32);
	O(ldc_imm8_exr); O(ldc_r8l_exr); O(ldc_w_abs16_exr); O(ldc_w_abs32_exr); O(ldc_w_r32d16h_exr); O(ldc_w_r32d32hh_exr); O(ldc_w_r32ih_exr); O(ldc_w_r32ph_exr);
	O(ldm_l_spp_r32n2l); O(ldm_l_spp_r32n3l); O(ldm_l_spp_r32n4l);
	O(orc_imm8_exr);
	O(rotl_b_two_r8l); O(rotl_l_two_r32l); O(rotl_w_two_r16l);
	O(rotr_b_two_r8l); O(rotr_l_two_r32l); O(rotr_w_two_r16l);
	O(rotxl_b_two_r8l); O(rotxl_l_two_r32l); O(rotxl_w_two_r16l);
	O(rotxr_b_two_r8l); O(rotxr_l_two_r32l); O(rotxr_w_two_r16l);
	O(shal_b_two_r8l); O(shal_l_two_r32l); O(shal_w_two_r16l);
	O(shar_b_two_r8l); O(shar_l_two_r32l); O(shar_w_two_r16l);
	O(shll_b_two_r8l); O(shll_l_two_r32l); O(shll_w_two_r16l);
	O(shlr_b_two_r8l); O(shlr_l_two_r32l); O(shlr_w_two_r16l);
	O(stc_exr_r8l);O(stc_w_exr_abs16); O(stc_w_exr_abs32); O(stc_w_exr_pr32h); O(stc_w_exr_r32d16h); O(stc_w_exr_r32d32hh); O(stc_w_exr_r32ih);
	O(stm_l_r32n2l_psp); O(stm_l_r32n3l_psp); O(stm_l_r32n4l_psp);
	O(tas_r32ih);
	O(xorc_imm8_exr);

	O(state_trace);
	O(state_dtc);
	O(state_dtc_vector);
	O(state_dtc_writeback);
#undef O
};

#endif
