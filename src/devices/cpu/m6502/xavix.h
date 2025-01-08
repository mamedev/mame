// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix.h

***************************************************************************/
#ifndef MAME_CPU_M6502_XAVIX_H
#define MAME_CPU_M6502_XAVIX_H

#pragma once

#include "m6502.h"

class xavix_device : public m6502_device {
public:
	xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(callf_xa3);
	O(jmp_xa3);
	O(retf_imp);
	O(brk_xav_imp);
	O(rti_xav_imp);

	O(ora_xav_idx);
	O(ora_xav_idy);
	O(and_xav_idx);
	O(and_xav_idy);
	O(eor_xav_idx);
	O(eor_xav_idy);
	O(adc_xav_idx);
	O(adc_xav_idy);
	O(sta_xav_idx);
	O(sta_xav_idy);
	O(lda_xav_idx);
	O(lda_xav_idy);
	O(cmp_xav_idx);
	O(cmp_xav_idy);
	O(sbc_xav_idx);
	O(sbc_xav_idy);

	O(plp_xav_imp);
	O(pla_xav_imp);
	O(php_xav_imp);
	O(pha_xav_imp);
	O(jsr_xav_adr);
	O(rts_xav_imp);



	O(adc_xav_zpg);
	O(and_xav_zpg);
	O(asl_xav_zpg);
	O(bit_xav_zpg);
	O(cmp_xav_zpg);
	O(cpx_xav_zpg);
	O(cpy_xav_zpg);
	O(dec_xav_zpg);
	O(eor_xav_zpg);
	O(inc_xav_zpg);
	O(lda_xav_zpg);
	O(ldx_xav_zpg);
	O(ldy_xav_zpg);
	O(lsr_xav_zpg);
	O(ora_xav_zpg);
	O(rol_xav_zpg);
	O(ror_xav_zpg);
	O(sbc_xav_zpg);
	O(sta_xav_zpg);
	O(stx_xav_zpg);
	O(sty_xav_zpg);
	O(dcp_xav_zpg);
	O(isb_xav_zpg);
	O(lax_xav_zpg);
	O(rla_xav_zpg);
	O(rra_xav_zpg);
	O(sax_xav_zpg);
	O(slo_xav_zpg);
	O(sre_xav_zpg);
	O(nop_xav_zpg);

	O(ldx_xav_zpy);
	O(stx_xav_zpy);
	O(lax_xav_zpy);
	O(sax_xav_zpy);

	O(adc_xav_zpx);
	O(and_xav_zpx);
	O(asl_xav_zpx);
	O(cmp_xav_zpx);
	O(dec_xav_zpx);
	O(eor_xav_zpx);
	O(inc_xav_zpx);
	O(lda_xav_zpx);
	O(ldy_xav_zpx);
	O(lsr_xav_zpx);
	O(ora_xav_zpx);
	O(rol_xav_zpx);
	O(ror_xav_zpx);
	O(sbc_xav_zpx);
	O(sta_xav_zpx);
	O(sty_xav_zpx);
	O(dcp_xav_zpx);
	O(isb_xav_zpx);
	O(rla_xav_zpx);
	O(rra_xav_zpx);
	O(slo_xav_zpx);
	O(sre_xav_zpx);
	O(nop_xav_zpx);

	O(slo_xav_idx);
	O(rla_xav_idx);
	O(sre_xav_idx);
	O(rra_xav_idx);
	O(sax_xav_idx);
	O(lax_xav_idx);
	O(dcp_xav_idx);
	O(isb_xav_idx);

	O(slo_xav_idy);
	O(rla_xav_idy);
	O(sre_xav_idy);
	O(rra_xav_idy);
	O(sha_xav_idy);
	O(lax_xav_idy);
	O(dcp_xav_idy);
	O(isb_xav_idy);



#undef O

	typedef device_delegate<int16_t (int which, int half)> xavix_interrupt_vector_delegate;

	template <typename... T> void set_vector_callback(T &&... args) { m_vector_callback.set(std::forward<T>(args)...); }

	uint8_t read_full_data(uint8_t databank, uint16_t addr);
	uint8_t read_full_data(uint32_t addr);
	void write_full_data(uint8_t databank, uint16_t adr, uint8_t val);
	void write_full_data(uint32_t addr, uint8_t val);

protected:
	class mi_xavix : public memory_interface {
	public:
		xavix_device *base;

		mi_xavix(xavix_device *base);
		virtual ~mi_xavix() {}

		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	uint8_t m_databank;
	uint8_t m_codebank;
	uint32_t XPC;

	xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t adr_with_codebank(uint16_t adr) { return adr | (get_codebank() << 16); }

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual offs_t pc_to_external(u16 pc) override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_special_data_config;
	address_space *m_special_data_space;
	address_space_config m_lowbus_config;
	address_space_config m_extbus_config;
	address_space *m_lowbus_space;
	address_space *m_extbus_space;

protected:
	xavix_interrupt_vector_delegate m_vector_callback;

	void set_codebank(uint8_t bank);
	uint8_t get_codebank();
	void set_databank(uint8_t bank);
	uint8_t get_databank();

	void write_special_stack(uint8_t data);
	void dec_special_stack();
	void inc_special_stack();
	uint8_t read_special_stack();

	/* we store the additional 'codebank' used for far calls in a different, private stack
	   this seems to be neccessary for 'rad_hnt2' not to crash when bringing up the calibration / score table screens
	   and also for ekara 'a1' cart not to crash shortly after going ingame
	   it's possible however the stack format is just incorrect, so the exact reason for this being needed does
	   need further research */
	uint8_t m_special_stack[0x100];
	uint8_t m_special_stackpos;

	uint8_t read_stack(uint32_t addr);
	void write_stack(uint32_t addr, uint8_t val);

	uint8_t read_zeropage(uint32_t addr);
	void write_zeropage(uint32_t addr, uint8_t val);

};

enum {
	XAVIX_DATABANK = M6502_IR+1,
	XAVIX_CODEBANK,
};


DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
