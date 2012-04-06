/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/tagteam.h"

// TODO: fix or confirm resnet implementation
// schematics say emitter circuit with 47 ohm pullup and 470 ohm pulldown but this results in a bad palette
static const res_net_info tagteam_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_EMITTER, 4700, 0, 3, { 4700, 3300, 1500 } },
		{ RES_NET_AMP_EMITTER, 4700, 0, 3, { 4700, 3300, 1500 } },
		{ RES_NET_AMP_EMITTER, 4700, 0, 2, {       3300, 1500 } }
	}
};

static const res_net_decode_info tagteam_decode_info =
{
	1,				/* single PROM per color */
	0x000, 0x01f,	/* start/end */
	/* R     G     B */
	{  0x00, 0x00, 0x00 }, /* offsets */
	{  0x00, 0x03, 0x06 }, /* shifts */
	{  0x07, 0x07, 0x03 }  /* masks */
};

PALETTE_INIT( tagteam )
{
	rgb_t *rgb;

	rgb = compute_res_net_all(machine, color_prom, &tagteam_decode_info, &tagteam_net_info);
	palette_set_colors(machine, 0x00, rgb, 0x20);
	auto_free(machine, rgb);
}


WRITE8_MEMBER(tagteam_state::tagteam_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(tagteam_state::tagteam_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(tagteam_state::tagteam_mirrorvideoram_r)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_videoram[offset];
}

READ8_MEMBER(tagteam_state::tagteam_mirrorcolorram_r)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_colorram[offset];
}

WRITE8_MEMBER(tagteam_state::tagteam_mirrorvideoram_w)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	tagteam_videoram_w(space,offset,data);
}

WRITE8_MEMBER(tagteam_state::tagteam_mirrorcolorram_w)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	tagteam_colorram_w(space,offset,data);
}

WRITE8_MEMBER(tagteam_state::tagteam_control_w)
{

	// d0-3: color for blank screen, applies to h/v borders too
	// (not implemented yet, and tagteam doesn't have a global screen on/off bit)

	// d7: palette bank
	m_palettebank = (data & 0x80) >> 7;
}

WRITE8_MEMBER(tagteam_state::tagteam_flipscreen_w)
{
	// d0: flip screen
	if (flip_screen_get(machine()) != (data &0x01))
	{
		flip_screen_set(machine(), data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	// d6/7: coin counters
	coin_counter_w(machine(), 0, data & 0x80);
	coin_counter_w(machine(), 1, data & 0x40);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	tagteam_state *state = machine.driver_data<tagteam_state>();
	int code = state->m_videoram[tile_index] + 256 * state->m_colorram[tile_index];
	int color = state->m_palettebank << 1;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( tagteam )
{
	tagteam_state *state = machine.driver_data<tagteam_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_flip_x,
		 8, 8, 32, 32);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tagteam_state *state = machine.driver_data<tagteam_state>();
	int offs;

	for (offs = 0; offs < 0x20; offs += 4)
	{
		int spritebank = (state->m_videoram[offs] & 0x30) << 4;
		int code = state->m_videoram[offs + 1] + 256 * spritebank;
		int color = state->m_palettebank << 1 | 1;
		int flipx = state->m_videoram[offs] & 0x04;
		int flipy = state->m_videoram[offs] & 0x02;
		int sx = 240 - state->m_videoram[offs + 3];
		int sy = 240 - state->m_videoram[offs + 2];

		if (!(state->m_videoram[offs] & 0x01)) continue;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect,
			machine.gfx[1],
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* Wrap around */

		code = state->m_videoram[offs + 0x20] + 256 * spritebank;
		color = state->m_palettebank;
		sy += (flip_screen_get(machine) ? -256 : 256);

		drawgfx_transpen(bitmap, cliprect,
			machine.gfx[1],
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

SCREEN_UPDATE_IND16( tagteam )
{
	tagteam_state *state = screen.machine().driver_data<tagteam_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
