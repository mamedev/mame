// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RS-232 Cartridge (K-6317)

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_RS232_H__
#define __VTECH_MEMEXP_RS232_H__

#include "emu.h"
#include "memexp.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rs232_interface_device

class rs232_interface_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rs232_rx_w(int state);
	uint8_t receive_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void transmit_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<rs232_port_device> m_rs232;

	int m_rx;
};

// device type definition
extern const device_type RS232_INTERFACE;

#endif // __VTECH_MEMEXP_RS232_H__
