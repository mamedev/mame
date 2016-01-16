// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES SuperScope

    TODO: x,y positions are not correctly latched

**********************************************************************/

#include "sscope.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_SUPERSCOPE = &device_creator<snes_sscope_device>;


static INPUT_PORTS_START( snes_sscope )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Superscope Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Superscope Cursor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Superscope Turbo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Superscope Pause")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )    // On-screen (handled below in port_poll)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )    // Noise

	PORT_START("SSX")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Superscope X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)

	PORT_START("SSY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_NAME("Superscope Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor snes_sscope_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( snes_sscope );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_sscope_device - constructor
//-------------------------------------------------

snes_sscope_device::snes_sscope_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SNES_SUPERSCOPE, "Nintendo SNES / SFC SuperScope", tag, owner, clock, "snes_sscope", __FILE__),
					device_snes_control_port_interface(mconfig, *this),
					m_buttons(*this, "BUTTONS"),
					m_xaxis(*this, "SSX"),
					m_yaxis(*this, "SSY"), m_strobe(0), m_idx(0), m_latch(0), m_x(0), m_y(0), m_turbo_lock(0), m_pause_lock(0), m_fire_lock(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_sscope_device::device_start()
{
	save_item(NAME(m_strobe));
	save_item(NAME(m_idx));
	save_item(NAME(m_latch));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_turbo_lock));
	save_item(NAME(m_pause_lock));
	save_item(NAME(m_fire_lock));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void snes_sscope_device::device_reset()
{
	m_strobe = 0;
	m_idx = 0;
	m_latch = 0;
	m_x = 0;
	m_y = 0;
	m_turbo_lock = 0;
	m_pause_lock = 0;
	m_fire_lock = 0;
}


//-------------------------------------------------
//  poll
//-------------------------------------------------

void snes_sscope_device::port_poll()
{
	// first read input bits
	UINT8 input = m_buttons->read();
	m_x = m_xaxis->read();
	m_y = m_yaxis->read();
	m_idx = 0;

	// then start elaborating input bits
	// 1. only keep old turbo value
	m_latch &= 0x04;

	// 2. set onscreen/offscreen
	if (!m_port->m_onscreen_cb.isnull())
		m_latch |= (m_port->m_onscreen_cb(m_x, m_y) ? 0x00 : 0x40);

	// 3. pause is a button that is always edge sensitive
	if (BIT(input, 3) && !m_pause_lock)
	{
		m_latch |= 0x08;
		m_pause_lock = 1;
	}
	else if (!BIT(input, 3))
		m_pause_lock = 0;

	// 4. turbo is a switch; toggle is edge sensitive
	if (BIT(input, 2) && !m_turbo_lock)
	{
		m_latch ^= 0x04;
		m_turbo_lock = 1;
	}
	else if (!BIT(input, 2))
		m_turbo_lock = 0;

	// 5. cursor is a button that is always level sensitive
	m_latch |= BIT(input, 1);

	// 6. fire is a button with two behaviors: if turbo is active, trigger is level sensitive;
	//    otherwise it is edge sensitive
	if (BIT(input, 0) && (BIT(m_latch, 2) || !m_fire_lock))
	{
		m_latch |= 0x01;
		m_fire_lock = 1;
	}
	else if (!BIT(input, 0))
		m_fire_lock = 0;

	// If we have pressed fire or cursor and we are on-screen and SuperScope is in Port2, then latch video signal.
	// Notice that this only works in Port2 because its IOBit pin is connected to bit7 of the IO Port, while Port1
	// has IOBit pin connected to bit6 of the IO Port, and the latter is not detected by the H/V Counters. In other
	// words, you can connect SuperScope to Port1, but there is no way SNES could detect its on-screen position
	if ((m_latch & 0x03) && !(m_latch & 0x40) && !m_port->m_gunlatch_cb.isnull())
		m_port->m_gunlatch_cb(m_x, m_y);
}

//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 snes_sscope_device::read_pin4()
{
	UINT8 res = 0;

	if (m_idx >= 8) // bits 8-15 = ID = all 1s; bits >= 16 all 1s
		res |= 0x01;
	else
		res |= BIT(m_latch, m_idx++);

	return res;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_sscope_device::write_strobe(UINT8 data)
{
	int old = m_strobe;
	m_strobe = data & 0x01;

	if (m_strobe < old) // 1 -> 0 transition
		port_poll();
}
