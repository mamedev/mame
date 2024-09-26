// license:BSD-3-Clause
// copyright-holders:

/*
Astro Corp. 'Hummer' (VGA)
Main components:
Astro V102PX-0XX CPU (004 for Jack's Venture, 013 for Penguin Party)
DigiArt AM001 ADPCM & MP3 sound chip
22.579 MHz XTAL
Actel Igloo AGLP125-CSG289
Astro ROHS (GFX?)
4x LY61L25616ML-20 SRAM
2x LY621024SL-70LL SRAM
4-DIP bank
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hummer_state : public driver_device
{
public:
	hummer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hummer(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t hummer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hummer_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}


static INPUT_PORTS_START( hummer )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void hummer_state::hummer(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 22.579_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hummer_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(120_MHz_XTAL / 10 * 2, 781, 0, 512, 261*2, 0, 240*2); // TODO
	screen.set_screen_update(FUNC(hummer_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::BGR_565, 0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// AM001 for sound
}


ROM_START( pengprty ) // PCBHR REV:E + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-tm_fpus01.01b.tu1", 0x00000, 0x40000, CRC(2569e2b9) SHA1(dcec1e9bfe73a062b891812f2c8eb8407066b993) )
	ROM_LOAD16_BYTE( "2-tm_fpus01.01b.tu3", 0x00001, 0x40000, CRC(23cda107) SHA1(f2c5cba9a3c2c8bfea6bce0b221fd9209810fdf3) )

	ROM_REGION( 0x8000000, "sprites", 0 ) // TODO: probably interleaved
	ROM_LOAD( "mx29gl128eh.u1", 0x0000000, 0x1000000, CRC(80f0d70f) SHA1(82de9bb82a2c5901d5e2dc8f93cd2eee5d65a20b) )
	ROM_LOAD( "mx29gl128eh.u2", 0x1000000, 0x1000000, CRC(9a9da6b4) SHA1(41d95e2de41a99c172ba210e51170e14219d393e) )
	ROM_LOAD( "mx29gl128eh.u3", 0x2000000, 0x1000000, CRC(458f22e5) SHA1(cf3b3084a980568646c588d33123576bd25261a5) )
	ROM_LOAD( "mx29gl128eh.u4", 0x3000000, 0x1000000, CRC(91f1069c) SHA1(67c7e56345f63c01279d39527b31fa9eeb7b4bf0) )
	ROM_LOAD( "mx29gl128eh.u5", 0x4000000, 0x1000000, CRC(fd4bc0d5) SHA1(0829ed0762311b3afd3b2a86c128077138793b53) )
	ROM_LOAD( "mx29gl128eh.u6", 0x5000000, 0x1000000, CRC(2280b3db) SHA1(601a7c1a7b868a0bb9395f38999426b45f008199) )
	ROM_LOAD( "mx29gl128eh.u7", 0x6000000, 0x1000000, CRC(8cfc06f9) SHA1(cc4386c3cde41145672102e3e29cb5c949246f62) )
	ROM_LOAD( "mx29gl128eh.u8", 0x7000000, 0x1000000, CRC(83907de9) SHA1(480df6092e9a78da69b220bfd0f2727a58d677c0) )

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "mx29lv640eb.u53", 0x000000, 0x800000, CRC(0289fef0) SHA1(f349abd69fcbb9b92ba1362f01c3a83a521fdcc3) )
ROM_END

ROM_START( jackvent ) // PCBHR REV:C + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_jvus02.01a.tu1", 0x00000, 0x40000, CRC(71471ff7) SHA1(b342d93417b9f8d2e5e36152a31acb09b6a5acd3) )
	ROM_LOAD16_BYTE( "2_jvus02.01a.tu3", 0x00001, 0x40000, CRC(945a01b1) SHA1(e621c1dd0db573dd5e3bc2202b04c997d84af4fc) )

	ROM_REGION( 0x4000000, "sprites", 0 ) // TODO: probably interleaved
	ROM_LOAD( "29lv640.u1", 0x0000000, 0x0800000, CRC(5be4c27d) SHA1(f16bb283e7d28148efacae7d42091985d96825b8) )
	ROM_LOAD( "29lv640.u2", 0x0800000, 0x0800000, CRC(1dd17d97) SHA1(a8e3b9f47bc8cf85f1008e16223245bbe6dc7f79) )
	ROM_LOAD( "29lv640.u3", 0x1000000, 0x0800000, CRC(970f6dd2) SHA1(b5db6c64e3ad6c8b0c0014b9cbdc2efadeb9b900) )
	ROM_LOAD( "29lv640.u4", 0x1800000, 0x0800000, CRC(d92453da) SHA1(612aacbb6724c195cf28ed25a73a6220c2e1fc32) )
	ROM_LOAD( "29lv640.u5", 0x2000000, 0x0800000, CRC(113218d6) SHA1(1182fe1ce29fbbbbfdd7ea0a5f6f6f85eab7acc6) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u6", 0x2800000, 0x0800000, CRC(e8e06adb) SHA1(4c7871c405d6caa5ab5516410f730017c713984d) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u7", 0x3000000, 0x0800000, CRC(ef777222) SHA1(5a53c72584c1a8b098df085bade866d887f48b29) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u8", 0x3800000, 0x0800000, CRC(a48acc31) SHA1(4d35fd060dc86a10bd69715e17a9399ff97e4343) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "29lv640.u53", 0x000000, 0x800000, CRC(c891b5ff) SHA1(5e0dce5b33230bd181f3eed94f64da328d11be28) )
ROM_END

} // anonymous namespace

GAME ( 2009,  pengprty, 0, hummer, hummer, hummer_state, empty_init, ROT0, "Astro Corp.", "Penguin Party",                  MACHINE_IS_SKELETON )
GAME ( 2012,  jackvent, 0, hummer, hummer, hummer_state, empty_init, ROT0, "Astro Corp.", "Jack's Venture - Inca Treasure", MACHINE_IS_SKELETON )
