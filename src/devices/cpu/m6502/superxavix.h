// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_CPU_M6502_SUPERXAVIX_H
#define MAME_CPU_M6502_SUPERXAVIX_H

#pragma once

#include "xavix2000.h"

class superxavix_device : public xavix2000_device {
public:
	superxavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto memory_control_callback() { return m_memory_control_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// Instruction sequences whose timing differs from earlier generations.
	O(brk_xav_imp); O(ora_xav_idx); O(cmc_imp); O(oraj_imp);
	O(asr_xav_zpg); O(ora_xav_zpg); O(asl_xav_zpg); O(orak_imp);
	O(php_xav_imp); O(asl_acc); O(oral_imp); O(asr_aba);
	O(asl_aba); O(oram_imp); O(bpl_rel); O(ora_xav_idy);
	O(phx_imp); O(orapa_imp); O(asr_xav_zpx); O(ora_xav_zpx);
	O(asl_xav_zpx); O(orapb_imp); O(clc_imp); O(ora_aby);
	O(spa0_imp); O(asr_abx); O(ora_abx); O(asl_abx);
	O(spb0_imp); O(jsr_xav_adr); O(and_xav_idx); O(callf_xa3);
	O(andj_imp); O(bit_xav_zpg); O(and_xav_zpg); O(rol_xav_zpg);
	O(andk_imp); O(plp_xav_imp); O(rol_acc); O(andl_imp);
	O(rol_aba); O(andm_imp); O(bmi_rel); O(and_xav_idy);
	O(plx_imp); O(andpa_imp); O(bit_xav_zpx); O(and_xav_zpx);
	O(rol_xav_zpx); O(andpb_imp); O(sec_imp); O(and_aby);
	O(lpa0_imp); O(bit_abx); O(and_abx); O(rol_abx);
	O(lpb0_imp); O(rti_xav_imp); O(eor_xav_idx); O(nop_imp);
	O(eorj_imp); O(eor_xav_zpg); O(lsr_xav_zpg); O(eork_imp);
	O(pha_xav_imp); O(lsr_acc); O(eorl_imp); O(lsr_aba);
	O(eorm_imp); O(bvc_rel); O(eor_xav_idy); O(phy_imp);
	O(eorpa_imp); O(eor_xav_zpx); O(lsr_xav_zpx); O(eorpb_imp);
	O(cli_imp); O(eor_aby); O(spa1_imp); O(eor_abx);
	O(lsr_abx); O(spb1_imp); O(rts_xav_imp); O(adc_xav_idx);
	O(adcj_imp); O(ror_xav_zpg); O(adck_imp); O(pla_xav_imp);
	O(ror_acc); O(adcl_imp); O(ror_aba); O(adcm_imp);
	O(bvs_rel); O(adc_xav_idy); O(ply_imp); O(adcpa_imp);
	O(adc_xav_zpx); O(ror_xav_zpx); O(adcpb_imp); O(sei_imp);
	O(adc_nd_aby); O(lpa1_imp); O(adc_nd_abx); O(ror_abx);
	O(lpb1_imp); O(retf_imp); O(sta_xav_idx); O(staj_imp);
	O(stak_imp); O(dey_imp); O(sev_imp); O(txa_imp);
	O(stal_imp); O(stam_imp); O(bcc_rel); O(sta_xav_idy);
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
	O(lpb2_imp); O(cmp_xav_idx); O(dec_acc); O(cmpj_imp);
	O(dec_xav_zpg); O(cmpk_imp); O(iny_imp); O(dex_imp);
	O(cmpl_imp); O(dec_aba); O(cmpm_imp); O(bne_rel);
	O(cmp_xav_idy); O(not_acc); O(cmppa_imp); O(cmp_xav_zpx);
	O(dec_xav_zpx); O(cmppb_imp); O(cld_imp); O(cmp_aby);
	O(decpa_imp); O(cmp_abx); O(dec_abx); O(decpb_imp);
	O(sbc_xav_idx); O(inc_acc); O(sbcj_imp); O(inc_xav_zpg);
	O(sbck_imp); O(inx_imp); O(sbcl_imp); O(inc_aba);
	O(sbcm_imp); O(beq_rel); O(sbc_xav_idy); O(neg_acc);
	O(sbcpa_imp); O(sbc_xav_zpx); O(inc_xav_zpx); O(sbcpb_imp);
	O(sed_imp); O(sbc_nd_aby); O(incpa_imp); O(sbc_nd_abx);
	O(inc_abx); O(incpb_imp); O(reset);

#undef O

private:
	class mi_superxavix : public mi_xavix
	{
	public:
		mi_superxavix(superxavix_device &base) : mi_xavix(&base), m_cpu(base) { }

		virtual u8 read_sync(u16 address) override;
		virtual u8 read_arg(u16 address) override;
		virtual u8 read_banked(u32 address) override;
		virtual void write_banked(u32 address, u8 data) override;

	private:
		superxavix_device &m_cpu;
	};

	enum access_kind : u8 { NONE, INSTRUCTION, DATA, WRITE };

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


DECLARE_DEVICE_TYPE(SUPERXAVIX, superxavix_device)

#endif // MAME_CPU_M6502_SUPERXAVIX_H
