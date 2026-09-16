// license:BSD-3-Clause
// copyright-holders:Peter Bortas
#ifndef MAME_BUS_ABCBUS_CADMOUSE_H
#define MAME_BUS_ABCBUS_CADMOUSE_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class abc_cadmouse_device :  public device_t,
	public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_cadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Flag non working features
	static constexpr feature_type unemulated_features() { return feature::MOUSE | feature::GRAPHICS; }

protected:
	void abc_cadmouse_mem(address_map &map) ATTR_COLD;
	void abc_cadmouse_io(address_map &map) ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
 private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_CADMOUSE, abc_cadmouse_device)

#endif // MAME_BUS_ABCBUS_CADMOUSE_H
