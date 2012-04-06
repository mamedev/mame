/***************************************************************************

  Video Hardware for Double Dragon 3

***************************************************************************/

#include "emu.h"
#include "includes/ddragon3.h"

WRITE16_MEMBER(ddragon3_state::ddragon3_scroll_w)
{

	switch (offset)
	{
		case 0: COMBINE_DATA(&m_fg_scrollx);	break;	// Scroll X, BG1
		case 1: COMBINE_DATA(&m_fg_scrolly);	break;	// Scroll Y, BG1
		case 2: COMBINE_DATA(&m_bg_scrollx);	break;	// Scroll X, BG0
		case 3: COMBINE_DATA(&m_bg_scrolly);	break;	// Scroll Y, BG0
		case 4:										break;	// Unknown write
		case 5: flip_screen_set(machine(), data & 0x01);		break;	// Flip Screen
		case 6:
			COMBINE_DATA(&m_bg_tilebase);			// BG Tile Base
			m_bg_tilebase &= 0x1ff;
			m_bg_tilemap->mark_all_dirty();
			break;
	}
}

READ16_MEMBER(ddragon3_state::ddragon3_scroll_r)
{

	switch (offset)
	{
		case 0: return m_fg_scrollx;
		case 1: return m_fg_scrolly;
		case 2: return m_bg_scrollx;
		case 3: return m_bg_scrolly;
		case 5: return flip_screen_get(machine());
		case 6: return m_bg_tilebase;
	}

	return 0;
}

WRITE16_MEMBER(ddragon3_state::ddragon3_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(ddragon3_state::ddragon3_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	ddragon3_state *state = machine.driver_data<ddragon3_state>();
	UINT16 attr = state->m_bg_videoram[tile_index];
	int code = (attr & 0x0fff) | ((state->m_bg_tilebase & 0x01) << 12);
	int color = ((attr & 0xf000) >> 12) + 16;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	ddragon3_state *state = machine.driver_data<ddragon3_state>();
	int offs = tile_index * 2;
	UINT16 attr = state->m_fg_videoram[offs];
	int code = state->m_fg_videoram[offs + 1] & 0x1fff;
	int color = attr & 0xf;
	int flags = (attr & 0x40) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( ddragon3 )
{
	ddragon3_state *state = machine.driver_data<ddragon3_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);
}

/*
 * Sprite Format
 * ----------------------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | --------xxxxxxxx | ypos (signed)
 * -----+------------------+
 *   1  | --------xxx----- | height
 *   1  | -----------xx--- | yflip, xflip
 *   1  | -------------x-- | msb x
 *   1  | --------------x- | msb y?
 *   1  | ---------------x | enable
 * -----+------------------+
 *   2  | --------xxxxxxxx | tile number
 * -----+------------------+
 *   3  | --------xxxxxxxx | bank
 * -----+------------------+
 *   4  | ------------xxxx |color
 * -----+------------------+
 *   5  | --------xxxxxxxx | xpos
 * -----+------------------+
 *   6,7| unused
 */

static void draw_sprites( running_machine& machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ddragon3_state *state = machine.driver_data<ddragon3_state>();
	UINT16 *source = state->m_spriteram;
	UINT16 *finish = source + 0x800;

	while (source < finish)
	{
		UINT16 attr = source[1];

		if (attr & 0x01)	/* enable */
		{
			int i;
			int bank = source[3] & 0xff;
			int code = (source[2] & 0xff) + (bank * 256);
			int color = source[4] & 0xf;
			int flipx = attr & 0x10;
			int flipy = attr & 0x08;
			int sx = source[5] & 0xff;
			int sy = source[0] & 0xff;
			int height = (attr >> 5) & 0x07;

			if (attr & 0x04) sx |= 0x100;
			if (attr & 0x02) sy = 239 + (0x100 - sy); else sy = 240 - sy;
			if (sx > 0x17f) sx = 0 - (0x200 - sx);

			if (flip_screen_get(machine))
			{
				sx = 304 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			for (i = 0; i <= height; i++)
			{
				drawgfx_transpen(bitmap, cliprect,
					machine.gfx[1], code + i, color, flipx, flipy,
					sx, sy + (flip_screen_get(machine) ? (i * 16) : (-i * 16)), 0);
			}
		}

		source += 8;
	}
}

SCREEN_UPDATE_IND16( ddragon3 )
{
	ddragon3_state *state = screen.machine().driver_data<ddragon3_state>();

	state->m_bg_tilemap->set_scrollx(0, state->m_bg_scrollx);
	state->m_bg_tilemap->set_scrolly(0, state->m_bg_scrolly);
	state->m_fg_tilemap->set_scrollx(0, state->m_fg_scrollx);
	state->m_fg_tilemap->set_scrolly(0, state->m_fg_scrolly);

	if ((state->m_vreg & 0x60) == 0x40)
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
	}
	else if ((state->m_vreg & 0x60) == 0x60)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
	}
	else
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	return 0;
}

SCREEN_UPDATE_IND16( ctribe )
{
	ddragon3_state *state = screen.machine().driver_data<ddragon3_state>();

	state->m_bg_tilemap->set_scrollx(0, state->m_bg_scrollx);
	state->m_bg_tilemap->set_scrolly(0, state->m_bg_scrolly);
	state->m_fg_tilemap->set_scrollx(0, state->m_fg_scrollx);
	state->m_fg_tilemap->set_scrolly(0, state->m_fg_scrolly);

	if(state->m_vreg & 8)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
	{
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
		draw_sprites(screen.machine(), bitmap, cliprect);
	}
	return 0;
}
