// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-300 WordPro Cartridge

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_WORDPRO_H__
#define __VTECH_MEMEXP_WORDPRO_H__

#include "emu.h"
#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wordpro_device

class wordpro_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	wordpro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const;
	virtual void device_start();
	virtual void device_reset();
};

// device type definition
extern const device_type WORDPRO;

#endif // __VTECH_MEMEXP_WORDPRO_H__
