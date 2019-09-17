// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Mouse for use with the v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)
    for the TI-99/4A

    Michael Zapf, 2017-03-18

    2019-08-14: Changed to push behavior (MZ)

*****************************************************************************/

#include "emu.h"
#include "busmouse.h"

#define LOG_BUTTON         (1U<<1)   // Buttons
#define LOG_MOVEX          (1U<<2)   // x movement
#define LOG_MOVEY          (1U<<3)   // y movement

#define VERBOSE ( LOG_GENERAL )
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(V9938_BUSMOUSE, bus::ti99::colorbus, v9938_busmouse_device, "v9938_busmouse", "V9938 Bus Mouse")

namespace bus { namespace ti99 { namespace colorbus {

v9938_busmouse_device::v9938_busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, V9938_BUSMOUSE, tag, owner, clock),
	  device_v9938_colorbus_interface(mconfig, *this),
	  m_buttons(*this, "MOUSEBUT"),
	  m_xaxis(*this, "MOUSEX"),
	  m_yaxis(*this, "MOUSEY")
{
}

void v9938_busmouse_device::device_start()
{
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_bstate));
}

void v9938_busmouse_device::device_reset()
{
	m_last_x = 0;
	m_last_y = 0;
	m_bstate = 0;
}

INPUT_CHANGED_MEMBER( v9938_busmouse_device::mouse_button_changed )
{
	const int mask(param);
	LOGMASKED(LOG_BUTTON, "Button %d: %d\n", mask, newval);
	if (newval==1)
		m_bstate |= mask;
	else
		m_bstate &= ~mask;
	m_colorbus->buttons(m_bstate);
}

INPUT_CHANGED_MEMBER( v9938_busmouse_device::mouse_pos_changed )
{
	const int axis(param);
	int16_t pos = (int16_t)newval;
	int delta;

	if (axis==1)
	{
		delta = pos - m_last_x;
		LOGMASKED(LOG_MOVEX, "posx = %d, delta x = %d\n", pos, delta);
		m_last_x = pos;
		m_colorbus->movex(delta);
	}
	else
	{
		delta = pos - m_last_y;
		LOGMASKED(LOG_MOVEY, "posy = %d, delta y = %d\n", pos, delta);
		m_last_y = pos;
		m_colorbus->movey(delta);
	}
}

INPUT_PORTS_START( busmouse )
	PORT_START("MOUSEX") /* Mouse - X AXIS */
		PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, v9938_busmouse_device, mouse_pos_changed, 1)

	PORT_START("MOUSEY") /* Mouse - Y AXIS */
		PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, v9938_busmouse_device, mouse_pos_changed, 2)

	PORT_START("MOUSEBUT") /* mouse buttons */
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button") PORT_CHANGED_MEMBER(DEVICE_SELF, v9938_busmouse_device, mouse_button_changed, 4)
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button") PORT_CHANGED_MEMBER(DEVICE_SELF, v9938_busmouse_device, mouse_button_changed, 1)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Middle mouse button") PORT_CHANGED_MEMBER(DEVICE_SELF, v9938_busmouse_device, mouse_button_changed, 2)
INPUT_PORTS_END

ioport_constructor v9938_busmouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( busmouse );
}
} } } // end namespace bus::ti99::colorbus
