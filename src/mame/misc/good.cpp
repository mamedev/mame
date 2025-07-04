// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
  'Good' Driver by David Haywood
  todo: finish inputs etc.


+----------------------------+
|    voice.rom  6116         |
|      M6295    6116         |
|J             HY6264        |
|A             HY6264 grp-01 |
|M DSW1 DSW2 A40MX04  grp-02 |
|M                    grp-03 |
|A IS61C256x2         grp-04 |
| s1 s2                      |
| 68000      16MHz 12MHz     |
+----------------------------+

Motorola MC68000P8
OKI M6295 (badged as AD-65)
Actel A40MX04-F PL84

Silk screened under the roms:

  system1 - SYSTEM 1
  system2 - SYSTEM 2
   grp-01 - GRP-01
   grp-02 - GRP-02
   grp-03 - GRP-03
   grp-04 - GRP-04
voice.rom - VOICE ROM
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class good_state : public driver_device
{
public:
	good_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_tilemapram(*this, "fg_tilemapram"),
		m_bg_tilemapram(*this, "bg_tilemapram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	void good(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_fg_tilemapram;
	required_shared_ptr<uint16_t> m_bg_tilemapram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	void fg_tilemapram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_tilemapram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_good(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	void good_map(address_map &map) ATTR_COLD;
};


void good_state::fg_tilemapram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_tilemapram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(good_state::get_fg_tile_info)
{
	int tileno = m_fg_tilemapram[tile_index * 2];
	int attr = m_fg_tilemapram[tile_index * 2 + 1] & 0xf;
	tileinfo.set(0, tileno, attr, 0);
}

void good_state::bg_tilemapram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_tilemapram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(good_state::get_bg_tile_info)
{
	int tileno = m_bg_tilemapram[tile_index * 2];
	int attr = m_bg_tilemapram[tile_index * 2 + 1] & 0xf;
	tileinfo.set(1, tileno, attr, 0);
}



void good_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(good_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(good_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap->set_transparent_pen(0xf);
}

uint32_t good_state::screen_update_good(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void good_state::good_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	//map(0x270000, 0x270007).ram(); // scroll?
	map(0x270001, 0x270001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x280000, 0x280001).portr("IN0");
	map(0x280002, 0x280003).portr("IN1");
	map(0x280004, 0x280005).portr("IN2");

	map(0x800000, 0x8007ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");

	map(0x820000, 0x820fff).ram().w(FUNC(good_state::fg_tilemapram_w)).share("fg_tilemapram");
	map(0x822000, 0x822fff).ram().w(FUNC(good_state::bg_tilemapram_w)).share("bg_tilemapram");

	map(0xff0000, 0xffefff).ram();
}

static INPUT_PORTS_START( good )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "1" )
	PORT_DIPSETTING(  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )

	// The following appears to be DSW
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds) )
	PORT_DIPSETTING(  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, "Credits at Start" )
	PORT_DIPSETTING(  0x000e, "50"  )
	PORT_DIPSETTING(  0x000c, "60"  )
	PORT_DIPSETTING(  0x000a, "70"  )
	PORT_DIPSETTING(  0x0008, "80"  )
	PORT_DIPSETTING(  0x0006, "100" )
	PORT_DIPSETTING(  0x0004, "120" )
	PORT_DIPSETTING(  0x0002, "150" )
	PORT_DIPSETTING(  0x0000, "200" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Double Up Test Mode" )
	PORT_DIPSETTING(  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(  0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(  0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(  0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Mature Content" )
	PORT_DIPSETTING(  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Maximum Bet" )
	PORT_DIPSETTING(  0x2000, "100" )
	PORT_DIPSETTING(  0x0000, "150" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout good_layout2 =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0,1,2,3,4,5,6,7, 256,257,258,259,260,261,262,263 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16  },
	32*16
};


static GFXDECODE_START( gfx_good )
	GFXDECODE_ENTRY( "gfx1", 0, good_layout2,  0x100, 16  ) // fg tiles
	GFXDECODE_ENTRY( "gfx1", 0, good_layout2,  0x200, 16  ) // bg tiles
GFXDECODE_END


void good_state::good(machine_config &config)
{
	M68000(config, m_maincpu, 16000000 /2);
	m_maincpu->set_addrmap(AS_PROGRAM, &good_state::good_map);
	m_maincpu->set_vblank_int("screen", FUNC(good_state::irq2_line_hold));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_good);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*16, 32*16);
	screen.set_visarea(1*16, 23*16-1, 0*16, 14*16-1);
	screen.set_screen_update(FUNC(good_state::screen_update_good));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x400);

	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 1);
}


ROM_START( good )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "system1", 0x00001, 0x10000, CRC(128374cb) SHA1(a6521f506a6e4f8e62936ec0e66b080106a43f36) )
	ROM_LOAD16_BYTE( "system2", 0x00000, 0x10000, CRC(c4eada4e) SHA1(2d9875487626796db8633520625ad6ad642723ef) )

	ROM_REGION( 0x040000, "oki", 0 ) // Samples
	ROM_LOAD( "voice.rom", 0x00000, 0x40000, CRC(a5a23482) SHA1(51ca69589086c1da44d64575ee9a4da7b88ba669) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "grp-01", 0x00000, 0x20000, CRC(33f8458e) SHA1(7f0c5fb44e3350c579f2dea82f0ec1d2ac5967ff) )
	ROM_LOAD16_BYTE( "grp-02", 0x00001, 0x20000, CRC(c0b98e6c) SHA1(fdafced2418feeec0ed3d87bbbc88d5aa28f380a) )
	ROM_LOAD16_BYTE( "grp-03", 0x40000, 0x20000, CRC(41da3bf4) SHA1(e7d1973951d4470fd2e0fa3c4690633219b71c06) )
	ROM_LOAD16_BYTE( "grp-04", 0x40001, 0x20000, CRC(83dbbb52) SHA1(e597f3cbb54b5cdf2230ea6318f970319061e31b) )
ROM_END

} // Anonymous namespace


GAME( 1998, good, 0, good, good, good_state, empty_init, ROT0,  "<unknown>", "Good (Korea)", MACHINE_SUPPORTS_SAVE )
