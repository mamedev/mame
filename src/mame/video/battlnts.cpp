// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/battlnts.h"

/***************************************************************************

  Callback for the K007342

***************************************************************************/

K007342_CALLBACK_MEMBER(battlnts_state::battlnts_tile_callback)
{
	*code |= ((*color & 0x0f) << 9) | ((*color & 0x40) << 2);
	*color = 0;
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

K007420_CALLBACK_MEMBER(battlnts_state::battlnts_sprite_callback)
{
	*code |= ((*color & 0xc0) << 2) | m_spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0;
}

WRITE8_MEMBER(battlnts_state::battlnts_spritebank_w)
{
	m_spritebank = 1024 * (data & 1);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

UINT32 battlnts_state::screen_update_battlnts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k007342->tilemap_update();

	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE ,0);
	m_k007420->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(1));
	m_k007342->tilemap_draw(screen, bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE ,0);
	return 0;
}
