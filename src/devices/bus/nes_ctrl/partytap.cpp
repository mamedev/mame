// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Yonezawa / Party Room 21 Partytap Controller

**********************************************************************/

#include "emu.h"
#include "partytap.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_PARTYTAP, nes_partytap_device, "nes_partytap", "Yonezawa Partytap Controller")


static INPUT_PORTS_START( nes_partytap )
	PORT_START("INPUTS")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Button") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Button") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P3 Button") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P4 Button") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P5 Button") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P6 Button") PORT_CODE(KEYCODE_N)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_partytap_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_partytap );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_partytap_device - constructor
//-------------------------------------------------

nes_partytap_device::nes_partytap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_PARTYTAP, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_inputs(*this, "INPUTS")
	, m_latch(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_partytap_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_partytap_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	if (offset == 1)    //$4017
	{
		if (m_strobe)
			m_latch = m_inputs->read();
		ret = m_latch & 0x1c;
		m_latch >>= 3;
		m_latch |= 0xa0; // after first two reads, 0x14 will be returned
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_partytap_device::write(u8 data)
{
	if (write_strobe(data))
		m_latch = m_inputs->read();
}
