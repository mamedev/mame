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
#include "bus/ti99x/ti99defs.h"

ti99_colorbus_device::ti99_colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	:   device_t(mconfig, TI99_COLORBUS, tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_connected(nullptr),
		m_v9938(*owner, VDP_TAG),
		m_left_button_pressed(false)
{
}

void ti99_colorbus_device::poll()
{
	int delta_x, delta_y, buttons;

	// only middle and right button go to V9938
	m_connected->poll(delta_x, delta_y, buttons);
	m_v9938->update_mouse_state(delta_x, delta_y, buttons & 0x03);
	m_left_button_pressed = (buttons & 0x04)!=0;
}

line_state ti99_colorbus_device::left_button()
{
	return m_left_button_pressed? ASSERT_LINE : CLEAR_LINE;
}

void ti99_colorbus_device::device_config_complete()
{
	m_connected = dynamic_cast<device_ti99_colorbus_interface*>(subdevices().first());
}

/*****************************************************************************/

void device_ti99_colorbus_interface::interface_config_complete()
{
	m_colorbus = dynamic_cast<ti99_colorbus_device*>(device().owner());
}

SLOT_INTERFACE_START( ti99_colorbus_port )
	SLOT_INTERFACE("busmouse", TI99_BUSMOUSE)
SLOT_INTERFACE_END

DEFINE_DEVICE_TYPE(TI99_COLORBUS, ti99_colorbus_device, "ti99_colorbus", "v9938 Color bus")
