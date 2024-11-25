// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco 6-speed Gearbox device
    Ridge Racer Clutch Pedal assembly part # 88800032,
    6 Speed Gear Shift assembly part # 88300672 + 88300683

    Used in Ridge Racer 1,2,Rave Racer deluxe cabinet and Driver's Eyes
    User side gear scheme:
    1 3 5
    |-|-|
    2 4 6

    Being a mechanical part there are currently two methods hooked up,
    emulated and natural.
    First one just uses whatever is read in the inputs, second one
    simulates clutch lock as in a real car.

    On the arcade cabinet(goes for Ridge Racer), the clutch isn't mechanically
    or electronically connected to the shifter. The clutch pedal simply connects
    to a microswitch. The shifter itself is basically a loose 8-way joystick with
    a panel on top to restrict movement. There is no software-controlled lock,
    maybe magnets.

    TODO:
    - Better way to lock current gear via MAME's input system; no clutch involved
    - gear output for artwork system;

***************************************************************************/

#include "emu.h"
#include "namcoio_gearbox.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NAMCOIO_GEARBOX, namcoio_gearbox_device, "namcoio_gearbox", "Namco I/O Gearbox")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  namcoio_gearbox_device - constructor
//-------------------------------------------------

namcoio_gearbox_device::namcoio_gearbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCOIO_GEARBOX, tag, owner, clock)
{
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namcoio_gearbox_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namcoio_gearbox_device::device_reset()
{
	m_gearbox_state = 0xf;
}

static INPUT_PORTS_START( gearbox_inputs )
	PORT_START("GEARBOX")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_NAME("Gearbox Up")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gearbox Down")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gearbox Left")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Gearbox Right")

	PORT_START("CLUTCH")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3   ) PORT_NAME("Clutch Pedal")

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Clutch Mode" )
	PORT_CONFSETTING(    0x01, "Emulated" )
	PORT_CONFSETTING(    0x00, "Natural" )
	PORT_CONFNAME( 0x02, 0x02, "Gearbox Debug")
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
INPUT_PORTS_END

ioport_constructor namcoio_gearbox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gearbox_inputs );
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//static const ioport_value gearbox_table[] = { 0x0f, 0x0a, 0x09, 0x0e, 0x0d, 0x06, 0x05 };

ioport_value namcoio_gearbox_device::in_r()
{
	if(ioport("CONFIG")->read() & 1)
		return ioport("GEARBOX")->read() & 0xf;

	bool clutch_pressed = (ioport("CLUTCH")->read() & 1) == 0;
	const char gearbox_output[16] = { '-', '-', '-', '-',
									  '-', '6', '5', 'N',
									  '-', '2', '1', 'N',
									  '-', '4', '3', 'N' };

	if(ioport("CONFIG")->read() & 2)
		popmessage("%c %c",gearbox_output[m_gearbox_state],clutch_pressed == true ? '*' : '.');

	if(clutch_pressed == false)
		return m_gearbox_state;

	m_gearbox_state = ioport("GEARBOX")->read() & 0xf;

	return 0xf; // return neutral while changing gear
}

int namcoio_gearbox_device::clutch_r()
{
	return ioport("CLUTCH")->read() & 1;
}
