// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Western Digital WD1007A ESDI hard disk controller

**********************************************************************/

#pragma once

#ifndef MAME_BUS_ISA_WD1007A_H
#define MAME_BUS_ISA_WD1007A_H

#include "isa.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd1007a_device

class wd1007a_device : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	wd1007a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(WD1007A, wd1007a_device)

#endif // MAME_BUS_ISA_WD1007A_H
