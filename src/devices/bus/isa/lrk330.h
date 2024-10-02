// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_ISA_LRK330_H
#define MAME_BUS_ISA_LRK330_H

#pragma once

#include "isa.h"
#include "cpu/mcs51/mcs51.h"

class lrk331_device : public device_t, public device_isa16_card_interface
{
public:
	lrk331_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void ucode_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_mcu;
	required_region_ptr<u8> m_bios;
	required_ioport m_config;
};

DECLARE_DEVICE_TYPE(LRK331, lrk331_device)

#endif // MAME_BUS_ISA_LRK330_H
