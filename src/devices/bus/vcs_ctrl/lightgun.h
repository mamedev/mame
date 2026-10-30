// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64/128 light gun emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_LIGHTGUN_H
#define MAME_BUS_VCS_CTRL_LIGHTGUN_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_lightgun_device

class vcs_lightgun_device : public device_t,
							public device_vcs_control_port_interface
{
protected:
	vcs_lightgun_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_ioport m_lightx;
	required_ioport m_lighty;

private:
	TIMER_CALLBACK_MEMBER( sensor_tick );

	emu_timer *m_sensor_timer;
};


// ======================> magnum_light_phaser_device

class magnum_light_phaser_device : public vcs_lightgun_device
{
public:
	magnum_light_phaser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_pot_y_r() override;
	virtual bool has_pot_y() override { return true; }

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_trigger;
};


// ======================> stack_light_rifle_device

class stack_light_rifle_device : public vcs_lightgun_device
{
public:
	stack_light_rifle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joy;
};


// ======================> inkwell_184c_device

class inkwell_184c_device : public vcs_lightgun_device
{
public:
	inkwell_184c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;
	virtual uint8_t vcs_pot_y_r() override;
	virtual bool has_pot_y() override { return true; }

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joy;
	required_ioport m_button2;
};


// ======================> vcs_gunstick_device

class vcs_gunstick_device : public device_t,
                			public device_vcs_control_port_interface
{
public:
  vcs_gunstick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

  // device_vcs_control_port_interface overrides
  virtual uint8_t vcs_joy_r() override;

protected:
  // device_t implementation
  virtual void device_start() override ATTR_COLD;

  // optional information overrides
  virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
  required_ioport m_joy;
  required_ioport m_lightx;
  required_ioport m_lighty;
};


// device type definitions
DECLARE_DEVICE_TYPE(MAGNUM_LIGHT_PHASER, magnum_light_phaser_device)
DECLARE_DEVICE_TYPE(STACK_LIGHT_RIFLE, stack_light_rifle_device)
DECLARE_DEVICE_TYPE(GUN_STICK, vcs_gunstick_device)
DECLARE_DEVICE_TYPE(INKWELL_184C, inkwell_184c_device)

#endif // MAME_BUS_VCS_CTRL_LIGHTGUN_H
