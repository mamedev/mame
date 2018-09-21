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
		M740_MAX_INT_LINE = M740_INT14_LINE,
		M740_SET_OVERFLOW = m6502_device::V_LINE
	};

	m740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void execute_set_input(int inputnum, int state) override;

protected:
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
	O(brk740_imp);
	O(clt_imp);
	O(set_imp);
	O(ldm_imz);
	O(jsr_spg);
	O(reset740);
	O(seb_biz); O(seb_bac);
	O(clb_biz); O(clb_bac);
	O(bbc_bzr); O(bbc_bar);
	O(bbs_bzr); O(bbs_bar);
	O(rrf_zpg);
	O(bra_rel);
	O(jmp_zpi);

	O(adct_aba); O(adct_abx); O(adct_aby); O(adct_idx); O(adct_idy); O(adct_imm); O(adct_zpg); O(adct_zpx);
	O(andt_aba); O(andt_abx); O(andt_aby); O(andt_imm); O(andt_idx); O(andt_idy); O(andt_zpg); O(andt_zpx);
	O(cmpt_aba); O(cmpt_abx); O(cmpt_aby); O(cmpt_idx); O(cmpt_idy); O(cmpt_imm); O(cmpt_zpg); O(cmpt_zpx);
	O(com_zpg);
	O(dec_acc);
	O(dect_acc);
	O(eort_aba); O(eort_abx); O(eort_aby); O(eort_idx); O(eort_idy); O(eort_imm); O(eort_zpg); O(eort_zpx);
	O(inc_acc);
	O(inct_acc);
	O(ldt_aba); O(ldt_abx); O(ldt_aby); O(ldt_idx); O(ldt_idy); O(ldt_imm); O(ldt_zpg); O(ldt_zpx);
	O(ort_aba); O(ort_abx); O(ort_aby); O(ort_imm); O(ort_idx); O(ort_idy); O(ort_zpg); O(ort_zpx);
	O(sbct_aba); O(sbct_abx); O(sbct_aby); O(sbct_idx); O(sbct_idy); O(sbct_imm); O(sbct_zpg); O(sbct_zpx);

#undef O

	uint32_t m_irq_multiplex;
	uint16_t m_irq_vector;

	void set_irq_line(int line, int state);
};

DECLARE_DEVICE_TYPE(M740, m740_device)

#endif // MAME_CPU_M6502_M740_H
