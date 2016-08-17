// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "video/resnet.h"
#include "includes/mappy.h"


/***************************************************************************

  Convert the color PROMs.

  All games except Phozon have one 32x8 palette PROM and two 256x4 color
  lookup table PROMs (one for characters, one for sprites), except todruaga
  which has a larger 1024x4 PROM for sprites.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The way how the lookup tables are mapped to palette colors, and priority
  handling, are controlled by a PAL (SPV-5 in Super Pacman, MPI-4 in Mappy),
  so the two hardwares work differently.
  Super Pacman has a special "super priority" for sprite colors, allowing
  one pen to be over high priority tiles (used by Pac & Pal for ghost eyes),
  which isn't present in Mappy.

***************************************************************************/

PALETTE_INIT_MEMBER(mappy_state,superpac)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
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
	color_prom += 32;

	/* characters map to the upper 16 palette entries */
	for (i = 0; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, (ctabentry ^ 15) + 0x10);
	}

	/* sprites map to the lower 16 palette entries */
	for (i = 64*4; i < 128*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

PALETTE_INIT_MEMBER(mappy_state,mappy)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
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
	color_prom += 32;

	/* characters map to the upper 16 palette entries */
	for (i = 0*4; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry + 0x10);
	}

	/* sprites map to the lower 16 palette entries */
	for (i = 64*4; i < palette.entries(); i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}


/***************************************************************************

  In Phozon, the palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(mappy_state,phozon)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, &resistances[0], rweights, 0, 0,
			4, &resistances[0], gweights, 0, 0,
			4, &resistances[0], bweights, 0, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters map to the lower 16 palette entries */
	for (i = 0; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites map to the upper 16 palette entries */
	for (i = 64*4; i < 128*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry + 0x10);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(mappy_state::superpac_tilemap_scan)
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

/* tilemap is a composition of a 32x60 scrolling portion and two 2x28 fixed portions on the sides */
TILEMAP_MAPPER_MEMBER(mappy_state::mappy_tilemap_scan)
{
	int offs;

	col -= 2;
	if (col & 0x20)
	{
		/* in the following code, note the +2 followed by & 0x0f. This causes unintuitive
		   mapping from logical to hardware coordinates, which is true to the hardware.
		   Not doing it that way would cause missing tiles in motos and todruaga */
		if (row & 0x20)
			offs = 0x7ff;   // outside visible area
		else
			offs = ((row + 2) & 0x0f) + (row & 0x10) + ((col & 3) << 5) + 0x780;
	}
	else
		offs = col + (row << 5);

	return offs;
}

TILE_GET_INFO_MEMBER(mappy_state::superpac_get_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x400];

	tileinfo.category = (attr & 0x40) >> 6;
	tileinfo.group = attr & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index],
			attr & 0x3f,
			0);
}

TILE_GET_INFO_MEMBER(mappy_state::phozon_get_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x400];

	tileinfo.category = (attr & 0x40) >> 6;
	tileinfo.group = attr & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x3f,
			0);
}

TILE_GET_INFO_MEMBER(mappy_state::mappy_get_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x800];

	tileinfo.category = (attr & 0x40) >> 6;
	tileinfo.group = attr & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index],
			attr & 0x3f,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(mappy_state,superpac)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mappy_state::superpac_get_tile_info),this),tilemap_mapper_delegate(FUNC(mappy_state::superpac_tilemap_scan),this),8,8,36,28);
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 31);
}

VIDEO_START_MEMBER(mappy_state,phozon)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mappy_state::phozon_get_tile_info),this),tilemap_mapper_delegate(FUNC(mappy_state::superpac_tilemap_scan),this),8,8,36,28);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 15);
}

VIDEO_START_MEMBER(mappy_state,mappy)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mappy_state::mappy_get_tile_info),this),tilemap_mapper_delegate(FUNC(mappy_state::mappy_tilemap_scan),this),8,8,36,60);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 31);
	m_bg_tilemap->set_scroll_cols(36);

	save_item(NAME(m_scroll));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(mappy_state::superpac_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(mappy_state::mappy_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(mappy_state::superpac_flipscreen_w)
{
	flip_screen_set(data & 1);
}

READ8_MEMBER(mappy_state::superpac_flipscreen_r)
{
	flip_screen_set(1);
	return 0xff;
}

WRITE8_MEMBER(mappy_state::mappy_scroll_w)
{
	m_scroll = offset >> 3;
}



/***************************************************************************

  Display refresh

***************************************************************************/

void mappy_state::mappy_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base)
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int sprite = spriteram[offs];
			int color = spriteram[offs+1];
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 40;
			int sy = 256 - spriteram_2[offs] + 1;   // sprites are buffered and delayed by one scanline
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = (spriteram_3[offs] & 0x04) >> 2;
			int sizey = (spriteram_3[offs] & 0x08) >> 3;
			int x,y;

			sprite &= ~sizex;
			sprite &= ~(sizey << 1);

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;  // fix wraparound

			if (flip_screen())
			{
				flipx ^= 1;
				flipy ^= 1;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 15));
				}
			}
		}
	}
}


/*
sprite format:

spriteram
0   xxxxxxxx  tile number
1   --xxxxxx  color

spriteram_2
0   xxxxxxxx  Y position
1   xxxxxxxx  X position

spriteram_3
0   xx------  tile number LSB
0   --xx----  Y size (16, 8, 32, 4?)
0   ----xx--  X size (16, 8, 32, 4?)
0   ------x-  Y flip
0   -------x  X flip
1   ------x-  disable
1   -------x  X position MSB
*/

void mappy_state::phozon_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base)
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 size[4] = { 1, 0, 3, 0 };    /* 16, 8, 32 pixels; fourth combination unused? */
			static const UINT8 gfx_offs[4][4] =
			{
				{ 0, 1, 4, 5 },
				{ 2, 3, 6, 7 },
				{ 8, 9,12,13 },
				{10,11,14,15 }
			};
			int sprite = (spriteram[offs] << 2) | ((spriteram_3[offs] & 0xc0) >> 6);
			int color = spriteram[offs+1] & 0x3f;
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 69;
			int sy = 256 - spriteram_2[offs];
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = size[(spriteram_3[offs] & 0x0c) >> 2];
			int sizey = size[(spriteram_3[offs] & 0x30) >> 4];
			int x,y;

			sy -= 8 * sizey;
			sy = (sy & 0xff) - 32;  // fix wraparound

			if (flip_screen())
			{
				flipx ^= 1;
				flipy ^= 1;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 8*x,sy + 8*y,
						m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 31));
				}
			}
		}
	}
}


UINT32 mappy_state::screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 &sprite_bitmap = m_sprite_bitmap;
	int x,y;

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	sprite_bitmap.fill(15, cliprect);
	mappy_draw_sprites(sprite_bitmap,cliprect,m_spriteram);
	copybitmap_trans(bitmap,sprite_bitmap,0,0,0,0,cliprect,15);

	/* Redraw the high priority characters */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1,0);

	/* sprite color 0/1 still has priority over that (ghost eyes in Pac 'n Pal) */
	for (y = 0;y < sprite_bitmap.height();y++)
	{
		for (x = 0;x < sprite_bitmap.width();x++)
		{
			int spr_entry = sprite_bitmap.pix16(y, x);
			int spr_pen = m_palette->pen_indirect(spr_entry);
			if (spr_pen == 0 || spr_pen == 1)
				bitmap.pix16(y, x) = spr_entry;
		}
	}
	return 0;
}

UINT32 mappy_state::screen_update_phozon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* flip screen control is embedded in RAM */
	flip_screen_set(m_spriteram[0x1f7f-0x800] & 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	phozon_draw_sprites(bitmap,cliprect,m_spriteram);

	/* Redraw the high priority characters */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1,0);
	return 0;
}

UINT32 mappy_state::screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 2;offs < 34;offs++)
		m_bg_tilemap->set_scrolly(offs,m_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	mappy_draw_sprites(bitmap,cliprect,m_spriteram);

	/* Redraw the high priority characters */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1,0);
	return 0;
}
