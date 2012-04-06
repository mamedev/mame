/***************************************************************************
  Functions to emulate video hardware on these Taito games:

  - rainbow islands
  - jumping (bootleg)

***************************************************************************/

#include "emu.h"
#include "video/taitoic.h"
#include "includes/rainbow.h"

/***************************************************************************/

WRITE16_MEMBER(rbisland_state::rbisland_spritectrl_w)
{

	if (offset == 0)
	{
		/* bits 0 and 1 always set */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		pc090oj_set_sprite_ctrl(m_pc090oj, (data & 0xe0) >> 5);
	}
}

WRITE16_MEMBER(rbisland_state::jumping_spritectrl_w)
{

	if (offset == 0)
	{
		/* bits 0 and 1 are set after 15 seconds */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		m_sprite_ctrl = data;
	}
}

/***************************************************************************/

SCREEN_UPDATE_IND16( rainbow )
{
	rbisland_state *state = screen.machine().driver_data<rbisland_state>();
	int layer[2];

	pc080sn_tilemap_update(state->m_pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	screen.machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(state->m_pc090oj, bitmap, cliprect, 1);
	return 0;
}


/***************************************************************************

Jumping uses different sprite controller
than Rainbow Island. - values are remapped
at address 0x2EA in the code. Apart from
physical layout, the main change is that
the Y settings are active low.

*/

VIDEO_START( jumping )
{
	rbisland_state *state = machine.driver_data<rbisland_state>();

	pc080sn_set_trans_pen(state->m_pc080sn, 1, 15);

	state->m_sprite_ctrl = 0;
	state->m_sprites_flipscreen = 0;

	/* not 100% sure Jumping needs to save both... */
	state->save_item(NAME(state->m_sprite_ctrl));
	state->save_item(NAME(state->m_sprites_flipscreen));
}


SCREEN_UPDATE_IND16( jumping )
{
	rbisland_state *state = screen.machine().driver_data<rbisland_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, layer[2];
	int sprite_colbank = (state->m_sprite_ctrl & 0xe0) >> 1;

	pc080sn_tilemap_update(state->m_pc080sn);

	/* Override values, or foreground layer is in wrong position */
	pc080sn_set_scroll(state->m_pc080sn, 1, 16, 0);

	layer[0] = 0;
	layer[1] = 1;

	screen.machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);

	/* Draw the sprites. 128 sprites in total */
	for (offs = state->m_spriteram_size / 2 - 8; offs >= 0; offs -= 8)
	{
		int tile = spriteram[offs];
		if (tile < screen.machine().gfx[1]->total_elements)
		{
			int sx,sy,color,data1;

			sy = ((spriteram[offs + 1] - 0xfff1) ^ 0xffff) & 0x1ff;
			if (sy > 400) sy = sy - 512;
			sx = (spriteram[offs + 2] - 0x38) & 0x1ff;
			if (sx > 400) sx = sx - 512;

			data1 = spriteram[offs + 3];
			color = (spriteram[offs + 4] & 0x0f) | sprite_colbank;

			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0],
					tile,
					color,
					data1 & 0x40, data1 & 0x80,
					sx,sy+1,15);
		}
	}

	pc080sn_tilemap_draw(state->m_pc080sn, bitmap, cliprect, layer[1], 0, 0);

#if 0
	{
		char buf[80];
		sprintf(buf,"sprite_ctrl: %04x", state->m_sprite_ctrl);
		popmessage(buf);
	}
#endif
	return 0;
}
