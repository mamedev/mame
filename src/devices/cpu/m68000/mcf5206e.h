// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_MCF5206E_H
#define MAME_CPU_M68000_MCF5206E_H

#pragma once

#include "m68kmusashi.h"

class mcf5206e_device : public m68000_musashi_device
{
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }


	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)

#endif
