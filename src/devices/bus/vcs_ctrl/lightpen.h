// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64/128 lightpen emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_LIGHTPEN_H
#define MAME_BUS_VCS_CTRL_LIGHTPEN_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_lightpen_device

class vcs_lightpen_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;

	DECLARE_INPUT_CHANGED_MEMBER( trigger );

protected:
	vcs_lightpen_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_ioport m_joy;

private:
	TIMER_CALLBACK_MEMBER( sensor_tick );

	required_ioport m_lightx;
	required_ioport m_lighty;

	emu_timer *m_sensor_timer;
	bool m_armed;
	bool m_strobed;
};


// ======================> vcs_light_pen_up_device

class vcs_light_pen_up_device : public vcs_lightpen_device
{
public:
	vcs_light_pen_up_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> vcs_light_pen_left_device

class vcs_light_pen_left_device : public vcs_lightpen_device
{
public:
	vcs_light_pen_left_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> vcs_light_pen_right_device

class vcs_light_pen_right_device : public vcs_lightpen_device
{
public:
	vcs_light_pen_right_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definitions
DECLARE_DEVICE_TYPE(VCS_LIGHTPEN_UP, vcs_light_pen_up_device)
DECLARE_DEVICE_TYPE(VCS_LIGHTPEN_LEFT, vcs_light_pen_left_device)
DECLARE_DEVICE_TYPE(VCS_LIGHTPEN_RIGHT, vcs_light_pen_right_device)

#endif // MAME_BUS_VCS_CTRL_LIGHTPEN_H
