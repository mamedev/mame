// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RS-232 Cartridge

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
	rs232_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( rs232_rx_w );
	DECLARE_READ8_MEMBER( receive_data_r );
	DECLARE_WRITE8_MEMBER( transmit_data_w );

protected:
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

private:
	required_device<rs232_port_device> m_rs232;

	int m_rx;
};

// device type definition
extern const device_type RS232_INTERFACE;

#endif // __VTECH_MEMEXP_RS232_H__
