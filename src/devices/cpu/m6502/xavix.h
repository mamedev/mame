// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn


#ifndef MAME_CPU_M6502_XAVIX_H
#define MAME_CPU_M6502_XAVIX_H

#pragma once

#include "m6502.h"

class xavix_device : public m6502_device {
public:
	enum {
		AS_LOW_BUS = 5,
		AS_EXTERNAL_BUS = 6
	};

	xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	using xavix_interrupt_vector_delegate = device_delegate<int16_t (int source, int low_byte)>;

	template <typename... T> void set_interrupt_vector_callback(T &&... args) { m_interrupt_vector_callback.set(std::forward<T>(args)...); }
	void set_data_address_mask(u32 mask) { m_data_address_mask = mask; }

	uint8_t read_banked_data(uint8_t databank, uint16_t addr);
	uint8_t read_banked_data(uint32_t addr);
	void write_banked_data(uint8_t databank, uint16_t adr, uint8_t val);
	void write_banked_data(uint32_t addr, uint8_t val);
	// Other bus masters bypass CPU buffering and wait states.
	void write_banked_data_raw(uint32_t addr, uint8_t val);

protected:
	enum {
		XAVIX_DATABANK = M6502_IR+1,
		XAVIX_CODEBANK,
	};

	class mi_xavix : public memory_interface {
	public:
		xavix_device *m_base;

		mi_xavix(xavix_device *base);

		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
		virtual uint8_t read_banked(uint32_t adr);
		virtual void write_banked(uint32_t adr, uint8_t val);
	};

	uint8_t read_banked_data_raw(uint32_t addr);
	uint8_t read_banked_data_raw(uint8_t databank, uint16_t addr);
	void write_banked_data_raw(uint8_t databank, uint16_t addr, uint8_t val);

	uint8_t m_databank;
	uint8_t m_codebank;
	u32 m_data_address_mask;
	bool m_full_bank_register_decode;

	xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool full_bank_register_decode, bool use_separate_codebank_stack);

	uint32_t code_address(uint16_t adr) { return adr | (m_codebank << 16); }

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual offs_t pc_to_external(u16 pc) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_lowbus_config;
	address_space_config m_extbus_config;
	memory_access<15, 0, 0, ENDIANNESS_LITTLE>::specific m_lowbus;
	memory_access<24, 0, 0, ENDIANNESS_LITTLE>::specific m_extbus;

#define O(o) void o ## _full(); void o ## _partial()

	// XaviX opcodes
	O(callf_xa3);
	O(jmp_xa3);
	O(retf_imp);
	O(brk_xav_imp);
	O(rti_xav_imp);

	// Standard 6502 opcodes whose arithmetic differs on XaviX
	O(adc_nd_aba); O(adc_nd_abx); O(adc_nd_aby); O(adc_nd_imm);
	O(arr_nd_imm);
	O(isb_nd_aba); O(isb_nd_abx); O(isb_nd_aby);
	O(rra_nd_aba); O(rra_nd_abx); O(rra_nd_aby);
	O(sbc_nd_aba); O(sbc_nd_abx); O(sbc_nd_aby); O(sbc_nd_imm);

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

	xavix_interrupt_vector_delegate m_interrupt_vector_callback;

	// XaviX retains the decimal flag, but arithmetic is always binary.
	void do_adc(uint8_t val) { do_adc_nd(val); }
	void do_sbc(uint8_t val) { do_sbc_nd(val); }

	void set_codebank(uint8_t bank);
	uint8_t codebank();
	void set_databank(uint8_t bank);
	uint8_t databank();

	void write_codebank_stack(uint32_t addr, uint8_t data);
	void dec_codebank_stack_pointer();
	void inc_codebank_stack_pointer();
	uint8_t read_codebank_stack(uint32_t addr);

	// Original XaviX software can change S independently before returning from a far call or interrupt.
	// The emulated bank stack preserves that nesting; its physical implementation is unconfirmed.
	uint8_t m_codebank_stack[0x100];
	uint8_t m_codebank_stack_pointer;

	uint8_t read_stack(uint32_t addr);
	void write_stack(uint32_t addr, uint8_t val);

	uint8_t read_zeropage(uint32_t addr);
	void write_zeropage(uint32_t addr, uint8_t val);

	// Some original XaviX games require the extra byte of far-call and interrupt frames to be hidden, but
	// dblmouse on suprtvpc directly reads it from the ordinary stack.  This may be a generation difference,
	// so retain the conventional frame for the later CPUs.
	//
	// pausing in rad_hnt2 (press shift when on the map) is broken without this logic as the stack becomes too
	// big with the extra bytes
	//
	bool m_use_separate_codebank_stack;
};

DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
