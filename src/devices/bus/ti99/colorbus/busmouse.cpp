// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Mouse for use with the v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)
    for the TI-99/4A

    Michael Zapf, 2017-03-18

*****************************************************************************/

#include "emu.h"
#include "busmouse.h"

DEFINE_DEVICE_TYPE_NS(TI99_BUSMOUSE, bus::ti99::colorbus, geneve_busmouse_device, "ti99_busmouse", "Geneve Bus Mouse")

namespace bus { namespace ti99 { namespace colorbus {

geneve_busmouse_device::geneve_busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_BUSMOUSE, tag, owner, clock), device_ti99_colorbus_interface(mconfig, *this)
	, m_buttons(*this, "MOUSEBUT"), m_xaxis(*this, "MOUSEX"), m_yaxis(*this, "MOUSEY")
{
}

void geneve_busmouse_device::poll(int& delta_x, int& delta_y, int& buttons)
{
	buttons = m_buttons->read();
	int const new_mx = m_xaxis->read();
	int const new_my = m_yaxis->read();

	/* compute x delta */
	delta_x = new_mx - m_last_mx;
	m_last_mx = new_mx;

	/* compute y delta */
	delta_y = new_my - m_last_my;
	m_last_my = new_my;
}

void geneve_busmouse_device::device_start()
{
	save_item(NAME(m_last_mx));
	save_item(NAME(m_last_my));
}

void geneve_busmouse_device::device_reset()
{
	m_last_mx = 0;
	m_last_my = 0;
}

INPUT_PORTS_START( busmouse )
	PORT_START("MOUSEX") /* Mouse - X AXIS */
		PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEY") /* Mouse - Y AXIS */
		PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEBUT") /* mouse buttons */
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Middle mouse button")
INPUT_PORTS_END

ioport_constructor geneve_busmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( busmouse );
}
} } } // end namespace bus::ti99::colorbus
