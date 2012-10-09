#include "emu.h"
#include "video/konicdev.h"
#include "includes/crshrace.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(crshrace_state::get_tile_info1)
{
	int code = m_videoram1[tile_index];

	SET_TILE_INFO_MEMBER(1, (code & 0xfff) + (m_roz_bank << 12), code >> 12, 0);
}

TILE_GET_INFO_MEMBER(crshrace_state::get_tile_info2)
{
	int code = m_videoram2[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void crshrace_state::video_start()
{

	m_tilemap1 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(crshrace_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap2 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(crshrace_state::get_tile_info2),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap1->set_transparent_pen(0x0f);
	m_tilemap2->set_transparent_pen(0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(crshrace_state::crshrace_videoram1_w)
{

	COMBINE_DATA(&m_videoram1[offset]);
	m_tilemap1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crshrace_state::crshrace_videoram2_w)
{

	COMBINE_DATA(&m_videoram2[offset]);
	m_tilemap2->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crshrace_state::crshrace_roz_bank_w)
{

	if (ACCESSING_BITS_0_7)
	{
		if (m_roz_bank != (data & 0xff))
		{
			m_roz_bank = data & 0xff;
			m_tilemap1->mark_all_dirty();
		}
	}
}


WRITE16_MEMBER(crshrace_state::crshrace_gfxctrl_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_gfxctrl = data & 0xdf;
		m_flipscreen = data & 0x20;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_bg( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	state->m_tilemap2->draw(bitmap, cliprect, 0, 0);
}


static void draw_fg(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_tilemap1, 0, 0, 1);
}


UINT32 crshrace_state::screen_update_crshrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	if (m_gfxctrl & 0x04)	/* display disable? */
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
		return 0;
	}

	bitmap.fill(0x1ff, cliprect);



	switch (m_gfxctrl & 0xfb)
	{
		case 0x00:	/* high score screen */
			m_spr->draw_sprites_crshrace(m_spriteram->buffer(), 0x2000, m_spriteram2->buffer(), machine(), bitmap, cliprect, m_flipscreen);
			draw_bg(machine(), bitmap, cliprect);
			draw_fg(machine(), bitmap, cliprect);
			break;
		case 0x01:
		case 0x02:
			draw_bg(machine(), bitmap, cliprect);
			draw_fg(machine(), bitmap, cliprect);
			m_spr->draw_sprites_crshrace(m_spriteram->buffer(), 0x2000, m_spriteram2->buffer(), machine(), bitmap, cliprect, m_flipscreen);
			break;
		default:
			popmessage("gfxctrl = %02x", m_gfxctrl);
			break;
	}
	return 0;
}

void crshrace_state::screen_eof_crshrace(screen_device &screen, bool state)
{
	m_spriteram->vblank_copy_rising(screen, state);
	m_spriteram2->vblank_copy_rising(screen, state);
}
