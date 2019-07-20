// license:BSD-3-Clause
// copyright-holders:

/*
  This hardware seems to be an evolution of the one found in lucky74.cpp. Its main components are:
  * A001 CPU block, containing probably a Z80 or compatible CPU and ROM(s) (not dumped).
    Dumper description after opening it: 3 unknown/white chips, which had no pins, with no markings.
  * HD647180X0P6 MCU with internal ROM (not dumped)
  * 1 32.000 MHz and 1 21.000 MHz XTALs
  * 1 OKI M6295V with 1.056 MHz resonator
  * 1 custom 06B30P
  * 1 custom 101B10P
  * 6 8-dip banks
  * 1 MB8421-12L

  Exact markings on CPU block and MCU from the PCBs the games were dumped from:

  Lucky 37:
  A001: marked A 9A2. Sticker: 8907 1990.6 L-37 TYPE301
  MCU: Sticker: 8907 1990.6 L-37 M.COM

  Lucky 25:
  A001: marked B 9F2. No sticker
  MCU: No sticker

  Lucky 21:
  A001: marked A 9B2. Sticker: 8907 199(unreadable) L-21 TYPE302
  MCU: Sticker: 8907 1990.6 L-21 M.COM


  Lucky 21-D:
  A001: marked B 9G1. Sticker: 8907 1991.03 L-21-D BET/WON
  MCU: Sticker: 8907 1990.10 L21-D M.COM

  Not much can be done until main CPU and MCU ROMs are dumped.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/z180/z180.h"
#include "machine/mb8421.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"

class lucky37_state : public driver_device
{
public:
	lucky37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{ }

	void lucky37(machine_config &config);
};

static INPUT_PORTS_START( lucky37 )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")

	PORT_START("DSW5")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW5:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW5:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW5:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW5:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW5:8")

	PORT_START("DSW6")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW6:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW6:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW6:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW6:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW6:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW6:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW6:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW6:8")
INPUT_PORTS_END

void lucky37_state::lucky37(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, "maincpu", 32_MHz_XTAL / 8); // not verified
}

ROM_START( lucky21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-a9b2",    0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0xc0000, "unsorted", 0 ) // no ROM 6 present
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(2198b736) SHA1(556fd89dc9d1183a672324b7e1bb6350331459d2) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(fe7bbfef) SHA1(5b1413d26049e4e5c04e05a71f552d2999d57ed5) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(c4c3f642) SHA1(4dba751f74717e4ef158f21c3e2a1b2d4802bb51) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(358d3791) SHA1(d3e01008dbfc0daea255053f1d269e898d8698ea) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(0331c70a) SHA1(e6ace84001bfbbd78acdd30c8d5f631705731e81) )
	ROM_LOAD( "7.bin", 0xa0000, 0x20000, CRC(e43403d2) SHA1(3beddbd0476d88aa5f7b918b95fec382c28a4fe5) )
ROM_END

ROM_START( lucky21d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-b9g1",    0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0xe0000, "unsorted", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(e512ec6d) SHA1(28925c54edc002ee9d575e2ef53bccb02df176a0) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(8ffcb12c) SHA1(36c395765c8f50cf76eee18bf6c81a6bf20afb09) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(23b72a30) SHA1(dca28dadb4ed4200a37e77706b4db003e07f7336) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(f47ee6d7) SHA1(97235e053e2913041953be37352c51ab8399a209) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(ba7c7d3b) SHA1(cb849218ec9716f4ed48115ab0a091a2d19d5314) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(f20ef81e) SHA1(5db3f106b555b98518ef0e5b11cc582369e52ff8) )
	ROM_LOAD( "7.bin", 0xc0000, 0x20000, CRC(e43403d2) SHA1(3beddbd0476d88aa5f7b918b95fec382c28a4fe5) )
ROM_END

ROM_START( lucky25 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-b9f2",    0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0xc0000, "unsorted", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(e504afa8) SHA1(efc984037ca692de44d7f829fec6445315bf5a54) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(d81e51cc) SHA1(273c93bf0657da4921de55ffdfba0940ff90bc15) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(4dc0e8c4) SHA1(3dd5b64dbe6d503872e06fcb9e9a85b645accf8c) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(f449eae6) SHA1(07830626a4d68a6ee3721f5306addfaf05c319ca) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(d10202a3) SHA1(3a866bc0585f90c5cfd75ba1ced2912a8a448678) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(6c551fee) SHA1(d69a7badfa05fa35d3cad1cf565ad554f927c4b4) )
ROM_END

ROM_START( lucky37 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-a9a2",    0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x8000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0xc0000, "unsorted", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(15729cbb) SHA1(2e3255f0ff2e084311be41c908ab5a2be8c1b25d) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(4a48de16) SHA1(f74fabc3523126e935ccedd8e5efbe1d1c8b80ee) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(84a4535c) SHA1(8f6ff1503e5fd5ee6f9f2ff101c4958fad040321) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(bd6d49ff) SHA1(b19be51a93b76e506eab8f1cee898b1750c2ee96) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(a8fad083) SHA1(85c963859a0432b26ed66ad6e4edc56071ac55a3) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(3283c1b7) SHA1(7a30dd55216b47332ea7c18c2378352f405b2f0a) )
ROM_END

GAME( 199?, lucky21,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 21",   MACHINE_IS_SKELETON )
GAME( 199?, lucky21d, 0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 21-D", MACHINE_IS_SKELETON )
GAME( 199?, lucky25,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 25",   MACHINE_IS_SKELETON )
GAME( 199?, lucky37,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 37",   MACHINE_IS_SKELETON )
