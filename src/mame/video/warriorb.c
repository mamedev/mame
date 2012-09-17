#include "emu.h"
#include "video/taitoic.h"
#include "includes/warriorb.h"

/**********************************************************/

void warriorb_state::video_start()
{

	/* Ensure palette from correct TC0110PCR used for each screen */
	tc0100scn_set_colbanks(m_tc0100scn_1, 0x0, 0x100, 0x0);
}


/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs )
{
	warriorb_state *state = machine.driver_data<warriorb_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, data, data2, tilenum, color, flipx, flipy;
	int x, y, priority, pri_mask;

#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	/* pdrawgfx() needs us to draw sprites front to back */
	for (offs = 0; offs < state->m_spriteram.bytes() / 2; offs += 4)
	{
		data = spriteram[offs + 1];
		tilenum = data & 0x7fff;

		data = spriteram[offs + 0];
		y = (-(data & 0x1ff) - 24) & 0x1ff;	/* (inverted y adjusted for vis area) */
		flipy = (data & 0x200) >> 9;

		data2 = spriteram[offs + 2];
		/* 8,4 also seen in msbyte */
		priority = (data2 & 0x0100) >> 8; // 1 = low

		if(priority)
			pri_mask = 0xfffe;
		else
			pri_mask = 0;

		color = (data2 & 0x7f);

		data = spriteram[offs + 3];
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

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				 tilenum,
				 color,
				 flipx,flipy,
				 x,y,
				 machine.priority_bitmap,pri_mask,0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

static UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, device_t *tc0100scn)
{
	UINT8 layer[3], nodraw;

	tc0100scn_tilemap_update(tc0100scn);

	layer[0] = tc0100scn_bottomlayer(tc0100scn);
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	/* Clear priority bitmap */
	screen.machine().priority_bitmap.fill(0, cliprect);

	/* chip 0 does tilemaps on the left, chip 1 does the ones on the right */
	// draw bottom layer
	nodraw  = tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);	/* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap.fill(get_black_pen(screen.machine()), cliprect);

	// draw middle layer
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[1], 0, 1);

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen.machine(), bitmap, cliprect, xoffs, 8); // draw sprites

	// draw top(text) layer
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[2], 0, 0);
	return 0;
}

UINT32 warriorb_state::screen_update_warriorb_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 40 * 8 * 0, screen.machine().driver_data<warriorb_state>()->m_tc0100scn_1); }
UINT32 warriorb_state::screen_update_warriorb_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 40 * 8 * 1, screen.machine().driver_data<warriorb_state>()->m_tc0100scn_2); }
