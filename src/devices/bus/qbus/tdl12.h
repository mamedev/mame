// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_QBUS_TDL12_H
#define MAME_BUS_QBUS_TDL12_H

#pragma once

#include "qbus.h"
#include "machine/ncr5380.h"


class tdl12_device : public device_t, public device_qbus_card_interface
{
public:
	// device type constructor
	tdl12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 latch_r(offs_t offset);
	void latch_w(offs_t offset, u8 data);
	u8 in40_r();
	void out40_w(u8 data);
	void out70_w(u8 data);
	void out74_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_tdcpu;
};

// device type declaration
DECLARE_DEVICE_TYPE(TDL12, tdl12_device)

#endif // MAME_BUS_QBUS_TDL12_H
