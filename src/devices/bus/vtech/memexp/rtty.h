// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-200/300 RTTY Cartridge (K-6318)

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_RTTY_H__
#define __VTECH_MEMEXP_RTTY_H__

#include "emu.h"
#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rtty_interface_device

class rtty_interface_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	rtty_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( receive_data_r );
	DECLARE_WRITE8_MEMBER( transmit_data_w );
	DECLARE_WRITE8_MEMBER( relay_w );

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

// device type definition
extern const device_type RTTY_INTERFACE;

#endif // __VTECH_MEMEXP_RTTY_H__
