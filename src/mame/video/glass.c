/***************************************************************************

  Glass Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/glass.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | code
      1  | -------- ---xxxxx | color (uses colors 0x200-0x3ff)
      1  | -------- --x----- | not used?
      1  | -------- -x------ | flip x
      1  | -------- x------- | flip y
      1  | xxxxxxxx -------- | not used
*/

static TILE_GET_INFO( get_tile_info_glass_screen0 )
{
	glass_state *state = machine->driver_data<glass_state>();
	int data = state->videoram[tile_index << 1];
	int data2 = state->videoram[(tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}


static TILE_GET_INFO( get_tile_info_glass_screen1 )
{
	glass_state *state = machine->driver_data<glass_state>();
	int data = state->videoram[(0x1000 / 2) + (tile_index << 1)];
	int data2 = state->videoram[(0x1000 / 2) + (tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}

/***************************************************************************

    Blitter

***************************************************************************/

/*
    The blitter is accessed writting 5 consecutive bits. The stream is: P0 P1 B2 B1 B0

    if P0 is set, the hardware selects the first half of ROM H9 (girls)
    if P1 is set, the hardware selects the second half of ROM H9 (boys)

    B2B1B0 selects the picture (there are 8 pictures in each half of the ROM)
*/

WRITE16_HANDLER( glass_blitter_w )
{
	glass_state *state = space->machine->driver_data<glass_state>();
	state->blitter_serial_buffer[state->current_bit] = data & 0x01;
	state->current_bit++;

	if (state->current_bit == 5)
	{
		state->current_command = (state->blitter_serial_buffer[0] << 4) |
							(state->blitter_serial_buffer[1] << 3) |
							(state->blitter_serial_buffer[2] << 2) |
							(state->blitter_serial_buffer[3] << 1) |
							(state->blitter_serial_buffer[4] << 0);
		state->current_bit = 0;

		/* fill the screen bitmap with the current picture */
		{
			int i, j;
			UINT8 *gfx = (UINT8 *)space->machine->region("gfx3")->base();

			gfx = gfx + (state->current_command & 0x07) * 0x10000 + (state->current_command & 0x08) * 0x10000 + 0x140;

			if ((state->current_command & 0x18) != 0)
			{
				for (j = 0; j < 200; j++)
				{
					for (i = 0; i < 320; i++)
					{
						int color = *gfx;
						gfx++;
						*BITMAP_ADDR16(state->screen_bitmap, j, i) = color & 0xff;
					}
				}
			}
			else
				bitmap_fill(state->screen_bitmap, 0, 0);
		}
	}
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( glass_vram_w )
{
	glass_state *state = space->machine->driver_data<glass_state>();
	COMBINE_DATA(&state->videoram[offset]);
	tilemap_mark_tile_dirty(state->pant[offset >> 11], ((offset << 1) & 0x0fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( glass )
{
	glass_state *state = machine->driver_data<glass_state>();
	state->pant[0] = tilemap_create(machine, get_tile_info_glass_screen0, tilemap_scan_rows, 16, 16, 32, 32);
	state->pant[1] = tilemap_create(machine, get_tile_info_glass_screen1, tilemap_scan_rows, 16, 16, 32, 32);
	state->screen_bitmap = auto_bitmap_alloc (machine, 320, 200, machine->primary_screen->format());

	state_save_register_global_bitmap(machine, state->screen_bitmap);

	tilemap_set_transparent_pen(state->pant[0], 0);
	tilemap_set_transparent_pen(state->pant[1], 0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color (uses colors 0x100-0x1ff)
      2  | xx------ -------- | not used?
      3  | xxxxxxxx xxxxxxxx | sprite code
*/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	glass_state *state = machine->driver_data<glass_state>();
	int i;
	const gfx_element *gfx = machine->gfx[0];

	for (i = 3; i < (0x1000 - 6) / 2; i += 4)
	{
		int sx = state->spriteram[i + 2] & 0x01ff;
		int sy = (240 - (state->spriteram[i] & 0x00ff)) & 0x00ff;
		int number = state->spriteram[i + 3];
		int color = (state->spriteram[i + 2] & 0x1e00) >> 9;
		int attr = (state->spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;

		number = ((number & 0x03) << 14) | ((number & 0x0fffc) >> 2);

		drawgfx_transpen(bitmap,cliprect,gfx,number,
				0x10 + (color & 0x0f),xflip,yflip,
				sx-0x0f,sy,0);
	}
}

/***************************************************************************

    Display Refresh

****************************************************************************/

VIDEO_UPDATE( glass )
{
	glass_state *state = screen->machine->driver_data<glass_state>();
	/* set scroll registers */
	tilemap_set_scrolly(state->pant[0], 0, state->vregs[0]);
	tilemap_set_scrollx(state->pant[0], 0, state->vregs[1] + 0x04);
	tilemap_set_scrolly(state->pant[1], 0, state->vregs[2]);
	tilemap_set_scrollx(state->pant[1], 0, state->vregs[3]);

	/* draw layers + sprites */
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	copybitmap(bitmap, state->screen_bitmap, 0, 0, 0x18, 0x24, cliprect);
	tilemap_draw(bitmap, cliprect, state->pant[1], 0, 0);
	tilemap_draw(bitmap, cliprect, state->pant[0], 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
