// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

System Sacom AMD-98 (AmuseMent boarD)

3 PSG chips, one of the first sound boards released for PC98
Superseded by later NEC in-house sound boards
Incompatible with 286+ machines (that uses $f0 for machine status read)

TODO:
- not sure if it's AY8910 or YM2149, from a PCB pic it looks with stock AY logos?
- f/f not completely understood;
- PCM section;
- mixing routing on itself and against -26 (thexder);
- convert joystick to MSX_GENERAL_PURPOSE_PORTs;

===================================================================================================

- Known games with AMD-98 support
  brownrun
  dome
  highway
  marchen1
  marchen2
  zone
  relics
  thexder

**************************************************************************************************/

#include "emu.h"

#include "amd98.h"
#include "speaker.h"

#define LOG_LATCH   (1U << 1)   // Detailed AY3 latch setups


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGLATCH(...)     LOGMASKED(LOG_LATCH, __VA_ARGS__)

DEFINE_DEVICE_TYPE(AMD98, amd98_device, "amd98", "System Sacom AMD-98 sound card")

amd98_device::amd98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AMD98, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_ay1(*this, "ay1")
	, m_ay2(*this, "ay2")
	, m_ay3(*this, "ay3")
	, m_pit(*this, "pit")
{
}


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

	// dome/marchen1~2 needs this
	PIT8253(config, m_pit);
	m_pit->set_clk<2>(1'996'800);
	m_pit->out_handler<2>().set([this] (int state) {
		// requires inverted polarity
		m_bus->int_w(6, !state);
	});
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
}

void amd98_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x00ff, *this, &amd98_device::io_map);
		// thexder access with following
		m_bus->install_device(0x3800, 0x38ff, *this, &amd98_device::io_map);
	}
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void amd98_device::io_map(address_map &map)
{
	// TODO: configurable PnP, needs dip/jumper sheet
	map(0xf0, 0xf0).lr8(NAME([] () { return 0x18; }));

	map(0xd8, 0xd8).w(m_ay1, FUNC(ay8910_device::address_w));
	map(0xd9, 0xd9).w(m_ay2, FUNC(ay8910_device::address_w));
	map(0xda, 0xda).rw(m_ay1, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xdb, 0xdb).rw(m_ay2, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xdc, 0xdc).lrw8(
		NAME([this] (offs_t offset) { return m_pit->read(2); }),
		NAME([this] (offs_t offset, u8 data) { m_pit->write(2, data); })
	);
	map(0xde, 0xde).lrw8(
		NAME([this] (offs_t offset) { return m_pit->read(3); }),
		NAME([this] (offs_t offset, u8 data) { m_pit->write(3, data); })
	);
}

void amd98_device::ay3_address_w(u8 data)
{
	LOGLATCH("AMD-98 AY3 latch %02x\n", data);
	m_ay3_latch = data;
}

void amd98_device::ay3_data_latch_w(u8 data)
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
