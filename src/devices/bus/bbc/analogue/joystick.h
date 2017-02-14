// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Joystick Controllers

**********************************************************************/

#pragma once

#ifndef __BBC_JOYSTICK__
#define __BBC_JOYSTICK__


#include "analogue.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_joystick_device

class bbc_joystick_device :
	public device_t,
	public device_bbc_analogue_interface
{
public:
	// construction/destruction
	bbc_joystick_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t ch_r(int channel) override;
	virtual uint8_t pb_r() override;

private:
	required_ioport_array<4> m_joy;
	required_ioport m_buttons;
};

class bbc_acornjoy_device : public bbc_joystick_device
{
public:
	bbc_acornjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};

class bbc_voltmace3b_device : public bbc_joystick_device
{
public:
	bbc_voltmace3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type BBC_ACORNJOY;
extern const device_type BBC_VOLTMACE3B;


#endif
