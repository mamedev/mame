// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Olympia / Seletron, Long Beach

  PCB was broken, and there are no known references.
  16MHz XTAL, M6800 @ 500kHz
  2x 5101 sram 256x4bit (256 byte) battery backed
  4x 4045 sram 1kx4 (2K byte)

  Game should be in b&w? But then highlighted blocks in testmode
  would be invisible.

  6800 hits many illegal opcodes (0x02), though it's harmless.
  Maybe they meant to inserts nops (0x01) to remove debug stuff
  or to make room for future additions?

TODO:
- discrete sound
- improve colors
- unknown writes

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/nvram.h"

#include "lbeach.lh"


class lbeach_state : public driver_device
{
public:
	lbeach_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bg_vram(*this, "bg_vram"),
		m_fg_vram(*this, "fg_vram"),
		m_scroll_y(*this, "scroll_y"),
		m_sprite_x(*this, "sprite_x"),
		m_sprite_code(*this, "sprite_code"),
		m_collision_bg_car(0),
		m_collision_fg_car(0),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	/* devices / memory pointers */
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_bg_vram;
	required_shared_ptr<UINT8> m_fg_vram;
	required_shared_ptr<UINT8> m_scroll_y;
	required_shared_ptr<UINT8> m_sprite_x;
	required_shared_ptr<UINT8> m_sprite_code;

	int m_collision_bg_car;
	int m_collision_fg_car;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bitmap_ind16 m_colmap_car;
	tilemap_t* m_bg_tilemap;
	tilemap_t* m_fg_tilemap;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	DECLARE_WRITE8_MEMBER(lbeach_bg_vram_w);
	DECLARE_WRITE8_MEMBER(lbeach_fg_vram_w);
	DECLARE_READ8_MEMBER(lbeach_in1_r);
	DECLARE_READ8_MEMBER(lbeach_in2_r);

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(lbeach);
	UINT32 screen_update_lbeach(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(lbeach_state, lbeach)
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
	UINT8 code = m_bg_vram[tile_index];

	SET_TILE_INFO_MEMBER(1, code & 0x1f, code >> 6 & 3, 0);
}

TILE_GET_INFO_MEMBER(lbeach_state::get_fg_tile_info)
{
	UINT8 code = m_fg_vram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void lbeach_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lbeach_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lbeach_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_screen->register_screen_bitmap(m_colmap_car);
}


UINT32 lbeach_state::screen_update_lbeach(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw bg layer (road)
	m_bg_tilemap->set_scrolly(0, *m_scroll_y);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// check collision
	int sprite_code = *m_sprite_code & 0xf;
	int sprite_x = *m_sprite_x * 2 - 4;
	int sprite_y = 160;

	m_colmap_car.fill(0, cliprect);
	m_gfxdecode->gfx(2)->transpen(m_colmap_car,cliprect, sprite_code, 0, 0, 0, sprite_x, sprite_y, 0);
	bitmap_ind16 &fg_bitmap = m_fg_tilemap->pixmap();

	m_collision_bg_car = 0;
	m_collision_fg_car = 0;

	for (int y = sprite_y; y < (sprite_y + 16); y++)
	{
		for (int x = sprite_x; x < (sprite_x + 16) && cliprect.contains(x, y); x++)
		{
			m_collision_bg_car |= (bitmap.pix16(y, x) & m_colmap_car.pix16(y, x) & 1);
			m_collision_fg_car |= (fg_bitmap.pix16(y, x) & m_colmap_car.pix16(y, x) & 1);
		}
	}

	// draw fg layer (tiles)
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw player car
	m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, sprite_code, 0, 0, 0, sprite_x, sprite_y, 0);

	return 0;
}


WRITE8_MEMBER(lbeach_state::lbeach_bg_vram_w)
{
	m_bg_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(lbeach_state::lbeach_fg_vram_w)
{
	m_fg_vram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}



/***************************************************************************

  Memory Map, I/O

***************************************************************************/

READ8_MEMBER(lbeach_state::lbeach_in1_r)
{
	// d6,7(steering wheel) need to be swapped
	return BITSWAP8(ioport("IN1")->read(),6,7,5,4,3,2,1,0);
}

READ8_MEMBER(lbeach_state::lbeach_in2_r)
{
	// d6 and d7 are for collision detection
	UINT8 d6 = m_collision_fg_car ? 0x40 : 0;
	UINT8 d7 = m_collision_bg_car ? 0x80 : 0;

	return (ioport("IN2")->read() & 0x3f) | d6 | d7;
}

static ADDRESS_MAP_START( lbeach_map, AS_PROGRAM, 8, lbeach_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x4000, 0x4000) AM_READ(lbeach_in1_r)
	AM_RANGE(0x4000, 0x41ff) AM_RAM_WRITE(lbeach_bg_vram_w) AM_SHARE("bg_vram")
	AM_RANGE(0x4200, 0x43ff) AM_RAM
	AM_RANGE(0x4400, 0x47ff) AM_RAM_WRITE(lbeach_fg_vram_w) AM_SHARE("fg_vram")
	AM_RANGE(0x8000, 0x8000) AM_READ(lbeach_in2_r)
	AM_RANGE(0x8000, 0x8000) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x8001, 0x8001) AM_WRITEONLY AM_SHARE("sprite_x")
	AM_RANGE(0x8002, 0x8002) AM_WRITEONLY AM_SHARE("sprite_code")
//  AM_RANGE(0x8003, 0x8003) AM_WRITENOP // ?
//  AM_RANGE(0x8004, 0x8004) AM_WRITENOP // ?
//  AM_RANGE(0x8005, 0x8005) AM_WRITENOP // ?
	AM_RANGE(0x8007, 0x8007) AM_WRITENOP // probably watchdog
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
//  AM_RANGE(0xa003, 0xa003) AM_READNOP // ? tests d7 at game over
	AM_RANGE(0xc000, 0xcfff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( lbeach )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas Pedal")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shifter 1st Gear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shifter 2nd Gear")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter 3rd Gear")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Shifter 4th Gear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Record") // called RR in testmode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Coin Counter") // called CC in testmode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT(0xc0, 0x40, IPT_PADDLE ) PORT_INVERT PORT_MINMAX(0x00,0x80) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CENTERDELTA(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) // dupe
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Refueling Point" )
	PORT_DIPSETTING(    0x00, "200 km" )
	PORT_DIPSETTING(    0x04, "250 km" )
	PORT_DIPSETTING(    0x08, "300 km" )
	PORT_DIPSETTING(    0x0c, "350 km" )
	PORT_DIPNAME( 0x30, 0x10, "Fuel Drop Rate" )
	PORT_DIPSETTING(    0x30, "1" ) // slow
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" ) // fast
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout tile_layout_16x8 =
{
	16,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout tile_layout_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( lbeach )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout_16x8, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout_16x16, 2, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout_16x16, 10, 1 )
GFXDECODE_END


void lbeach_state::machine_start()
{
	save_item(NAME(m_collision_bg_car));
	save_item(NAME(m_collision_fg_car));
}

void lbeach_state::machine_reset()
{
}

static MACHINE_CONFIG_START( lbeach, lbeach_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_16MHz / 32) // Motorola MC6800P, 500kHz
	MCFG_CPU_PROGRAM_MAP(lbeach_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lbeach_state, nmi_line_pulse)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // ~60Hz
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 511-32, 0, 255-24)
	MCFG_SCREEN_UPDATE_DRIVER(lbeach_state, screen_update_lbeach)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE) // needed for collision detection
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lbeach)
	MCFG_PALETTE_ADD("palette", 2+8+2)
	MCFG_PALETTE_INIT_OWNER(lbeach_state, lbeach)
	/* sound hardware */
	// ...
MACHINE_CONFIG_END



/***************************************************************************

  Game Drivers

***************************************************************************/

ROM_START( lbeach )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "x.c1",   0xc000, 0x0800, CRC(767fc287) SHA1(da8d59c4827b8479064084e14fa8a7bf71a65293) )
	ROM_LOAD( "c2.c2",  0xc800, 0x0800, CRC(9f30e0de) SHA1(607873566fd330486d2bc22cabd5623c021b1dc8) )
	ROM_LOAD( "f03.c3", 0xf000, 0x0800, CRC(6728aa45) SHA1(7f41d52f1e63cafd82ad0853a66ecbbdbee070b3) )
	ROM_LOAD( "f47.c4", 0xf800, 0x0800, CRC(950f3154) SHA1(e8bd1a818fe293163466169b65e07ac635ddfdb1) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "15.e2",  0x0000, 0x0800, CRC(9f24897c) SHA1(5ae05b06dadf4e935e39ac289f2db7719ff3b230) )
	ROM_LOAD( "s2.d2",  0x0800, 0x0800, CRC(c5d59466) SHA1(663df8397081daa89f9e3f7aec7e07557cf5b310) )

	ROM_REGION( 0x0400, "gfx2", 0 )
	ROM_LOAD( "r.d1",   0x0000, 0x0400, CRC(07e99395) SHA1(67a68dee8e97ae74ab13af62cdc5279c358dc897) )

	ROM_REGION( 0x0200, "gfx3", 0 )
	ROM_LOAD( "93448.h4", 0x0000, 0x0200, BAD_DUMP CRC(0dca040d) SHA1(d775cbf9fbb9449881ce4997187a1945d34d3cb6) ) // bad dump: stuck a5
ROM_END


GAMEL(1979, lbeach, 0, lbeach, lbeach, driver_device, 0, ROT0, "Olympia / Seletron", "Long Beach", MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_lbeach )
