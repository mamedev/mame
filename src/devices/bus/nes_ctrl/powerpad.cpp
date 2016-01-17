// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System - Bandai Power Pad

**********************************************************************/

#include "powerpad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_POWERPAD = &device_creator<nes_powerpad_device>;


static INPUT_PORTS_START( nes_powerpad )
	PORT_START("LAYOUT")
	PORT_CONFNAME( 0x01, 0x00, "Power Pad Button Layout")
	PORT_CONFSETTING(  0x00, "Side A" )
	PORT_CONFSETTING(  0x01, "Side B" )

	// difference between the two sides is that we mirror the key mapping to match the real pad layout!
	PORT_START("POWERPAD1")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Top1")  PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Mid1")  PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Mid2")  PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Low1")  PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Low2")  PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Mid3")  PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 2")     PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 1")     PORT_CODE(KEYCODE_R) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 5")     PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 9")     PORT_CODE(KEYCODE_V) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 6")     PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 10")    PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 11")    PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 7")     PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("POWERPAD2")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Top2")  PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad Mid4")  PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 4")     PORT_CODE(KEYCODE_U) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 3")     PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 12")    PORT_CODE(KEYCODE_M) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("PowerPad 8")     PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )                                                  PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_powerpad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_powerpad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_powerpad_device - constructor
//-------------------------------------------------

nes_powerpad_device::nes_powerpad_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_POWERPAD, "Bandai Power Pad", tag, owner, clock, "nes_powerpad", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_ipt1(*this, "POWERPAD1"),
					m_ipt2(*this, "POWERPAD2")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_powerpad_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_powerpad_device::device_reset()
{
	m_latch[0] = 0;
	m_latch[1] = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_powerpad_device::read_bit34()
{
	UINT8 ret = 0;
	ret |= (m_latch[0] & 0x01) << 3;
	ret |= (m_latch[1] & 0x01) << 4;
	m_latch[0] >>= 1;
	m_latch[1] >>= 1;
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_powerpad_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	m_latch[0] = m_ipt1->read();
	m_latch[1] = m_ipt2->read() | 0xf0;
}
