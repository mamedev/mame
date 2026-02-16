// license:BSD-3-Clause
// copyright-holders: Oleksandr Kovalchuk
/**********************************************************************

    Kempston Mouse Interface (original, 2-buttons)

    Description:
    https://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Mouse%20Interfaces/Kempston%20Mouse%20Interface/

	Mouse interface with two 8bit reverse counters. Readed in ports:
    0xFADF (button bits: 0 - right, 1 - left)
    0xFBDF - X
    0xFFDF - Y

**********************************************************************/

#include "emu.h"
#include "kempmouse.h"

DEFINE_DEVICE_TYPE(SPECTRUM_KEMPMOUSE, spectrum_kempmouse_device, "spectrum_kempmouse", "Kempston Mouse Interface")

static INPUT_PORTS_START( kempmouse )
	PORT_START("mouse_x")       // 0xFBDF
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_y")       // 0xFFDF
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_REVERSE PORT_SENSITIVITY(30)

	PORT_START("mouse_buttons") // 0xFADF
	// The right key was the primary key in most implementations in the 1980s–1990s.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button Right") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Button Left") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


ioport_constructor spectrum_kempmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kempmouse );
}

spectrum_kempmouse_device::spectrum_kempmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_KEMPMOUSE, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_mouse_x(*this, "mouse_x")
	, m_mouse_y(*this, "mouse_y")
	, m_mouse_buttons(*this, "mouse_buttons")
{
}

void spectrum_kempmouse_device::device_start()
{
}

uint8_t spectrum_kempmouse_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	switch (offset & 0xffff)
	{
	case 0xfbdf:
		data = m_mouse_x->read() & 0xff;
		break;
	case 0xffdf:
		data = m_mouse_y->read() & 0xff;
		break;
	case 0xfadf:
		data = m_mouse_buttons->read() | (0xff ^ 0x03);
		break;
	}

	return data;
}
