// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser Joystick Interface

    VTech Laser JS 20
    Dick Smith Electronics X-7315

***************************************************************************/

#ifndef MAME_BUS_VTECH_IOEXP_JOYSTICK_H
#define MAME_BUS_VTECH_IOEXP_JOYSTICK_H

#pragma once

#include "ioexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_joystick_interface_device

class vtech_joystick_interface_device : public vtech_ioexp_device
{
public:
	// construction/destruction
	vtech_joystick_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t joystick_r(offs_t offset);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_ioport m_joy0;
	required_ioport m_joy0_arm;
	required_ioport m_joy1;
	required_ioport m_joy1_arm;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_JOYSTICK_INTERFACE, vtech_joystick_interface_device)

#endif // MAME_BUS_VTECH_IOEXP_JOYSTICK_H
