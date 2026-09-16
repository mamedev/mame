// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000.h (Super XaviX)

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
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

#define O(o) void o ## _full(); void o ## _partial()

	// Super XaviX opcodes

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

	O(spa2_imp); // Store accumulator in 24-bit address register PA MSB (Bank bit)
	O(lpa2_imp); // Load accumulator from 24-bit address register PA MSB (Bank bit)
	O(spa0_imp); // Store accumulator in 24-bit address register PA LSB (Low 8 bits of address)
	O(lpa0_imp); // Load accumulator from 24-bit address register PA LSB (Low 8 bits of address)
	O(spa1_imp); // Store accumulator in 24-bit address register PA MID (High 8 bits of address)
	O(lpa1_imp); // Load accumulator from 24-bit address register PA MID (High 8 bits of address)

	O(spb2_imp); // Store accumulator in 24-bit address register PB MSB (Bank bit)
	O(lpb2_imp); // Load accumulator from 24-bit address register PB MSB (Bank bit)
	O(spb0_imp); // Store accumulator in 24-bit address register PB LSB (Low 8 bits of address)
	O(lpb0_imp); // Load accumulator from 24-bit address register PB LSB (Low 8 bits of address)
	O(spb1_imp); // Store accumulator in 24-bit address register PB MID (High 8 bits of address)
	O(lpb1_imp); // Load accumulator from 24-bit address register PB MID (High 8 bits of address)

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

	O(jmpf_ind);

#undef O

	uint8_t m_j;
	uint8_t m_k;
	uint8_t m_l;
	uint8_t m_m;

	uint32_t m_pa; // 24-bit address register?
	uint32_t m_pb; // ^

};

enum {
	SXAVIX_J = XAVIX_CODEBANK+1,
	SXAVIX_K,
	SXAVIX_L,
	SXAVIX_M,
	SXAVIX_PA,
	SXAVIX_PB
};

class xavix2002_device : public xavix2000_device {
public:
	xavix2002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(XAVIX2000, xavix2000_device)
DECLARE_DEVICE_TYPE(XAVIX2002, xavix2002_device)

#endif // MAME_CPU_M6502_XAVIX2000_H
