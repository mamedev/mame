// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX2000 CPU

***************************************************************************/
#ifndef MAME_CPU_M6502_XAVIX2000_H
#define MAME_CPU_M6502_XAVIX2000_H

#pragma once

#include "xavix.h"

class xavix2000_device : public xavix_device {
public:
	xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

protected:
	xavix2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;

#define O(o) void o ## _full(); void o ## _partial()

	// Extended opcodes shared by XaviX2000 and Super XaviX

	O(phx_imp); // 12
	O(phy_imp); // 52
	O(plx_imp); // 32
	O(ply_imp); // 72

	O(clr_acc); // b2
	O(dec_acc); // c2
	O(not_acc); // d2
	O(inc_acc); // e2
	O(neg_acc); // f2

	O(oraj_imp);
	O(orak_imp);
	O(oral_imp);
	O(oram_imp);

	O(andj_imp);
	O(andk_imp);
	O(andl_imp);
	O(andm_imp);

	O(eorj_imp);
	O(eork_imp);
	O(eorl_imp);
	O(eorm_imp);

	O(adcj_imp);
	O(adck_imp);
	O(adcl_imp);
	O(adcm_imp);

	O(staj_imp);
	O(stak_imp);
	O(stal_imp);
	O(stam_imp);

	O(ldaj_imp);
	O(ldak_imp);
	O(ldal_imp);
	O(ldam_imp);

	O(cmpj_imp);
	O(cmpk_imp);
	O(cmpl_imp);
	O(cmpm_imp);

	O(sbcj_imp);
	O(sbck_imp);
	O(sbcl_imp);
	O(sbcm_imp);

	O(spa2_imp); // Store accumulator in 24-bit address register PA MSB (Bank byte)
	O(lpa2_imp); // Load accumulator from 24-bit address register PA MSB (Bank byte)
	O(spa0_imp); // Store accumulator in 24-bit address register PA LSB (Low 8 bits of address)
	O(lpa0_imp); // Load accumulator from 24-bit address register PA LSB (Low 8 bits of address)
	O(spa1_imp); // Store accumulator in 24-bit address register PA MID (High 8 bits of address)
	O(lpa1_imp); // Load accumulator from 24-bit address register PA MID (High 8 bits of address)

	O(spb2_imp); // Store accumulator in 24-bit address register PB MSB (Bank byte)
	O(lpb2_imp); // Load accumulator from 24-bit address register PB MSB (Bank byte)
	O(spb0_imp); // Store accumulator in 24-bit address register PB LSB (Low 8 bits of address)
	O(lpb0_imp); // Load accumulator from 24-bit address register PB LSB (Low 8 bits of address)
	O(spb1_imp); // Store accumulator in 24-bit address register PB MID (High 8 bits of address)
	O(lpb1_imp); // Load accumulator from 24-bit address register PB MID (High 8 bits of address)

	O(incpa_imp);
	O(decpa_imp);

	O(incpb_imp);
	O(decpb_imp);

	// Accumulator operations use the banked address in PA/PB directly, without Y indexing.
	O(orapa_imp);
	O(andpa_imp);
	O(eorpa_imp);
	O(adcpa_imp);
	O(stapa_imp);
	O(ldapa_imp);
	O(cmppa_imp);
	O(sbcpa_imp);

	O(orapb_imp);
	O(andpb_imp);
	O(eorpb_imp);
	O(adcpb_imp);
	O(stapb_imp);
	O(ldapb_imp);
	O(cmppb_imp);
	O(sbcpb_imp);

	O(stx_aby);
	O(sty_abx);

	O(stz_aba);
	O(stz_xav_zpg);

	O(bit_xav_zpx);
	O(bit_abx);
	O(bit_imm);

	O(asr_xav_zpg);
	O(asr_aba);
	O(asr_xav_zpx);
	O(asr_acc);
	O(asr_abx);

	O(cmc_imp);
	O(sev_imp);

	O(jmp_ind);
	O(jmpf_ind);
	O(brk_xav_imp);
	O(rti_xav_imp);
	O(plp_xav_imp);

	// Memory operations write only the modified value.
	O(asl_xav_zpg); O(asl_xav_zpx); O(asl_aba); O(asl_abx);
	O(lsr_xav_zpg); O(lsr_xav_zpx); O(lsr_aba); O(lsr_abx);
	O(rol_xav_zpg); O(rol_xav_zpx); O(rol_aba); O(rol_abx);
	O(ror_xav_zpg); O(ror_xav_zpx); O(ror_aba); O(ror_abx);
	O(inc_xav_zpg); O(inc_xav_zpx); O(inc_aba); O(inc_abx);
	O(dec_xav_zpg); O(dec_xav_zpx); O(dec_aba); O(dec_abx);

#undef O

	// The opcode generator charges one internal cycle without a bus access.
	void read_idle() { }

	uint8_t m_j;
	uint8_t m_k;
	uint8_t m_l;
	uint8_t m_m;

	// 24-bit banked address registers: bank in bits 23-16, address in bits 15-0.
	uint32_t m_pa;
	uint32_t m_pb;
	bool m_interrupt_is_nmi;

private:
	enum {
		XAVIX2000_J = XAVIX_CODEBANK + 1,
		XAVIX2000_K,
		XAVIX2000_L,
		XAVIX2000_M,
		XAVIX2000_PA,
		XAVIX2000_PB
	};
};

DECLARE_DEVICE_TYPE(XAVIX2000, xavix2000_device)

#endif // MAME_CPU_M6502_XAVIX2000_H
