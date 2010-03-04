#include "emu.h"
#include "includes/othldrby.h"


#define VIDEORAM_SIZE      0x1c00
#define SPRITERAM_START    0x1800
#define SPRITERAM_SIZE     (VIDEORAM_SIZE - SPRITERAM_START)


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int plane )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;
	UINT16 attr;

	tile_index = 2 * tile_index + 0x800 * plane;
	attr = state->vram[tile_index];
	SET_TILE_INFO(
			1,
			state->vram[tile_index + 1],
			attr & 0x7f,
			0);
	tileinfo->category = (attr & 0x0600) >> 9;
}

static TILE_GET_INFO( get_tile_info0 )
{
	get_tile_info(machine, tileinfo, tile_index, 0);
}

static TILE_GET_INFO( get_tile_info1 )
{
	get_tile_info(machine, tileinfo, tile_index, 1);
}

static TILE_GET_INFO( get_tile_info2 )
{
	get_tile_info(machine, tileinfo, tile_index, 2);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( othldrby )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 16, 16, 32, 32);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 16, 16, 32, 32);
	state->bg_tilemap[2] = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 16, 16, 32, 32);

	state->vram = auto_alloc_array(machine, UINT16, VIDEORAM_SIZE);
	state->buf_spriteram = auto_alloc_array(machine, UINT16, 2 * SPRITERAM_SIZE);
	state->buf_spriteram2 = state->buf_spriteram + SPRITERAM_SIZE;

	tilemap_set_transparent_pen(state->bg_tilemap[0], 0);
	tilemap_set_transparent_pen(state->bg_tilemap[1], 0);
	tilemap_set_transparent_pen(state->bg_tilemap[2], 0);

	state_save_register_global_pointer(machine, state->vram, VIDEORAM_SIZE);
	state_save_register_global_pointer(machine, state->buf_spriteram, 2 * SPRITERAM_SIZE);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( othldrby_videoram_addr_w )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;
	state->vram_addr = data;
}

READ16_HANDLER( othldrby_videoram_r )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;

	if (state->vram_addr < VIDEORAM_SIZE)
		return state->vram[state->vram_addr++];
	else
	{
		popmessage("GFXRAM OUT OF BOUNDS %04x", state->vram_addr);
		return 0;
	}
}

WRITE16_HANDLER( othldrby_videoram_w )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;

	if (state->vram_addr < VIDEORAM_SIZE)
	{
		if (state->vram_addr < SPRITERAM_START)
			tilemap_mark_tile_dirty(state->bg_tilemap[state->vram_addr / 0x800], (state->vram_addr & 0x7ff) / 2);
		state->vram[state->vram_addr++] = data;
	}
	else
		popmessage("GFXRAM OUT OF BOUNDS %04x", state->vram_addr);
}

WRITE16_HANDLER( othldrby_vreg_addr_w )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;
	state->vreg_addr = data & 0x7f;	/* bit 7 is set when screen is flipped */
}

WRITE16_HANDLER( othldrby_vreg_w )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;

	if (state->vreg_addr < OTHLDRBY_VREG_SIZE)
		state->vreg[state->vreg_addr++] = data;
	else
		popmessage("%06x: VREG OUT OF BOUNDS %04x", cpu_get_pc(space->cpu), state->vreg_addr);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < SPRITERAM_SIZE; offs += 4)
	{
		int x, y, color, code, sx, sy, flipx, flipy, sizex, sizey, pri;

		pri = (state->buf_spriteram[offs] & 0x0600) >> 9;
		if (pri != priority)
			continue;

		flipx = state->buf_spriteram[offs] & 0x1000;
		flipy = 0;
		color = (state->buf_spriteram[offs] & 0x01fc) >> 2;
		code = state->buf_spriteram[offs + 1] | ((state->buf_spriteram[offs] & 0x0003) << 16);
		sx = (state->buf_spriteram[offs + 2] >> 7);
		sy = (state->buf_spriteram[offs + 3] >> 7);
		sizex = (state->buf_spriteram[offs + 2] & 0x000f) + 1;
		sizey = (state->buf_spriteram[offs + 3] & 0x000f) + 1;

		if (flip_screen_get(machine))
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 246 - sx;
			sy = 16 - sy;
		}

		for (y = 0; y < sizey; y++)
		{
			for (x = 0; x < sizex; x++)
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
						code + x + sizex * y,
						color,
						flipx,flipy,
						(sx + (flipx ? (-8*(x+1)+1) : 8*x) - state->vreg[6]+44) & 0x1ff,(sy + (flipy ? (-8*(y+1)+1) : 8*y) - state->vreg[7]-9) & 0x1ff,0);
			}
		}
	}
}

VIDEO_UPDATE( othldrby )
{
	othldrby_state *state = (othldrby_state *)screen->machine->driver_data;
	int layer;

	flip_screen_set(screen->machine, state->vreg[0x0f] & 0x80);

	for (layer = 0; layer < 3; layer++)
	{
		if (flip_screen_get(screen->machine))
		{
			tilemap_set_scrollx(state->bg_tilemap[layer], 0, state->vreg[2 * layer] + 59);
			tilemap_set_scrolly(state->bg_tilemap[layer], 0, state->vreg[2 * layer + 1] + 248);
		}
		else
		{
			tilemap_set_scrollx(state->bg_tilemap[layer], 0, state->vreg[2 * layer] - 58);
			tilemap_set_scrolly(state->bg_tilemap[layer], 0, state->vreg[2 * layer+1] + 9);
		}
	}

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	bitmap_fill(bitmap, cliprect, 0);

	for (layer = 0; layer < 3; layer++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[layer], 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 0);

	for (layer = 0; layer < 3; layer++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[layer], 1, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 1);

	for (layer = 0; layer < 3; layer++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[layer], 2, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 2);

	for (layer = 0; layer < 3; layer++)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[layer], 3, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 3);

	return 0;
}

VIDEO_EOF( othldrby )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;

	/* sprites need to be delayed two frames */
	memcpy(state->buf_spriteram, state->buf_spriteram2, SPRITERAM_SIZE * sizeof(state->buf_spriteram[0]));
	memcpy(state->buf_spriteram2, &state->vram[SPRITERAM_START], SPRITERAM_SIZE * sizeof(state->buf_spriteram[0]));
}
