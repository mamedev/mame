// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Konami Hyper Shot Controllers

**********************************************************************/

#include "konamihs.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_KONAMIHS = &device_creator<nes_konamihs_device>;


static INPUT_PORTS_START( nes_konamihs )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("PI Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("PI Jump")

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("PII Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("PII Jump")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_konamihs_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_konamihs );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_konamihs_device - constructor
//-------------------------------------------------

nes_konamihs_device::nes_konamihs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_KONAMIHS, "Konami Hyper Shot Controller", tag, owner, clock, "nes_konamihs", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_ipt_p1(*this, "P1"),
					m_ipt_p2(*this, "P2"), m_latch_p1(0), m_latch_p2(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_konamihs_device::device_start()
{
	save_item(NAME(m_latch_p1));
	save_item(NAME(m_latch_p2));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_konamihs_device::device_reset()
{
	m_latch_p1 = 0;
	m_latch_p2 = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_konamihs_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 1)    //$4017
	{
		ret |= m_latch_p1 << 1;
		ret |= m_latch_p2 << 3;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_konamihs_device::write(UINT8 data)
{
	if ((data & 0x02) == 0)
		m_latch_p1 = m_ipt_p1->read();
	if ((data & 0x04) == 0)
		m_latch_p2 = m_ipt_p2->read();
}
