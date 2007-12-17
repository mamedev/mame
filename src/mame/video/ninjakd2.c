#include "driver.h"

UINT8 *ninjakd2_bg_videoram, *ninjakd2_fg_videoram;

static UINT8  ninjakd2_bg_enable = 1, sprite_overdraw_enabled = 0;
static UINT16 ninjakd2_scrollx, ninjakd2_scrolly;
static tilemap *fg_tilemap, *bg_tilemap;
static mame_bitmap *bitmap_sp;	/* for sprite overdraw */

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = ((ninjakd2_bg_videoram[tile_index*2 + 1] & 0xc0) << 2) | ninjakd2_bg_videoram[tile_index*2];
	int color = ninjakd2_bg_videoram[tile_index*2 + 1] & 0xf;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX((ninjakd2_bg_videoram[tile_index*2 + 1] & 0x30) >> 4));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = ((ninjakd2_fg_videoram[tile_index*2 + 1] & 0xc0) << 2) | ninjakd2_fg_videoram[tile_index*2];
	int color = ninjakd2_fg_videoram[tile_index*2 + 1] & 0xf;
	SET_TILE_INFO(2, code, color, TILE_FLIPYX((ninjakd2_fg_videoram[tile_index*2 + 1] & 0x30) >> 4));
}

VIDEO_START( ninjakd2 )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      16, 16, 32, 32);
	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,  8,  8,  32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 15);

	bitmap_sp = auto_bitmap_alloc (machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
}

WRITE8_HANDLER( ninjakd2_bgvideoram_w )
{
	ninjakd2_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
}

WRITE8_HANDLER( ninjakd2_fgvideoram_w )
{
	ninjakd2_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset >> 1);
}

WRITE8_HANDLER( ninjakd2_scrollx_w )
{
	if(offset)
		ninjakd2_scrollx = ((ninjakd2_scrollx & 0x0ff) | data*256) & 0x1ff;
	else
		ninjakd2_scrollx = ((ninjakd2_scrollx & 0x100) | data) & 0x1ff;
}

WRITE8_HANDLER( ninjakd2_scrolly_w )
{
	if(offset)
		ninjakd2_scrolly = ((ninjakd2_scrolly & 0x0ff) | data*256) & 0x1ff;
	else
		ninjakd2_scrolly = ((ninjakd2_scrolly & 0x100) | data) & 0x1ff;
}

WRITE8_HANDLER( ninjakd2_background_enable_w )
{
	ninjakd2_bg_enable = data & 1;
}

WRITE8_HANDLER( ninjakd2_sprite_overdraw_w )
{
	sprite_overdraw_enabled = data & 1;

	if(sprite_overdraw_enabled)
		fillbitmap(bitmap_sp, 15, &Machine->screen[0].visarea);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */

	for (offs = 11 ;offs < spriteram_size; offs+=16)
	{
		int sx,sy,tile,color,flipx,flipy;

		if (spriteram[offs+2] & 2)
		{
			sx = spriteram[offs+1];
			sy = spriteram[offs];
			if (spriteram[offs+2] & 1) sx-=256;
			tile = spriteram[offs+3]+((spriteram[offs+2] & 0xc0)<<2);
			flipx = spriteram[offs+2] & 0x10;
			flipy = spriteram[offs+2] & 0x20;
			color = spriteram[offs+4] & 0x0f;

			if(sprite_overdraw_enabled && (color >= 0x0c && color <= 0x0e) )
			{
				/* "static" sprites */
				drawgfx(bitmap_sp,machine->gfx[1],
						tile,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,
						TRANSPARENCY_PEN, 15);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[1],
						tile,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,
						TRANSPARENCY_PEN, 15);

				/* "normal" sprites with color = 0x0f clear the "static" ones */
				if(sprite_overdraw_enabled && color == 0x0f)
				{
					int x,y,offset = 0;
					const gfx_element *gfx = machine->gfx[1];
					UINT8 *srcgfx = gfx->gfxdata + tile * gfx->char_modulo;

					for(y = 0; y < gfx->height; y++)
					{
						for(x = 0; x < gfx->width; x++)
						{
							if(srcgfx[offset] != 15)
							{
								*BITMAP_ADDR16(bitmap_sp, sy + y, sx + x) = 15;
							}

							offset++;
						}
					}
				}
			}
		}
	}

	if(sprite_overdraw_enabled)
		copybitmap(bitmap, bitmap_sp, 0, 0, 0, 0, cliprect, TRANSPARENCY_PEN, 15);
}

VIDEO_UPDATE( ninjakd2 )
{
	fillbitmap(bitmap, machine->pens[0],0);

	tilemap_set_scrollx(bg_tilemap, 0, ninjakd2_scrollx);
	tilemap_set_scrolly(bg_tilemap, 0, ninjakd2_scrolly);

	if (ninjakd2_bg_enable)
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}
