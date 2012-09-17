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

UINT32 rbisland_state::screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[2];

	pc080sn_tilemap_update(m_pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	screen.machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(m_pc090oj, bitmap, cliprect, 1);
	return 0;
}


/***************************************************************************

Jumping uses different sprite controller
than Rainbow Island. - values are remapped
at address 0x2EA in the code. Apart from
physical layout, the main change is that
the Y settings are active low.

*/

VIDEO_START_MEMBER(rbisland_state,jumping)
{

	pc080sn_set_trans_pen(m_pc080sn, 1, 15);

	m_sprite_ctrl = 0;
	m_sprites_flipscreen = 0;

	/* not 100% sure Jumping needs to save both... */
	save_item(NAME(m_sprite_ctrl));
	save_item(NAME(m_sprites_flipscreen));
}


UINT32 rbisland_state::screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *spriteram = m_spriteram;
	int offs, layer[2];
	int sprite_colbank = (m_sprite_ctrl & 0xe0) >> 1;

	pc080sn_tilemap_update(m_pc080sn);

	/* Override values, or foreground layer is in wrong position */
	pc080sn_set_scroll(m_pc080sn, 1, 16, 0);

	layer[0] = 0;
	layer[1] = 1;

	screen.machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);

	/* Draw the sprites. 128 sprites in total */
	for (offs = m_spriteram.bytes() / 2 - 8; offs >= 0; offs -= 8)
	{
		int tile = spriteram[offs];
		if (tile < screen.machine().gfx[1]->elements())
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

	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[1], 0, 0);

#if 0
	{
		char buf[80];
		sprintf(buf,"sprite_ctrl: %04x", m_sprite_ctrl);
		popmessage(buf);
	}
#endif
	return 0;
}
