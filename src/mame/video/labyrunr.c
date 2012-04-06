#include "emu.h"
#include "video/konicdev.h"
#include "includes/labyrunr.h"

PALETTE_INIT( labyrunr )
{
	int pal;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		/* chars, no lookup table */
		if (pal & 1)
		{
			int i;

			for (i = 0; i < 0x100; i++)
				colortable_entry_set_value(machine.colortable, (pal << 8) | i, (pal << 4) | (i & 0x0f));
		}
		/* sprites */
		else
		{
			int i;

			for (i = 0; i < 0x100; i++)
			{
				UINT8 ctabentry;

				if (color_prom[i] == 0)
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[i] & 0x0f);

				colortable_entry_set_value(machine.colortable, (pal << 8) | i, ctabentry);
			}
		}
	}
}


static void set_pens( running_machine &machine )
{
	labyrunr_state *state = machine.driver_data<labyrunr_state>();
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->m_paletteram[i | 1] | (state->m_paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine.colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	labyrunr_state *state = machine.driver_data<labyrunr_state>();
	UINT8 ctrl_3 = k007121_ctrlram_r(state->m_k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->m_k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->m_k007121, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->m_k007121, 6);
	int attr = state->m_videoram1[tile_index];
	int code = state->m_videoram1[tile_index + 0x400];
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

	SET_TILE_INFO(
			0,
			code + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16)+(attr & 7),
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	labyrunr_state *state = machine.driver_data<labyrunr_state>();
	UINT8 ctrl_3 = k007121_ctrlram_r(state->m_k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->m_k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->m_k007121, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->m_k007121, 6);
	int attr = state->m_videoram2[tile_index];
	int code = state->m_videoram2[tile_index + 0x400];
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

	SET_TILE_INFO(
			0,
			code+bank*256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( labyrunr )
{
	labyrunr_state *state = machine.driver_data<labyrunr_state>();

	state->m_layer0 = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_layer1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_layer0->set_transparent_pen(0);
	state->m_layer1->set_transparent_pen(0);

	state->m_clip0 = machine.primary_screen->visible_area();
	state->m_clip0.min_x += 40;

	state->m_clip1 = machine.primary_screen->visible_area();
	state->m_clip1.max_x = 39;
	state->m_clip1.min_x = 0;

	state->m_layer0->set_scroll_cols(32);
}



/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_MEMBER(labyrunr_state::labyrunr_vram1_w)
{
	m_videoram1[offset] = data;
	m_layer0->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(labyrunr_state::labyrunr_vram2_w)
{
	m_videoram2[offset] = data;
	m_layer1->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

SCREEN_UPDATE_IND16( labyrunr )
{
	labyrunr_state *state = screen.machine().driver_data<labyrunr_state>();
	UINT8 ctrl_0 = k007121_ctrlram_r(state->m_k007121, 0);
	rectangle finalclip0, finalclip1;

	set_pens(screen.machine());

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (~k007121_ctrlram_r(state->m_k007121, 3) & 0x20)
	{
		int i;

		finalclip0 = state->m_clip0;
		finalclip1 = state->m_clip1;

		finalclip0 &= cliprect;
		finalclip1 &= cliprect;

		state->m_layer0->set_scrollx(0, ctrl_0 - 40);
		state->m_layer1->set_scrollx(0, 0);

		for(i = 0; i < 32; i++)
		{
			/* enable colscroll */
			if((k007121_ctrlram_r(state->m_k007121, 1) & 6) == 6) // it's probably just one bit, but it's only used once in the game so I don't know which it's
				state->m_layer0->set_scrolly((i + 2) & 0x1f, k007121_ctrlram_r(state->m_k007121, 2) + state->m_scrollram[i]);
			else
				state->m_layer0->set_scrolly((i + 2) & 0x1f, k007121_ctrlram_r(state->m_k007121, 2));
		}

		state->m_layer0->draw(bitmap, finalclip0, TILEMAP_DRAW_OPAQUE, 0);
		k007121_sprites_draw(state->m_k007121, bitmap, cliprect, screen.machine().gfx[0], screen.machine().colortable, state->m_spriteram,(k007121_ctrlram_r(state->m_k007121, 6) & 0x30) * 2, 40,0,(k007121_ctrlram_r(state->m_k007121, 3) & 0x40) >> 5);
		/* we ignore the transparency because layer1 is drawn only at the top of the screen also covering sprites */
		state->m_layer1->draw(bitmap, finalclip1, TILEMAP_DRAW_OPAQUE, 0);
	}
	else
	{
		int use_clip3[2] = { 0, 0 };
		rectangle finalclip3;

		/* custom cliprects needed for the weird effect used in the endinq sequence to hide and show the needed part of text */
		finalclip0.min_y = finalclip1.min_y = cliprect.min_y;
		finalclip0.max_y = finalclip1.max_y = cliprect.max_y;

		if(k007121_ctrlram_r(state->m_k007121, 1) & 1)
		{
			finalclip0.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip0.max_x = cliprect.max_x;

			if(ctrl_0 >= 40)
			{
				finalclip1.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[0] = 1;

				finalclip1.min_x = 40 - ctrl_0;
			}

			finalclip1.max_x = cliprect.max_x - ctrl_0 + 8;

		}
		else
		{
			if(ctrl_0 >= 40)
			{
				finalclip0.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[1] = 1;

				finalclip0.min_x = 40 - ctrl_0;
			}

			finalclip0.max_x = cliprect.max_x - ctrl_0 + 8;

			finalclip1.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip1.max_x = cliprect.max_x;
		}

		if(use_clip3[0] || use_clip3[1])
		{
			finalclip3.min_y = cliprect.min_y;
			finalclip3.max_y = cliprect.max_y;
			finalclip3.min_x = cliprect.min_x;
			finalclip3.max_x = 40 - ctrl_0 - 8;
		}

		state->m_layer0->set_scrollx(0, ctrl_0 - 40);
		state->m_layer1->set_scrollx(0, ctrl_0 - 40);

		state->m_layer0->draw(bitmap, finalclip0, 0, 1);
		if(use_clip3[0])
			state->m_layer0->draw(bitmap, finalclip3, 0, 1);

		state->m_layer1->draw(bitmap, finalclip1, 0, 1);
		if(use_clip3[1])
			state->m_layer1->draw(bitmap, finalclip3, 0, 1);

		k007121_sprites_draw(state->m_k007121, bitmap, cliprect, screen.machine().gfx[0], screen.machine().colortable, state->m_spriteram, (k007121_ctrlram_r(state->m_k007121, 6) & 0x30) * 2,40,0,(k007121_ctrlram_r(state->m_k007121, 3) & 0x40) >> 5);
	}
	return 0;
}
