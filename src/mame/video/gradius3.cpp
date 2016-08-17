// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/gradius3.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(gradius3_state::tile_callback)
{
	static const int layer_colorbase[] = { 0 / 16, 512 / 16, 768 / 16 };

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(gradius3_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	#define L0 GFX_PMASK_1
	#define L1 GFX_PMASK_2
	#define L2 GFX_PMASK_4
	static const int primask[2][4] =
	{
		{ L0|L2, L0, L0|L2, L0|L1|L2 },
		{ L1|L2, L2, 0,     L0|L1|L2 }
	};
	#undef L0
	#undef L1
	#undef L2

	int pri = ((*color & 0x60) >> 5);

	if (m_priority == 0)
		*priority = primask[0][pri];
	else
		*priority = primask[1][pri];

	*code |= (*color & 0x01) << 13;
	*color = sprite_colorbase + ((*color & 0x1e) >> 1);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gradius3_state::gradius3_postload()
{
	m_k052109->gfx(0)->mark_all_dirty();
}

void gradius3_state::video_start()
{
	machine().save().register_postload(save_prepost_delegate(FUNC(gradius3_state::gradius3_postload), this));
}

/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(gradius3_state::gradius3_gfxrom_r)
{
	return (m_gfxrom[2 * offset + 1] << 8) | m_gfxrom[2 * offset];
}

WRITE16_MEMBER(gradius3_state::gradius3_gfxram_w)
{
	int oldword = m_gfxram[offset];

	COMBINE_DATA(&m_gfxram[offset]);

	if (oldword != m_gfxram[offset])
		m_k052109->gfx(0)->mark_dirty(offset / 16);
}

/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 gradius3_state::screen_update_gradius3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* TODO: this kludge enforces the char banks. For some reason, they don't work otherwise. */
	address_space &space = machine().driver_data()->generic_space();
	m_k052109->write(space, 0x1d80, 0x10);
	m_k052109->write(space, 0x1f00, 0x32);

	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	if (m_priority == 0)
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 2);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 4);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 1);
	}
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 4);
	}

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}
