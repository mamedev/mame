// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1571CR Single Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1571CR_H
#define MAME_BUS_CBMIEC_C1571CR_H

#pragma once

#include "c1571.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1571cr_device

class c1571cr_device :  public c1571_device
{
public:
	// construction/destruction
	c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void via0_pa_w(uint8_t data);
	void via0_pb_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(C1571CR, c1571cr_device)

#endif // MAME_BUS_CBMIEC_C1571CR_H
