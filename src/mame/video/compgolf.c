/****************************************************************************************

 Competition Golf Final Round
 video hardware emulation

****************************************************************************************/

#include "emu.h"
#include "includes/compgolf.h"


PALETTE_INIT( compgolf )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0,bit1,bit2,r,g,b;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( compgolf_video_w )
{
	compgolf_state *state = (compgolf_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->text_tilemap, offset / 2);
}

WRITE8_HANDLER( compgolf_back_w )
{
	compgolf_state *state = (compgolf_state *)space->machine->driver_data;
	state->bg_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

static TILE_GET_INFO( get_text_info )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;
	tile_index <<= 1;
	SET_TILE_INFO(2, state->videoram[tile_index + 1] | (state->videoram[tile_index] << 8), state->videoram[tile_index] >> 2, 0);
}

static TILEMAP_MAPPER( back_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

static TILE_GET_INFO( get_back_info )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;
	int attr = state->bg_ram[tile_index * 2];
	int code = state->bg_ram[tile_index * 2 + 1] + ((attr & 1) << 8);
	int color = (attr & 0x3e) >> 1;

	SET_TILE_INFO(1, code, color, 0);
}

VIDEO_START( compgolf )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_back_info, back_scan, 16, 16, 32, 32);
	state->text_tilemap = tilemap_create(machine, get_text_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->text_tilemap, 0);
}

/*
preliminary sprite list:
       0        1        2        3
xx------ xxxxxxxx -------- -------- sprite code
---x---- -------- -------- -------- Double Height
----x--- -------- -------- -------- Color,all of it?
-------- -------- xxxxxxxx -------- Y pos
-------- -------- -------- xxxxxxxx X pos
-----x-- -------- -------- -------- Flip X
-------- -------- -------- -------- Flip Y(used?)
*/
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;
	int offs, fx, fy, x, y, color, sprite;

	for (offs = 0; offs < 0x60; offs += 4)
	{
		sprite = state->spriteram[offs + 1] + (((state->spriteram[offs] & 0xc0) >> 6) * 0x100);
		x = 240 - state->spriteram[offs + 3];
		y = state->spriteram[offs + 2];
		color = (state->spriteram[offs] & 8)>>3;
		fx = state->spriteram[offs] & 4;
		fy = 0; /* ? */

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				sprite,
				color,fx,fy,x,y,0);

		/* Double Height */
		if(state->spriteram[offs] & 0x10)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				sprite + 1,
				color, fx, fy, x, y + 16, 0);
		}
	}
}

VIDEO_UPDATE( compgolf )
{
	compgolf_state *state = (compgolf_state *)screen->machine->driver_data;
	int scrollx = state->scrollx_hi + state->scrollx_lo;
	int scrolly = state->scrolly_hi + state->scrolly_lo;

	tilemap_set_scrollx(state->bg_tilemap, 0, scrollx);
	tilemap_set_scrolly(state->bg_tilemap, 0, scrolly);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->text_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
