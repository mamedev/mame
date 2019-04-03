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

	O(xavora_idx);
	O(xavora_idy);
	O(xavand_idx);
	O(xavand_idy);
	O(xaveor_idx);
	O(xaveor_idy);
	O(xavadc_idx);
	O(xavadc_idy);
	O(xavsta_idx);
	O(xavsta_idy);
	O(xavlda_idx);
	O(xavlda_idy);
	O(xavcmp_idx);
	O(xavcmp_idy);
	O(xavsbc_idx);
	O(xavsbc_idy);

	typedef device_delegate<int16_t (int which, int half)> xavix_interrupt_vector_delegate;

	template <typename Object> void set_vector_callback(Object &&cb) { m_vector_callback = std::forward<Object>(cb); }
	void set_vector_callback(xavix_interrupt_vector_delegate callback) { m_vector_callback = callback; }
	template <class FunctionClass> void set_vector_callback(const char *devname, int16_t (FunctionClass::*callback)(int, int), const char *name)
	{
		set_vector_callback(xavix_interrupt_vector_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_vector_callback(int16_t (FunctionClass::*callback)(int, int), const char *name)
	{
		set_vector_callback(xavix_interrupt_vector_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}


#undef O


	uint8_t read_full_data(uint8_t databank, uint16_t addr);
	uint8_t read_full_data(uint32_t addr);
	void write_full_data(uint8_t databank, uint16_t adr, uint8_t val);
	void write_full_data(uint32_t addr, uint8_t val);

	// used for opcodes etc. that can't see certain things in banks > 0x80
	uint8_t read_full_data_sp(uint8_t databank, uint16_t adr);
	uint8_t read_full_data_sp(uint32_t adr);
	void write_full_data_sp(uint8_t databank, uint16_t adr, uint8_t val);
	void write_full_data_sp(uint32_t adr, uint8_t val);

protected:
	class mi_xavix_normal : public memory_interface {
	public:
		xavix_device *base;

		mi_xavix_normal(xavix_device *base);
		virtual ~mi_xavix_normal() {}

		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	class mi_xavix_nd : public mi_xavix_normal {
	public:
		mi_xavix_nd(xavix_device *base);
		virtual ~mi_xavix_nd() {}

		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
	};

	uint8_t m_databank;
	uint8_t m_codebank;
	uint32_t XPC;

	xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint32_t adr_with_codebank(uint16_t adr) { return adr | (get_codebank() << 16); }

	virtual void device_start() override;
	virtual void device_reset() override;
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

	uint8_t read_special(uint16_t adr);

protected:
	xavix_interrupt_vector_delegate m_vector_callback;

	void set_codebank(uint8_t bank);
	uint8_t get_codebank();
	void set_databank(uint8_t bank);
	uint8_t get_databank();

};

enum {
	XAVIX_DATABANK = M6502_IR+1,
	XAVIX_CODEBANK,
};


DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
