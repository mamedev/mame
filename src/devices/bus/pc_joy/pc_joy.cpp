// license:BSD-3-Clause
// copyright-holders:Carl
/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#include "emu.h"
#include "pc_joy.h"
#include "pc_joy_sw.h"

DEFINE_DEVICE_TYPE(PC_JOY, pc_joy_device, "pc_joy", "PC joystick port")

pc_joy_device::pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC_JOY, tag, owner, clock),
	device_single_card_slot_interface<device_pc_joy_interface>(mconfig, *this),
	m_dev(nullptr)
{
	option_reset();
	pc_joysticks(*this);
	set_default_option("basic_joy");
	set_fixed(false);
}

READ8_MEMBER ( pc_joy_device::joy_port_r )
{
	int delta = ((machine().time() - m_stime) * 256 * 2325).seconds();

	if(!m_dev)
		return 0xf0;

	return (m_dev->btn() << 4) | (m_dev->y2(delta) << 3) | (m_dev->x2(delta) << 2) | (m_dev->y1(delta) << 1) | m_dev->x1(delta);
}

WRITE8_MEMBER ( pc_joy_device::joy_port_w )
{
	m_stime = machine().time();
	if(!m_dev)
		return;
	m_dev->port_write();
}

void pc_joy_device::device_config_complete()
{
	m_dev = get_card_device();
}

device_pc_joy_interface::device_pc_joy_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pcjoy")
{
}

device_pc_joy_interface::~device_pc_joy_interface()
{
}

static INPUT_PORTS_START( pc_joystick )
	PORT_START("btn")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1")
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Joystick 1 Button 2")
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Joystick 2 Button 2") PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)

	PORT_START("x1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("y1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("x2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)

	PORT_START("y2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2)
INPUT_PORTS_END

ioport_constructor pc_basic_joy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_joystick );
}

DEFINE_DEVICE_TYPE(PC_BASIC_JOY, pc_basic_joy_device, "pc_basic_joy", "PC basic joystick")

pc_basic_joy_device::pc_basic_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC_BASIC_JOY, tag, owner, clock),
	device_pc_joy_interface(mconfig, *this),
	m_btn(*this, "btn"),
	m_x1(*this, "x1"),
	m_y1(*this, "y1"),
	m_x2(*this, "x2"),
	m_y2(*this, "y2")
{
}

void pc_joysticks(device_slot_interface &device)
{
	device.option_add("basic_joy", PC_BASIC_JOY);
	device.option_add("mssw_pad", PC_MSSW_PAD);
}
