// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "taito_l.h"
#include "screen.h"

#include <algorithm>

/***************************************************************************

  Memory handlers

***************************************************************************/

void horshoes_state::horshoes_tile_cb(u32 &code)
{
	code |= m_horshoes_gfxbank << 12;
}

void horshoes_state::bankg_w(u8 data)
{
	if (m_horshoes_gfxbank != data)
	{
		m_horshoes_gfxbank = data;

		m_main_cpu->mark_all_layer_dirty();
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

WRITE_LINE_MEMBER(taitol_state::screen_vblank_taitol)
{
	// rising edge
	if (state)
	{
		m_main_cpu->screen_eof();
	}
}
