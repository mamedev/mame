// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "ladybug.h"
#include "includes/ladybug.h"

#include "video/resnet.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(LADYBUG_VIDEO,  ladybug_video_device,  "ladybug_video",  "Lady Bug/Space Raider background")
DEFINE_DEVICE_TYPE(ZEROHOUR_STARS, zerohour_stars_device, "zerohour_stars", "Zero Hour/Red Clash/Space Raider starfield")


/***************************************************************************

  ladybug/sraider video

***************************************************************************/

ladybug_video_device::ladybug_video_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LADYBUG_VIDEO, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_spr_ram()
	, m_bg_ram()
	, m_bg_tilemap(nullptr)
{
}

WRITE8_MEMBER(ladybug_video_device::bg_w)
{
	m_bg_ram[offset & 0x07ff] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x03ff);
}

void ladybug_video_device::draw(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect, bool flip)
{
	// TODO: confirm whether sraider hardware actually does this - not used by the game
	for (unsigned offs = 0; offs < 32; ++offs)
	{
		int const scroll = m_bg_ram[((offs & 0x03) << 5) | (offs >> 2)];
		m_bg_tilemap->set_scrollx(offs, flip ? -scroll : scroll);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0);

	for (int offs = 0x400 - (0x40 << 1); (0x40 << 1) <= offs; offs -= 0x40)
	{
		int i = 0;
		while ((0x40 > i) && m_spr_ram[offs + i])
			i += 4;

		while (0 < i)
		{
			/*
			 abccdddd eeeeeeee fffghhhh iiiiiiii

			 a enable?
			 b size (0 = 8x8, 1 = 16x16)
			 cc flip
			 dddd y offset
			 eeeeeeee sprite code (shift right 2 bits for 16x16 sprites)
			 fff unknown
			 g sprite bank
			 hhhh color
			 iiiiiiii x position
			*/
			i -= 4;
			bool const enable(m_spr_ram[offs + i] & 0x80);
			if (enable)
			{
				bool const big(m_spr_ram[offs + i] & 0x40);
				bool const xflip(m_spr_ram[offs + i] & 0x20);
				bool const yflip(m_spr_ram[offs + i] & 0x10);
				int const code(m_spr_ram[offs + i + 1] | (BIT(m_spr_ram[offs + i + 2], 4) << 8));
				int const color(m_spr_ram[offs + i + 2] & 0x0f);
				int const xpos(m_spr_ram[offs + i + 3]);
				int const ypos((offs >> 2) | (m_spr_ram[offs + i] & 0x0f));
				if (big) // 16x16
					m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code >> 2, color, xflip, yflip, xpos, ypos - 8, 0);
				else // 8x8
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, xflip, yflip, xpos, ypos, 0);
			}
		}
	}
}

void ladybug_video_device::device_start()
{
	m_spr_ram = std::make_unique<u8 []>(0x0400);
	m_bg_ram = std::make_unique<u8 []>(0x0800);
	std::fill_n(m_spr_ram.get(), 0x0400, 0);
	std::fill_n(m_bg_ram.get(), 0x0800, 0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ladybug_video_device::get_bg_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(32);
	m_bg_tilemap->set_transparent_pen(0);

	save_pointer(NAME(m_spr_ram), 0x0400);
	save_pointer(NAME(m_bg_ram), 0x0800);
}

TILE_GET_INFO_MEMBER(ladybug_video_device::get_bg_tile_info)
{
	int const code = m_bg_ram[tile_index] + (BIT(m_bg_ram[0x0400 | tile_index], 3) << 8);
	int const color = m_bg_ram[0x0400 | tile_index] & 0x07;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


/***************************************************************************

  zerohour/redclash/sraider stars

  These functions emulate the star generator board
  All this comes from the schematics for Zero Hour

  It has a 17-bit LFSR which has a period of 2^17-1 clocks
  (This is one pixel shy of "two screens" worth.)
  So, there are two starfields drawn on alternate frames
  These will scroll at a rate controlled by the speed register

  I'm basically doing the same thing by drawing each
  starfield on alternate frames, and then offseting them

***************************************************************************/

zerohour_stars_device::zerohour_stars_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZEROHOUR_STARS, tag, owner, clock)
	, m_enable(0)
	, m_speed(0)
	, m_offset(0)
	, m_count(0)
{
}

// This line can reset the LFSR to zero and disables the star generator
void zerohour_stars_device::set_enable(bool on)
{
	if (!m_enable && on)
		m_offset = 0;

	m_enable = on ? 1 : 0;
}

// This sets up which starfield to draw and the offset, to be called from screen_vblank_*()
void zerohour_stars_device::update_state()
{
	if (m_enable)
	{
		m_count = m_count ? 0 : 1;
		if (!m_count)
		{
			m_offset += ((m_speed * 2) - 0x09);
			m_offset %= 256 * 256;
			m_state = 0;
		}
		else
		{
			m_state = 0x1fc71;
		}
	}
}

// Set the speed register (3 bits)
void zerohour_stars_device::set_speed(u8 speed, u8 mask)
{
	// 0 left/down fastest (-9/2 pix per frame)
	// 1 left/down faster  (-7/2 pix per frame)
	// 2 left/down fast    (-5/2 pix per frame)
	// 3 left/down medium  (-3/2 pix per frame)
	// 4 left/down slow    (-1/2 pix per frame)
	// 5 right/up slow     (+1/2 pix per frame)
	// 6 right/up medium   (+3/2 pix per frame)
	// 7 right/up fast     (+5/2 pix per frame)
	m_speed = (m_speed & ~mask) | (speed & mask);
}

// Draw the stars
// Space Raider doesn't use the Va bit, and it is also set up to window the stars to a certain x range
void zerohour_stars_device::draw(bitmap_ind16 &bitmap, rectangle const &cliprect, u8 pal_offs, bool has_va, u8 firstx, u8 lastx)
{
	if (m_enable)
	{
		u32 state(m_state);
		for (int i = 0; (256 * 256) > i; ++i)
		{
			u8 const xloc((m_offset + i) & 0x00ff);
			u8 const yloc(((m_offset + i) >> 8) & 0x00ff);

			bool const tempbit(!(state & 0x10000));
			bool const feedback((state & 0x00020) ? !tempbit : tempbit);

			bool const hcond(BIT(xloc + 8, 4));
			bool const vcond(!has_va || BIT(yloc, 0));

			if (cliprect.contains(xloc, yloc) && (hcond == vcond))
			{
				if (((state & 0x000ff) == 0x000ff) && !feedback && (xloc >= firstx) && (xloc <= lastx))
					bitmap.pix16(yloc, xloc) = pal_offs + ((state >> 9) & 0x1f);
			}

			// update LFSR state
			state = ((state << 1) & 0x1fffe) | (feedback ? 1 : 0);
		}
	}
}

void zerohour_stars_device::device_start()
{
	save_item(NAME(m_enable));
	save_item(NAME(m_speed));
	save_item(NAME(m_state));
	save_item(NAME(m_offset));
	save_item(NAME(m_count));
}

void zerohour_stars_device::device_reset()
{
	m_enable = 0;
	m_speed = 0;
	m_state = 0;
	m_offset = 0;
	m_count = 0;
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Lady Bug has a 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- inverter -- 220 ohm resistor  -- BLUE
        -- inverter -- 220 ohm resistor  -- GREEN
        -- inverter -- 220 ohm resistor  -- RED
        -- inverter -- 470 ohm resistor  -- BLUE
        -- unused
        -- inverter -- 470 ohm resistor  -- GREEN
        -- unused
  bit 0 -- inverter -- 470 ohm resistor  -- RED

***************************************************************************/
/***************************************************************************

  Convert the color PROMs into a more useable format.

  Lady Bug has a 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- inverter -- 220 ohm resistor  -- BLUE
        -- inverter -- 220 ohm resistor  -- GREEN
        -- inverter -- 220 ohm resistor  -- RED
        -- inverter -- 470 ohm resistor  -- BLUE
        -- unused
        -- inverter -- 470 ohm resistor  -- GREEN
        -- unused
  bit 0 -- inverter -- 470 ohm resistor  -- RED

***************************************************************************/

void ladybug_base_state::palette_init_common(palette_device &palette, const uint8_t *color_prom,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1) const
{
	static constexpr int resistances[2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[2], gweights[2], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			2, resistances, rweights, 470, 0,
			2, resistances, gweights, 470, 0,
			2, resistances, bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(~color_prom[i], r_bit0);
		bit1 = BIT(~color_prom[i], r_bit1);
		int const r = combine_weights(rweights, bit0, bit1);

		// green component
		bit0 = BIT(~color_prom[i], g_bit0);
		bit1 = BIT(~color_prom[i], g_bit1);
		int const g = combine_weights(gweights, bit0, bit1);

		// blue component
		bit0 = BIT(~color_prom[i], b_bit0);
		bit1 = BIT(~color_prom[i], b_bit1);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// characters
	for (int i = 0; i < 0x20; i++)
	{
		uint8_t const ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites
	for (int i = 0; i < 0x20; i++)
	{
		uint8_t ctabentry;

		ctabentry = bitswap<4>((color_prom[i] >> 0) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x20, ctabentry);

		ctabentry = bitswap<4>((color_prom[i] >> 4) & 0x0f, 0,1,2,3);
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}
}


void ladybug_state::ladybug_palette(palette_device &palette) const
{
	palette_init_common(palette, memregion("proms")->base(), 0, 5, 2, 6, 4, 7);
}

WRITE_LINE_MEMBER(ladybug_state::flipscreen_w)
{
	if (flip_screen() != state)
	{
		flip_screen_set(state);
		machine().tilemap().mark_all_dirty();
	}
}

uint32_t ladybug_state::screen_update_ladybug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_video->draw(screen, bitmap, cliprect, flip_screen());
	return 0;
}


void sraider_state::sraider_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// the resistor net may be probably different than Lady Bug
	palette_init_common(palette, color_prom, 3, 0, 5, 4, 7, 6);

	// star colors
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1;

		// red component
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		int const b = 0x47 * bit0 + 0x97 * bit1;

		// green component
		bit0 = BIT(i, 1);
		bit1 = BIT(i, 2);
		int const g = 0x47 * bit0 + 0x97 * bit1;

		// blue component
		bit0 = BIT(i, 0);
		int const r = 0x47 * bit0;

		palette.set_indirect_color(i + 0x20, rgb_t(r, g, b));
	}

	for (int i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i + 0x60, i + 0x20);

	// stationary part of grid
	palette.set_pen_indirect(0x81, 0x40);
}

TILE_GET_INFO_MEMBER(sraider_state::get_grid_tile_info)
{
	if (tile_index < 512)
	{
		SET_TILE_INFO_MEMBER(3, tile_index, 0, 0);
	}
	else
	{
		int temp = tile_index / 32;
		tile_index = (31 - temp) * 32 + (tile_index % 32);
		SET_TILE_INFO_MEMBER(4, tile_index, 0, 0);
	}
}

WRITE8_MEMBER(sraider_state::sraider_io_w)
{
	// bit7 = flip
	// bit6 = grid red
	// bit5 = grid green
	// bit4 = grid blue
	// bit3 = enable stars
	// bit210 = stars speed/dir

	if (flip_screen() != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}

	m_grid_color = data & 0x70;

	m_stars->set_enable(BIT(data, 3));

	// There must be a subtle clocking difference between
	// Space Raider and the other games using this star generator,
	// hence the -1 here
	m_stars->set_speed((data & 0x07) - 1, 0x07);
}

void sraider_state::video_start()
{
	ladybug_base_state::video_start();

	m_grid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sraider_state::get_grid_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_grid_tilemap->set_scroll_rows(32);
	m_grid_tilemap->set_transparent_pen(0);
}

WRITE_LINE_MEMBER(sraider_state::screen_vblank_sraider)/* update starfield position */
{
	// falling edge
	if (!state)
		m_stars->update_state();
}

uint32_t sraider_state::screen_update_sraider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// clear the bg bitmap
	bitmap.fill(0, cliprect);

	// draw the stars
	if (flip_screen())
		m_stars->draw(bitmap, cliprect, 0x60, false, 0x27, 0xff);
	else
		m_stars->draw(bitmap, cliprect, 0x60, false, 0x00, 0xd8);

	// draw the gridlines
	m_palette->set_indirect_color(0x40, rgb_t(
			m_grid_color & 0x40 ? 0xff : 0,
			m_grid_color & 0x20 ? 0xff : 0,
			m_grid_color & 0x10 ? 0xff : 0));
	m_grid_tilemap->draw(screen, bitmap, cliprect, 0, flip_screen());

	for (unsigned i = 0; i < 0x100; i++)
	{
		if (m_grid_data[i] != 0)
		{
			uint8_t x = i;
			int height = cliprect.max_y - cliprect.min_y + 1;

			if (flip_screen())
				x = ~x;

			bitmap.plot_box(x, cliprect.min_y, 1, height, 0x81);
		}
	}

	// now the chars/sprites
	m_video->draw(screen, bitmap, cliprect, flip_screen());

	return 0;
}
