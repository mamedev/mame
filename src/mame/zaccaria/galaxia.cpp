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

HW has many similarities with quasar.cpp / misc/cvs.cpp / zac1b1120.cpp
real hardware video of Astro Wars can be seen here: youtu.be/eSrQFBMeDlM

--------------------------------------------------------------------------------

TODO:
- What are ports 0 and 4 read for in galaxiaa? Is it some sort of protection?
  It's also suspicious that they moved IN0 to the main memory map.
- What is port 5 exactly for? If it doesn't return 0xff, collision is disabled,
  maybe for testing/debugging?
- Are there other versions of galaxia with different colors? The alternate sets
  in MAME took the gfx roms from the 1st dumped version, but there are references
  online showing that not all versions look alike.
- support screen raw params, blanking is much like how laserbat hardware does it
  and is needed to correct the speed in all machines
- provide accurate sprite/bg sync in astrowar
- improve starfield RNG pattern. Other than the colors, it should be the same as
  CVS. It may differ per PCB, not all PCB videos have the same star RNG pattern.
  Maybe initial contents of the shift registers?
- add sound board emulation (info is in the schematics)
- add support for flip screen

*/

#include "emu.h"

#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class galaxia_state : public driver_device
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s2636(*this, "s2636_%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram", 0x400, ENDIANNESS_BIG),
		m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_BIG),
		m_bullet_ram(*this, "bullet_ram"),
		m_ram_view(*this, "video_color_ram_view")
	{ }

	void galaxia(machine_config &config) ATTR_COLD;
	void galaxiaa(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// max stars is more than it needs to be, to allow experimenting with the star generator
	static constexpr u16 MAX_STARS = 0x800;
	static constexpr u8 STAR_PEN = 0x18;
	static constexpr u8 BULLET_PEN = 0x58;

	// devices
	required_device<s2650_device> m_maincpu;
	optional_device_array<s2636_device, 3> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory
	memory_share_creator<u8> m_video_ram;
	memory_share_creator<u8> m_color_ram;
	required_shared_ptr<u8> m_bullet_ram;

	memory_view m_ram_view;

	bitmap_ind16 m_temp_bitmap;
	tilemap_t *m_bg_tilemap = nullptr;

	u8 m_collision = 0;
	bool m_stars_on = false;
	u16 m_stars_scroll = 0;
	u16 m_total_stars = 0;

	struct star_t
	{
		u16 x = 0;
		u16 y = 0;
		u8 color = 0;
	};
	star_t m_stars[MAX_STARS];

	template <u8 Which> void video_w(offs_t offset, u8 data);
	void galaxia_mem(address_map &map) ATTR_COLD;
	void galaxiaa_mem(address_map &map) ATTR_COLD;
	void galaxia_io(address_map &map) ATTR_COLD;
	void galaxiaa_io(address_map &map) ATTR_COLD;
	void galaxia_data(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void stars_bullet_palette(palette_device &palette) const ATTR_COLD;
	virtual void palette(palette_device &palette) const ATTR_COLD;
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void scroll_stars(int state);
	void init_stars() ATTR_COLD;
	void update_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void scroll_w(u8 data);
	u8 collision_r();
	u8 collision_clear_r();
	void ctrlport_w(u8 data);
	void dataport_w(u8 data);
};

class astrowar_state : public galaxia_state
{
public:
	astrowar_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaxia_state(mconfig, type, tag)
	{ }

	void astrowar(machine_config &config) ATTR_COLD;

protected:
	virtual void palette(palette_device &palette) const override ATTR_COLD;
	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	void astrowar_mem(address_map &map) ATTR_COLD;
};


void galaxia_state::machine_start()
{
	save_item(NAME(m_collision));
	save_item(NAME(m_stars_on));
	save_item(NAME(m_stars_scroll));
}

void galaxia_state::machine_reset()
{
	m_collision = 0;
	m_stars_on = false;
	m_stars_scroll = 0;
}



/*******************************************************************************
    Palette
*******************************************************************************/

void galaxia_state::stars_bullet_palette(palette_device &palette) const
{
	// 6bpp pens for the stars
	for (int i = 0; i < 0x40; i++)
	{
		int b = pal2bit(BIT(i, 1) << 1 | BIT(i, 0));
		int g = pal2bit(BIT(i, 3) << 1 | BIT(i, 2));
		int r = pal2bit(BIT(i, 5) << 1 | BIT(i, 4));

		palette.set_pen_color(i + STAR_PEN, r, g, b);
	}

	assert((BULLET_PEN & 3) == 0);
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
}

void galaxia_state::palette(palette_device &palette) const
{
	u8 const *const color_prom = memregion("proms")->base();

	// background from A5-A8
	for (int i = 0; i < 0x10; i++)
	{
		int index = bitswap<4>(i, 0, 1, 2, 3) << 5;
		u8 data = color_prom[index] & 7;

		palette.set_pen_color(i, pal1bit(BIT(data, 0)), pal1bit(BIT(data, 1)), pal1bit(BIT(data, 2)));
	}

	// sprites from A0-A3
	for (int i = 0; i < 8; i++)
	{
		u8 data = ~color_prom[i] & 7;
		palette.set_pen_color(i | 0x10, pal1bit(BIT(data, 2)), pal1bit(BIT(data, 1)), pal1bit(BIT(data, 0)));
	}

	stars_bullet_palette(palette);
}

void astrowar_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		// background
		palette.set_pen_color(i * 2, 0, 0, 0);
		palette.set_pen_color(i * 2 + 1, pal1bit(BIT(~i, 2)), pal1bit(BIT(~i, 1)), pal1bit(BIT(~i, 0)));

		// sprites
		palette.set_pen_color(i | 0x10, pal1bit(BIT(i, 0)), pal1bit(BIT(i, 1)), pal1bit(BIT(i, 2)));
	}

	stars_bullet_palette(palette);
}



/*******************************************************************************
    Stars
*******************************************************************************/

void galaxia_state::init_stars()
{
	u32 generator = 0;
	m_total_stars = 0;

	// precalculate the star background
	for (int y = 0; y < 272; y++)
	{
		for (int x = 0; x < 480; x++)
		{
			generator <<= 1;
			generator |= BIT(~generator, 17) ^ BIT(generator, 5);

			// stars are enabled if the shift register output is 0, and bits 1-7 are set
			if ((generator & 0x100fe) == 0xfe && m_total_stars != MAX_STARS)
			{
				m_stars[m_total_stars].x = x;
				m_stars[m_total_stars].y = y;
				m_stars[m_total_stars].color = generator >> 8 & 0x3f;

				m_total_stars++;
			}
		}
	}
}

void galaxia_state::update_stars(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_total_stars; offs++)
	{
		u8 x = ((m_stars[offs].x + m_stars_scroll) >> 1) % 240;
		u16 y = m_stars[offs].y;

		if ((BIT(y, 4) ^ BIT(y, 8) ^ BIT(x, 5)))
		{
			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = STAR_PEN + m_stars[offs].color;
		}
	}
}

void galaxia_state::scroll_stars(int state)
{
	if (state)
		m_stars_scroll = (m_stars_scroll + 1) % 480;
}



/*******************************************************************************
    Background
*******************************************************************************/

TILE_GET_INFO_MEMBER(galaxia_state::get_bg_tile_info)
{
	u8 code = m_video_ram[tile_index]; // d7 unused for galaxia
	u8 color = m_color_ram[tile_index]; // highest bits unused

	tileinfo.set(0, code, color, 0);
}

void galaxia_state::video_start()
{
	init_stars();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxia_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);

	m_screen->register_screen_bitmap(m_temp_bitmap);
}

static GFXDECODE_START( gfx_galaxia )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x2_planar, 0, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_astrowar )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 8 )
GFXDECODE_END


void galaxia_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_stars_on)
		update_stars(bitmap, cliprect);

	// tilemap doesn't wrap
	rectangle bg_clip = cliprect;
	bg_clip.max_y = 32*8-1;
	bg_clip &= cliprect;

	m_temp_bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, m_temp_bitmap, bg_clip, 0);
	copybitmap_trans(bitmap, m_temp_bitmap, 0, 0, 0, 0, cliprect, 0);
}



/*******************************************************************************
    Screen Update
*******************************************************************************/

u32 galaxia_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);
	bitmap_ind16 const &s2636_2_bitmap = m_s2636[2]->update(cliprect);

	draw_background(screen, bitmap, cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const bullet_pos = (((y < 0x100) ? m_bullet_ram[y] : 0) ^ 0xff) - 8;
			bool const bullet = (bullet_pos != 0xff - 8) && (x <= bullet_pos && x > bullet_pos - 4);
			bool const background = (m_temp_bitmap.pix(y, x) & 3) != 0;

			// draw bullets
			if (bullet)
			{
				// background vs. bullet collision detection
				if (background) m_collision |= 0x80;

				// draw white 1x4-size bullet
				bitmap.pix(y, x) = BULLET_PEN;
			}

			// copy the S2636 images into the main bitmap and check collision
			int const pixel0 = s2636_0_bitmap.pix(y, x);
			int const pixel1 = s2636_1_bitmap.pix(y, x);
			int const pixel2 = s2636_2_bitmap.pix(y, x);

			int const pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. S2636 collision detection
				if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) m_collision |= 0x01;
				if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) m_collision |= 0x02;
				if (S2636_IS_PIXEL_DRAWN(pixel2) && S2636_IS_PIXEL_DRAWN(pixel0)) m_collision |= 0x04;

				// S2636 vs. bullet collision detection
				if (bullet) m_collision |= 0x08;

				// S2636 vs. background collision detection
				if (background)
				{
					/* bit4 causes problems on 2nd level
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision |= 0x10; */
					if (S2636_IS_PIXEL_DRAWN(pixel1)) m_collision |= 0x20;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision |= 0x40;
				}

				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel) | 0x10;
			}
		}
	}

	return 0;
}


u32 astrowar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// astrowar has only one S2636
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);

	draw_background(screen, bitmap, cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const bullet_pos = (((y < 0x100) ? m_bullet_ram[y] : 0) ^ 0xff) - 8;
			bool const bullet = (bullet_pos != 0xff - 8) && (x <= bullet_pos && x > bullet_pos - 4);

			// draw bullets first
			if (bullet)
			{
				// background vs. bullet collision detection
				if (m_temp_bitmap.pix(y, x) & 1)
					m_collision |= 0x02;

				// draw white 1x4-size bullet
				bitmap.pix(y, x) = BULLET_PEN;
			}
		}

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// NOTE: similar to zac1b1120.cpp, the sprite chip runs at a different frequency than the background generator
			// the exact timing ratio is unknown, so we'll have to do with guesswork
			float const s_ratio = 256.0f / 196.0f;

			float const sx = x * s_ratio;
			if (int(sx + 0.5f) > cliprect.right())
				break;

			// copy the S2636 bitmap into the main bitmap and check collision
			int const pixel = s2636_0_bitmap.pix(y, x);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. bullet collision detection N/A

				// S2636 vs. background collision detection
				if ((m_temp_bitmap.pix(y, int(sx)) | m_temp_bitmap.pix(y, int(sx + 0.5f))) & 1)
					m_collision |= 0x01;

				bitmap.pix(y, int(sx)) = S2636_PIXEL_COLOR(pixel) | 0x10;
				bitmap.pix(y, int(sx + 0.5f)) = S2636_PIXEL_COLOR(pixel) | 0x10;
			}
		}
	}

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

template <u8 Which>
void galaxia_state::video_w(offs_t offset, u8 data)
{
	m_bg_tilemap->mark_tile_dirty(offset);
	Which ? m_video_ram[offset] = data : m_color_ram[offset] = data;
}

void galaxia_state::scroll_w(u8 data)
{
	// fixed scrolling area
	for (int i = 1; i < 6; i++)
		m_bg_tilemap->set_scrolly(i, data);
}

u8 galaxia_state::collision_r()
{
	return m_collision;
}

u8 galaxia_state::collision_clear_r()
{
	if (!machine().side_effects_disabled())
		m_collision = 0;

	return 0;
}

void galaxia_state::ctrlport_w(u8 data)
{
	// d0: triggers on every new credit
	// d1: coin counter? if you put a coin in slot A, galaxia constantly
	// strobes sets and clears the bit. if you put a coin in slot B
	// however, the bit is set and cleared only once.

	// d3: flip screen?
	//flip_screen_set(BIT(data, 3));

	// d5: enable stars
	m_stars_on = bool(BIT(data, 5));

	// other bits: unknown
}

void galaxia_state::dataport_w(u8 data)
{
	// seems to be related to sound board comms
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void galaxia_state::galaxia_mem(address_map &map)
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
}

void galaxia_state::galaxiaa_mem(address_map &map)
{
	galaxia_state::galaxia_mem(map);
	map(0x7214, 0x7214).portr("IN0");
}

void astrowar_state::astrowar_mem(address_map &map)
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

void galaxia_state::galaxia_io(address_map &map)
{
	map.global_mask(0x07);
	map(0x00, 0x00).w(FUNC(galaxia_state::scroll_w)).portr("IN0");
	map(0x02, 0x02).portr("IN1");
	map(0x05, 0x05).lr8(NAME([] () { return 0xff; }));
	map(0x06, 0x06).portr("DSW0");
	map(0x07, 0x07).portr("DSW1");
}

void galaxia_state::galaxiaa_io(address_map &map)
{
	galaxia_state::galaxia_io(map);
	map(0x00, 0x00).unmapr();
}

void galaxia_state::galaxia_data(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(galaxia_state::collision_r), FUNC(galaxia_state::ctrlport_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(galaxia_state::collision_clear_r), FUNC(galaxia_state::dataport_w));
}



/*******************************************************************************
    Inputs
*******************************************************************************/

static INPUT_PORTS_START( galaxia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("3N:7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coin_B ) )     PORT_DIPLOCATION("3N:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("3N:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("3N:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "High Score" )          PORT_DIPLOCATION("2N:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Random" )
	PORT_DIPNAME( 0x06, 0x00, "Random H.S." )         PORT_DIPLOCATION("2N:6,5") // only if high score is set to random
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, "Medium-High" )
	PORT_DIPSETTING(    0x06, DEF_STR( High ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("2N:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x08, "2500" )
	PORT_DIPSETTING(    0x10, "3500" )
	PORT_DIPSETTING(    0x18, "5500" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )    PORT_DIPLOCATION("2N:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( galaxiaa )
	PORT_INCLUDE( galaxia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( astrowar )
	PORT_INCLUDE( galaxia )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("2N:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x18, "7000" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void galaxia_state::galaxia(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, 14.318181_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxia_state::galaxia_mem);
	m_maincpu->set_addrmap(AS_IO, &galaxia_state::galaxia_io);
	m_maincpu->set_addrmap(AS_DATA, &galaxia_state::galaxia_data);
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));
	m_maincpu->flag_handler().set([this] (int state) { m_ram_view.select(state); });
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(3500));
	m_screen->set_size(256, 312);
	m_screen->set_visarea(0*8, 29*8-1, 0*8, 34*8-1);
	m_screen->set_screen_update(FUNC(galaxia_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);
	m_screen->screen_vblank().append(FUNC(galaxia_state::scroll_stars));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxia);
	PALETTE(config, m_palette, FUNC(galaxia_state::palette), 0x18 + 0x40 + 1);

	S2636(config, m_s2636[0], 0);
	m_s2636[0]->set_offsets(3, -26);
	m_s2636[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[1], 0);
	m_s2636[1]->set_offsets(3, -26);
	m_s2636[1]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[2], 0);
	m_s2636[2]->set_offsets(3, -26);
	m_s2636[2]->add_route(ALL_OUTPUTS, "mono", 0.25);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}

void galaxia_state::galaxiaa(machine_config &config)
{
	galaxia(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxia_state::galaxiaa_mem);
	m_maincpu->set_addrmap(AS_IO, &galaxia_state::galaxiaa_io);
}

void astrowar_state::astrowar(machine_config &config)
{
	galaxia(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &astrowar_state::astrowar_mem);

	// video hardware
	GFXDECODE(config.replace(), m_gfxdecode, m_palette, gfx_astrowar);

	m_s2636[0]->set_offsets(3, -8);
	config.device_remove("s2636_1");
	config.device_remove("s2636_2");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( galaxia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(1d22219b) SHA1(6ab8ea8c78db30d80de98879018726d0420d30fe) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) )
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.3d", 0x00000, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from galaxiaa
	ROM_LOAD( "galaxia.1d", 0x00400, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from galaxiaa

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) ) // colors + priority
ROM_END

ROM_START( galaxiaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(1520eb3d) SHA1(3683174da701e1124af0f9c2ee4a9a84f3fea33a) )
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(1d22219b) SHA1(6ab8ea8c78db30d80de98879018726d0420d30fe) )
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(c0baa654) SHA1(80e0880c32ad285fbce0f7f552268b964b97cab3) ) // sldh
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(cf653b9a) SHA1(fef5943de60cb5ba2459fc6ae7419e29c96a76cd) )
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(50c6a645) SHA1(46638907bc393df6be25fc7461d73047d1746ffc) )
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.3d", 0x00000, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) )
	ROM_LOAD( "galaxia.1d", 0x00400, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) ) // colors + priority
ROM_END

ROM_START( galaxiab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galaxia.8h",  0x00000, 0x0400, CRC(f3b4ffde) SHA1(15b004e7821bfc145158b1e9435f061c524f6b86) )
	ROM_LOAD( "galaxia.10h", 0x00400, 0x0400, CRC(6d07fdd4) SHA1(d7d4b345a055275d59951788569db370bccd5195) )
	ROM_LOAD( "galaxia.11h", 0x00800, 0x0400, CRC(5682d56f) SHA1(15afb3296e93f8371d36b686ce372f917bd5b771) ) // sldh
	ROM_LOAD( "galaxia.13h", 0x00c00, 0x0400, CRC(80dafe84) SHA1(8a71a05f1b0ddba36bf748a4801f3a78f63af1db) ) // sldh
	ROM_LOAD( "galaxia.8i",  0x01000, 0x0400, CRC(45b88599) SHA1(3b79c21db1aa9d80fac81ac5a554e438805febd1) )
	ROM_LOAD( "galaxia.10i", 0x02000, 0x0400, CRC(76bd9fe3) SHA1(1abc8e40063aaa9140ea5e0341127eb0a7e86c88) )
	ROM_LOAD( "galaxia.11i", 0x02400, 0x0400, CRC(4456808a) SHA1(f9e8cfdde0e17f13f1be297b2b4503ccc959b33c) )
	ROM_LOAD( "galaxia.13i", 0x02800, 0x0400, CRC(ffe86fdb) SHA1(67b02a5c39dbe515b6d68583c8831b0dae15374a) ) // sldh
	ROM_LOAD( "galaxia.11l", 0x02c00, 0x0400, CRC(8e3f5343) SHA1(6298be9bb33975854cb3d009b89913b1a8018aee) ) // sldh
	ROM_LOAD( "galaxia.13l", 0x03000, 0x0400, CRC(3a9c38c7) SHA1(d1e934092b69c0f3f9636eba05a1d8a6d9588e6b) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "galaxia.3d", 0x00000, 0x0400, CRC(1dc30185) SHA1(e3c75eecb80b376ece98f602e1b9587487841824) ) // taken from galaxiaa
	ROM_LOAD( "galaxia.1d", 0x00400, 0x0400, CRC(2dd50aab) SHA1(758d7a5383c9a1ee134d99e3f7025819cfbe0e0f) ) // taken from galaxiaa

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.11o", 0x0000, 0x0200, CRC(ae816417) SHA1(9497857d13c943a2735c3b85798199054e613b2c) ) // colors + priority
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
	ROM_LOAD( "astro.13i", 0x02800, 0x0400, CRC(8d1575e0) SHA1(3d7f65ecf786704ebcd20cfaa2479ea24fd4e739) )
	ROM_LOAD( "astro.11l", 0x02c00, 0x0400, CRC(29f52f57) SHA1(5cb50b82e09c537eeaeae167351fca686fde8228) )
	ROM_LOAD( "astro.13l", 0x03000, 0x0400, CRC(882cdb87) SHA1(062ee8d296316cbce2eb62e72774aa4181e9847d) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "astro.1d",  0x00000, 0x0400, CRC(6053f834) SHA1(e0b76800c241b3c8010c09869cecbc109b25310a) )
	ROM_LOAD( "astro.3d",  0x00400, 0x0400, CRC(822505aa) SHA1(f9d3465e14bb850a286f8b4f42aa0a4044413b67) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Game Drivers
*******************************************************************************/

//    YEAR, NAME,     PARENT,  MACHINE,  INPUT,    CLASS,          INIT,       SCREEN, COMPANY,            FULLNAME,          FLAGS
GAME( 1979, galaxia,  0,       galaxia,  galaxia,  galaxia_state,  empty_init, ROT90,  "Zaccaria / Zelco", "Galaxia (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiaa, galaxia, galaxiaa, galaxiaa, galaxia_state,  empty_init, ROT90,  "Zaccaria / Zelco", "Galaxia (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // protected?
GAME( 1979, galaxiab, galaxia, galaxia,  galaxia,  galaxia_state,  empty_init, ROT90,  "Zaccaria / Zelco", "Galaxia (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, astrowar, 0,       astrowar, astrowar, astrowar_state, empty_init, ROT90,  "Zaccaria / Zelco", "Astro Wars",      MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
