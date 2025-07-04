// license:BSD-3-Clause
// copyright-holders:

/*
Sealy Z80-based games

Main components:
Z80C0006PEC or equivalent
12.000 MHz XTAL
6264 RAM (manufacturer and latency may vary)
62256 RAM (manufacturer and latency may vary)
Altera MAX EPM3256AOC208-10
93C46 EEPROM
U6295 (Oki M6295 clone)

TODO:
- program ROMs are encrypted
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sealy_z80_state : public driver_device
{
public:
	sealy_z80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sealy(machine_config &config) ATTR_COLD;

	void init_djddz() ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t sealy_z80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void sealy_z80_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


static INPUT_PORTS_START( djddz ) // TODO
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

// no DIP switches, just one reset push button
INPUT_PORTS_END


static const gfx_layout gfxlayout_8x8x16 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 2) },
	{ STEP8(0, 8*2) },
	{ STEP8(0, 8*8*2) },
	8*8*16
};


static GFXDECODE_START( gfx ) // TODO: not 100% correct, but enough to see some stuff
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x16, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout_8x8x16, 0, 1 )
GFXDECODE_END


void sealy_z80_state::sealy(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // divider not verified, but part rated for 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sealy_z80_state::program_map);

	EEPROM_93C46_8BIT(config, "eeprom");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(sealy_z80_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);
	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divider and pin 7 not verified
}


// 百变斗地主 (Bǎi Biàn Dòu Dìzhǔ). All labels have 百变斗地主
ROM_START( bbddz )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u7", 0x00000, 0x20000, CRC(d82df292) SHA1(f354a3d9b29abb61a447d507b37d28f49983e59d) ) // 27c1001a

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u15", 0x000000, 0x200000, CRC(937e9e76) SHA1(95907ddadc2bf260d88a391ab1f61e8931ec7cc3) ) // 29f1611

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "u13", 0x000000, 0x200000, CRC(1ee033bb) SHA1(14ab0702e17add44dfc82ce21dd5a37c05a1b2a2) ) // 29f1611

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u9", 0x00000, 0x80000, CRC(249b1a34) SHA1(94af1a9c64fb7d06a7510d527c176b2fa6845885) ) // 29f040

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, CRC(b4c229f0) SHA1(632cd6749ed9c4564258b2487d27f10785639653) )
ROM_END

// 斗地主Ⅱ (Dòu Dìzhǔ II). All labels prepend 斗地主Ⅱ to what's below
ROM_START( ddz2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(01cbe7a5) SHA1(f46339bec4e898afaa78831632cea4013877258d) ) // 27c4010

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(200ece45) SHA1(ab9b19464850c9e75646e382334a5d28a360195c) ) // 27c4096

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(c62be0a4) SHA1(1cbfecba43b475f1a175f69fda498e662ac26720) ) // 27c4096

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x40000, CRC(e089cf82) SHA1(567736b1418b86ea35e29fb9f8a408436c8a03c8) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, CRC(3228fc88) SHA1(6679cf740ffbdb1aef485c2a3218030947f63ba4) )
ROM_END

// 顶级斗地主 (Dǐngjí Dòu Dìzhǔ). All labels prepend 顶级斗地主 to what's below
// same data was also found in ROMs with 顶级100分 (Dǐngjí 100 Fēn) labels
ROM_START( djddz )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(54abc7a0) SHA1(25494e0862aa6b03398270efe2a3659180be38ec) ) // 27c010

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(6fa8f11e) SHA1(731f90a929b5b638fa45de24918df6377136276d) ) // 27c4096, FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(74997b0f) SHA1(1c2b0aeaf71fa000856b8aa405d7853f8e652257) ) // 27c4096

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x80000, CRC(249b1a34) SHA1(94af1a9c64fb7d06a7510d527c176b2fa6845885) ) // 27c040

	ROM_REGION( 0x80, "eeprom", 0 ) // keeping dumps from both PCBs for now, until it can be determined if they are useful or just user data
	ROM_LOAD( "93c46",   0x00, 0x80, CRC(bed2d363) SHA1(7e0c0d4c47274a87024e3e5cd74bc883b7d46415) ) // this dump comes from the PCB with 顶级100分 labels
	ROM_LOAD( "93c46_2", 0x00, 0x80, CRC(43c87f6c) SHA1(1e9ed6033cacd5412de7d236392f626abd1e2eb8) ) // this dump comes from the PCB with 顶级斗地主 labels
ROM_END

// 漂亮金花 (Piàoliang Jīnhuā). All labels prepend 漂亮金花 to what's below
ROM_START( pljh )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u11", 0x00000, 0x20000, CRC(18b6d64d) SHA1(a17e298098a44a4ffd19c008c45eef09aa35b110) ) // 27c010, 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "1.u18", 0x000000, 0x200000, CRC(8e4cbc34) SHA1(2dcc9ff890f90a440da210742b6564894a627c3b) ) // 27c4096

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "2.u21", 0x000000, 0x200000, CRC(0b774cdd) SHA1(f9b192a67538596d295550ad6316d3fe1e5f7f6a) ) // 27c4096

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "4.u23", 0x00000, 0x40000, CRC(8cbb5623) SHA1(90169df14264c1e53040bc43106fd8b86b4f1d59) ) // 27c020

	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD( "93c46", 0x00, 0x80, NO_DUMP )
ROM_END


void sealy_z80_state::init_djddz() // TODO: not enough
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x14ca)
		{
			case 0x0000: rom[i] ^= 0xc0; break;
			case 0x0002: rom[i] ^= 0x41; break;
			case 0x0008: rom[i] ^= 0xc0; break;
			case 0x000a: rom[i] ^= 0x30; break;
			case 0x0040: rom[i] ^= 0x64; break;
			case 0x0042: rom[i] ^= 0x41; break;
			case 0x0048: rom[i] ^= 0x30; break;
			case 0x004a: rom[i] ^= 0x41; break;
			case 0x0080: rom[i] ^= 0x30; break;
			case 0x0082: rom[i] ^= 0x81; break;
			case 0x0088: rom[i] ^= 0x41; break;
			case 0x008a: rom[i] ^= 0x64; break;
			case 0x00c0: rom[i] ^= 0x30; break;
			case 0x00c2: rom[i] ^= 0x81; break;
			case 0x00c8: rom[i] ^= 0x41; break;
			case 0x00ca: rom[i] ^= 0x30; break;
			case 0x0400: rom[i] ^= 0xc0; break;
			case 0x0402: rom[i] ^= 0x30; break;
			case 0x0408: rom[i] ^= 0x81; break;
			case 0x040a: rom[i] ^= 0xc0; break;
			case 0x0440: rom[i] ^= 0x64; break;
			case 0x0442: rom[i] ^= 0x41; break;
			case 0x0448: rom[i] ^= 0x64; break;
			case 0x044a: rom[i] ^= 0x81; break;
			case 0x0480: rom[i] ^= 0x41; break;
			case 0x0482: rom[i] ^= 0x81; break;
			case 0x0488: rom[i] ^= 0x30; break;
			case 0x048a: rom[i] ^= 0xc0; break;
			case 0x04c0: rom[i] ^= 0x64; break;
			case 0x04c2: rom[i] ^= 0x81; break;
			case 0x04c8: rom[i] ^= 0xc0; break;
			case 0x04ca: rom[i] ^= 0x64; break;
			case 0x1000: rom[i] ^= 0x30; break;
			case 0x1002: rom[i] ^= 0x81; break;
			case 0x1008: rom[i] ^= 0xc0; break;
			case 0x100a: rom[i] ^= 0x64; break;
			case 0x1040: rom[i] ^= 0xc0; break;
			case 0x1042: rom[i] ^= 0x30; break;
			case 0x1048: rom[i] ^= 0x81; break;
			case 0x104a: rom[i] ^= 0x41; break;
			case 0x1080: rom[i] ^= 0x81; break;
			case 0x1082: rom[i] ^= 0x64; break;
			case 0x1088: rom[i] ^= 0xc0; break;
			case 0x108a: rom[i] ^= 0x41; break;
			case 0x10c0: rom[i] ^= 0x30; break;
			case 0x10c2: rom[i] ^= 0x64; break;
			case 0x10c8: rom[i] ^= 0x81; break;
			case 0x10ca: rom[i] ^= 0x64; break;
			case 0x1400: rom[i] ^= 0x41; break;
			case 0x1402: rom[i] ^= 0xc0; break;
			case 0x1408: rom[i] ^= 0x41; break;
			case 0x140a: rom[i] ^= 0x81; break;
			case 0x1440: rom[i] ^= 0x30; break;
			case 0x1442: rom[i] ^= 0xc0; break;
			case 0x1448: rom[i] ^= 0x41; break;
			case 0x144a: rom[i] ^= 0x64; break;
			case 0x1480: rom[i] ^= 0xc0; break;
			case 0x1482: rom[i] ^= 0x81; break;
			case 0x1488: rom[i] ^= 0x30; break;
			case 0x148a: rom[i] ^= 0xc0; break;
			case 0x14c0: rom[i] ^= 0x41; break;
			case 0x14c2: rom[i] ^= 0x64; break;
			case 0x14c8: rom[i] ^= 0x30; break;
			case 0x14ca: rom[i] ^= 0x81; break;
		}
	}
}

} // anonymous namespace


GAME( 2005, bbddz,    0, sealy, djddz, sealy_z80_state, init_djddz, ROT0, "Sealy", "Bai Bian Dou Dizhu", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, ddz2,     0, sealy, djddz, sealy_z80_state, empty_init, ROT0, "Sealy", "Dou Dizhu II",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, djddz,    0, sealy, djddz, sealy_z80_state, init_djddz, ROT0, "Sealy", "Dingji Dou Dizhu",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, pljh,     0, sealy, djddz, sealy_z80_state, empty_init, ROT0, "Sealy", "Piaoliang Jinhua",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
