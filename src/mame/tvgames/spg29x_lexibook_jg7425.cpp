// license:BSD-3-Clause
// copyright-holders:David Haywood


/*

NOTE: these are SPG293 based, which seems to be the same as SPG290 (hyperscan) but maybe with some different on-die modules available.
      see https://github.com/gatecat/emu293

---------

a 221 games console which uses a 4GB sd-card for gamestorage and a MX29LV160 flashrom for the internal bios. (only 512kb are used from the 2mb romspace)
Starting the console without SD-Card just show's a looping video with "Please insert Memory Card".

SD card image produced with WinHex (hardware write blocker used to prevent Windows from corrupting data)
compressed with "chdman createhd -i 4GBSD.img -o lexibook_jg7425_4gbsd.chd" (is this correct?)

TODO:
is there an internal ROM / bootstrap area, or does this SunPlus core use vectors in a different way to the one in hyperscan.cpp?

(only noteworthy features of PCB are ROM + RAM + Cpu Glob)

*/

#include "emu.h"
#include "screen.h"
#include "cpu/score/score.h"


namespace {

class lexibook_jg7425_state : public driver_device
{
public:
	lexibook_jg7425_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_romregion(*this, "extrom")
	{ }

	void lexibook_jg7425(machine_config &config);

	void map(address_map &map) ATTR_COLD;

protected:
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<score7_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_mainram;
	required_region_ptr<uint8_t> m_romregion;
};



void lexibook_jg7425_state::machine_start()
{
}

void lexibook_jg7425_state::machine_reset()
{
	// I think this code should be running from RAM at least, probably some kind of bootstrap / internal ROM to copy it? (hyperscan.cpp indicates that SoC can have internal ROM at least)

	/*

	the first 0x20 bytes of the ROM seem to be some pointers

	F7 FF FE FF
	FF FF FF FF
	FF FF FF FF
	FC 01 50 A0 (address A05001FC) - load address for the main program
	FC 01 70 A0 (address A07001FC) - maybe top of program to copy?
	00 10 50 A0 (address A0501000) - entry point??
	20 00 00 98 (address 98000020)
	00 00 00 00

	then there is a program which loads at A05001FC between 0x20 - 0x6DBD3
	this program is then partially repeated until it is abruptly cut off
	with a block of 00 at 0x7ff20
	the 2nd half of the ROM (0x80000+) is just 0xff fill

	*/

	uint32_t loadaddr = (m_romregion[0x0c] << 0) | (m_romregion[0x0d] << 8) | (m_romregion[0x0e] << 16) | (m_romregion[0x0f] << 24);
	uint32_t endaddr  = (m_romregion[0x10] << 0) | (m_romregion[0x11] << 8) | (m_romregion[0x12] << 16) | (m_romregion[0x13] << 24);
	uint32_t entry    = (m_romregion[0x14] << 0) | (m_romregion[0x15] << 8) | (m_romregion[0x16] << 16) | (m_romregion[0x17] << 24);

	uint8_t* rom = (uint8_t*)&m_romregion[0];

	for (int i = 0; i < endaddr-loadaddr; i++)
	{
		uint32_t data = rom[0x20 + i];

		m_maincpu->space(AS_PROGRAM).write_byte(loadaddr+i, data);
	}

	m_maincpu->set_state_int(SCORE_PC, entry);
}

static INPUT_PORTS_START( lexibook_jg7425 )
INPUT_PORTS_END


uint32_t lexibook_jg7425_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void lexibook_jg7425_state::map(address_map &map)
{

	map(0xa0000000, 0xa0ffffff).ram().share("mainram");

	map(0x9f000000, 0x9fffffff).ram().share("mainram");
	map(0xbf000000, 0xbfffffff).ram().share("mainram");

	// it quickly ends up jumping to BF000024, which is probably internal ROM - can we simulate what it wants?
}


void lexibook_jg7425_state::lexibook_jg7425(machine_config &config)
{
	/* basic machine hardware */
	SCORE7(config, m_maincpu, XTAL(27'000'000) * 4);   // ? not certain on exact type
	m_maincpu->set_addrmap(AS_PROGRAM, &lexibook_jg7425_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(lexibook_jg7425_state::screen_update));
}

ROM_START( fundr200 )
	ROM_REGION( 0x200000, "extrom", ROMREGION_ERASEFF )
	ROM_LOAD( "mx29lv160.u6", 0x000000, 0x200000, CRC(43c90080) SHA1(4c9e5c8f880d40bd684357ce67ae45c3f5d24b62) )

	DISK_REGION( "ata:0:hdd" ) /* 4GB SD Card */
	DISK_IMAGE( "funderdome", 0, SHA1(ffe80581455ed41acb2e968d25f29a2c2a173b54) )
ROM_END

ROM_START( lx_jg7425 )
	ROM_REGION( 0x200000, "extrom", ROMREGION_ERASEFF )
	ROM_LOAD( "mx29lv160.u6", 0x000000, 0x200000, CRC(43c90080) SHA1(4c9e5c8f880d40bd684357ce67ae45c3f5d24b62) )

	DISK_REGION( "ata:0:hdd" ) /* 4GB SD Card */
	DISK_IMAGE( "lexibook_jg7425_4gbsd", 0, SHA1(dc0985103edec3992efdd493feef6185daedb3fd) )
ROM_END

ROM_START( lx_aven )
	ROM_REGION( 0x200000, "extrom", ROMREGION_ERASEFF )
	ROM_LOAD( "29lv800.bin", 0x000000, 0x100000, CRC(7b107f6c) SHA1(3a8e37e51dab5cab9977261e0ac17ba5194a9370) )

	DISK_REGION( "ata:0:hdd" ) /* 4GB SD Card */
	DISK_IMAGE( "sd-card", 0, SHA1(911da7bf7dac391e3329e17e3f411caafac52f0f) )
ROM_END

ROM_START( lx_frozen )
	ROM_REGION( 0x200000, "extrom", ROMREGION_ERASEFF )
	ROM_LOAD( "29lv800.bin", 0x000000, 0x100000, CRC(7b107f6c) SHA1(3a8e37e51dab5cab9977261e0ac17ba5194a9370) )

	DISK_REGION( "ata:0:hdd" ) /* 4GB SD Card */
	DISK_IMAGE( "sdcard", 0, SHA1(0d727815ba06d7bfe8e092007e24d4931b302ef9) )
ROM_END

ROM_START( zone3d )
	ROM_REGION( 0x100000, "extrom", 0 ) // SPI ROM in this case
	ROM_LOAD("zone_25l8006e_c22014.bin", 0x000000, 0x100000, CRC(8c571771) SHA1(cdb46850286d31bf58d45b75ffc396ed774ac4fd) )

	/*
	model: Lexar SD
	revision: LX01
	serial number: 00000000XL10

	size: 362.00 MiB (741376 sectors * 512 bytes)
	unk1: 0000000000000007
	unk2: 00000000000000fa
	unk3: 01

	The SD card has no label, but there's some printing on the back:
	MMAGF0380M3085-WY
	TC00201106 by Taiwan

	--
	Dumped with hardware write blocker, so this image is correct, and hasn't been corrupted by Windows

	Image contains a FAT filesystem with a number of compressed? programs that presumably get loaded into RAM by
	the bootloader in the serial flash ROM
	*/

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "zone3d", 0, SHA1(77971e2dbfb2ceac12f482d72539c2e042fd9108) )

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("internal.rom", 0x000000, 0x008000, NO_DUMP)
ROM_END



ROM_START( ubox30 )
	ROM_REGION( 0x200000, "extrom", 0 ) // SPI ROM in this case
	ROM_LOAD("ubox_xm25qe16bzig_204015.bin", 0x000000, 0x200000, CRC(f8135947) SHA1(f2a075ae2b0bae186202f1019b566ffc411742bb) )
	ROM_IGNORE(0x300)

	DISK_REGION( "cfcard" )
	DISK_IMAGE( "ubox_sd_512", 0, SHA1(99f2f1437d644a5e1fe48ce1445acf48fb8b0359) )

	ROM_REGION( 0x008000, "spg290", ROMREGION_32BIT | ROMREGION_LE )
	ROM_LOAD32_DWORD("internal.rom", 0x000000, 0x008000, NO_DUMP)
ROM_END


} // anonymous namespace

CONS( 2015, fundr200,    0,         0,     lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Funderdome", "Funderdome Video Game Entertainment System 200+ Games (FUN-GAME32-1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // FUN-GAME32-1 on manual
CONS( 2015, lx_jg7425,   0,         0,     lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Lexibook JG7425 221-in-1", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2016, lx_aven,     0,         0,     lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Marvel Avengers TV Game Console (32-bit, Lexibook)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2016, lx_frozen,   0,         0,     lexibook_jg7425,   lexibook_jg7425, lexibook_jg7425_state, empty_init, "Lexibook", "Disney Frozen TV Game Console (32-bit, Lexibook, JG7420FZ)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// slightly different, but same basic structure of the external ROM
COMP( 201?, zone3d,      0,         0,      lexibook_jg7425,  lexibook_jg7425, lexibook_jg7425_state, empty_init,"Zone", "Zone 3D", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// Unknown hardware, HDMI dongle with wireless pads.
// Uses standard chips, not globs, but surface details on CPU/SoC have been erased.
//
// It has the GPspispi header, so is definitely a GeneralPlus / SunPlus chip, and it has a mix of '32-bit' games
// as well as some 8-bit NES/FC games presumably running on an emulator like the above units, but the code in the
// SPI ROM does not seem to disassemble to anything meaningful, maybe compressed?
// Front and back have slightly different product names, unknown if NubSup is the manufacturer or part of the product name
COMP( 201?, ubox30,      0,         0,      lexibook_jg7425,  lexibook_jg7425, lexibook_jg7425_state, empty_init,"<unknown>", "NubSup TV Interactive Extreme u-box / Extreme u-box Game Station 32Bit Interactive System - New 30", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
