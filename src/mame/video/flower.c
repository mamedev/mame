/* Flower Video Hardware */

#include "driver.h"

static tilemap *flower_bg0_tilemap, *flower_bg1_tilemap, *flower_text_tilemap, *flower_text_right_tilemap;
UINT8 *flower_textram, *flower_bg0ram, *flower_bg1ram, *flower_bg0_scroll, *flower_bg1_scroll;


PALETTE_INIT( flower )
{
	int i;

	for (i=0; i<256; i++)
	{
		palette_set_color_rgb(machine, i, pal4bit(color_prom[i]), pal4bit(color_prom[i+0x100]), pal4bit(color_prom[i+0x200]));
		colortable[i] = i;
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	const gfx_element *gfx = machine->gfx[1];
	UINT8 *source = spriteram + 0x200;
	UINT8 *finish = source - 0x200;

	source -= 8;

	while( source>=finish )
	{
		int xblock,yblock;
		int sy = 256-32-source[0]+1;
		int	sx = (source[4]|(source[5]<<8))-55;
		int code = source[1] & 0x3f;
		int color = (source[6]>>4);

		/*
            Byte 0: Y
            Byte 1:
                0x80 - FlipY
                0x40 - FlipX
                0x3f - Tile
            Byte 2:
                0x08 - Tile MSB
                0x01 - Tile MSB
            Byte 3:
                0x07 - X Zoom
                0x08 - X Size
                0x70 - Y Zoom
                0x80 - Y Size
            Byte 4: X LSB
            Byte 5: X MSB
            Byte 6:
                0xf0 - Colour
        */

		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		int size = source[3];

		int xsize = ((size & 0x08)>>3);
		int ysize = ((size & 0x80)>>7);

		xsize++;
		ysize++;

		if (ysize==2) sy -= 16;

		code |= ((source[2] & 0x01) << 6);
		code |= ((source[2] & 0x08) << 4);

		if(flip_screen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = sx+16;
			sy = 250-sy;

			if (ysize==2) sy += 16;
		}

		for (xblock = 0; xblock<xsize; xblock++)
		{
			int xoffs=!flipx ? (xblock*8) : ((xsize-xblock-1)*8);
			int zoomx=((size&7)+1)<<13;
			int zoomy=((size&0x70)+0x10)<<9;
			int xblocksizeinpixels=(zoomx*16)>>16;
			int yblocksizeinpixels=(zoomy*16)>>16;

			for (yblock = 0; yblock<ysize; yblock++)
			{
				int yoffs=!flipy ? yblock : (ysize-yblock-1);
				int sxoffs=(16-xblocksizeinpixels)/2;
				int syoffs=(16-yblocksizeinpixels)/2;
				if (xblock) sxoffs+=xblocksizeinpixels;
				if (yblock) syoffs+=yblocksizeinpixels;

				drawgfxzoom(bitmap,gfx,
						code+yoffs+xoffs,
						color,
						flipx,flipy,
						sx+sxoffs,sy+syoffs,
						cliprect,TRANSPARENCY_PEN,15,
						zoomx,zoomy);
			}
		}
		source -= 8;
	}

}

static TILE_GET_INFO( get_bg0_tile_info )
{
	int code = flower_bg0ram[tile_index];
	int color = flower_bg0ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(2, code, color>>4, 0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	int code = flower_bg1ram[tile_index];
	int color = flower_bg1ram[tile_index+0x100];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(2, code, color>>4, 0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int code = flower_textram[tile_index];
	int color = flower_textram[tile_index+0x400];
	/* Todo - may be tile flip bits? */

	SET_TILE_INFO(0, code, color>>2, 0);
}

VIDEO_START(flower)
{
	flower_bg0_tilemap        = tilemap_create(get_bg0_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,16,16);
	flower_bg1_tilemap        = tilemap_create(get_bg1_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,16,16);
	flower_text_tilemap       = tilemap_create(get_text_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,32,32);
	flower_text_right_tilemap = tilemap_create(get_text_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN, 8, 8, 2,32);

	tilemap_set_transparent_pen(flower_bg1_tilemap,15);
	tilemap_set_transparent_pen(flower_text_tilemap,3);
	tilemap_set_transparent_pen(flower_text_right_tilemap,3);

	tilemap_set_scrolly(flower_text_tilemap, 0, 16);
	tilemap_set_scrolly(flower_text_right_tilemap, 0, 16);
}

VIDEO_UPDATE( flower )
{
	rectangle myclip = *cliprect;

	tilemap_set_scrolly(flower_bg0_tilemap,0, flower_bg0_scroll[0]+16);
	tilemap_set_scrolly(flower_bg1_tilemap,0, flower_bg1_scroll[0]+16);

	tilemap_draw(bitmap,cliprect,flower_bg0_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,flower_bg1_tilemap,0,0);

	draw_sprites(machine,bitmap,cliprect);

	if(flip_screen)
	{
		myclip.min_x = cliprect->min_x;
		myclip.max_x = cliprect->min_x + 15;
	}
	else
	{
		myclip.min_x = cliprect->max_x - 15;
		myclip.max_x = cliprect->max_x;
	}

	tilemap_draw(bitmap,cliprect,flower_text_tilemap,0,0);
	tilemap_draw(bitmap,&myclip,flower_text_right_tilemap,0,0);
	return 0;
}

WRITE8_HANDLER( flower_textram_w )
{
	flower_textram[offset] = data;
	tilemap_mark_tile_dirty(flower_text_tilemap, offset);
	tilemap_mark_all_tiles_dirty(flower_text_right_tilemap);
}

WRITE8_HANDLER( flower_bg0ram_w )
{
	flower_bg0ram[offset] = data;
	tilemap_mark_tile_dirty(flower_bg0_tilemap, offset & 0x1ff);
}

WRITE8_HANDLER( flower_bg1ram_w )
{
	flower_bg1ram[offset] = data;
	tilemap_mark_tile_dirty(flower_bg1_tilemap, offset & 0x1ff);
}

WRITE8_HANDLER( flower_flipscreen_w )
{
	flip_screen_set(data);
}
