// license:BSD-3-Clause
// copyright-holders: David Haywood, hap

/*

Galaxia by Zaccaria (1979)
Also released in several regions as "Super Galaxians".
Set regions are unknown, so all are currently named Galaxia.

Taken from an untested board.

1K byte files were 2708 or equivalent.
512 byte file is a 82S130 PROM.

This is not a direct pirate of Galaxian as you might think from the name.
The game uses a Signetics 2650A CPU with three 40-pin 2636 chips, which are
responsible for basic sound and some video functions.

Other than that, the video hardware looks like it's similar to Galaxian
(2 x 2114, 2 x 2101, 2 x EPROM) but there is no attack RAM and the graphics
EPROMS are 2708. The graphics EPROMS do contain Galaxian-like graphics...

Quick PCB sketch:
  ------------------------------------------------------------------------------
  |                                                                            |
  |                   13l    13i    13h                                        |
  |                                                                            |
  |    PROM           11l    11i    11h      S2636            S2621    XTAL    |
|6-|                                                                 14.31818  |
  |                          10i    10h      S2636                             |
|5-|                                                                           |
|--|                          8i     8h      S2636                   S2650A    |
|--|                                                                           |
|--|                                                                           |
  |                                                                            |
|4-|                                                                           |
|--|                                                                           |
|--|                                                                           |
|--|                                                                           |
  |                                                                            |
|3-|                                                                           |
|--|                                                                           |
|--|                                                                           |
|--|       DSW                                                                 |
  |                                                     3d                     |
|2-|       DSW                                                                |-1|
  |                                                     1d                    |--|
  |                                                                           |--|
  ------------------------------------------------------------------------------

Astro Wars (port of Astro Fighter) is on a stripped down board of Galaxia,
using only one 2636 chip, less RAM, and no PROM.

Manual and Schematic for Galaxia can be found at:
http://www.zzzaccaria.com/manuals/SuperGalaxiansTechnicalManual.zip
http://www.zzzaccaria.com/manuals/GalaxiaSchematics.zip

The manual for Astro Wars can also be found at:
http://www.opdenkelder.com/Astrowars_manual.zip

HW has many similarities with quasar.cpp / cvs.cpp / zac2650.cpp
real hardware video of Astro Wars can be seen here: youtu.be/eSrQFBMeDlM
---

TODO:
- go through everything in the schematics for astrowar / galaxia
- video rewrite to:
   * support RAW_PARAMS, blanking is much like how laserbat hardware does it
   and is needed to correct the speed in all machines
   * improve bullets
   * provide correct color/star generation, using info from Galaxia technical
   manual and schematics
   * provide accurate sprite/bg sync in astrowar
- what is the PROM for? schematics are too burnt to tell anything
- add sound board emulation

*/

#include "emu.h"

#include "cvs_base.h"

#include "speaker.h"
#include "tilemap.h"


namespace {

class galaxia_state : public cvs_base_state
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_base_state(mconfig, type, tag)
	{ }

	void galaxia(machine_config &config) ATTR_COLD;

	void init_common() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	tilemap_t *m_bg_tilemap = nullptr;

	template <uint8_t Which> void video_w(offs_t offset, uint8_t data);
	void vblank_irq(int state);
	void data_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

private:
	void scroll_w(uint8_t data);
	void ctrlport_w(uint8_t data);
	void dataport_w(uint8_t data);
	uint8_t collision_r();
	uint8_t collision_clear();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mem_map(address_map &map) ATTR_COLD;
};

class astrowar_state : public galaxia_state
{
public:
	astrowar_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxia_state(mconfig, type, tag)
	{ }

	void astrowar(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	bitmap_ind16 m_temp_bitmap;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mem_map(address_map &map) ATTR_COLD;
};


static constexpr uint8_t SPRITE_PEN_BASE = 0x10;
static constexpr uint8_t STAR_PEN = 0x18;
static constexpr uint8_t BULLET_PEN = 0x19;


// Colors are 3bpp, but how they are generated is a mystery
// there's no color PROM on the PCB, nor palette RAM

void galaxia_state::palette(palette_device &palette) const
{
	// estimated with video/photo references
	constexpr int lut_clr[0x18] = {
		// background
		0, 1, 4, 5,
		0, 3, 6, 2,
		0, 1, 4, 5, // unused?
		0, 3, 1, 7,

		// sprites
		0, 4, 3, 6, 1, 5, 2, 7
	};

	for (int i = 0; i < 0x18; i++)
		palette.set_pen_color(i, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

void astrowar_state::palette(palette_device &palette) const
{
	// no reference material available(?), except for Data East astrof
	constexpr int lut_clr[8] = { 7, 3, 5, 1, 4, 2, 6, 7 };

	for (int i = 0; i < 8; i++)
	{
		// background
		palette.set_pen_color(i * 2, 0, 0, 0);
		palette.set_pen_color(i * 2 + 1, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

		// sprites
		palette.set_pen_color(i | SPRITE_PEN_BASE, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

TILE_GET_INFO_MEMBER(galaxia_state::get_bg_tile_info)
{
	uint8_t code = m_video_ram[tile_index] & 0x7f; // d7 unused
	uint8_t color = m_color_ram[tile_index] & 3; // highest bits unused

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(astrowar_state::get_bg_tile_info)
{
	uint8_t code = m_video_ram[tile_index];
	uint8_t color = m_color_ram[tile_index] & 7; // highest bits unused

	tileinfo.set(0, code, color, 0);
}

void galaxia_state::init_common()
{
	assert((STAR_PEN & 7) == 0);
	init_stars();
}

void galaxia_state::video_start()
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxia_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);

}

void astrowar_state::video_start()
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(astrowar_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);
	m_bg_tilemap->set_scrolldx(8, 8);

	m_screen->register_screen_bitmap(m_temp_bitmap);
}


/********************************************************************************/

uint32_t galaxia_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);
	bitmap_ind16 const &s2636_2_bitmap = m_s2636[2]->update(cliprect);

	bitmap.fill(0, cliprect);
	update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			bool const bullet = m_bullet_ram[y] && x == (m_bullet_ram[y] ^ 0xff);
			bool const background = (bitmap.pix(y, x) & 3) != 0;

			// draw bullets (guesswork)
			if (bullet)
			{
				// background vs. bullet collision detection
				if (background) m_collision_register |= 0x80;

				// bullet size/color/priority is guessed
				bitmap.pix(y, x) = BULLET_PEN;
				if (x) bitmap.pix(y, x - 1) = BULLET_PEN;
			}

			// copy the S2636 images into the main bitmap and check collision
			int const pixel0 = s2636_0_bitmap.pix(y, x);
			int const pixel1 = s2636_1_bitmap.pix(y, x);
			int const pixel2 = s2636_2_bitmap.pix(y, x);

			int const pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. S2636 collision detection
				if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x01;
				if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
				if (S2636_IS_PIXEL_DRAWN(pixel2) && S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x04;

				// S2636 vs. bullet collision detection
				if (bullet) m_collision_register |= 0x08;

				// S2636 vs. background collision detection
				if (background)
				{
					/* bit4 causes problems on 2nd level
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x10; */
					if (S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x20;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x40;
				}

				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}


uint32_t astrowar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// astrowar has only one S2636
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);

	bitmap.fill(0, cliprect);
	update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	copybitmap(m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		// draw bullets (guesswork)
		if (m_bullet_ram[y])
		{
			uint8_t const pos = m_bullet_ram[y] ^ 0xff;

			// background vs. bullet collision detection
			if (m_temp_bitmap.pix(y, pos) & 1)
				m_collision_register |= 0x02;

			// bullet size/color/priority is guessed
			bitmap.pix(y, pos) = BULLET_PEN;
			if (pos) bitmap.pix(y, pos - 1) = BULLET_PEN;
		}

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// NOTE: similar to zac2650.cpp, the sprite chip runs at a different frequency than the background generator
			// the exact timing ratio is unknown, so we'll have to do with guesswork
			float const s_ratio = 256.0f / 196.0f;

			float const sx = x * s_ratio;
			if (int(sx + 0.5f) > cliprect.right())
				break;

			// copy the S2636 bitmap into the main bitmap and check collision
			int const pixel = s2636_0_bitmap.pix(y, x);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. background collision detection
				if ((m_temp_bitmap.pix(y, int(sx)) | m_temp_bitmap.pix(y, int(sx + 0.5f))) & 1)
					m_collision_register |= 0x01;

				bitmap.pix(y, int(sx)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
				bitmap.pix(y, int(sx + 0.5f)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}


void galaxia_state::vblank_irq(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		scroll_start();
	}
}


/***************************************************************************

  I/O

***************************************************************************/

template <uint8_t Which>
void galaxia_state::video_w(offs_t offset, uint8_t data)
{
	m_bg_tilemap->mark_tile_dirty(offset);
	Which ? m_video_ram[offset] = data : m_color_ram[offset] = data;
}

void galaxia_state::scroll_w(uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());

	// fixed scrolling area
	for (int i = 1; i < 6; i++)
		m_bg_tilemap->set_scrolly(i, data);
}

void galaxia_state::ctrlport_w(uint8_t data)
{
	// d0: triggers on every new credit
	// d1: coin counter? if you put a coin in slot A, galaxia constantly
	// strobes sets and clears the bit. if you put a coin in slot B
	// however, the bit is set and cleared only once.
	// d5: set as soon as the game completes selftest
	// other bits: unknown
}

void galaxia_state::dataport_w(uint8_t data)
{
	// seems to be related to sound board comms
}

uint8_t galaxia_state::collision_r()
{
	m_screen->update_partial(m_screen->vpos());
	return m_collision_register;
}

uint8_t galaxia_state::collision_clear()
{
	m_screen->update_partial(m_screen->vpos());
	m_collision_register = 0;
	return 0xff;
}

void galaxia_state::mem_map(address_map &map)
{
	map(0x0000, 0x13ff).rom();
	map(0x1400, 0x14ff).mirror(0x6000).ram().share(m_bullet_ram);
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1700, 0x17ff).mirror(0x6000).rw(m_s2636[2], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1800, 0x1bff).mirror(0x6000).view(m_ram_view);
	m_ram_view[0](0x1800, 0x1bff).ram().w(FUNC(galaxia_state::video_w<0>)).share(m_color_ram);
	m_ram_view[1](0x1800, 0x1bff).ram().w(FUNC(galaxia_state::video_w<1>)).share(m_video_ram);
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
	map(0x2000, 0x33ff).rom();
	map(0x7214, 0x7214).portr("IN0");
}

void astrowar_state::mem_map(address_map &map)
{
	map(0x0000, 0x13ff).rom();
	map(0x1400, 0x14ff).mirror(0x6000).ram();
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1800, 0x1bff).mirror(0x6000).view(m_ram_view);
	m_ram_view[0](0x1800, 0x1bff).ram().w(FUNC(astrowar_state::video_w<0>)).share(m_color_ram);
	m_ram_view[1](0x1800, 0x1bff).ram().w(FUNC(astrowar_state::video_w<1>)).share(m_video_ram);
	map(0x1c00, 0x1cff).mirror(0x6000).ram().share(m_bullet_ram);
	map(0x2000, 0x33ff).rom();
}

void galaxia_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).w(FUNC(galaxia_state::scroll_w)).portr("IN0");
	map(0x02, 0x02).portr("IN1");
	map(0x05, 0x05).nopr();
	map(0x06, 0x06).portr("DSW0");
	map(0x07, 0x07).portr("DSW1");
	map(0xac, 0xac).nopr();
}

void galaxia_state::data_map(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(galaxia_state::collision_r), FUNC(galaxia_state::ctrlport_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(galaxia_state::collision_clear), FUNC(galaxia_state::dataport_w));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( galaxia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc3, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1C_1C B 2C_1C" )
	PORT_DIPSETTING(    0x01, "A 1C_2C B 2C_1C" )
	PORT_DIPSETTING(    0x02, "A 1C_3C B 2C_1C" )
	PORT_DIPSETTING(    0x03, "A 1C_5C B 2C_1C" )
	PORT_DIPSETTING(    0x04, "A 1C_1C B 1C_1C" )
	PORT_DIPSETTING(    0x05, "A 1C_2C B 1C_1C" )
	PORT_DIPSETTING(    0x06, "A 1C_3C B 1C_1C" )
	PORT_DIPSETTING(    0x07, "A 1C_5C B 1C_1C" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )

	PORT_DIPNAME( 0x10, 0x00, "UNK04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "UNK05" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "UNK06" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "UNK07" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "UNK10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "UNK11" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "UNK12" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "UNK13" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x00, "UNK14" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "UNK15" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "UNK16" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "UNK17" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Configs

***************************************************************************/

static const gfx_layout tiles8x8x2_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_galaxia )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8x2_layout, 0, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_astrowar )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 8 )
GFXDECODE_END


void galaxia_state::galaxia(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, XTAL(14'318'181) / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxia_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &galaxia_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &galaxia_state::data_map);
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));
	m_maincpu->flag_handler().set(FUNC(galaxia_state::write_s2650_flag));
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(60); // wrong
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0*8, 30*8-1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(galaxia_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(galaxia_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxia);
	PALETTE(config, m_palette, FUNC(galaxia_state::palette), 0x18+2);

	S2636(config, m_s2636[0], 0);
	m_s2636[0]->set_offsets(-13, -26);
	m_s2636[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[1], 0);
	m_s2636[1]->set_offsets(-13, -26);
	m_s2636[1]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[2], 0);
	m_s2636[2]->set_offsets(-13, -26);
	m_s2636[2]->add_route(ALL_OUTPUTS, "mono", 0.25);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}

void astrowar_state::astrowar(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, XTAL(14'318'181) / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &astrowar_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &astrowar_state::io_map);
	m_maincpu->set_addrmap(AS_DATA, &astrowar_state::data_map);
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));
	m_maincpu->flag_handler().set(FUNC(astrowar_state::write_s2650_flag));
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(1*8, 31*8-1, 2*8, 32*8-1);
	m_screen->set_screen_update(FUNC(astrowar_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(astrowar_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_astrowar);
	PALETTE(config, m_palette, FUNC(astrowar_state::palette), 0x18+2);

	S2636(config, m_s2636[0], 0);
	m_s2636[0]->set_offsets(-13, -8);
	m_s2636[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( galaxia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(c0baa654) SHA1(80e0880c32ad285fbce0f7f552268b964b97cab3) )
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) )
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(c4482770) SHA1(aee983cc3d80989f49aea4138961bb623039484a) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(1d22219b) SHA1(6ab8ea8c78db30d80de98879018726d0420d30fe) ) // sldh - only 1 bit difference compared with set 1/2, however not considered a bad dump since it was found on two boards
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END

ROM_START( galaxiac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(5682d56f) SHA1(15afb3296e93f8371d36b686ce372f917bd5b771) ) // sldh
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(80dafe84) SHA1(8a71a05f1b0ddba36bf748a4801f3a78f63af1db) ) // sldh
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(ffe86fdb) SHA1(67b02a5c39dbe515b6d68583c8831b0dae15374a) ) // sldh
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(8e3f5343) SHA1(6298be9bb33975854cb3d009b89913b1a8018aee) ) // sldh
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.1d", 0x00000, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from parent
	ROM_LOAD( "galaxia.3d", 0x00400, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from parent

	ROM_REGION( 0x0200, "proms", 0 ) // unknown function
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) )
ROM_END


ROM_START( astrowar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astro.8h",  0x00000, 0x0400, CRC(b0ec246c) SHA1(f9123b5e317938655f5e8b3f8a5810d0b2b7c7af) )
	ROM_LOAD( "astro.10h", 0x00400, 0x0400, CRC(090d360f) SHA1(528ddcdc30a5a291bd8850ff6f134fcc19af562f) )
	ROM_LOAD( "astro.11h", 0x00800, 0x0400, CRC(72ab1378) SHA1(50743c64c4775076aa6f1d8ab2e05c14884bf0ba) )
	ROM_LOAD( "astro.13h", 0x00c00, 0x0400, CRC(2dc4c895) SHA1(831afbfd4ebfd6522ab0758222bc6f9826148a5d) )
	ROM_LOAD( "astro.8i",  0x01000, 0x0400, CRC(ab87fbfc) SHA1(34b670f96c260f186c643e588995ae5d80377784) )
	ROM_LOAD( "astro.10i", 0x02000, 0x0400, CRC(533675c1) SHA1(69cc066e1874a135a53a21b7b2461bda456504f1) )
	ROM_LOAD( "astro.11i", 0x02400, 0x0400, CRC(59cf8901) SHA1(e849d4c99350b7e3453c156d91618b71b5be1163) )
	ROM_LOAD( "astro.13i", 0x02800, 0x0400, CRC(5149c121) SHA1(232ba594e283fb25c31d8ae0b7d8315a81852a71) BAD_DUMP ) // suspected bad byte at 0x2a00
	ROM_LOAD( "astro.11l", 0x02c00, 0x0400, CRC(29f52f57) SHA1(5cb50b82e09c537eeaeae167351fca686fde8228) )
	ROM_LOAD( "astro.13l", 0x03000, 0x0400, CRC(882cdb87) SHA1(062ee8d296316cbce2eb62e72774aa4181e9847d) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "astro.1d",  0x00000, 0x0400, CRC(6053f834) SHA1(e0b76800c241b3c8010c09869cecbc109b25310a) )
	ROM_LOAD( "astro.3d",  0x00400, 0x0400, CRC(822505aa) SHA1(f9d3465e14bb850a286f8b4f42aa0a4044413b67) )
ROM_END

} // anonymous namespace


GAME( 1979, galaxia,  0,       galaxia,  galaxia, galaxia_state,  empty_init, ROT90, "Zaccaria / Zelco", "Galaxia (set 1)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiaa, galaxia, galaxia,  galaxia, galaxia_state,  empty_init, ROT90, "Zaccaria / Zelco", "Galaxia (set 2)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiab, galaxia, galaxia,  galaxia, galaxia_state,  empty_init, ROT90, "Zaccaria / Zelco", "Galaxia (set 3)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiac, galaxia, galaxia,  galaxia, galaxia_state,  empty_init, ROT90, "Zaccaria / Zelco", "Galaxia (set 4)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1980, astrowar, 0,       astrowar, galaxia, astrowar_state, empty_init, ROT90, "Zaccaria / Zelco", "Astro Wars",      MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
