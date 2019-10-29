// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/galaga.h"
#include "includes/bosco.h"


#define STARS_COLOR_BASE (64*4+64*4+4)
#define VIDEO_RAM_SIZE 0x400

#define LFSR_SEED 0x7FFF


void bosco_state::bosco_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// core palette
	for (int i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
		color_prom++;
	}

	// palette for the stars
	for (int i = 0; i < 64; i++)
	{
		static constexpr int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		int const r = map[(i >> 0) & 0x03];
		int const g = map[(i >> 2) & 0x03];
		int const b = map[(i >> 4) & 0x03];

		palette.set_indirect_color(32 + i, rgb_t(r, g, b));
	}

	// characters / sprites
	for (int i = 0; i < 64*4; i++)
	{
		palette.set_pen_indirect(i, (color_prom[i] & 0x0f) | 0x10); // chars
		palette.set_pen_indirect(i + 64*4, color_prom[i] & 0x0f); // sprites
	}

	// bullets lookup table
	// they use colors 28-31, I think - PAL 5A controls it
	for (int i = 0; i < 4; i++)
		palette.set_pen_indirect(64*4+64*4+i, 31 - i);

	// now the stars
	for (int i = 0; i < 64; i++)
		palette.set_pen_indirect(64*4+64*4+4+i, 32 + i);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* the video RAM has space for 32x32 tiles and is only partially used for the radar */
TILEMAP_MAPPER_MEMBER(bosco_state::fg_tilemap_scan )
{
	return col + (row << 5);
}


inline void bosco_state::get_tile_info_bosco(tile_data &tileinfo,int tile_index,int ram_offs)
{
	uint8_t attr = m_videoram[ram_offs + tile_index + 0x800];
	tileinfo.category = (attr & 0x20) >> 5;
	tileinfo.group = attr & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[ram_offs + tile_index],
			attr & 0x3f,
			TILE_FLIPYX(attr >> 6) ^ TILE_FLIPX);
}

TILE_GET_INFO_MEMBER(bosco_state::bg_get_tile_info )
{
	get_tile_info_bosco(tileinfo,tile_index,0x400);
}

TILE_GET_INFO_MEMBER(bosco_state::fg_get_tile_info )
{
	get_tile_info_bosco(tileinfo,tile_index,0x000);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(bosco_state,bosco)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bosco_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bosco_state::fg_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(bosco_state::fg_tilemap_scan)), 8,8, 8,32);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_bg_tilemap->set_scrolldx(3,3);

	m_spriteram = m_videoram + 0x03d4;
	m_spriteram_size = 0x0c;
	m_spriteram2 = m_spriteram + 0x0800;
	m_bosco_radarx = m_videoram + 0x03f0;
	m_bosco_radary = m_bosco_radarx + 0x0800;

	m_lfsr = LFSR_SEED;
	m_stars_scroll_index_x = 0;
	m_stars_scroll_index_y = 0;
	m_bosco_starclr = 1;

	save_item(NAME(m_lfsr));
	save_item(NAME(m_stars_scroll_index_x));
	save_item(NAME(m_stars_scroll_index_y));
	save_item(NAME(m_bosco_starclr));

}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER( bosco_state::bosco_videoram_w )
{
	m_videoram[offset] = data;
	if (offset & 0x400)
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
	else
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER( bosco_state::bosco_scrollx_w )
{
	m_bg_tilemap->set_scrollx(0,data);
}

WRITE8_MEMBER( bosco_state::bosco_scrolly_w )
{
	m_bg_tilemap->set_scrolly(0,data);
}

WRITE8_MEMBER( bosco_state::bosco_starclr_w )
{
	m_bosco_starclr = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

void bosco_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	uint8_t *spriteram = m_spriteram;
	uint8_t *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = 0;offs < m_spriteram_size;offs += 2)
	{
		int sx = spriteram[offs + 1] - 1;
		int sy = 240 - spriteram_2[offs];
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;
		int color = spriteram_2[offs + 1] & 0x3f;

		if (flip) sx += 32-2;

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x0f));
	}
}


void bosco_state::draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	int offs;

	for (offs = 4; offs < 0x10;offs++)
	{
		int x = m_bosco_radarx[offs] + ((~m_bosco_radarattr[offs] & 0x01) << 8) - 2;
		int y = 251 - m_bosco_radary[offs];

		if (flip)
		{
			x -= 1;
			y += 2;
		}

		m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
				((m_bosco_radarattr[offs] & 0x0e) >> 1) ^ 0x07,
				0,
				!flip,!flip,
				x,y,0xf0);
	}
}


void bosco_state::draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	int set_a,set_b;  // star sets

	uint32_t pre_vis_cycle_count = pre_vis_cycle_count_values[m_stars_scroll_index_y];
	uint32_t post_vis_cycle_count = post_vis_cycle_count_values[m_stars_scroll_index_y];

	// X scrolling adjustment occurs during pre-visible portion
	pre_vis_cycle_count += speed_X_cycle_count_offset[m_stars_scroll_index_x];

	/* two sets of stars controlled by these bits */
	set_a = m_videolatch->q4_r();
	set_b = m_videolatch->q5_r() | 2;

	/* $9840 controls the stars ON/OFF */
	if (!m_bosco_starclr)
	{
		int x,y;

		// Advance the LFSR during the pre-visible portion of the frame
		do { m_lfsr = get_next_lfsr_state(m_lfsr); } while (--pre_vis_cycle_count);

		// Now we are in visible portion of the frame - Output all LFSR hits here
		for (y=STARFIELD_Y_OFFSET_BOSCO;y<VISIBLE_LINES+STARFIELD_Y_OFFSET_BOSCO;y++)
		{
			// The starfield sits between X pos 0...255
			for (x=0;x<STARFIELD_PIXEL_WIDTH;x++)
			{
				// Check lfsr for hit
				if ((m_lfsr&LFSR_HIT_MASK) == LFSR_HIT_VALUE)
				{
					int star_set = ((m_lfsr>>9)&0x2) | ((m_lfsr>>8)&0x1);

					if ((set_a == star_set) || (set_b == star_set))
					{
						// don't draw the stars that are off the screen
						if (x < 224)
						{
							int dx = x;

							if (flip) dx += 64;

							if (cliprect.contains(dx, y))
							{
								uint8_t color;

								color  = (m_lfsr>>5)&0x7;
								color |= (m_lfsr<<3)&0x18;
								color |= (m_lfsr<<2)&0x20;
								color = (~color)&0x3F;

								bitmap.pix16(y, dx) = STARS_COLOR_BASE + color;
							}
						}
					}
				}

				// Advance LFSR
				m_lfsr = get_next_lfsr_state(m_lfsr);
			}
		}

		// Advance the LFSR during the post-visible portion of the frame
		do { m_lfsr = get_next_lfsr_state(m_lfsr); } while (--post_vis_cycle_count);
	}
	else
	{
		// _STARCLR is high - reset the lfsr
		m_lfsr = LFSR_SEED;
	}
}


uint32_t bosco_state::screen_update_bosco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
	   the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = cliprect;
	rectangle bg_clip = cliprect;
	int flip = flip_screen();
	if (flip)
	{
		bg_clip.min_x = 8*8;
		fg_clip.max_x = 8*8-1;
	}
	else
	{
		bg_clip.max_x = 28*8-1;
		fg_clip.min_x = 28*8;
	}

	bitmap.fill(m_palette->black_pen(), cliprect);
	draw_stars(bitmap,cliprect,flip);

	m_bg_tilemap->draw(screen, bitmap, bg_clip, 0,0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 0,0);

	draw_sprites(bitmap,cliprect,flip);

	/* draw the high priority characters */
	m_bg_tilemap->draw(screen, bitmap, bg_clip, 1,0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 1,0);

	draw_bullets(bitmap,cliprect,flip);

	return 0;
}


WRITE_LINE_MEMBER(bosco_state::screen_vblank_bosco)
{
	// falling edge
	if (!state)
	{
		m_stars_scroll_index_x = m_bosco_starcontrol[0] & 0x07;
		m_stars_scroll_index_y = (m_bosco_starcontrol[0] & 0x38) >> 3;
	}
}
