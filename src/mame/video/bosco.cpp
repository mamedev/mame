// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/galaga.h"


#define MAX_STARS 252
#define STARS_COLOR_BASE (64*4+64*4+4)
#define VIDEO_RAM_SIZE 0x400

PALETTE_INIT_MEMBER(bosco_state,bosco)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* core palette */
	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		bit0 = ((*color_prom) >> 0) & 0x01;
		bit1 = ((*color_prom) >> 1) & 0x01;
		bit2 = ((*color_prom) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = ((*color_prom) >> 3) & 0x01;
		bit1 = ((*color_prom) >> 4) & 0x01;
		bit2 = ((*color_prom) >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = ((*color_prom) >> 6) & 0x01;
		bit2 = ((*color_prom) >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i,rgb_t(r,g,b));
		color_prom++;
	}

	/* palette for the stars */
	for (i = 0;i < 64;i++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];

		palette.set_indirect_color(32 + i,rgb_t(r,g,b));
	}

	/* characters / sprites */
	for (i = 0;i < 64*4;i++)
	{
		palette.set_pen_indirect(i, (color_prom[i] & 0x0f) + 0x10); /* chars */
		palette.set_pen_indirect(i+64*4, color_prom[i] & 0x0f); /* sprites */
	}

	/* bullets lookup table */
	/* they use colors 28-31, I think - PAL 5A controls it */
	for (i = 0;i < 4;i++)
		palette.set_pen_indirect(64*4+64*4+i, 31-i);

	/* now the stars */
	for (i = 0;i < 64;i++)
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
	UINT8 attr = m_videoram[ram_offs + tile_index + 0x800];
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
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bosco_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bosco_state::fg_get_tile_info),this),tilemap_mapper_delegate(FUNC(bosco_state::fg_tilemap_scan),this),  8,8, 8,32);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);
	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x1f);

	m_bg_tilemap->set_scrolldx(3,3);

	m_spriteram = m_videoram + 0x03d4;
	m_spriteram_size = 0x0c;
	m_spriteram2 = m_spriteram + 0x0800;
	m_bosco_radarx = m_videoram + 0x03f0;
	m_bosco_radary = m_bosco_radarx + 0x0800;

	save_item(NAME(m_stars_scrollx));
	save_item(NAME(m_stars_scrolly));
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
}



/***************************************************************************

  Display refresh

***************************************************************************/

void bosco_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
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
	if (1)
	{
		int star_cntr;
		int set_a, set_b;

		/* two sets of stars controlled by these bits */
		set_a = (m_bosco_starblink[0] & 1);
		set_b = (m_bosco_starblink[1] & 1) | 2;

		for (star_cntr = 0;star_cntr < MAX_STARS;star_cntr++)
		{
			int x,y;

			if ( (set_a == m_star_seed_tab[star_cntr].set) || ( set_b == m_star_seed_tab[star_cntr].set) )
			{
				x = (m_star_seed_tab[star_cntr].x + m_stars_scrollx) % 256;
				y = (m_star_seed_tab[star_cntr].y + m_stars_scrolly) % 256;

				/* don't draw the stars that are off the screen */
				if ( x < 224 )
				{
					if (flip) x += 64;

					if (cliprect.contains(x, y))
						bitmap.pix16(y, x) = STARS_COLOR_BASE + m_star_seed_tab[star_cntr].col;
				}
			}
		}
	}
}


UINT32 bosco_state::screen_update_bosco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


void bosco_state::screen_eof_bosco(screen_device &screen, bool state)
{
	// falling edge
	if (!state)
	{
		static const int speedsx[8] = { -1, -2, -3, 0, 3, 2, 1, 0 };
		static const int speedsy[8] = { 0, -1, -2, -3, 0, 3, 2, 1 };

		m_stars_scrollx += speedsx[m_bosco_starcontrol[0] & 0x07];
		m_stars_scrolly += speedsy[(m_bosco_starcontrol[0] & 0x38) >> 3];
	}
}
