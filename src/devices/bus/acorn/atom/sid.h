// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AtomSID emulation

**********************************************************************/

#ifndef MAME_BUS_ACORN_ATOM_SID_H
#define MAME_BUS_ACORN_ATOM_SID_H

#include "bus/acorn/bus.h"
#include "sound/mos6581.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atom_sid_device

class atom_sid_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	atom_sid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<mos6581_device> m_sid;
};


// device type definition
DECLARE_DEVICE_TYPE(ATOM_SID, atom_sid_device)


#endif // MAME_BUS_ACORN_ATOM_SID_H
