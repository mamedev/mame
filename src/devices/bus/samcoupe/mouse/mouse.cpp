// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Mouse Interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "mouse.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_MOUSE, sam_mouse_device, "sam_mouse", "SAM Coupe Mouse Interface")

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( mouse )
	PORT_START("buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Button 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("x")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_X) PORT_PLAYER(1) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_REVERSE

	PORT_START("y")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_Y) PORT_PLAYER(1) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)
INPUT_PORTS_END

ioport_constructor sam_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mouse );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_mouse_device - constructor
//-------------------------------------------------

sam_mouse_device::sam_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_MOUSE, tag, owner, clock),
	device_samcoupe_mouse_interface(mconfig, *this),
	m_io_buttons(*this, "buttons"),
	m_io_x(*this, "x"),
	m_io_y(*this, "y")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_mouse_device::device_start()
{
	// allocate timer
	m_reset = timer_alloc();

	// register for savestates
	save_item(NAME(m_mouse_index));
	save_item(NAME(m_mouse_data));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void sam_mouse_device::device_reset()
{
	m_mouse_index = 0;
	m_mouse_data[0] = 0xff;
	m_mouse_data[1] = 0xff;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void sam_mouse_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	m_mouse_index = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sam_mouse_device::read()
{
	uint8_t data;

	// on a read, reset the timer
	m_reset->adjust(attotime::from_usec(50));

	// update when we are about to read the first real values
	if (m_mouse_index == 2)
	{
		int mouse_x = m_io_x->read();
		int mouse_y = m_io_y->read();

		// distance moved
		int mouse_dx = m_mouse_x - mouse_x;
		int mouse_dy = m_mouse_y - mouse_y;

		m_mouse_x = mouse_x;
		m_mouse_y = mouse_y;

		m_mouse_data[2] = m_io_buttons->read();
		m_mouse_data[3] = (mouse_dy & 0xf00) >> 8;
		m_mouse_data[4] = (mouse_dy & 0x0f0) >> 4;
		m_mouse_data[5] = (mouse_dy & 0x00f) >> 0;
		m_mouse_data[6] = (mouse_dx & 0xf00) >> 8;
		m_mouse_data[7] = (mouse_dx & 0x0f0) >> 4;
		m_mouse_data[8] = (mouse_dx & 0x00f) >> 0;
	}

	data = m_mouse_data[m_mouse_index++];

	// reset if we are at the end
	if (m_mouse_index == sizeof(m_mouse_data))
		m_mouse_index = 1;

	return data;
}
