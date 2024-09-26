// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Taxan KY-80

***************************************************************************/

#ifndef MAME_CPU_Z80_KY80_H
#define MAME_CPU_Z80_KY80_H

#pragma once

#include "kc82.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ky80_device : public kc82_device
{
public:
	// device type constructor
	ky80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

private:
	// internal address maps
	void internal_ram(address_map &map) ATTR_COLD;
	void internal_io(address_map &map) ATTR_COLD;
};


// device type declaration
DECLARE_DEVICE_TYPE(KY80, ky80_device)

#endif // MAME_CPU_Z80_KY80_H
