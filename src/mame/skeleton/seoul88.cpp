// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************

 Skeleton driver for "Seoul 88 Fever", a Korean video slot from Mecca.

  ____________________________________________________________________________________________________
 | ____      ____________________    ____________   _________   ________   _________  _________       |
 ||BATT|    | ZILOG Z0840006PSC  |  | ROM U02    | |PAL22V10_| |74LS245N| |__DIPSx8_||__DIPSx8_|      |
 ||____|    |____________________|  |____________|                                                    |
 |                                                   ________   ________                           ___|
 |                                   ____________   |74LS273N| |74LS245N|     ________   ___      |
 |  ___      ________   _________   |GM76C88AL-15|                           |74LS245N|  |()|     |___
 |  Xtal    |74LS157N| |HY18CV8S_|  |____________|   ________  _________                Switch      __|
 | 12.000                                           |74LS245N||PALCE22V10H    ________              __|
 |                                                                           |74LS245N|             __|
 | ________     ________      ________    ________   ________   ________                            __|
 ||74HCTLS04N  |74LS273N|    |74LS157N|  |N82S129N| |74LS245N| |74LS174N|     ________              __|
 |                                                                           |74LS245N|             __|
 |              ________                             ________   ________                            __|
 |             |74LS273N|                           |74LS245N| |N82S129N|     ________              __|
 |                                                                           |74LS273N|             __|
 | ________                      ______________      ________   ________                            __|
 ||74LS74AN|    ____________    | ACTEL        |    |74LS245N| |N82S129N|     ________              __|
 |             | 28F1000PPC |   | A40MX04-F    |                             |74LS245N|             __|
 | ________    |____________|   | PL84 0017    |     ________   ________                            __|
 ||74LS174AN                    |              |    |74LS273N| |N82S129N|     ________              __|
 |              ____________    |______________|                             |SN76489AN             __|
 | ________    | HY6264P-12 |                        ________   ________                            __|
 ||74LS157N|   |____________|                       |74LS273N| |N82S129N|                           __|
 |                                                                                                  __|
 | ________                                          ________   ________                           ___|
 ||74LS157N|    ____________     ____________       |74LS273N| |N82S129N|                         |
 |             | LH5168-10L |   | A277308-90 |                                                    |___
 | ________    |____________|   |____________|       ________   ________                              |
 ||74LS157N|                                        |74LS174N| |N82S129N|                             |
 |                                                                                                    |
 | ··      ________    ________    ________    ________    ________       ________   ________         |
 | ··    MC74HC574AN  |74HC174AN   AT89C2051  |74HC245P|  |74HC245P|     MC74HC74AN IN74HC174AN       |
 | ··                                                                                                 |
 | ··      ________    ________      ____      ________                   ________   ________         |
 | ··    MC74HC574AN  |ULN2003AN     Xtal     |_DIPSx8_|    Switch       |74HC174AN |ULN2003AN        |
 |                                  24.000                                                            |
 |____________________________________________________________________________________________________|


Seems derived from igs/goldstar.cpp.

***********************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class seoul88_state : public driver_device
{
public:
	seoul88_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_video_ram(*this, "video_ram"),
		m_attribute_ram(*this, "attribute_ram")
	{ }

	void seoul88(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_attribute_ram;

	tilemap_t *m_tilemap = nullptr;

	void video_ram_w(offs_t offset, uint8_t data);
	void attribute_ram_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void seoul88_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seoul88_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(seoul88_state::get_tile_info) // TODO
{
	int const code = m_video_ram[tile_index];
	int const attr = m_attribute_ram[tile_index];

	tileinfo.set(0, code | (attr & 0xf0) << 4, attr & 0x0f, 0);
}

void seoul88_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void seoul88_state::attribute_ram_w(offs_t offset, uint8_t data)
{
	m_attribute_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

uint32_t seoul88_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void seoul88_state::program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram(); // reels?
	map(0xd000, 0xd03f).ram(); // reels attr?
	map(0xd200, 0xd23f).ram(); // reels attr?
	map(0xd400, 0xd43f).ram(); // reels attr?
	map(0xd600, 0xd63f).ram(); // reels attr?
	map(0xe000, 0xe7ff).ram().w(FUNC(seoul88_state::video_ram_w)).share(m_video_ram);
	map(0xe800, 0xefff).ram().w(FUNC(seoul88_state::attribute_ram_w)).share(m_attribute_ram);
	map(0xf000, 0xffff).ram();
}

void seoul88_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x10, 0x10).portr("IN0");
	map(0x11, 0x11).portr("IN1");
	// map(0x1f, 0x1f).portr(""); some dips in test mode are read from here, but test mode doesn't seem to match what's on PCB
	map(0xb8, 0xb8).portr("DSW1");
	map(0xb9, 0xb9).portr("DSW2");
	map(0xba, 0xba).portr("DSW3");
}

static INPUT_PORTS_START( seoul88 ) // there are 3 8-dip banks on PCB but test mode shows 5 (probably remnant of other previous games)
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

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
INPUT_PORTS_END

const gfx_layout gfx_8x32x4 =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	32*8
};

static GFXDECODE_START( gfx_seoul88 )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_planar,    0, 16 )
	GFXDECODE_ENTRY( "reels",   0, gfx_8x32x4,        128,  8 )  // TODO: decode may be not 100% correct
GFXDECODE_END

void seoul88_state::seoul88(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 12.000_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &seoul88_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &seoul88_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(seoul88_state::irq0_line_hold));

	// AT89C2051 MCU
	//...

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seoul88_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_seoul88);
	PALETTE(config, "palette").set_entries(0x100); // TODO

	// Audio hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn", 12_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.30); // actually SN76489AN, clock not verified
}

ROM_START( seoul88 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am27c512.u02",        0x00000, 0x10000, CRC(890b2d38) SHA1(28e6f5d84c9b283ad565236747aca39bc00c0efb) )

	ROM_REGION( 0x4000, "mcu", 0 )
	ROM_LOAD( "at89c2051-24pc.u1",   0x0000, 0x4000, NO_DUMP ) // 2 Kbytes internal ROM

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "amic_a277308-90.u07", 0x00000, 0x20000, CRC(ea0aafdc) SHA1(232fc5a542d7b61f466e82bc0a8e14b3f2f81e1d) )

	ROM_REGION( 0x20000, "reels", 0 )
	ROM_LOAD( "mx_28f1000ppc.u43",   0x00000, 0x20000, CRC(b824f1c6) SHA1(a390e7cc4e5705770f4f8d9c604ad304982aabf8) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "88-s1.bin",           0x00000, 0x00100, CRC(a18f1b83) SHA1(6ea1980c5f686933ae05922671e1d7c9561ba62a) )
	ROM_LOAD( "88-s2.bin",           0x00100, 0x00100, CRC(452e6591) SHA1(83c16fdc839e634bada4f754f582806c207fe2f1) )
	ROM_LOAD( "88-s3.bin",           0x00200, 0x00100, CRC(c242965a) SHA1(0654f38af98d906e8e993907bb2e8f1084937092) )
	ROM_LOAD( "88-s4.bin",           0x00300, 0x00100, CRC(1e4cc1ad) SHA1(2d93d267320525c44d9eb4ee17cf8ebf69842cb4) )
	ROM_LOAD( "88-s5.bin",           0x00400, 0x00100, CRC(67a6b674) SHA1(5a810c5e9da71dc6465ff843c55f61bd321e0b1e) )
	ROM_LOAD( "88-s6.bin",           0x00500, 0x00100, CRC(11c88e29) SHA1(16b07fe6a83b3a300fdb081609729595c16588e9) )
	ROM_LOAD( "88-s7.bin",           0x00600, 0x00100, CRC(83c3ec8f) SHA1(4a6452ef73061a446e6a8ceb9d077bc71cc8e2b2) )

	ROM_REGION( 0x09ae, "plds", 0 )
	ROM_LOAD( "hy18cv8s-25.u10",     0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "pal22v10-7pc.u18",    0x00117, 0x002dd, NO_DUMP )
	ROM_LOAD( "pal22v10-25pc.u17",   0x006d1, 0x002dd, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY  FULLNAME          FLAGS
GAME( 1989, seoul88, 0,      seoul88, seoul88, seoul88_state, empty_init, ROT0, "Mecca", "Seoul 88 Fever", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
