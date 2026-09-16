// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_FSCPU32_H
#define MAME_CPU_M68000_FSCPU32_H

#pragma once

#include "m68kmusashi.h"

class fscpu32_device : public m68000_musashi_device
{
public:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

protected:
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, address_map_constructor internal_map);
};

#endif
