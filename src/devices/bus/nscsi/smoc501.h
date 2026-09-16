// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_NSCSI_SMOC501_H
#define MAME_BUS_NSCSI_SMOC501_H

#pragma once

#include "machine/nscsi_bus.h"

class smoc501_device : public device_t, public nscsi_slot_card_interface
{
public:
	smoc501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SMOC501, smoc501_device)

#endif // MAME_BUS_NSCSI_SMOC501_H
