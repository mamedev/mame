// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Joypads

    The original Famicom had two hardwired controller, with the second
    controller slightly different from the first one: it featured no
    Start nor Select buttons, but had a built-in microphone (with
    very limited capabilities, since it basically only detected two
    states: something blowing into it / nothing blowing into it) for
    some games to react to users "talking" into it

    Crazy Climber Pads are not really a kind of separate controllers,
    but just a couple of small sticks to be put on top of d-pads of
    the regular controllers. Users should then control the game by
    using both controllers, turned 90 degrees, as a couple of dual
    sticks like in the arcade control panel. However, we emulate them
    separately so to map the controls to a friendlier default.

    The Arcade Stick we emulate is a controller (apparently manufactured
    by Hori, but possibly licensed by Nintendo, since it use the official
    logo and brand) which fits into the expansion port and allows to
    daisy chain a second controller to the first one, to play 4players
    game (an image of such connection is shown e.g. in Nekketsu Koukou
    Dodgeball Bu manual)

    FIXME: The official Nintendo stick mentioned above by Hori has the
    P1/P2 and 4/8-way switches but it DOESN'T have a daisy chain port.
    ASCII made at least two sticks, AS-2088-FC and AS-3399-FC, that may
    be the only two that exist with a daisy chain port. The former has
    no additional switches at all, so its port may be a pass-thru. The
    latter has many features: P1/P2 and 4/8-way switches, turbo, jacks
    for data recorder, etc.

**********************************************************************/

#include "emu.h"
#include "joypad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_JOYPAD,      nes_joypad_device,   "nes_joypad",   "Nintendo NES / FC Control Pad")
DEFINE_DEVICE_TYPE(NES_FCPAD_EXP,   nes_fcpadexp_device, "nes_fcpadexp", "Nintendo Family Computer Expansion Port Pad")
DEFINE_DEVICE_TYPE(NES_FCPAD_P2,    nes_fcpad2_device,   "nes_fcpad2",   "Nintendo Family Computer P2 Pad")
DEFINE_DEVICE_TYPE(NES_CCPAD_LEFT,  nes_ccpadl_device,   "nes_ccpadl",   "FC Crazy Climber Left Pad")
DEFINE_DEVICE_TYPE(NES_CCPAD_RIGHT, nes_ccpadr_device,   "nes_ccpadr",   "FC Crazy Climber Right Pad")
DEFINE_DEVICE_TYPE(NES_ARCSTICK,    nes_arcstick_device, "nes_arcstick", "Nintendo Family Computer Arcade Stick")
DEFINE_DEVICE_TYPE(NES_VBOYCTRL,    nes_vboyctrl_device, "nes_vboyctrl", "Nintendo Virtual Boy Controller")

INPUT_PORTS_START( nes_joypad )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( nes_fcpad_p2 )
	PORT_INCLUDE( nes_joypad )

	PORT_MODIFY("JOYPAD")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // no select button
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // or start button on controller 2

	PORT_START("MIC")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("%p Microphone")
INPUT_PORTS_END

static INPUT_PORTS_START( nes_ccpad_left )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( nes_ccpad_right )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( nes_arcstick )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "4 Way / 8 Way Joystick")
	PORT_CONFSETTING(  0x00, "4 Way" )
	PORT_CONFSETTING(  0x01, "8 Way" )
	PORT_CONFNAME( 0x02, 0x00, "Player ID")
	PORT_CONFSETTING(  0x00, "Player I" )
	PORT_CONFSETTING(  0x02, "Player II" )

	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY     PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY   PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY   PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY     PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY   PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY   PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY  PORT_CONDITION("CONFIG", 0x01, EQUALS, 0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( nes_vboyctrl )
	PORT_START("JOYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("%p L") // Left button on back
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("%p R") // Right button on back
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p B")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p A")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNUSED ) // Always 1
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED ) // Battery low
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_joypad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_joypad );
}

ioport_constructor nes_fcpad2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_fcpad_p2 );
}

ioport_constructor nes_ccpadl_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_ccpad_left );
}

ioport_constructor nes_ccpadr_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_ccpad_right );
}

ioport_constructor nes_arcstick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_arcstick );
}

ioport_constructor nes_vboyctrl_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_vboyctrl );
}

static void arcstick_daisy(device_slot_interface &device)
{
	device.option_add("arcstick", NES_ARCSTICK);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_arcstick_device::device_add_mconfig(machine_config &config)
{
	// expansion port to allow daisy chaining
	NES_CONTROL_PORT(config, m_daisychain, arcstick_daisy, nullptr);
	if (m_port != nullptr)
		m_daisychain->set_screen_tag(m_port->m_screen);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_joypad_device - constructor
//-------------------------------------------------

nes_joypad_device::nes_joypad_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 latch_fill)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_joypad(*this, "JOYPAD")
	, m_latch(0)
	, m_latch_fill(latch_fill)
{
}

nes_joypad_device::nes_joypad_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_joypad_device(mconfig, NES_JOYPAD, tag, owner, clock)
{
}

nes_fcpadexp_device::nes_fcpadexp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 latch_fill)
	: nes_joypad_device(mconfig, type, tag, owner, clock, latch_fill)
{
}

nes_fcpadexp_device::nes_fcpadexp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_fcpadexp_device(mconfig, NES_FCPAD_EXP, tag, owner, clock)
{
}

nes_fcpad2_device::nes_fcpad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_joypad_device(mconfig, NES_FCPAD_P2, tag, owner, clock)
	, m_mic(*this, "MIC")
{
}

nes_ccpadl_device::nes_ccpadl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_joypad_device(mconfig, NES_CCPAD_LEFT, tag, owner, clock)
{
}

nes_ccpadr_device::nes_ccpadr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_joypad_device(mconfig, NES_CCPAD_RIGHT, tag, owner, clock)
{
}

nes_arcstick_device::nes_arcstick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_fcpadexp_device(mconfig, NES_ARCSTICK, tag, owner, clock)
	, m_daisychain(*this, "subexp")
	, m_cfg(*this, "CONFIG")
{
}

nes_vboyctrl_device::nes_vboyctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_joypad_device(mconfig, NES_VBOYCTRL, tag, owner, clock, 0x8000)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_joypad_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_joypad_device::read_bit0()
{
	if (m_strobe)
		set_latch();

	u8 ret = m_latch & 1;
	m_latch >>= 1;
	m_latch |= m_latch_fill;

	return ret;
}

u8 nes_fcpadexp_device::read_exp(offs_t offset)
{
	return offset ? 0 : nes_joypad_device::read_bit0() << 1;
}

u8 nes_fcpad2_device::read_bit2()
{
	return m_mic->read();
}

// NOTE: I haven't found any documentation about what happens when
// users connect two arcade stick both set as P1 or P2 in config.
// does the FC only acknowledge the first one, or do they conflict
// with each other? currently, we only support the following setup:
// if the first pad is set as P1, the daisy chained pad is checked
// for P2 only, and vice versa.
u8 nes_arcstick_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (BIT(m_cfg->read(), 1) == offset)    // P# matches the port? That's us!
		ret = nes_fcpadexp_device::read_exp(0);
	else                                    // otherwise it's possibly the other controller
		ret = m_daisychain->read_exp(offset);

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_joypad_device::write(u8 data)
{
	if (write_strobe(data))
		set_latch();
}

void nes_arcstick_device::write(u8 data)
{
	m_daisychain->write(data);
	nes_fcpadexp_device::write(data);
}
