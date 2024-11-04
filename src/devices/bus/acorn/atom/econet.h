// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Econet Interface

**********************************************************************/


#ifndef MAME_BUS_ACORN_ATOM_ECONET_H
#define MAME_BUS_ACORN_ATOM_ECONET_H

#pragma once

#include "bus/acorn/bus.h"
#include "bus/econet/econet.h"
#include "machine/mc6854.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class atom_econet_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	atom_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t statid_r();
	void bus_irq_w(int state);

	required_device<mc6854_device> m_adlc;
	required_device<econet_device> m_econet;
};


// device type definition
DECLARE_DEVICE_TYPE(ATOM_ECONET, atom_econet_device)


#endif // MAME_BUS_ACORN_ATOM_ECONET_H
