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

class vtech_rtty_interface_device : public device_t, public device_vtech_memexp_interface
{
public:
	// construction/destruction
	vtech_rtty_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( receive_data_r );
	DECLARE_WRITE8_MEMBER( transmit_data_w );
	DECLARE_WRITE8_MEMBER( relay_w );

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_RTTY_INTERFACE, vtech_rtty_interface_device)

#endif // MAME_BUS_VTECH_MEMEXP_RTTY_H
