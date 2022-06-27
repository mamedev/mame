// license:BSD-3-Clause
// copyright-holders:Fabrice Frances, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "emu.h"
#include "gottlieb.h"
#include "video/resnet.h"


/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

void gottlieb_state::palette_w(offs_t offset, u8 data)
{
	int val;

	m_paletteram[offset] = data;

	/* blue & green are encoded in the even bytes */
	val = m_paletteram[offset & ~1];
	int const g = combine_weights(m_weights, BIT(val, 4), BIT(val, 5), BIT(val, 6), BIT(val, 7));
	int const b = combine_weights(m_weights, BIT(val, 0), BIT(val, 1), BIT(val, 2), BIT(val, 3));

	/* red is encoded in the odd bytes */
	val = m_paletteram[offset | 1];
	int const r = combine_weights(m_weights, BIT(val, 0), BIT(val, 1), BIT(val, 2), BIT(val, 3));

	/* alpha is set to 0 if laserdisc video is enabled */
	int const a = (m_transparent0 && offset / 2 == 0) ? 0 : 255;
	m_palette->set_pen_color(offset / 2, rgb_t(a, r, g, b));
}


/*************************************
 *
 *  Video controls
 *
 *************************************/

void gottlieb_state::video_control_w(u8 data)
{
	/* bit 0 controls foreground/background priority */
	if (m_background_priority != (BIT(data, 0)))
		m_screen->update_partial(m_screen->vpos());
	m_background_priority = BIT(data, 0);

	/* bit 1 controls horizontal flip screen */
	flip_screen_x_set(BIT(data, 1));

	/* bit 2 controls vertical flip screen */
	flip_screen_y_set(BIT(data, 2));
}


void gottlieb_state::laserdisc_video_control_w(u8 data)
{
	/* bit 0 works like the other games */
	video_control_w(BIT(data, 0));

	/* bit 1 controls the sprite bank. */
	m_spritebank = BIT(data, 1);

	/* bit 2 video enable (0 = black screen) */
	/* bit 3 genlock control (1 = show laserdisc image) */
	m_laserdisc->overlay_enable((data & 0x04) ? true : false);
	m_laserdisc->video_enable(((data & 0x0c) == 0x0c) ? true : false);

	/* configure the palette if the laserdisc is enabled */
	m_transparent0 = BIT(data, 3);
	palette_w(0, m_paletteram[0]);
}


/*************************************
 *
 *  Video RAM and character RAM access
 *
 *************************************/

void gottlieb_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void gottlieb_state::charram_w(offs_t offset, u8 data)
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
	int code = m_videoram[tile_index];
	if ((code & 0x80) == 0)
		tileinfo.set(m_gfxcharlo, code, 0, 0);
	else
		tileinfo.set(m_gfxcharhi, code, 0, 0);
}

TILE_GET_INFO_MEMBER(gottlieb_state::get_screwloo_bg_tile_info)
{
	int code = m_videoram[tile_index];
	if ((code & 0xc0) == 0)
		tileinfo.set(m_gfxcharlo, code, 0, 0);
	else
		tileinfo.set(m_gfxcharhi, code, 0, 0);
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
	m_transparent0 = false;

	/* configure the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gottlieb_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);

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
	m_transparent0 = false;

	/* configure the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gottlieb_state::get_screwloo_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);

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
	rectangle clip = cliprect;
	int offs;

	/* this is a temporary guess until the sprite hardware is better understood */
	/* there is some additional clipping, but this may not be it */
	clip.min_x = 8;

	for (offs = 0; offs < 256; offs += 4)
	{
		/* coordinates hand tuned to make the position correct in Q*Bert Qubes start */
		/* of level animation. */
		int sx = (m_spriteram[offs + 1]) - 4;
		int sy = (m_spriteram[offs]) - 13;
		int code = (255 ^ m_spriteram[offs + 2]) + 256 * m_spritebank;

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

uint32_t gottlieb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
