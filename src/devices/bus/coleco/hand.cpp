// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision Hand Controller emulation

**********************************************************************/

#include "hand.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COLECO_HAND_CONTROLLER = &device_creator<coleco_hand_controller_t>;


CUSTOM_INPUT_MEMBER( coleco_hand_controller_t::keypad_r )
{
	UINT8 data = 0xf;
	UINT16 keypad = m_io_keypad->read();

	if (!BIT(keypad, 0)) data &= 0x0a;
	if (!BIT(keypad, 1)) data &= 0x0d;
	if (!BIT(keypad, 2)) data &= 0x07;
	if (!BIT(keypad, 3)) data &= 0x0c;
	if (!BIT(keypad, 4)) data &= 0x02;
	if (!BIT(keypad, 5)) data &= 0x03;
	if (!BIT(keypad, 6)) data &= 0x0e;
	if (!BIT(keypad, 7)) data &= 0x05;
	if (!BIT(keypad, 8)) data &= 0x01;
	if (!BIT(keypad, 9)) data &= 0x0b;
	if (!BIT(keypad, 10)) data &= 0x06;
	if (!BIT(keypad, 11)) data &= 0x09;

	return data;
}

static INPUT_PORTS_START( coleco_hand_controller )
	PORT_START("COMMON0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("COMMON1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, coleco_hand_controller_t, keypad_r, nullptr)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )

	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_PLUS_PAD)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coleco_hand_controller_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( coleco_hand_controller );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coleco_hand_controller_t - constructor
//-------------------------------------------------

coleco_hand_controller_t::coleco_hand_controller_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COLECO_HAND_CONTROLLER, "ColecoVision Hand Controller", tag, owner, clock, "coleco_hand", __FILE__),
	device_colecovision_control_port_interface(mconfig, *this),
	m_io_common0(*this, "COMMON0"),
	m_io_common1(*this, "COMMON1"),
	m_io_keypad(*this, "KEYPAD")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coleco_hand_controller_t::device_start()
{
	// state saving
	save_item(NAME(m_common0));
	save_item(NAME(m_common1));
}


//-------------------------------------------------
//  joy_r - joystick read
//-------------------------------------------------

UINT8 coleco_hand_controller_t::joy_r()
{
	UINT8 data = 0x7f;

	if (!m_common0) data &= m_io_common0->read();
	if (!m_common1) data &= m_io_common1->read();

	return data;
}
