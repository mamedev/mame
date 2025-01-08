// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Joystick Interface EG2013

***************************************************************************/

#ifndef MAME_BUS_CGENIE_PARALLEL_JOYSTICK_H
#define MAME_BUS_CGENIE_PARALLEL_JOYSTICK_H

#pragma once

#include "parallel.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cgenie_joystick_device

class cgenie_joystick_device : public device_t, public device_cg_parallel_interface
{
public:
	// construction/destruction
	cgenie_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void pa_w(uint8_t data) override;
	virtual uint8_t pb_r() override;

private:
	required_ioport_array<4> m_joy;
	required_ioport_array<6> m_keypad;

	uint8_t m_select;
};

// device type definition
DECLARE_DEVICE_TYPE(CGENIE_JOYSTICK, cgenie_joystick_device)

#endif // MAME_BUS_CGENIE_PARALLEL_JOYSTICK_H
