/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/goldstar.h"




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


WRITE8_HANDLER( cm_girl_scroll_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->cm_girl_scroll = data;
	/*
        xxxx ----  yscroll
        ---- xxxx  xscroll

        this isn't very fine scrolling, but i see no other registers.
        1000 1000 is the center of the screen.
    */
}

WRITE8_HANDLER( cm_outport0_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->cm_enable_reg = data;
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
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->fg_vidram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

WRITE8_HANDLER( goldstar_fg_atrram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->fg_atrram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_fg_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->fg_vidram[tile_index];
	int attr = state->fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0xf0)<<4,
			attr&0x0f,
			0);
}

static TILE_GET_INFO( get_magical_fg_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->fg_vidram[tile_index];
	int attr = state->fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			(code | (attr & 0xf0)<<4)+state->tile_bank*0x1000,
			attr&0x0f,
			0);
}


// colour / high tile bits are swapped around
static TILE_GET_INFO( get_cherrym_fg_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->fg_vidram[tile_index];
	int attr = state->fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}



WRITE8_HANDLER( goldstar_reel1_ram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->reel1_ram[offset] = data;
	tilemap_mark_tile_dirty(state->reel1_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel1_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel1_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->bgcolor,
			0);
}


WRITE8_HANDLER( goldstar_reel2_ram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;
	state->reel2_ram[offset] = data;
	tilemap_mark_tile_dirty(state->reel2_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel2_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel2_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->bgcolor,
			0);
}

WRITE8_HANDLER( goldstar_reel3_ram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->reel3_ram[offset] = data;
	tilemap_mark_tile_dirty(state->reel3_tilemap,offset);
}

static TILE_GET_INFO( get_goldstar_reel3_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel3_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->bgcolor,
			0);
}

WRITE8_HANDLER( unkch_reel1_attrram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->reel1_attrram[offset] = data;
	tilemap_mark_tile_dirty(state->reel1_tilemap,offset);
}

WRITE8_HANDLER( unkch_reel2_attrram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->reel2_attrram[offset] = data;
	tilemap_mark_tile_dirty(state->reel2_tilemap,offset);
}


WRITE8_HANDLER( unkch_reel3_attrram_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->reel3_attrram[offset] = data;
	tilemap_mark_tile_dirty(state->reel3_tilemap,offset);
}


static TILE_GET_INFO( get_unkch_reel1_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel1_ram[tile_index];
	int attr = state->reel1_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}

static TILE_GET_INFO( get_unkch_reel2_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel2_ram[tile_index];
	int attr = state->reel2_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}

static TILE_GET_INFO( get_unkch_reel3_tile_info )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;
	int code = state->reel3_ram[tile_index];
	int attr = state->reel3_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}




VIDEO_START( goldstar )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;

	state->reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(state->reel1_tilemap, 64);
	tilemap_set_scroll_cols(state->reel2_tilemap, 64);
	tilemap_set_scroll_cols(state->reel3_tilemap, 64);

	state->fg_tilemap = tilemap_create(machine,get_goldstar_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(state->fg_tilemap,0);

	// is there an enable reg for this game?
	state->cm_enable_reg = 0x0b;
}

VIDEO_START( magical )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;

	state->reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(state->reel1_tilemap, 32);
	tilemap_set_scroll_cols(state->reel2_tilemap, 32);
	tilemap_set_scroll_cols(state->reel3_tilemap, 32);

	state->fg_tilemap = tilemap_create(machine,get_magical_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(state->fg_tilemap,0);

	// is there an enable reg for this game?
	state->cm_enable_reg = 0x0b;
}

VIDEO_START( unkch )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;

	state->reel1_tilemap = tilemap_create(machine,get_unkch_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel2_tilemap = tilemap_create(machine,get_unkch_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel3_tilemap = tilemap_create(machine,get_unkch_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(state->reel1_tilemap, 32);
	tilemap_set_scroll_cols(state->reel2_tilemap, 32);
	tilemap_set_scroll_cols(state->reel3_tilemap, 32);

	state->cmaster_girl_num = 0;
	state->cmaster_girl_pal = 0;
	state->unkch_vidreg = 0x00;

	state->fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(state->fg_tilemap,0);

	state->cm_enable_reg = 0x0b;
}

VIDEO_START( cherrym )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;

	state->reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	tilemap_set_scroll_cols(state->reel1_tilemap, 64);
	tilemap_set_scroll_cols(state->reel2_tilemap, 64);
	tilemap_set_scroll_cols(state->reel3_tilemap, 64);

	state->cmaster_girl_num = 0;
	state->cmaster_girl_pal = 0;

	state->fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	tilemap_set_transparent_pen(state->fg_tilemap,0);

	state->cm_enable_reg = 0x0b;
}



WRITE8_HANDLER( goldstar_fa00_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	state->bgcolor = (data & 0x04) >> 2;
	tilemap_mark_all_tiles_dirty (state->reel1_tilemap);
	tilemap_mark_all_tiles_dirty (state->reel2_tilemap);
	tilemap_mark_all_tiles_dirty (state->reel3_tilemap);
}

WRITE8_HANDLER( cm_background_col_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	//printf("cm_background_col_w %02x\n",data);

	/* cherry master writes

    so it's probably

    0ggg cc00

    where g is which girl to display and c is the colour palette

    (note, this doesn't apply to the amcoe games which have no girls, I'm unsure how the priority/positioning works)


    */
	state->cmaster_girl_num = (data >> 4)&0x7;
	state->cmaster_girl_pal = (data >> 2)&0x3;

	//bgcolor = (data & 0x03) >> 0;

	// apparently some boards have this colour scheme?
	// i'm not convinced it isn't just a different prom on them
	#if 0
	state->bgcolor = 0;
	state->bgcolor |= (data & 0x01) << 1;
	state->bgcolor |= (data & 0x02) >> 1;
	#else
	state->bgcolor = (data & 0x03) >> 0;
	#endif

	tilemap_mark_all_tiles_dirty (state->reel1_tilemap);
	tilemap_mark_all_tiles_dirty (state->reel2_tilemap);
	tilemap_mark_all_tiles_dirty (state->reel3_tilemap);
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

static const rectangle unkch_visible1 = { 0*8, (14+48)*8-1,  3*8,  (3+7)*8-1 };
static const rectangle unkch_visible2 = { 0*8, (14+48)*8-1, 10*8, (10+7)*8-1 };
static const rectangle unkch_visible3 = { 0*8, (14+48)*8-1, 17*8, (17+7)*8-1 };

static const rectangle magical_visible1 = { 0*8, (14+48)*8-1,  4*8,  (4+8)*8-1 };
static const rectangle magical_visible2 = { 0*8, (14+48)*8-1, 12*8, (12+8)*8-1 };
static const rectangle magical_visible3 = { 0*8, (14+48)*8-1, 20*8, (20+8)*8-1 };

static const rectangle magical_visible1alt = { 0*8, (16+48)*8-1,  4*8,  16*8-1 };
static const rectangle magical_visible2alt = { 0*8, (16+48)*8-1, 16*8,  28*8-1 };


VIDEO_UPDATE( goldstar )
{
	goldstar_state *state = (goldstar_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!state->cm_enable_reg &0x01)
		return 0;

	if (state->cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i]);
			tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i]);
			tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i]);
		}


		tilemap_draw(bitmap, &visible1, state->reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &visible2, state->reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &visible3, state->reel3_tilemap, 0, 0);
	}

	if (state->cm_enable_reg &0x04)
	{
		if (memory_region(screen->machine,"user1"))
		{
			const gfx_element *gfx = screen->machine->gfx[2];
			int girlyscroll = (INT8)((state->cm_girl_scroll & 0xf0));
			int girlxscroll = (INT8)((state->cm_girl_scroll & 0x0f)<<4);

			drawgfxzoom_transpen(bitmap,cliprect,gfx,state->cmaster_girl_num,state->cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (state->cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0, 0);
	}

	return 0;
}


VIDEO_UPDATE( magical )
{
	goldstar_state *state = (goldstar_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!state->cm_enable_reg &0x01)
		return 0;

	if (state->cm_enable_reg &0x08)
	{
		// guess, could be wrong, but different screens clearly need different reel layouts
		if (state->unkch_vidreg & 2)
		{
			for (i= 0;i < 32;i++)
			{
				tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i*2]);
				tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i*2]);
			//  tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i*2]);
			}


			tilemap_draw(bitmap, &magical_visible1alt, state->reel1_tilemap, 0, 0);
			tilemap_draw(bitmap, &magical_visible2alt, state->reel2_tilemap, 0, 0);
			//tilemap_draw(bitmap, &magical_visible3, state->reel3_tilemap, 0, 0);
		}
		else
		{
			for (i= 0;i < 32;i++)
			{
				tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i*2]);
				tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i*2]);
				tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i*2]);
			}


			tilemap_draw(bitmap, &magical_visible1, state->reel1_tilemap, 0, 0);
			tilemap_draw(bitmap, &magical_visible2, state->reel2_tilemap, 0, 0);
			tilemap_draw(bitmap, &magical_visible3, state->reel3_tilemap, 0, 0);
		}
	}

	if (state->cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0, 0);
	}

	return 0;
}


VIDEO_UPDATE( unkch )
{
	goldstar_state *state = (goldstar_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!state->cm_enable_reg &0x01)
		return 0;

	if (state->cm_enable_reg &0x08)
	{
		// guess, this could be something else completely!!
		// only draw the first 'reels' tilemap, but fullscreen, using alt registers? (or no scrolling at all? - doubtful, see girl)
		if (state->unkch_vidreg & 0x40)
		{
			for (i= 0;i < 32;i++)
			{
				tilemap_set_scrolly(state->reel1_tilemap, i, -0x08/*state->reel1_scroll[(i*2)+1]*/);
			//  tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[(i*2)+1]);
			//  tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[(i*2)+1]);
			}

			tilemap_draw(bitmap, cliprect, state->reel1_tilemap, 0, 0);

		}
		// or draw the reels normally?
		else
		{
			for (i= 0;i < 32;i++)
			{
				tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i*2]);
				tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i*2]);
				tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i*2]);
			}

			tilemap_draw(bitmap, &unkch_visible1, state->reel1_tilemap, 0, 0);
			tilemap_draw(bitmap, &unkch_visible2, state->reel2_tilemap, 0, 0);
			tilemap_draw(bitmap, &unkch_visible3, state->reel3_tilemap, 0, 0);
		}
	}

	if (state->cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap,cliprect, state->fg_tilemap, 0, 0);
	}

	return 0;
}

VIDEO_UPDATE( cmast91 )
{
	goldstar_state *state = (goldstar_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!state->cm_enable_reg &0x01)
		return 0;

	if (state->cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i]);
			tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i]);
			tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i]);
		}

		tilemap_draw(bitmap, &cm91_visible1, state->reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &cm91_visible2, state->reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &cm91_visible3, state->reel3_tilemap, 0, 0);
	}

	if (state->cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	}

	return 0;
}

VIDEO_UPDATE( amcoe1a )
{
	goldstar_state *state = (goldstar_state *)screen->machine->driver_data;
	int i;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (!state->cm_enable_reg &0x01)
		return 0;

	if (state->cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			tilemap_set_scrolly(state->reel1_tilemap, i, state->reel1_scroll[i]);
			tilemap_set_scrolly(state->reel2_tilemap, i, state->reel2_scroll[i]);
			tilemap_set_scrolly(state->reel3_tilemap, i, state->reel3_scroll[i]);
		}

		tilemap_draw(bitmap, &am1a_visible1, state->reel1_tilemap, 0, 0);
		tilemap_draw(bitmap, &am1a_visible2, state->reel2_tilemap, 0, 0);
		tilemap_draw(bitmap, &am1a_visible3, state->reel3_tilemap, 0, 0);
	}

	if (state->cm_enable_reg &0x04)
	{
		// no girls
	}

	if (state->cm_enable_reg &0x02)
	{
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	}

	return 0;
}
