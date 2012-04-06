/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/goldstar.h"




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


WRITE8_MEMBER(goldstar_state::cm_girl_scroll_w)
{

	m_cm_girl_scroll = data;
	/*
        xxxx ----  yscroll
        ---- xxxx  xscroll

        this isn't very fine scrolling, but i see no other registers.
        1000 1000 is the center of the screen.
    */
}

WRITE8_MEMBER(goldstar_state::cm_outport0_w)
{

	m_cm_enable_reg = data;
	/*
        ---- ---x  (global enable or irq enable?)
        ---- --x-  (fg enable)
        ---- -x--  (girl enable?)
        ---- x---  (reels enable)

        xxxx ----  unused?

    */
	//popmessage("%02x",data);
}

WRITE8_MEMBER(goldstar_state::goldstar_fg_vidram_w)
{

	m_fg_vidram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(goldstar_state::goldstar_fg_atrram_w)
{

	m_fg_atrram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_goldstar_fg_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_fg_vidram[tile_index];
	int attr = state->m_fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0xf0)<<4,
			attr&0x0f,
			0);
}

static TILE_GET_INFO( get_magical_fg_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_fg_vidram[tile_index];
	int attr = state->m_fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			(code | (attr & 0xf0)<<4)+state->m_tile_bank*0x1000,
			attr&0x0f,
			0);
}


// colour / high tile bits are swapped around
static TILE_GET_INFO( get_cherrym_fg_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_fg_vidram[tile_index];
	int attr = state->m_fg_atrram[tile_index];

	SET_TILE_INFO(
			0,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}



WRITE8_MEMBER(goldstar_state::goldstar_reel1_ram_w)
{

	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_goldstar_reel1_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel1_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->m_bgcolor,
			0);
}


WRITE8_MEMBER(goldstar_state::goldstar_reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_goldstar_reel2_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel2_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->m_bgcolor,
			0);
}

WRITE8_MEMBER(goldstar_state::goldstar_reel3_ram_w)
{

	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_goldstar_reel3_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel3_ram[tile_index];

	SET_TILE_INFO(
			1,
			code,
			state->m_bgcolor,
			0);
}

WRITE8_MEMBER(goldstar_state::unkch_reel1_attrram_w)
{

	m_reel1_attrram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(goldstar_state::unkch_reel2_attrram_w)
{

	m_reel2_attrram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(goldstar_state::unkch_reel3_attrram_w)
{

	m_reel3_attrram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}


static TILE_GET_INFO( get_unkch_reel1_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel1_ram[tile_index];
	int attr = state->m_reel1_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}

static TILE_GET_INFO( get_unkch_reel2_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel2_ram[tile_index];
	int attr = state->m_reel2_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}

static TILE_GET_INFO( get_unkch_reel3_tile_info )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();
	int code = state->m_reel3_ram[tile_index];
	int attr = state->m_reel3_attrram[tile_index];

	SET_TILE_INFO(
			1,
			code | (attr & 0x0f)<<8,
			(attr&0xf0)>>4,
			0);
}




VIDEO_START( goldstar )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();

	state->m_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	state->m_reel1_tilemap->set_scroll_cols(64);
	state->m_reel2_tilemap->set_scroll_cols(64);
	state->m_reel3_tilemap->set_scroll_cols(64);

	state->m_fg_tilemap = tilemap_create(machine,get_goldstar_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	state->m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	state->m_cm_enable_reg = 0x0b;
}

VIDEO_START( bingowng )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();

	state->m_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);

	state->m_reel1_tilemap->set_scroll_cols(64);

	state->m_fg_tilemap = tilemap_create(machine,get_goldstar_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	state->m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	state->m_cm_enable_reg = 0x0b;
}

VIDEO_START( magical )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();

	state->m_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	state->m_reel1_tilemap->set_scroll_cols(32);
	state->m_reel2_tilemap->set_scroll_cols(32);
	state->m_reel3_tilemap->set_scroll_cols(32);

	state->m_fg_tilemap = tilemap_create(machine,get_magical_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	state->m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	state->m_cm_enable_reg = 0x0b;
}

VIDEO_START( unkch )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();

	state->m_reel1_tilemap = tilemap_create(machine,get_unkch_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel2_tilemap = tilemap_create(machine,get_unkch_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel3_tilemap = tilemap_create(machine,get_unkch_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	state->m_reel1_tilemap->set_scroll_cols(32);
	state->m_reel2_tilemap->set_scroll_cols(32);
	state->m_reel3_tilemap->set_scroll_cols(32);

	state->m_cmaster_girl_num = 0;
	state->m_cmaster_girl_pal = 0;
	state->m_unkch_vidreg = 0x00;

	state->m_fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_cm_enable_reg = 0x0b;
}

VIDEO_START( cherrym )
{
	goldstar_state *state = machine.driver_data<goldstar_state>();

	state->m_reel1_tilemap = tilemap_create(machine,get_goldstar_reel1_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel2_tilemap = tilemap_create(machine,get_goldstar_reel2_tile_info,tilemap_scan_rows,8,32, 64, 8);
	state->m_reel3_tilemap = tilemap_create(machine,get_goldstar_reel3_tile_info,tilemap_scan_rows,8,32, 64, 8);

	state->m_reel1_tilemap->set_scroll_cols(64);
	state->m_reel2_tilemap->set_scroll_cols(64);
	state->m_reel3_tilemap->set_scroll_cols(64);

	state->m_cmaster_girl_num = 0;
	state->m_cmaster_girl_pal = 0;

	state->m_fg_tilemap = tilemap_create(machine,get_cherrym_fg_tile_info,tilemap_scan_rows,8,8, 64, 32);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_cm_enable_reg = 0x0b;
}



WRITE8_MEMBER(goldstar_state::goldstar_fa00_w)
{

	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	m_bgcolor = (data & 0x04) >> 2;
	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();
}

WRITE8_MEMBER(goldstar_state::cm_background_col_w)
{

	//printf("cm_background_col_w %02x\n",data);

	/* cherry master writes

    so it's probably

    0ggg cc00

    where g is which girl to display and c is the colour palette

    (note, this doesn't apply to the amcoe games which have no girls, I'm unsure how the priority/positioning works)


    */
	m_cmaster_girl_num = (data >> 4)&0x7;
	m_cmaster_girl_pal = (data >> 2)&0x3;

	//bgcolor = (data & 0x03) >> 0;

	// apparently some boards have this colour scheme?
	// i'm not convinced it isn't just a different prom on them
	#if 0
	m_bgcolor = 0;
	m_bgcolor |= (data & 0x01) << 1;
	m_bgcolor |= (data & 0x02) >> 1;
	#else
	m_bgcolor = (data & 0x03) >> 0;
	#endif

	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();
}



SCREEN_UPDATE_IND16( goldstar )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i]);
			state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i]);
			state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i]);
		}


		// are these hardcoded, or registers?
		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+7)*8-1);
		const rectangle visible2(0*8, (14+48)*8-1, 12*8, (12+7)*8-1);
		const rectangle visible3(0*8, (14+48)*8-1, 20*8, (20+7)*8-1);

		state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
		state->m_reel2_tilemap->draw(bitmap, visible2, 0, 0);
		state->m_reel3_tilemap->draw(bitmap, visible3, 0, 0);
	}

	if (state->m_cm_enable_reg &0x04)
	{
		if (screen.machine().region("user1")->base())
		{
			const gfx_element *gfx = screen.machine().gfx[2];
			int girlyscroll = (INT8)((state->m_cm_girl_scroll & 0xf0));
			int girlxscroll = (INT8)((state->m_cm_girl_scroll & 0x0f)<<4);

			drawgfxzoom_transpen(bitmap,cliprect,gfx,state->m_cmaster_girl_num,state->m_cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}


SCREEN_UPDATE_IND16( bingowng )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i]);
		}


		const rectangle visible1(0*8, (14+48)*8-1,  3*8,  (4+7)*8-1);
		state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
	}

	if (state->m_cm_enable_reg &0x04)
	{
		if (screen.machine().region("user1")->base())
		{
			const gfx_element *gfx = screen.machine().gfx[2];
			int girlyscroll = (INT8)((state->m_cm_girl_scroll & 0xf0));
			int girlxscroll = (INT8)((state->m_cm_girl_scroll & 0x0f)<<4);

			drawgfxzoom_transpen(bitmap,cliprect,gfx,state->m_cmaster_girl_num,state->m_cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}


SCREEN_UPDATE_IND16( magical )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		// guess, could be wrong, but different screens clearly need different reel layouts
		if (state->m_unkch_vidreg & 2)
		{
			for (i= 0;i < 32;i++)
			{
				state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i*2]);
				state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i*2]);
			//  state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i*2]);
			}


			const rectangle visible1alt(0*8, (16+48)*8-1,  4*8,  16*8-1);
			const rectangle visible2alt(0*8, (16+48)*8-1, 16*8,  28*8-1);

			state->m_reel1_tilemap->draw(bitmap, visible1alt, 0, 0);
			state->m_reel2_tilemap->draw(bitmap, visible2alt, 0, 0);
			//state->m_reel3_tilemap->draw(bitmap, &magical_visible3, 0, 0);
		}
		else
		{
			for (i= 0;i < 32;i++)
			{
				state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i*2]);
				state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i*2]);
				state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i*2]);
			}


			const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+8)*8-1);
			const rectangle visible2(0*8, (14+48)*8-1, 12*8, (12+8)*8-1);
			const rectangle visible3(0*8, (14+48)*8-1, 20*8, (20+8)*8-1);

			state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
			state->m_reel2_tilemap->draw(bitmap, visible2, 0, 0);
			state->m_reel3_tilemap->draw(bitmap, visible3, 0, 0);
		}
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}


SCREEN_UPDATE_IND16( unkch )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		// guess, this could be something else completely!!
		// only draw the first 'reels' tilemap, but fullscreen, using alt registers? (or no scrolling at all? - doubtful, see girl)
		if (state->m_unkch_vidreg & 0x40)
		{
			for (i= 0;i < 32;i++)
			{
				state->m_reel1_tilemap->set_scrolly(i, -0x08/*state->m_reel1_scroll[(i*2)+1]*/);
			//  state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[(i*2)+1]);
			//  state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[(i*2)+1]);
			}

			state->m_reel1_tilemap->draw(bitmap, cliprect, 0, 0);

		}
		// or draw the reels normally?
		else
		{
			for (i= 0;i < 32;i++)
			{
				state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i*2]);
				state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i*2]);
				state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i*2]);
			}

			const rectangle visible1(0*8, (14+48)*8-1,  3*8,  (3+7)*8-1);
			const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
			const rectangle visible3(0*8, (14+48)*8-1, 17*8, (17+7)*8-1);

			state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
			state->m_reel2_tilemap->draw(bitmap, visible2, 0, 0);
			state->m_reel3_tilemap->draw(bitmap, visible3, 0, 0);
		}
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}

SCREEN_UPDATE_IND16( cmast91 )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i]);
			state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i]);
			state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i]);
		}

		const rectangle visible1(0*8, (14+48)*8-1, 4*8,  (4+7)*8-1);	/* same start for reel1 */
		const rectangle visible2(0*8, (14+48)*8-1, 11*8, (12+7)*8-1);	/* 4 pixels less for reel2 */
		const rectangle visible3(0*8, (14+48)*8-1, 19*8, (19+7)*8-1);	/* 8 pixels less for reel3 */

		state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
		state->m_reel2_tilemap->draw(bitmap, visible2, 0, 0);
		state->m_reel3_tilemap->draw(bitmap, visible3, 0, 0);
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}

SCREEN_UPDATE_IND16( amcoe1a )
{
	goldstar_state *state = screen.machine().driver_data<goldstar_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_cm_enable_reg &0x01)
		return 0;

	if (state->m_cm_enable_reg &0x08)
	{
		for (i= 0;i < 64;i++)
		{
			state->m_reel1_tilemap->set_scrolly(i, state->m_reel1_scroll[i]);
			state->m_reel2_tilemap->set_scrolly(i, state->m_reel2_scroll[i]);
			state->m_reel3_tilemap->set_scrolly(i, state->m_reel3_scroll[i]);
		}

		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+6)*8-1);
		const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+6)*8-1);
		const rectangle visible3(0*8, (14+48)*8-1, 16*8, (16+6)*8-1);

		state->m_reel1_tilemap->draw(bitmap, visible1, 0, 0);
		state->m_reel2_tilemap->draw(bitmap, visible2, 0, 0);
		state->m_reel3_tilemap->draw(bitmap, visible3, 0, 0);
	}

	if (state->m_cm_enable_reg &0x04)
	{
		// no girls
	}

	if (state->m_cm_enable_reg &0x02)
	{
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	}

	return 0;
}
