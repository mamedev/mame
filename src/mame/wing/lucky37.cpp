// license:BSD-3-Clause
// copyright-holders:

/*
  This hardware seems to be an evolution of the one found in lucky74.cpp. Its main components are:
  * A001 CPU block, containing probably a Z80 or compatible CPU and ROM(s) (not dumped).
    Dumper description after opening it: 3 unknown/white chips, which had no pins, with no markings.
  * HD647180X0P6 MCU with internal ROM (decapped and dumped but for lucky25)
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


  Bingo 75:
  A001: marks can't be seen. Sticker: WE8802-A 1989.2
  PCB marked: Wing 8802-C

  1x scratched DIL40 (@ location 3r)
  1x 89206A 61H09516P (@ location 7s)
  1x 101B10P (@ location 7p)
  2x 06B53P (@ locations 1d & 2d)
  1x 06B49P (@ location 2b)

  1x 12 MHz Xtal (@ location 3t)
  5x 8 DIP switches banks (@ locations 5l, 5m, 5n, 5p & 5r)


  Not much can be done until main CPU ROMs are dumped.
*/

#include "emu.h"

#include "cpu/z180/hd647180x.h"
#include "cpu/z80/z80.h"
#include "machine/mb8421.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "screen.h"
#include "speaker.h"


namespace {

class lucky37_state : public driver_device
{
public:
	lucky37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
	{ }

	void lucky37(machine_config &config);

private:
	void mcu_mem_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
};

void lucky37_state::mcu_mem_map(address_map &map)
{
	map(0x04000, 0x047ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0x08000, 0x09fff).ram();
}

void lucky37_state::mcu_io_map(address_map &map)
{
	map(0x0000, 0x007f).noprw(); // internal registers
	map(0x0083, 0x0083).mirror(0xff00).nopw();
}

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
	Z80(config, "maincpu", 32_MHz_XTAL / 8).set_disable(); // not verified

	hd647180x_device &mcu(HD647180X(config, "mcu", 32_MHz_XTAL / 2)); // clock not verified
	mcu.set_addrmap(AS_PROGRAM, &lucky37_state::mcu_mem_map);
	mcu.set_addrmap(AS_IO, &lucky37_state::mcu_io_map);

	mb8421_device &dpram(MB8421(config, "dpram"));
	dpram.intl_callback().set_inputline("mcu", INPUT_LINE_NMI);
}

ROM_START( lucky21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-a9b2",    0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x4000, CRC(23291c00) SHA1(881f722f75e621fe7aa743d005c5c6b336f4e033) ) // decapped

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

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x4000, CRC(b990a28e) SHA1(afaa1fa9d1e314c67af5e23ef2a1b28f6e995a0e) ) // decapped

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

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x4000, NO_DUMP )

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

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x4000,  CRC(c57e6818) SHA1(1d746fed6d13f7f711bcac6685af13e295891a38) ) // decapped

	ROM_REGION( 0xc0000, "unsorted", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(15729cbb) SHA1(2e3255f0ff2e084311be41c908ab5a2be8c1b25d) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(4a48de16) SHA1(f74fabc3523126e935ccedd8e5efbe1d1c8b80ee) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(84a4535c) SHA1(8f6ff1503e5fd5ee6f9f2ff101c4958fad040321) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(bd6d49ff) SHA1(b19be51a93b76e506eab8f1cee898b1750c2ee96) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(a8fad083) SHA1(85c963859a0432b26ed66ad6e4edc56071ac55a3) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(3283c1b7) SHA1(7a30dd55216b47332ea7c18c2378352f405b2f0a) )
ROM_END

ROM_START( bingo75 )    // runs on wing 8802-c board
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a001-nomarks", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "internal_rom", 0x0000, 0x4000,  NO_DUMP )

	ROM_REGION( 0x40000, "unsorted", 0 )
	ROM_LOAD( "6.bin", 0x00000, 0x10000, CRC(bda18251) SHA1(7e9ddea30bbedcc9f4b48a2ca2660505ed45ca8d) )
	ROM_LOAD( "7.bin", 0x10000, 0x10000, CRC(1316d78b) SHA1(9f7a7c5407642d2aad6765d3fb00c7fb5bd08561) )
	ROM_LOAD( "8.bin", 0x20000, 0x10000, CRC(6a3ce8f1) SHA1(48d3a1d3c1739200c10599ec9451683ba60487eb) )
	ROM_LOAD( "9.bin", 0x30000, 0x10000, CRC(325169c5) SHA1(00f3fa44c8bda7fd4a9e7855b5681f254ab6de84) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "27s21.4h",  0x0000, 0x0100, CRC(21ae1edd) SHA1(df6204f30412b5c1835146127f45956bc986b182) )
	ROM_LOAD( "27s21.4j",  0x0100, 0x0100, CRC(a6c2ed16) SHA1(e472bebdc982239ebef5ebe361fb07e6469fc5ed) )
	ROM_LOAD( "27s21.5h",  0x0200, 0x0100, CRC(a5b1dd09) SHA1(0af6eacc09742f35d2703ba011df20f582ce676a) )
	ROM_LOAD( "27s21.5j",  0x0300, 0x0100, CRC(0d985358) SHA1(ceb32eb6fd1804febdd1766f8590ae0db1b03faa) )
	ROM_LOAD( "27s21.5k",  0x0400, 0x0100, CRC(40e880c0) SHA1(74eafbf9506651991b66b6264664325204f61227) )
	ROM_LOAD( "82s129.4k", 0x0500, 0x0100, CRC(0a9e160d) SHA1(d2377850aa344b027a974ef116bb1aee852d61d8) )
	ROM_LOAD( "82s129.5f", 0x0600, 0x0100, CRC(83c3ec8f) SHA1(4a6452ef73061a446e6a8ceb9d077bc71cc8e2b2) )
ROM_END

} // anonymous namespace


GAME( 199?, lucky21,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 21",   MACHINE_IS_SKELETON )
GAME( 199?, lucky21d, 0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 21-D", MACHINE_IS_SKELETON )
GAME( 199?, lucky25,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 25",   MACHINE_IS_SKELETON )
GAME( 199?, lucky37,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Lucky 37",   MACHINE_IS_SKELETON )
GAME( 199?, bingo75,  0, lucky37, lucky37,  lucky37_state, empty_init, ROT0, "Wing Co., Ltd.", "Bingo 75",   MACHINE_IS_SKELETON )
