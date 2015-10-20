// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ABC_SLUTPROV__
#define __ABC_SLUTPROV__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_slutprov_device

class abc_slutprov_device :  public device_t,
								public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_slutprov_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);
};


// device type definition
extern const device_type ABC_SLUTPROV;



#endif
