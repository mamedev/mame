// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Irem M52 hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/m52.h"

#define BGHEIGHT 64


/*************************************
 *
 *  Palette configuration
 *
 *************************************/

PALETTE_INIT_MEMBER(m52_state, m52)
{
	const UINT8 *color_prom = memregion("proms")->base();
	const UINT8 *char_pal = color_prom + 0x000;
	const UINT8 *back_pal = color_prom + 0x200;
	const UINT8 *sprite_pal = color_prom + 0x220;
	const UINT8 *sprite_table = color_prom + 0x240;
	static const int resistances_3[3] = { 1000, 470, 220 };
	static const int resistances_2[2]  = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;
	int i;

	/* compute palette information for characters/backgrounds */
	scale = compute_resistor_weights(0, 255, -1.0,
			3, resistances_3, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			2, resistances_2, weights_b, 0, 0);

	/* character palette */
	for (i = 0; i < 512; i++)
	{
		UINT8 promval = char_pal[i];
		int r = combine_3_weights(weights_r, BIT(promval,0), BIT(promval,1), BIT(promval,2));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_2_weights(weights_b, BIT(promval,6), BIT(promval,7));

		palette.set_indirect_color(i, rgb_t(r,g,b));
	}

	/* background palette */
	for (i = 0; i < 32; i++)
	{
		UINT8 promval = back_pal[i];
		int r = combine_3_weights(weights_r, BIT(promval,0), BIT(promval,1), BIT(promval,2));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_2_weights(weights_b, BIT(promval,6), BIT(promval,7));

		palette.set_indirect_color(512+i, rgb_t(r,g,b));
	}

	/* compute palette information for sprites */
	compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	/* sprite palette */
	for (i = 0; i < 32; i++)
	{
		UINT8 promval = sprite_pal[i];
		int r = combine_2_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_3_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(512 + 32 + i, rgb_t(r,g,b));
	}

	/* character lookup table */
	for (i = 0; i < 512; i++)
		palette.set_pen_indirect(i, i);

	/* sprite lookup table */
	for (i = 0; i < 16 * 4; i++)
	{
		UINT8 promval = sprite_table[(i & 3) | ((i & ~3) << 1)];
		palette.set_pen_indirect(512 + i, 512 + 32 + promval);
	}

	/* background */
	/* the palette is a 32x8 PROM with many colors repeated. The address of */
	/* the colors to pick is as follows: */
	/* xbb00: mountains */
	/* 0xxbb: hills */
	/* 1xxbb: city */
	palette.set_pen_indirect(512+16*4+0*4+0, 512);
	palette.set_pen_indirect(512+16*4+0*4+1, 512+4);
	palette.set_pen_indirect(512+16*4+0*4+2, 512+8);
	palette.set_pen_indirect(512+16*4+0*4+3, 512+12);
	palette.set_pen_indirect(512+16*4+1*4+0, 512);
	palette.set_pen_indirect(512+16*4+1*4+1, 512+1);
	palette.set_pen_indirect(512+16*4+1*4+2, 512+2);
	palette.set_pen_indirect(512+16*4+1*4+3, 512+3);
	palette.set_pen_indirect(512+16*4+2*4+0, 512);
	palette.set_pen_indirect(512+16*4+2*4+1, 512+16+1);
	palette.set_pen_indirect(512+16*4+2*4+2, 512+16+2);
	palette.set_pen_indirect(512+16*4+2*4+3, 512+16+3);
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m52_state::get_tile_info)
{
	UINT8 video = m_videoram[tile_index];
	UINT8 color = m_colorram[tile_index];

	int flag = 0;
	int code = 0;

	code = video;

	if (color & 0x80)
	{
		code |= 0x100;
	}

	if (tile_index / 32 <= 6)
	{
		flag |= TILE_FORCE_LAYER0; /* lines 0 to 6 are opaqe? */
	}

	SET_TILE_INFO_MEMBER(0, code, color & 0x3f, flag);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void m52_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m52_state::get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(127, 127);
	m_bg_tilemap->set_scrolldy(16, 16);
	m_bg_tilemap->set_scroll_rows(4); /* only lines 192-256 scroll */

	save_item(NAME(m_bg1xpos));
	save_item(NAME(m_bg1ypos));
	save_item(NAME(m_bg2xpos));
	save_item(NAME(m_bg2ypos));
	save_item(NAME(m_bgcontrol));
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_MEMBER(m52_state::m52_scroll_w)
{
/*
    According to the schematics there is only one video register that holds the X scroll value
    with a NAND gate on the V64 and V128 lines to control when it's read, and when
    255 (via 8 pull up resistors) is used.

    So we set the first 3 quarters to 255 and the last to the scroll value
*/
	m_bg_tilemap->set_scrollx(0, 255);
	m_bg_tilemap->set_scrollx(1, 255);
	m_bg_tilemap->set_scrollx(2, 255);
	m_bg_tilemap->set_scrollx(3, -data);
}



/*************************************
 *
 *  Video RAM write handlers
 *
 *************************************/

WRITE8_MEMBER(m52_state::m52_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(m52_state::m52_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Custom protection
 *
 *************************************/

/* This looks like some kind of protection implemented by a custom chip on the
   scroll board. It mangles the value written to the port m52_bg1xpos_w, as
   follows: result = popcount(value & 0x7f) ^ (value >> 7) */
READ8_MEMBER(m52_state::m52_protection_r)
{
	int popcount = 0;
	int temp;

	for (temp = m_bg1xpos & 0x7f; temp != 0; temp >>= 1)
		popcount += temp & 1;
	return popcount ^ (m_bg1xpos >> 7);
}



/*************************************
 *
 *  Background control write handlers
 *
 *************************************/

WRITE8_MEMBER(m52_state::m52_bg1ypos_w)
{
	m_bg1ypos = data;
}

WRITE8_MEMBER(m52_state::m52_bg1xpos_w)
{
	m_bg1xpos = data;
}

WRITE8_MEMBER(m52_state::m52_bg2xpos_w)
{
	m_bg2xpos = data;
}

WRITE8_MEMBER(m52_state::m52_bg2ypos_w)
{
	m_bg2ypos = data;
}

WRITE8_MEMBER(m52_state::m52_bgcontrol_w)
{
	m_bgcontrol = data;
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_MEMBER(m52_state::m52_flipscreen_w)
{
	/* screen flip is handled both by software and hardware */
	flip_screen_set((data & 0x01) ^ (~ioport("DSW2")->read() & 0x01));

	coin_counter_w(machine(), 0, data & 0x02);
	coin_counter_w(machine(), 1, data & 0x20);
}

WRITE8_MEMBER(m52_state::alpha1v_flipscreen_w)
{
	flip_screen_set(data & 0x01);
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

void m52_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image)
{
	rectangle rect;
	const rectangle &visarea = m_screen->visible_area();


	if (flip_screen())
	{
		xpos = 127 - xpos;
		ypos = 255 - ypos - BGHEIGHT;
	}

	xpos += 128;

	/* this may not be correct */
	ypos = ypos + (22 - 8);


		m_gfxdecode->gfx(image)->transpen(bitmap,cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos,
		ypos, 0);


		m_gfxdecode->gfx(image)->transpen(bitmap,cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos - 256,
		ypos, 0);

	rect.min_x = visarea.min_x;
	rect.max_x = visarea.max_x;

	if (flip_screen())
	{
		rect.min_y = ypos - BGHEIGHT;
		rect.max_y = ypos - 1;
	}
	else
	{
		rect.min_y = ypos + BGHEIGHT;
		rect.max_y = ypos + 2 * BGHEIGHT - 1;
	}

	bitmap.fill(m_gfxdecode->gfx(image)->colorbase() + 3, rect);
}



/*************************************
 *
 *  Video render
 *
 *************************************/

UINT32 m52_state::screen_update_m52(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	bitmap.fill(0, cliprect);

	if (!(m_bgcontrol & 0x20))
	{
		if (!(m_bgcontrol & 0x10))
			draw_background(bitmap, cliprect, m_bg2xpos, m_bg2ypos, 2); /* distant mountains */

		if (!(m_bgcontrol & 0x02))
			draw_background(bitmap, cliprect, m_bg1xpos, m_bg1ypos, 3); /* hills */

		if (!(m_bgcontrol & 0x04))
			draw_background(bitmap, cliprect, m_bg1xpos, m_bg1ypos, 4); /* cityscape */
	}

	m_bg_tilemap->set_flip(flip_screen() ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0xfc; offs >= 0; offs -= 4)
	{
		int sy = 257 - m_spriteram[offs];
		int color = m_spriteram[offs + 1] & 0x3f;
		int flipx = m_spriteram[offs + 1] & 0x40;
		int flipy = m_spriteram[offs + 1] & 0x80;
		int code = m_spriteram[offs + 2];
		int sx = m_spriteram[offs + 3];
		rectangle clip;

		/* sprites from offsets $00-$7F are processed in the upper half of the frame */
		/* sprites from offsets $80-$FF are processed in the lower half of the frame */
		clip = cliprect;
		if (!(offs & 0x80))
			clip.min_y = 0, clip.max_y = 127;
		else
			clip.min_y = 128, clip.max_y = 255;

		/* adjust for flipping */
		if (flip_screen())
		{
			int temp = clip.min_y;
			clip.min_y = 255 - clip.max_y;
			clip.max_y = 255 - temp;
			flipx = !flipx;
			flipy = !flipy;
			sx = 112 - sx;
			sy = 257 + 11 - sy;
		}

		sx += 128;

		/* in theory anyways; in practice, some of the molecule-looking guys get clipped */
#ifdef SPLIT_SPRITES
		sect_rect(&clip, cliprect);
#else
		clip = cliprect;
#endif

		m_gfxdecode->gfx(1)->transmask(bitmap,clip,
			code, color, flipx, flipy, sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 512 + 32));
	}
	return 0;
}
