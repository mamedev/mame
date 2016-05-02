// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    SNK Neo Geo Joystick emulation

**********************************************************************/

#pragma once

#ifndef __NEOGEO_JOYSTICK__
#define __NEOGEO_JOYSTICK__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neogeo_joystick_device

class neogeo_joystick_device : public device_t,
							public device_neogeo_control_port_interface
{
public:
	// construction/destruction
	neogeo_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_neogeo_control_port_interface overrides
	virtual UINT8 read_ctrl() override;
	virtual UINT8 read_start_sel() override;

private:
	required_ioport m_joy;
	required_ioport m_ss;
};


// ======================> neogeo_joy_ac_device

class neogeo_joy_ac_device : public device_t,
							public device_neogeo_ctrl_edge_interface
{
public:
	// construction/destruction
	neogeo_joy_ac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_neogeo_ctrl_edge_interface overrides
	virtual DECLARE_READ8_MEMBER( in0_r ) override;
	virtual DECLARE_READ8_MEMBER( in1_r ) override;

private:
	required_ioport m_joy1;
	required_ioport m_joy2;
};


// device type definition
extern const device_type NEOGEO_JOY;
extern const device_type NEOGEO_JOY_AC;


#endif
