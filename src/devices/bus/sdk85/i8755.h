// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_SDK85_I8755_H
#define MAME_BUS_SDK85_I8755_H

#pragma once

#include "memexp.h"
#include "machine/i8355.h"

// ======================> sdk85exp_i8755_device

class sdk85exp_i8755_device : public device_t, public device_sdk85_romexp_card_interface
{
public:
	// construction/destruction
	sdk85exp_i8755_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_sdk85_romexp_card_interface overrides
	virtual u8 read_memory(offs_t offset) override;
	virtual void write_memory(offs_t offset, u8 data) override;
	virtual u8 read_io(offs_t offset) override;
	virtual void write_io(offs_t offset, u8 data) override;
	virtual u8 *get_rom_base(u32 size) override;

private:
	required_device<i8355_device> m_i8755;
};

// device type declaration
DECLARE_DEVICE_TYPE(SDK85_I8755, sdk85exp_i8755_device)

#endif // MAME_BUS_SDK85_I8755_H
