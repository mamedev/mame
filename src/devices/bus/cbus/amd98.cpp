// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

System Sacom AMD-98 (AmuseMent boarD)

3 PSG chips, one of the first sound boards released for PC98
Superseded by later NEC in-house sound boards

TODO:
- not sure if it's AY8910 or YM2203, from a PCB pic it looks with stock AY logos?
- f/f not completely understood;
- PIT control;
- PCM section;

===================================================================================================

- Known games with AMD-98 support
    Brown's Run (System Sacom)
    Dome (System Sacom)
    Highway Star (System Sacom)
    Marchen Veil I (System Sacom)
    Marchen Veil II (System Sacom)
    Zone (System Sacom)
    Relics (Bothtec)
    Thexder (Game Arts)

**************************************************************************************************/

#include "emu.h"
#include "bus/cbus/amd98.h"
#include "speaker.h"

#define LOG_LATCH   (1U << 1)   // Detailed AY3 latch setups


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGLATCH(...)     LOGMASKED(LOG_LATCH, __VA_ARGS__)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AMD98, amd98_device, "amd98", "System Sacom AMD-98")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void amd98_device::device_add_mconfig(machine_config &config)
{
	// Assume mono, as per highway making engine noise from ay1 only
	SPEAKER(config, "speaker").front_center();
	AY8910(config, m_ay1, 1'996'800);
	m_ay1->port_a_read_callback().set_ioport("OPN_PA1");
	m_ay1->port_b_write_callback().set(FUNC(amd98_device::ay3_address_w));
	m_ay1->add_route(ALL_OUTPUTS, "speaker", 0.50);

	AY8910(config, m_ay2, 1'996'800);
	m_ay2->port_a_read_callback().set_ioport("OPN_PA2");
	m_ay2->port_b_write_callback().set(FUNC(amd98_device::ay3_data_latch_w));
	m_ay2->add_route(ALL_OUTPUTS, "speaker", 0.50);

	AY8910(config, m_ay3, 1'996'800);
	m_ay3->port_b_write_callback().set([this] (u8 data) {
		LOG("AMD-98 DAC %02x\n", data);
	});
	m_ay3->add_route(ALL_OUTPUTS, "speaker", 0.25);
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

ioport_constructor amd98_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_amd98 );
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  amd98_device - constructor
//-------------------------------------------------

amd98_device::amd98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AMD98, tag, owner, clock)
	, m_bus(*this, DEVICE_SELF_OWNER)
	, m_ay1(*this, "ay1")
	, m_ay2(*this, "ay2")
	, m_ay3(*this, "ay3")
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void amd98_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void amd98_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void amd98_device::device_reset()
{
	m_bus->install_io(0x00d8, 0x00df, read8sm_delegate(*this, FUNC(amd98_device::read)), write8sm_delegate(*this, FUNC(amd98_device::write)));
	// thexder access with following
	m_bus->install_io(0x38d8, 0x38df, read8sm_delegate(*this, FUNC(amd98_device::read)), write8sm_delegate(*this, FUNC(amd98_device::write)));
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t amd98_device::read(offs_t offset)
{
	switch(offset)
	{
		case 2:
			return m_ay1->data_r();
		case 3:
			return m_ay2->data_r();
	}

	LOG("AMD-98: unhandled %02x read\n", offset + 0xd8);

	return 0xff;
}

void amd98_device::write(offs_t offset, uint8_t data)
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
			LOG("AMD-98: unhandled %02x write %02x\n", offset + 0xd8, data);
	}
}

void amd98_device::ay3_address_w(uint8_t data)
{
	LOGLATCH("AMD-98 AY3 latch %02x\n", data);
	m_ay3_latch = data;
}

void amd98_device::ay3_data_latch_w(uint8_t data)
{
	// TODO: actually goes 0 -> 1 -> 0
	// TODO: thexder is the odd one: uses 0x00 -> 0x40 -> 0x47 (address) -> 0x40 -> 0x40 -> 0x43 (data) -> 0x40
	if (!BIT(m_ay3_ff, 0) && BIT(data, 0))
	{
		switch(data & 0xc2)
		{
			case 0x42:
				LOG("AMD-98 AY3 write %02x address (f/f %02x)\n", m_ay3_latch, m_ay3_ff);
				m_ay3->address_w(m_ay3_latch);
				break;
			case 0x40:
				LOG("AMD-98 AY3 write %02x data (f/f %02x)\n", m_ay3_latch, m_ay3_ff);
				m_ay3->data_w(m_ay3_latch);
				break;
		}
	}
	LOGLATCH("AMD-98 f/f %02x %02x\n", data, m_ay3_latch);
	m_ay3_ff = data;
}
