/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *vastar_bg1videoram,*vastar_bg2videoram,*vastar_fgvideoram;
UINT8 *vastar_bg1_scroll,*vastar_bg2_scroll;
UINT8 *vastar_sprite_priority;

static tilemap *fg_tilemap, *bg1_tilemap, *bg2_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, color;

	code = vastar_fgvideoram[tile_index + 0x800] | (vastar_fgvideoram[tile_index + 0x400] << 8);
	color = vastar_fgvideoram[tile_index];
	SET_TILE_INFO(
			0,
			code,
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	int code, color;

	code = vastar_bg1videoram[tile_index + 0x800] | (vastar_bg1videoram[tile_index] << 8);
	color = vastar_bg1videoram[tile_index + 0xc00];
	SET_TILE_INFO(
			4,
			code,
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	int code, color;

	code = vastar_bg2videoram[tile_index + 0x800] | (vastar_bg2videoram[tile_index] << 8);
	color = vastar_bg2videoram[tile_index + 0xc00];
	SET_TILE_INFO(
			3,
			code,
			color & 0x3f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( vastar )
{
	fg_tilemap  = tilemap_create(get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	bg1_tilemap = tilemap_create(get_bg1_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	bg2_tilemap = tilemap_create(get_bg2_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(bg1_tilemap,0);
	tilemap_set_transparent_pen(bg2_tilemap,0);

	tilemap_set_scroll_cols(bg1_tilemap, 32);
	tilemap_set_scroll_cols(bg2_tilemap, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( vastar_fgvideoram_w )
{
	vastar_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( vastar_bg1videoram_w )
{
	vastar_bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(bg1_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( vastar_bg2videoram_w )
{
	vastar_bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(bg2_tilemap,offset & 0x3ff);
}


READ8_HANDLER( vastar_bg1videoram_r )
{
	return vastar_bg1videoram[offset];
}

READ8_HANDLER( vastar_bg2videoram_r )
{
	return vastar_bg2videoram[offset];
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;


	for (offs = 0; offs < spriteram_size; offs += 2)
	{
		int code, sx, sy, color, flipx, flipy;


		code = ((spriteram_3[offs] & 0xfc) >> 2) + ((spriteram_2[offs] & 0x01) << 6)
				+ ((offs & 0x20) << 2);

		sx = spriteram_3[offs + 1];
		sy = spriteram[offs];
		color = spriteram[offs + 1] & 0x3f;
		flipx = spriteram_3[offs] & 0x02;
		flipy = spriteram_3[offs] & 0x01;

		if (flip_screen)
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		if (spriteram_2[offs] & 0x08)	/* double width */
		{
			if (!flip_screen)
				sy = 224 - sy;

			drawgfx(bitmap,machine->gfx[2],
					code/2,
					color,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
			/* redraw with wraparound */
			drawgfx(bitmap,machine->gfx[2],
					code/2,
					color,
					flipx,flipy,
					sx,sy+256,
					cliprect,TRANSPARENCY_PEN,0);
		}
		else
		{
			if (!flip_screen)
				sy = 240 - sy;

			drawgfx(bitmap,machine->gfx[1],
					code,
					color,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( vastar )
{
	int i;


	for (i = 0;i < 32;i++)
	{
		tilemap_set_scrolly(bg1_tilemap,i,vastar_bg1_scroll[i]);
		tilemap_set_scrolly(bg2_tilemap,i,vastar_bg2_scroll[i]);
	}

	switch (*vastar_sprite_priority)
	{
	case 0:
		tilemap_draw(bitmap,cliprect, bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect, bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, fg_tilemap, 0,0);
		break;

	case 2:
		tilemap_draw(bitmap,cliprect, bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect, bg1_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, fg_tilemap, 0,0);
		break;

	case 3:
		tilemap_draw(bitmap,cliprect, bg1_tilemap, TILEMAP_DRAW_OPAQUE,0);
		tilemap_draw(bitmap,cliprect, bg2_tilemap, 0,0);
		tilemap_draw(bitmap,cliprect, fg_tilemap, 0,0);
		draw_sprites(machine, bitmap,cliprect);
		break;

	default:
		logerror("Unimplemented priority %X\n", *vastar_sprite_priority);
		break;
	}
	return 0;
}
