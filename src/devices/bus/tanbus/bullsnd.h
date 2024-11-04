// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Bulldog Sound Generator Board

**********************************************************************/


#ifndef MAME_BUS_TANBUS_BULLSND_H
#define MAME_BUS_TANBUS_BULLSND_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_bullsnd_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_bullsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_device_array<ay8910_device, 2> m_ay8910;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_BULLSND, tanbus_bullsnd_device)


#endif // MAME_BUS_TANBUS_BULLSND_H
