// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 32K Dynamic RAM Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_32KDRAM.html

**********************************************************************/


#ifndef MAME_BUS_ACORN_SYSTEM_32K_H
#define MAME_BUS_ACORN_SYSTEM_32K_H

#pragma once

#include "bus/acorn/bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class acorn_32k_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	acorn_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_links;
	u8 m_ram[32768];
};


// device type definition
DECLARE_DEVICE_TYPE(ACORN_32K, acorn_32k_device)


#endif // MAME_BUS_ACORN_SYSTEM_32K_H
