/* Kick Goal - video */

#include "emu.h"
#include "includes/kickgoal.h"


WRITE16_MEMBER(kickgoal_state::kickgoal_fgram_w)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bgram_w)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bgtm->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(kickgoal_state::kickgoal_bg2ram_w)
{
	COMBINE_DATA(&m_bg2ram[offset]);
	m_bg2tm->mark_tile_dirty(offset / 2);
}

/* FG */
static TILE_GET_INFO( get_kickgoal_fg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_fgram[tile_index * 2] & 0x0fff;
	int color = state->m_fgram[tile_index * 2 + 1] & 0x000f;

	SET_TILE_INFO(0, tileno + state->m_fg_base, color + 0x00, 0);
}

/* BG */
static TILE_GET_INFO( get_kickgoal_bg_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bgram[tile_index * 2] & state->m_bg_mask;
	int color = state->m_bgram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->m_bgram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->m_bgram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(1, tileno + state->m_bg_base, color + 0x10, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}

/* BG 2 */
static TILE_GET_INFO( get_kickgoal_bg2_tile_info )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	int tileno = state->m_bg2ram[tile_index * 2] & state->m_bg2_mask;
	int color = state->m_bg2ram[tile_index * 2 + 1] & 0x000f;
	int flipx = state->m_bg2ram[tile_index * 2 + 1] & 0x0020;
	int flipy = state->m_bg2ram[tile_index * 2 + 1] & 0x0040;

	SET_TILE_INFO(state->m_bg2_region, tileno + state->m_bg2_base, color + 0x20, (flipx ? TILE_FLIPX : 0) | (flipy ? TILE_FLIPY : 0));
}


static TILEMAP_MAPPER( tilemap_scan_kicksfg )
{
	/* logical (col,row) -> memory offset */
	return col * 32 + (row & 0x1f) + ((row & 0x20) >> 5) * 0x800;
}

static TILEMAP_MAPPER( tilemap_scan_kicksbg )
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}


static TILEMAP_MAPPER( tilemap_scan_kicksbg2 ) // 16x16 tiles
{
	/* logical (col,row) -> memory offset */
	return col * 8 + (row & 0x7) + ((row & 0x3c) >> 3) * 0x200;
}


static TILEMAP_MAPPER( tilemap_scan_actionhwbg2 ) // 32x32 tiles
{
	/* logical (col,row) -> memory offset */
	return col * 16 + (row & 0xf) + ((row & 0x70) >> 4) * 0x400;
}









static void kickgoal_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();
	UINT16 *spriteram = state->m_spriteram;
	gfx_element *gfx = machine.gfx[1];
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes() / 2; offs += 4)
	{
		int xpos = spriteram[offs + 3];
		int ypos = spriteram[offs + 0] & 0x00ff;
		int tileno = spriteram[offs + 2] & 0x3fff;
		int flipx = spriteram[offs + 1] & 0x0020;
		int color = spriteram[offs + 1] & 0x000f;

		if (spriteram[offs + 0] & 0x0100) break;

		ypos = 0x110 - ypos;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno+state->m_sprbase,
				0x30 + color,
				flipx,0,
				xpos-16+4,ypos-32,15);
	}
}


VIDEO_START( kickgoal )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();

	state->m_sprbase = 0x0000;

	state->m_fg_base = 0x7000;
	state->m_bg_base = 0x1000;
	state->m_bg_mask = 0x0fff;

	state->m_bg2_region = 2; // 32x32 tile source
	state->m_bg2_base = 0x2000 / 4;
	state->m_bg2_mask = (0x2000/4) - 1;

	state->m_fgtm = tilemap_create(machine, get_kickgoal_fg_tile_info, tilemap_scan_kicksfg, 8, 8, 64, 64);
	state->m_bgtm = tilemap_create(machine, get_kickgoal_bg_tile_info, tilemap_scan_kicksbg, 16, 16, 64, 64);
	state->m_bg2tm = tilemap_create(machine, get_kickgoal_bg2_tile_info, tilemap_scan_kicksbg2, 32, 32, 64, 64);

	state->m_fgtm->set_transparent_pen(15);
	state->m_bgtm->set_transparent_pen(15);
}

VIDEO_START( actionhw )
{
	kickgoal_state *state = machine.driver_data<kickgoal_state>();

	state->m_sprbase = 0x4000;
	state->m_fg_base = 0x7000 * 2;

	state->m_bg_base = 0x0000;
	state->m_bg_mask = 0x1fff;

	state->m_bg2_region = 1; // 16x16 tile source
	state->m_bg2_base = 0x2000;
	state->m_bg2_mask = 0x2000 - 1;

	state->m_fgtm = tilemap_create(machine, get_kickgoal_fg_tile_info, tilemap_scan_kicksfg, 8, 8, 64, 64);
	state->m_bgtm = tilemap_create(machine, get_kickgoal_bg_tile_info, tilemap_scan_kicksbg, 16, 16, 64, 64);
	state->m_bg2tm = tilemap_create(machine, get_kickgoal_bg2_tile_info, tilemap_scan_actionhwbg2, 16, 16, 64, 64);

	state->m_fgtm->set_transparent_pen(15);
	state->m_bgtm->set_transparent_pen(15);
}



SCREEN_UPDATE_IND16( kickgoal )
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

	kickgoal_draw_sprites(screen.machine(), bitmap, cliprect);

	state->m_fgtm->draw(bitmap, cliprect, 0, 0);

	return 0;
}
