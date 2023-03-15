// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "ninjaw.h"

/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

void ninjaw_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs, int chip)
{
#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	static const u32 primask[2] =
	{
		GFX_PMASK_4, // draw sprites with priority 0 which are over the mid layer
		(GFX_PMASK_4 | GFX_PMASK_2) // draw sprites with priority 1 which are under the mid layer
	};

	for (int offs = 0; offs < (m_spriteram.bytes() / 2); offs += 4)
	{
		int data = m_spriteram[offs + 2];
		const u32 tilenum = data & 0x7fff;

		if (!tilenum)
			continue;

		data = m_spriteram[offs + 0];
		int x = (data - 32) & 0x3ff;    /* aligns sprites on rock outcrops and sewer hole */

		data = m_spriteram[offs + 1];
		int y = (data - 0) & 0x1ff;

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
		const bool flipx = (data & 0x1);
		const bool flipy = (data & 0x2) >> 1;
		const int priority = (data & 0x4) >> 2; // 1 = low
		/* data&0x8 - unknown */
		const u32 color = (data & 0x7f00) >> 8;
		/* data&0x8000 - unknown */

#ifdef MAME_DEBUG
		if (data & 0x80f0)   unknown |= (data &0x80f0);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		const int curx = x;
		const int cury = y;
		const u32 code = tilenum;

		m_gfxdecode[chip]->gfx(0)->prio_transpen(bitmap,cliprect,
				code, color,
				flipx, flipy,
				curx, cury,
				screen.priority(), primask[priority],
				0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

u32 ninjaw_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip)
{
	tc0100scn_device *tc0100scn = m_tc0100scn[chip];
	xoffs *= chip;
	u8 layer[3];

	tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn[0]->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);
	/* chip 0 does tilemaps on the left, chip 1 center, chip 2 the right */
	// draw bottom layer
	u8 nodraw = tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);    /* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap.fill(m_tc0110pcr[chip]->black_pen(), cliprect);

	// draw middle layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

	// draw top(text) layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen, bitmap, cliprect, xoffs, 8, chip);

	return 0;
}

u32 ninjaw_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8, 0); }
u32 ninjaw_state::screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8, 1); }
u32 ninjaw_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 36 * 8, 2); }
