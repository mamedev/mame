/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/crbaloon.h"

static UINT8 spritectrl[3];

INT8 crbaloon_collision;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Balloon has no PROMs, the color code directly maps to a color:
  all bits are inverted
  bit 3 HALF (intensity)
  bit 2 BLUE
  bit 1 GREEN
  bit 0 RED

***************************************************************************/
PALETTE_INIT( crbaloon )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int intensity,r,g,b;


		intensity = (~i & 0x08) ? 0xff : 0x55;

		/* red component */
		r = intensity * ((~i >> 0) & 1);
		/* green component */
		g = intensity * ((~i >> 1) & 1);
		/* blue component */
		b = intensity * ((~i >> 2) & 1);
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	for (i = 0;i < TOTAL_COLORS(0);i += 2)
	{
		COLOR(0,i) = 15;		/* black background */
		COLOR(0,i + 1) = i / 2;	/* colored foreground */
	}
}

WRITE8_HANDLER( crbaloon_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( crbaloon_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( crbaloon_spritectrl_w )
{
	spritectrl[offset] = data;
}

WRITE8_HANDLER( crbaloon_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( crbaloon )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows_flip_xy,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tmpbitmap = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);

	state_save_register_global_array(spritectrl);
	state_save_register_global(crbaloon_collision);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int x,y;

	/* Check Collision - Draw balloon in background colour, if no */
    /* collision occured, bitmap will be same as tmpbitmap        */

	int bx = spritectrl[1];
	int by = spritectrl[2] - 32;

	tilemap_draw(tmpbitmap, 0, bg_tilemap, 0, 0);

	if (flip_screen)
	{
		by += 32;
	}

	drawgfx(bitmap,machine->gfx[1],
			spritectrl[0] & 0x0f,
			15,
			0,0,
			bx,by,
			cliprect,TRANSPARENCY_PEN,0);

    crbaloon_collision = 0;

	for (x = bx; x < bx + machine->gfx[1]->width; x++)
	{
		for (y = by; y < by + machine->gfx[1]->height; y++)
        {
			if ((x < machine->screen[0].visarea.min_x) ||
			    (x > machine->screen[0].visarea.max_x) ||
			    (y < machine->screen[0].visarea.min_y) ||
			    (y > machine->screen[0].visarea.max_y))
			{
				continue;
			}

        	if (*BITMAP_ADDR16(bitmap, y, x) != *BITMAP_ADDR16(tmpbitmap, y, x))
        	{
				crbaloon_collision = -1;
				break;
			}
        }
	}


	/* actually draw the balloon */

	drawgfx(bitmap,machine->gfx[1],
			spritectrl[0] & 0x0f,
			(spritectrl[0] & 0xf0) >> 4,
			0,0,
			bx,by,
			cliprect,TRANSPARENCY_PEN,0);
}

VIDEO_UPDATE( crbaloon )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
