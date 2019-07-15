// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/***************************************************************************
  Functions to emulate video hardware on these Taito games:

  - rastan

***************************************************************************/

#include "emu.h"
#include "includes/rastan.h"
#include "screen.h"

/***************************************************************************/

void rastan_state::rastan_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	/* bits 5-7 are the sprite palette bank */
	sprite_colbank = (sprite_ctrl & 0xe0) >> 1;
	pri_mask = 0; /* sprites over everything */
}

void rastan_state::spritectrl_w(u16 data)
{
	m_pc090oj->sprite_ctrl_w(data);

	/* bit 4 unused */

	/* bits 0 and 1 are coin lockout */
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x02);

	/* bits 2 and 3 are the coin counters */
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
}

/***************************************************************************/

u32 rastan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[2];

	m_pc080sn->tilemap_update();

	layer[0] = 0;
	layer[1] = 1;

	screen.priority().fill(0, cliprect);

	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

	m_pc090oj->draw_sprites(screen, bitmap, cliprect);
	return 0;
}
