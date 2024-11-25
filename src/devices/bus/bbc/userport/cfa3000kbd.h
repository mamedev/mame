// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Keyboard

**********************************************************************/


#ifndef MAME_BUS_BBC_USERPORT_CFA3000KBD_H
#define MAME_BUS_BBC_USERPORT_CFA3000KBD_H

#pragma once

#include "userport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cfa3000_kbd_device

class cfa3000_kbd_device :
	public device_t,
	public device_bbc_userport_interface
{
public:
	// construction/destruction
	cfa3000_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t pb_r() override;

private:
	required_ioport_array<4> m_kbd;
};


// device type definition
DECLARE_DEVICE_TYPE(CFA3000_KBD, cfa3000_kbd_device)


#endif // MAME_BUS_BBC_USERPORT_CFA3000KBD_H
