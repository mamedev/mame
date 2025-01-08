// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68040_H
#define MAME_CPU_M68000_M68040_H

#pragma once

#include "m68kmusashi.h"

class m68ec040_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68lc040_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68040_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M68EC040, m68ec040_device)
DECLARE_DEVICE_TYPE(M68LC040, m68lc040_device)
DECLARE_DEVICE_TYPE(M68040, m68040_device)

#endif
