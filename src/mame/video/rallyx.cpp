// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

The video mixer of this hardware is peculiar.

There are two 16-colors palette banks: the first is used for characters and
sprites, the second for "bullets".

When a bullet is on screen, it selects the second palette bank and replaces
the bottom 2 bits of the tile palette entry with its own, while leaving the
other 2 bits untouched. Therefore, in theory a bullet could have 4 different
colors depending on the color of the background it is drawn over; but none
of the games use this peculiarity, since the bullet palette is just the same
colors repeated four time. This is NOT emulated.

When there is a sprite under the bullet, the palette bank is changed, but the
palette entry number is NOT changed; therefore, the sprite pixels that are
covered by the bullet just change bank. This is emulated by first drawing the
bullets normally, then drawing the sprites (with pdrawgfx so they are not
drawn over high priority tiles), then drawing the pullets again with
drawgfx_transtable mode, so that bullets not covered by sprites remain
the same while the others alter the sprite color.


The tile/sprite priority is controlled by the top bit of the tile color code.
This feature seems to be disabled in Jungler, probably because that game
needs more color combination to render its graphics.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/rallyx.h"

#define STARS_COLOR_BASE    (0x104)


/***************************************************************************

  Convert the color PROMs.

  Rally X has one 32x8 palette PROM and one 256x4 color lookup table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  In Rally-X there is a 1 kohm pull-down on B only, in Locomotion the
  1 kohm pull-down is an all three RGB outputs.

***************************************************************************/

PALETTE_INIT_MEMBER(rallyx_state,rallyx)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights,    0, 0,
			3, &resistances_rg[0], gweights,    0, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* character/sprites lookup table */
	for (i = 0x000; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* bullets use colors 0x10-0x13 */
	for (i = 0x100; i < 0x104; i++)
		palette.set_pen_indirect(i, (i - 0x100) | 0x10);
}


PALETTE_INIT_MEMBER(rallyx_state,jungler)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3]   = { 1000, 470, 220 };
	static const int resistances_b [2]   = { 470, 220 };
	static const int resistances_star[3] = { 150, 100 };
	double rweights[3], gweights[3], bweights[2];
	double rweights_star[2], gweights_star[2], bweights_star[2];
	int i;

	/* compute the color output resistor weights */
	double scale = compute_resistor_weights(0,  255, -1.0,
						2, resistances_star, rweights_star, 0, 0,
						2, resistances_star, gweights_star, 0, 0,
						2, resistances_star, bweights_star, 0, 0);

					compute_resistor_weights(0, 255, scale,
						3, resistances_rg, rweights, 1000, 0,
						3, resistances_rg, gweights, 1000, 0,
						2, resistances_b,  bweights, 1000, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* star pens */
	for (i = 0x20; i < 0x60; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		bit1 = ((i - 0x20) >> 1) & 0x01;
		r = combine_2_weights(rweights_star, bit0, bit1);

		/* green component */
		bit0 = ((i - 0x20) >> 2) & 0x01;
		bit1 = ((i - 0x20) >> 3) & 0x01;
		g = combine_2_weights(gweights_star, bit0, bit1);

		/* blue component */
		bit0 = ((i - 0x20) >> 4) & 0x01;
		bit1 = ((i - 0x20) >> 5) & 0x01;
		b = combine_2_weights(bweights_star, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* character/sprites lookup table */
	for (i = 0x000; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* bullets use colors 0x10-0x13 */
	for (i = 0x100; i < 0x104; i++)
		palette.set_pen_indirect(i, (i - 0x100) | 0x10);

	/* stars */
	for (i = 0x104; i < 0x144; i++)
		palette.set_pen_indirect(i, (i - 0x104) + 0x20);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* the video RAM has space for 32x32 tiles and is only partially used for the radar */
TILEMAP_MAPPER_MEMBER(rallyx_state::fg_tilemap_scan)
{
	return col + (row << 5);
}


inline void rallyx_state::rallyx_get_tile_info( tile_data &tileinfo, int tile_index, int ram_offs)
{
	UINT8 attr = m_videoram[ram_offs + tile_index + 0x800];
	tileinfo.category = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[ram_offs + tile_index],
			attr & 0x3f,
			TILE_FLIPYX(attr >> 6) ^ TILE_FLIPX);
}

TILE_GET_INFO_MEMBER(rallyx_state::rallyx_bg_get_tile_info)
{
	rallyx_get_tile_info(tileinfo, tile_index, 0x400);
}

TILE_GET_INFO_MEMBER(rallyx_state::rallyx_fg_get_tile_info)
{
	rallyx_get_tile_info(tileinfo, tile_index, 0x000);
}


inline void rallyx_state::locomotn_get_tile_info(tile_data &tileinfo,int tile_index,int ram_offs)
{
	UINT8 attr = m_videoram[ram_offs + tile_index + 0x800];
	int code = m_videoram[ram_offs + tile_index];
	code = (code & 0x7f) + 2 * (attr & 0x40) + 2 * (code & 0x80);
	tileinfo.category = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0x3f,
			(attr & 0x80) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}

TILE_GET_INFO_MEMBER(rallyx_state::locomotn_bg_get_tile_info)
{
	locomotn_get_tile_info(tileinfo, tile_index, 0x400);
}

TILE_GET_INFO_MEMBER(rallyx_state::locomotn_fg_get_tile_info)
{
	locomotn_get_tile_info(tileinfo, tile_index, 0x000);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void rallyx_state::calculate_star_field(  )
{
	int generator;
	int x, y;

	/* precalculate the star background */
	m_total_stars = 0;
	generator = 0;

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 288; x++)
		{
			int bit1, bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2)
				generator |= 1;

			if (((~generator >> 16) & 1) && (generator & 0xfe) == 0xfe)
			{
				int color = (~(generator >> 8)) & 0x3f;

				if (color && m_total_stars < JUNGLER_MAX_STARS)
				{
					m_stars[m_total_stars].x = x;
					m_stars[m_total_stars].y = y;
					m_stars[m_total_stars].color = color;

					m_total_stars++;
				}
			}
		}
	}
}

void rallyx_state::rallyx_video_start_common(  )
{
	int i;

	m_spriteram = m_videoram + 0x00;
	m_spriteram2 = m_spriteram + 0x800;
	m_radarx = m_videoram + 0x20;
	m_radary = m_radarx + 0x800;

	for (i = 0; i < 16; i++)
		m_palette->shadow_table()[i] = i + 16;

	for (i = 16; i < 32; i++)
		m_palette->shadow_table()[i] = i;

	for (i = 0; i < 3; i++)
		m_drawmode_table[i] = DRAWMODE_SHADOW;

	m_drawmode_table[3] = DRAWMODE_NONE;
}

VIDEO_START_MEMBER(rallyx_state,rallyx)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::rallyx_bg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::rallyx_fg_get_tile_info),this), tilemap_mapper_delegate(FUNC(rallyx_state::fg_tilemap_scan),this), 8, 8, 8, 32);

	/* the scrolling tilemap is slightly misplaced in Rally X */
	m_bg_tilemap->set_scrolldx(3, 3);

	m_spriteram_base = 0x14;

	rallyx_video_start_common();
}


VIDEO_START_MEMBER(rallyx_state,jungler)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::rallyx_bg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::rallyx_fg_get_tile_info),this), tilemap_mapper_delegate(FUNC(rallyx_state::fg_tilemap_scan),this), 8, 8, 8, 32);

	m_spriteram_base = 0x14;

	rallyx_video_start_common();
	calculate_star_field();
}


VIDEO_START_MEMBER(rallyx_state,locomotn)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::locomotn_bg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::locomotn_fg_get_tile_info),this), tilemap_mapper_delegate(FUNC(rallyx_state::fg_tilemap_scan),this), 8, 8, 8, 32);

	m_spriteram_base = 0x14;

	rallyx_video_start_common();
	calculate_star_field();
}


VIDEO_START_MEMBER(rallyx_state,commsega)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::locomotn_bg_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rallyx_state::locomotn_fg_get_tile_info),this), tilemap_mapper_delegate(FUNC(rallyx_state::fg_tilemap_scan),this), 8, 8, 8, 32);

	/* commsega has more sprites and bullets than the other games */
	m_spriteram_base = 0x00;

	rallyx_video_start_common();
	calculate_star_field();
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(rallyx_state::rallyx_videoram_w)
{
	m_videoram[offset] = data;
	if (offset & 0x400)
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
	else
		m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(rallyx_state::rallyx_scrollx_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(rallyx_state::rallyx_scrolly_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(rallyx_state::tactcian_starson_w)
{
	m_stars_enable = data & 1;
}


void rallyx_state::plot_star( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y, int color )
{
	if (!cliprect.contains(x, y))
		return;

	if (flip_screen_x())
		x = 255 - x;

	if (flip_screen_y())
		y = 255 - y;

	if (m_palette->pen_indirect(bitmap.pix16(y, x) % 0x144) == 0)
		bitmap.pix16(y, x) = STARS_COLOR_BASE + color;
}

void rallyx_state::draw_stars( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_total_stars; offs++)
	{
		int x = m_stars[offs].x;
		int y = m_stars[offs].y;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
			plot_star(bitmap, cliprect, x, y, m_stars[offs].color);
	}
}


void rallyx_state::rallyx_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = 0x20 - 2; offs >= m_spriteram_base; offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1);
		int sy = 241 - spriteram_2[offs];
		int color = spriteram_2[offs + 1] & 0x3f;
		int flipx = spriteram[offs] & 1;
		int flipy = spriteram[offs] & 2;

		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect,
				(spriteram[offs] & 0xfc) >> 2,
				color,
				flipx,flipy,
				sx,sy,
				screen.priority(),0x02,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

void rallyx_state::locomotn_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = 0x20 - 2; offs >= m_spriteram_base; offs -= 2)
	{
		int sx = spriteram[offs + 1] + ((spriteram_2[offs + 1] & 0x80) << 1);
		int sy = 241 - spriteram_2[offs];
		int color = spriteram_2[offs + 1] & 0x3f;
		int flip = spriteram[offs] & 2;

		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect,
				((spriteram[offs] & 0x7c) >> 2) + 0x20*(spriteram[offs] & 0x01) + ((spriteram[offs] & 0x80) >> 1),
				color,
				flip,flip,
				sx,sy,
				screen.priority(),0x02,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

void rallyx_state::rallyx_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen )
{
	int offs;

	for (offs = m_spriteram_base; offs < 0x20; offs++)
	{
		int x, y;

		x = m_radarx[offs] + ((~m_radarattr[offs & 0x0f] & 0x01) << 8);
		y = 253 - m_radary[offs];
		if (flip_screen())
			x -= 3;

		if (transpen)
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					((m_radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			m_gfxdecode->gfx(2)->transtable(bitmap,cliprect,
					((m_radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
					0,
					0,0,
					x,y,
					m_drawmode_table);
	}
}

void rallyx_state::jungler_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen )
{
	int offs;

	for (offs = m_spriteram_base; offs < 0x20; offs++)
	{
		int x, y;

		x = m_radarx[offs] + ((~m_radarattr[offs & 0x0f] & 0x08) << 5);
		y = 253 - m_radary[offs];

		if (transpen)
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					(m_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			m_gfxdecode->gfx(2)->transtable(bitmap,cliprect,
					(m_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					m_drawmode_table);
	}
}

void rallyx_state::locomotn_draw_bullets( bitmap_ind16 &bitmap, const rectangle &cliprect, int transpen )
{
	int offs;

	for (offs = m_spriteram_base; offs < 0x20; offs++)
	{
		int x, y;


		/* it looks like in commsega the addresses used are
		   a000-a003  a004-a00f
		   8020-8023  8034-803f
		   8820-8823  8834-883f
		   so 8024-8033 and 8824-8833 are not used
		*/

		x = m_radarx[offs] + ((~m_radarattr[offs & 0x0f] & 0x08) << 5);
		y = 252 - m_radary[offs];

		if (transpen)
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					(m_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					3);
		else
			m_gfxdecode->gfx(2)->transtable(bitmap,cliprect,
					(m_radarattr[offs & 0x0f] & 0x07) ^ 0x07,
					0,
					0,0,
					x,y,
					m_drawmode_table);
	}
}


UINT32 rallyx_state::screen_update_rallyx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
	   the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = cliprect;
	rectangle bg_clip = cliprect;

	if (flip_screen())
	{
		bg_clip.min_x = 8 * 8;
		fg_clip.max_x = 8 * 8 - 1;
	}
	else
	{
		bg_clip.max_x = 28 * 8 - 1;
		fg_clip.min_x = 28 * 8;
	}

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, bg_clip, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, bg_clip, 1, 1);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 1, 1);

	rallyx_draw_bullets(bitmap, cliprect, TRUE);
	rallyx_draw_sprites(screen, bitmap, cliprect);
	rallyx_draw_bullets(bitmap, cliprect, FALSE);

	return 0;
}


UINT32 rallyx_state::screen_update_jungler(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
	   the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = cliprect;
	rectangle bg_clip = cliprect;

	if (flip_screen())
	{
		bg_clip.min_x = 8 * 8;
		fg_clip.max_x = 8 * 8 - 1;
	}
	else
	{
		bg_clip.max_x = 28 * 8 - 1;
		fg_clip.min_x = 28 * 8;
	}

	screen.priority().fill(0, cliprect);

	/* tile priority doesn't seem to be supported in Jungler */
	m_bg_tilemap->draw(screen, bitmap, bg_clip, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, bg_clip, 1, 0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 1, 0);

	jungler_draw_bullets(bitmap, cliprect, TRUE);
	rallyx_draw_sprites(screen, bitmap, cliprect);
	jungler_draw_bullets(bitmap, cliprect, FALSE);

	if (m_stars_enable)
		draw_stars(bitmap, cliprect);

	return 0;
}


UINT32 rallyx_state::screen_update_locomotn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* the radar tilemap is just 8x32. We rely on the tilemap code to repeat it across
	   the screen, and clip it to only the position where it is supposed to be shown */
	rectangle fg_clip = cliprect;
	rectangle bg_clip = cliprect;

	if (flip_screen())
	{
		/* handle reduced visible area in some games */
		if (screen.visible_area().max_x == 32 * 8 - 1)
		{
			bg_clip.min_x = 4 * 8;
			fg_clip.max_x = 4 * 8 - 1;
		}
		else
		{
			bg_clip.min_x = 8 * 8;
			fg_clip.max_x = 8 * 8-1;
		}
	}
	else
	{
		bg_clip.max_x = 28 * 8 - 1;
		fg_clip.min_x = 28 * 8;
	}

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, bg_clip, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, bg_clip, 1, 1);
	m_fg_tilemap->draw(screen, bitmap, fg_clip, 1, 1);

	locomotn_draw_bullets(bitmap, cliprect, TRUE);
	locomotn_draw_sprites(screen, bitmap, cliprect);
	locomotn_draw_bullets(bitmap, cliprect, FALSE);

	if (m_stars_enable)
		draw_stars(bitmap, cliprect);

	return 0;
}
