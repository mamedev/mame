// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Seletron / Olympia Long Beach
Olympia probably took care of distribution, PCB and game by Seletron

Hardware notes:
- 16MHz XTAL, MC6800P @ 500kHz
- 2x 5101 sram 256x4bit (256 byte) battery backed
- 4x 4045 sram 1kx4 (2K byte)

6800 hits many illegal opcodes (0x02), though it's harmless. Maybe they meant
to inserts nops (0x01) to remove debug stuff or to make room for future additions?

Speed Race is on the same hardware, no mention of Olympia, but it is listed
online on Olympia game lists. It is a reimagination of Taito's Speed Race,
not a bootleg.

Speed Race PCB where this dump came from had an incorrect CPU in place (MC68B09)
maybe from a wrong repair. PCB label: Seletron, 10-50058. Less ROM data than lbeach.

TODO:
- discrete sound
- unknown writes (most of it is sound)
- improve colors, lbeach is monochrome but what about speedrs?
- is steering wheel analog? eg. with pulses to d7
- some unknown romlabels (see "x")
- lbeach 93448.h4 rom is a bad dump? it misses animation frames compared to
  speedrs, but when compared to enemy cars, it looks ok
- speedrs has nvram? high scores are not saved
- sprite x position is wrong, but moving it to the left will break speedrs
  (it does a sprite collision check at boot)
- right side of screen should not be rendered? but speedrs sprite collision
  check wouldn't work then. Video recording(pcb to monitor, not cabinet)
  of lbeach shows right side is cropped - it is handled in the .lay file

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include "lbeach.lh"
#include "speedrs.lh"


namespace {

class lbeach_state : public driver_device
{
public:
	lbeach_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bg_vram(*this, "bg_vram"),
		m_fg_vram(*this, "fg_vram"),
		m_scroll_y(*this, "scroll_y"),
		m_sprite_x(*this, "sprite_x"),
		m_sprite_code(*this, "sprite_code"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	ioport_value col_bg_r() { return m_collision_bg_car; }
	ioport_value col_fg_r() { return m_collision_fg_car; }

	void lbeach(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_bg_vram;
	required_shared_ptr<u8> m_fg_vram;
	required_shared_ptr<u8> m_scroll_y;
	required_shared_ptr<u8> m_sprite_x;
	required_shared_ptr<u8> m_sprite_code;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_collision_bg_car = 0;
	int m_collision_fg_car = 0;

	bitmap_ind16 m_colmap_car;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void bg_vram_w(offs_t offset, u8 data);
	void fg_vram_w(offs_t offset, u8 data);

	void init_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};

void lbeach_state::machine_start()
{
	save_item(NAME(m_collision_bg_car));
	save_item(NAME(m_collision_fg_car));
}



/******************************************************************************
    Video
******************************************************************************/

void lbeach_state::init_palette(palette_device &palette) const
{
	// tiles
	palette.set_pen_color(0, 0x00, 0x00, 0x00);
	palette.set_pen_color(1, 0xc0, 0xc0, 0xc0);

	// road
	palette.set_pen_color(2, 0x00, 0x00, 0x00);
	palette.set_pen_color(3, 0xff, 0xff, 0xff);

	palette.set_pen_color(4, 0x80, 0x80, 0x80);
	palette.set_pen_color(5, 0xff, 0xff, 0xff);

	palette.set_pen_color(6, 0x00, 0x00, 0x00);
	palette.set_pen_color(7, 0x80, 0x80, 0x80);

	palette.set_pen_color(8, 0x80, 0x80, 0x80);
	palette.set_pen_color(9, 0xff, 0xff, 0xff);

	// player car
	palette.set_pen_color(10, 0x00, 0x00, 0x00);
	palette.set_pen_color(11, 0xff, 0xff, 0xff);
}


TILE_GET_INFO_MEMBER(lbeach_state::get_bg_tile_info)
{
	// d0-d4: code
	// d5: unused?
	// d6,d7: color
	u8 code = m_bg_vram[tile_index];
	tileinfo.set(1, code & 0x1f, code >> 6 & 3, 0);
}

TILE_GET_INFO_MEMBER(lbeach_state::get_fg_tile_info)
{
	u8 code = m_fg_vram[tile_index];
	tileinfo.set(0, code, 0, 0);
}

void lbeach_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lbeach_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lbeach_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_screen->register_screen_bitmap(m_colmap_car);
}


u32 lbeach_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw bg layer (road)
	m_bg_tilemap->set_scrolly(0, *m_scroll_y);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// check collision
	int sprite_code = *m_sprite_code & 0xf;
	int sprite_x = *m_sprite_x * 2 + 2;
	int sprite_y = 160;

	m_colmap_car.fill(0, cliprect);
	m_gfxdecode->gfx(2)->transpen(m_colmap_car, cliprect, sprite_code, 0, 0, 0, sprite_x, sprite_y, 0);
	bitmap_ind16 &fg_bitmap = m_fg_tilemap->pixmap();

	m_collision_bg_car = 0;
	m_collision_fg_car = 0;

	for (int y = sprite_y; y < (sprite_y + 16); y++)
	{
		for (int x = sprite_x; x < (sprite_x + 16) && cliprect.contains(x, y); x++)
		{
			m_collision_bg_car |= (bitmap.pix(y, x) & m_colmap_car.pix(y, x) & 1);
			m_collision_fg_car |= (fg_bitmap.pix(y, x) & m_colmap_car.pix(y, x) & 1);
		}
	}

	// draw fg layer (tiles)
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw player car
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, sprite_code, 0, 0, 0, sprite_x, sprite_y, 0);
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, sprite_code | 0x10, 0, 0, 0, sprite_x, sprite_y + 16, 0);

	return 0;
}


void lbeach_state::bg_vram_w(offs_t offset, u8 data)
{
	m_bg_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lbeach_state::fg_vram_w(offs_t offset, u8 data)
{
	m_fg_vram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void lbeach_state::main_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("nvram");
	map(0x4000, 0x41ff).ram().w(FUNC(lbeach_state::bg_vram_w)).share("bg_vram");
	map(0x4000, 0x4000).portr("IN.0");
	map(0x4200, 0x43ff).ram();
	map(0x4400, 0x47ff).ram().w(FUNC(lbeach_state::fg_vram_w)).share("fg_vram");
	map(0x8000, 0x8000).portr("IN.1");
	map(0x8000, 0x8000).writeonly().share("scroll_y");
	map(0x8001, 0x8001).writeonly().share("sprite_x");
	map(0x8002, 0x8002).writeonly().share("sprite_code");
//  map(0x8003, 0x8003).nopw(); // ?
//  map(0x8004, 0x8004).nopw(); // ?
//  map(0x8005, 0x8005).nopw(); // ?
	map(0x8007, 0x8007).nopw(); // probably watchdog
	map(0xa000, 0xa000).portr("IN.2");
//  map(0xa003, 0xa003).nopr(); // ? tests d7 at game over
	map(0xc000, 0xcfff).rom();
	map(0xf000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( lbeach )
	PORT_START("IN.0")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, 0x40, IPT_CUSTOM ) PORT_CONDITION("STEER", 0x02, EQUALS, 0x00) // steering wheel direction
	PORT_BIT( 0x80, 0x80, IPT_CUSTOM ) PORT_CONDITION("STEER", 0x01, EQUALS, 0x00) // steering wheel active

	PORT_START("IN.1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) // dupe
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x04, "Refueling Point" )
	PORT_DIPSETTING(    0x00, "200 km" )
	PORT_DIPSETTING(    0x04, "250 km" )
	PORT_DIPSETTING(    0x08, "300 km" )
	PORT_DIPSETTING(    0x0c, "350 km" )
	PORT_DIPNAME( 0x30, 0x20, "Fuel Drop Rate" )
	PORT_DIPSETTING(    0x30, "1" ) // slow
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" ) // fast
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(lbeach_state, col_fg_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(lbeach_state, col_bg_r)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas Pedal")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shifter 1st Gear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shifter 2nd Gear")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter 3rd Gear")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Shifter 4th Gear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Record") // called RR in testmode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Coin Counter") // called CC in testmode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("STEER")
	PORT_BIT( 0x03, 0x01, IPT_PADDLE ) PORT_MINMAX(0x00,0x02) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CENTERDELTA(1)
INPUT_PORTS_END

static INPUT_PORTS_START( speedrs )
	PORT_INCLUDE( lbeach )

	PORT_MODIFY("IN.1")
	PORT_DIPNAME( 0x0c, 0x04, "Refueling Point" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "2500" )
	PORT_DIPSETTING(    0x0c, "3000" )

	PORT_MODIFY("IN.2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x7c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/******************************************************************************
    GFX Layouts
******************************************************************************/

static const gfx_layout layout_16x8 =
{
	16,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout layout_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_lbeach )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x8, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16, 2, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16, 10, 1 )
GFXDECODE_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void lbeach_state::lbeach(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 16_MHz_XTAL / 32); // Motorola MC6800P, 500kHz
	m_maincpu->set_addrmap(AS_PROGRAM, &lbeach_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60); // measured ~60Hz
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 511-32, 0, 255-24);
	m_screen->set_screen_update(FUNC(lbeach_state::screen_update));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // needed for collision detection
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lbeach);
	PALETTE(config, m_palette, FUNC(lbeach_state::init_palette), 2 + 8 + 2);

	/* sound hardware */
	// ...
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( lbeach )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "x.c1",   0xc000, 0x0800, CRC(767fc287) SHA1(da8d59c4827b8479064084e14fa8a7bf71a65293) )
	ROM_LOAD( "c2.c2",  0xc800, 0x0800, CRC(9f30e0de) SHA1(607873566fd330486d2bc22cabd5623c021b1dc8) )
	ROM_LOAD( "f03.c3", 0xf000, 0x0800, CRC(6728aa45) SHA1(7f41d52f1e63cafd82ad0853a66ecbbdbee070b3) )
	ROM_LOAD( "f47.c4", 0xf800, 0x0800, CRC(950f3154) SHA1(e8bd1a818fe293163466169b65e07ac635ddfdb1) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "15.e2",  0x0000, 0x0800, CRC(9f24897c) SHA1(5ae05b06dadf4e935e39ac289f2db7719ff3b230) )
	ROM_LOAD( "s2.d2",  0x0800, 0x0800, CRC(c5d59466) SHA1(663df8397081daa89f9e3f7aec7e07557cf5b310) )

	ROM_REGION( 0x0400, "gfx2", 0 ) // road
	ROM_LOAD( "r.d1",   0x0000, 0x0400, CRC(07e99395) SHA1(67a68dee8e97ae74ab13af62cdc5279c358dc897) )

	ROM_REGION( 0x0400, "gfx3", ROMREGION_ERASE00 )
	ROM_LOAD( "93448.h4", 0x0000, 0x0200, BAD_DUMP CRC(0dca040d) SHA1(d775cbf9fbb9449881ce4997187a1945d34d3cb6) ) // stuck a5
ROM_END

ROM_START( speedrs )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "40.y3",  0xc000, 0x0400, CRC(237a8593) SHA1(7f56d32a3f9ddead1eb0170807bbc0221e2d6b9a) )
	ROM_LOAD( "x.y4",   0xc400, 0x0400, CRC(4e489c42) SHA1(3efeac5a194f524c5b4ae61517781e63ab7fbb5a) )
	ROM_LOAD( "44.x6",  0xf000, 0x0400, CRC(37d4fd97) SHA1(f7555264db1a4a1caa429aa75239b997c4635fcd) )
	ROM_LOAD( "45a.x7", 0xf400, 0x0400, CRC(e8e3854a) SHA1(79309633a3f3d19f2d2eaf92f745512d7e007ca1) )
	ROM_LOAD( "46.y1",  0xf800, 0x0400, CRC(c770488e) SHA1(8eee3fe43b0ca2ed741a53c4982c1aa59bed6f9a) )
	ROM_LOAD( "47a.y2", 0xfc00, 0x0400, CRC(5c5e9da9) SHA1(332e5e180b48acbee59ebf2603894e5cbecc0f3e) )

	ROM_REGION( 0x1000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "48.x2",  0x0000, 0x0400, CRC(8516b7c6) SHA1(e75e193b746ef9cf6524efb0c642573005cf56c5) )
	ROM_LOAD( "49.x3",  0x0400, 0x0400, CRC(dc39e711) SHA1(547e2d9c19bd2d91d27b395ba1f3a8e103c42da9) )

	ROM_REGION( 0x0400, "gfx2", 0 )
	ROM_LOAD( "x.x1",   0x0000, 0x0400, CRC(20007b6f) SHA1(036005e30a1c369fabd403d5d85a96bb7183ecdb) )

	ROM_REGION( 0x0400, "gfx3", 0 )
	ROM_LOAD( "x.h4",   0x0000, 0x0400, CRC(b0f2c70c) SHA1(638ccb0f50731d26bb546d9573296e4193e9ca11) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//     YEAR  NAME     PARENT  MACHINE  INPUT    STATE         INIT        COMPANY, FULLNAME, FLAGS
GAMEL( 1979, lbeach,  0,      lbeach,  lbeach,  lbeach_state, empty_init, ROT0, "Seletron / Olympia", "Long Beach", MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_lbeach )
GAMEL( 1980, speedrs, 0,      lbeach,  speedrs, lbeach_state, empty_init, ROT0, "Seletron / Olympia", "Speed Race (Seletron / Olympia)", MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_speedrs )
