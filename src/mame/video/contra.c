/***************************************************************************

    video/contra.c

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/contra.h"


/***************************************************************************
**
**  Contra has palette RAM, but it also has four lookup table PROMs
**
**  0   sprites #0
**  1   tiles   #0
**  2   sprites #1
**  3   tiles   #1
**
***************************************************************************/

PALETTE_INIT( contra )
{
	int chip;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x80);

	for (chip = 0; chip < 2; chip++)
	{
		int pal;

		for (pal = 0; pal < 8; pal++)
		{
			int i;
			int clut = (chip << 1) | (pal & 1);

			for (i = 0; i < 0x100; i++)
			{
				UINT8 ctabentry;

				if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

				colortable_entry_set_value(machine->colortable, (chip << 11) | (pal << 8) | i, ctabentry);
			}
		}
	}
}


static void set_pens( running_machine *machine )
{
	contra_state *state = machine->driver_data<contra_state>();
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->paletteram[i] | (state->paletteram[i | 1] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine->colortable, i >> 1, color);
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	contra_state *state = machine->driver_data<contra_state>();
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121_1, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121_1, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121_1, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_1, 6);
	int attr = state->fg_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO(
			0,
			state->fg_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	contra_state *state = machine->driver_data<contra_state>();
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121_2, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121_2, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121_2, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_2, 6);
	int attr = state->bg_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	// 2009-12 FP: TO BE VERIFIED - old code used ctrl4 from chip 0?!?
	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO(
			1,
			state->bg_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	contra_state *state = machine->driver_data<contra_state>();
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121_1, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_1, 6);
	int attr = state->tx_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO(
			0,
			state->tx_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( contra )
{
	contra_state *state = machine->driver_data<contra_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->spriteram = auto_alloc_array(machine, UINT8, 0x800);
	state->spriteram_2 = auto_alloc_array(machine, UINT8, 0x800);

	state->bg_clip = machine->primary_screen->visible_area();
	state->bg_clip.min_x += 40;

	state->fg_clip = state->bg_clip;

	state->tx_clip = machine->primary_screen->visible_area();
	state->tx_clip.max_x = 39;
	state->tx_clip.min_x = 0;

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state_save_register_global_pointer(machine, state->spriteram, 0x800);
	state_save_register_global_pointer(machine, state->spriteram_2, 0x800);
}


/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_HANDLER( contra_fg_vram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->fg_vram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( contra_fg_cram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->fg_cram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( contra_bg_vram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->bg_vram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( contra_bg_cram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->bg_cram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( contra_text_vram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->tx_vram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE8_HANDLER( contra_text_cram_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();

	state->tx_cram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE8_HANDLER( contra_K007121_ctrl_0_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_1, 6);

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			memcpy(state->spriteram, space->machine->generic.spriteram.u8 + 0x800, 0x800);
		else
			memcpy(state->spriteram, space->machine->generic.spriteram.u8, 0x800);
	}

	if (offset == 6)
	{
		if (ctrl_6 != data)
			tilemap_mark_all_tiles_dirty(state->fg_tilemap);
	}

	if (offset == 7)
		tilemap_set_flip(state->fg_tilemap, (data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	k007121_ctrl_w(state->k007121_1, offset, data);
}

WRITE8_HANDLER( contra_K007121_ctrl_1_w )
{
	contra_state *state = space->machine->driver_data<contra_state>();
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_2, 6);

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			memcpy(state->spriteram_2, space->machine->generic.spriteram.u8 + 0x2800, 0x800);
		else
			memcpy(state->spriteram_2, space->machine->generic.spriteram.u8 + 0x2000, 0x800);
	}
	if (offset == 6)
	{
		if (ctrl_6 != data )
			tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
	if (offset == 7)
		tilemap_set_flip(state->bg_tilemap, (data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	k007121_ctrl_w(state->k007121_2, offset, data);
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank )
{
	contra_state *state = machine->driver_data<contra_state>();
	device_t *k007121 = bank ? state->k007121_2 : state->k007121_1;
	int base_color = (k007121_ctrlram_r(k007121, 6) & 0x30) * 2;
	const UINT8 *source;

	if (bank == 0)
		source = state->spriteram;
	else
		source = state->spriteram_2;

	k007121_sprites_draw(k007121, bitmap, cliprect, machine->gfx[bank], machine->colortable, source, base_color, 40, 0, (UINT32)-1);
}

VIDEO_UPDATE( contra )
{
	contra_state *state = screen->machine->driver_data<contra_state>();
	UINT8 ctrl_1_0 = k007121_ctrlram_r(state->k007121_1, 0);
	UINT8 ctrl_1_2 = k007121_ctrlram_r(state->k007121_1, 2);
	UINT8 ctrl_2_0 = k007121_ctrlram_r(state->k007121_2, 0);
	UINT8 ctrl_2_2 = k007121_ctrlram_r(state->k007121_2, 2);
	rectangle bg_finalclip = state->bg_clip;
	rectangle fg_finalclip = state->fg_clip;
	rectangle tx_finalclip = state->tx_clip;

	sect_rect(&bg_finalclip, cliprect);
	sect_rect(&fg_finalclip, cliprect);
	sect_rect(&tx_finalclip, cliprect);

	set_pens(screen->machine);

	tilemap_set_scrollx(state->fg_tilemap, 0, ctrl_1_0 - 40);
	tilemap_set_scrolly(state->fg_tilemap, 0, ctrl_1_2);
	tilemap_set_scrollx(state->bg_tilemap, 0, ctrl_2_0 - 40);
	tilemap_set_scrolly(state->bg_tilemap, 0, ctrl_2_2);

	tilemap_draw(bitmap, &bg_finalclip, state->bg_tilemap, 0 ,0);
	tilemap_draw(bitmap, &fg_finalclip, state->fg_tilemap, 0 ,0);
	draw_sprites(screen->machine,bitmap,cliprect, 0);
	draw_sprites(screen->machine,bitmap,cliprect, 1);
	tilemap_draw(bitmap, &tx_finalclip, state->tx_tilemap, 0 ,0);
	return 0;
}
