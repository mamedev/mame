// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for NSM/Löwen ST25 platform of gambling machines
Infos can be found at https://wiki.goldserie.de/index.php?title=Spiel_und_System_Modul_25

 NSM STE25.1 216575A
  ___________________________________________________________________________________
 |                                                        XTAL       SERIAL         |
 | .            TL7705ACP               ______________  16.000 MHz                  |
 | .                                   | NEC V25     |                ___________   |
 | .                                   | D70322L-8   |                |74HC123N |   |
 | .                                   |             |                ___________   |
 | .                                   |             |                |D43256B  |   |
 | .                                   |_____________|                              |
 | .           ___________               __________   __________   _____________    |
 | .          |74HC32N   |              |         |  |         |  | Spiel und   |   |
 | .           ___________   __________  __________   __________  | System      |   |
 |     RST    |74HC08N   |  |74HC368B1| |         |  |74HCT21N |  | Modul       |   |
 | .           ___________               __________   __________  | ROM Module  |   |
 | .          |74HC00N                  |74HC4050N|  |         |  |             |   |
 | .                                                              |             |   |
 | .                                                              |             |   |
 | .                                                              |_____________|   |
 |              SERVICE                                            __________       |
 |                                                                | OKI     |       |
 |                                                                | M6376   |       |
 |                                                                |__________       |
 | .                 VOL                                                            |
 | .                                                                                |
 |__________________________________________________________________________________|

 Rom Module
  ______________
 | [CONNECTOR]  |
 |              |
 |  TMS27C020   | // Program ROM IC2
 |              |
 |M48T18-150PC1 | // Timekeeper RAM IC3
 |              |
 |  TMS27C020   | // Sound ROM IC1
 |              |
 | [CONNECTOR]  |
 _______________

*/

#include "emu.h"

#include "cpu/nec/v25.h"
#include "machine/timekpr.h"
#include "sound/okim6376.h"

#include "speaker.h"


namespace {

class st25_state : public driver_device
{
public:
	st25_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void st25(machine_config &config) ATTR_COLD;

private:
	required_device<v25_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};


void st25_state::program_map(address_map &map)
{
	//map(0x00000, 0x3ffff).ram();
	//map(0x40000, 0x7ffff).rom().region("maincpu", 0);
    map(0xfc000, 0xfffff).rom().region("maskrom", 0);
}

void st25_state::io_map(address_map &map)
{
	// map(0x8000, 0x8000).w();
}

void st25_state::data_map(address_map &map)
{
	map(0x100, 0x1ff).ram();
}


static INPUT_PORTS_START(st25)
	PORT_START("IN0")

INPUT_PORTS_END


void st25_state::st25(machine_config &config)
{
	// Basic machine hardware

	V25(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &st25_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &st25_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &st25_state::data_map);
	m_maincpu->pt_in_cb().set([this] () { logerror("%s: pt in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_in_cb().set([this] () { logerror("%s: p0 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p1_in_cb().set([this] () { logerror("%s: p1 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p2_in_cb().set([this] () { logerror("%s: p2 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p0_out_cb().set([this] (uint8_t data) { logerror("%s: p0 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p1_out_cb().set([this] (uint8_t data) { logerror("%s: p1 out %02X\n", machine().describe_context(), data); });
	m_maincpu->p2_out_cb().set([this] (uint8_t data) { logerror("%s: p2 out %02X\n", machine().describe_context(), data); });


	M48T02(config, "m48t18", 0); // ST M48T18-150PC1

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	OKIM6376(config, "oki", 4_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
}

ROM_START(stakeoff)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c020a.ic2", 0x00000, 0x40000, CRC(b1553dc1) SHA1(d04d1e0d7cf553588d6abf2f5c95e0d8a761f8b6))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1",   0x00000, 0x80000, CRC(d9592e5e) SHA1(5de917a1c584a39a85e6f356d25924a65eaddf89))

ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT   MACHINE INPUT    CLASS   INIT        ROT   COMPANY         FULLNAME                                                  FLAGS
GAME(2001, stakeoff,  0,       st25, st25, st25_state, empty_init, ROT0, u8"NSM/Löwen", "Super Take Off E", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
