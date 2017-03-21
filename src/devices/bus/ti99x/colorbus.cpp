// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    v9938 Color bus

    The color bus runs from the v9938 video processor to some external device
    like a mouse or a lightpen.

    The Geneve mouse offers three buttons, of which the leftmost one is not
    connected to the v9938; the color bus implementation on the Geneve board
    offers a separate line going to the 9901.

    Michael Zapf

    March 2017

*****************************************************************************/

#include "emu.h"
#include "colorbus.h"
#include "busmouse.h"
#include "ti99defs.h"

colorbus_device::colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   device_t(mconfig, COLORBUS, "Color bus", tag, owner, clock, "colorbus", __FILE__),
		device_slot_interface(mconfig, *this),
		m_connected(nullptr),
		m_v9938(*owner, VDP_TAG),
		m_left_button_pressed(false)
{
}

void colorbus_device::poll()
{
	int delta_x, delta_y, buttons;

	// only middle and right button go to V9938
	m_connected->poll(delta_x, delta_y, buttons);
	m_v9938->update_mouse_state(delta_x, delta_y, buttons & 0x03);
	m_left_button_pressed = (buttons & 0x04)!=0;
}

line_state colorbus_device::left_button()
{
	return m_left_button_pressed? ASSERT_LINE : CLEAR_LINE;
}

void colorbus_device::device_config_complete()
{
	m_connected = static_cast<colorbus_attached_device*>(subdevices().first());
}

/*****************************************************************************/

void colorbus_attached_device::device_config_complete()
{
	m_colorbus = static_cast<colorbus_device*>(owner());
}

SLOT_INTERFACE_START( colorbus_port )
	SLOT_INTERFACE("busmouse", BUSMOUSE)
SLOT_INTERFACE_END

const device_type COLORBUS = device_creator<colorbus_device>;
