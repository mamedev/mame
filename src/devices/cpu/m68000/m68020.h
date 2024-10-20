// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68020_H
#define MAME_CPU_M68000_M68020_H

#pragma once

#include "m68kmusashi.h"

class m68ec020_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68020_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68020fpu_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68020pmmu_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

class m68020hmmu_device : public m68000_musashi_device
{
public:
	// construction/destruction
	m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	virtual bool memory_translate(int space, int intention, offs_t &address, address_space *&target_space) override;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M68EC020, m68ec020_device)
DECLARE_DEVICE_TYPE(M68020, m68020_device)
DECLARE_DEVICE_TYPE(M68020FPU, m68020fpu_device)
DECLARE_DEVICE_TYPE(M68020PMMU, m68020pmmu_device)
DECLARE_DEVICE_TYPE(M68020HMMU, m68020hmmu_device)

#endif
