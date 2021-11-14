// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Entertainment System - Bandai Power Pad

**********************************************************************/

#include "emu.h"
#include "powerpad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_POWERPAD, nes_powerpad_device, "nes_powerpad", "Bandai Power Pad")


static INPUT_PORTS_START( nes_powerpad )
	PORT_START("LAYOUT")
	PORT_CONFNAME( 0x01, 0x00, "Power Pad Button Layout")
	PORT_CONFSETTING(    0x00, "Side A" )
	PORT_CONFSETTING(    0x01, "Side B" )

	// difference between the two sides is that we mirror the key mapping to match the real pad layout!
	PORT_START("POWERPAD.0")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("PowerPad Top2")  PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )                                                    PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("PowerPad Mid4")  PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )                                                    PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("PowerPad Mid3")  PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("PowerPad Low2")  PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("PowerPad Low1")  PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("PowerPad Mid2")  PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("PowerPad 2")     PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )  PORT_NAME("PowerPad 1")     PORT_CODE(KEYCODE_R) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("PowerPad 5")     PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_NAME("PowerPad 9")     PORT_CODE(KEYCODE_V) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("PowerPad 6")     PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("PowerPad 10")    PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("PowerPad 11")    PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("PowerPad 7")     PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("POWERPAD.1")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                    PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("PowerPad Top1")  PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                    PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("PowerPad Mid1")  PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )                                                     PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )  PORT_NAME("PowerPad 4")     PORT_CODE(KEYCODE_U) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("PowerPad 3")     PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("PowerPad 12")    PORT_CODE(KEYCODE_M) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("PowerPad 8")     PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )                                                     PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
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

nes_powerpad_device::nes_powerpad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_POWERPAD, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_ipt(*this, "POWERPAD.%u", 0)
	, m_strobe(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_powerpad_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_powerpad_device::read_bit34()
{
	u8 ret = 0;

	for (int i = 0; i < 2; i++)
	{
		if (m_strobe)
			m_latch[i] = m_ipt[i]->read();
		ret |= (m_latch[i] & 1) << (i + 3);
		m_latch[i] = (m_latch[i] >> 1) | 0x80;
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_powerpad_device::write(u8 data)
{
	u8 prev_strobe = m_strobe;
	m_strobe = data & 1;

	if (prev_strobe && !m_strobe)
		for (int i = 0; i < 2; i++)
			m_latch[i] = m_ipt[i]->read();
}
