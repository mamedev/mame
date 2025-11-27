// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for NSM/Löwen ST25 platform of gambling machines
Infos can be found at https://wiki.goldserie.de/index.php?title=Spiel_und_System_Modul_25

  NSM STE25.1 216575A/1012
  ___________________________________________________________________________________
 |                 __________                             XTAL       SERIAL         |
 | .              |TL7705ACP|           ______________  16.000 MHz                  |
 | .               __________          | NEC V25     |                ___________   |
 | .              |74HCT574 |          | D70322L-8   |                |74HC123N |   |
 | .               __________          |             |                ____________  |
 | .              |74HCT245N|          |             |                |V62C518256|  |
 | .                                   |_____________|                              |
 | .           ___________               __________   __________   _____________    |
 | .          |74HC32N   |              |74HC138N |  |74HC04N  |  | Spiel und   |   |
 | .           ___________   __________  __________   __________  | System      |   |
 |     RST    |74HC08N   |  |74HC368B1| |74AS138N |  |74HCT21N |  | Modul       |   |
 | .           ___________               __________   __________  | ROM Module  |   |
 | .          |74HC00N   |              |74HC4050N|  |74HC04N  |  | [SCC2592AC] |   |
 | .           ___________                                        |             |   |
 | .          |74HC32N   |                                XTAL    | [M27C4001 ] |   |
 | .                                                    3.686MHz  |_____________|   |
 |             +SERVICE+                                           __________       |
 |                                                                | OKI     |       |
 |                                                                | M6376   |       |
 |                                 TDA2005                        |__________       |
 | .                 VOL         __HEATSINK__                                       |
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

#include "machine/mc68681.h"
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

	SCN2681(config, "uart", 3.6864_MHz_XTAL); // Philips SCC2692AC1N28

	// Sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6376(config, "oki", 4_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // Divider not verified
}

ROM_START(alpha)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("w27e40.ic2", 0x00000, 0x80000, CRC(3cba9ebe) SHA1(f49a00e0d6f6e34e7fa24bc4339e51c6834bba67))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("w27e40.ic1",   0x00000, 0x80000, CRC(f893b557) SHA1(194135c0cbcb270ebeb297c2f2e26e6101b44daf))
ROM_END

ROM_START(amarillo)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

    ROM_REGION(0x80000, "soundrom", 0) //????????
    ROM_LOAD("27c4001_snd.bin", 0x00000, 0x80000, CRC(2ccf9464) SHA1(02b16fe7465ad28ce96f38390fafbceafca2d23c))

    ROM_REGION(0x80000, "oki", 0)
    ROM_LOAD("27c040_ic1.bin", 0x00000, 0x80000, CRC(2114485c) SHA1(ee0bb436367e87bacfe703d0a8ee98c5362e0014))

    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD("27c040_ic2.bin", 0x00000, 0x80000, CRC(b5058562) SHA1(c96ca309ca8214dcaeeef41ac29e8c325c08a9d9))
ROM_END

ROM_START(arenau)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

    ROM_REGION(0x80000, "oki", 0)
    ROM_LOAD("ic1_27c4001", 0x00000, 0x80000, CRC(93cdf476) SHA1(5b80e76bd04056ff53c7e11cfc5364cea30e4aed))

    ROM_REGION(0x40000, "maincpu", 0)
    ROM_LOAD("ic2_27c2001", 0x00000, 0x40000, CRC(2348e6c3) SHA1(7708a2ffc3b5154bd1793fb7332e26125cdc9696))
ROM_END

ROM_START(avanti)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

    ROM_REGION(0x80000, "oki", 0)
    ROM_LOAD("avantie_w27e040_st1.bin", 0x00000, 0x80000, CRC(47defe53) SHA1(65c246e9051fa1b0f9a855331d55282c6b4ccbc0))

    ROM_REGION(0x80000, "maincpu", 0)
    ROM_LOAD("avantie_w27e040_st2.bin", 0x00000, 0x80000, CRC(14278f3a) SHA1(82a8a5e35e0eee8f4dbcb0e7b6491528c6444fad))
ROM_END

ROM_START(ballermann)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

    ROM_REGION(0x40000, "maincpu", 0)
    ROM_LOAD("27c2001_ic2", 0x00000, 0x40000, CRC(a20915f1) SHA1(cd7e1339bc635a8e16381858b93fe28f04fa725d))

    ROM_REGION(0x80000, "oki", 0)
    ROM_LOAD("27c4001_ic1", 0x00000, 0x80000, CRC(1dd6fee1) SHA1(bdd0e478069f822d2b940aa449ec61e9f07d4b3b))
ROM_END

ROM_START(bigactione)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("27c020a.ic2", 0x00000, 0x80000, CRC(177e3fee) SHA1(a4ca38dfdf79eb3524381ea3b6fa7700ad24a966))

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("m48t18.ic3", 0x00000, 0x2000, CRC(5b1e8172) SHA1(3ee9dfcba8fea095b6e003ba20bbc57fbeb5359e))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1",   0x00000, 0x80000, CRC(8c708e53) SHA1(ef91a5a21ba69ad2870f7201bb4d90b4bc94c4ec))
ROM_END

ROM_START(stakeoffe)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("27c020a.ic2", 0x00000, 0x40000, CRC(b1553dc1) SHA1(d04d1e0d7cf553588d6abf2f5c95e0d8a761f8b6))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("27c040.ic1",   0x00000, 0x80000, CRC(d9592e5e) SHA1(5de917a1c584a39a85e6f356d25924a65eaddf89))
ROM_END

ROM_START(superpasch)
	ROM_REGION(0x4000, "maskrom", 0)
	ROM_LOAD("d70322.icc2", 0x0000, 0x4000, CRC(a3be4fee) SHA1(3e19009d90f71ab21d927cdd31dc60dda652e045))

	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("124253.ic2", 0x00000, 0x80000, CRC(fe23b37a) SHA1(9d461b01d05c6e71e3d32800a429ad3f733d7274))

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("124254.ic1",   0x00000, 0x80000, CRC(f893b557) SHA1(194135c0cbcb270ebeb297c2f2e26e6101b44daf))
ROM_END


} // anonymous namespace


//   YEAR  NAME      PARENT   MACHINE INPUT    CLASS   INIT        ROT   COMPANY         FULLNAME                                                  FLAGS
GAME(1997, ballermann, 0,       st25, st25, st25_state, empty_init, ROT0, "Panther", "Ballermann 6",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(1998, arenau,     0,       st25, st25, st25_state, empty_init, ROT0, u8"Löwen", "Unimint Arena",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2000, bigactione, 0,       st25, st25, st25_state, empty_init, ROT0, "Panther", "Big Action 3000 E", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2001, stakeoffe,  0,       st25, st25, st25_state, empty_init, ROT0, "Panther", "Super Take Off E",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2004, avanti,     0,       st25, st25, st25_state, empty_init, ROT0, u8"Löwen", "Avanti",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2005, alpha,      0,       st25, st25, st25_state, empty_init, ROT0, u8"Löwen", "Alpha",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2005, superpasch, 0,       st25, st25, st25_state, empty_init, ROT0, u8"Löwen", "Super Pasch",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)
GAME(2007, amarillo,   0,       st25, st25, st25_state, empty_init, ROT0, u8"Löwen", "Amarillo",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK)

