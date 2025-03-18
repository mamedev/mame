// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "x68k_mouse.h"

#include <algorithm>

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum status_mask : u8
{
	STS_RB = 0x01, // right button
	STS_LB = 0x02, // left button
	STS_OX = 0x10, // x overflow
	STS_UX = 0x20, // x underflow
	STS_OY = 0x40, // y overflow
	STS_UY = 0x80, // y underflow
};

DEFINE_DEVICE_TYPE(X68K_MOUSE, x68k_mouse_device, "x68k_mouse", "Sharp X68000 Mouse")

x68k_mouse_device::x68k_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: buffered_rs232_device<3>(mconfig, X68K_MOUSE, tag, owner, clock)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
{
}

void x68k_mouse_device::device_start()
{
	buffered_rs232_device<3>::device_start();

	save_item(NAME(m_b));
	save_item(NAME(m_x));
	save_item(NAME(m_y));

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_tra_rate(4'800);

	transmit_register_reset();

	m_b = 0;
	m_x = 0;
	m_y = 0;
}

s16 read_axis(ioport_port &port, u16 &old_val)
{
	u16 const new_val = port.read();
	s16 const delta = new_val - old_val;

	old_val = new_val;

	return delta;
}

void x68k_mouse_device::input_rts(int state)
{
	if (!state && fifo_empty())
	{
		u8 status = m_buttons->read();
		s16 const dx = read_axis(*m_x_axis, m_x);
		s16 const dy = read_axis(*m_y_axis, m_y);

		if (dx || dy || m_b != status)
		{
			if (dy < -128)
				status |= STS_UY;
			if (dy > 127)
				status |= STS_OY;
			if (dx < -128)
				status |= STS_UX;
			if (dx > 127)
				status |= STS_OX;

			transmit_byte(status);
			transmit_byte(s8(std::clamp<s16>(dx, -128, 127)));
			transmit_byte(s8(std::clamp<s16>(dy, -128, 127)));

			m_b = status & (STS_LB | STS_RB);
		}
	}
}

INPUT_PORTS_START(x68k)
	PORT_START("BTN")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("X")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)

	PORT_START("Y")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
INPUT_PORTS_END

ioport_constructor x68k_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(x68k);
}
