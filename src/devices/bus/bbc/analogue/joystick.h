// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Joystick Controllers

**********************************************************************/

#ifndef MAME_BUS_BBC_ANALOGUE_JOYSTICK_H
#define MAME_BUS_BBC_ANALOGUE_JOYSTICK_H

#pragma once


#include "analogue.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_joystick_device

class bbc_joystick_device :
	public device_t,
	public device_bbc_analogue_interface
{
protected:
	// construction/destruction
	bbc_joystick_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class bbc_voltmace3b_device : public bbc_joystick_device
{
public:
	bbc_voltmace3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ACORNJOY,   bbc_acornjoy_device)
DECLARE_DEVICE_TYPE(BBC_VOLTMACE3B, bbc_voltmace3b_device)

#endif // MAME_BUS_BBC_ANALOGUE_JOYSTICK_H
