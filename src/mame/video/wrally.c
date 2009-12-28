/***************************************************************************

  World Rally Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "driver.h"
#include "includes/wrally.h"

UINT16 *wrally_spriteram;
UINT16 *wrally_vregs;
UINT16 *wrally_videoram;

tilemap_t *wrally_pant[2];

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (64*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | --xxxxxx xxxxxxxx | code
      0  | xx------ -------- | not used?
      1  | -------- ---xxxxx | color
      1  | -------- --x----- | priority
      1  | -------- -x------ | flip y
      1  | -------- x------- | flip x
      1  | ---xxxxx -------- | data used to handle collisions, speed, etc
      1  | xxx----- -------- | not used?
*/

static TILE_GET_INFO( get_tile_info_wrally_screen0 )
{
	int data = wrally_videoram[tile_index << 1];
	int data2 = wrally_videoram[(tile_index << 1) + 1];
	int code = data & 0x3fff;

	tileinfo->category = (data2 >> 5) & 0x01;

	SET_TILE_INFO(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

static TILE_GET_INFO( get_tile_info_wrally_screen1 )
{
	int data = wrally_videoram[(0x2000/2) + (tile_index << 1)];
	int data2 = wrally_videoram[(0x2000/2) + (tile_index << 1) + 1];
	int code = data & 0x3fff;

	tileinfo->category = (data2 >> 5) & 0x01;

	SET_TILE_INFO(0, code, data2 & 0x1f, TILE_FLIPYX((data2 >> 6) & 0x03));
}

/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( wrally )
{
	wrally_pant[0] = tilemap_create(machine, get_tile_info_wrally_screen0,tilemap_scan_rows,16,16,64,32);
	wrally_pant[1] = tilemap_create(machine, get_tile_info_wrally_screen1,tilemap_scan_rows,16,16,64,32);

	tilemap_set_transmask(wrally_pant[0],0,0xff01,0x00ff); /* this layer is split in two (pens 1..7, pens 8-15) */
	tilemap_set_transparent_pen(wrally_pant[1],0);
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
      1  | xxxxxxxx xxxxxxxx | unknown
      2  | ------xx xxxxxxxx | x position
      2  | --xxxx-- -------- | sprite color (low 4 bits)
      2  | -x------ -------- | shadows/highlights (see below)
      2  | x------- -------- | not used?
      3  | --xxxxxx xxxxxxxx | sprite code
      3  | xx------ -------- | not used?

    For shadows/highlights, the tile color below the sprite will be set using a
    palette (from the 8 available) based on the gfx pen of the sprite. Only pens
    in the range 0x8-0xf are used.
*/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int i, px, py;
	const gfx_element *gfx = machine->gfx[0];

	for (i = 6/2; i < (0x1000 - 6)/2; i += 4) {
		int sx = wrally_spriteram[i+2] & 0x03ff;
		int sy = (240 - (wrally_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = wrally_spriteram[i+3] & 0x3fff;
		int color = (wrally_spriteram[i+2] & 0x7c00) >> 10;
		int attr = (wrally_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int color_effect = (color & 0x10) >> 4;
		int high_priority = number >= 0x3700;
		color = color & 0x0f;

		if (high_priority != priority) continue;

		if (flip_screen_get(machine)) {
			sy = sy + 248;
		}

		if (!color_effect) {
			drawgfx_transpen(bitmap,cliprect,gfx,number,
					0x20 + color,xflip,yflip,
					sx - 0x0f,sy,0);
		} else {
			/* get a pointer to the current sprite's gfx data */
			const UINT8 *gfx_src = gfx_element_get_data(gfx, number % gfx->total_elements);

			for (py = 0; py < gfx->height; py++){
				/* get a pointer to the current line in the screen bitmap */
				int ypos = ((sy + py) & 0x1ff);
				UINT16 *srcy = BITMAP_ADDR16(bitmap, ypos, 0);

				int gfx_py = yflip ? (gfx->height - 1 - py) : py;

				if ((ypos < cliprect->min_y) || (ypos > cliprect->max_y)) continue;

				for (px = 0; px < gfx->width; px++){
					/* get current pixel */
					int xpos = (((sx + px) & 0x3ff) - 0x0f) & 0x3ff;
					UINT16 *pixel = srcy + xpos;
					int src_color = *pixel;

					int gfx_px = xflip ? (gfx->width - 1 - px) : px;

					/* get asociated pen for the current sprite pixel */
					int gfx_pen = gfx_src[gfx->line_modulo*gfx_py + gfx_px];

					/* pens 8..15 are used to select a palette */
					if ((gfx_pen < 8) || (gfx_pen >= 16)) continue;

					if ((xpos < cliprect->min_x) || (xpos > cliprect->max_x)) continue;

					/* modify the color of the tile */
					*pixel = src_color + (gfx_pen-8)*1024;
				}
			}
		}
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( wrally )
{
	/* set scroll registers */
	if (!flip_screen_get(screen->machine)) {
		tilemap_set_scrolly(wrally_pant[0], 0, wrally_vregs[0]);
		tilemap_set_scrollx(wrally_pant[0], 0, wrally_vregs[1]+4);
		tilemap_set_scrolly(wrally_pant[1], 0, wrally_vregs[2]);
		tilemap_set_scrollx(wrally_pant[1], 0, wrally_vregs[3]);
	} else {
		tilemap_set_scrolly(wrally_pant[0], 0, 248 - wrally_vregs[0]);
		tilemap_set_scrollx(wrally_pant[0], 0, 1024 - wrally_vregs[1] - 4);
		tilemap_set_scrolly(wrally_pant[1], 0, 248 - wrally_vregs[2]);
		tilemap_set_scrollx(wrally_pant[1], 0, 1024 - wrally_vregs[3]);
	}

	/* draw tilemaps + sprites */
	tilemap_draw(bitmap,cliprect,wrally_pant[1],TILEMAP_DRAW_OPAQUE,0);
	tilemap_draw(bitmap,cliprect,wrally_pant[0],TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER0,0);
	tilemap_draw(bitmap,cliprect,wrally_pant[0],TILEMAP_DRAW_CATEGORY(0) | TILEMAP_DRAW_LAYER1,0);

	tilemap_draw(bitmap,cliprect,wrally_pant[1],TILEMAP_DRAW_CATEGORY(1),0);
	tilemap_draw(bitmap,cliprect,wrally_pant[0],TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER0,0);

	draw_sprites(screen->machine,bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,wrally_pant[0],TILEMAP_DRAW_CATEGORY(1) | TILEMAP_DRAW_LAYER1,0);

	draw_sprites(screen->machine,bitmap,cliprect,1);

	return 0;
}
