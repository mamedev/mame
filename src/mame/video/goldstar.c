/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
size_t goldstar_video_size;
UINT8 *goldstar_reel1_scroll, *goldstar_reel2_scroll, *goldstar_reel3_scroll;

static int bgcolor;



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static tilemap *goldstar_fg_tilemap;


WRITE8_HANDLER( goldstar_fg_vidram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(goldstar_fg_tilemap,offset);
}

WRITE8_HANDLER( goldstar_fg_atrram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(goldstar_fg_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_fg_tile_info )
{
	int code = videoram[tile_index];
	int attr = colorram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0xf0)<<4,
			attr&0x0f,
			0);
}

// colour / high tile bits are swapped around
static TILE_GET_INFO( get_cherrym_fg_tile_info )
{
	int code = videoram[tile_index];
	int attr = colorram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}



static tilemap *goldstar_reel1_tilemap;
UINT8 *goldstar_reel1_ram;

WRITE8_HANDLER( goldstar_reel1_ram_w )
{
	goldstar_reel1_ram[offset] = data;
	tilemap_mark_tile_dirty(goldstar_reel1_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel1_tile_info )
{
	int code = goldstar_reel1_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			bgcolor,
			0);
}


static tilemap *goldstar_reel2_tilemap;
UINT8 *goldstar_reel2_ram;

WRITE8_HANDLER( goldstar_reel2_ram_w )
{
	goldstar_reel2_ram[offset] = data;
	tilemap_mark_tile_dirty(goldstar_reel2_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel2_tile_info )
{
	int code = goldstar_reel2_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			bgcolor,
			0);
}

static tilemap *goldstar_reel3_tilemap;
UINT8 *goldstar_reel3_ram;

WRITE8_HANDLER( goldstar_reel3_ram_w )
{
	goldstar_reel3_ram[offset] = data;
	tilemap_mark_tile_dirty(goldstar_reel3_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel3_tile_info )
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
	goldstar_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(goldstar_reel1_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel2_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel3_tilemap, 64);

	goldstar_fg_tilemap = tilemap_create(machine,get_goldstar_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(goldstar_fg_tilemap,0);
}

VIDEO_START( cherrym )
{
	goldstar_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(goldstar_reel1_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel2_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel3_tilemap, 64);

	goldstar_fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(goldstar_fg_tilemap,0);
}



WRITE8_HANDLER( goldstar_fa00_w )
{
	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	bgcolor = (data & 0x04) >> 2;
}


// are these hardcoded, or registers?
static const rectangle visible1 = { 0*8, (14+48)*8-1,  4*8,  (4+7)*8-1 };
static const rectangle visible2 = { 0*8, (14+48)*8-1, 12*8, (12+7)*8-1 };
static const rectangle visible3 = { 0*8, (14+48)*8-1, 20*8, (20+7)*8-1 };

static const rectangle cm91_visible1 = { 0*8, (14+48)*8-1, 4*8,  (4+7)*8-1 };	/* same start for reel1 */ 
static const rectangle cm91_visible2 = { 0*8, (14+48)*8-1, 11*8, (12+7)*8-1 };	/* 4 pixels less for reel2 */
static const rectangle cm91_visible3 = { 0*8, (14+48)*8-1, 19*8, (19+7)*8-1 };	/* 8 pixels less for reel3 */


VIDEO_UPDATE( goldstar )
{
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	for (i= 0;i < 64;i++)
	{
		tilemap_set_scrolly(goldstar_reel1_tilemap, i, goldstar_reel1_scroll[i]);
		tilemap_set_scrolly(goldstar_reel2_tilemap, i, goldstar_reel2_scroll[i]);
		tilemap_set_scrolly(goldstar_reel3_tilemap, i, goldstar_reel3_scroll[i]);
	}

	tilemap_draw(bitmap,&visible1,goldstar_reel1_tilemap,0,0);
	tilemap_draw(bitmap,&visible2,goldstar_reel2_tilemap,0,0);
	tilemap_draw(bitmap,&visible3,goldstar_reel3_tilemap,0,0);

	tilemap_draw(bitmap,cliprect,goldstar_fg_tilemap,0,0);

	return 0;
}

VIDEO_UPDATE( cmast91 )
{
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	for (i= 0;i < 64;i++)
	{
		tilemap_set_scrolly(goldstar_reel1_tilemap, i, goldstar_reel1_scroll[i]);
		tilemap_set_scrolly(goldstar_reel2_tilemap, i, goldstar_reel2_scroll[i]);
		tilemap_set_scrolly(goldstar_reel3_tilemap, i, goldstar_reel3_scroll[i]);
	}

	tilemap_draw(bitmap, &cm91_visible1, goldstar_reel1_tilemap, 0, 0);
	tilemap_draw(bitmap, &cm91_visible2, goldstar_reel2_tilemap, 0, 0);
	tilemap_draw(bitmap, &cm91_visible3, goldstar_reel3_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, goldstar_fg_tilemap, 0, 0);

	return 0;
}
