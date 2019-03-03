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


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PC9801_AMD98, pc9801_amd98_device, "pc9801_amd98", "pc9801_amd98")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pc9801_amd98_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8910(config, m_ay1, 1'996'800);
	m_ay1->port_a_read_callback().set_ioport("OPN_PA1");
	m_ay1->port_b_write_callback().set(FUNC(pc9801_amd98_device::ay3_address_w));
	m_ay1->add_route(ALL_OUTPUTS, "lspeaker", 0.50);

	AY8910(config, m_ay2, 1'996'800);
	m_ay2->port_a_read_callback().set_ioport("OPN_PA2");
	m_ay2->port_b_write_callback().set(FUNC(pc9801_amd98_device::ay3_data_latch_w));
	m_ay2->add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	AY8910(config, m_ay3, 1'996'800);
	m_ay3->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_ay3->add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}

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

void pc9801_amd98_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc9801_amd98_device::device_reset()
{
	m_bus->install_io(0x00d8, 0x00df, read8_delegate(FUNC(pc9801_amd98_device::read), this), write8_delegate(FUNC(pc9801_amd98_device::write), this) );
	// Thexder access with following
	m_bus->install_io(0x38d8, 0x38df, read8_delegate(FUNC(pc9801_amd98_device::read), this), write8_delegate(FUNC(pc9801_amd98_device::write), this) );
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER(pc9801_amd98_device::read)
{
	switch(offset)
	{
		case 2:
			return m_ay1->data_r();
		case 3:
			return m_ay2->data_r();
	}

	printf("%02x\n",offset);

	return 0xff;
}

WRITE8_MEMBER(pc9801_amd98_device::write)
{
	switch(offset)
	{
		case 0:
			m_ay1->address_w(data);
			break;
		case 1:
			m_ay2->address_w(data);
			break;
		case 2:
			m_ay1->data_w(data);
			break;
		case 3:
			m_ay2->data_w(data);
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
			m_ay3->address_w(m_ay3_latch);
			break;
		case 0x43:
			//printf("%02x data\n",m_ay3_latch);
			m_ay3->data_w(m_ay3_latch);
			break;
	}
}
