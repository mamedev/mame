// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
#include "emu.h"
#include "includes/asuka.h"

/**************************************************************
                 SPRITE READ AND WRITE HANDLERS
**************************************************************/

WRITE16_MEMBER(asuka_state::asuka_spritectrl_w)
{
	/* Bits 2-5 are color bank; in asuka games bit 0 is global priority */
	m_pc090oj->set_sprite_ctrl(((data & 0x3c) >> 2) | ((data & 0x1) << 15));
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

UINT32 asuka_state::screen_update_asuka(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 layer[3];

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

	/* Sprites may be over or under top bg layer */
	m_pc090oj->draw_sprites(bitmap, cliprect, screen.priority(), 2);
	return 0;
}


UINT32 asuka_state::screen_update_bonzeadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 layer[3];

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

	/* Sprites are always over both bg layers */
	m_pc090oj->draw_sprites(bitmap, cliprect, screen.priority(), 0);
	return 0;
}
