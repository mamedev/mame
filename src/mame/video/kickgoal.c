/* Kick Goal - video */

#include "emu.h"
#include "includes/kickgoal.h"


WRITE16_MEMBER(kickgoal_state::kickgoal_fgram_w)
{
	m_fgram[offset] = data;
	m_fgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bgram_w)
{
	m_bgram[offset] = data;
	m_bgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bg2ram_w)
{
	m_bg2ram[offset] = data;
	m_bg2tm->mark_tile_dirty(offset / 2);
}



/* FG */
static TILE_GET_INFO( get_kickgoal_fg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_fgram[tile_index * 2] & 0x0fff;
	int color = state->m_fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(0, tileno + 0x7000, color + 0x00, 0);
}

/* BG */
static TILE_GET_INFO( get_kickgoal_bg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bgram[tile_index * 2] & 0x0fff;
	int color = state->m_bgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(1, tileno + 0x1000, color + 0x10, 0);
}

/* BG 2 */
static TILE_GET_INFO( get_kickgoal_bg2_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bg2ram[tile_index * 2] & 0x07ff;
	int color = state->m_bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->m_bg2ram[tile_index * 2 + 1] & 0x0020;

	SET_TILE_INFO(2, tileno + 0x800, color + 0x20, flipx ? TILE_FLIPX : 0);
}


static TILEMAP_MAPPER( tilemap_scan_kicksbg2 )
{
	/* logical (col,row) -> memory offset */
	return col * 8 + (row & 0x7) + ((row & 0x3c) >> 3) * 0x200;
}

static TILEMAP_MAPPER( tilemap_scan_kicksbg )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_kicksfg )
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}


VIDEO_START( kickgoal )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();

	state->m_fgtm = tilemap_create(machine, get_kickgoal_fg_tile_info, tilemap_scan_kicksfg, 8, 16, 64, 64);
	state->m_bgtm = tilemap_create(machine, get_kickgoal_bg_tile_info, tilemap_scan_kicksbg, 16, 32, 64, 64);
	state->m_bg2tm = tilemap_create(machine, get_kickgoal_bg2_tile_info, tilemap_scan_kicksbg2, 32, 64, 64, 64);

	state->m_fgtm->set_transparent_pen(15);
	state->m_bgtm->set_transparent_pen(15);
}



static void kickgoal_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	UINT16 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[1];
	int offs;

	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x0fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100)
			break;

		ypos *= 2;
		ypos = 0x200 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


SCREEN_UPDATE_IND16( kickgoal )
{
	kickgoal_state *state = screen.machine().driver_data<kickgoal_state>();

	/* set scroll */
	state->m_fgtm->set_scrollx(0, state->m_scrram[0]);
	state->m_fgtm->set_scrolly(0, state->m_scrram[1] * 2);
	state->m_bgtm->set_scrollx(0, state->m_scrram[2]);
	state->m_bgtm->set_scrolly(0, state->m_scrram[3] * 2);
	state->m_bg2tm->set_scrollx(0, state->m_scrram[4]);
	state->m_bg2tm->set_scrolly(0, state->m_scrram[5] * 2);

	/* draw */
	state->m_bg2tm->draw(bitmap, cliprect, 0, 0);
	state->m_bgtm->draw(bitmap, cliprect, 0, 0);

	kickgoal_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_fgtm->draw(bitmap, cliprect, 0, 0);

	/*
    popmessage ("Regs %04x %04x %04x %04x %04x %04x %04x %04x",
    state->m_scrram[0],
    state->m_scrram[1],
    state->m_scrram[2],
    state->m_scrram[3],
    state->m_scrram[4],
    state->m_scrram[5],
    state->m_scrram[6],
    state->m_scrram[7]);
    */
	return 0;
}

/* Holywood Action */

/* FG */
static TILE_GET_INFO( get_actionhw_fg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_fgram[tile_index * 2] & 0x0fff;
	int color = state->m_fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(0, tileno + 0x7000 * 2, color + 0x00, 0);
}

/* BG */
static TILE_GET_INFO( get_actionhw_bg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bgram[tile_index * 2] & 0x1fff;
	int color = state->m_bgram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->m_bgram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->m_bgram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(1, tileno + 0x0000, color + 0x10, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}

/* BG 2 */
static TILE_GET_INFO( get_actionhw_bg2_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bg2ram[tile_index * 2] & 0x1fff;
	int color = state->m_bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->m_bg2ram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->m_bg2ram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(1, tileno + 0x2000, color + 0x20, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}


static TILEMAP_MAPPER( tilemap_scan_actionhwbg2 )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_actionhwbg )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}

static TILEMAP_MAPPER( tilemap_scan_actionhwfg )
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}


VIDEO_START( actionhw )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();

	state->m_fgtm = tilemap_create(machine, get_actionhw_fg_tile_info, tilemap_scan_actionhwfg, 8, 8, 64, 64);
	state->m_bgtm = tilemap_create(machine, get_actionhw_bg_tile_info, tilemap_scan_actionhwbg, 16, 16, 64, 64);
	state->m_bg2tm = tilemap_create(machine, get_actionhw_bg2_tile_info, tilemap_scan_actionhwbg2, 16, 16, 64, 64);

	state->m_fgtm->set_transparent_pen(15);
	state->m_bgtm->set_transparent_pen(15);
}


static void actionhw_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	UINT16 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[1];
	int offs;

	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x3fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100) break;

		ypos = 0x110 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno+0x4000,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


SCREEN_UPDATE_IND16( actionhw )
{
	kickgoal_state *state = screen.machine().driver_data<kickgoal_state>();
	/* set scroll */
	state->m_fgtm->set_scrollx(0, state->m_scrram[0]);
	state->m_fgtm->set_scrolly(0, state->m_scrram[1]);
	state->m_bgtm->set_scrollx(0, state->m_scrram[2]);
	state->m_bgtm->set_scrolly(0, state->m_scrram[3]);
	state->m_bg2tm->set_scrollx(0, state->m_scrram[4]);
	state->m_bg2tm->set_scrolly(0, state->m_scrram[5]);

	/* draw */
	state->m_bg2tm->draw(bitmap, cliprect, 0, 0);
	state->m_bgtm->draw(bitmap, cliprect, 0, 0);

	actionhw_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_fgtm->draw(bitmap, cliprect, 0, 0);
	return 0;
}
