// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RTTY Cartridge (K-6318)

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_RTTY_H
#define MAME_BUS_VTECH_MEMEXP_RTTY_H

#pragma once

#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_rtty_interface_device

class vtech_rtty_interface_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_rtty_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;

private:
	uint8_t receive_data_r();
	void transmit_data_w(uint8_t data);
	void relay_w(uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_RTTY_INTERFACE, vtech_rtty_interface_device)

#endif // MAME_BUS_VTECH_MEMEXP_RTTY_H
