// license:BSD-3-Clause
// copyright-holders:Fabrice Frances, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "includes/gottlieb.h"
#include "video/resnet.h"


/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

WRITE8_MEMBER(gottlieb_state::gottlieb_paletteram_w)
{
	int r, g, b, a, val;

	m_generic_paletteram_8[offset] = data;

	/* blue & green are encoded in the even bytes */
	val = m_generic_paletteram_8[offset & ~1];
	g = combine_4_weights(m_weights, (val >> 4) & 1, (val >> 5) & 1, (val >> 6) & 1, (val >> 7) & 1);
	b = combine_4_weights(m_weights, (val >> 0) & 1, (val >> 1) & 1, (val >> 2) & 1, (val >> 3) & 1);

	/* red is encoded in the odd bytes */
	val = m_generic_paletteram_8[offset | 1];
	r = combine_4_weights(m_weights, (val >> 0) & 1, (val >> 1) & 1, (val >> 2) & 1, (val >> 3) & 1);

	/* alpha is set to 0 if laserdisc video is enabled */
	a = (m_transparent0 && offset / 2 == 0) ? 0 : 255;
	m_palette->set_pen_color(offset / 2, rgb_t(a, r, g, b));
}



/*************************************
 *
 *  Video controls
 *
 *************************************/

WRITE8_MEMBER(gottlieb_state::gottlieb_video_control_w)
{
	/* bit 0 controls foreground/background priority */
	if (m_background_priority != (data & 0x01))
		m_screen->update_partial(m_screen->vpos());
	m_background_priority = data & 0x01;

	/* bit 1 controls horizontal flip screen */
	flip_screen_x_set(data & 0x02);

	/* bit 2 controls vertical flip screen */
	flip_screen_y_set(data & 0x04);
}


WRITE8_MEMBER(gottlieb_state::gottlieb_laserdisc_video_control_w)
{
	/* bit 0 works like the other games */
	gottlieb_video_control_w(space, offset, data & 0x01);

	/* bit 1 controls the sprite bank. */
	m_spritebank = (data & 0x02) >> 1;

	/* bit 2 video enable (0 = black screen) */
	/* bit 3 genlock control (1 = show laserdisc image) */
	m_laserdisc->overlay_enable((data & 0x04) ? TRUE : FALSE);
	m_laserdisc->video_enable(((data & 0x0c) == 0x0c) ? TRUE : FALSE);

	/* configure the palette if the laserdisc is enabled */
	m_transparent0 = (data >> 3) & 1;
	gottlieb_paletteram_w(space, 0, m_generic_paletteram_8[0]);
}



/*************************************
 *
 *  Video RAM and character RAM access
 *
 *************************************/

WRITE8_MEMBER(gottlieb_state::gottlieb_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(gottlieb_state::gottlieb_charram_w)
{
	if (m_charram[offset] != data)
	{
		m_charram[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty(offset / 32);
	}
}



/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

TILE_GET_INFO_MEMBER(gottlieb_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[tile_index];
	if ((code & 0x80) == 0)
		SET_TILE_INFO_MEMBER(m_gfxcharlo, code, 0, 0);
	else
		SET_TILE_INFO_MEMBER(m_gfxcharhi, code, 0, 0);
}

TILE_GET_INFO_MEMBER(gottlieb_state::get_screwloo_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[tile_index];
	if ((code & 0xc0) == 0)
		SET_TILE_INFO_MEMBER(m_gfxcharlo, code, 0, 0);
	else
		SET_TILE_INFO_MEMBER(m_gfxcharhi, code, 0, 0);
}


void gottlieb_state::video_start()
{
	static const int resistances[4] = { 2000, 1000, 470, 240 };

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the realtive resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, m_weights, 180, 0,
			4, resistances, m_weights, 180, 0,
			4, resistances, m_weights, 180, 0);
	m_transparent0 = FALSE;

	/* configure the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gottlieb_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_source(m_charram);

	/* save some state */
	save_item(NAME(m_background_priority));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_transparent0));
}

VIDEO_START_MEMBER(gottlieb_state,screwloo)
{
	static const int resistances[4] = { 2000, 1000, 470, 240 };

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the realtive resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, m_weights, 180, 0,
			4, resistances, m_weights, 180, 0,
			4, resistances, m_weights, 180, 0);
	m_transparent0 = FALSE;

	/* configure the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gottlieb_state::get_screwloo_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_source(m_charram);

	/* save some state */
	save_item(NAME(m_background_priority));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_transparent0));
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void gottlieb_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	rectangle clip = cliprect;
	int offs;

	/* this is a temporary guess until the sprite hardware is better understood */
	/* there is some additional clipping, but this may not be it */
	clip.min_x = 8;

	for (offs = 0; offs < 256; offs += 4)
	{
		/* coordinates hand tuned to make the position correct in Q*Bert Qubes start */
		/* of level animation. */
		int sx = (spriteram[offs + 1]) - 4;
		int sy = (spriteram[offs]) - 13;
		int code = (255 ^ spriteram[offs + 2]) + 256 * m_spritebank;

		if (flip_screen_x()) sx = 233 - sx;
		if (flip_screen_y()) sy = 228 - sy;


			m_gfxdecode->gfx(2)->transpen(bitmap,clip,
			code, 0,
			flip_screen_x(), flip_screen_y(),
			sx,sy, 0);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 gottlieb_state::screen_update_gottlieb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* if the background has lower priority, render it first, else clear the screen */
	if (!m_background_priority)
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else
		bitmap.fill(m_palette->pen(0), cliprect);

	/* draw the sprites */
	draw_sprites(bitmap, cliprect);

	/* if the background has higher priority, render it now */
	if (m_background_priority)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
