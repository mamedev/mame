/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "emu.h"
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
	palette_set_color(machine(), offset / 2, MAKE_ARGB(a, r, g, b));
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
		machine().primary_screen->update_partial(machine().primary_screen->vpos());
	m_background_priority = data & 0x01;

	/* bit 1 controls horizonal flip screen */
	if (flip_screen_x_get(machine()) != (data & 0x02))
	{
		flip_screen_x_set(machine(), data & 0x02);
		machine().tilemap().mark_all_dirty();
	}

	/* bit 2 controls horizonal flip screen */
	if (flip_screen_y_get(machine()) != (data & 0x04))
	{
		flip_screen_y_set(machine(), data & 0x04);
		machine().tilemap().mark_all_dirty();
	}

	/* in Q*Bert Qubes only, bit 4 controls the sprite bank */
	m_spritebank = (data & 0x10) >> 4;
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
		gfx_element_mark_dirty(machine().gfx[0], offset / 32);
	}
}



/*************************************
 *
 *  Palette RAM writes
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index];
	if ((code & 0x80) == 0)
		SET_TILE_INFO(state->m_gfxcharlo, code, 0, 0);
	else
		SET_TILE_INFO(state->m_gfxcharhi, code, 0, 0);
}

static TILE_GET_INFO( get_screwloo_bg_tile_info )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index];
	if ((code & 0xc0) == 0)
		SET_TILE_INFO(state->m_gfxcharlo, code, 0, 0);
	else
		SET_TILE_INFO(state->m_gfxcharhi, code, 0, 0);
}


VIDEO_START( gottlieb )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	static const int resistances[4] = { 2000, 1000, 470, 240 };

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the realtive resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, state->m_weights, 180, 0,
			4, resistances, state->m_weights, 180, 0,
			4, resistances, state->m_weights, 180, 0);
	state->m_transparent0 = FALSE;

	/* configure the background tilemap */
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(0, 318 - 256);

	gfx_element_set_source(machine.gfx[0], state->m_charram);

	/* save some state */
	state_save_register_global(machine, state->m_background_priority);
	state_save_register_global(machine, state->m_spritebank);
	state_save_register_global(machine, state->m_transparent0);
}

VIDEO_START( screwloo )
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	static const int resistances[4] = { 2000, 1000, 470, 240 };

	/* compute palette information */
	/* note that there really are pullup/pulldown resistors, but this situation is complicated */
	/* by the use of transistors, so we ignore that and just use the realtive resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, state->m_weights, 180, 0,
			4, resistances, state->m_weights, 180, 0,
			4, resistances, state->m_weights, 180, 0);
	state->m_transparent0 = FALSE;

	/* configure the background tilemap */
	state->m_bg_tilemap = tilemap_create(machine, get_screwloo_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(0, 318 - 256);

	gfx_element_set_source(machine.gfx[0], state->m_charram);

	/* save some state */
	state_save_register_global(machine, state->m_background_priority);
	state_save_register_global(machine, state->m_spritebank);
	state_save_register_global(machine, state->m_transparent0);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gottlieb_state *state = machine.driver_data<gottlieb_state>();
	UINT8 *spriteram = state->m_spriteram;
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
		int code = (255 ^ spriteram[offs + 2]) + 256 * state->m_spritebank;

		if (flip_screen_x_get(machine)) sx = 233 - sx;
		if (flip_screen_y_get(machine)) sy = 244 - sy;

		drawgfx_transpen(bitmap, clip,
			machine.gfx[2],
			code, 0,
			flip_screen_x_get(machine), flip_screen_y_get(machine),
			sx,sy, 0);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_RGB32( gottlieb )
{
	gottlieb_state *state = screen.machine().driver_data<gottlieb_state>();
	/* if the background has lower priority, render it first, else clear the screen */
	if (!state->m_background_priority)
		state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else
		bitmap.fill(screen.machine().pens[0], cliprect);

	/* draw the sprites */
	draw_sprites(screen.machine(), bitmap, cliprect);

	/* if the background has higher priority, render it now */
	if (state->m_background_priority)
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
