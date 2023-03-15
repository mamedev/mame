// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Konami Hyper Shot Controllers

**********************************************************************/

#include "emu.h"
#include "konamihs.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_KONAMIHS, nes_konamihs_device, "nes_konamihs", "Konami Hyper Shot Controller")


static INPUT_PORTS_START( nes_konamihs )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("I Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("I Jump")

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("II Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("II Jump")
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

nes_konamihs_device::nes_konamihs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_KONAMIHS, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_ipt(*this, "P%u", 1)
	, m_latch(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_konamihs_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_konamihs_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	if (offset == 1)    //$4017
		for (int i = 0; i < 2; i++)
			if (BIT(m_latch, i))
				ret |= m_ipt[i]->read() << (2 * i + 1);
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_konamihs_device::write(u8 data)
{
	m_latch = ~data >> 1;
}
