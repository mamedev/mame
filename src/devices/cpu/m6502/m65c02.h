// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02.h

    Mostek 6502, CMOS variant with some additional instructions (but
    not the bitwise ones)

***************************************************************************/

#ifndef __M65C02_H__
#define __M65C02_H__

#include "m6502.h"

class m65c02_device : public m6502_device {
public:
	m65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m65c02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
#define O(o) void o ## _full(); void o ## _partial()

	// 65c02 opcodes
	O(adc_c_aba); O(adc_c_abx); O(adc_c_aby); O(adc_c_idx); O(adc_c_idy); O(adc_c_imm); O(adc_c_zpg); O(adc_c_zpi); O(adc_c_zpx);
	O(and_zpi);
	O(asl_c_abx);
	O(bbr_zpb);
	O(bbs_zpb);
	O(bit_abx); O(bit_imm); O(bit_zpx);
	O(bra_rel);
	O(brk_c_imp);
	O(cmp_zpi);
	O(dec_acc);
	O(eor_zpi);
	O(inc_acc);
	O(jmp_c_ind); O(jmp_iax);
	O(lda_zpi);
	O(lsr_c_abx);
	O(nop_c_aba); O(nop_c_abx); O(nop_c_imp);
	O(ora_zpi);
	O(phx_imp);
	O(phy_imp);
	O(plx_imp);
	O(ply_imp);
	O(rmb_bzp);
	O(rol_c_abx);
	O(ror_c_abx);
	O(sbc_c_aba); O(sbc_c_abx); O(sbc_c_aby); O(sbc_c_idx); O(sbc_c_idy); O(sbc_c_imm); O(sbc_c_zpg); O(sbc_c_zpi); O(sbc_c_zpx);
	O(smb_bzp);
	O(stp_imp);
	O(sta_zpi);
	O(stz_aba); O(stz_abx); O(stz_zpg); O(stz_zpx);
	O(trb_aba); O(trb_zpg);
	O(tsb_aba); O(tsb_zpg);
	O(wai_imp);

#undef O
};

enum {
	M65C02_IRQ_LINE = m6502_device::IRQ_LINE,
	M65C02_NMI_LINE = m6502_device::NMI_LINE,
	M65C02_SET_OVERFLOW = m6502_device::V_LINE
};

extern const device_type M65C02;

#endif
