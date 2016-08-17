// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Food Fight hardware

****************************************************************************/

#include "emu.h"
#include "includes/foodf.h"
#include "video/resnet.h"


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(foodf_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (data & 0xff) | ((data >> 7) & 0x100);
	int color = (data >> 8) & 0x3f;
	SET_TILE_INFO_MEMBER(0, code, color, m_playfield_flip ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(foodf_state,foodf)
{
	static const int resistances[3] = { 1000, 470, 220 };

	/* adjust the playfield for the 8 pixel offset */
	m_playfield_tilemap->set_scrollx(0, -8);
	save_item(NAME(m_playfield_flip));

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3,  &resistances[0], m_rweights, 0, 0,
			3,  &resistances[0], m_gweights, 0, 0,
			2,  &resistances[1], m_bweights, 0, 0);
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

void foodf_state::foodf_set_flip(int flip)
{
	if (flip != m_playfield_flip)
	{
		m_playfield_flip = flip;
		m_playfield_tilemap->mark_all_dirty();
	}
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE16_MEMBER(foodf_state::foodf_paletteram_w)
{
	int newword, r, g, b, bit0, bit1, bit2;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	newword = m_generic_paletteram_16[offset];

	/* only the bottom 8 bits are used */
	/* red component */
	bit0 = (newword >> 0) & 0x01;
	bit1 = (newword >> 1) & 0x01;
	bit2 = (newword >> 2) & 0x01;
	r = combine_3_weights(m_rweights, bit0, bit1, bit2);

	/* green component */
	bit0 = (newword >> 3) & 0x01;
	bit1 = (newword >> 4) & 0x01;
	bit2 = (newword >> 5) & 0x01;
	g = combine_3_weights(m_gweights, bit0, bit1, bit2);

	/* blue component */
	bit0 = (newword >> 6) & 0x01;
	bit1 = (newword >> 7) & 0x01;
	b = combine_2_weights(m_bweights, bit0, bit1);

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 foodf_state::screen_update_foodf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	gfx_element *gfx = m_gfxdecode->gfx(1);
	bitmap_ind8 &priority_bitmap = screen.priority();
	UINT16 *spriteram16 = m_spriteram;

	/* first draw the playfield opaquely */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	/* then draw the non-transparent parts with a priority of 1 */
	priority_bitmap.fill(0);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	/* draw the motion objects front-to-back */
	for (offs = 0x80-2; offs >= 0x20; offs -= 2)
	{
		int data1 = spriteram16[offs];
		int data2 = spriteram16[offs+1];

		int pict = data1 & 0xff;
		int color = (data1 >> 8) & 0x1f;
		int xpos = (data2 >> 8) & 0xff;
		int ypos = (0xff - data2 - 16) & 0xff;
		int hflip = (data1 >> 15) & 1;
		int vflip = (data1 >> 14) & 1;
		int pri = (data1 >> 13) & 1;

		gfx->prio_transpen(bitmap,cliprect, pict, color, hflip, vflip,
				xpos, ypos, priority_bitmap, pri * 2, 0);

		/* draw again with wraparound (needed to get the end of level animation right) */
		gfx->prio_transpen(bitmap,cliprect, pict, color, hflip, vflip,
				xpos - 256, ypos, priority_bitmap, pri * 2, 0);
	}

	return 0;
}
