// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Apple II analog joysticks

*********************************************************************/

#ifndef MAME_BUS_A2GAMEIO_JOYSTICK_H
#define MAME_BUS_A2GAMEIO_JOYSTICK_H

#pragma once

#include "bus/a2gameio/gameio.h"

// ======================> apple2_joystick_device

class apple2_joystick_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	// device_a2gameio_interface overrides
	virtual u8 pdl0_r() override;
	virtual u8 pdl1_r() override;
	virtual u8 pdl2_r() override;
	virtual u8 pdl3_r() override;
	virtual DECLARE_READ_LINE_MEMBER(sw0_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw1_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw2_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw3_r) override;

private:
	// input ports
	required_ioport_array<2> m_joy_x;
	required_ioport_array<2> m_joy_y;
	required_ioport m_buttons;
};

// device type declaration
DECLARE_DEVICE_TYPE(APPLE2_JOYSTICK, apple2_joystick_device)

#endif // MAME_BUS_A2GAMEIO_JOYSTICK_H
