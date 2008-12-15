/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
size_t goldstar_video_size;
UINT8 *goldstar_reel1_scroll, *goldstar_reel2_scroll, *goldstar_reel3_scroll;

static bitmap_t *tmpbitmap4;
static int bgcolor;



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
static tilemap *reel1_tilemap;
UINT8 *goldstar_reel1_ram;

WRITE8_HANDLER( goldstar_reel1_ram_w )
{
	goldstar_reel1_ram[offset] = data;
	tilemap_mark_tile_dirty(reel1_tilemap,offset);
}

static TILE_GET_INFO( get_reel1_tile_info )
{
	int code = goldstar_reel1_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			bgcolor,
			0);
}


static tilemap *reel2_tilemap;
UINT8 *goldstar_reel2_ram;

WRITE8_HANDLER( goldstar_reel2_ram_w )
{
	goldstar_reel2_ram[offset] = data;
	tilemap_mark_tile_dirty(reel2_tilemap,offset);
}

static TILE_GET_INFO( get_reel2_tile_info )
{
	int code = goldstar_reel2_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			bgcolor,
			0);
}

static tilemap *reel3_tilemap;
UINT8 *goldstar_reel3_ram;

WRITE8_HANDLER( goldstar_reel3_ram_w )
{
	goldstar_reel3_ram[offset] = data;
	tilemap_mark_tile_dirty(reel3_tilemap,offset);
}

static TILE_GET_INFO( get_reel3_tile_info )
{
	int code = goldstar_reel3_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			bgcolor,
			0);
}



VIDEO_START( goldstar )
{
	tmpbitmap4 = video_screen_auto_bitmap_alloc(machine->primary_screen);

	reel1_tilemap = tilemap_create(machine,get_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	reel2_tilemap = tilemap_create(machine,get_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	reel3_tilemap = tilemap_create(machine,get_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(reel1_tilemap, 64);
	tilemap_set_scroll_cols(reel2_tilemap, 64);
	tilemap_set_scroll_cols(reel3_tilemap, 64);

}




WRITE8_HANDLER( goldstar_fa00_w )
{
	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	bgcolor = (data & 0x04) >> 2;
}


// are these hardcoded, or registers?
static const rectangle visible1 = { 14*8, (14+48)*8-1,  4*8,  (4+7)*8-1 };
static const rectangle visible2 = { 14*8, (14+48)*8-1, 12*8, (12+7)*8-1 };
static const rectangle visible3 = { 14*8, (14+48)*8-1, 20*8, (20+7)*8-1 };


VIDEO_UPDATE( goldstar )
{
	int offs;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	for (i= 0;i < 64;i++)
	{
		tilemap_set_scrolly(reel1_tilemap, i, goldstar_reel1_scroll[i]);
		tilemap_set_scrolly(reel2_tilemap, i, goldstar_reel2_scroll[i]);
		tilemap_set_scrolly(reel3_tilemap, i, goldstar_reel3_scroll[i]);
	}

	tilemap_draw(bitmap,&visible1,reel1_tilemap,0,0);
	tilemap_draw(bitmap,&visible2,reel2_tilemap,0,0);
	tilemap_draw(bitmap,&visible3,reel3_tilemap,0,0);



	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = offs % 64;
		sy = offs / 64;

		drawgfx(bitmap,screen->machine->gfx[0],
				videoram[offs] + ((colorram[offs] & 0xf0) << 4),
				colorram[offs] & 0x0f,
				0,0,
				8*sx,8*sy,
				0,TRANSPARENCY_PEN,0);
	}

	return 0;
}


VIDEO_UPDATE( cherrym )
{
	int offs;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	for (i= 0;i < 64;i++)
	{
		tilemap_set_scrolly(reel1_tilemap, i, goldstar_reel1_scroll[i]);
		tilemap_set_scrolly(reel2_tilemap, i, goldstar_reel2_scroll[i]);
		tilemap_set_scrolly(reel3_tilemap, i, goldstar_reel3_scroll[i]);
	}

	tilemap_draw(bitmap,&visible1,reel1_tilemap,0,0);
	tilemap_draw(bitmap,&visible2,reel2_tilemap,0,0);
	tilemap_draw(bitmap,&visible3,reel3_tilemap,0,0);

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = offs % 64;
		sy = offs / 64;

		drawgfx(bitmap,screen->machine->gfx[0],
				videoram[offs] + ((colorram[offs] & 0x0f) << 8),
				(colorram[offs] & 0xf0)>>4,
				0,0,
				8*sx,8*sy,
				0,TRANSPARENCY_PEN,0);
	}

	return 0;
}
