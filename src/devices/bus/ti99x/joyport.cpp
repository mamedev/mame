// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Joystick port

    Now explicitly implemented as a slot device
    A joystick port allows for plugging in digital devices like joysticks or
    a Mechatronics mouse, and the TI-99/4 (prototype) also offered IR handsets
    driven over this port. The 99/4 had an additional line for triggering an
    interrupt.

    +-----------+
    | 1 2 3 4 5 |
     \ 6 7 8 9 /
      +-------+

    Getting the joystick directions and button is pretty simple: The TMS9901 in
    the TI console lowers the select line, and the joystick shorts a line for
    the respective action. The lines go back to the inputs of the TMS9901.

    pin 1   nc
        2   select joystick 2
        3   up
        4   button
        5   left
        6   nc
        7   select joystick 1
        8   down
        9   right

    Michael Zapf

    June 2012

*****************************************************************************/

#include "joyport.h"
#include "handset.h"
#include "mecmouse.h"

joyport_device::joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:   device_t(mconfig, JOYPORT, "Joystick port", tag, owner, clock, "ti99_joyport", __FILE__),
		device_slot_interface(mconfig, *this),
		m_interrupt(*this), m_connected(nullptr)
{
}

/*
    Reads a value from the port.
*/
UINT8 joyport_device::read_port()
{
	return m_connected->read_dev();
}

/*
    This is used to select the device at the port. The device should keep this
    value until read() is called.
*/
void joyport_device::write_port(int data)
{
	m_connected->write_dev(data);
}

/*
    This is only used for the handset device of the TI-99/4. It is driven by the VDP interrupt.
*/
void joyport_device::pulse_clock()
{
	m_connected->pulse_clock();
}

/*
    Propagate the interrupt to the defined target. Only used for the handset
    at the prototype 99/4.
*/
WRITE_LINE_MEMBER( joyport_device::set_interrupt )
{
	m_interrupt(state);
}

void joyport_device::device_start()
{
	m_interrupt.resolve();
}

void joyport_device::device_config_complete()
{
	m_connected = static_cast<joyport_attached_device*>(subdevices().first());
}

/*****************************************************************************/

void joyport_attached_device::device_config_complete()
{
	m_joyport = static_cast<joyport_device*>(owner());
}

SLOT_INTERFACE_START( joystick_port )
	SLOT_INTERFACE("twinjoy", TI99_JOYSTICK)
	SLOT_INTERFACE("mecmouse", MECMOUSE)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( joystick_port_gen )
	SLOT_INTERFACE("twinjoy", TI99_JOYSTICK)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( joystick_port_994 )
	SLOT_INTERFACE("twinjoy", TI99_JOYSTICK)
	SLOT_INTERFACE("mecmouse", MECMOUSE)
	SLOT_INTERFACE("handset", HANDSET)
SLOT_INTERFACE_END

const device_type JOYPORT = &device_creator<joyport_device>;
