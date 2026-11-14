// license:BSD-3-Clause
// copyright-holders:Carl, Angelo Salese

#include "emu.h"
#include "basic_joy.h"

// TODO: overhaul description (it's 2 joysticks/2 axes/2 buttons)
DEFINE_DEVICE_TYPE(PC_BASIC_JOY, pc_basic_joy_device, "pc_basic_joy", "PC basic joystick")

pc_basic_joy_device::pc_basic_joy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pc_joy_interface(mconfig, *this),
	m_btn(*this, "btn"),
	m_x1(*this, "x1"),
	m_y1(*this, "y1"),
	m_x2(*this, "x2"),
	m_y2(*this, "y2")
{
}

pc_basic_joy_device::pc_basic_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pc_basic_joy_device(mconfig, PC_BASIC_JOY, tag, owner, clock)
{
}

static INPUT_PORTS_START( pc_joystick )
	// TODO: improve mapping, get rid of PORT_CODE
	PORT_START("btn")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Joystick 1 Button 2") PORT_PLAYER(1)
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(2)
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Joystick 2 Button 2") PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)

	PORT_START("x1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)

	PORT_START("y1")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)

	PORT_START("x2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)

	PORT_START("y2")
	PORT_BIT(0xff,0x80,IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2)
INPUT_PORTS_END

ioport_constructor pc_basic_joy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_joystick );
}

/*
 * Gravis PC GamePad
 *
 * https://www.pcgamingwiki.com/wiki/Controller:Gravis_PC_GamePad
 * https://dosdays.co.uk/topics/Manufacturers/gravis_controllers.php#gamepad
 *
 * Original d-pad / 4 buttons setup. Joystick can be mounted on top of the d-pad thru a central screw.
 *
 *          ‖
 * /---\_2__^_______
 * |                \
 * |  |          B  |
 * | -o-        R G |
 * |  |          Y  |
 * \_____1_____     |
 *             \----/
 *
 * 1. Button Switch for autofire
 * 2. Control Switch for left handed operation
 *
 * win95 can handle this with with either "2-axis, 2/4-button joystick" or the specific
 * "Gravis GamePad" option (which essentially is just an alias)
 *
 */

DEFINE_DEVICE_TYPE(PC_GRAVIS_GAMEPAD, pc_joy_gravis_gamepad_device, "pc_gamepad", "Gravis PC GamePad")

pc_joy_gravis_gamepad_device::pc_joy_gravis_gamepad_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: pc_basic_joy_device(mconfig, PC_GRAVIS_GAMEPAD, tag, owner, clock)
{
}

static INPUT_PORTS_START( pc_gamepad )
	// TODO: has two separate switch modes
	// Autofire rate is unknown, assumed to be divided from the base stime.
//  PORT_START("config")
//  PORT_CONFNAME(0x01, 0x01, "Operational Mode (Button Switch)")
//  PORT_CONFSETTING(   0x00, "2 Button with autofire")
//  PORT_CONFSETTING(   0x01, "4 Button")
	// Layout swaps directions, doable with a XOR if anyone has the need for it ...
//  PORT_CONFNAME(0x02, 0x02, "Layout (Control Switch)")
//  PORT_CONFSETTING(   0x00, "Left Handed")
//  PORT_CONFSETTING(   0x02, "Right Handed")

	// TODO: enumeration gets a 2 with %p use
	PORT_START("btn")
	// colors are trusted over GRAVUTIL test
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button A (Red)") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button B (Blue)") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button C (Yellow)") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button D (Green)") PORT_PLAYER(1)
	// autofire layout
//  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button A (Red)") PORT_PLAYER(1)
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button B (Yellow)") PORT_PLAYER(1)
//  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button A-turbo (Blue)") PORT_PLAYER(1)
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button B-turbo (Green)") PORT_PLAYER(1)

	PORT_START("x1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)

	PORT_START("y1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)

	PORT_START("x2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("y2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc_joy_gravis_gamepad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_gamepad );
}

/*
 * Interact Magnum 6 Game Controller
 *
 * Flight stick, default with 6 buttons, has alternative modes with extra throttle and X view
 * sliders.
 *
 * Software driver contained in ibm5170:magnum6
 *
 * ibm5170_cdrom:fulltilt (at very least) expects L and R to be held low otherwise it goes
 * 3 FPS with this enabled
 *
 */

DEFINE_DEVICE_TYPE(PC_MAGNUM6_STICK, pc_joy_magnum6_device, "magnum6_stick", "Interact Magnum 6 Game Controller")

pc_joy_magnum6_device::pc_joy_magnum6_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: pc_basic_joy_device(mconfig, PC_MAGNUM6_STICK, tag, owner, clock)
{
}

static INPUT_PORTS_START( magnum6 )
	// TODO: input modes from bottom of base
	// bit format unknown, these are from bullet points appearing at the end of the installation
//  PORT_START("config")
//  PORT_CONFNAME(0x03, 0x01, "Operational Mode")
//  PORT_CONFSETTING(   0x01, "6 Button")
//  PORT_CONFSETTING(   0x02, "4 Button with Throttle")
//  PORT_CONFSETTING(   0x03, "4 Button with 4-way View")

	// TODO: enumeration gets a 2 with %p use
	PORT_START("btn")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Joystick 1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Joystick 1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Joystick 1 Button C") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Joystick 1 Button D") PORT_PLAYER(1)
	// Naming from driver test utility, chassis base has an 'F' and a '6' labels instead (and no 'E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Joystick 1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Joystick 1 Button L") PORT_PLAYER(1)

	PORT_START("x1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)

	PORT_START("y1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_MINMAX(1,0xff) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)

	PORT_START("x2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("y2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc_joy_magnum6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( magnum6 );
}
