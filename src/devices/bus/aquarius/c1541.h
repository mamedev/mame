// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius C1541 DOS Interface by Ron Koenig

**********************************************************************/

#ifndef MAME_BUS_AQUARIUS_C1541_H
#define MAME_BUS_AQUARIUS_C1541_H

#pragma once

#include "slot.h"
#include "bus/cbmiec/cbmiec.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class aquarius_c1541_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_c1541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_ce_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<cbm_iec_device> m_iec;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_C1541, aquarius_c1541_device)


#endif // MAME_BUS_AQUARIUS_C1541_H
