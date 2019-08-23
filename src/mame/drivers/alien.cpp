// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************************

    Capcom Medal / Medalusion / Medalusion 2 hardware (c) 2005 Capcom

    skeleton driver

   Main boards:
   Capcom AMT-04054 "YUI2"
    - Hitachi SH-4 HD6417750S at 200MHz
    - Elpida DS1232AA-75 1M x 32-bit x 4-banks (128Mbit) SDRAM
    - Altera ACEX EP1K50TC144-3 FPGA
    - 2 x Xilinx XC9572XL CPLD (TQFP-100) stamped AMTPS005 and AMTPS011
    - M48T35Y timekeeper device
  * - 2 x Fujitsu MB86292 'Orchid' Graphics Controller
  * - 8 x Fujitsu MB81E161622-10FH 512K x 16-bit x 2-banks (16Mbit) FCRAM
  * - 2 x ADV7120 Video DAC
  * - Yamaha YMZ770B-F 'AMMSL' SPU
   components marked * might be not populated

   Capcom AMT-02008 "YUI"
    - FPGA is Altera FLEX 6000, other differences / details is unknown

   Upper boards (game specific):

   Capcom AMT-04041 (Alien: The Arcade)
    - 2 x Panasonic MN677511DE MPEG2 decoder
    - 2 x Hynix HY57V161610DTC-7 512K x 16-bit x 2-banks (16Mbit) SDRAM (TSOPII-50)
    - Hynix HY57V643220DTP-7 512K x 32-bit x 4-banks (64Mbit) SDRAM (TSOPII-86)
    - Altera ACEX EP1K100FC484-2 FPGA
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

   Capcom AMT-05057 (Pingu's Ice Block and Donkey Kong Banana Kingdom)
    - Altera MAX EPM3064ATC100-10 CPLD (QFP-100) stamped PS015
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

    Games list: http://web.archive.org/web/20090227151212/http://www.capcom.co.jp:80/arcade/medalgame.html

    6-8 player coin pusher machines:
 *   - Alien: The Arcade Medal Edition (c) 2005
     - Alien Danger (c) 2007
     - Chibi Maruko-chan Aim Fuji Nippon Ichi! (c) 2008
 *   - Donkey Kong Banana Kingdom (c) 2006
 !   - Super Mario Fushigi no Korokoro Party 2 (c) 2005

    Single player medal machines:
    Medalusion:
     - Chibi Maruko-chan's "Sugoroku Playfully Everyone" Volume (c) 2003
     - Donkey Kong: Jungle Fever (c) 2005 Capcom / Nintendo / Namco
     - Rockman EXE The Medal Operation (c) 2005
     - Super Mario Fushigi No JanJanLand (c) 2005

    Medalusion 2:
 !   - Doko Demo Issho Toro's Fishing (c) 2006
 !   - Pingu's Ice Block (c) 2005
     - Geki Makaimura (c) 2005
 !   - Won! Tertainment Happy Channel (c) 2008 note: main board is different, uses Yamaha YMZ770C instead of YMZ770B

 * - dumped
 ! - CF card dumped, boot roms missing

***********************************************************************************/


#include "emu.h"
#include "cpu/sh/sh4.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK    XTAL(200'000'000)

class alien_state : public driver_device
{
public:
	alien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void alien(machine_config &config);

	void init_dkbanans();

private:
	DECLARE_READ64_MEMBER(test_r);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void alien_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
	virtual void video_start() override;
};

void alien_state::video_start()
{
}

uint32_t alien_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ64_MEMBER( alien_state::test_r )
{
	return machine().rand();
}

void alien_state::alien_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom();
	map(0x08000000, 0x08000007).r(FUNC(alien_state::test_r)); //hangs if zero
	map(0x0cfe0000, 0x0cffffff).ram();
	map(0x10000000, 0x13ffffff).ram();
	map(0x18000000, 0x1800000f).r(FUNC(alien_state::test_r)).nopw();
}




static INPUT_PORTS_START( alien )

INPUT_PORTS_END


void alien_state::machine_reset()
{
	//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void alien_state::alien(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, MASTER_CLOCK);    /* 200MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &alien_state::alien_map);
	m_maincpu->set_force_no_drc(true);

	/* video hardware */

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(alien_state::screen_update));
	screen.set_size((32)*8, (32)*8);
	screen.set_visarea_full();

	PALETTE(config, "palette").set_entries(0x1000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

void alien_state::init_dkbanans()
{
	uint8_t *rom = memregion("maincpu")->base();
	rom[2] = 0x02;
	rom[3] = 0x60;
}

/*************************
*        Rom Load        *
*************************/

ROM_START( alien )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "aln_s04.4.ic30", 0x000000, 0x400000, CRC(11777d3f) SHA1(8cc9fcae7911e6be273b4532d89b44a309687ead) )
	ROM_LOAD32_WORD( "aln_s05.5.ic33", 0x000002, 0x400000, CRC(71d2f22c) SHA1(16b25aa34f8b0d988565e7ab7cecc4df62ee8cf3) )

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "s29jl064hxxtfi00.u35", 0x000000, 0x800100, CRC(01890c61) SHA1(4fad321f42eab835351c6d5f73539bdbed80affe) )

	ROM_REGION( 0x8000, "nvram", ROMREGION_ERASEFF) //timekeeper device
	ROM_LOAD( "m48t35y.3.ic26", 0x000000, 0x007ff8, CRC(060b0a75) SHA1(7ddf380ee0e7b54533ef7e248405bfce1c5dbb4b) )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "alien", 0, SHA1(0328f12765db41a9ef5c8bfb88d4983345093072) )
ROM_END

// ROM board only dumped, main board is missing
ROM_START( pingu )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "ic10", 0x000000, 0x800100, CRC(04cf9722) SHA1(854e056a03d6f7ac9b438ba9ce8a0499a79bdec8) )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "pingu", 0, SHA1(9c74e30906f229eba4bff8262c98e556d3ea1c23) )
ROM_END

// Host unit board, GPUs and YMZ770B not populated.
ROM_START( dkbanana )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_m04.ic30", 0x000000, 0x400000, CRC(a294f17c) SHA1(7e0f865342f63f93a9a31ad7e6d3b70c59f3fa1b) )
	ROM_LOAD32_WORD( "dnk_m05.ic33", 0x000002, 0x400000, CRC(22f5db87) SHA1(bdca65d39e94d88979218c8c586c6f20bb00e5ce) )

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "29lj064.ic10", 0x000000, 0x800100, CRC(67cec133) SHA1(1412287fe977eb422a3cca6a0da1523859c2562e) )

	ROM_REGION( 0x8000, "nvram", ROMREGION_ERASEFF) //timekeeper device
	ROM_LOAD( "m48t35y.ic26", 0x000000, 0x008000, CRC(a708bbeb) SHA1(ec96decbc7e63d700b844704dafde14513eea20e) )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// Satellite unit board
ROM_START( dkbanans )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_s04.ic30", 0x000000, 0x400000, CRC(eed7d46f) SHA1(43edb15ff72952f7c9825e5735faa238edfd934d) )
	ROM_LOAD32_WORD( "dnk_s05.ic33", 0x000002, 0x400000, BAD_DUMP CRC(2fc88385) SHA1(03393bdb1fa526c70d766469c37b453f0e1eb8a3) ) // 2 first bytes is bad/wrong or (unlikely) supplied by protection, see driver init

	ROM_REGION( 0x1000000, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "ic10", 0x000000, 0x1000000, NO_DUMP )

	ROM_REGION( 0x8000, "nvram", ROMREGION_ERASEFF) //timekeeper device
	ROM_LOAD( "m48t35y.ic26", 0x000000, 0x008000, CRC(68ca07b7) SHA1(d0a3353bf0fbd98ce335dc5116b214b18c4188dd) )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( dokodemo )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800100, "ymz770b", ROMREGION_ERASEFF ) //sound samples flash rom, not really needed, programmed by boot loader

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dokodemo", 0, SHA1(0c786b6857a29b26971578abe1c8439fe43d94b5) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( masmario )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x1000000, "ymz770b", ROMREGION_ERASEFF )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "massmario", 0, SHA1(9632c91bf2e4983ee29f417e3122e9380baee25b) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( wontame )
	ROM_REGION( 0x800000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800000, "ymz770c", ROMREGION_ERASEFF )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "wontame", 0, SHA1(eb4fe73d5f723b3af08d96c6d3061c9bbc7b2488) )
ROM_END


GAME( 2005, alien,    0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom",               "Alien: The Arcade Medal Edition", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2006, pingu,    0,        alien, alien, alien_state, empty_init,    ROT0, "Pygos Group / Capcom", "Pingu's Ice Block", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanana, 0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom",               "Donkey Kong Banana Kingdom (host)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanans, dkbanana, alien, alien, alien_state, init_dkbanans, ROT0, "Capcom",               "Donkey Kong Banana Kingdom (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2006, dokodemo, 0,        alien, alien, alien_state, empty_init,    ROT0, "Sony / Capcom",        "Doko Demo Issho: Toro's Fishing", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, masmario, 0,        alien, alien, alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party 2", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2008, wontame,  0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom / Tomy",        "Won! Tertainment Happy Channel (Ver E)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
