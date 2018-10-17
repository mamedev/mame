// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000.h (Super XaviX)

***************************************************************************/
#ifndef MAME_CPU_M6502_XAVIX2000_H
#define MAME_CPU_M6502_XAVIX2000_H

#pragma once

#include "xavix.h"

#define MCFG_XAVIX2000_VECTOR_CALLBACK(_class, _method) \
	downcast<xavix2000_device &>(*device).set_vector_callback(xavix2000_device::xavix2000_interrupt_vector_delegate(&_class::_method, #_class "::" #_method, this));

class xavix2000_device : public xavix_device {
public:
	xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// Super XaviX opcodes

	O(phx_imp); // 12
	O(phy_imp); // 52
	O(plx_imp); // 32
	O(ply_imp); // 72

	O(clrl_imp); // b2
	O(decl_imp); // c2
	O(notl_imp); // d2
	O(incl_imp); // e2
	O(negl_imp); // f2

	O(oral0_acc);
	O(oral1_acc);
	O(oral2_acc);
	O(oral3_acc);

	O(andl0_acc);
	O(andl1_acc);
	O(andl2_acc);
	O(andl3_acc);

	O(eorl0_acc);
	O(eorl1_acc);
	O(eorl2_acc);
	O(eorl3_acc);

	O(adcl0_acc);
	O(adcl1_acc);
	O(adcl2_acc);
	O(adcl3_acc);
	
	O(stal0_acc);
	O(stal1_acc);
	O(stal2_acc);
	O(stal3_acc);

	O(ldal0_acc);
	O(ldal1_acc);
	O(ldal2_acc);
	O(ldal3_acc);

	O(cmpl0_acc);
	O(cmpl1_acc);
	O(cmpl2_acc);
	O(cmpl3_acc);

	O(sbcl0_acc);
	O(sbcl1_acc);
	O(sbcl2_acc);
	O(sbcl3_acc);

	O(spa2_acc); // Store accumulator in 24-bit address register PA MSB (Bank bit)
	O(lpa2_acc); // Load accumulator from 24-bit address register PA MSB (Bank bit)
	O(spa0_acc); // Store accumulator in 24-bit address register PA LSB (Low 8 bits of address)
	O(lpa0_acc); // Load accumulator from 24-bit address register PA LSB (Low 8 bits of address)
	O(spa1_acc); // Store accumulator in 24-bit address register PA MID (High 8 bits of address)
	O(lpa1_acc); // Load accumulator from 24-bit address register PA MID (High 8 bits of address)

	O(spb2_acc); // Store accumulator in 24-bit address register PB MSB (Bank bit)
	O(lpb2_acc); // Load accumulator from 24-bit address register PB MSB (Bank bit)
	O(spb0_acc); // Store accumulator in 24-bit address register PB LSB (Low 8 bits of address)
	O(lpb0_acc); // Load accumulator from 24-bit address register PB LSB (Low 8 bits of address)
	O(spb1_acc); // Store accumulator in 24-bit address register PB MID (High 8 bits of address)
	O(lpb1_acc); // Load accumulator from 24-bit address register PB MID (High 8 bits of address)

	O(incpa_imp);
	O(decpa_imp);

	O(incpb_imp);
	O(decpb_imp);

	O(orapa_imp); // (ora ($PA), y) ?
	O(andpa_imp); // (and ($PA), y) ?
	O(eorpa_imp); // (eor ($PA), y) ?
	O(adcpa_imp); // (adc ($PA), y) ?
	O(stapa_imp); // (sta ($PA), y) ?
	O(ldapa_imp); // (lda ($PA), y) ?
	O(cmppa_imp); // (cmp ($PA), y) ?
	O(sbcpa_imp); // (sbc ($PA), y) ?

	O(orapb_imp); // (ora ($PB), y) ?
	O(andpb_imp); // (and ($PB), y) ?
	O(eorpb_imp); // (eor ($PB), y) ?
	O(adcpb_imp); // (adc ($PB), y) ?
	O(stapb_imp); // (sta ($PB), y) ?
	O(ldapb_imp); // (lda ($PB), y) ?
	O(cmppb_imp); // (cmp ($PB), y) ?
	O(sbcpb_imp); // (sbc ($PB), y) ?

	O(stx_aby);
	O(sty_abx);

	O(stz_aba);
	O(stz_zpg);

	O(bit_zpx);
	O(bit_abx);
	O(bit_imm);

	O(asr_zpg);
	O(asr_aba);
	O(asr_zpx);
	O(asr_acc);
	O(asr_abx);

	O(cmc_imp);
	O(sev_imp);

	O(callf_aba);
	O(callf_ind);

#undef O

	uint32_t m_l; // 32-bit register?
	uint32_t m_pa; // 24-bit address register?
	uint32_t m_pb; // ^

};

DECLARE_DEVICE_TYPE(XAVIX2000, xavix2000_device)

#endif // MAME_CPU_M6502_XAVIX2000_H
