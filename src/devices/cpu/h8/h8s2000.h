// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2000.h

    H8S-2000 base cpu emulation

    Adds the exr register, the move multiple instructions, the shifts by
    two, tas and optionally the trace mode.


***************************************************************************/

#ifndef __H8S2000_H__
#define __H8S2000_H__

#include "h8h.h"

class h8s2000_device : public h8h_device {
public:
	h8s2000_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, address_map_delegate map_delegate);

protected:
	static const disasm_entry disasm_entries[];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()
	O(ldc_imm8_exr); O(ldc_r8l_exr); O(ldc_w_abs16_exr); O(ldc_w_abs32_exr); O(ldc_w_r32d16h_exr); O(ldc_w_r32d32hh_exr); O(ldc_w_r32ih_exr); O(ldc_w_r32ph_exr);
	O(ldm_l_spp_r32n2l); O(ldm_l_spp_r32n3l); O(ldm_l_spp_r32n4l);
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

	O(state_trace);
#undef O
};

#endif
