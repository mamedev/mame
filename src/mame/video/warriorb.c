// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "includes/warriorb.h"

/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

void warriorb_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs )
{
	int offs, data, data2, tilenum, color, flipx, flipy;
	int x, y, priority, pri_mask;

#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	/* pdrawgfx() needs us to draw sprites front to back */
	for (offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		data = m_spriteram[offs + 1];
		tilenum = data & 0x7fff;

		data = m_spriteram[offs + 0];
		y = (-(data & 0x1ff) - 24) & 0x1ff; /* (inverted y adjusted for vis area) */
		flipy = (data & 0x200) >> 9;

		data2 = m_spriteram[offs + 2];
		/* 8,4 also seen in msbyte */
		priority = (data2 & 0x0100) >> 8; // 1 = low

		if(priority)
			pri_mask = 0xfffe;
		else
			pri_mask = 0;

		color = (data2 & 0x7f);

		data = m_spriteram[offs + 3];
		x = (data & 0x3ff);
		flipx = (data & 0x400) >> 10;


#ifdef MAME_DEBUG
		if (data2 & 0xf280)   unknown |= (data2 &0xf280);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
					tilenum,
					color,
					flipx,flipy,
					x,y,
					screen.priority(),pri_mask,0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

UINT32 warriorb_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, tc0100scn_device *tc0100scn)
{
	UINT8 layer[3], nodraw;

	tc0100scn->tilemap_update();

	layer[0] = tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	/* Clear priority bitmap */
	screen.priority().fill(0, cliprect);

	/* chip 0 does tilemaps on the left, chip 1 does the ones on the right */
	// draw bottom layer
	nodraw  = tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);    /* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap.fill(m_palette->black_pen(), cliprect);

	// draw middle layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen, bitmap, cliprect, xoffs, 8); // draw sprites

	// draw top(text) layer
	tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 0);
	return 0;
}

UINT32 warriorb_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 40 * 8 * 0, m_tc0100scn_1); }
UINT32 warriorb_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 40 * 8 * 1, m_tc0100scn_2); }
