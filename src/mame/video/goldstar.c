/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *goldstar_reel1_scroll, *goldstar_reel2_scroll, *goldstar_reel3_scroll;

static int bgcolor;



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static tilemap *goldstar_fg_tilemap;
static UINT8 cmaster_girl_num;
static UINT8 cmaster_girl_pal;
static UINT8 cm_enable_reg;
static UINT8 cm_girl_scroll;

WRITE8_HANDLER( cm_girl_scroll_w )
{
	cm_girl_scroll = data;
	/*
        xxxx ----  yscroll
        ---- xxxx  xscroll

        this isn't very fine scrolling, but i see no other registers.
        1000 1000 is the center of the screen.
    */
}

WRITE8_HANDLER( cm_outport0_w )
{
	cm_enable_reg = data;
	/*
        ---- ---x  (global enable or irq enable?)
        ---- --x-  (fg enable)
        ---- -x--  (girl enable?)
        ---- x---  (reels enable)

        xxxx ----  unused?

    */
	//popmessage("%02x",data);
}

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

	// is there an enable reg for this game?
	cm_enable_reg = 0x0b;
}

VIDEO_START( cherrym )
{
	goldstar_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	goldstar_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(goldstar_reel1_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel2_tilemap, 64);
	tilemap_set_scroll_cols(goldstar_reel3_tilemap, 64);

	cmaster_girl_num = 0;
	cmaster_girl_pal = 0;

	goldstar_fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(goldstar_fg_tilemap,0);

	cm_enable_reg = 0x0b;
}



WRITE8_HANDLER( goldstar_fa00_w )
{
	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	bgcolor = (data & 0x04) >> 2;
	tilemap_mark_all_tiles_dirty (goldstar_reel1_tilemap);
	tilemap_mark_all_tiles_dirty (goldstar_reel2_tilemap);
	tilemap_mark_all_tiles_dirty (goldstar_reel3_tilemap);
}

WRITE8_HANDLER( cm_background_col_w )
{

	//printf("cm_background_col_w %02x\n",data);

	/* cherry master writes

    so it's probably

    0ggg cc00

    where g is which girl to display and c is the colour palette

    (note, this doesn't apply to the amcoe games which have no girls, I'm unsure how the priority/positioning works)


    */
	cmaster_girl_num = (data >> 4)&0x7;
	cmaster_girl_pal = (data >> 2)&0x3;

	//bgcolor = (data & 0x03) >> 0;

	// apparently some boards have this colour scheme?
	// i'm not convinced it isn't just a different prom on them
	#if 0
	bgcolor = 0;
	bgcolor |= (data & 0x01) << 1;
	bgcolor |= (data & 0x02) >> 1;
	#else
	bgcolor = (data & 0x03) >> 0;
	#endif

	tilemap_mark_all_tiles_dirty (goldstar_reel1_tilemap);
	tilemap_mark_all_tiles_dirty (goldstar_reel2_tilemap);
	tilemap_mark_all_tiles_dirty (goldstar_reel3_tilemap);
}

// are these hardcoded, or registers?
static const rectangle visible1 = { 0*8, (14+48)*8-1,  4*8,  (4+7)*8-1 };
static const rectangle visible2 = { 0*8, (14+48)*8-1, 12*8, (12+7)*8-1 };
static const rectangle visible3 = { 0*8, (14+48)*8-1, 20*8, (20+7)*8-1 };

static const rectangle cm91_visible1 = { 0*8, (14+48)*8-1, 4*8,  (4+7)*8-1 };	/* same start for reel1 */
static const rectangle cm91_visible2 = { 0*8, (14+48)*8-1, 11*8, (12+7)*8-1 };	/* 4 pixels less for reel2 */
static const rectangle cm91_visible3 = { 0*8, (14+48)*8-1, 19*8, (19+7)*8-1 };	/* 8 pixels less for reel3 */

static const rectangle am1a_visible1 = { 0*8, (14+48)*8-1,  4*8,  (4+6)*8-1 };
static const rectangle am1a_visible2 = { 0*8, (14+48)*8-1, 10*8, (10+6)*8-1 };
static const rectangle am1a_visible3 = { 0*8, (14+48)*8-1, 16*8, (16+6)*8-1 };


VIDEO_UPDATE( goldstar )
{
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!cm_enable_reg &0x01)
		return 0;

	if (cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(goldstar_reel1_tilemap, i, goldstar_reel1_scroll[i]);
			tilemap_set_scrolly(goldstar_reel2_tilemap, i, goldstar_reel2_scroll[i]);
			tilemap_set_scrolly(goldstar_reel3_tilemap, i, goldstar_reel3_scroll[i]);
		}


		tilemap_draw(bitmap, &visible1, goldstar_reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &visible2, goldstar_reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &visible3, goldstar_reel3_tilemap, 0, 0);
	}

	if (cm_enable_reg &0x04)
	{
		if (memory_region(screen->machine,"user1"))
		{
			const gfx_element *gfx = screen->machine->gfx[2];
			int girlyscroll = (INT8)((cm_girl_scroll & 0xf0));
			int girlxscroll = (INT8)((cm_girl_scroll & 0x0f)<<4);

			drawgfxzoom_transpen(bitmap,cliprect,gfx,cmaster_girl_num,cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap,cliprect, goldstar_fg_tilemap, 0, 0);
	}

	return 0;
}

VIDEO_UPDATE( cmast91 )
{
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!cm_enable_reg &0x01)
		return 0;

	if (cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(goldstar_reel1_tilemap, i, goldstar_reel1_scroll[i]);
			tilemap_set_scrolly(goldstar_reel2_tilemap, i, goldstar_reel2_scroll[i]);
			tilemap_set_scrolly(goldstar_reel3_tilemap, i, goldstar_reel3_scroll[i]);
		}

		tilemap_draw(bitmap, &cm91_visible1, goldstar_reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &cm91_visible2, goldstar_reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &cm91_visible3, goldstar_reel3_tilemap, 0, 0);
	}

	if (cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap, cliprect, goldstar_fg_tilemap, 0, 0);
	}

	return 0;
}

VIDEO_UPDATE( amcoe1a )
{
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!cm_enable_reg &0x01)
		return 0;

	if (cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(goldstar_reel1_tilemap, i, goldstar_reel1_scroll[i]);
			tilemap_set_scrolly(goldstar_reel2_tilemap, i, goldstar_reel2_scroll[i]);
			tilemap_set_scrolly(goldstar_reel3_tilemap, i, goldstar_reel3_scroll[i]);
		}

		tilemap_draw(bitmap, &am1a_visible1, goldstar_reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &am1a_visible2, goldstar_reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &am1a_visible3, goldstar_reel3_tilemap, 0, 0);
	}

	if (cm_enable_reg &0x04)
	{
		// no girls
	}

	if (cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap, cliprect, goldstar_fg_tilemap, 0, 0);
	}

	return 0;
}
