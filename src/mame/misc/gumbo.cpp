// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Gumbo (c)1994 Min Corp (Main Corp written on PCB)
Miss Bingo (c)1994 Min Corp
Miss Puzzle (c)1994 Min Corp

argh they have the same music as news (misc/news.cpp)

*/

/* working notes (Gumbo)

68k interrupts
lev 1 : 0x64 : 0000 0142 -
lev 2 : 0x68 : 0000 0142 -
lev 3 : 0x6c : 0000 0142 -
lev 4 : 0x70 : 0000 0142 -
lev 5 : 0x74 : 0000 0142 -
lev 6 : 0x78 : 0000 0142 -
lev 7 : 0x7c : 0000 0142 -

PCB Layout
----------

|---------------------------------|
|                                 |
|   M6295    U210     6264   U512 |
|                     6264   U511 |
|  ACTEL                          |
|J A1020A   14.31818MHz           |
|A                                |
|M DSW1        6116               |
|M             6116               |
|A                                |
|                            6116 |
|                            6116 |
| 6264  6264                      |
|  U1    U2                       |
|  68000P10           U421   U420 |
|---------------------------------|

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gumbo_state : public driver_device
{
public:
	gumbo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void mspuzzle(machine_config &config);
	void dblpoint(machine_config &config);
	void gumbo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dblpoint_map(address_map &map) ATTR_COLD;
	void gumbo_map(address_map &map) ATTR_COLD;
	void mspuzzle_map(address_map &map) ATTR_COLD;
};



void gumbo_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	tileinfo.set(0, tileno, 0, 0);
}


void gumbo_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index];
	tileinfo.set(1, tileno, 1, 0);
}


void gumbo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gumbo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gumbo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);
	m_fg_tilemap->set_transparent_pen(0xff);
}

uint32_t gumbo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void gumbo_state::gumbo_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram(); // main ram
	map(0x1b0000, 0x1b03ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x1c0100, 0x1c0101).portr("P1_P2");
	map(0x1c0200, 0x1c0201).portr("DSW");
	map(0x1c0301, 0x1c0301).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1e0000, 0x1e0fff).ram().w(FUNC(gumbo_state::bg_videoram_w)).share(m_bg_videoram); // bg tilemap
	map(0x1f0000, 0x1f3fff).ram().w(FUNC(gumbo_state::fg_videoram_w)).share(m_fg_videoram); // fg tilemap
}

// Miss Puzzle has a different memory map

void gumbo_state::mspuzzle_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); // main ram
	map(0x190000, 0x197fff).ram().w(FUNC(gumbo_state::fg_videoram_w)).share(m_fg_videoram); // fg tilemap
	map(0x1a0000, 0x1a03ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x1b0100, 0x1b0101).portr("P1_P2");
	map(0x1b0200, 0x1b0201).portr("DSW");
	map(0x1b0301, 0x1b0301).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1c0000, 0x1c1fff).ram().w(FUNC(gumbo_state::bg_videoram_w)).share(m_bg_videoram); // bg tilemap
}

void gumbo_state::dblpoint_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x083fff).ram(); // main ram
	map(0x1b0000, 0x1b03ff).w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x1c0100, 0x1c0101).portr("P1_P2");
	map(0x1c0200, 0x1c0201).portr("DSW");
	map(0x1c0301, 0x1c0301).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1e0000, 0x1e3fff).ram().w(FUNC(gumbo_state::fg_videoram_w)).share(m_fg_videoram); // fg tilemap
	map(0x1f0000, 0x1f0fff).ram().w(FUNC(gumbo_state::bg_videoram_w)).share(m_bg_videoram); // bg tilemap
}

static INPUT_PORTS_START( gumbo )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Rotate" - also IPT_START1
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Help"
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Rotate" - also IPT_START2
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Help"
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Helps" )         PORT_DIPLOCATION("SW1:3")   // "Power Count" in test mode
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPNAME( 0x0800, 0x0800, "Bonus Bar Level" )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Picture View" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( msbingo )
	PORT_INCLUDE( gumbo )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c00, 0x0c00, "Chance Count" )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0c00, "0" )
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x1000, 0x1000, "Play Level" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Play Speed" )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Left Count" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( mspuzzle )
	PORT_INCLUDE( gumbo )

	PORT_MODIFY("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0200, "Time Mode" )     PORT_DIPLOCATION("SW1:1,2") // Manual lists this as "Game Level" with Levels 1 through 4
	PORT_DIPSETTING(      0x0300, "0" )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Test" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "View Staff Credits" )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Picture View" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( dblpoint )
	PORT_INCLUDE( gumbo )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c00, 0x0800, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0c00, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Test" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Picture View" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW1:7" )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static const gfx_layout gumbo_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2)+0, 8,RGN_FRAC(1,2)+8,  16,RGN_FRAC(1,2)+16,24,RGN_FRAC(1,2)+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout gumbo2_layout =
{
	4,4,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2)+0, 8,RGN_FRAC(1,2)+8 },
	{ 0*16, 1*16, 2*16, 3*16 },
	4*16
};

static GFXDECODE_START( gfx_gumbo )
	GFXDECODE_ENTRY( "bgtiles", 0, gumbo_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "fgtiles", 0, gumbo2_layout,  0x0, 2  )
GFXDECODE_END


void gumbo_state::gumbo(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(14'318'181) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gumbo_state::gumbo_map);
	m_maincpu->set_vblank_int("screen", FUNC(gumbo_state::irq1_line_hold)); // all the same

	GFXDECODE(config, m_gfxdecode, "palette", gfx_gumbo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gumbo_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);

	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(14'318'181) / 16, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 1);
}

void gumbo_state::mspuzzle(machine_config &config)
{
	gumbo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gumbo_state::mspuzzle_map);
}

void gumbo_state::dblpoint(machine_config &config)
{
	gumbo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &gumbo_state::dblpoint_map);
}

ROM_START( gumbo )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x20000, CRC(e09899e4) SHA1(b62876dc3ada8509b766a80f496f1227b6af0ced) )
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x20000, CRC(60e59acb) SHA1(dd11329374c8f63851ddf5af54c91f78fad4fd3d) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(16fbe06b) SHA1(4e40e62341dc886fcabdb07f64217dc086f43c67) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "u421.bin", 0x00000, 0x80000, CRC(42445132) SHA1(f29d09d040644c8ef12a1cfdfc0d066e8ed9b82d) ) // Nude & semi-nude girls, both real pictures & hand drawn
	ROM_LOAD( "u420.bin", 0x80000, 0x80000, CRC(de1f0e2f) SHA1(3f46d19af48392794838a4b54f8c45b809c67d49) )

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x20000, CRC(97741798) SHA1(3603e14511817da19f6819d5612728d333695e99) )
	ROM_LOAD( "u511.bin", 0x20000, 0x20000, CRC(1411451b) SHA1(941d5f311f727e3a8d41ecbbe35b687d48cc2cef) )
ROM_END

ROM_START( mspuzzleg ) // This version is a clone of Gumbo... NOT the other Miss Puzzle sets
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1", 0x00001, 0x20000, CRC(95218ff1) SHA1(9617d979d026872dbe68eaae21c3ab1f5f9f4bfd) ) // Korean bootleg / clone / hack???
	ROM_LOAD16_BYTE( "u2", 0x00000, 0x20000, CRC(7ea7d96c) SHA1(17b9afb3214a07b1af5913f1926c7aeac27ea0e8) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(16fbe06b) SHA1(4e40e62341dc886fcabdb07f64217dc086f43c67) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421", 0x100000, 0x80000, CRC(afa06a93) SHA1(5c77b7aafd07d19a1eedf22858a00766a3e37389) ) // Non-nude graphics, hand drawn girls & animals
	ROM_LOAD( "u420", 0x000000, 0x80000, CRC(2b387153) SHA1(c36f93d4f3a7ea8af58babeb33250d726067a35d) )

	ROM_REGION( 0x40000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x20000, CRC(97741798) SHA1(3603e14511817da19f6819d5612728d333695e99) )
	ROM_LOAD( "u511.bin", 0x20000, 0x20000, CRC(1411451b) SHA1(941d5f311f727e3a8d41ecbbe35b687d48cc2cef) )
ROM_END

ROM_START( msbingo )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x20000, CRC(6eeb6d89) SHA1(d3e8870a2e95a1ee1c0ce80995c902a97b25a85c) )
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x20000, CRC(f15dd4b5) SHA1(b49713e92f11f8c603f561e071df9ffb838c8795) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(55011f69) SHA1(47a27151e6f9ecbc49a95cd8fb4bc627c3efde46) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421.bin", 0x000000, 0x80000, CRC(b73f21ab) SHA1(9abb940c0b489fb59f5377516f8d3552cb286c73) )
	ROM_LOAD( "u420.bin", 0x100000, 0x80000, CRC(c2fe9175) SHA1(1cd3afd77325721e45362cf4b7d992538e427c24) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, CRC(8a46d467) SHA1(23ef7a6c25bb30a993bd796ed9b60da0f6f0d443) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(d5fd3e2e) SHA1(9f805bff62f884b2b35c88c2da016bf6264d2ab6) )
ROM_END

ROM_START( mspuzzle )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x40000, CRC(d9e63f12) SHA1(c826c604f101d68057fdebf1b231293e4b2811f0) ) // identical halves
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x40000, CRC(9c3fc677) SHA1(193606fe739dbf5f26962f91be968ca371b7fd74) ) // identical halves

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(0a223a38) SHA1(e5aefbdbb09c18cc230bc852df3ea1defb1a21a8) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421.bin", 0x000000, 0x80000, CRC(5387ab3a) SHA1(69913fde1a323ab1356ef52bb4efbf12caed594c) )
	ROM_LOAD( "u420.bin", 0x100000, 0x80000, CRC(c3f892e6) SHA1(5e8e4ae45a0eebaf2bbad00b1208b68f3e81df0c) )
	ROM_LOAD( "u425.bin", 0x080000, 0x80000, CRC(f53a9042) SHA1(70fcc3aaef46282a888466454714dc59daeb174d) )
	ROM_LOAD( "u426.bin", 0x180000, 0x80000, CRC(c927e8da) SHA1(2219f99bce6b2b9a827177c83952813df1a32c72) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, CRC(505ee3c2) SHA1(a719958c34d9c54445ad207bca1f49df3aff938b) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(3d6b6c78) SHA1(3016423102b4d47c0f1296471cf1670258acc856) )
ROM_END

ROM_START( mspuzzlea ) // sticker on PCB stated:  MISS PUZZLE  V ..8
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1.u1", 0x00001, 0x40000, CRC(5e96ea17) SHA1(9b4cfa32e386eacca4e65190d29978cf9cf08fbc) ) // identical halves
	ROM_LOAD16_BYTE( "u2.u2", 0x00000, 0x40000, CRC(8f161b0c) SHA1(783dbb34ae5fb7603558c17a39abbe0a93e3a01f) ) // identical halves

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.bin", 0x00000, 0x40000, CRC(0a223a38) SHA1(e5aefbdbb09c18cc230bc852df3ea1defb1a21a8) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421.u421", 0x000000, 0x80000, CRC(52b67ee5) SHA1(209fdf845c5ef2e65fbc0de8aeb26562b6d033ab) ) // several more explicit pictures have been replaced compared to the set below
	ROM_LOAD( "u420.u420", 0x100000, 0x80000, CRC(3565696e) SHA1(5234dc25f73e6245b199071fa3a9c0d2367cae9f) ) // Starting with 204 & 205, as well as many in 300 range
	ROM_LOAD( "u425.u425", 0x080000, 0x80000, CRC(933544e3) SHA1(d77b1050c4b56d0fd3be703bec961863d5188484) )
	ROM_LOAD( "u426.u426", 0x180000, 0x80000, CRC(e458eb9d) SHA1(bead05ec57d97c86ed727b2c4b95f8ffdbd7ef4c) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, CRC(505ee3c2) SHA1(a719958c34d9c54445ad207bca1f49df3aff938b) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(3d6b6c78) SHA1(3016423102b4d47c0f1296471cf1670258acc856) )
ROM_END

ROM_START( mspuzzleb )
	// all the ROMs for this set needs a full redump, the PCB was in pretty bad condition and data reads were not consistent
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
//  ROM_LOAD16_BYTE( "u1.rom", 0x00001, 0x20000, BAD_DUMP CRC(ec940df4) SHA1(20bb6e2757868cf8fbbb11e05adf8c1d625ee172) )
//  ROM_LOAD16_BYTE( "u2.rom", 0x00000, 0x20000, BAD_DUMP CRC(7b9cac82) SHA1(c5edfb3fbdf43219ba317c18222e671ebed94469) )
	ROM_LOAD16_BYTE( "u1.u1", 0x00001, 0x40000, CRC(5e96ea17) SHA1(9b4cfa32e386eacca4e65190d29978cf9cf08fbc) ) // identical halves - use program ROMs from mspuzzlea for now
	ROM_LOAD16_BYTE( "u2.u2", 0x00000, 0x40000, CRC(8f161b0c) SHA1(783dbb34ae5fb7603558c17a39abbe0a93e3a01f) ) // identical halves - use program ROMs from mspuzzlea for now

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.rom", 0x00000, 0x40000, CRC(8826b018) SHA1(075e5cef114146c6c72c0331dd3434b27fed180d) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421.rom", 0x000000, 0x80000, BAD_DUMP CRC(3c567c55) SHA1(100e0c9535bf07f3ca8537b3a172486b65e5f24a) )
	// 0x000000-0x3FFFF of u421.rom == 0x000000-0x3FFFF of u421 in mspuzzlea, all other data is corrupt to some degree
	ROM_LOAD( "u420.rom", 0x100000, 0x80000, BAD_DUMP CRC(f52ab7fd) SHA1(e5b8905cae3e15a8a379c4c149441d849931cbde) ) // pictures 204 & 205 are corrupt (as shown in score box as 002040 & 002050)
	ROM_LOAD( "u425.rom", 0x080000, 0x80000, BAD_DUMP CRC(1c4c8fc1) SHA1(90e3f297db68a44cba0966b599bb7c593eced16e) ) // Title screen and Course Select graphics & girls
	ROM_LOAD( "u426.rom", 0x180000, 0x80000, BAD_DUMP CRC(c28b2743) SHA1(df4bf998ae17ddebf1b4047564eb296c69bc9071) ) // Every other line of girls matched with U425

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, CRC(505ee3c2) SHA1(a719958c34d9c54445ad207bca1f49df3aff938b) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(3d6b6c78) SHA1(3016423102b4d47c0f1296471cf1670258acc856) )
ROM_END

ROM_START( dblpoint )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1.bin", 0x00001, 0x20000, CRC(b05c9e02) SHA1(40ae2926cc4a77e8f871e3a4845314384a15c3e0) )
	ROM_LOAD16_BYTE( "u2.bin", 0x00000, 0x20000, CRC(cab35cbe) SHA1(63a35a880c962a9c9560bf779bf9edec18c3878d) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "u210.rom", 0x00000, 0x40000, CRC(d35f975c) SHA1(03490c92afadbd24c5b75f0ab114a2681b65c10e) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "u421.bin", 0x000000, 0x80000, CRC(b0e9271f) SHA1(4873e4dda177f5116164b2a47dabd82bc75e9bdf) )
	ROM_LOAD( "u420.bin", 0x100000, 0x80000, CRC(252789e8) SHA1(7b365035a4c4f6aae0d4075db70d59973569b12b) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "u512.bin", 0x00000, 0x40000, CRC(b57b8534) SHA1(1d96dc93111e56a7982c3602864a71a785a4782a) )
	ROM_LOAD( "u511.bin", 0x40000, 0x40000, CRC(74ed13ff) SHA1(2522bd5fe40123a5b07e955252ae96b913a3ac0d) )
ROM_END

// based on the labels this doesn't seem to be an original Min Corp. board
ROM_START( dblpointd )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "d12.bin", 0x00001, 0x20000, CRC(44bc1bd9) SHA1(8b72909c53b09b9287bf90bcd8970bdf9c1b8798) )
	ROM_LOAD16_BYTE( "d13.bin", 0x00000, 0x20000, CRC(625a311b) SHA1(38fa0d240b253fcc8dc89438582a9c446410b636) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "d11.bin", 0x00000, 0x40000, CRC(d35f975c) SHA1(03490c92afadbd24c5b75f0ab114a2681b65c10e) )

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD( "d16.bin", 0x00000, 0x80000, CRC(afea0158) SHA1(dc97f9268533048690715f377fb35d70e7e5a53f) )
	ROM_LOAD( "d17.bin", 0x80000, 0x80000, CRC(c971dcb5) SHA1(40f15b3d61ea0325883f19f24f2b61e24bb12a98) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "d14.bin", 0x00000, 0x40000, CRC(41943db5) SHA1(2f245402f7bbaeca7e50161397ee45e7c7c90cfc) )
	ROM_LOAD( "d15.bin", 0x40000, 0x40000, CRC(6b899a51) SHA1(04114ec9695caaac722800ac1a4ffb563ec433c9) )
ROM_END

} // anonymous namespace


GAME( 1994, gumbo,     0,        gumbo,    gumbo,    gumbo_state, empty_init, ROT0,  "Min Corp.",                     "Gumbo",                                       MACHINE_SUPPORTS_SAVE )
GAME( 1994, mspuzzleg, gumbo,    gumbo,    gumbo,    gumbo_state, empty_init, ROT0,  "Min Corp.",                     "Miss Puzzle (Clone of Gumbo)",                MACHINE_SUPPORTS_SAVE )
GAME( 1994, msbingo,   0,        mspuzzle, msbingo,  gumbo_state, empty_init, ROT0,  "Min Corp.",                     "Miss Bingo",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1994, mspuzzle,  0,        mspuzzle, mspuzzle, gumbo_state, empty_init, ROT90, "Min Corp.",                     "Miss Puzzle",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1994, mspuzzlea, mspuzzle, mspuzzle, mspuzzle, gumbo_state, empty_init, ROT90, "Min Corp.",                     "Miss Puzzle (Nudes, less explicit)",          MACHINE_SUPPORTS_SAVE )
GAME( 1994, mspuzzleb, mspuzzle, mspuzzle, mspuzzle, gumbo_state, empty_init, ROT90, "Min Corp.",                     "Miss Puzzle (Nudes, more explicit)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, dblpoint,  0,        dblpoint, dblpoint, gumbo_state, empty_init, ROT0,  "Min Corp.",                     "Double Point",                                MACHINE_SUPPORTS_SAVE )
GAME( 1995, dblpointd, dblpoint, dblpoint, dblpoint, gumbo_state, empty_init, ROT0,  "bootleg? (Dong Bang Electron)", "Double Point (Dong Bang Electron, bootleg?)", MACHINE_SUPPORTS_SAVE )
