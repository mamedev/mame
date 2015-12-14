// license:BSD-3-Clause
// copyright-holders:Hau
/***************************************************************************

  video.c

***************************************************************************/

#include "emu.h"
#include "includes/qdrmfgp.h"


#if 0
K056832_CB_MEMBER(qdrmfgp_state::qdrmfgp_tile_callback)
{
	*color = ((*color>>2) & 0x0f) | m_pal;
}

K056832_CB_MEMBER(qdrmfgp_state::qdrmfgp2_tile_callback)
{
	*color = (*color>>1) & 0x7f;
}
#endif

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp)
{
#if 0
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, 2, 0);
	m_k056832->set_layer_offs(1, 4, 0);
	m_k056832->set_layer_offs(2, 6, 0);
	m_k056832->set_layer_offs(3, 8, 0);
#endif
}

VIDEO_START_MEMBER(qdrmfgp_state,qdrmfgp2)
{
#if 0
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, 3, 1);
	m_k056832->set_layer_offs(1, 5, 1);
	m_k056832->set_layer_offs(2, 7, 1);
	m_k056832->set_layer_offs(3, 9, 1);
#endif
}

/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t qdrmfgp_state::screen_update_qdrmfgp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 4);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);
#endif
	return 0;
}
