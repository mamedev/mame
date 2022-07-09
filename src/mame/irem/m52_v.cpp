// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Irem M52 hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "m52.h"

#define BGHEIGHT 128


/*************************************
 *
 *  Palette configuration
 *
 *************************************/

void m52_state::init_palette()
{
	constexpr int resistances_3[3] = { 1000, 470, 220 };
	constexpr int resistances_2[2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;

	/* compute palette information for characters/backgrounds */
	scale = compute_resistor_weights(0, 255, -1.0,
			3, resistances_3, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			2, resistances_2, weights_b, 0, 0);

	/* character palette */
	const uint8_t *char_pal = memregion("tx_pal")->base();
	for (int i = 0; i < 512; i++)
	{
		uint8_t const promval = char_pal[i];
		int const r = combine_weights(weights_r, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 6), BIT(promval, 7));

		m_tx_palette->set_pen_color(i, rgb_t(r, g, b));
	}

	/* background palette */
	const uint8_t *back_pal = memregion("bg_pal")->base();
	for (int i = 0; i < 32; i++)
	{
		uint8_t promval = back_pal[i];
		int r = combine_weights(weights_r, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));
		int g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int b = combine_weights(weights_b, BIT(promval, 6), BIT(promval, 7));

		m_bg_palette->set_indirect_color(i, rgb_t(r, g, b));
	}

	/* background
	 the palette is a 32x8 PROM with many colors repeated. The address of
	 the colors to pick is as follows:
	 xbb00: mountains
	 0xxbb: hills
	 1xxbb: city

	 this seems hacky, surely all bytes in the PROM should be used, not just picking the ones that give the colours we want?

	 */
	m_bg_palette->set_pen_indirect(0 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(0 * 4 + 1, 4);
	m_bg_palette->set_pen_indirect(0 * 4 + 2, 8);
	m_bg_palette->set_pen_indirect(0 * 4 + 3, 12);
	m_bg_palette->set_pen_indirect(1 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(1 * 4 + 1, 1);
	m_bg_palette->set_pen_indirect(1 * 4 + 2, 2);
	m_bg_palette->set_pen_indirect(1 * 4 + 3, 3);
	m_bg_palette->set_pen_indirect(2 * 4 + 0, 0);
	m_bg_palette->set_pen_indirect(2 * 4 + 1, 16 + 1);
	m_bg_palette->set_pen_indirect(2 * 4 + 2, 16 + 2);
	m_bg_palette->set_pen_indirect(2 * 4 + 3, 16 + 3);

	init_sprite_palette(resistances_3, resistances_2, weights_r, weights_g, weights_b, scale);
}

template <size_t N, size_t O, size_t P>
void m52_state::init_sprite_palette(const int *resistances_3, const int *resistances_2, double (&weights_r)[N], double (&weights_g)[O], double (&weights_b)[P], double scale)
{
	const uint8_t *sprite_pal = memregion("spr_pal")->base();
	const uint8_t *sprite_table = memregion("spr_clut")->base();

	/* compute palette information for sprites */
	compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	/* sprite palette */
	for (int i = 0; i < 32; i++)
	{
		uint8_t const promval = sprite_pal[i];
		int const r = combine_weights(weights_r, BIT(promval, 6), BIT(promval, 7));
		int const g = combine_weights(weights_g, BIT(promval, 3), BIT(promval, 4), BIT(promval, 5));
		int const b = combine_weights(weights_b, BIT(promval, 0), BIT(promval, 1), BIT(promval, 2));

		m_sp_palette->set_indirect_color(i, rgb_t(r, g, b));
	}

	/* sprite lookup table */
	for (int i = 0; i < 256; i++)
	{
		uint8_t promval = sprite_table[i];
		m_sp_palette->set_pen_indirect(i, promval);
	}
}


/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m52_state::get_tile_info)
{
	uint8_t video = m_videoram[tile_index];
	uint8_t color = m_colorram[tile_index];

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

	tileinfo.set(0, code, color & 0x7f, flag);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void m52_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_tx_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m52_state::get_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_scrolldx(127, 127);
	m_tx_tilemap->set_scrolldy(16, 16);
	m_tx_tilemap->set_scroll_rows(4); /* only lines 192-256 scroll */

	init_palette();

	save_item(NAME(m_bg1xpos));
	save_item(NAME(m_bg1ypos));
	save_item(NAME(m_bg2xpos));
	save_item(NAME(m_bg2ypos));
	save_item(NAME(m_bgcontrol));

	m_spritelimit = 0x100-4;
	m_do_bg_fills = true;
}

void m52_alpha1v_state::video_start()
{
	m52_state::video_start();

	// is the limit really just higher anyway or is this a board mod?
	m_spritelimit = 0x200-4;
	m_do_bg_fills = false; // or you get solid green areas below the stars bg image.  does the doubled up tilemap ROM maybe mean double height instead?

	// the scrolling orange powerups 'orbs' are a single tile in the tilemap, your ship is huge, it is unclear where the hitboxes are meant to be
	// using the same value as mpatrol puts the collision at the very back of your ship
	// maybe the sprite positioning is incorrect instead or this is just how the game is designed
	//m_tx_tilemap->set_scrolldx(127+8, 127-8);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

void m52_state::m52_scroll_w(uint8_t data)
{
/*
    According to the schematics there is only one video register that holds the X scroll value
    with a NAND gate on the V64 and V128 lines to control when it's read, and when
    255 (via 8 pull up resistors) is used.

    So we set the first 3 quarters to 255 and the last to the scroll value
*/
	m_tx_tilemap->set_scrollx(0, 255);
	m_tx_tilemap->set_scrollx(1, 255);
	m_tx_tilemap->set_scrollx(2, 255);
	m_tx_tilemap->set_scrollx(3, -(data + 1));
}

void m52_alpha1v_state::m52_scroll_w(uint8_t data)
{
/*
   alpha1v must have some board mod to invert scroll register use, as it expects only the first block to remain static
   the scrolling powerups are part of the tx layer!

   TODO: check if this configuration works with Moon Patrol, maybe the schematics were read incorrectly?
*/
	m_tx_tilemap->set_scrollx(0,  255);
	m_tx_tilemap->set_scrollx(1, -(data + 1));
	m_tx_tilemap->set_scrollx(2, -(data + 1));
	m_tx_tilemap->set_scrollx(3, -(data + 1));
}


/*************************************
 *
 *  Video RAM write handlers
 *
 *************************************/

void m52_state::m52_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}


void m52_state::m52_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Custom protection
 *
 *************************************/

/* This looks like some kind of protection implemented by a custom chip on the
   scroll board. It mangles the value written to the port m52_bg1xpos_w, as
   follows: result = popcount(value & 0x7f) ^ (value >> 7) */
uint8_t m52_state::m52_protection_r()
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

void m52_state::m52_bg1ypos_w(uint8_t data)
{
	m_bg1ypos = data;
}

void m52_state::m52_bg1xpos_w(uint8_t data)
{
	m_bg1xpos = data;
}

void m52_state::m52_bg2xpos_w(uint8_t data)
{
	m_bg2xpos = data;
}

void m52_state::m52_bg2ypos_w(uint8_t data)
{
	m_bg2ypos = data;
}

void m52_state::m52_bgcontrol_w(uint8_t data)
{
	m_bgcontrol = data;
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

void m52_state::m52_flipscreen_w(uint8_t data)
{
	/* screen flip is handled both by software and hardware */
	flip_screen_set((data & 0x01) ^ (~ioport("DSW2")->read() & 0x01));

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}

void m52_alpha1v_state::alpha1v_flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x01);
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

void m52_state::draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect, int xpos, int ypos, int image)
{
	rectangle rect;
	const rectangle &visarea = m_screen->visible_area();
	const pen_t *paldata = m_bg_palette->pens();


	if (flip_screen())
	{
		xpos = 264 - xpos;
		ypos = 264 - ypos - BGHEIGHT;
	}

	xpos += 124;

	/* this may not be correct */
	ypos += 16;


	m_bg_gfxdecode->gfx(image)->transpen(bitmap,cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos,
		ypos, 0);


	m_bg_gfxdecode->gfx(image)->transpen(bitmap,cliprect,
		0, 0,
		flip_screen(),
		flip_screen(),
		xpos - 256,
		ypos, 0);

	// create a solid fill below the 64 pixel high bg images
	if (m_do_bg_fills)
	{
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

		bitmap.fill(paldata[m_bg_gfxdecode->gfx(image)->colorbase() + 3], rect);
	}
}



/*************************************
 *
 *  Sprites rendering
 *
 *************************************/

void m52_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int initoffs)
{
	int offs;

	/* draw the sprites */
	for (offs = initoffs; offs >= (initoffs & 0xc0); offs -= 4)
	{
		int sy = 257 - m_spriteram[offs];
		int color = m_spriteram[offs + 1] & 0x3f;
		int flipx = m_spriteram[offs + 1] & 0x40;
		int flipy = m_spriteram[offs + 1] & 0x80;
		int code = m_spriteram[offs + 2];
		int sx = m_spriteram[offs + 3];

		/* sprites from offsets $00-$7F are processed in the upper half of the frame */
		/* sprites from offsets $80-$FF are processed in the lower half of the frame */
		rectangle clip = cliprect;
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
			sx = 238 - sx;
			sy = 282 - sy;
		}

		sx += 129;

		/* in theory anyways; in practice, some of the molecule-looking guys get clipped */
#ifdef SPLIT_SPRITES
		sect_rect(&clip, cliprect);
#else
		clip = cliprect;
#endif

		m_sp_gfxdecode->gfx(0)->transmask(bitmap,clip,
			code, color, flipx, flipy, sx, sy,
			m_sp_palette->transpen_mask(*m_sp_gfxdecode->gfx(0), color,  0));
	}
}



/*************************************
 *
 *  Video render
 *
 *************************************/

uint32_t m52_state::screen_update_m52(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int offs;
	const pen_t *paldata = m_sp_palette->pens();

	bitmap.fill(paldata[0], cliprect);

	if (!(m_bgcontrol & 0x20))
	{
		if (!(m_bgcontrol & 0x10))
			draw_background(bitmap, cliprect, m_bg2xpos, m_bg2ypos, 0); /* distant mountains */

		// only one of these be drawn at once (they share the same scroll register) (alpha1v leaves everything enabled)
		if (!(m_bgcontrol & 0x02))
			draw_background(bitmap, cliprect, m_bg1xpos, m_bg1ypos, 1); /* hills */
		else if (!(m_bgcontrol & 0x04))
			draw_background(bitmap, cliprect, m_bg1xpos, m_bg1ypos, 2); /* cityscape */
	}

	m_tx_tilemap->set_flip(flip_screen() ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	for (offs = 0x3c; offs <= m_spritelimit; offs += 0x40)
		draw_sprites(bitmap, cliprect, offs);

	return 0;
}
