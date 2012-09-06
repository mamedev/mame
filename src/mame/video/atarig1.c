/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig1.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarig1_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarig1_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig1 )
{
	atarig1_state *state = machine.driver_data<atarig1_state>();

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x10);

	/* initialize the playfield */
	state->m_playfield_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(atarig1_state::get_playfield_tile_info),state), TILEMAP_SCAN_ROWS,  8,8, 64,64);

	/* initialize the motion objects */
	state->m_rle = machine.device("rle");

	/* initialize the alphanumerics */
	state->m_alpha_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(atarig1_state::get_alpha_tile_info),state), TILEMAP_SCAN_ROWS,  8,8, 64,32);
	state->m_alpha_tilemap->set_transparent_pen(0);

	/* reset statics */
	state->m_pfscroll_xoffset = state->m_is_pitfight ? 2 : 0;

	/* state saving */
	state->save_item(NAME(state->m_current_control));
	state->save_item(NAME(state->m_playfield_tile_bank));
	state->save_item(NAME(state->m_playfield_xscroll));
	state->save_item(NAME(state->m_playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig1_mo_control_w )
{
	atarig1_state *state = space->machine().driver_data<atarig1_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space->machine().primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->m_current_control);
}


void atarig1_scanline_update(screen_device &screen, int scanline)
{
	atarig1_state *state = screen.machine().driver_data<atarig1_state>();
	UINT16 *base = &state->m_alpha[(scanline / 8) * 64 + 48];
	int i;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->m_alpha[0x800])
		return;
	screen.update_partial(MAX(scanline - 1, 0));

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		/* first word controls horizontal scroll */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) + state->m_pfscroll_xoffset) & 0x1ff;
			if (newscroll != state->m_playfield_xscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				state->m_playfield_tilemap->set_scrollx(0, newscroll);
				state->m_playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != state->m_playfield_yscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				state->m_playfield_tilemap->set_scrolly(0, newscroll);
				state->m_playfield_yscroll = newscroll;
			}
			if (newbank != state->m_playfield_tile_bank)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				state->m_playfield_tilemap->mark_all_dirty();
				state->m_playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( atarig1 )
{
	atarig1_state *state = screen.machine().driver_data<atarig1_state>();

	/* draw the playfield */
	state->m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);

	/* copy the motion objects on top */
	copybitmap_trans(bitmap, *atarirle_get_vram(state->m_rle, 0), 0, 0, 0, 0, cliprect, 0);

	/* add the alpha on top */
	state->m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_VBLANK( atarig1 )
{
	// rising edge
	if (vblank_on)
	{
		atarig1_state *state = screen.machine().driver_data<atarig1_state>();

		atarirle_eof(state->m_rle);
	}
}
