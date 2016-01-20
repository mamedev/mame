// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8h.h

    H8-300H base cpu emulation

    Adds 32-bits support and a bunch of other stuff.


***************************************************************************/

#ifndef __H8H_H__
#define __H8H_H__

#include "h8.h"

class h8h_device : public h8_device {
public:
	h8h_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, address_map_delegate map_delegate);

protected:
	static const disasm_entry disasm_entries[];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	inline void r32_w(int reg, UINT32 val) { R[reg & 7] = val; R[(reg & 7) | 8] = val >> 16; }
	inline UINT32 r32_r(int reg) const { return R[reg & 7] | (R[(reg & 7) | 8] << 16); }

#define O(o) void o ## _full(); void o ## _partial()

	O(add_l_r32h_r32l); O(add_l_imm32_r32l);
	O(adds_l_four_r32l); O(adds_l_one_r32l); O(adds_l_two_r32l);
	O(and_l_r32h_r32l); O(and_l_imm32_r32l); O(and_w_r16h_r16l);
	O(band_imm3_abs32); O(band_imm3_r32ihh);
	O(bcc_rel16);
	O(bclr_imm3_abs32); O(bclr_imm3_r32ihh); O(bclr_r8h_abs32); O(bclr_r8h_r32ihh);
	O(bcs_rel16);
	O(beq_rel16);
	O(bf_rel16);
	O(bge_rel16);
	O(bgt_rel16);
	O(bhi_rel16);
	O(biand_imm3_abs32); O(biand_imm3_r32ihh);
	O(bild_imm3_abs32); O(bild_imm3_r32ihh);
	O(bior_imm3_abs32); O(bior_imm3_r32ihh);
	O(bist_imm3_abs32); O(bist_imm3_r32ihh);
	O(bixor_imm3_abs32); O(bixor_imm3_r32ihh);
	O(bld_imm3_abs32); O(bld_imm3_r32ihh);
	O(ble_rel16);
	O(bls_rel16);
	O(blt_rel16);
	O(bmi_rel16);
	O(bne_rel16);
	O(bnot_imm3_abs32); O(bnot_imm3_r32ihh); O(bnot_r8h_abs32); O(bnot_r8h_r32ihh);
	O(bor_imm3_abs32); O(bor_imm3_r32ihh);
	O(bpl_rel16);
	O(bset_imm3_abs32); O(bset_imm3_r32ihh); O(bset_r8h_abs32); O(bset_r8h_r32ihh);
	O(bsr_rel16); O(bsr_rel8);
	O(bst_imm3_abs32); O(bst_imm3_r32ihh);
	O(bt_rel16);
	O(btst_imm3_abs32); O(btst_imm3_r32ihh); O(btst_r8h_abs32); O(btst_r8h_r32ihh);
	O(bvc_rel16);
	O(bvs_rel16);
	O(bxor_imm3_abs32); O(bxor_imm3_r32ihh);
	O(cmp_l_r32h_r32l); O(cmp_l_imm32_r32l);
	O(dec_l_one_r32l); O(dec_l_two_r32l);
	O(divxs_b_r8h_r16l); O(divxs_w_r16h_r32l);
	O(divxu_w_r16h_r32l);
	O(eepmov_b); O(eepmov_w);
	O(exts_b_r8l); O(exts_l_r32l); O(exts_w_r16l);
	O(extu_b_r8l); O(extu_l_r32l); O(extu_w_r16l);
	O(inc_l_one_r32l); O(inc_l_two_r32l); O(inc_w_one_r16l); O(inc_w_two_r16l);
	O(jmp_abs8i); O(jmp_abs24e); O(jmp_r32h);
	O(jsr_abs8i); O(jsr_abs16e); O(jsr_abs24e); O(jsr_r32h);
	O(ldc_w_abs16_ccr); O(ldc_w_abs32_ccr); O(ldc_w_r32d16h_ccr); O(ldc_w_r32d32hh_ccr); O(ldc_w_r32ih_ccr); O(ldc_w_r32ph_ccr);
	O(mov_b_abs32_r8l); O(mov_b_r32d16h_r8l); O(mov_b_r32d32hh_r8l); O(mov_b_r32ih_r8l); O(mov_b_r32ph_r8l); O(mov_b_r8l_abs32); O(mov_b_r8l_pr32h); O(mov_b_r8l_r32d16h); O(mov_b_r8l_r32d32hh); O(mov_b_r8l_r32ih);
	O(mov_l_abs16_r32l); O(mov_l_abs32_r32l); O(mov_l_r32d16h_r32l); O(mov_l_r32d32hh_r32l); O(mov_l_r32h_r32l); O(mov_l_r32ih_r32l); O(mov_l_r32l_abs16); O(mov_l_r32l_abs32); O(mov_l_r32l_pr32h); O(mov_l_r32l_r32d16h); O(mov_l_r32l_r32d32hh); O(mov_l_r32l_r32ih); O(mov_l_r32ph_r32l);
	O(mov_w_abs32_r16l); O(mov_l_imm32_r32l); O(mov_w_r16l_abs32); O(mov_w_r16l_pr32h); O(mov_w_r16l_r32d16h); O(mov_w_r16l_r32d32hh); O(mov_w_r16l_r32ih); O(mov_w_r32d16h_r16l); O(mov_w_r32d32hh_r16l); O(mov_w_r32ih_r16l); O(mov_w_r32ph_r16l);
	O(mulxs_b_r8h_r16l); O(mulxs_w_r16h_r32l);
	O(mulxu_w_r16h_r32l);
	O(neg_l_r32l); O(neg_w_r16l);
	O(not_l_r32l); O(not_w_r16l);
	O(or_l_r32h_r32l); O(or_l_imm32_r32l); O(or_w_r16h_r16l);
	O(rotl_l_r32l); O(rotl_w_r16l);
	O(rotr_l_r32l); O(rotr_w_r16l);
	O(rotxl_l_r32l); O(rotxl_w_r16l);
	O(rotxr_l_r32l); O(rotxr_w_r16l);
	O(rte);
	O(rts);
	O(shal_l_r32l); O(shal_w_r16l);
	O(shar_l_r32l); O(shar_w_r16l);
	O(shll_l_r32l); O(shll_w_r16l);
	O(shlr_l_r32l); O(shlr_w_r16l);
	O(stc_w_ccr_abs16); O(stc_w_ccr_abs32); O(stc_w_ccr_pr32h); O(stc_w_ccr_r32d16h); O(stc_w_ccr_r32d32hh); O(stc_w_ccr_r32ih);
	O(sub_l_r32h_r32l); O(sub_l_imm32_r32l);
	O(subs_l_four_r32l); O(subs_l_one_r32l); O(subs_l_two_r32l);
	O(trapa_imm2);
	O(xor_l_r32h_r32l); O(xor_l_imm32_r32l); O(xor_w_r16h_r16l);

	O(state_irq);
#undef O
};

#endif
