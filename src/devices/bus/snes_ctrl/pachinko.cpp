// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom - Sunsoft Pachinko Controller

**********************************************************************/

#include "pachinko.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SNES_PACHINKO = &device_creator<snes_pachinko_device>;


static INPUT_PORTS_START( snes_pachinko )
	PORT_START("DIAL")
	PORT_BIT( 0x7f, 0x3f, IPT_PADDLE) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_CENTERDELTA(0) PORT_MINMAX(0x18,0x7f)

	PORT_START("BUTTON")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Button")
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor snes_pachinko_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( snes_pachinko );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  snes_pachinko_device - constructor
//-------------------------------------------------

snes_pachinko_device::snes_pachinko_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SNES_PACHINKO, "Sunsoft Pachinko Controller", tag, owner, clock, "snes_pachinko", __FILE__),
					device_snes_control_port_interface(mconfig, *this),
					m_dial(*this, "DIAL"),
					m_button(*this, "BUTTON"), m_strobe(0), m_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_pachinko_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void snes_pachinko_device::device_reset()
{
	m_latch = 0;
	m_strobe = 0;
}


//-------------------------------------------------
//  poll
//-------------------------------------------------

void snes_pachinko_device::port_poll()
{
	UINT8 dial = BITSWAP8(m_dial->read() ^ 0xff,7,6,5,4,3,2,1,0);
	m_latch = m_button->read() | (dial << 25) | 0xee7000;   // add ID
}

//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 snes_pachinko_device::read_pin4()
{
	UINT8 ret = m_latch & 1;
	m_latch >>= 1;
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void snes_pachinko_device::write_strobe(UINT8 data)
{
	int old = m_strobe;
	m_strobe = data & 0x01;

	if (m_strobe < old) // 1 -> 0 transition
		port_poll();
}
