#include "emu.h"
#include "video/konicdev.h"
#include "includes/labyrunr.h"

PALETTE_INIT( labyrunr )
{
	int pal;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		/* chars, no lookup table */
		if (pal & 1)
		{
			int i;

			for (i = 0; i < 0x100; i++)
				colortable_entry_set_value(machine->colortable, (pal << 8) | i, (pal << 4) | (i & 0x0f));
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

				colortable_entry_set_value(machine->colortable, (pal << 8) | i, ctabentry);
			}
		}
	}
}


static void set_pens( running_machine *machine )
{
	labyrunr_state *state = (labyrunr_state *)machine->driver_data;
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->paletteram[i | 1] | (state->paletteram[i] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine->colortable, i >> 1, color);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	labyrunr_state *state = (labyrunr_state *)machine->driver_data;
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121, 6);
	int attr = state->videoram1[tile_index];
	int code = state->videoram1[tile_index + 0x400];
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
	labyrunr_state *state = (labyrunr_state *)machine->driver_data;
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121, 6);
	int attr = state->videoram2[tile_index];
	int code = state->videoram2[tile_index + 0x400];
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
	labyrunr_state *state = (labyrunr_state *)machine->driver_data;

	state->layer0 = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 8, 8, 32, 32);
	state->layer1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->layer0, 0);
	tilemap_set_transparent_pen(state->layer1, 0);

	state->clip0 = *video_screen_get_visible_area(machine->primary_screen);
	state->clip0.min_x += 40;

	state->clip1 = *video_screen_get_visible_area(machine->primary_screen);
	state->clip1.max_x = 39;
	state->clip1.min_x = 0;

	tilemap_set_scroll_cols(state->layer0, 32);
}



/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_HANDLER( labyrunr_vram1_w )
{
	labyrunr_state *state = (labyrunr_state *)space->machine->driver_data;
	state->videoram1[offset] = data;
	tilemap_mark_tile_dirty(state->layer0, offset & 0x3ff);
}

WRITE8_HANDLER( labyrunr_vram2_w )
{
	labyrunr_state *state = (labyrunr_state *)space->machine->driver_data;
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->layer1, offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( labyrunr )
{
	labyrunr_state *state = (labyrunr_state *)screen->machine->driver_data;
	UINT8 ctrl_0 = k007121_ctrlram_r(state->k007121, 0);
	rectangle finalclip0, finalclip1;

	set_pens(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap, cliprect,0);
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (~k007121_ctrlram_r(state->k007121, 3) & 0x20)
	{
		int i;

		finalclip0 = state->clip0;
		finalclip1 = state->clip1;

		sect_rect(&finalclip0, cliprect);
		sect_rect(&finalclip1, cliprect);

		tilemap_set_scrollx(state->layer0, 0, ctrl_0 - 40);
		tilemap_set_scrollx(state->layer1, 0, 0);

		for(i = 0; i < 32; i++)
		{
			/* enable colscroll */
			if((k007121_ctrlram_r(state->k007121, 1) & 6) == 6) // it's probably just one bit, but it's only used once in the game so I don't know which it's
				tilemap_set_scrolly(state->layer0, (i + 2) & 0x1f, k007121_ctrlram_r(state->k007121, 2) + state->scrollram[i]);
			else
				tilemap_set_scrolly(state->layer0, (i + 2) & 0x1f, k007121_ctrlram_r(state->k007121, 2));
		}

		tilemap_draw(bitmap, &finalclip0, state->layer0, TILEMAP_DRAW_OPAQUE, 0);
		k007121_sprites_draw(state->k007121, bitmap, cliprect, screen->machine->gfx[0], screen->machine->colortable, state->spriteram,(k007121_ctrlram_r(state->k007121, 6) & 0x30) * 2, 40,0,(k007121_ctrlram_r(state->k007121, 3) & 0x40) >> 5);
		/* we ignore the transparency because layer1 is drawn only at the top of the screen also covering sprites */
		tilemap_draw(bitmap, &finalclip1, state->layer1, TILEMAP_DRAW_OPAQUE, 0);
	}
	else
	{
		int use_clip3[2] = { 0, 0 };
		rectangle finalclip3;

		/* custom cliprects needed for the weird effect used in the endinq sequence to hide and show the needed part of text */
		finalclip0.min_y = finalclip1.min_y = cliprect->min_y;
		finalclip0.max_y = finalclip1.max_y = cliprect->max_y;

		if(k007121_ctrlram_r(state->k007121, 1) & 1)
		{
			finalclip0.min_x = cliprect->max_x - ctrl_0 + 8;
			finalclip0.max_x = cliprect->max_x;

			if(ctrl_0 >= 40)
			{
				finalclip1.min_x = cliprect->min_x;
			}
			else
			{
				use_clip3[0] = 1;

				finalclip1.min_x = 40 - ctrl_0;
			}

			finalclip1.max_x = cliprect->max_x - ctrl_0 + 8;

		}
		else
		{
			if(ctrl_0 >= 40)
			{
				finalclip0.min_x = cliprect->min_x;
			}
			else
			{
				use_clip3[1] = 1;

				finalclip0.min_x = 40 - ctrl_0;
			}

			finalclip0.max_x = cliprect->max_x - ctrl_0 + 8;

			finalclip1.min_x = cliprect->max_x - ctrl_0 + 8;
			finalclip1.max_x = cliprect->max_x;
		}

		if(use_clip3[0] || use_clip3[1])
		{
			finalclip3.min_y = cliprect->min_y;
			finalclip3.max_y = cliprect->max_y;
			finalclip3.min_x = cliprect->min_x;
			finalclip3.max_x = 40 - ctrl_0 - 8;
		}

		tilemap_set_scrollx(state->layer0, 0, ctrl_0 - 40);
		tilemap_set_scrollx(state->layer1, 0, ctrl_0 - 40);

		tilemap_draw(bitmap, &finalclip0, state->layer0, 0, 1);
		if(use_clip3[0])
			tilemap_draw(bitmap, &finalclip3, state->layer0, 0, 1);

		tilemap_draw(bitmap, &finalclip1, state->layer1, 0, 1);
		if(use_clip3[1])
			tilemap_draw(bitmap, &finalclip3, state->layer1, 0, 1);

		k007121_sprites_draw(state->k007121, bitmap, cliprect, screen->machine->gfx[0], screen->machine->colortable, state->spriteram, (k007121_ctrlram_r(state->k007121, 6) & 0x30) * 2,40,0,(k007121_ctrlram_r(state->k007121, 3) & 0x40) >> 5);
	}
	return 0;
}
