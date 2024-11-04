// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RS-232 Cartridge (K-6317)

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_RS232_H
#define MAME_BUS_VTECH_MEMEXP_RS232_H

#pragma once

#include "memexp.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_rs232_interface_device

class vtech_rs232_interface_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;

private:
	required_device<rs232_port_device> m_rs232;

	uint8_t receive_data_r();
	void transmit_data_w(uint8_t data);

	int m_rx;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_RS232_INTERFACE, vtech_rs232_interface_device)

#endif // MAME_BUS_VTECH_MEMEXP_RS232_H
