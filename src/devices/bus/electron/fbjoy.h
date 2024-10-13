// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Switched Joystick Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/FirstByte_JoystickIF.html

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_FBJOY_H
#define MAME_BUS_ELECTRON_FBJOY_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_fbjoy_device

class electron_fbjoy_device :
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_fbjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_FBJOY, electron_fbjoy_device)


#endif // MAME_BUS_ELECTRON_FBJOY_H
