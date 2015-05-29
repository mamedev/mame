// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Seibu Stinger/Wiz hardware

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/wiz.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(wiz_state, wiz)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[4] = { 1000, 470, 220, 100 };
	double rweights[4], gweights[4], bweights[4];

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	/* initialize the palette with these colors */
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
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

		m_palette->set_pen_color(i, rgb_t(r, g, b));
	}
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(wiz_state::wiz_palette_bank_w)
{
	m_palbank[offset] = data & 1;
}

WRITE8_MEMBER(wiz_state::wiz_char_bank_w)
{
	m_charbank[offset] = data & 1;
}

WRITE8_MEMBER(wiz_state::wiz_sprite_bank_w)
{
	m_sprite_bank = data & 1;
}

WRITE8_MEMBER(wiz_state::wiz_bgcolor_w)
{
	m_bgcolor = data;
}

WRITE8_MEMBER(wiz_state::wiz_flipx_w)
{
	m_flipx = data & 1;
}

WRITE8_MEMBER(wiz_state::wiz_flipy_w)
{
	m_flipy = data & 1;
}



/***************************************************************************

  Screen Update

***************************************************************************/

void wiz_state::draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int charbank, int colortype)
{
	UINT8 *vram = layer ? m_videoram2 : m_videoram;
	UINT8 *aram = layer ? m_attrram2 : m_attrram;
	UINT8 *cram = layer ? m_colorram2 : m_colorram;
	gfx_element *gfx = m_gfxdecode->gfx(charbank);
	int palbank = m_palbank[1] << 4 | m_palbank[0] << 3;

	/* draw the tiles. They are characters, but draw them as sprites. */
	for (int offs = 0x400-1; offs >= 0; offs--)
	{
		int code = vram[offs];
		int sx = offs & 0x1f;
		int sy = offs >> 5;
		int color = aram[sx << 1 | 1] & 7;

		// wiz/kungfut hw allows more color variety on screen
		if (colortype)
			color = layer ? (cram[offs] & 7) : ((color & 4) | (code & 3));

		int scroll = (8*sy + 256 - aram[sx << 1]) & 0xff;
		if (m_flipy)
			scroll = (248 - scroll) & 0xff;
		if (m_flipx)
			sx = 31 - sx;

		gfx->transpen(bitmap,cliprect,
			code,
			palbank | color,
			m_flipx,m_flipy,
			8*sx,scroll,0);
	}
}


void wiz_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int set, int charbank)
{
	UINT8 *sram = set ? m_spriteram2 : m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(charbank);
	int palbank = m_palbank[1] << 4 | m_palbank[0] << 3;

	for (int offs = 0x20-4; offs >= 0; offs -= 4)
	{
		int code = sram[offs + 1];
		int sx = sram[offs + 3];
		int sy = sram[offs];
		int color = sram[offs + 2] & 7; // high bits unused

		if (!sx || !sy) continue;

		// like on galaxian hw, the first three sprites match against y-1 (not on m_spriteram2)
		if (set == 0 && offs <= 8)
			sy += (m_flipy) ? 1 : -1;

		if ( m_flipx) sx = 240 - sx;
		if (!m_flipy) sy = 240 - sy;

		gfx->transpen(bitmap,cliprect,
			code,
			palbank | color,
			m_flipx,m_flipy,
			sx,sy,0);
	}
}


/**************************************************************************/

UINT32 wiz_state::screen_update_kungfut(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);
	draw_tiles(bitmap, cliprect, 0, 2 + m_charbank[0], 1);
	draw_tiles(bitmap, cliprect, 1, m_charbank[1], 1);
	draw_sprites(bitmap, cliprect, 1, 4);
	draw_sprites(bitmap, cliprect, 0, 5);
	return 0;
}

UINT32 wiz_state::screen_update_wiz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);
	draw_tiles(bitmap, cliprect, 0, 2 + ((m_charbank[0] << 1) | m_charbank[1]), 1);
	draw_tiles(bitmap, cliprect, 1, m_charbank[1], 1);

	const rectangle spritevisiblearea(2*8, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-1, 2*8, 30*8-1);
	const rectangle &visible_area = m_flipx ? spritevisibleareaflipx : spritevisiblearea;

	draw_sprites(bitmap, visible_area, 1, 6);
	draw_sprites(bitmap, visible_area, 0, 7 + m_sprite_bank);
	return 0;
}


UINT32 wiz_state::screen_update_stinger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);
	draw_tiles(bitmap, cliprect, 0, 2 + m_charbank[0], 0);
	draw_tiles(bitmap, cliprect, 1, m_charbank[1], 0);
	draw_sprites(bitmap, cliprect, 1, 4);
	draw_sprites(bitmap, cliprect, 0, 5);
	return 0;
}
