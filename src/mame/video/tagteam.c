// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Brad Oliver
/***************************************************************************

    tagteam.c

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
	1,              /* single PROM per color */
	0x000, 0x01f,   /* start/end */
	/* R     G     B */
	{  0x00, 0x00, 0x00 }, /* offsets */
	{  0x00, 0x03, 0x06 }, /* shifts */
	{  0x07, 0x07, 0x03 }  /* masks */
};

PALETTE_INIT_MEMBER(tagteam_state, tagteam)
{
	const UINT8 *color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, tagteam_decode_info, tagteam_net_info);
	palette.set_pen_colors(0x00, rgb);
}


WRITE8_MEMBER(tagteam_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(tagteam_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(tagteam_state::mirrorvideoram_r)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_videoram[offset];
}

READ8_MEMBER(tagteam_state::mirrorcolorram_r)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return m_colorram[offset];
}

WRITE8_MEMBER(tagteam_state::mirrorvideoram_w)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	videoram_w(space,offset,data);
}

WRITE8_MEMBER(tagteam_state::mirrorcolorram_w)
{
	int x,y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	colorram_w(space,offset,data);
}

WRITE8_MEMBER(tagteam_state::control_w)
{
	// d0-3: color for blank screen, applies to h/v borders too
	// (not implemented yet, and tagteam doesn't have a global screen on/off bit)

	// d7: palette bank
	m_palettebank = (data & 0x80) >> 7;
}

WRITE8_MEMBER(tagteam_state::flipscreen_w)
{
	// d0: flip screen
	if (flip_screen() != (data &0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	// d6/7: coin counters
	coin_counter_w(machine(), 0, data & 0x80);
	coin_counter_w(machine(), 1, data & 0x40);
}

TILE_GET_INFO_MEMBER(tagteam_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + 256 * m_colorram[tile_index];
	int color = m_palettebank << 1;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void tagteam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tagteam_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS_FLIP_X,
			8, 8, 32, 32);

	save_item(NAME(m_palettebank));
}

void tagteam_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	for (offs = 0; offs < 0x20; offs += 4)
	{
		int spritebank = (m_videoram[offs] & 0x30) << 4;
		int code = m_videoram[offs + 1] + 256 * spritebank;
		int color = m_palettebank << 1 | 1;
		int flipx = m_videoram[offs] & 0x04;
		int flipy = m_videoram[offs] & 0x02;
		int sx = 240 - m_videoram[offs + 3];
		int sy = 240 - m_videoram[offs + 2];

		if (!(m_videoram[offs] & 0x01)) continue;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* Wrap around */

		code = m_videoram[offs + 0x20] + 256 * spritebank;
		color = m_palettebank;
		sy += (flip_screen() ? -256 : 256);


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

UINT32 tagteam_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
