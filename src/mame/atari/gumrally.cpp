// license:BSD-3-Clause
// copyright-holders:

/*
    Gumball Rally (c) 1990(?) Atari

    Electromechanical redemption game
    Video: https://www.youtube.com/watch?v=Q7NljgN72qE
  _____________________________________________________________________________________________________________________________
 |                                                                                                                             |
 |..                                                                                                     _  o o o              |
 |..                                                                                               LED->(_) o o o              |
 |..                                                          __________                    __________      o o o              |
 |..                                                         |ULN_2003A|                   |_TL084CN_|      o o o              |
 |..                  __________   __________                ___________                                                       |
 |..                 SN74LS257BN  SN74LS257BN               |SN74LS273N|  ____   ____                       o o o              |
 |..                                                                    LM358AN LM358AN                     o o o              |
 |..                 ___________   SWITCH      ____________                                                 o o o              |
 |..                |SN74LS273N|  SELF TEST   |VLSI 9012AV|    __________                                   o o o              |
 |..                                          |___________|   SN74LS166AN                                                      |
 |..   ___________      __________   __________   __________   __________   __________   ______________                        |
 |    GAL16V8A-25LP    |SN74LS138N  |SN74LS138N  SN74LS163AN  SN74LS163AN  |SN74LS393N  | YM2149F     |                        |
 |                    ____________                _                                     |_____________|                        |
 |                   |KM28C16-20 |               (_)<-LED                   __________                                         |
 |..                 |___________|   __________   __________   __________  |_74LS04N_|                                         |
 |..   ____________   ____________  |_74LS08N_|  |_74LS197N|  |74LS163AN|   ______________   _____                             |
 |..  |HY6264LP-85|  |HY6264LP-85|   __________   __________   __________  | EEPROM      |  | OKI | Xtal                       |
 |..  |___________|  |___________|  |SN74LS163AN GAL16V8A-25LP SN74LS74AN  |_____________|  |M6295| 14.318                     |
 |..   ____________   ____________   __________   __________   __________   ______________  |_____|      __________            |
 |..  | EEPROM    |  | EEPROM    |  |_74LS04N_|  SN74LS374AN  |SN74LS86N|  | EEPROM      |              |_7408N___|            |
 |..  |___________|  |___________|   __________   __________   __________  |_____________|  __________   __________            |
 |..   ___________________________  |SN74LS32N|  |SN74LS273N  SN74LS163AN   ______________ |_GD74LS00|  |_7406N___|   ____     |
 |..  | Motorola MC68000P8       |   __________   __________   __________  | EEPROM      |  __________   __________  LM358AN   |
 |..  |                          |  |_74LS20N_|  SN74LS163AN  SN74LS163AN  |_____________| |SN74LS32N|  |SN74LS74AN            |
 |..  |__________________________|                __________   __________   ______________  __________                         |
 |..                                             |SN74LS273N  SN74LS163AN  | EEPROM      | |SN74LS74AN                         |
 |                                                                         |_____________|                                     |
 |_____________________________________________________________________________________________________________________________|

    Main PCB marked Atari Games @ 90 Gumball Rally Main
    Main components:
    1 x MC68000P8
    2 x M27512FI main CPU ROMs
    4 x TMS 27C512 OKI ROMs
    2 x HY6264LP RAMs
    2 x GAL16V8A
    1 x KM28C16 EEPROM
    1 x VLSI VGC7205A0672 ATARI 137304-2002
    1 x YM2149F
    1 x OKI M6295
    1 x 14.318MHz Osc (near sound chips)
    1 x selftest switch
    no dips

    The manual also lists the following PCBs:
    Bar Graph Display PCB with 16 x 7-segment displays
    Score Display PCB with 16 x 7-segment displays
    Display Blanking PCB
    APB Triac PCB
    Steering Wheel PCB

    TODO:
    - everything.
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"

#include "speaker.h"


namespace {

class gumrally_state : public driver_device
{
public:
	gumrally_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void gumrally(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void prg_map(address_map &map) ATTR_COLD;
};


void gumrally_state::prg_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( gumrally )
INPUT_PORTS_END


void gumrally_state::gumrally(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14'318'000); // clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &gumrally_state::prg_map);

	EEPROM_2816(config, "eeprom");

	// video hardware
	// TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2149(config, "ym", 14'318'000 / 12).add_route(ALL_OUTPUTS, "mono", 0.3); // clock not verified

	OKIM6295(config, "oki", 14'318'000 / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // clock and pin not verified
}


ROM_START( gumrally )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gum_4002.c10", 0x00000, 0x10000, CRC(d02a6149) SHA1(80d5242f6469e406523aba07e3b8f08ec01b4275) ) // 1xxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "gum_4008.c8",  0x00001, 0x10000, CRC(1dd18740) SHA1(692c96e979be906d90852bd1fbe85973ddd0c74f) ) // 1xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "061490_1990_atari_136080_1004_cs_52b2.a4", 0x00000, 0x10000, CRC(4a7b5633) SHA1(c08a5557136565317a42e718ec869e8439e117ec) )
	ROM_LOAD( "061490_1990_atari_136080_1005_cs_f7a2.b4", 0x10000, 0x10000, CRC(f0d72b71) SHA1(f92e5ad2255b97e630b3bdbc599f062c84a208f9) )
	ROM_LOAD( "061490_1990_atari_136080_1006_cs_ffda.d4", 0x20000, 0x10000, CRC(e4854f24) SHA1(f1f4ef6b11eb7f4ece8cac8af1076b97517b70de) )
	ROM_LOAD( "061490_1990_atari_136080_1007_cs_b0b4.e4", 0x30000, 0x10000, CRC(1dee18e3) SHA1(aeb1e762d2e6bece68a1135fd36ae24c5acb3a5e) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "36080-1000_gal16v8a.10h", 0x000, 0x117, CRC(1fcb654c) SHA1(17f2b812e8049134ebfc63d1564ccc975eec9f21) )
	ROM_LOAD( "36080-1001_gal16v8a.6e",  0x200, 0x117, CRC(46345b41) SHA1(22864ad5bf99fad514d774dd2934b9cf3249b5c7) )
ROM_END

} // Anonymous namespace


GAME( 1990, gumrally, 0, gumrally, gumrally, gumrally_state, empty_init, ROT0, "Atari Games", "Gumball Rally", MACHINE_IS_SKELETON_MECHANICAL ) // year taken from labels on OKI ROMs
