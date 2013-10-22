// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __VCS_CONTROL_PORT__
#define __VCS_CONTROL_PORT__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VCS_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, VCS_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_VCS_CONTROL_PORT_TRIGGER_HANDLER(_devcb) \
	devcb = &vcs_control_port_device::set_trigger_handler(*device, DEVCB2_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_control_port_device

class device_vcs_control_port_interface;

class vcs_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~vcs_control_port_device();

	// static configuration helpers
	template<class _Object> static devcb2_base &set_trigger_handler(device_t &device, _Object object) { return downcast<vcs_control_port_device &>(device).m_trigger_handler.set_callback(object); }

	// computer interface

	// Data returned by the joy_r methods:
	// bit 0 - pin 1 - Up
	// bit 1 - pin 2 - Down
	// bit 2 - pin 3 - Left
	// bit 3 - pin 4 - Right
	// bit 4 - pin 5 -
	// bit 5 - pin 6 - Button
	//         pin 7 - +5V
	//         pin 8 - GND
	// bit 6 - pin 9 -
	//
	UINT8 joy_r();
	DECLARE_READ8_MEMBER( joy_r );
	UINT8 pot_x_r();
	DECLARE_READ8_MEMBER( pot_x_r );
	UINT8 pot_y_r();
	DECLARE_READ8_MEMBER( pot_y_r );

	void joy_w( UINT8 data );
	DECLARE_WRITE8_MEMBER( joy_w );

	bool exists();
	bool has_pot_x();
	bool has_pot_y();

	void trigger_w(int state);

protected:
	// device-level overrides
	virtual void device_start();

	device_vcs_control_port_interface *m_device;

private:
	devcb2_write_line m_trigger_handler;
};


// ======================> device_vcs_control_port_interface

// class representing interface-specific live vcs_expansion card
class device_vcs_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vcs_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vcs_control_port_interface();

	virtual UINT8 vcs_joy_r() { return 0xff; };
	virtual UINT8 vcs_pot_x_r() { return 0xff; };
	virtual UINT8 vcs_pot_y_r() { return 0xff; };
	virtual void vcs_joy_w(UINT8 data) { };

	virtual bool has_pot_x() { return false; }
	virtual bool has_pot_y() { return false; }

protected:
	vcs_control_port_device *m_port;
};


// device type definition
extern const device_type VCS_CONTROL_PORT;


// slot devices
#include "joybooster.h"
#include "joystick.h"
#include "keypad.h"
#include "lightpen.h"
#include "paddles.h"
#include "wheel.h"

SLOT_INTERFACE_EXTERN( vcs_control_port_devices );


#endif
