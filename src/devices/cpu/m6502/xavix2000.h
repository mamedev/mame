// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    xavix2000.h (XaviX2000 and Super XaviX)

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

class superxavix_device : public xavix2000_device {
public:
	superxavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto memory_control_callback() { return m_memory_control_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// All instruction bodies use this device's timed memory accesses.
	O(brk_xav_imp); O(ora_xav_idx); O(cmc_imp); O(oraj_imp);
	O(asr_xav_zpg); O(ora_xav_zpg); O(asl_xav_zpg); O(orak_imp);
	O(php_xav_imp); O(ora_imm); O(asl_acc); O(oral_imp);
	O(asr_aba); O(ora_aba); O(asl_aba); O(oram_imp);
	O(bpl_rel); O(ora_xav_idy); O(phx_imp); O(orapa_imp);
	O(asr_xav_zpx); O(ora_xav_zpx); O(asl_xav_zpx); O(orapb_imp);
	O(clc_imp); O(ora_aby); O(asr_acc); O(spa0_imp);
	O(asr_abx); O(ora_abx); O(asl_abx); O(spb0_imp);
	O(jsr_xav_adr); O(and_xav_idx); O(callf_xa3); O(andj_imp);
	O(bit_xav_zpg); O(and_xav_zpg); O(rol_xav_zpg); O(andk_imp);
	O(plp_xav_imp); O(and_imm); O(rol_acc); O(andl_imp);
	O(bit_aba); O(and_aba); O(rol_aba); O(andm_imp);
	O(bmi_rel); O(and_xav_idy); O(plx_imp); O(andpa_imp);
	O(bit_xav_zpx); O(and_xav_zpx); O(rol_xav_zpx); O(andpb_imp);
	O(sec_imp); O(and_aby); O(bit_imm); O(lpa0_imp);
	O(bit_abx); O(and_abx); O(rol_abx); O(lpb0_imp);
	O(rti_xav_imp); O(eor_xav_idx); O(nop_imp); O(eorj_imp);
	O(eor_xav_zpg); O(lsr_xav_zpg); O(eork_imp); O(pha_xav_imp);
	O(eor_imm); O(lsr_acc); O(eorl_imp); O(jmp_adr);
	O(eor_aba); O(lsr_aba); O(eorm_imp); O(bvc_rel);
	O(eor_xav_idy); O(phy_imp); O(eorpa_imp); O(eor_xav_zpx);
	O(lsr_xav_zpx); O(eorpb_imp); O(cli_imp); O(eor_aby);
	O(spa1_imp); O(jmp_xa3); O(eor_abx); O(lsr_abx);
	O(spb1_imp); O(rts_xav_imp); O(adc_xav_idx); O(adcj_imp);
	O(adc_xav_zpg); O(ror_xav_zpg); O(adck_imp); O(pla_xav_imp);
	O(adc_nd_imm); O(ror_acc); O(adcl_imp); O(jmp_ind);
	O(adc_nd_aba); O(ror_aba); O(adcm_imp); O(bvs_rel);
	O(adc_xav_idy); O(ply_imp); O(adcpa_imp); O(adc_xav_zpx);
	O(ror_xav_zpx); O(adcpb_imp); O(sei_imp); O(adc_nd_aby);
	O(lpa1_imp); O(jmpf_ind); O(adc_nd_abx); O(ror_abx);
	O(lpb1_imp); O(retf_imp); O(sta_xav_idx); O(stz_xav_zpg);
	O(staj_imp); O(sty_xav_zpg); O(sta_xav_zpg); O(stx_xav_zpg);
	O(stak_imp); O(dey_imp); O(sev_imp); O(txa_imp);
	O(stal_imp); O(sty_aba); O(sta_aba); O(stx_aba);
	O(stam_imp); O(bcc_rel); O(sta_xav_idy); O(stz_aba);
	O(stapa_imp); O(sty_xav_zpx); O(sta_xav_zpx); O(stx_xav_zpy);
	O(stapb_imp); O(tya_imp); O(sta_aby); O(txs_imp);
	O(spa2_imp); O(sty_abx); O(sta_abx); O(stx_aby);
	O(spb2_imp); O(ldy_imm); O(lda_xav_idx); O(ldx_imm);
	O(ldaj_imp); O(ldy_xav_zpg); O(lda_xav_zpg); O(ldx_xav_zpg);
	O(ldak_imp); O(tay_imp); O(lda_imm); O(tax_imp);
	O(ldal_imp); O(ldy_aba); O(lda_aba); O(ldx_aba);
	O(ldam_imp); O(bcs_rel); O(lda_xav_idy); O(clr_acc);
	O(ldapa_imp); O(ldy_xav_zpx); O(lda_xav_zpx); O(ldx_xav_zpy);
	O(ldapb_imp); O(clv_imp); O(lda_aby); O(tsx_imp);
	O(lpa2_imp); O(ldy_abx); O(lda_abx); O(ldx_aby);
	O(lpb2_imp); O(cpy_imm); O(cmp_xav_idx); O(dec_acc);
	O(cmpj_imp); O(cpy_xav_zpg); O(cmp_xav_zpg); O(dec_xav_zpg);
	O(cmpk_imp); O(iny_imp); O(cmp_imm); O(dex_imp);
	O(cmpl_imp); O(cpy_aba); O(cmp_aba); O(dec_aba);
	O(cmpm_imp); O(bne_rel); O(cmp_xav_idy); O(not_acc);
	O(cmppa_imp); O(cmp_xav_zpx); O(dec_xav_zpx); O(cmppb_imp);
	O(cld_imp); O(cmp_aby); O(decpa_imp); O(cmp_abx);
	O(dec_abx); O(decpb_imp); O(cpx_imm); O(sbc_xav_idx);
	O(inc_acc); O(sbcj_imp); O(cpx_xav_zpg); O(sbc_xav_zpg);
	O(inc_xav_zpg); O(sbck_imp); O(inx_imp); O(sbc_nd_imm);
	O(sbcl_imp); O(cpx_aba); O(sbc_nd_aba); O(inc_aba);
	O(sbcm_imp); O(beq_rel); O(sbc_xav_idy); O(neg_acc);
	O(sbcpa_imp); O(sbc_xav_zpx); O(inc_xav_zpx); O(sbcpb_imp);
	O(sed_imp); O(sbc_nd_aby); O(incpa_imp); O(sbc_nd_abx);
	O(inc_abx); O(incpb_imp); O(reset);

#undef O

private:
	enum access_kind : u8 { NONE, INSTRUCTION, DATA, WRITE };

	u32 code_address(u16 address) { return (u32(m_codebank) << 16) | address; }
	u8 read(u16 address) { return memory_access((u32(m_databank) << 16) | address, access_kind::DATA); }
	u8 read_arg(u16 address) { return memory_access(code_address(address), access_kind::INSTRUCTION); }
	u8 read_pc() { return read_arg(m_PC); }
	u8 read_sync(u16 address) { return read_arg(address); }
	void write(u16 address, u8 data) { memory_access((u32(m_databank) << 16) | address, access_kind::WRITE, data); }
	u8 read_stack(u32 address) { return memory_access(0x100 | (address & 0xff), access_kind::DATA); }
	void write_stack(u32 address, u8 data) { memory_access(0x100 | (address & 0xff), access_kind::WRITE, data); }
	u8 read_codebank_stack(u32 address) { return read_stack(address); }
	void write_codebank_stack(u32 address, u8 data) { write_stack(address, data); }
	u8 read_zeropage(u32 address) { return memory_access(address & 0xff, access_kind::DATA); }
	void write_zeropage(u32 address, u8 data) { memory_access(address & 0xff, access_kind::WRITE, data); }
	u8 read_banked_data(u32 address) { return memory_access(address, access_kind::DATA); }
	void write_banked_data(u32 address, u8 data) { memory_access(address, access_kind::WRITE, data); }
	u8 read_vector(bool nmi, bool low);
	void read_idle() { read_idle(code_address(m_PC)); }
	void read_idle(u32 address);

	u8 memory_access(u32 address, access_kind kind, u8 data = 0);
	bool wait_for_cycle(u64 cycle);
	bool advance_external_bus(u64 cycle);
	void start_external_access();
	void observe_address(u32 address);
	u8 raw_instruction(u32 address);
	static bool internal_address(u32 address) { return !(address & 0x808000); }
	static unsigned access_cycles(u8 control) { unsigned const cycles = BIT(control, 1, 3); return cycles ? cycles : 8; }
	static unsigned page_cycles(u8 control) { unsigned const cycles = BIT(control, 6, 2); return cycles ? cycles : 4; }

	devcb_read8 m_memory_control_cb;
	u32 m_instruction_tag;
	u8 m_instruction_cache[4];
	u8 m_instruction_valid;
	bool m_instruction_burst;
	u32 m_data_tag;
	u8 m_data_cache[2];
	u8 m_data_valid;
	u32 m_bus_address;
	u64 m_bus_cycle;
	u64 m_bus_available;
	u8 m_bus_kind;
	u8 m_bus_last_kind;
	u8 m_bus_remaining;
	u8 m_bus_control;
	u8 m_bus_write_data;
	u8 m_bus_bytes[2];
	u8 m_bus_lanes;
	u32 m_access_address;
	u64 m_access_start;
	u64 m_access_cycle;
	u8 m_access_kind;
	u8 m_access_data;
	bool m_access_started;
	u8 m_operand;
};


DECLARE_DEVICE_TYPE(XAVIX2000, xavix2000_device)
DECLARE_DEVICE_TYPE(SUPERXAVIX, superxavix_device)

#endif // MAME_CPU_M6502_XAVIX2000_H
