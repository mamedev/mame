/***************************************************************************

    Video Hardware description for Taito Gladiator

***************************************************************************/

#include "driver.h"

UINT8 *gladiatr_videoram, *gladiatr_colorram, *gladiatr_textram;

static int video_attributes;
static int fg_scrollx, fg_scrolly, bg_scrollx, bg_scrolly;
static int sprite_bank, sprite_buffer;


static tilemap *fg_tilemap, *bg_tilemap;
static int fg_tile_bank, bg_tile_bank;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( bg_get_tile_info )
{
	UINT8 attr = gladiatr_colorram[tile_index];

	SET_TILE_INFO(
			1,
			gladiatr_videoram[tile_index] + ((attr & 0x07) << 8) + (bg_tile_bank << 11),
			(attr >> 3) ^ 0x1f,
			0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	SET_TILE_INFO(
			0,
			gladiatr_textram[tile_index] + (fg_tile_bank << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ppking )
{
	bg_tilemap = tilemap_create(bg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,64);
	fg_tilemap = tilemap_create(fg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,64);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scroll_cols(bg_tilemap, 0x10);

	sprite_bank = 1;
}

VIDEO_START( gladiatr )
{
	bg_tilemap = tilemap_create(bg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);
	fg_tilemap = tilemap_create(fg_get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(bg_tilemap, -0x30, 0x12f);
	tilemap_set_scrolldx(fg_tilemap, -0x30, 0x12f);

	sprite_bank = 2;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( gladiatr_videoram_w )
{
	gladiatr_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_colorram_w )
{
	gladiatr_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_textram_w )
{
	gladiatr_textram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_paletteram_w )
{
	int r,g,b;

	paletteram[offset] = data;
	offset &= 0x3ff;

	r = (paletteram[offset] >> 0) & 0x0f;
	g = (paletteram[offset] >> 4) & 0x0f;
	b = (paletteram[offset + 0x400] >> 0) & 0x0f;

	r = (r << 1) + ((paletteram[offset + 0x400] >> 4) & 0x01);
	g = (g << 1) + ((paletteram[offset + 0x400] >> 5) & 0x01);
	b = (b << 1) + ((paletteram[offset + 0x400] >> 6) & 0x01);

	palette_set_color_rgb(Machine,offset,pal5bit(r),pal5bit(g),pal5bit(b));
}


WRITE8_HANDLER( gladiatr_spritebuffer_w )
{
	sprite_buffer = data & 1;
}

WRITE8_HANDLER( gladiatr_spritebank_w )
{
	sprite_bank = (data & 1) ? 4 : 2;
}


WRITE8_HANDLER( ppking_video_registers_w )
{
	switch (offset & 0x300)
	{
		case 0x000:
			tilemap_set_scrolly(bg_tilemap, offset & 0x0f, 0x100-data);
			break;
		case 0x200:
			if (data & 0x80)
				fg_scrolly = data + 0x100;
			else
				fg_scrolly = data;
			break;
		case 0x300:
			if (fg_tile_bank != (data & 0x03))
			{
				fg_tile_bank = data & 0x03;
				tilemap_mark_all_tiles_dirty(fg_tilemap);
			}
			video_attributes = data;
			break;
	}

//popmessage("%02x %02x",fg_scrolly, video_attributes);
}

WRITE8_HANDLER( gladiatr_video_registers_w )
{
	switch (offset)
	{
		case 0x000:
			fg_scrolly = data;
			break;
		case 0x080:
			if (fg_tile_bank != (data & 0x03))
			{
				fg_tile_bank = data & 0x03;
				tilemap_mark_all_tiles_dirty(fg_tilemap);
			}
			if (bg_tile_bank != ((data & 0x10) >> 4))
			{
				bg_tile_bank = (data & 0x10) >> 4;
				tilemap_mark_all_tiles_dirty(bg_tilemap);
			}
			video_attributes = data;
			break;
		case 0x100:
			fg_scrollx = data;
			break;
		case 0x200:
			bg_scrolly = data;
			break;
		case 0x300:
			bg_scrollx = data;
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static int tile_offset[2][2] =
		{
			{0x0,0x1},
			{0x2,0x3},
		};
		UINT8 *src = &spriteram[offs + (sprite_buffer << 7)];
		int attributes = src[0x800];
		int size = (attributes & 0x10) >> 4;
		int bank = (attributes & 0x01) + ((attributes & 0x02) ? sprite_bank : 0);
		int tile_number = (src[0]+256*bank);
		int sx = src[0x400+1] + 256*(src[0x801]&1) - 0x38;
		int sy = 240 - src[0x400] - (size ? 16 : 0);
		int xflip = attributes & 0x04;
		int yflip = attributes & 0x08;
		int color = src[1] & 0x1f;
		int x,y;

		if (flip_screen)
		{
			xflip = !xflip;
			yflip = !yflip;
		}

		for (y = 0; y <= size; y++)
		{
			for (x = 0; x <= size; x++)
			{
				int ex = xflip ? (size - x) : x;
				int ey = yflip ? (size - y) : y;

				int t = tile_offset[ey][ex] + tile_number;

				drawgfx(bitmap,machine->gfx[2],
						t,
						color,
						xflip, yflip,
						sx+x*16, sy+y*16,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}



VIDEO_UPDATE( ppking )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect);

	/* the fg layer just selects the upper palette bank on underlying pixels */
	{
		mame_bitmap *flagsbitmap;
		int sx = cliprect->min_x;
		int sy = cliprect->min_y;

		tilemap_get_pixmap( fg_tilemap );
		flagsbitmap = tilemap_get_flagsmap( fg_tilemap );

		while( sy <= cliprect->max_y )
		{
			int x = sx;
			int y = (sy + fg_scrolly) & 0x1ff;

			UINT16 *dest = BITMAP_ADDR16(bitmap, sy, sx);
			while( x <= cliprect->max_x )
			{
				if( *BITMAP_ADDR8(flagsbitmap, y, x)&TILEMAP_PIXEL_LAYER0 )
				{
					*dest += 512;
				}
				x++;
				dest++;
			} /* next x */
			sy++;
		} /* next y */
	}
	return 0;
}

VIDEO_UPDATE( gladiatr )
{
	if (video_attributes & 0x20)
	{
		int scroll;

		scroll = bg_scrollx + ((video_attributes & 0x04) << 6);
		tilemap_set_scrollx(bg_tilemap, 0, scroll ^ (flip_screen ? 0x0f : 0));
		scroll = fg_scrollx + ((video_attributes & 0x08) << 5);
		tilemap_set_scrollx(fg_tilemap, 0, scroll ^ (flip_screen ? 0x0f : 0));

		// always 0 anyway
		tilemap_set_scrolly(bg_tilemap, 0, bg_scrolly);
		tilemap_set_scrolly(fg_tilemap, 0, fg_scrolly);

		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		draw_sprites(machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	}
	else
		fillbitmap( bitmap, get_black_pen(machine), cliprect );
	return 0;
}
