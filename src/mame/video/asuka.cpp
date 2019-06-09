// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
#include "emu.h"
#include "includes/asuka.h"
#include "screen.h"

/**************************************************************
                 SPRITE READ AND WRITE HANDLERS
**************************************************************/

void asuka_state::asuka_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	sprite_colbank = (sprite_ctrl & 0x3c) << 2;
	pri_mask = (sprite_ctrl & 1) ? 0xfc : 0xf0; /* variable sprite/tile priority */
}

void asuka_state::bonzeadv_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	sprite_colbank = (sprite_ctrl & 0x3c) << 2;
	pri_mask = 0xf0; /* sprites over top bg layer */
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

uint32_t asuka_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_pc090oj->draw_sprites(screen, bitmap, cliprect);
	return 0;
}
