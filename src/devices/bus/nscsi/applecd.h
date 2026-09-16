// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_NSCSI_APPLECD_H
#define MAME_BUS_NSCSI_APPLECD_H

#pragma once

#include "machine/nscsi_bus.h"

class applecd150_device : public device_t, public nscsi_slot_card_interface
{
public:
	applecd150_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(APPLECD150, applecd150_device)

#endif // MAME_BUS_NSCSI_APPLECD_H
