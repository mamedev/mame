// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

 Skeleton driver for Fun Industries' "Fun-E-Ball" electromechanical machine.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "speaker.h"

namespace
{

class funeball_state : public driver_device
{
public:
	funeball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void funeball(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
};

void funeball_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

INPUT_PORTS_START(funeball)
INPUT_PORTS_END

void funeball_state::funeball(machine_config &config)
{
	P80C562(config, m_maincpu, 8_MHz_XTAL); // Philips P80C562EBA
	m_maincpu->set_addrmap(AS_PROGRAM, &funeball_state::prog_map);

	SPEAKER(config, "mono").front_center();
}

// PCB SC1781
ROM_START(funeball)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("feb.u5", 0x0000, 0x8000, CRC(abfcc5f2) SHA1(5b9578447c31decfb9a40d98e013460f1b948adb))

	// The PCB has room for two ISD1420 (U8 and U13), but only one was present (U8) on this machine
	ROM_REGION(0x00100, "samples", 0)
	ROM_LOAD("fun-e-ball_sound_1420_rev_b_12-14-97.u8", 0x00000, 0x00100, NO_DUMP) // ISD1420, internal ROM size unknown

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD("93c46.u15", 0x00, 0x80, CRC(01ec6e74) SHA1(12e774b25af57a0fbceecc537af087ead09d6dd4))
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                FULLNAME      FLAGS
GAME( 1997, funeball, 0,     funeball, funeball, funeball_state, empty_init, ROT0, "Fun Industries Inc.", "Fun-E-Ball", MACHINE_IS_SKELETON_MECHANICAL )
