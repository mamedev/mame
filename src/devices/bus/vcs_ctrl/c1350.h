// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1350/1351 mouse emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_C1350_H
#define MAME_BUS_VCS_CTRL_C1350_H

#pragma once

#include "ctrl.h"

#include "machine/quadmouse.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1350_device_base

class c1350_device_base : public device_t,
						  public device_vcs_control_port_interface
{
protected:
	// construction/destruction
	c1350_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_vcs_control_port_interface implementation
	virtual uint8_t vcs_joy_r() override;

	required_ioport m_joy;
	uint8_t m_count[2];

private:
	void encoder_w(int axis, int phase, int state);
	TIMER_CALLBACK_MEMBER(pulse_expired);

	required_device_array<quadencoder_device, 2> m_encoder;

	emu_timer *m_pulse_timer[2];
	uint8_t m_phase[2];
	int8_t m_direction[2];
};


// ======================> c1350_device

class c1350_device : public c1350_device_base
{
public:
	// construction/destruction
	c1350_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c1351_device

class c1351_device : public c1350_device_base
{
public:
	// construction/destruction
	c1351_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_vcs_control_port_interface implementation
	virtual uint8_t vcs_joy_r() override;
	virtual uint8_t vcs_pot_x_r() override;
	virtual uint8_t vcs_pot_y_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return true; }

private:
	bool m_joystick_mode;
};


// device type declarations
DECLARE_DEVICE_TYPE(C1350, c1350_device)
DECLARE_DEVICE_TYPE(C1351, c1351_device)

#endif // MAME_BUS_VCS_CTRL_C1350_H
