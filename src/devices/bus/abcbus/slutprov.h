// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ABC_SLUTPROV__
#define __ABC_SLUTPROV__

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
	abc_slutprov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
};


// device type definition
extern const device_type ABC_SLUTPROV;



#endif
