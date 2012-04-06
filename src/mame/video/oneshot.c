/* One Shot One Kill Video Hardware */

#include "emu.h"
#include "includes/oneshot.h"


/* bg tilemap */
static TILE_GET_INFO( get_oneshot_bg_tile_info )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();
	int tileno = state->m_bg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 0, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

/* mid tilemap */
static TILE_GET_INFO( get_oneshot_mid_tile_info )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();
	int tileno = state->m_mid_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 2, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_mid_videoram_w)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}


/* fg tilemap */
static TILE_GET_INFO( get_oneshot_fg_tile_info )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();
	int tileno = state->m_fg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO(0, tileno, 3, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

VIDEO_START( oneshot )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();

	state->m_bg_tilemap =  tilemap_create(machine, get_oneshot_bg_tile_info,  tilemap_scan_rows, 16, 16, 32, 32);
	state->m_mid_tilemap = tilemap_create(machine, get_oneshot_mid_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_fg_tilemap =  tilemap_create(machine, get_oneshot_fg_tile_info,  tilemap_scan_rows, 16, 16, 32, 32);

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_mid_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);
}

static void draw_crosshairs( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();
	//int xpos,ypos;

	/* get gun raw coordinates (player 1) */
	state->m_gun_x_p1 = (input_port_read(machine, "LIGHT0_X") & 0xff) * 320 / 256;
	state->m_gun_y_p1 = (input_port_read(machine, "LIGHT0_Y") & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009ab0) */
	//xpos = state->m_gun_x_p1;
	//ypos = state->m_gun_y_p1;

	state->m_gun_x_p1 += state->m_gun_x_shift;

	state->m_gun_y_p1 -= 0x0a;
	if (state->m_gun_y_p1 < 0)
		state->m_gun_y_p1 = 0;


	/* get gun raw coordinates (player 2) */
	state->m_gun_x_p2 = (input_port_read(machine, "LIGHT1_X") & 0xff) * 320 / 256;
	state->m_gun_y_p2 = (input_port_read(machine, "LIGHT1_Y") & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009b6e) */
	//xpos = state->m_gun_x_p2;
	//ypos = state->m_gun_y_p2;

	state->m_gun_x_p2 += state->m_gun_x_shift - 0x0a;
	if (state->m_gun_x_p2 < 0)
		state->m_gun_x_p2 = 0;
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	oneshot_state *state = machine.driver_data<oneshot_state>();
	const UINT16 *source = state->m_sprites;
	const UINT16 *finish = source + (0x1000 / 2);
	const gfx_element *gfx = machine.gfx[1];

	int xpos, ypos;

	while (source < finish)
	{
		int blockx, blocky;
		int num = source[1] & 0xffff;
		int xsize = (source[2] & 0x000f) + 1;
		int ysize = (source[3] & 0x000f) + 1;

		ypos = source[3] & 0xff80;
		xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;


		if (source[0] == 0x0001)
			break;

		xpos -= 8;
		ypos -= 6;

		for (blockx = 0; blockx < xsize; blockx++)
		{
			for (blocky = 0; blocky < ysize; blocky++)
			{
				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8, ypos + blocky * 8, 0);

				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8 - 0x200, ypos + blocky * 8, 0);
			}
		}
		source += 0x4;
	}

}

SCREEN_UPDATE_IND16( oneshot )
{
	oneshot_state *state = screen.machine().driver_data<oneshot_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	state->m_mid_tilemap->set_scrollx(0, state->m_scroll[0] - 0x1f5);
	state->m_mid_tilemap->set_scrolly(0, state->m_scroll[1]);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_mid_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_crosshairs(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( maddonna )
{
	oneshot_state *state = screen.machine().driver_data<oneshot_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	state->m_mid_tilemap->set_scrolly(0, state->m_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	state->m_mid_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);

//  popmessage ("%04x %04x %04x %04x %04x %04x %04x %04x", state->m_scroll[0], state->m_scroll[1], state->m_scroll[2], state->m_scroll[3], state->m_scroll[4], state->m_scroll[5], state->m_scroll[6], state->m_scroll[7]);
	return 0;
}
