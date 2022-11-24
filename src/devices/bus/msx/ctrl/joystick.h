// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX Digital Joystick emulation

**********************************************************************/

#ifndef MAME_BUS_MSX_CTRL_JOYSTICK_H
#define MAME_BUS_MSX_CTRL_JOYSTICK_H

#pragma once

#include "ctrl.h"


class msx_joystick_device : public device_t,
							public device_msx_general_purpose_port_interface
{
public:
	msx_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;

protected:
	virtual void device_start() override { }
	virtual ioport_constructor device_input_ports() const override;

private:
	required_ioport m_joy;
};


DECLARE_DEVICE_TYPE(MSX_JOYSTICK, msx_joystick_device)


#endif // MAME_BUS_MSX_CTRL_JOYSTICK_H
