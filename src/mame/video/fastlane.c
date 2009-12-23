#include "driver.h"
#include "video/konicdev.h"
#include "includes/fastlane.h"


PALETTE_INIT( fastlane )
{
	int pal;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x400);

	for (pal = 0; pal < 0x10; pal++)
	{
		int i;

		for (i = 0; i < 0x400; i++)
		{
			UINT8 ctabentry = (i & 0x3f0) | color_prom[(pal << 4) | (i & 0x0f)];
			colortable_entry_set_value(machine->colortable, (pal << 10) | i, ctabentry);
		}
	}
}


static void set_pens( running_machine *machine )
{
	fastlane_state *state = (fastlane_state *)machine->driver_data;
	int i;

	for (i = 0x00; i < 0x800; i += 2)
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
	fastlane_state *state = (fastlane_state *)machine->driver_data;
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121, 5);
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
			code+bank*256,
			1 + 64 * (attr & 0x0f),
			0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	fastlane_state *state = (fastlane_state *)machine->driver_data;
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121, 5);
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
			0 + 64 * (attr & 0x0f),
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( fastlane )
{
	fastlane_state *state = (fastlane_state *)machine->driver_data;

	state->layer0 = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 8, 8, 32, 32);
	state->layer1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_scroll_rows(state->layer0, 32);

	state->clip0 = *video_screen_get_visible_area(machine->primary_screen);
	state->clip0.min_x += 40;

	state->clip1 = *video_screen_get_visible_area(machine->primary_screen);
	state->clip1.max_x = 39;
	state->clip1.min_x = 0;
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_HANDLER( fastlane_vram1_w )
{
	fastlane_state *state = (fastlane_state *)space->machine->driver_data;
	state->videoram1[offset] = data;
	tilemap_mark_tile_dirty(state->layer0, offset & 0x3ff);
}

WRITE8_HANDLER( fastlane_vram2_w )
{
	fastlane_state *state = (fastlane_state *)space->machine->driver_data;
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->layer1, offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

VIDEO_UPDATE( fastlane )
{
	fastlane_state *state = (fastlane_state *)screen->machine->driver_data;
	rectangle finalclip0 = state->clip0, finalclip1 = state->clip1;
	int i, xoffs;

	sect_rect(&finalclip0, cliprect);
	sect_rect(&finalclip1, cliprect);

	set_pens(screen->machine);

	/* set scroll registers */
	xoffs = k007121_ctrlram_r(state->k007121, 0);
	for (i = 0; i < 32; i++)
		tilemap_set_scrollx(state->layer0, i, state->k007121_regs[0x20 + i] + xoffs - 40);

	tilemap_set_scrolly(state->layer0, 0, k007121_ctrlram_r(state->k007121, 2));

	tilemap_draw(bitmap, &finalclip0, state->layer0, 0, 0);
	k007121_sprites_draw(state->k007121, bitmap, cliprect, screen->machine->gfx[0], screen->machine->colortable, state->spriteram, 0, 40, 0, -1);
	tilemap_draw(bitmap, &finalclip1, state->layer1, 0, 0);
	return 0;
}
