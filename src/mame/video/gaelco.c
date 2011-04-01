/***************************************************************************

  Gaelco Type 1 Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/gaelco.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- -------x | flip x
      0  | -------- ------x- | flip y
      0  | xxxxxxxx xxxxxx-- | code
      1  | -------- --xxxxxx | color
      1  | -------- xx------ | priority
      1  | xxxxxxxx -------- | not used
*/

static TILE_GET_INFO( get_tile_info_gaelco_screen0 )
{
	gaelco_state *state = machine.driver_data<gaelco_state>();
	int data = state->m_videoram[tile_index << 1];
	int data2 = state->m_videoram[(tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}


static TILE_GET_INFO( get_tile_info_gaelco_screen1 )
{
	gaelco_state *state = machine.driver_data<gaelco_state>();
	int data = state->m_videoram[(0x1000 / 2) + (tile_index << 1)];
	int data2 = state->m_videoram[(0x1000 / 2) + (tile_index << 1) + 1];
	int code = ((data & 0xfffc) >> 2);

	tileinfo->category = (data2 >> 6) & 0x03;

	SET_TILE_INFO(1, 0x4000 + code, data2 & 0x3f, TILE_FLIPYX(data & 0x03));
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_HANDLER( gaelco_vram_w )
{
	gaelco_state *state = space->machine().driver_data<gaelco_state>();
	COMBINE_DATA(&state->m_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_tilemap[offset >> 11], ((offset << 1) & 0x0fff) >> 2);
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( bigkarnk )
{
	gaelco_state *state = machine.driver_data<gaelco_state>();
	state->m_tilemap[0] = tilemap_create(machine, get_tile_info_gaelco_screen0, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tilemap[1] = tilemap_create(machine, get_tile_info_gaelco_screen1, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transmask(state->m_tilemap[0], 0, 0xff01, 0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
	tilemap_set_transmask(state->m_tilemap[1], 0, 0xff01, 0x00ff); /* pens 1-7 opaque, pens 0, 8-15 transparent */
}

VIDEO_START( maniacsq )
{
	gaelco_state *state = machine.driver_data<gaelco_state>();
	state->m_tilemap[0] = tilemap_create(machine, get_tile_info_gaelco_screen0, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_tilemap[1] = tilemap_create(machine, get_tile_info_gaelco_screen1, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->m_tilemap[0], 0);
	tilemap_set_transparent_pen(state->m_tilemap[1], 0);
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
      0  | -----xxx -------- | not used
      0  | ----x--- -------- | sprite size
      0  | --xx---- -------- | sprite priority
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | -xxxxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (8x8 cuadrant)
      3  | xxxxxxxx xxxxxx-- | sprite code
*/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gaelco_state *state = machine.driver_data<gaelco_state>();
	int i, x, y, ex, ey;
	const gfx_element *gfx = machine.gfx[0];

	static const int x_offset[2] = {0x0,0x2};
	static const int y_offset[2] = {0x0,0x1};

	for (i = 0x800 - 4 - 1; i >= 3; i -= 4)
	{
		int sx = state->m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (state->m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = state->m_spriteram[i + 3];
		int color = (state->m_spriteram[i + 2] & 0x7e00) >> 9;
		int attr = (state->m_spriteram[i] & 0xfe00) >> 9;
		int priority = (state->m_spriteram[i] & 0x3000) >> 12;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size, pri_mask;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38)
			priority = 4;

		switch (priority)
		{
			case 0: pri_mask = 0xff00; break;
			case 1: pri_mask = 0xff00 | 0xf0f0; break;
			case 2: pri_mask = 0xff00 | 0xf0f0 | 0xcccc; break;
			case 3: pri_mask = 0xff00 | 0xf0f0 | 0xcccc | 0xaaaa; break;
			default:
			case 4: pri_mask = 0; break;
		}

		if (attr & 0x04)
			spr_size = 1;
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++)
		{
			for (x = 0; x < spr_size; x++)
			{
				ex = xflip ? (spr_size - 1 - x) : x;
				ey = yflip ? (spr_size - 1 - y) : y;

				pdrawgfx_transpen(bitmap,cliprect,gfx,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x0f+x*8,sy+y*8,
						machine.priority_bitmap,pri_mask,0);
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

SCREEN_UPDATE( maniacsq )
{
	gaelco_state *state = screen->machine().driver_data<gaelco_state>();

	/* set scroll registers */
	tilemap_set_scrolly(state->m_tilemap[0], 0, state->m_vregs[0]);
	tilemap_set_scrollx(state->m_tilemap[0], 0, state->m_vregs[1] + 4);
	tilemap_set_scrolly(state->m_tilemap[1], 0, state->m_vregs[2]);
	tilemap_set_scrollx(state->m_tilemap[1], 0, state->m_vregs[3]);

	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], 3, 0);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], 3, 0);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], 2, 1);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], 2, 1);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], 1, 2);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], 1, 2);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], 0, 4);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], 0, 4);

	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( bigkarnk )
{
	gaelco_state *state = screen->machine().driver_data<gaelco_state>();

	/* set scroll registers */
	tilemap_set_scrolly(state->m_tilemap[0], 0, state->m_vregs[0]);
	tilemap_set_scrollx(state->m_tilemap[0], 0, state->m_vregs[1] + 4);
	tilemap_set_scrolly(state->m_tilemap[1], 0, state->m_vregs[2]);
	tilemap_set_scrollx(state->m_tilemap[1], 0, state->m_vregs[3]);

	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER1 | 3, 0);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER1 | 3, 0);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER0 | 3, 1);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER0 | 3, 1);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER1 | 2, 1);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER1 | 2, 1);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER0 | 2, 2);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER0 | 2, 2);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER1 | 1, 2);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER1 | 1, 2);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER0 | 1, 4);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER0 | 1, 4);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER1 | 0, 4);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER1 | 0, 4);

	tilemap_draw(bitmap, cliprect, state->m_tilemap[1], TILEMAP_DRAW_LAYER0 | 0, 8);
	tilemap_draw(bitmap, cliprect, state->m_tilemap[0], TILEMAP_DRAW_LAYER0 | 0, 8);

	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
