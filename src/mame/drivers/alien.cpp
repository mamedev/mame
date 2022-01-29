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
  * - Yamaha YMZ770B-F 'AMMSL' SPU at 16.934MHz
   components marked * might be not populated

   Capcom AMT-02008 "YUI"
    - FPGA is Altera FLEX 6000, other differences / details is unknown

   Upper boards (game specific):

   Capcom AMT-02012-01 (Medalusion 1)
    - TODO

   Capcom AMT-04041 (Alien: The Arcade)
    - 2 x Panasonic MN677511DE MPEG2 decoder
    - 2 x Hynix HY57V161610DTC-7 512K x 16-bit x 2-banks (16Mbit) SDRAM (TSOPII-50)
    - Hynix HY57V643220DTP-7 512K x 32-bit x 4-banks (64Mbit) SDRAM (TSOPII-86)
    - Altera ACEX EP1K100FC484-2 FPGA
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

   Capcom AMT-05057 (Donkey Kong Banana Kingdom and Medalusion 2 games)
    - Altera MAX EPM3064ATC100-10 CPLD (QFP-100) stamped PS015
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

    Games list: http://web.archive.org/web/20090227151212/http://www.capcom.co.jp:80/arcade/medalgame.html

    6-8 player coin pusher machines:
 *   - Alien: The Arcade Medal Edition (c) 2005
     - Alien Danger (c) 2007
     - Chibi Maruko-chan Aim Fuji Nippon Ichi! (c) 2008
 *   - Donkey Kong Banana Kingdom (c) 2006
 *   - Super Mario Fushigi no Korokoro Party (c) 2004
 !   - Super Mario Fushigi no Korokoro Party 2 (c) 2005

    Single player medal machines:
    Medalusion:
 *   - Chibi Maruko-chan ~Minna de Sugoroku Asobi~ no Maki (c) 2003
     - Donkey Kong: Jungle Fever (c) 2005 Capcom / Nintendo / Namco
     - Rockman EXE The Medal Operation (c) 2005
 *   - Super Mario Fushigi No JanJanLand (c) 2005

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
#include "machine/timekpr.h"
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
	u8 fpga_r();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void alien_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
};

u32 alien_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 alien_state::fpga_r()
{
	u8 fpga_type = 1; // 2 bit value
	return (fpga_type << 5) | 0x10 | (rand() & 7); // status bits TODO
}

void alien_state::alien_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom();
	map(0x04000000, 0x04007fff).rw("m48t35", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0x04800000, 0x04800000).r(FUNC(alien_state::fpga_r));
	map(0x04a00000, 0x04a00007).nopw(); // FPGA config
	map(0x08000000, 0x08000007).portr("DSW");
	map(0x0c000000, 0x0cffffff).ram(); // main RAM
	map(0x10000000, 0x107fffff).ram().share("vram1"); // GPU 1 VRAM
	map(0x11fc0000, 0x11ffffff).ram().share("vregs1"); // GPU 1 regs
	//map(0x12000000, 0x127fffff).ram(); // GPU 2 VRAM
	//map(0x13fc0000, 0x13ffffff).ram(); // GPU 2 regs
	//map(0x18000000, 0x1800000f).r(FUNC(alien_state::test_r)).nopw(); // Alien CF ATA, other games have it other way
}




static INPUT_PORTS_START( alien )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x00010000, 0x00010000, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00020000, 0x00020000, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00040000, 0x00040000, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00080000, 0x00080000, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00100000, 0x00100000, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00200000, 0x00200000, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00400000, 0x00400000, "SW1:7" )
	PORT_DIPNAME( 0x00800000, 0x00000000, "RAM Tests" ) PORT_DIPLOCATION("SW1:8") // disable tests to make things faster
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x01000000, 0x01000000, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02000000, 0x02000000, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04000000, 0x04000000, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08000000, 0x08000000, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10000000, 0x10000000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20000000, 0x20000000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40000000, 0x40000000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80000000, 0x80000000, "SW2:8" )

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

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	M48T35(config, "m48t35");
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

////////////////////////////
// Custom multi-unit games
////////////////////////////

ROM_START( alien )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "aln_s04.4.ic30", 0x000000, 0x400000, CRC(11777d3f) SHA1(8cc9fcae7911e6be273b4532d89b44a309687ead) )
	ROM_LOAD32_WORD( "aln_s05.5.ic33", 0x000002, 0x400000, CRC(71d2f22c) SHA1(16b25aa34f8b0d988565e7ab7cecc4df62ee8cf3) )

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "s29jl064hxxtfi00.u35", 0x000000, 0x800100, CRC(01890c61) SHA1(4fad321f42eab835351c6d5f73539bdbed80affe) )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "alien", 0, SHA1(0328f12765db41a9ef5c8bfb88d4983345093072) )
ROM_END

// Host unit board, GPUs and YMZ770B not populated.
ROM_START( dkbanana )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_m04.ic30", 0x000000, 0x400000, CRC(a294f17c) SHA1(7e0f865342f63f93a9a31ad7e6d3b70c59f3fa1b) )
	ROM_LOAD32_WORD( "dnk_m05.ic33", 0x000002, 0x400000, CRC(22f5db87) SHA1(bdca65d39e94d88979218c8c586c6f20bb00e5ce) )

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "29lj064.ic10", 0x000000, 0x800100, CRC(67cec133) SHA1(1412287fe977eb422a3cca6a0da1523859c2562e) )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// Satellite unit board
ROM_START( dkbanans )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_s04.ic30", 0x000000, 0x400000, CRC(eed7d46f) SHA1(43edb15ff72952f7c9825e5735faa238edfd934d) )
	ROM_LOAD32_WORD( "dnk_s05.ic33", 0x000002, 0x400000, BAD_DUMP CRC(2fc88385) SHA1(03393bdb1fa526c70d766469c37b453f0e1eb8a3) ) // 2 first bytes is bad/wrong or (unlikely) supplied by protection, see driver init

	ROM_REGION( 0x1000000, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "ic10", 0x000000, 0x1000000, NO_DUMP )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// 'Center' unit, GPUs and YMZ770B not populated.
ROM_START( masmario )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "mpf_m04f.ic30", 0x000000, 0x200000, CRC(f83ffb1a) SHA1(fa0ec83c21d81288b69e23ee46db359a3902648e) )
	ROM_LOAD32_WORD( "mpf_m05f.ic33", 0x000002, 0x200000, CRC(fe19dfb7) SHA1(2fdc2feb86840448eb9e47f7bd4dcc9adfc36bdf) )

	ROM_REGION( 0x800000, "ymz770b", ROMREGION_ERASEFF ) // not populated
ROM_END

// Satellite unit
ROM_START( masmarios )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS and game code/data
	ROM_LOAD32_WORD( "mpf_s04j.ic30", 0x000000, 0x400000, CRC(73d8b6bd) SHA1(8353df96107303427fc146da29b6a6a5d303c4ee) )
	ROM_LOAD32_WORD( "mpf_s05j.ic33", 0x000002, 0x400000, CRC(1b1e5429) SHA1(4b25fc83172c7422bfe3f2aed9d2ee6c8a2c537f) )
	ROM_LOAD32_WORD( "mpf_s06j.ic39", 0x800000, 0x400000, CRC(dc20e3cc) SHA1(5b9bd0fc4a6abdda16781727b01014b0a68ef8df) )
	ROM_LOAD32_WORD( "mpf_s07j.ic42", 0x800002, 0x400000, CRC(cb08dc74) SHA1(31e658f8bd03fea3dffa5f32dc7ac2e73930b383) )

	ROM_REGION( 0x800000, "ymz770b", 0 )
	ROM_LOAD16_WORD_SWAP( "mpf_s01.ic31", 0x000000, 0x400000, CRC(99688b6d) SHA1(2052471e2a742c05c2bbd6bcb24deca681df41c3) )
	ROM_LOAD16_WORD_SWAP( "mpf_s02.ic38", 0x400000, 0x400000, CRC(251f7111) SHA1(4d6e4111d76e7f56e9aeff19686dd84717ccb78a) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( masmario2 )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x1000000, "ymz770b", ROMREGION_ERASEFF )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "massmario2", 0, SHA1(9632c91bf2e4983ee29f417e3122e9380baee25b) )
ROM_END

////////////////////////
// Medalusion 1 platform
////////////////////////

// uses main board with 1 GPU populated and AMT-02012-01 upper I/O board

ROM_START( mariojjl )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 )
	ROM_LOAD32_WORD( "spm_04c.ic30", 0x000000, 0x400000, CRC(159e912d) SHA1(5db1434d34e52f9c35d71e05675dd035765d2e6f) )
	ROM_LOAD32_WORD( "spm_05c.ic33", 0x000002, 0x400000, CRC(482d2b32) SHA1(01fb4b5f2441dc8c0f07943f190429c19c60b9d6) )

	ROM_REGION( 0x400000, "ymz770b", 0 )
	ROM_LOAD16_WORD_SWAP( "spm_01.ic31", 0x000000, 0x400000, CRC(141761a7) SHA1(ab1029c9277b3932d43308a7b4c106cd526a82c7) )
ROM_END

ROM_START( mmaruchan )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 )
	ROM_LOAD32_WORD( "spt_04b.ic30", 0x000000, 0x400000, CRC(9899f171) SHA1(d114c1ef0608c0740b7d58561c9f39c13b453e3a) )
	ROM_LOAD32_WORD( "spt_05b.ic33", 0x000002, 0x400000, CRC(108efb71) SHA1(3f9e1c59f7af60976d140bf68b75c270a364f3a2) )

	ROM_REGION( 0x400000, "ymz770b", 0 )
	ROM_LOAD16_WORD_SWAP( "spt_01.ic31", 0x000000, 0x400000, CRC(790b4bed) SHA1(3df68610f8b81dd5f74dca0f05da47a539b45163) )
ROM_END

////////////////////////
// Medalusion 2 platform
////////////////////////

// CF card only dumped, boot ROMs is missing
ROM_START( dokodemo )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800100, "ymz770b", ROMREGION_ERASEFF ) //sound samples flash rom, not really needed, programmed by boot loader

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "dokodemo", 0, SHA1(0c786b6857a29b26971578abe1c8439fe43d94b5) )
ROM_END

// ROM board only dumped, main board is missing
ROM_START( pingu )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800100, "ymz770b", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "ic10", 0x000000, 0x800100, CRC(04cf9722) SHA1(854e056a03d6f7ac9b438ba9ce8a0499a79bdec8) )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "pingu", 0, SHA1(9c74e30906f229eba4bff8262c98e556d3ea1c23) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( wontame )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(6b2d2ef1) SHA1(0db6490b40c5716c1271b7f99608e8c7ad916516) ) //
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(64049fc3) SHA1(b373b2c8cb4d66b9c700e0542bd26444484fae40) ) // modified boot roms from dkbanans

	ROM_REGION( 0x800000, "ymz770c", ROMREGION_ERASEFF )

	DISK_REGION( "card" ) //compact flash
	DISK_IMAGE( "wontame", 0, SHA1(eb4fe73d5f723b3af08d96c6d3061c9bbc7b2488) )
ROM_END

// Custom
GAME( 2005, alien,     0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom",               "Alien: The Arcade Medal Edition", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanana,  0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom",               "Donkey Kong Banana Kingdom (host)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanans,  dkbanana, alien, alien, alien_state, init_dkbanans, ROT0, "Capcom",               "Donkey Kong Banana Kingdom (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, masmario,  0,        alien, alien, alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party (center)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, masmarios, 0,        alien, alien, alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, masmario2, 0,        alien, alien, alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party 2", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// Medalusion 1
GAME( 2006, mariojjl,  0,        alien, alien, alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no JanJanLand", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, mmaruchan, 0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom",               "Chibi Maruko-chan ~Minna de Sugoroku Asobi~ no Maki", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // ちびまる子ちゃん「みんなですごろく遊び」の巻
// Medalusion 2
GAME( 2006, dokodemo,  0,        alien, alien, alien_state, empty_init,    ROT0, "Sony / Capcom",        "Doko Demo Issho: Toro's Fishing", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2006, pingu,     0,        alien, alien, alien_state, empty_init,    ROT0, "Pygos Group / Capcom", "Pingu's Ice Block", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2008, wontame,   0,        alien, alien, alien_state, empty_init,    ROT0, "Capcom / Tomy",        "Won! Tertainment Happy Channel (Ver E)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
