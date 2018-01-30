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

protected:
	xavix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

#define O(o) void o ## _full(); void o ## _partial()

	// xaviv opcodes
	O(callf_xa3);
	O(retf_imp);

#undef O
};

DECLARE_DEVICE_TYPE(XAVIX, xavix_device)

#endif // MAME_CPU_M6502_XAVIX_H
