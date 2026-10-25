// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)
/***************************************************************************

Driver for ICE Cyclone, a redemption game for three players.
A light runs around a ring of 84 bulbs and each player tries to stop it on
the jackpot, between the two neon arches of their section.

Main PCB (CC2034X):
- MC68HC11A1P (U2). Its EEPROM holds the programming options, and its RAM
  is kept by a 3V battery (BT1) on MODB/VSTBY.
- 27C040 EPROM (U4) with the sound samples and, in its last 16 KB, the
  program.
- Two 74HC165 (U11, U14) for the coin, unlock and ticket dispenser inputs.
- Two 74HC273 (U13, U15) for the ticket dispensers, the coin and ticket
  counters and the button lamps.
- 74HC273 (U12) driving an R-2R ladder for sound, with the volume set from
  PA3-PA6 through 74HC4066 switches.
- 74HC237 (U23) routing the SPI clock to the light ring (Y0), the stations
  (Y1-Y3) and the neon board (Y4).

Each station has a 4-digit tickets owed display chained to a 4-digit
jackpot display, each one made of four 74HC164 driving seven-segment digits.

TODO:
- Displays, light ring and neon board: the MC68HC11 SPI isn't emulated.
- Sound.
- Ticket dispensers.
- Battery-backed internal RAM (jackpot memory option).

***************************************************************************/

#include "emu.h"

#include "cpu/mc68hc11/mc68hc11.h"


namespace {

class cyclone_state : public driver_device
{
public:
	cyclone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_inputs(*this, "IN")
		, m_buttons(*this, "PA")
		, m_button_lamps(*this, "button_lamp%u", 1U)
	{
	}

	void cyclone(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	void bank_w(u8 data);
	void load_w(u8 data);
	void clock_w(u8 data);
	void out1_w(u8 data);
	void out2_w(u8 data);
	u8 pa_r();

	required_device<mc68hc11_cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_ioport m_inputs;
	required_ioport m_buttons;
	output_finder<3> m_button_lamps;

	u16 m_shift = 0;
};

void cyclone_state::machine_start()
{
	m_rombank->configure_entries(0, 32, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_shift));
}

void cyclone_state::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x1f);
}

void cyclone_state::load_w(u8 data)
{
	m_shift = m_inputs->read();
}

void cyclone_state::clock_w(u8 data)
{
	m_shift = (m_shift << 1) | 1;
}

void cyclone_state::out1_w(u8 data)
{
	// bits 0, 1 and 6 run the ticket dispensers
	m_button_lamps[0] = BIT(data, 3);
	m_button_lamps[1] = BIT(data, 2);
}

void cyclone_state::out2_w(u8 data)
{
	m_button_lamps[2] = BIT(data, 5);
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));

	// ticket counters
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(3, BIT(data, 0));
}

u8 cyclone_state::pa_r()
{
	return m_buttons->read() | (BIT(m_shift, 15) << 7);
}

void cyclone_state::mem_map(address_map &map)
{
	map(0x4000, 0x7fff).bankr(m_rombank).w(FUNC(cyclone_state::bank_w));
	map(0x8000, 0xbfff).nopw(); // U12
	map(0xc000, 0xffff).rom().region("maincpu", 0x7c000);
	map(0xc800, 0xcfff).w(FUNC(cyclone_state::load_w));
	map(0xd000, 0xd7ff).w(FUNC(cyclone_state::clock_w));
	map(0xd800, 0xdfff).w(FUNC(cyclone_state::out1_w));
	map(0xe000, 0xe7ff).w(FUNC(cyclone_state::out2_w));
	map(0xe800, 0xefff).nopw(); // U23 address latch
}

INPUT_PORTS_START(cyclone)
	PORT_START("IN") // U11 A-H in bits 0-7, U14 A-H in bits 8-15
	PORT_BIT(0x0003, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_SERVICE4) PORT_NAME("Quick Start") // the schematic shows it on U11 A
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1 Unlock")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN) // ticket dispenser 1 sensor
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN) // ticket dispenser 2 sensor
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN) // SPARE1
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2 Unlock")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P3 Unlock")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN) // ticket dispenser 3 sensor
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN) // SPARE3
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN) // SPARE2

	PORT_START("PA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(3)

	PORT_START("PE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Programming")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME("Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE3) PORT_NAME("Step")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME("Self Test")
INPUT_PORTS_END

void cyclone_state::cyclone(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cyclone_state::mem_map);
	m_maincpu->in_pa_callback().set(FUNC(cyclone_state::pa_r));
	m_maincpu->in_pe_callback().set_ioport("PE");
}

ROM_START(cyclone)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("cyclone_v3.22.u4", 0x00000, 0x80000, CRC(ab1bf390) SHA1(835705d57828c3edd6c6a0f53d1820f1cce3111e))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY  FULLNAME   FLAGS
GAME( 1995, cyclone, 0,      cyclone, cyclone, cyclone_state, empty_init, ROT0, "ICE",   "Cyclone", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
