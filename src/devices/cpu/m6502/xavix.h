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
	xavix_device::set_vector_callback(*device, xavix_device::xavix_interrupt_vector_delegate(&_class::_method, #_class "::" #_method, this));

class xavix_device : public m6502_device {
public:
	xavix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual util::disasm_interface *create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(callf_xa3);
	O(retf_imp);
	O(brk_xav_imp);
	O(rti_xav_imp);

	typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

	static void set_vector_callback(device_t &device, xavix_interrupt_vector_delegate &&cb) { downcast<xavix_device &>(device).m_vector_callback = std::move(cb); }


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

	uint8_t m_farbank;
	uint32_t XPC;

	uint8_t farbank_r() { return m_farbank; }
	void farbank_w(uint8_t data) { m_farbank = data; }

	uint32_t adr_with_bank(uint16_t adr) { return adr | (m_farbank << 16); }

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual offs_t pc_to_external(u16 pc) override;

private:
	xavix_interrupt_vector_delegate m_vector_callback;

};

DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
