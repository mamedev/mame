// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Yonezawa / PartyRoom 21 Party Tap Controller

**********************************************************************/

#include "partytap.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_PARTYTAP = &device_creator<nes_partytap_device>;


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

nes_partytap_device::nes_partytap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_PARTYTAP, "Yonezawa Party Tap Controller", tag, owner, clock, "nes_partytap", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_inputs(*this, "INPUTS"), m_mode(0), m_latch(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_partytap_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_mode));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_partytap_device::device_reset()
{
	m_mode = 0xe0;
	m_latch = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_partytap_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 1)    //$4017
	{
		ret |= m_latch & 0x1c;
		m_latch >>= 3;
		// append mode bits
		m_latch |= m_mode;
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_partytap_device::write(UINT8 data)
{
	// inputs are read in two chunks of 3 bits, before the second one is read bit2 is written here
	// probably a mechanism for the game to detect which group of inputs is being read
	m_mode = BIT(data, 2) ? 0xa0 : 0xe0;

	if (data & 0x01)
		return;

	m_latch = m_inputs->read();
}
