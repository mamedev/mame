// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Keyboard

    240 Pattern Right    0000
    241 Pattern Left     0001
    242 Patient Response 0010
    243 Present Stimuli  0011

    244 Up               0100
    245 Down             0101
    246 Left             0110
    247 Right            0111

    248 D                1000
    249 C                1001
    250 B                1010
    251 A                1011

    252 Mode             1100
    253 Erase            1101
    254 Restart          1110
    255 Print            1111

**********************************************************************/


#include "emu.h"
#include "cfa3000kbd.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CFA3000_KBD, cfa3000_kbd_device, "cfa3000kbd", "Henson CFA 3000 Keyboard")


//-------------------------------------------------
//  INPUT_PORTS( cfa3000kbd )
//-------------------------------------------------

static INPUT_PORTS_START( cfa3000kbd )
	PORT_START("KBD.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pattern ->")       PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pattern <-")       PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Patient Response") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Present Stimuli")  PORT_CODE(KEYCODE_S)
	PORT_START("KBD.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up")               PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down")             PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left/Menu")        PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right/Extend")     PORT_CODE(KEYCODE_RIGHT)
	PORT_START("KBD.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D")                PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C")                PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B")                PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A")                PORT_CODE(KEYCODE_A)
	PORT_START("KBD.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mode")             PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Erase")            PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Restart")          PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print")            PORT_CODE(KEYCODE_P)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor cfa3000_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cfa3000kbd );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cfa3000_kbd_device - constructor
//-------------------------------------------------

cfa3000_kbd_device::cfa3000_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CFA3000_KBD, tag, owner, clock),
	device_bbc_userport_interface(mconfig, *this),
	m_kbd(*this, "KBD.%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cfa3000_kbd_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cfa3000_kbd_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(cfa3000_kbd_device::pb_r)
{
	uint8_t data = 0x00;

	if (m_kbd[0]->read())
	{
		data = m_kbd[0]->read() >> 1;
		data = 0xf0 | (BIT(data, 2) ? 0x03 : data);
	}
	if (m_kbd[1]->read())
	{
		data = m_kbd[1]->read() >> 1;
		data = 0xf4 | (BIT(data, 2) ? 0x03 : data);
	}
	if (m_kbd[2]->read())
	{
		data = m_kbd[2]->read() >> 1;
		data = 0xf8 | (BIT(data, 2) ? 0x03 : data);
	}
	if (m_kbd[3]->read())
	{
		data = m_kbd[3]->read() >> 1;
		data = 0xfc | (BIT(data, 2) ? 0x03 : data);
	}
	return data;
}
