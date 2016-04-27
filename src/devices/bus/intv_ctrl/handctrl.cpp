// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Mattel Intellivision Hand Controllers

**********************************************************************/

#include "handctrl.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type INTV_HANDCTRL = &device_creator<intv_handctrl_device>;


static INPUT_PORTS_START( intv_handctrl )
	PORT_START("KEYPAD")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Clear") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Upper") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Lower-Left") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Lower-Right") PORT_PLAYER(1)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DISC_DG")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Up") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Up-Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right-Up-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right-Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Down-Down-Right") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Down") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Down-Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left-Down-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Left") PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left-Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Up-Up-Left") PORT_CONDITION("OPTIONS",0x01,EQUALS,0x00)

	PORT_START("DISC_AN_X")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_X ) PORT_NAME("X") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x01)

	PORT_START("DISC_AN_Y")
	PORT_BIT( 0xff, 0x50, IPT_AD_STICK_Y ) PORT_NAME("Y") PORT_MINMAX(0x00,0x9f) PORT_SENSITIVITY(100) PORT_KEYDELTA(0x50) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_PLAYER(1) PORT_CONDITION("OPTIONS",0x01,EQUALS,0x01)


	PORT_START("OPTIONS")
	PORT_CONFNAME( 0x01, 0x01, "Controller Disc Emulation" )
	PORT_CONFSETTING(    0x00, "Digital" )
	PORT_CONFSETTING(    0x01, "Analog" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor intv_handctrl_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( intv_handctrl );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intv_handctrl_device - constructor
//-------------------------------------------------

intv_handctrl_device::intv_handctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, INTV_HANDCTRL, "Mattel Intellivision Hand Controller", tag, owner, clock, "intv_hand", __FILE__),
					device_intv_control_port_interface(mconfig, *this),
					m_cfg(*this, "OPTIONS"),
					m_keypad(*this, "KEYPAD"),
					m_disc_dig(*this, "DISC_DG"),
					m_disc_anx(*this, "DISC_AN_X"),
					m_disc_any(*this, "DISC_AN_Y")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intv_handctrl_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void intv_handctrl_device::device_reset()
{
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 intv_handctrl_device::read_ctrl()
{
	static const UINT8 keypad_table[] =
	{
		0xff, 0x3f, 0x9f, 0x5f, 0xd7, 0xb7, 0x77, 0xdb,
		0xbb, 0x7b, 0xdd, 0xbd, 0x7d, 0xde, 0xbe, 0x7e
	};

	static const UINT8 disc_table[] =
	{
		0xf3, 0xe3, 0xe7, 0xf7, 0xf6, 0xe6, 0xee, 0xfe,
		0xfc, 0xec, 0xed, 0xfd, 0xf9, 0xe9, 0xeb, 0xfb
	};

	static const UINT8 discyx_table[5][5] =
	{
		{ 0xe3, 0xf3, 0xfb, 0xeb, 0xe9 },
		{ 0xe7, 0xe3, 0xfb, 0xe9, 0xf9 },
		{ 0xf7, 0xf7, 0xff, 0xfd, 0xfd },
		{ 0xf6, 0xe6, 0xfe, 0xec, 0xed },
		{ 0xe6, 0xee, 0xfe, 0xfc, 0xec }
	};

	int x, y;
	UINT8 res = 0xff;

	/* keypad */
	x = m_keypad->read();
	for (int i = 0; i < 16; i++)
	{
		if (BIT(x, i))
			res &= keypad_table[i];
	}

	switch (m_cfg->read() & 1)
	{
		/* disc == digital */
		case 0:
		default:
			x = m_disc_dig->read();
			for (int i = 0; i < 16; i++)
			{
				if (BIT(x, i))
					res &= disc_table[i];
			}
			break;

		/* disc == _fake_ analog */
		case 1:
			x = m_disc_anx->read();
			y = m_disc_any->read();
			res &= discyx_table[y / 32][x / 32];
			break;
	}

	return res;
}
