// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/***********************************
 Super Speed Race Jr (c) 1985 Taito
 driver by  Tomasz Slanina


 TODO:
 - colors (missing proms)
 - dips
 - when a car sprite goes outside of the screen it gets stuck for a split frame on top of screen

HW info :

    0000-7fff ROM
    c000-dfff VRAM ( 4 tilemaps (4 x $800) )
    e000-e7ff RAM
    e800-efff SCROLL RAM
    f003      ??
  f400-f401 AY 8910
  fc00      ??
  f800      ??

 Scroll RAM contains x and y offsets for each tileline,
 as well as other data (priorities ? additional flags ?)
 All moving objects (cars, etc) are displayed on tilemap 3.

 ------------------------------------
 Cheat :  $e210 - timer

************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ssrj_state : public driver_device
{
public:
	ssrj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_buffered_spriteram(*this, "scrollram"),
		m_vram(*this, "vram%u", 1U),
		m_in1(*this, "IN1")
	{ }

	void ssrj(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_buffered_spriteram;

	required_shared_ptr_array<uint8_t, 4> m_vram;

	required_ioport m_in1;

	int16_t m_oldport = 0;
	tilemap_t *m_tilemap[4]{};

	uint8_t wheel_r();
	template <uint8_t Which> void vram_w(offs_t offset, uint8_t data);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_objects(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
};



template <uint8_t Which>
void ssrj_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset >> 1);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(ssrj_state::get_tile_info)
{
	int const code = m_vram[Which][tile_index << 1] + (m_vram[Which][(tile_index << 1) + 1] << 8);
	tileinfo.set(0,
		code & 0x3ff,
		((code >> 12) & 0x3) + Which * 4,
		((code & 0x8000) ? TILE_FLIPY : 0) |( (code & 0x4000) ? TILE_FLIPX : 0));
}


/*
TODO: This table is nowhere near as accurate. If you bother, here's how colors should be:
-"START" sign is red with dark blue background.
-Sidewalk is yellow-ish.
-first opponents have swapped colors (blue/yellow instead of yellow/blue)
-after the first stage, houses have red/white colors.
*/

static constexpr rgb_t fakecols[4 * 4][8] =
{
{{0x00,0x00,0x00},
	{42,87,140},
	{0,0,0},
	{33,75,160},
	{0xff,0xff,0xff},
	{37,56,81},
	{0x1f,0x1f,0x2f},
	{55,123,190}},

{{0x00,0x00,0x00},
	{0x00,99,41},
	{0x00,0x00,0xff},
	{0x00,0xff,0},
	{255,255,255},
	{0xff,0x00,0x00},
	{0,45,105},
	{0xff,0xff,0}},


{{0x00,0x00,0x00},
	{0x00,0x20,0x00},
	{0x00,0x40,0x00},
	{0x00,0x60,0x00},
	{0x00,0x80,0x00},
	{0x00,0xa0,0x00},
	{0x00,0xc0,0x00},
	{0x00,0xf0,0x00}},

	{{0x00,0x00,0x00},
	{0x20,0x00,0x20},
	{0x40,0x00,0x40},
	{0x60,0x00,0x60},
	{0x80,0x00,0x80},
	{0xa0,0x00,0xa0},
	{0xc0,0x00,0xc0},
	{0xf0,0x00,0xf0}},

{{0x00,0x00,0x00},
	{0xff,0x00,0x00},
	{0x7f,0x00,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0xaf,0x00,0x00},
	{0xff,0xff,0xff},
	{0xff,0x7f,0x7f}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{{0x00,0x00,0x00},
	{0xff,0x00,0x00},
	{0x00,0x00,0x9f},
	{0x60,0x60,0x60},
	{0x00,0x00,0x00},
	{0xff,0xff,0x00},
	{0x00,0xff,0x00},
	{0xff,0xff,0xff}},

{
	{0x00,0x00,0x00},
	{0x00,0x00,0xff},
	{0x00,0x00,0x7f},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0xaf},
	{0xff,0xff,0xff},
	{0x7f,0x7f,0xff}},

{{0x00,0x00,0x00},
	{0xff,0xff,0x00},
	{0x7f,0x7f,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0xaf,0xaf,0x00},
	{0xff,0xff,0xff},
	{0xff,0xff,0x7f}},

{{0x00,0x00,0x00},
	{0x00,0xff,0x00},
	{0x00,0x7f,0x00},
	{0x00,0x00,0x00},
	{0x00,0x00,0x00},
	{0x00,0xaf,0x00},
	{0xff,0xff,0xff},
	{0x7f,0xff,0x7f}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{{0x00,0x00,0x00},
	{0x20,0x20,0x20},
	{0x40,0x40,0x40},
	{0x60,0x60,0x60},
	{0x80,0x80,0x80},
	{0xa0,0xa0,0xa0},
	{0xc0,0xc0,0xc0},
	{0xf0,0xf0,0xf0}},

{
	{0x00,0x00,0x00},
	{0xff,0xaf,0xaf},
	{0x00,0x00,0xff},
	{0xff,0xff,0xff},
	{0x00,0x00,0x00},
	{0xff,0x50,0x50},
	{0xff,0xff,0x00},
	{0x00,0xff,0x00}
}

};

void ssrj_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssrj_state::get_tile_info<0>)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssrj_state::get_tile_info<1>)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssrj_state::get_tile_info<3>)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[3]->set_transparent_pen(0);
}


void ssrj_state::draw_objects(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *buffered_spriteram = m_buffered_spriteram->buffer();

	for (int i = 0; i < 6; i++)
	{
		int const y = buffered_spriteram[0x80 + 20 * i];
		int x = buffered_spriteram[0x80 + 20 * i + 2];
		if (!buffered_spriteram[0x80 + 20 * i + 3])
		{
			for (int k = 0; k < 5; k++, x += 8)
			{
				for (int j = 0; j < 0x20; j++)
				{
					int const offs = (i * 5 + k) * 64 + (31 - j) * 2;

					int const code = m_vram[2][offs] + 256 * m_vram[2][offs + 1];
					m_gfxdecode->gfx(0)->transpen(bitmap,
						cliprect,
						code & 1023,
						((code >> 12) & 0x3) + 8,
						code & 0x4000,
						code & 0x8000,
						x,
						(247 - (y + (j << 3))) & 0xff,
						0);
				}
			}
		}
	}
}


void ssrj_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 4 * 4; i++)
		for (int j = 0; j < 8; j++)
			palette.set_pen_color(i * 8 + j, fakecols[i][j]);
}

uint32_t ssrj_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *buffered_spriteram = m_buffered_spriteram->live();
	m_tilemap[0]->set_scrollx(0, 0xff - buffered_spriteram[2]);
	m_tilemap[0]->set_scrolly(0, buffered_spriteram[0]);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_objects(bitmap, cliprect);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	if (m_buffered_spriteram->live()[0x101] == 0xb) m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0); // hack to display 4th tilemap
	return 0;
}


void ssrj_state::machine_start()
{
	save_item(NAME(m_oldport));
}

void ssrj_state::machine_reset()
{
	uint8_t *rom = memregion("maincpu")->base();

	memset(&rom[0xc000], 0 ,0x3fff); // req for some control types
	m_oldport = 0x80;
}

uint8_t ssrj_state::wheel_r()
{
	int const port = m_in1->read() - 0x80;
	int const retval = port - m_oldport;

	m_oldport = port;
	return retval;
}

void ssrj_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().w(FUNC(ssrj_state::vram_w<0>)).share(m_vram[0]);
	map(0xc800, 0xcfff).ram().w(FUNC(ssrj_state::vram_w<1>)).share(m_vram[1]);
	map(0xd000, 0xd7ff).ram().share(m_vram[2]);
	map(0xd800, 0xdfff).ram().w(FUNC(ssrj_state::vram_w<3>)).share(m_vram[3]);
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("scrollram");
	map(0xf000, 0xf000).portr("IN0");
	map(0xf001, 0xf001).r(FUNC(ssrj_state::wheel_r));
	map(0xf002, 0xf002).portr("IN2");
	map(0xf003, 0xf003).nopw(); // unknown
	map(0xf401, 0xf401).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xf400, 0xf401).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xf800, 0xf800).nopw(); // wheel ?
	map(0xfc00, 0xfc00).nopw(); // unknown
}

static INPUT_PORTS_START( ssrj )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xe0, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0xe0) PORT_SENSITIVITY(50) PORT_KEYDELTA(0x20)

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("IN2")
	PORT_BIT( 0xf, IP_ACTIVE_LOW, IPT_UNUSED  ) PORT_CONDITION("IN3", 0x30, EQUALS, 0x00) // code @ $eef, tested when controls != type1
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Easy)")
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Normal)")
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_START3 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Difficult)")
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_START4 ) PORT_CONDITION("IN3", 0x30, NOTEQUALS, 0x00) PORT_NAME("Start (Very difficult)")
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING( 0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "No Hit" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Controls ) ) // Type 1 has no start button and uses difficulty DSW, type 2-4 have 4 start buttons that determine difficulty. MT07492 for more.
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x10, "Type 2" )
	PORT_DIPSETTING(    0x20, "Type 3" )
	PORT_DIPSETTING(    0x30, "Type 4" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // sometimes hangs after game over ($69b)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,3),  // 1024 characters
	3,  // 3 bits per pixel
	{ 0, RGN_FRAC(2,3), RGN_FRAC(1,3) },    // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_ssrj )
	GFXDECODE_ENTRY( "gfx", 0, charlayout, 0, 0x10 )
GFXDECODE_END

void ssrj_state::ssrj(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8000000 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssrj_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(ssrj_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 34*8-1, 1*8, 31*8-1); // unknown res
	screen.set_screen_update(FUNC(ssrj_state::screen_update));
	screen.screen_vblank().set(m_buffered_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ssrj);
	PALETTE(config, m_palette, FUNC(ssrj_state::palette), 128);

	BUFFERED_SPRITERAM8(config, m_buffered_spriteram);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 8000000 / 5));
	aysnd.port_b_read_callback().set_ioport("IN3");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ssrj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a40-01.bin",   0x0000, 0x4000, CRC(1ff7dbff) SHA1(a9e676ee087141d62f880cd98e7748db1e6e9461) )
	ROM_LOAD( "a40-02.bin",   0x4000, 0x4000, CRC(bbb36f9f) SHA1(9f85bac639d18ee932273a6c00b36ac969e69bb8) )

	ROM_REGION( 0x6000, "gfx", 0 )
	ROM_LOAD( "a40-03.bin",   0x0000, 0x2000, CRC(3753182a) SHA1(3eda34f967563b11416344da87b7be46cbecff2b) )
	ROM_LOAD( "a40-04.bin",   0x2000, 0x2000, CRC(96471816) SHA1(e24b690085602b8bde079e596c2879deab128c83) )
	ROM_LOAD( "a40-05.bin",   0x4000, 0x2000, CRC(dce9169e) SHA1(2cdda1453b2913fad931788e1db0bc01ce923a04) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "proms",  0x0000, 0x0100, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1985, ssrj, 0, ssrj,  ssrj, ssrj_state, empty_init, ROT90, "Taito Corporation", "Super Speed Race Junior (Japan)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
