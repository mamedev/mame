// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "includes/ninjaw.h"

/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

void ninjaw_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs )
{
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, curx, cury;
	int code;

#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	for (offs = (m_spriteram.bytes() / 2) - 4; offs >= 0; offs -= 4)
	{
		data = m_spriteram[offs + 2];
		tilenum = data & 0x7fff;

		if (!tilenum)
			continue;

		data = m_spriteram[offs + 0];
		x = (data - 32) & 0x3ff;    /* aligns sprites on rock outcrops and sewer hole */

		data = m_spriteram[offs + 1];
		y = (data - 0) & 0x1ff;

		/*
		    The purpose of the bit at data&0x8 (below) is unknown, but it is set
		    on Darius explosions, some enemy missiles and at least 1 boss.
		    It is most likely another priority bit but as there are no obvious
		    visual problems it will need checked against the original pcb.

		    There is a report this bit is set when the player intersects
		    the tank sprite in Ninja Warriors however I was unable to repro
		    this or find any use of this bit in that game.

		    Bit&0x8000 is set on some sprites in later levels of Darius
		    but is again unknown, and there is no obvious visual problem.
		*/
		data = m_spriteram[offs + 3];
		flipx    = (data & 0x1);
		flipy    = (data & 0x2) >> 1;
		priority = (data & 0x4) >> 2; // 1 = low
		/* data&0x8 - unknown */
		if (priority != primask)
			continue;
		color    = (data & 0x7f00) >> 8;
		/* data&0x8000 - unknown */

#ifdef MAME_DEBUG
		if (data & 0x80f0)   unknown |= (data &0x80f0);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		curx = x;
		cury = y;
		code = tilenum;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code, color,
				flipx, flipy,
				curx, cury, 0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

UINT32 ninjaw_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, tc0100scn_device *tc0100scn)
{
	UINT8 layer[3], nodraw;

	tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn_1->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	/* chip 0 does tilemaps on the left, chip 1 center, chip 2 the right */
	// draw bottom layer
	nodraw  = tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);    /* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap.fill(m_palette->black_pen(), cliprect);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(bitmap, cliprect, 1, xoffs, 8); // draw sprites with priority 1 which are under the mid layer

	// draw middle layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 0);

	draw_sprites(bitmap,cliprect,0,xoffs,8); // draw sprites with priority 0 which are over the mid layer

	// draw top(text) layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 0);
	return 0;
}

UINT32 ninjaw_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 0, m_tc0100scn_1); }
UINT32 ninjaw_state::screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 1, m_tc0100scn_2); }
UINT32 ninjaw_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8 * 2, m_tc0100scn_3); }
