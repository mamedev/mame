// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m740.h

    Mitsubishi M740 series (M507xx/M509xx)

***************************************************************************/

#ifndef MAME_CPU_M6502_M740_H
#define MAME_CPU_M6502_M740_H

#include "m6502.h"
#include "m740d.h"

class m740_device : public m6502_device, public m740_disassembler::config {
public:
	enum
	{
		M740_INT0_LINE = INPUT_LINE_IRQ0,   // (fffc)
		M740_INT1_LINE,  // (fffa)
		M740_INT2_LINE,  // (fff8)
		M740_INT3_LINE,  // (fff6)
		M740_INT4_LINE,  // (fff4)
		M740_INT5_LINE,  // (fff2)
		M740_INT6_LINE,  // (fff0)
		M740_INT7_LINE,  // (ffee)
		M740_INT8_LINE,  // (ffec)
		M740_INT9_LINE,  // (ffea)
		M740_INT10_LINE, // (ffe8)
		M740_INT11_LINE, // (ffe6)
		M740_INT12_LINE, // (ffe4)
		M740_INT13_LINE, // (ffe2)
		M740_INT14_LINE, // (ffe0)
		M740_MAX_INT_LINE = M740_INT14_LINE
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual void read_dummy(uint16_t adr) { (void)mintf->read(adr); }
	virtual uint8_t read_data(uint16_t adr) { return mintf->read(adr); }
	virtual void write_data(uint16_t adr, uint8_t val) { mintf->write(adr, val); }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void execute_set_input(int inputnum, int state) override;

	m740_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

#define O(o) void o ## _full(); void o ## _partial()

	virtual u32 get_state_base() const override;

	uint8_t do_clb(uint8_t in, uint8_t bit);
	uint8_t do_seb(uint8_t in, uint8_t bit);
	uint8_t do_rrf(uint8_t in);
	void do_sbc_dt(uint8_t val);
	void do_sbc_ndt(uint8_t val);
	void do_sbct(uint8_t val);
	void do_adc_dt(uint8_t val);
	void do_adc_ndt(uint8_t val);
	void do_adct(uint8_t val);

	// m740 opcodes
	O(brk_m_imp);
	O(clt_imp);
	O(set_imp);
	O(ldm_imz);
	O(dec_acc);
	O(inc_acc);
	O(jsr_spg);
	O(reset_m);
	O(seb_biz); O(seb_bac);
	O(clb_biz); O(clb_bac);
	O(bbc_bzr); O(bbc_bar);
	O(bbs_bzr); O(bbs_bar);
	O(com_zpg);
	O(rrf_zpg);
	O(tst_zpg);
	O(bra_rel);
	O(jmp_zpi);
	O(jsr_zpi);
	O(stp_imp);
	O(wit_imp);

	O(adc_m_abx); O(adc_m_aby); O(adc_m_idx); O(adc_m_idy); O(adc_m_zpx);
	O(and_m_abx); O(and_m_aby); O(and_m_idx); O(and_m_idy); O(and_m_zpx);
	O(asl_m_aba); O(asl_m_abx); O(asl_m_zpg); O(asl_m_zpx);
	O(bcc_m_rel);
	O(bcs_m_rel);
	O(beq_m_rel);
	O(bmi_m_rel);
	O(bne_m_rel);
	O(bpl_m_rel);
	O(bvc_m_rel);
	O(bvs_m_rel);
	O(cmp_m_abx); O(cmp_m_aby); O(cmp_m_idx); O(cmp_m_idy); O(cmp_m_zpx);
	O(dec_m_aba); O(dec_m_abx); O(dec_m_zpg); O(dec_m_zpx);
	O(eor_m_abx); O(eor_m_aby); O(eor_m_idx); O(eor_m_idy); O(eor_m_zpx);
	O(inc_m_aba); O(inc_m_abx); O(inc_m_zpg); O(inc_m_zpx);
	O(jmp_m_ind);
	O(jsr_m_adr);
	O(lda_m_abx); O(lda_m_aby); O(lda_m_idx); O(lda_m_idy); O(lda_m_zpx);
	O(ldx_m_aby); O(ldx_m_zpg); O(ldx_m_zpy);
	O(ldy_m_abx); O(ldy_m_zpg); O(ldy_m_zpx);
	O(lsr_m_aba); O(lsr_m_abx); O(lsr_m_zpg); O(lsr_m_zpx);
	O(ora_m_abx); O(ora_m_aby); O(ora_m_idx); O(ora_m_idy); O(ora_m_zpx);
	O(pla_m_imp);
	O(plp_m_imp);
	O(rol_m_aba); O(rol_m_abx); O(rol_m_zpg); O(rol_m_zpx);
	O(ror_m_aba); O(ror_m_abx); O(ror_m_zpg); O(ror_m_zpx);
	O(rti_m_imp);
	O(rts_m_imp);
	O(sbc_m_abx); O(sbc_m_aby); O(sbc_m_idx); O(sbc_m_idy); O(sbc_m_zpx);
	O(sta_m_aba); O(sta_m_abx); O(sta_m_aby); O(sta_m_idx); O(sta_m_idy); O(sta_m_zpg); O(sta_m_zpx);
	O(stx_m_aba); O(stx_m_zpg); O(stx_m_zpy);
	O(sty_m_aba); O(sty_m_zpg); O(sty_m_zpx);

	O(adct_aba); O(adct_abx); O(adct_aby); O(adct_idx); O(adct_idy); O(adct_imm); O(adct_zpg); O(adct_zpx);
	O(andt_aba); O(andt_abx); O(andt_aby); O(andt_imm); O(andt_idx); O(andt_idy); O(andt_zpg); O(andt_zpx);
	O(cmpt_aba); O(cmpt_abx); O(cmpt_aby); O(cmpt_idx); O(cmpt_idy); O(cmpt_imm); O(cmpt_zpg); O(cmpt_zpx);
	O(eort_aba); O(eort_abx); O(eort_aby); O(eort_idx); O(eort_idy); O(eort_imm); O(eort_zpg); O(eort_zpx);
	O(ldt_aba); O(ldt_abx); O(ldt_aby); O(ldt_idx); O(ldt_idy); O(ldt_imm); O(ldt_zpg); O(ldt_zpx);
	O(ort_aba); O(ort_abx); O(ort_aby); O(ort_imm); O(ort_idx); O(ort_idy); O(ort_zpg); O(ort_zpx);
	O(sbct_aba); O(sbct_abx); O(sbct_aby); O(sbct_idx); O(sbct_idy); O(sbct_imm); O(sbct_zpg); O(sbct_zpx);

#undef O

	uint32_t m_irq_multiplex;
	uint16_t m_irq_vector;

	void set_irq_line(int line, int state);
};

#endif // MAME_CPU_M6502_M740_H
