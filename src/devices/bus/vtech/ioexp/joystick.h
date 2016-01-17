// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser Joystick Interface

    VTech Laser JS 20
    Dick Smith Electronics X-7315

***************************************************************************/

#pragma once

#ifndef __VTECH_IOEXP_JOYSTICK_H__
#define __VTECH_IOEXP_JOYSTICK_H__

#include "emu.h"
#include "ioexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> joystick_interface_device

class joystick_interface_device : public device_t, public device_ioexp_interface
{
public:
	// construction/destruction
	joystick_interface_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( joystick_r );

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_joy0;
	required_ioport m_joy0_arm;
	required_ioport m_joy1;
	required_ioport m_joy1_arm;
};

// device type definition
extern const device_type JOYSTICK_INTERFACE;

#endif // __VTECH_IOEXP_JOYSTICK_H__
