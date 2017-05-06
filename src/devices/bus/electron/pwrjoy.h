// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Power Software Joystick Interface

**********************************************************************/

#pragma once

#ifndef __ELECTRON_PWRJOY__
#define __ELECTRON_PWRJOY__


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_pwrjoy_device

class electron_pwrjoy_device :
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_pwrjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER(joystick_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_exp_rom;
	required_ioport m_joy;
};


// device type definition
extern const device_type ELECTRON_PWRJOY;


#endif
