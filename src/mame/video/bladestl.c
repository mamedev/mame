// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/bladestl.h"


PALETTE_INIT_MEMBER(bladestl_state, bladestl)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* characters use pens 0x00-0x1f, no look-up table */
	for (i = 0; i < 0x20; i++)
		palette.set_pen_indirect(i, i);

	/* sprites use pens 0x20-0x2f */
	for (i = 0x20; i < 0x120; i++)
	{
		UINT8 ctabentry = (color_prom[i - 0x20] & 0x0f) | 0x20;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callback for the K007342

***************************************************************************/

K007342_CALLBACK_MEMBER(bladestl_state::bladestl_tile_callback)
{
	*code |= ((*color & 0x0f) << 8) | ((*color & 0x40) << 6);
	*color = layer;
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

K007420_CALLBACK_MEMBER(bladestl_state::bladestl_sprite_callback)
{
	*code |= ((*color & 0xc0) << 2) + m_spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0 + (*color & 0x0f);
}


/***************************************************************************

  Screen Refresh

***************************************************************************/

UINT32 bladestl_state::screen_update_bladestl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k007342->tilemap_update();

	m_k007342->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE ,0);
	m_k007420->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(1));
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 1, 1 | TILEMAP_DRAW_OPAQUE ,0);
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, 0 ,0);
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, 1 ,0);
	return 0;
}
