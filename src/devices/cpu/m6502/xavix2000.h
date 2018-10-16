// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000.h

***************************************************************************/
#ifndef MAME_CPU_M6502_XAVIX2000_H
#define MAME_CPU_M6502_XAVIX2000_H

#pragma once

#include "xavix.h"

#define MCFG_XAVIX2000_VECTOR_CALLBACK(_class, _method) \
	downcast<xavix2000_device &>(*device).set_vector_callback(xavix2000_device::xavix2000_interrupt_vector_delegate(&_class::_method, #_class "::" #_method, this));

class xavix2000_device : public xavix_device {
public:
	xavix2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(unh_imp);

	O(clr_acc); // b2

#undef O

};

DECLARE_DEVICE_TYPE(XAVIX2000, xavix2000_device)

#endif // MAME_CPU_M6502_XAVIX2000_H
