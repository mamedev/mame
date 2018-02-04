// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein Mouse

    TODO: Verify implementation (mouse directions are digital only?)

***************************************************************************/

#include "emu.h"
#include "mouse.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EINSTEIN_MOUSE, einstein_mouse_device, "einstein_mouse", "Einstein Mouse")

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( mouse )
	PORT_START("mouse_b")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("Mouse Left Button")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("Mouse Right Button")

	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(5)

	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_PLAYER(1) PORT_SENSITIVITY(100) PORT_KEYDELTA(5)
INPUT_PORTS_END

ioport_constructor einstein_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mouse );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  einstein_speech_device - constructor
//-------------------------------------------------

einstein_mouse_device::einstein_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EINSTEIN_MOUSE, tag, owner, clock),
	device_einstein_userport_interface(mconfig, *this),
	m_mouse_b(*this, "mouse_b"),
	m_mouse_x(*this, "mouse_x"),
	m_mouse_y(*this, "mouse_y"),
	m_x(0), m_y(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_mouse_device::device_start()
{
	// register for save states
	save_item(NAME(m_x));
	save_item(NAME(m_y));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// 7-------  not used
// -6------  not used
// --5-----  right button
// ---4----  left button
// ----3---  right
// -----2--  left
// ------1-  down
// -------0  up

uint8_t einstein_mouse_device::read()
{
	uint8_t data = 0;

	uint8_t x = m_mouse_x->read();
	uint8_t y = m_mouse_y->read();

	int dx = m_x - x;
	int dy = m_y - y;

	data |= m_mouse_b->read() << 4;
	data |= (dx < 0) ? 0 : 1 << 3;
	data |= (dx > 0) ? 0 : 1 << 2;
	data |= (dy < 0) ? 0 : 1 << 1;
	data |= (dy > 0) ? 0 : 1 << 0;

	m_x = x;
	m_y = y;

	return data;
}
