// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68010_H
#define MAME_CPU_M68000_M68010_H

#pragma once

#include "m68kmusashi.h"

class m68010_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

protected:
	m68010_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock);
	m68010_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }
};

DECLARE_DEVICE_TYPE(M68010, m68010_device)

#endif // MAME_CPU_M68000_M68010_H
