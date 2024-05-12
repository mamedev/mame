// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68000MUSASHI_H
#define MAME_CPU_M68000_M68000MUSASHI_H

#pragma once

#include "m68kmusashi.h"

class m68000msh_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68000msh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(M68000MUSASHI, m68000msh_device)

#endif
