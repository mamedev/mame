// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68000_H
#define MAME_CPU_M68000_M68000_H

#pragma once

#include "m68kmusashi.h"

class m68000_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);


	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;

protected:
	m68000_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock);

	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map);
};

DECLARE_DEVICE_TYPE(M68000, m68000_device)

#endif
