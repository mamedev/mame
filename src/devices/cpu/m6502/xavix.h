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

	virtual util::disasm_interface *create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(callf_xa3);
	O(retf_imp);

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

	uint8_t farbank_r() { return m_farbank; }
	void farbank_w(uint8_t data) { m_farbank = data; }

	uint32_t adr_with_bank(uint16_t adr) { return adr | (m_farbank << 16); }

	virtual void device_start() override;
	virtual void device_reset() override;

};

DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
