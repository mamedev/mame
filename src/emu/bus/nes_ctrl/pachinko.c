// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Pachinko Controller

**********************************************************************/

#include "pachinko.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_PACHINKO = &device_creator<nes_pachinko_device>;


static INPUT_PORTS_START( nes_pachinko )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("TRIGGER")
	PORT_BIT( 0xff, 0, IPT_PEDAL ) PORT_MINMAX(0, 0x63) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_pachinko_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_pachinko );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_pachinko_device - constructor
//-------------------------------------------------

nes_pachinko_device::nes_pachinko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_PACHINKO, "Famicom Pachinko Controller", tag, owner, clock, "nes_pachinko", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_joypad(*this, "JOYPAD"),
					m_trigger(*this, "TRIGGER")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_pachinko_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_pachinko_device::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_pachinko_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	// this controller behaves like a standard P3 joypad, with longer stream of inputs
	if (offset == 0)    //$4016
	{
		ret |= (m_latch & 1) << 1;
		m_latch >>= 1;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_pachinko_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	m_latch = m_joypad->read();
	m_latch |= ((m_trigger->read() ^ 0xff) & 0xff) << 8;
	m_latch |= 0xff0000;
}
