/***************************************************************************

    Haunted Castle video emulation

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/hcastle.h"


PALETTE_INIT( hcastle )
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


static void set_pens(running_machine *machine)
{
	hcastle_state *state = (hcastle_state *)machine->driver_data;
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

static TILEMAP_MAPPER( tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);	/* skip 0x400 */
}

static TILE_GET_INFO( get_fg_tile_info )
{
	hcastle_state *state = (hcastle_state *)machine->driver_data;
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121_1, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_1, 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = state->pf1_videoram[tile_index];
	int tile = state->pf1_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO(
			0,
			tile + bank * 0x100 + state->pf1_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	hcastle_state *state = (hcastle_state *)machine->driver_data;
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121_2, 5);
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_2, 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = state->pf2_videoram[tile_index];
	int tile = state->pf2_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO(
			1,
			tile + bank * 0x100 + state->pf2_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( hcastle )
{
	hcastle_state *state = (hcastle_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan, 8, 8, 64, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}



/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_HANDLER( hcastle_pf1_video_w )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;
	state->pf1_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0xbff);
}

WRITE8_HANDLER( hcastle_pf2_video_w )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;
	state->pf2_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0xbff);
}

WRITE8_HANDLER( hcastle_gfxbank_w )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;
	state->gfx_bank = data;
}

READ8_HANDLER( hcastle_gfxbank_r )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;
	return state->gfx_bank;
}

WRITE8_HANDLER( hcastle_pf1_control_w )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			buffer_spriteram(space->machine, space->machine->generic.spriteram.u8 + 0x800, 0x800);
		else
			buffer_spriteram(space->machine, space->machine->generic.spriteram.u8, 0x800);
	}
	else if (offset == 7)
	{
		tilemap_set_flip(state->fg_tilemap, (data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	k007121_ctrl_w(state->k007121_1, offset, data);
}

WRITE8_HANDLER( hcastle_pf2_control_w )
{
	hcastle_state *state = (hcastle_state *)space->machine->driver_data;

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			buffer_spriteram_2(space->machine, space->machine->generic.spriteram2.u8 + 0x800, 0x800);
		else
			buffer_spriteram_2(space->machine, space->machine->generic.spriteram2.u8, 0x800);
	}
	else if (offset == 7)
	{
		tilemap_set_flip(state->bg_tilemap, (data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	k007121_ctrl_w(state->k007121_2, offset, data);
}

/*****************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *sbank, int bank )
{
	hcastle_state *state = (hcastle_state *)machine->driver_data;
	const device_config *k007121 = bank ? state->k007121_2 : state->k007121_1;
	int base_color = (k007121_ctrlram_r(k007121, 6) & 0x30) * 2;
	int bank_base = (bank == 0) ? 0x4000 * (state->gfx_bank & 1) : 0;

	k007121_sprites_draw(k007121, bitmap, cliprect, machine->gfx[bank], machine->colortable, sbank, base_color, 0, bank_base, (UINT32)-1);
}

/*****************************************************************************/

VIDEO_UPDATE( hcastle )
{
	hcastle_state *state = (hcastle_state *)screen->machine->driver_data;

	UINT8 ctrl_1_0 = k007121_ctrlram_r(state->k007121_1, 0);
	UINT8 ctrl_1_1 = k007121_ctrlram_r(state->k007121_1, 1);
	UINT8 ctrl_1_2 = k007121_ctrlram_r(state->k007121_1, 2);
	UINT8 ctrl_1_3 = k007121_ctrlram_r(state->k007121_1, 3);
	UINT8 ctrl_2_0 = k007121_ctrlram_r(state->k007121_2, 0);
	UINT8 ctrl_2_1 = k007121_ctrlram_r(state->k007121_2, 1);
	UINT8 ctrl_2_2 = k007121_ctrlram_r(state->k007121_2, 2);
	UINT8 ctrl_2_3 = k007121_ctrlram_r(state->k007121_2, 3);

	set_pens(screen->machine);

	state->pf1_bankbase = 0x0000;
	state->pf2_bankbase = 0x4000 * ((state->gfx_bank & 2) >> 1);

	if (ctrl_1_3 & 0x01)
		state->pf1_bankbase += 0x2000;
	if (ctrl_2_3 & 0x01)
		state->pf2_bankbase += 0x2000;

	if (state->pf1_bankbase != state->old_pf1)
		tilemap_mark_all_tiles_dirty(state->fg_tilemap);

	if (state->pf2_bankbase != state->old_pf2)
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);

	state->old_pf1 = state->pf1_bankbase;
	state->old_pf2 = state->pf2_bankbase;

	tilemap_set_scrolly(state->bg_tilemap, 0, ctrl_2_2);
	tilemap_set_scrollx(state->bg_tilemap, 0, ((ctrl_2_1 << 8) + ctrl_2_0));
	tilemap_set_scrolly(state->fg_tilemap, 0, ctrl_1_2);
	tilemap_set_scrollx(state->fg_tilemap, 0, ((ctrl_1_1 << 8) + ctrl_1_0));

//  /* Sprite priority */
//  if (ctrl_1_3 & 0x20)
	if ((state->gfx_bank & 0x04) == 0)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u8, 0);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram2.u8, 1);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u8, 0);
		draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram2.u8, 1);
	}
	return 0;
}
