// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix.h

***************************************************************************/
#ifndef MAME_CPU_M6502_XAVIX_H
#define MAME_CPU_M6502_XAVIX_H

#pragma once

#include "m6502.h"

#define MCFG_XAVIX_VECTOR_CALLBACK(_class, _method) \
	downcast<xavix_device &>(*device).set_vector_callback(xavix_device::xavix_interrupt_vector_delegate(&_class::_method, #_class "::" #_method, this));

class xavix_device : public m6502_device {
public:
	xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(callf_xa3);
	O(retf_imp);
	O(brk_xav_imp);
	O(rti_xav_imp);

	typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

	template <typename Object> void set_vector_callback(Object &&cb) { m_vector_callback = std::forward<Object>(cb); }


#undef O

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

	uint32_t adr_with_codebank(uint16_t adr) { return adr | (get_codebank() << 16); }

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual offs_t pc_to_external(u16 pc) override;

protected:
	xavix_interrupt_vector_delegate m_vector_callback;

	void set_codebank(uint8_t bank);
	uint8_t get_codebank();
	void set_databank(uint8_t bank);
	uint8_t get_databank();

};

DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
