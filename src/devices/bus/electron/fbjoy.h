// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    First Byte Switched Joystick Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/FirstByte_JoystickIF.html

**********************************************************************/

#pragma once

#ifndef __ELECTRON_FBJOY__
#define __ELECTRON_FBJOY__


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
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER(joystick_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_joy;
};


// device type definition
extern const device_type ELECTRON_FBJOY;


#endif
