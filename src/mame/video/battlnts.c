#include "emu.h"
#include "includes/battlnts.h"

/***************************************************************************

  Callback for the K007342

***************************************************************************/

void battlnts_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags)
{
	battlnts_state *state = machine.driver_data<battlnts_state>();

	*code |= ((*color & 0x0f) << 9) | ((*color & 0x40) << 2);
	*color = state->m_layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

void battlnts_sprite_callback(running_machine &machine, int *code,int *color)
{
	battlnts_state *state = machine.driver_data<battlnts_state>();

	*code |= ((*color & 0xc0) << 2) | state->m_spritebank;
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
