// license:BSD-3-Clause
// copyright-holders:

/*
    Baby Suprem (c) 198? Andra / Vifico

    Slot machine.

    Main components:
     1 x Z80
     2 x AY-3-8910
     1 x 5517 RAM
     1 x 2.4576MHz Osc
     1 x 8-dip + 1 x 4 dip banks

    There's a complete manual with schematics at https://www.recreativas.org/manuales
    Note there are a number of errors in the manual/schematic.

    There is a basic layout file which makes the game playable.

    TODO: Requires a proper layout file with artwork.
          Add watchdog and determine timeout.
          There are a six lamps whose use needs identifying.
          Identify what's being accessed in the i/o map, this doesn't appear to
          affect the game at all.
          The manual lists all chips used and includes a Z80 DART and Z80 CTC neither
          of which appear in the schematic.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"

#include "speaker.h"

#include "babysuprem.lh"


namespace {

class bsuprem_state : public driver_device
{
public:
	bsuprem_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_psg(*this, "psg%u", 1U)
		, m_dsw1_switches2(*this, "DSW1_SWITCHES2")
		, m_dsw2(*this, "DSW2")
		, m_u13_switches(*this, "U13_SWITCHES")
		, m_test(*this, "TEST")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_buttons(*this, "button%u", 0U)
		, m_nixie(*this, "nixie%u", 0U)
		, m_hopper_5(*this, "hopper_5")
		, m_hopper_25(*this, "hopper_25")
		, m_hopper_100(*this, "hopper_100")
	{
	}

	void bsuprem(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(test_pressed);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_psg;
	required_ioport m_dsw1_switches2;
	required_ioport m_dsw2;
	required_ioport m_u13_switches;
	required_ioport m_test;
	output_finder<6> m_digits;
	output_finder<45> m_lamps;
	output_finder<5> m_buttons;
	output_finder<4> m_nixie;
	required_device<ticket_dispenser_device> m_hopper_5;
	required_device<ticket_dispenser_device> m_hopper_25;
	required_device<ticket_dispenser_device> m_hopper_100;

	uint8_t m_irq_state;
	uint8_t m_u1_porta_data;
	uint8_t m_u10_data;

	void ay8910_u1_porta_w(uint8_t data);
	void ay8910_u1_portb_w(uint8_t data);
	uint8_t ay8910_w_dsw1_switches2_r(offs_t offset);
	void ay8910_w(offs_t offset, uint8_t data);

	uint8_t dsw2_u13_switches_r(offs_t offset);
	void u9_w(offs_t offset, uint8_t data);
	void u10_w(offs_t offset, uint8_t data);

	void bsuprem_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(mains_irq);

};

static INPUT_PORTS_START( bsuprem )
	PORT_START("DSW1_SWITCHES2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3) PORT_NAME("Right stop")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2) PORT_NAME("Middle stop")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1) PORT_NAME("Left stop")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x80, 0x80, "SW1-3 (Test 4)") PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPNAME(0x40, 0x40, "SW1-1 (Test 2)") PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPNAME(0x20, 0x20, "SW1-2 (Test 3)") PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPNAME(0x10, 0x10, "SW1-4 (Test 5)") PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x10, DEF_STR( Off ))

	PORT_START("DSW2")
	PORT_DIPNAME(0x80, 0x80, "SW2-2 (Door)")        PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPNAME(0x40, 0x40, "SW2-4")               PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPNAME(0x20, 0x20, "SW2-6 Auto start")    PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(   0x20, DEF_STR( No ))
	PORT_DIPNAME(0x10, 0x10, "SW2-8 Percent")       PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(   0x00, "80%")
	PORT_DIPSETTING(   0x10, "85%")
	PORT_DIPNAME(0x08, 0x08, "SW2-7")               PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x08, DEF_STR( Off ))
	PORT_DIPNAME(0x04, 0x04, "SW2-5 Test")          PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x04, DEF_STR( Off ))
	PORT_DIPNAME(0x02, 0x02, "SW2-3 Instant start") PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(   0x02, DEF_STR( No ))
	PORT_DIPNAME(0x01, 0x01, "SW2-1 (Bet/Collect)") PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))

	PORT_START("U13_SWITCHES")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET)  PORT_NAME("Bet/Collect")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR)

	PORT_START("TEST")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_CHANGED_MEMBER(DEVICE_SELF, bsuprem_state, test_pressed, 0)

	PORT_START("SWITCHES")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper_25", ticket_dispenser_device, line_r)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper_100", ticket_dispenser_device, line_r)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper_5", ticket_dispenser_device, line_r)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

void bsuprem_state::bsuprem(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 2.4576_MHz_XTAL); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &bsuprem_state::bsuprem_map);
	m_maincpu->set_periodic_int(FUNC(bsuprem_state::mains_irq), attotime::from_hz(100));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper_5, attotime::from_msec(50));
	HOPPER(config, m_hopper_25, attotime::from_msec(50));
	HOPPER(config, m_hopper_100, attotime::from_msec(50));

	// Sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_psg[0], 2.4576_MHz_XTAL); // divider not verified
	m_psg[0]->port_a_write_callback().set(FUNC(bsuprem_state::ay8910_u1_porta_w));
	m_psg[0]->port_b_write_callback().set(FUNC(bsuprem_state::ay8910_u1_portb_w));
	m_psg[0]->add_route(ALL_OUTPUTS, "mono", 0.30);

	AY8910(config, m_psg[1], 2.4576_MHz_XTAL); // divider not verified
	m_psg[1]->add_route(ALL_OUTPUTS, "mono", 0.30);
}

void bsuprem_state::bsuprem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1fff).rw(FUNC(bsuprem_state::ay8910_w_dsw1_switches2_r), FUNC(bsuprem_state::ay8910_w));
	map(0x2000, 0x2fff).portr("SWITCHES");
	map(0x3000, 0x3fff).r(FUNC(bsuprem_state::dsw2_u13_switches_r));
	map(0x4000, 0x4fff).rom().region("maincpu", 0x2000);
	map(0x5000, 0x5fff).w(FUNC(bsuprem_state::u9_w));
	map(0x6000, 0x6fff).w(FUNC(bsuprem_state::u10_w));
	map(0x7000, 0x77ff).ram().mirror(0x0800).share("nvram");

	map(0x8000, 0x8fff).rom().region("maincpu", 0x1000);
	map(0xc000, 0xcfff).rom().region("maincpu", 0x3000);
}

void bsuprem_state::machine_reset()
{
	m_irq_state = 0;
	m_u1_porta_data = 0;
	m_u10_data = 0;
}

void bsuprem_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();
	m_buttons.resolve();
	m_nixie.resolve();

	save_item(NAME(m_irq_state));
	save_item(NAME(m_u1_porta_data));
	save_item(NAME(m_u10_data));
}

void bsuprem_state::ay8910_w(offs_t offset, uint8_t data)
{
	// a10 -> ay8910_1 A8, ay8910_2 !A9
	// a9 ->  ay8910_1 !A9, ay8910_2 A8
	// a8 ->  ay8910 BC2
	if (BIT(offset, 10) != BIT(offset, 9))
	{
		if (BIT(offset, 8))
		{
			m_psg[BIT(offset, 9)]->data_w(offset & 0xff);
		}
		else
		{
			m_psg[BIT(offset, 9)]->address_w(offset & 0xff);
		}
	}
}

uint8_t bsuprem_state::ay8910_w_dsw1_switches2_r(offs_t offset)
{
	ay8910_w(offset, 0);

	return m_dsw1_switches2->read();
}

uint8_t bsuprem_state::dsw2_u13_switches_r(offs_t offset)
{
	return m_dsw2->read() & m_u13_switches->read();
}

void bsuprem_state::ay8910_u1_porta_w(uint8_t data)
{
	m_u1_porta_data = data;
}

void bsuprem_state::ay8910_u1_portb_w(uint8_t data)
{
	constexpr uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511

	data = (data>>4) & 0x0f;

	switch (data)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
			for (int i = 0; i < 8 ; i++)
				m_lamps[(data * 8) + i] = BIT(m_u1_porta_data, i);
			break;

		case 0x05:
			for (int i = 0; i < 5 ; i++)
				m_lamps[40 + i] = BIT(m_u1_porta_data, i);
			break;

		case 0x08:
			m_nixie[0] = m_u1_porta_data & 0x07;
			m_nixie[1] = (m_u1_porta_data >> 4) & 0x07;
			break;

		case 0x09:
			m_nixie[2] = (m_u1_porta_data >> 3) & 0x07;
			m_nixie[3] = m_u1_porta_data & 0x07;
			break;

		case 0x0a:
		case 0x0b:
		case 0x0c:
			m_digits[(0x0c - data) * 2] = patterns[(m_u1_porta_data >> 4) & 0x0f];
			m_digits[((0x0c - data) * 2) + 1] = patterns[m_u1_porta_data & 0x0f];
			break;
	}
}

void bsuprem_state::u9_w(offs_t offset, uint8_t data)
{
	// bit 0 = stop button 3 lamp
	// bit 1 = stop button 2 lamp
	// bit 2 = stop button 1 lamp
	// bit 3 = coin validator control
	// bit 4 = start/bet button lamp
	// bit 5 = coin validator control
	// bit 6 = hopper 5
	// bit 7 = hopper 25
	m_buttons[0] = BIT(data, 4);
	m_buttons[1] = BIT(data, 2);
	m_buttons[2] = BIT(data, 1);
	m_buttons[3] = BIT(data, 0);
	m_buttons[4] = BIT(data, 4);

	m_hopper_5->motor_w(BIT(data, 6));
	m_hopper_25->motor_w(BIT(data, 7));
}


void bsuprem_state::u10_w(offs_t offset, uint8_t data)
{
	// bit 0 = coin validator control
	// bit 1 = 25pts meter
	// bit 2 = unused meter ?
	// bit 3 = 5pts meter
	// bit 4 = coin validator control
	// bit 5 = watchdog
	// bit 6 = disable NVRAM - This is from the schematic which appears to be wrong.
	// bit 7 = hopper 100
	m_u10_data = data;

	m_hopper_100->motor_w(BIT(data, 7));
}

INTERRUPT_GEN_MEMBER(bsuprem_state::mains_irq)
{
	m_irq_state = !m_irq_state;

	m_maincpu->set_input_line(0, m_irq_state ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(bsuprem_state::test_pressed)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

ROM_START( bsuprem )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "andra_baby_suprem.u4", 0x0000, 0x4000, CRC(71e4ae62) SHA1(3aa4e7125ea03464c58187eb85fb706b5392b9b5) )
ROM_END

} // Anonymous namespace


//     YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY           FULLNAME       FLAGS
GAMEL( 198?, bsuprem, 0,      bsuprem, bsuprem, bsuprem_state, empty_init, ROT0, "Andra / Vifico", "Baby Suprem", MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_babysuprem )
