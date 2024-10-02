// license:BSD-3-Clause
// copyright-holders:

/*
  Skeleton driver for Inder "Bullion 2" slot machine.
  The hardware for Bullion 3 (undumped) is almost the same, the main difference is that Bullion 2 has three reels,
  and Bullion 3 has four.

  Reels sequence for Bullion 2:

      +--------+--------+--------+
      |  BAR   |  BELL  | GRAPE  | <- Default after reset
      +--------+--------+--------+
      | MELON  | CHERRY |  PLUM  |
      +--------+--------+--------+
      | GRAPE  | CHERRY | MELON  |
      +--------+--------+--------+
      |  BELL  | BANANA | LEMON  |
      +--------+--------+--------+
      |  PLUM  | GRAPE  | CHERRY |
      +--------+--------+--------+
      | BANANA |  BAR   |  BELL  |
      +--------+--------+--------+
      | LEMON  | LEMON  |  BAR   |
      +--------+--------+--------+
      | CHERRY |  PLUM  | BANANA |
      +--------+--------+--------+

  Dip switches for Bullion 2:

      DIP1 -> |ON |HOPPER| default ON
      DIP2 |  |OFF|PULSES|
      DIP3 |
      DIP4 |     |ON |DEMO SOUND   | default ON
      DIP5 |  /->|OFF|NO DEMO SOUND|
      DIP6 --/
      DIP7 --> |OFF    |OFF    |ON     |ON     | default ON
      DIP8 --> |OFF    |ON     |OFF    |ON     | default ON
               |LONGER |LONG   |SHORT  |SHORTER|
               |      REELS SPINNING TIME      |

  Dip switches 2, 3, 4, 5 unused (default ON).

  Also, you can change the plug on the percentages PCB to change the payments:
      Plug A = 78%
      Plug B = 80% default
      Plug C = 82%
      Plug D = 84%

  Complete manual (for both Bullion 2 and Bullion 3) with schematics and dip switches can be downloaded from:
      https://www.recreativas.org/manuales/tragaperras
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "speaker.h"


namespace {

class bullion2_state : public driver_device
{
public:
	bullion2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "sn76489")
	{ }

	void bullion2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sn76489_device> m_sn;

	void program_map(address_map &map);
};

void bullion2_state::machine_start()
{
}

void bullion2_state::machine_reset()
{
}

void bullion2_state::program_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x43ff).ram();
	// map(0x4804, 0x4804).rw // ??
	// map(0x4900, 0x4907).w // latch for lamps and reels control?
	// map(0x4b00, 0x4b00).w("sn76489", FUNC(sn76489_device::write));  // SN?
}

static INPUT_PORTS_START( bullion2 )
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")
INPUT_PORTS_END

void bullion2_state::bullion2(machine_config &config)
{
	// CPU PCB Inder "60-977"

	Z80(config, m_maincpu, 4_MHz_XTAL); // NEC D780C
	m_maincpu->set_addrmap(AS_PROGRAM, &bullion2_state::program_map);
	m_maincpu->set_periodic_int(FUNC(bullion2_state::irq0_line_hold), attotime::from_hz(60*4)); // TODO: proper IRQs

	// Sound PCB Inder "60-083"

	SPEAKER(config, "mono").front_center();

	SN76489(config, m_sn, 4_MHz_XTAL);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( bullion2 )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "inder_sa_m-17_bullion_2_rom_0_020584.ci19", 0x0000, 0x1000, CRC(fb13407c) SHA1(16a3cdcbe87f17c4bb6a78a8ac47f58d25a4483f) )
	ROM_LOAD( "inder_sa_m-17_bullion_2_rom_1_020584.ci20", 0x1000, 0x1000, CRC(4857aa2d) SHA1(4f80fde62ce0d0f85df009ac5c1021263049b95a) )
	ROM_LOAD( "inder_sa_m-17_bullion_2_rom_2_020584.ci21", 0x2000, 0x1000, CRC(2f607456) SHA1(a52ad5e8aeeb7e67ff07569377f19de46ee185ea) )
ROM_END


} // anonymous namespace


//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY  FULLNAME     FLAGS
GAME( 1984, bullion2, 0,      bullion2, bullion2, bullion2_state, empty_init, ROT0,     "Inder", "Bullion 2", MACHINE_IS_SKELETON )
