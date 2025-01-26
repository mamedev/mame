// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SHARP_X68K_MOUSE_H
#define MAME_SHARP_X68K_MOUSE_H

#pragma once

#include "bus/rs232/rs232.h"

class x68k_mouse_device : public buffered_rs232_device<3>
{
public:
	x68k_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual void input_rts(int state) override; // MSCTRL (active low)

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void received_byte(u8 byte) override {}

private:
	required_ioport m_buttons;
	required_ioport m_x_axis;
	required_ioport m_y_axis;

	u8 m_b;
	u16 m_x;
	u16 m_y;
};

DECLARE_DEVICE_TYPE(X68K_MOUSE, x68k_mouse_device)

#endif // MAME_SHARP_X68K_MOUSE_H
