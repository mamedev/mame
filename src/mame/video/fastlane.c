#include "emu.h"
#include "video/konicdev.h"
#include "includes/fastlane.h"


void fastlane_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int pal;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x400);

	for (pal = 0; pal < 0x10; pal++)
	{
		int i;

		for (i = 0; i < 0x400; i++)
		{
			UINT8 ctabentry = (i & 0x3f0) | color_prom[(pal << 4) | (i & 0x0f)];
			colortable_entry_set_value(machine().colortable, (pal << 10) | i, ctabentry);
		}
	}
}


static void set_pens( running_machine &machine )
{
	fastlane_state *state = machine.driver_data<fastlane_state>();
	int i;

	for (i = 0x00; i < 0x800; i += 2)
	{
		UINT16 data = state->m_paletteram[i | 1] | (state->m_paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(fastlane_state::get_tile_info0)
{
	UINT8 ctrl_3 = k007121_ctrlram_r(m_k007121, generic_space(), 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(m_k007121, generic_space(), 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(m_k007121, generic_space(), 5);
	int attr = m_videoram1[tile_index];
	int code = m_videoram1[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(
			0,
			code+bank*256,
			1 + 64 * (attr & 0x0f),
			0);
}

TILE_GET_INFO_MEMBER(fastlane_state::get_tile_info1)
{
	UINT8 ctrl_3 = k007121_ctrlram_r(m_k007121, generic_space(), 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(m_k007121, generic_space(), 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(m_k007121, generic_space(), 5);
	int attr = m_videoram2[tile_index];
	int code = m_videoram2[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(
			0,
			code+bank*256,
			0 + 64 * (attr & 0x0f),
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void fastlane_state::video_start()
{

	m_layer0 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fastlane_state::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_layer1 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fastlane_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_layer0->set_scroll_rows(32);

	m_clip0 = machine().primary_screen->visible_area();
	m_clip0.min_x += 40;

	m_clip1 = machine().primary_screen->visible_area();
	m_clip1.max_x = 39;
	m_clip1.min_x = 0;
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_MEMBER(fastlane_state::fastlane_vram1_w)
{
	m_videoram1[offset] = data;
	m_layer0->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(fastlane_state::fastlane_vram2_w)
{
	m_videoram2[offset] = data;
	m_layer1->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( fastlane )
{
	fastlane_state *state = screen.machine().driver_data<fastlane_state>();
	rectangle finalclip0 = state->m_clip0, finalclip1 = state->m_clip1;
	int i, xoffs;

	finalclip0 &= cliprect;
	finalclip1 &= cliprect;

	set_pens(screen.machine());

	/* set scroll registers */
	address_space &space = screen.machine().driver_data()->generic_space();
	xoffs = k007121_ctrlram_r(state->m_k007121, space, 0);
	for (i = 0; i < 32; i++)
		state->m_layer0->set_scrollx(i, state->m_k007121_regs[0x20 + i] + xoffs - 40);

	state->m_layer0->set_scrolly(0, k007121_ctrlram_r(state->m_k007121, space, 2));

	state->m_layer0->draw(bitmap, finalclip0, 0, 0);
	k007121_sprites_draw(state->m_k007121, bitmap, cliprect, screen.machine().gfx[0], screen.machine().colortable, state->m_spriteram, 0, 40, 0, (UINT32)-1);
	state->m_layer1->draw(bitmap, finalclip1, 0, 0);
	return 0;
}
