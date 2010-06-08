/***************************************************************************

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/flkatck.h"

/***************************************************************************

  Callbacks for the K007121

***************************************************************************/

static TILE_GET_INFO( get_tile_info_A )
{
	flkatck_state *state = (flkatck_state *)machine->driver_data;
	UINT8 ctrl_0 = k007121_ctrlram_r(state->k007121, 0);
	UINT8 ctrl_2 = k007121_ctrlram_r(state->k007121, 2);
	UINT8 ctrl_3 = k007121_ctrlram_r(state->k007121, 3);
	UINT8 ctrl_4 = k007121_ctrlram_r(state->k007121, 4);
	UINT8 ctrl_5 = k007121_ctrlram_r(state->k007121, 5);
	int attr = state->k007121_ram[tile_index];
	int code = state->k007121_ram[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	if ((attr == 0x0d) && (!ctrl_0) && (!ctrl_2))
		bank = 0;	/*  this allows the game to print text
                    in all banks selected by the k007121 */

	SET_TILE_INFO(
			0,
			code + 256*bank,
			(attr & 0x0f) + 16,
			(attr & 0x20) ? TILE_FLIPY : 0);
}

static TILE_GET_INFO( get_tile_info_B )
{
	flkatck_state *state = (flkatck_state *)machine->driver_data;
	int attr = state->k007121_ram[tile_index + 0x800];
	int code = state->k007121_ram[tile_index + 0xc00];

	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f) + 16,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( flkatck )
{
	flkatck_state *state = (flkatck_state *)machine->driver_data;
	state->k007121_tilemap[0] = tilemap_create(machine, get_tile_info_A, tilemap_scan_rows, 8, 8, 32, 32);
	state->k007121_tilemap[1] = tilemap_create(machine, get_tile_info_B, tilemap_scan_rows, 8, 8, 32, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( flkatck_k007121_w )
{
	flkatck_state *state = (flkatck_state *)space->machine->driver_data;

	state->k007121_ram[offset] = data;
	if (offset < 0x1000)	/* tiles */
	{
		if (offset & 0x800)	/* score */
			tilemap_mark_tile_dirty(state->k007121_tilemap[1], offset & 0x3ff);
		else
			tilemap_mark_tile_dirty(state->k007121_tilemap[0], offset & 0x3ff);
	}
}

WRITE8_HANDLER( flkatck_k007121_regs_w )
{
	flkatck_state *state = (flkatck_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0x04:	/* ROM bank select */
			if (data != k007121_ctrlram_r(state->k007121, 4))
				tilemap_mark_all_tiles_dirty_all(space->machine);
			break;

		case 0x07:	/* flip screen + IRQ control */
			state->flipscreen = data & 0x08;
			tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			state->irq_enabled = data & 0x02;
			break;
	}

	k007121_ctrl_w(state->k007121, offset, data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

/***************************************************************************

    Flack Attack sprites. Each sprite has 16 bytes!:


***************************************************************************/

VIDEO_UPDATE( flkatck )
{
	flkatck_state *state = (flkatck_state *)screen->machine->driver_data;
	rectangle clip[2];
	const rectangle &visarea = screen->visible_area();

	if (state->flipscreen)
	{
		clip[0] = visarea;
		clip[0].max_x -= 40;

		clip[1] = visarea;
		clip[1].min_x = clip[1].max_x - 40;

		tilemap_set_scrollx(state->k007121_tilemap[0], 0, k007121_ctrlram_r(state->k007121, 0) - 56 );
		tilemap_set_scrolly(state->k007121_tilemap[0], 0, k007121_ctrlram_r(state->k007121, 2));
		tilemap_set_scrollx(state->k007121_tilemap[1], 0, -16);
	}
	else
	{
		clip[0] = visarea;
		clip[0].min_x += 40;

		clip[1] = visarea;
		clip[1].max_x = 39;
		clip[1].min_x = 0;

		tilemap_set_scrollx(state->k007121_tilemap[0], 0, k007121_ctrlram_r(state->k007121, 0) - 40 );
		tilemap_set_scrolly(state->k007121_tilemap[0], 0, k007121_ctrlram_r(state->k007121, 2));
		tilemap_set_scrollx(state->k007121_tilemap[1], 0, 0);
	}

	/* compute clipping */
	sect_rect(&clip[0], cliprect);
	sect_rect(&clip[1], cliprect);

	/* draw the graphics */
	tilemap_draw(bitmap, &clip[0], state->k007121_tilemap[0], 0, 0);
	k007121_sprites_draw(state->k007121, bitmap, cliprect, screen->machine->gfx[0], NULL, &state->k007121_ram[0x1000], 0, 40, 0, (UINT32)-1);
	tilemap_draw(bitmap, &clip[1], state->k007121_tilemap[1], 0, 0);
	return 0;
}
