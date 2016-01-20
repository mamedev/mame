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

**********************************************************************/

#include "joypad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_JOYPAD = &device_creator<nes_joypad_device>;
const device_type NES_FCPAD_P2 = &device_creator<nes_fcpad2_device>;
const device_type NES_CCPAD_LEFT = &device_creator<nes_ccpadl_device>;
const device_type NES_CCPAD_RIGHT = &device_creator<nes_ccpadr_device>;
const device_type NES_ARCSTICK = &device_creator<nes_arcstick_device>;


static INPUT_PORTS_START( nes_joypad )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( nes_fcpad_p2 )
	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Microphone")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B")
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



static SLOT_INTERFACE_START( arcstick_daisy )
	SLOT_INTERFACE("arcstick", NES_ARCSTICK)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( arcstick )
	// expansion port to allow daisy chaining
	MCFG_FC_EXPANSION_PORT_ADD("subexp", arcstick_daisy, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nes_arcstick_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( arcstick );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_joypad_device - constructor
//-------------------------------------------------

nes_joypad_device::nes_joypad_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
					device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_nes_control_port_interface(mconfig, *this),
					m_joypad(*this, "JOYPAD"), m_latch(0)
{
}

nes_joypad_device::nes_joypad_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_JOYPAD, "Nintendo NES / FC Control Pad", tag, owner, clock, "nes_joypad", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_joypad(*this, "JOYPAD"), m_latch(0)
{
}

nes_fcpad2_device::nes_fcpad2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					nes_joypad_device(mconfig, NES_FCPAD_P2, "Nintendo Family Computer P2 Pad", tag, owner, clock, "nes_fcpad2", __FILE__)
{
}

nes_ccpadl_device::nes_ccpadl_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					nes_joypad_device(mconfig, NES_CCPAD_LEFT, "FC Crazy Climber Left Pad", tag, owner, clock, "nes_ccpadl", __FILE__)
{
}

nes_ccpadr_device::nes_ccpadr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
					nes_joypad_device(mconfig, NES_CCPAD_RIGHT, "FC Crazy Climber Right Pad", tag, owner, clock, "nes_ccpadr", __FILE__)
{
}

nes_arcstick_device::nes_arcstick_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
						nes_joypad_device(mconfig, NES_ARCSTICK, "Nintendo Family Computer Arcade Stick", tag, owner, clock, "nes_arcstick", __FILE__),
						m_daisychain(*this, "subexp"),
						m_cfg(*this, "CONFIG")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_joypad_device::device_start()
{
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_joypad_device::device_reset()
{
	m_latch = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_joypad_device::read_bit0()
{
	UINT8 ret = m_latch & 1;
	m_latch >>= 1;
	return ret;
}

UINT8 nes_fcpad2_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (!offset)    // microphone input
		ret |= m_joypad->read() & 0x04;

	return ret;
}

// NOTE: I haven't found any documentation about what happens when
// users connect two arcade stick both set as P1 or P2 in config.
// does the FC only acknowledge the first one, or do they conflict
// with each other? currently, we only support the following setup:
// if the first pad is set as P1, the daisy chained pad is checked
// for P2 only, and vice versa.
UINT8 nes_arcstick_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 0)    //$4016
	{
		if ((m_cfg->read() & 2) == 0)   // we are P1 input
		{
			ret |= (m_latch & 1) << 1;
			m_latch >>= 1;
		}
		else
			ret |= m_daisychain->read_exp(0);
	}
	else    //$4017
	{
		if ((m_cfg->read() & 2) == 2)   // we are P2 input
		{
			ret |= (m_latch & 1) << 1;
			m_latch >>= 1;
		}
		else
			ret |= m_daisychain->read_exp(1);
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_joypad_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	m_latch = m_joypad->read();
}

void nes_fcpad2_device::write(UINT8 data)
{
	if (data & 0x01)
		return;

	// microphone is hooked to expansion bits, not to the controller bit
	m_latch = m_joypad->read() & ~0x04;
}

void nes_arcstick_device::write(UINT8 data)
{
	m_daisychain->write(data);

	if (data & 0x01)
		return;

	m_latch = m_joypad->read();
}
