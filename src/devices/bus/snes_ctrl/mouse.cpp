// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES Mouse

**********************************************************************/

#include "mouse.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_MOUSE = &device_creator<snes_mouse_device>;


static INPUT_PORTS_START( snes_mouse )
	PORT_START("BUTTONS")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) // these must be 0!
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Button Right")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Button Left")
	PORT_BIT( 0x0c00, IP_ACTIVE_HIGH, IPT_UNUSED ) // mouse speed: 0 = slow, 1 = normal, 2 = fast, 3 = unused
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	// we use IPT_LIGHTGUN instead of IPT_MOUSE to avoid input to wrap when you reach the screen border
	// due to the relative nature of movement detection in SNES mouse, when we wrap the system would
	// detect a sudden jump in the wrong direction, making the usage unfriendly...
	PORT_START("MOUSE_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_NAME("Superscope X Axis") PORT_SENSITIVITY(30) PORT_KEYDELTA(5)
//  PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(30) PORT_KEYDELTA(5)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_Y) PORT_NAME("Superscope Y Axis") PORT_SENSITIVITY(30) PORT_KEYDELTA(5)
//  PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(30) PORT_KEYDELTA(5)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor snes_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( snes_mouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_mouse_device - constructor
//-------------------------------------------------

snes_mouse_device::snes_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SNES_MOUSE, "Nintendo SNES / SFC Mouse Controller", tag, owner, clock, "snes_mouse", __FILE__),
					device_snes_control_port_interface(mconfig, *this),
					m_buttons(*this, "BUTTONS"),
					m_xaxis(*this, "MOUSE_X"),
					m_yaxis(*this, "MOUSE_Y"), m_strobe(0), m_idx(0), m_latch(0), m_x(0), m_y(0), m_oldx(0), m_oldy(0), m_deltax(0),
	m_deltay(0), m_speed(0), m_dirx(0), m_diry(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_mouse_device::device_start()
{
	save_item(NAME(m_strobe));
	save_item(NAME(m_idx));
	save_item(NAME(m_latch));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_oldx));
	save_item(NAME(m_oldy));
	save_item(NAME(m_deltax));
	save_item(NAME(m_deltay));
	save_item(NAME(m_speed));
	save_item(NAME(m_dirx));
	save_item(NAME(m_diry));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void snes_mouse_device::device_reset()
{
	m_strobe = 0;
	m_idx = 0;
	m_latch = 0;
	m_x = 0;
	m_y = 0;
	m_oldx = 0;
	m_oldy = 0;
	m_deltax = 0;
	m_deltay = 0;
	m_speed = 0;
	m_dirx = -1;
	m_diry = -1;
}


//-------------------------------------------------
//  poll
//-------------------------------------------------

void snes_mouse_device::port_poll()
{
	INT16 var;
	int new_dir;
	m_idx = 0;
	m_latch = m_buttons->read();

	m_oldx = m_x;
	m_oldy = m_y;
	m_x = m_xaxis->read();
	m_y = m_yaxis->read();

	var = m_x - m_oldx;
	if (var)
	{
		new_dir = (var < 0) ? 1 : 0;
		if (m_dirx != new_dir)
			m_dirx = new_dir;
	}

	if (var < -127)
	{
		m_deltax = 0x7f;
		m_oldx -= 127;
	}
	else if (var < 0)
	{
		m_deltax = -var;
		m_oldx = m_x;
	}
	else if (var > 127)
	{
		m_deltax = 0x7f;
		m_oldx += 127;
	}
	else
	{
		m_deltax = var;
		m_oldx = m_x;
	}

	var = m_y - m_oldy;
	if (var)
	{
		new_dir = (var < 0) ? 1 : 0;
		if (m_diry != new_dir)
			m_diry = new_dir;
	}

	if (var < -127)
	{
		m_deltay = 0x7f;
		m_oldy -= 127;
	}
	else if (var < 0)
	{
		m_deltay = -var;
		m_oldy = m_y;
	}
	else if (var > 127)
	{
		m_deltay = 0x7f;
		m_oldy += 127;
	}
	else
	{
		m_deltay = var;
		m_oldy = m_y;
	}

	m_deltax |= (m_dirx << 7);
	m_deltay |= (m_diry << 7);
}

//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 snes_mouse_device::read_pin4()
{
	UINT8 res = 0;

	if (m_strobe == 1)
	{
		// reading with strobe 1, changes mouse speed
		m_speed = (m_speed + 1) % 3;
		return res;
	}

	if (m_idx >= 32)
		res |= 0x01;
	else if (m_idx >= 24)
		res |= BIT(m_deltax, (31 - m_idx++));
	else if (m_idx >= 16)
		res |= BIT(m_deltay, (23 - m_idx++));
	else if (m_idx == 11)
	{
		res |= BIT(m_speed, 0);
		m_idx++;
	}
	else if (m_idx == 10)
	{
		res |= BIT(m_speed, 1);
		m_idx++;
	}
	else
		res |= BIT(m_latch, m_idx++);

	return res;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_mouse_device::write_strobe(UINT8 data)
{
	int old = m_strobe;
	m_strobe = data & 0x01;

	if (m_strobe < old) // 1 -> 0 transition
		port_poll();
}
