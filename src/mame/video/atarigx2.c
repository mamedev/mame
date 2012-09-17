/***************************************************************************

    Atari GX2 hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank1

****************************************************************************/


#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarigx2.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarigx2_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarigx2_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (m_playfield_base >> 5) + ((m_playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
	tileinfo.category = (m_playfield_color_bank >> 2) & 7;
}


TILEMAP_MAPPER_MEMBER(atarigx2_state::atarigx2_playfield_scan)
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(atarigx2_state,atarigx2)
{

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine(), 0, 2, 0x0f, 0x30);

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarigx2_state::get_playfield_tile_info),this), tilemap_mapper_delegate(FUNC(atarigx2_state::atarigx2_playfield_scan),this),  8,8, 128,64);

	/* initialize the motion objects */
	m_rle = machine().device("rle");

	/* initialize the alphanumerics */
	m_alpha_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarigx2_state::get_alpha_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,32);
	m_alpha_tilemap->set_transparent_pen(0);

	/* save states */
	save_item(NAME(m_current_control));
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_color_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarigx2_mo_control_w )
{
	atarigx2_state *state = space.machine().driver_data<atarigx2_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space.machine().primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->m_current_control);
}


void atarigx2_scanline_update(screen_device &screen, int scanline)
{
	atarigx2_state *state = screen.machine().driver_data<atarigx2_state>();
	UINT32 *base = &state->m_alpha32[(scanline / 8) * 32 + 24];
	int i;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->m_alpha32[0x400])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT32 word = *base++;

		if (word & 0x80000000)
		{
			int newscroll = (word >> 21) & 0x3ff;
			int newbank = (word >> 16) & 0x1f;
			if (newscroll != state->m_playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->set_scrollx(0, newscroll);
				state->m_playfield_xscroll = newscroll;
			}
			if (newbank != state->m_playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->mark_all_dirty();
				state->m_playfield_color_bank = newbank;
			}
		}

		if (word & 0x00008000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 15;
			if (newscroll != state->m_playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->set_scrolly(0, newscroll);
				state->m_playfield_yscroll = newscroll;
			}
			if (newbank != state->m_playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
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

UINT32 atarigx2_state::screen_update_atarigx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind8 &priority_bitmap = screen.machine().priority_bitmap;

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(bitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(bitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(bitmap, cliprect, 3, 3);
	m_playfield_tilemap->draw(bitmap, cliprect, 4, 4);
	m_playfield_tilemap->draw(bitmap, cliprect, 5, 5);
	m_playfield_tilemap->draw(bitmap, cliprect, 6, 6);
	m_playfield_tilemap->draw(bitmap, cliprect, 7, 7);

	/* copy the motion objects on top */
	{
		bitmap_ind16 *mo_bitmap = atarirle_get_vram(m_rle, 0);
		int left	= cliprect.min_x;
		int top		= cliprect.min_y;
		int right	= cliprect.max_x + 1;
		int bottom	= cliprect.max_y + 1;
		int x, y;

		/* now blend with the playfield */
		for (y = top; y < bottom; y++)
		{
			UINT16 *pf = &bitmap.pix16(y);
			UINT16 *mo = &mo_bitmap->pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = left; x < right; x++)
				if (mo[x] && (mo[x] >> ATARIRLE_PRIORITY_SHIFT) >= pri[x])
					pf[x] = mo[x] & ATARIRLE_DATA_MASK;
		}
	}

	/* add the alpha on top */
	m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

void atarigx2_state::screen_eof_atarigx2(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{

		atarirle_eof(m_rle);
	}
}
