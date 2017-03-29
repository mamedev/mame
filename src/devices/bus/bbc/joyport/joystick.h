// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Joystick

**********************************************************************/

#pragma once

#ifndef __BBCMC_JOYSTICK__
#define __BBCMC_JOYSTICK__


#include "joyport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbcmc_joystick_device

class bbcmc_joystick_device :
	public device_t,
	public device_bbc_joyport_interface
{
public:
	// construction/destruction
	bbcmc_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t cb_r() override;
	virtual uint8_t pb_r() override;

private:
	required_ioport m_joy;
};


// device type definition
extern const device_type BBCMC_JOYSTICK;


#endif
