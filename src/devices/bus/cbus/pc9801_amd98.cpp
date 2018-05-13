// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    System Sacom AMD-98 (AmuseMent boarD)

    3 PSG chips, one of the first sound boards released for PC98
    Superseded by later NEC in-house sound boards

    TODO:
    - not sure if it's AY8910 or YM2203, from a PCB pic it looks with stock AY logos?
    - Third AY (uses port B from BOTH AYs);
    - PIT control;
    - PCM section;

=============================================================================

- Known games with AMD-98 support
    Brown's Run (System Sacom)
    Dome (System Sacom)
    Highway Star (System Sacom)
    Marchen Veil I (System Sacom)
    Marchen Veil II (System Sacom)
    Zone (System Sacom)
    Relics (Bothtec)
    Thexder (Game Arts)

***************************************************************************/

#include "emu.h"
#include "bus/cbus/pc9801_amd98.h"
#include "speaker.h"


#define MAIN_CLOCK_X1 XTAL(1'996'800)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_AMD98, pc9801_amd98_device, "pc9801_amd98", "pc9801_amd98")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(pc9801_amd98_device::device_add_mconfig)
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MCFG_DEVICE_ADD("ay1", AY8910, MAIN_CLOCK_X1*2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("OPN_PA1"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(*this, pc9801_amd98_device,ay3_address_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MCFG_DEVICE_ADD("ay2", AY8910, MAIN_CLOCK_X1*2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("OPN_PA2"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(*this, pc9801_amd98_device,ay3_data_latch_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_DEVICE_ADD("ay3", AY8910, MAIN_CLOCK_X1*2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)
MACHINE_CONFIG_END

static INPUT_PORTS_START( pc9801_amd98 )
	PORT_START("OPN_PA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN_PA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 Joystick Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Joystick Button 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor pc9801_amd98_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_amd98 );
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_amd98_device - constructor
//-------------------------------------------------

pc9801_amd98_device::pc9801_amd98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_AMD98, tag, owner, clock),
		m_bus(*this, DEVICE_SELF_OWNER),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_ay3(*this, "ay3")
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void pc9801_amd98_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_amd98_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth = m_bus->io_space().data_width();
	switch(buswidth)
	{
		case 8:
			m_bus->io_space().install_readwrite_handler(start, end, rhandler, whandler, 0);
			break;
		case 16:
			m_bus->io_space().install_readwrite_handler(start, end, rhandler, whandler, 0xffff);
			break;
		case 32:
			m_bus->io_space().install_readwrite_handler(start, end, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("PC-9801-AMD98: Bus width %d not supported\n", buswidth);
	}
}


void pc9801_amd98_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_amd98_device::device_reset()
{
	install_device(0x00d8, 0x00df, read8_delegate(FUNC(pc9801_amd98_device::read), this), write8_delegate(FUNC(pc9801_amd98_device::write), this) );
	// Thexder access with following
	install_device(0x38d8, 0x38df, read8_delegate(FUNC(pc9801_amd98_device::read), this), write8_delegate(FUNC(pc9801_amd98_device::write), this) );
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER(pc9801_amd98_device::read)
{
	switch(offset)
	{
		case 2:
			return m_ay1->data_r(space,0);
		case 3:
			return m_ay2->data_r(space,0);
	}

	printf("%02x\n",offset);

	return 0xff;
}

WRITE8_MEMBER(pc9801_amd98_device::write)
{
	switch(offset)
	{
		case 0:
			m_ay1->address_w(space,0,data);
			break;
		case 1:
			m_ay2->address_w(space,0,data);
			break;
		case 2:
			m_ay1->data_w(space,0,data);
			break;
		case 3:
			m_ay2->data_w(space,0,data);
			break;
		default:
			printf("%02x %02x\n",offset,data);
	}
}

WRITE8_MEMBER(pc9801_amd98_device::ay3_address_w)
{
	m_ay3_latch = data;
}

WRITE8_MEMBER(pc9801_amd98_device::ay3_data_latch_w)
{
	// TODO: this actually uses a flip flop mechanism, not quite sure about how it works yet
	switch(data)
	{
		case 0x47:
			//printf("%02x addr\n",m_ay3_latch);
			m_ay3->address_w(space,0,m_ay3_latch);
			break;
		case 0x43:
			//printf("%02x data\n",m_ay3_latch);
			m_ay3->data_w(space,0,m_ay3_latch);
			break;
	}
}
