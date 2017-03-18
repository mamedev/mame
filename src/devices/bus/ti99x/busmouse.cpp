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

busmouse_device::busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: colorbus_attached_device(mconfig, BUSMOUSE, "Bus Mouse", tag, owner, clock, "busmouse", __FILE__)
{
}

void busmouse_device::poll(int& delta_x, int& delta_y, int& buttons)
{
	int new_mx, new_my;

	buttons = ioport("MOUSEBUT")->read();
	new_mx = ioport("MOUSEX")->read();
	new_my = ioport("MOUSEY")->read();

	/* compute x delta */
	delta_x = new_mx - m_last_mx;

	/* check for wrap */
	if (delta_x > 0x80)
		delta_x = 0x100-delta_x;
	if  (delta_x < -0x80)
		delta_x = -0x100-delta_x;

	m_last_mx = new_mx;

	/* compute y delta */
	delta_y = new_my - m_last_my;

	/* check for wrap */
	if (delta_y > 0x80)
		delta_y = 0x100-delta_y;
	if  (delta_y < -0x80)
		delta_y = -0x100-delta_y;

	m_last_my = new_my;
}

void busmouse_device::device_start(void)
{
	save_item(NAME(m_last_mx));
	save_item(NAME(m_last_my));
}

void busmouse_device::device_reset(void)
{
	m_last_mx = 0;
	m_last_my = 0;
}

INPUT_PORTS_START( busmouse )
	PORT_START("MOUSEX") /* Mouse - X AXIS */
		PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEY") /* Mouse - Y AXIS */
		PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEBUT") /* mouse buttons */
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Middle mouse button")
INPUT_PORTS_END

ioport_constructor busmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( busmouse );
}

const device_type BUSMOUSE = device_creator<busmouse_device>;
