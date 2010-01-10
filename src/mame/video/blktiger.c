#include "emu.h"
#include "includes/blktiger.h"


#define BGRAM_BANK_SIZE 0x1000
#define BGRAM_BANKS 4


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( bg8x4_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4) + ((row & 0x30) << 7);
}

static TILEMAP_MAPPER( bg4x8_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x30) << 4) + ((row & 0x70) << 6);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	blktiger_state *state = (blktiger_state *)machine->driver_data;
	/* the tile priority table is a guess compiled by looking at the game. It
       was not derived from a PROM so it could be wrong. */
	static const UINT8 split_table[16] =
	{
		3,3,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0
	};
	UINT8 attr = state->scroll_ram[2 * tile_index + 1];
	int color = (attr & 0x78) >> 3;
	SET_TILE_INFO(
			1,
			state->scroll_ram[2 * tile_index] + ((attr & 0x07) << 8),
			color,
			(attr & 0x80) ? TILE_FLIPX : 0);
	tileinfo->group = split_table[color];
}

static TILE_GET_INFO( get_tx_tile_info )
{
	blktiger_state *state = (blktiger_state *)machine->driver_data;
	UINT8 attr = state->txvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			state->txvideoram[tile_index] + ((attr & 0xe0) << 3),
			attr & 0x1f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( blktiger )
{
	blktiger_state *state = (blktiger_state *)machine->driver_data;

	state->scroll_ram = auto_alloc_array(machine, UINT8, BGRAM_BANK_SIZE * BGRAM_BANKS);

	state->tx_tilemap =    tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg_tilemap8x4 = tilemap_create(machine, get_bg_tile_info, bg8x4_scan, 16, 16, 128, 64);
	state->bg_tilemap4x8 = tilemap_create(machine, get_bg_tile_info, bg4x8_scan, 16, 16, 64, 128);

	tilemap_set_transparent_pen(state->tx_tilemap, 3);

	tilemap_set_transmask(state->bg_tilemap8x4, 0, 0xffff, 0x8000);	/* split type 0 is totally transparent in front half */
	tilemap_set_transmask(state->bg_tilemap8x4, 1, 0xfff0, 0x800f);	/* split type 1 has pens 4-15 transparent in front half */
	tilemap_set_transmask(state->bg_tilemap8x4, 2, 0xff00, 0x80ff);	/* split type 1 has pens 8-15 transparent in front half */
	tilemap_set_transmask(state->bg_tilemap8x4, 3, 0xf000, 0x8fff);	/* split type 1 has pens 12-15 transparent in front half */
	tilemap_set_transmask(state->bg_tilemap4x8, 0, 0xffff, 0x8000);
	tilemap_set_transmask(state->bg_tilemap4x8, 1, 0xfff0, 0x800f);
	tilemap_set_transmask(state->bg_tilemap4x8, 2, 0xff00, 0x80ff);
	tilemap_set_transmask(state->bg_tilemap4x8, 3, 0xf000, 0x8fff);

	state_save_register_global_pointer(machine, state->scroll_ram, BGRAM_BANK_SIZE * BGRAM_BANKS);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( blktiger_txvideoram_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	state->txvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->tx_tilemap,offset & 0x3ff);
}

READ8_HANDLER( blktiger_bgvideoram_r )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	return state->scroll_ram[offset + state->scroll_bank];
}

WRITE8_HANDLER( blktiger_bgvideoram_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	offset += state->scroll_bank;

	state->scroll_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap8x4, offset / 2);
	tilemap_mark_tile_dirty(state->bg_tilemap4x8, offset / 2);
}

WRITE8_HANDLER( blktiger_bgvideoram_bank_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	state->scroll_bank = (data % BGRAM_BANKS) * BGRAM_BANK_SIZE;
}


WRITE8_HANDLER( blktiger_scrolly_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	int scrolly;

	state->scroll_y[offset] = data;
	scrolly = state->scroll_y[0] | (state->scroll_y[1] << 8);
	tilemap_set_scrolly(state->bg_tilemap8x4, 0, scrolly);
	tilemap_set_scrolly(state->bg_tilemap4x8, 0, scrolly);
}

WRITE8_HANDLER( blktiger_scrollx_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	int scrollx;

	state->scroll_x[offset] = data;
	scrollx = state->scroll_x[0] | (state->scroll_x[1] << 8);
	tilemap_set_scrollx(state->bg_tilemap8x4, 0, scrollx);
	tilemap_set_scrollx(state->bg_tilemap4x8, 0, scrollx);
}


WRITE8_HANDLER( blktiger_video_control_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	/* bits 0 and 1 are coin counters */
	coin_counter_w(space->machine, 0,data & 1);
	coin_counter_w(space->machine, 1,data & 2);

	/* bit 5 resets the sound CPU */
	cpu_set_input_line(state->audiocpu, INPUT_LINE_RESET, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 6 flips screen */
	flip_screen_set(space->machine, data & 0x40);

	/* bit 7 enables characters? Just a guess */
	state->chon = ~data & 0x80;
}

WRITE8_HANDLER( blktiger_video_enable_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;

	/* not sure which is which, but I think that bit 1 and 2 enable background and sprites */
	/* bit 1 enables bg ? */
	state->bgon = ~data & 0x02;

	/* bit 2 enables sprites ? */
	state->objon = ~data & 0x04;
}

WRITE8_HANDLER( blktiger_screen_layout_w )
{
	blktiger_state *state = (blktiger_state *)space->machine->driver_data;
	state->screen_layout = data;
	tilemap_set_enable(state->bg_tilemap8x4, state->screen_layout);
	tilemap_set_enable(state->bg_tilemap4x8, !state->screen_layout);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
//  blktiger_state *state = (blktiger_state *)machine->driver_data;
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	int offs;

	/* Draw the sprites. */
	for (offs = machine->generic.spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int attr = buffered_spriteram[offs+1];
		int sx = buffered_spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = buffered_spriteram[offs + 2];
		int code = buffered_spriteram[offs] | ((attr & 0xe0) << 3);
		int color = attr & 0x07;
		int flipx = attr & 0x08;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
				code,
				color,
				flipx,flip_screen_get(machine),
				sx,sy,15);
	}
}

VIDEO_UPDATE( blktiger )
{
	blktiger_state *state = (blktiger_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, 1023);

	if (state->bgon)
		tilemap_draw(bitmap, cliprect, state->screen_layout ? state->bg_tilemap8x4 : state->bg_tilemap4x8, TILEMAP_DRAW_LAYER1, 0);

	if (state->objon)
		draw_sprites(screen->machine, bitmap, cliprect);

	if (state->bgon)
		tilemap_draw(bitmap, cliprect, state->screen_layout ? state->bg_tilemap8x4 : state->bg_tilemap4x8, TILEMAP_DRAW_LAYER0, 0);

	if (state->chon)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( blktiger )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
