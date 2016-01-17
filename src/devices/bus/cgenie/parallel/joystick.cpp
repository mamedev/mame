// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Joystick Interface EG2013

    Keypads are organized as 3x4 matrix.

***************************************************************************/

#include "joystick.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CGENIE_JOYSTICK = &device_creator<cgenie_joystick_device>;

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( cgenie_joystick )
	PORT_START("JOY.0")
	PORT_BIT(0x3f, 0x00, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_PLAYER(1)

	PORT_START("JOY.1")
	PORT_BIT(0x3f, 0x00, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("JOY.2")
	PORT_BIT(0x3f, 0x00, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_PLAYER(2)

	PORT_START("JOY.3")
	PORT_BIT(0x3f, 0x00, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("KEYPAD.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_NAME("Keypad 1 Button 3") PORT_PLAYER(1) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_NAME("Keypad 1 Button 6") PORT_PLAYER(1) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)  PORT_NAME("Keypad 1 Button 9") PORT_PLAYER(1) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4)  PORT_NAME("Keypad 1 Button #") PORT_PLAYER(1) PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("KEYPAD.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON5)  PORT_NAME("Keypad 1 Button 2") PORT_PLAYER(1) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON6)  PORT_NAME("Keypad 1 Button 5") PORT_PLAYER(1) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON7)  PORT_NAME("Keypad 1 Button 8") PORT_PLAYER(1) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON8)  PORT_NAME("Keypad 1 Button 0") PORT_PLAYER(1) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KEYPAD.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON9)  PORT_NAME("Keypad 1 Button 1") PORT_PLAYER(1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON10) PORT_NAME("Keypad 1 Button 4") PORT_PLAYER(1) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON11) PORT_NAME("Keypad 1 Button 7") PORT_PLAYER(1) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON12) PORT_NAME("Keypad 1 Button *") PORT_PLAYER(1) PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("KEYPAD.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)  PORT_NAME("Keypad 2 Button 3") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2)  PORT_NAME("Keypad 2 Button 6") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)  PORT_NAME("Keypad 2 Button 9") PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4)  PORT_NAME("Keypad 2 Button #") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)

	PORT_START("KEYPAD.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON5)  PORT_NAME("Keypad 2 Button 2") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON6)  PORT_NAME("Keypad 2 Button 5") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON7)  PORT_NAME("Keypad 2 Button 8") PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON8)  PORT_NAME("Keypad 2 Button 0") PORT_PLAYER(2)

	PORT_START("KEYPAD.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON9)  PORT_NAME("Keypad 2 Button 1") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON10) PORT_NAME("Keypad 2 Button 4") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON11) PORT_NAME("Keypad 2 Button 7") PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON12) PORT_NAME("Keypad 2 Button *") PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

ioport_constructor cgenie_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cgenie_joystick );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cgenie_joystick_device - constructor
//-------------------------------------------------

cgenie_joystick_device::cgenie_joystick_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CGENIE_JOYSTICK, "Joystick Interface EG2013", tag, owner, clock, "cgenie_joystick", __FILE__),
	device_parallel_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_keypad(*this, "KEYPAD"),
	m_select(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cgenie_joystick_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cgenie_joystick_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void cgenie_joystick_device::pa_w(UINT8 data)
{
	if (VERBOSE)
		logerror("%s: pa_w %02x\n", tag().c_str(), data);

	// d0 to d5 connected
	m_select = data & 0x3f;
}

UINT8 cgenie_joystick_device::pb_r()
{
	UINT8 data = 0x0f;

	// read button state
	for (int i = 0; i < 4; i++)
		if (!BIT(m_select, i))
			data &= m_keypad[i]->read();

	// and joystick state
	data |= m_joy[3]->read() > m_select ? 0x10 : 0x00;
	data |= m_joy[2]->read() > m_select ? 0x20 : 0x00;
	data |= m_joy[1]->read() > m_select ? 0x40 : 0x00;
	data |= m_joy[0]->read() > m_select ? 0x80 : 0x00;

	if (VERBOSE)
		logerror("%s: pb_r %02x\n", tag().c_str(), data);

	return data;
}
