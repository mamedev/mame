// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Acorn Risc PC Mouse
 *
 * TODO:
 *  - slotify
 */

#include "emu.h"
#include "riscpc_mouse.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

riscpc_mouse_device::riscpc_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: quadmouse_device(mconfig, RISCPC_MOUSE, tag, owner, clock)
	, m_buttons(*this, "buttons")
{
}

void riscpc_mouse_device::device_start()
{
	quadmouse_device::device_start();
}

void riscpc_mouse_device::device_reset()
{
	quadmouse_device::device_reset();
}

ioport_value riscpc_mouse_device::buttons_r()
{
	return m_buttons->read();
}

INPUT_PORTS_EXTERN(quadmouse);

static INPUT_PORTS_START(riscpc_mouse)
	PORT_INCLUDE(quadmouse)

	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Right")   PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Center")  PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Left")    PORT_CODE(MOUSECODE_BUTTON1)
INPUT_PORTS_END

ioport_constructor riscpc_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(riscpc_mouse);
}

DEFINE_DEVICE_TYPE(RISCPC_MOUSE, riscpc_mouse_device, "riscpc_mouse", "Acorn Risc PC Mouse")
