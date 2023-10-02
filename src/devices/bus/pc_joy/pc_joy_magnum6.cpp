// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc_joy_magnum6.h"

DEFINE_DEVICE_TYPE(PC_MAGNUM6_PAD, pc_joy_magnum6_device, "magnum6_pad", "Interact Magnum 6 Game Controller")


pc_joy_magnum6_device::pc_joy_magnum6_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: pc_basic_joy_device(mconfig, PC_MAGNUM6_PAD, tag, owner, clock)
{
}

static INPUT_PORTS_START( magnum6 )
	// TODO: input modes (4-button with throttle and 4-way view)
	PORT_START("btn")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Joystick 1 Button A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Joystick 1 Button B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Joystick 1 Button C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Joystick 1 Button D")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Joystick 1 Button R")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Joystick 1 Button L")

	PORT_START("x1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("y1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("x2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("y2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc_joy_magnum6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( magnum6 );
}
