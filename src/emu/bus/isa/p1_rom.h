// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 ROM cartridge device

**********************************************************************/

#pragma once

#ifndef __P1_ROM__
#define __P1_ROM__

#include "emu.h"
#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p1_rom_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	p1_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


// device type definition
extern const device_type P1_ROM;


#endif
