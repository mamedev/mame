/***************************************************************************

    Lemmings video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the vram format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

#include "driver.h"
#include "lemmings.h"

UINT16 *lemmings_pixel_0_data,*lemmings_pixel_1_data,*lemmings_vram_data,*lemmings_control_data;
static UINT16 *sprite_triple_buffer_0,*sprite_triple_buffer_1;
static UINT8 *vram_buffer;
static bitmap_t *bitmap0;
static tilemap *vram_tilemap;

/******************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16 *spritedata, int gfxbank, UINT16 pri)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spritedata[offs+1] & 0x3fff;

		if ((spritedata[offs+2]&0x2000)!=pri)
			continue;

		y = spritedata[offs];
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		x = spritedata[offs+2];
		colour = (x >>9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		if (x>320 || x<-16) continue;

		sprite &= ~multi;
		if (fy)
			inc = 1;
		else
		{
			sprite += multi;
			inc = -1;
		}
		mult=-16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[gfxbank],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

/******************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT16 tile=lemmings_vram_data[tile_index];

	SET_TILE_INFO(
			2,
			tile&0x7ff,
			(tile>>12)&0xf,
			0);
}

VIDEO_START( lemmings )
{
	bitmap0 = auto_bitmap_alloc(machine,2048,256,video_screen_get_format(machine->primary_screen));
	vram_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_cols,8,8,64,32);

	vram_buffer = auto_alloc_array(machine, UINT8, 2048*64); /* 64 bytes per VRAM character */
	sprite_triple_buffer_0 = auto_alloc_array(machine, UINT16, 0x800/2);
	sprite_triple_buffer_1 = auto_alloc_array(machine, UINT16, 0x800/2);

	tilemap_set_transparent_pen(vram_tilemap,0);
	bitmap_fill(bitmap0,0,0x100);

	gfx_element_set_source(machine->gfx[2], vram_buffer);
}

VIDEO_EOF( lemmings )
{
	memcpy(sprite_triple_buffer_0,buffered_spriteram16,0x800);
	memcpy(sprite_triple_buffer_1,buffered_spriteram16_2,0x800);
}

/******************************************************************************/

WRITE16_HANDLER( lemmings_pixel_0_w )
{
	int sx,sy,src,old;

	old=lemmings_pixel_0_data[offset];
	COMBINE_DATA(&lemmings_pixel_0_data[offset]);
	src=lemmings_pixel_0_data[offset];
	if (old==src)
		return;

	sy=(offset<<1)/0x800;
	sx=(offset<<1)&0x7ff;

	if (sx>2047 || sy>255)
		return;

	*BITMAP_ADDR16(bitmap0, sy, sx+0) = ((src>>8)&0xf)|0x100;
	*BITMAP_ADDR16(bitmap0, sy, sx+1) = ((src>>0)&0xf)|0x100;
}

WRITE16_HANDLER( lemmings_pixel_1_w )
{
	int sx,sy,src,old,tile;

	old=lemmings_pixel_1_data[offset];
	COMBINE_DATA(&lemmings_pixel_1_data[offset]);
	src=lemmings_pixel_1_data[offset];
//  if (old==src)
//      return;

	sy=((offset<<1)/0x200);
	sx=((offset<<1)&0x1ff);

	/* Copy pixel to buffer for easier decoding later */
	tile=((sx/8)*32)+(sy/8);
	gfx_element_mark_dirty(space->machine->gfx[2], tile);
	vram_buffer[(tile*64) + ((sx&7)) + ((sy&7)*8)]=(src>>8)&0xf;

	sx+=1; /* Update both pixels in the word */
	vram_buffer[(tile*64) + ((sx&7)) + ((sy&7)*8)]=(src>>0)&0xf;
}

WRITE16_HANDLER( lemmings_vram_w )
{
	COMBINE_DATA(&lemmings_vram_data[offset]);
	tilemap_mark_tile_dirty(vram_tilemap,offset);
}

VIDEO_UPDATE( lemmings )
{
	int x1=-lemmings_control_data[0],x0=-lemmings_control_data[2],y=0;
	rectangle rect;
	rect.max_y=cliprect->max_y;
	rect.min_y=cliprect->min_y;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	draw_sprites(screen->machine,bitmap,cliprect,sprite_triple_buffer_1,1,0x0000);

	/* Pixel layer can be windowed in hardware (two player mode) */
	if ((lemmings_control_data[6]&2)==0) {
		copyscrollbitmap_trans(bitmap,bitmap0,1,&x1,1,&y,cliprect,0x100);
	} else {
		rect.max_x=159;
		rect.min_x=0;
		copyscrollbitmap_trans(bitmap,bitmap0,1,&x0,1,&y,&rect,0x100);
		rect.max_x=319;
		rect.min_x=160;
		copyscrollbitmap_trans(bitmap,bitmap0,1,&x1,1,&y,&rect,0x100);
	}
	draw_sprites(screen->machine,bitmap,cliprect,sprite_triple_buffer_0,0,0x0000);
	draw_sprites(screen->machine,bitmap,cliprect,sprite_triple_buffer_1,1,0x2000);
	tilemap_draw(bitmap,cliprect,vram_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect,sprite_triple_buffer_0,0,0x2000);
	return 0;
}
