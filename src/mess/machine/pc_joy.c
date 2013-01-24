/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#include "emu.h"
#include "pc_joy.h"

READ8_MEMBER ( pc_joy_device::joy_port_r )
{
	UINT8 data = 0xf;
	int delta;
	attotime new_time = machine().time();
	delta = ((new_time - m_stime) * 256 * 1000).seconds;

	data = m_btn->read() ^ 0xf0;

	/* timer overflow? Nope, otherwise some HWs can't verify the port properly */
	//if ((new_time - JOY_time) > attotime::from_msec(10))
	//{
	//}
	//else

	if (m_x1->read() < delta) data &= ~0x01;
	if (m_y1->read() < delta) data &= ~0x02;
	if (m_x2->read() < delta) data &= ~0x04;
	if (m_y2->read() < delta) data &= ~0x08;
	return data;
}



WRITE8_MEMBER ( pc_joy_device::joy_port_w )
{
	m_stime = machine().time();
}

static INPUT_PORTS_START( pc_joystick )
	PORT_START("btn")
	PORT_BIT ( 0xf, 0xf,     IPT_UNUSED )
	PORT_BIT( 0x0010, 0x0000, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1")
	PORT_BIT( 0x0020, 0x0000, IPT_BUTTON2) PORT_NAME("Joystick 1 Button 2")
	PORT_BIT( 0x0040, 0x0000, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x0080, 0x0000, IPT_BUTTON2) PORT_NAME("Joystick 2 Button 2") PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)

	PORT_START("x1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("y1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("x2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)

	PORT_START("y2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2)
INPUT_PORTS_END

const device_type PC_JOY = &device_creator<pc_joy_device>;

pc_joy_device::pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_JOY, "PC joystick", tag, owner, clock),
	m_btn(*this, "btn"),
	m_x1(*this, "x1"),
	m_y1(*this, "y1"),
	m_x2(*this, "x2"),
	m_y2(*this, "y2")
{
}

ioport_constructor pc_joy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_joystick );
}
