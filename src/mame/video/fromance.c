/******************************************************************************

    Video Hardware for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

******************************************************************************/

#include "emu.h"
#include "includes/fromance.h"


static TIMER_CALLBACK( crtc_interrupt_gen );

/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

INLINE void get_fromance_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int layer )
{
	fromance_state *state = machine.driver_data<fromance_state>();
	int tile = ((state->m_local_videoram[layer][0x0000 + tile_index] & 0x80) << 9) |
				(state->m_local_videoram[layer][0x1000 + tile_index] << 8) |
				state->m_local_videoram[layer][0x2000 + tile_index];
	int color = state->m_local_videoram[layer][tile_index] & 0x7f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_fromance_bg_tile_info ) { get_fromance_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_fromance_fg_tile_info ) { get_fromance_tile_info(machine, tileinfo, tile_index, 1); }


INLINE void get_nekkyoku_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int layer )
{
	fromance_state *state = machine.driver_data<fromance_state>();
	int tile = (state->m_local_videoram[layer][0x0000 + tile_index] << 8) |
				state->m_local_videoram[layer][0x1000 + tile_index];
	int color = state->m_local_videoram[layer][tile_index + 0x2000] & 0x3f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_nekkyoku_bg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_nekkyoku_fg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 1); }



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void init_common( running_machine &machine )
{
	fromance_state *state = machine.driver_data<fromance_state>();

	/* allocate local videoram */
	state->m_local_videoram[0] = auto_alloc_array(machine, UINT8, 0x1000 * 3);
	state->m_local_videoram[1] = auto_alloc_array(machine, UINT8, 0x1000 * 3);

	/* allocate local palette RAM */
	state->m_local_paletteram = auto_alloc_array(machine, UINT8, 0x800 * 2);

	/* configure tilemaps */
	state->m_fg_tilemap->set_transparent_pen(15);

	/* reset the timer */
	state->m_crtc_timer = machine.scheduler().timer_alloc(FUNC(crtc_interrupt_gen));

	/* state save */
	state->save_item(NAME(state->m_selected_videoram));
	state->save_pointer(NAME(state->m_local_videoram[0]), 0x1000 * 3);
	state->save_pointer(NAME(state->m_local_videoram[1]), 0x1000 * 3);
	state->save_item(NAME(state->m_selected_paletteram));
	state->save_item(NAME(state->m_scrollx));
	state->save_item(NAME(state->m_scrolly));
	state->save_item(NAME(state->m_gfxreg));
	state->save_item(NAME(state->m_flipscreen));
	state->save_item(NAME(state->m_flipscreen_old));
	state->save_item(NAME(state->m_scrollx_ofs));
	state->save_item(NAME(state->m_scrolly_ofs));
	state->save_item(NAME(state->m_crtc_register));
	state->save_item(NAME(state->m_crtc_data));
	state->save_pointer(NAME(state->m_local_paletteram), 0x800 * 2);
}

VIDEO_START( fromance )
{
	fromance_state *state = machine.driver_data<fromance_state>();

	/* allocate tilemaps */
	state->m_bg_tilemap = tilemap_create(machine, get_fromance_bg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, get_fromance_fg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);

	init_common(machine);
}

VIDEO_START( nekkyoku )
{
	fromance_state *state = machine.driver_data<fromance_state>();

	/* allocate tilemaps */
	state->m_bg_tilemap = tilemap_create(machine, get_nekkyoku_bg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, get_nekkyoku_fg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);

	init_common(machine);
}

VIDEO_START( pipedrm )
{
	fromance_state *state = machine.driver_data<fromance_state>();

	VIDEO_START_CALL(fromance);
	state->m_scrolly_ofs = 0x00;
}

VIDEO_START( hatris )
{
	fromance_state *state = machine.driver_data<fromance_state>();

	VIDEO_START_CALL(fromance);
	state->m_scrollx_ofs = 0xB9;
	state->m_scrolly_ofs = 0x00;
}

/*************************************
 *
 *  Graphics control register
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_gfxreg_w)
{

	m_gfxreg = data;
	m_flipscreen = (data & 0x01);
	m_selected_videoram = (~data >> 1) & 1;
	m_selected_paletteram = (data >> 6) & 1;

	if (m_flipscreen != m_flipscreen_old)
	{
		m_flipscreen_old = m_flipscreen;
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  Banked palette RAM
 *
 *************************************/

READ8_MEMBER(fromance_state::fromance_paletteram_r)
{

	/* adjust for banking and read */
	offset |= m_selected_paletteram << 11;
	return m_local_paletteram[offset];
}


WRITE8_MEMBER(fromance_state::fromance_paletteram_w)
{
	int palword;

	/* adjust for banking and modify */
	offset |= m_selected_paletteram << 11;
	m_local_paletteram[offset] = data;

	/* compute R,G,B */
	palword = (m_local_paletteram[offset | 1] << 8) | m_local_paletteram[offset & ~1];
	palette_set_color_rgb(machine(), offset / 2, pal5bit(palword >> 10), pal5bit(palword >> 5), pal5bit(palword >> 0));
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

READ8_MEMBER(fromance_state::fromance_videoram_r)
{
	return m_local_videoram[m_selected_videoram][offset];
}


WRITE8_MEMBER(fromance_state::fromance_videoram_w)
{
	m_local_videoram[m_selected_videoram][offset] = data;
	(m_selected_videoram ? m_fg_tilemap : m_bg_tilemap)->mark_tile_dirty(offset & 0x0fff);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_MEMBER(fromance_state::fromance_scroll_w)
{
	if (m_flipscreen)
	{
		switch (offset)
		{
			case 0:
				m_scrollx[1] = (data + (((m_gfxreg & 0x08) >> 3) * 0x100) - m_scrollx_ofs);
				break;
			case 1:
				m_scrolly[1] = (data + (((m_gfxreg & 0x04) >> 2) * 0x100) - m_scrolly_ofs); // - 0x10
				break;
			case 2:
				m_scrollx[0] = (data + (((m_gfxreg & 0x20) >> 5) * 0x100) - m_scrollx_ofs);
				break;
			case 3:
				m_scrolly[0] = (data + (((m_gfxreg & 0x10) >> 4) * 0x100) - m_scrolly_ofs);
				break;
		}
	}
	else
	{
		switch (offset)
		{
			case 0:
				m_scrollx[1] = (data + (((m_gfxreg & 0x08) >> 3) * 0x100) - 0x1f7);
				break;
			case 1:
				m_scrolly[1] = (data + (((m_gfxreg & 0x04) >> 2) * 0x100) - 0xf9);
				break;
			case 2:
				m_scrollx[0] = (data + (((m_gfxreg & 0x20) >> 5) * 0x100) - 0x1f7);
				break;
			case 3:
				m_scrolly[0] = (data + (((m_gfxreg & 0x10) >> 4) * 0x100) - 0xf9);
				break;
		}
	}
}



/*************************************
 *
 *  Fake video controller
 *
 *************************************/

static TIMER_CALLBACK( crtc_interrupt_gen )
{
	fromance_state *state = machine.driver_data<fromance_state>();
	device_set_input_line(state->m_subcpu, 0, HOLD_LINE);
	if (param != 0)
		state->m_crtc_timer->adjust(machine.primary_screen->frame_period() / param, 0, machine.primary_screen->frame_period() / param);
}


WRITE8_MEMBER(fromance_state::fromance_crtc_data_w)
{
	m_crtc_data[m_crtc_register] = data;

	switch (m_crtc_register)
	{
		/* only register we know about.... */
		case 0x0b:
			m_crtc_timer->adjust(machine().primary_screen->time_until_vblank_start(), (data > 0x80) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", m_crtc_register, data & 0xff);
			break;
	}
}


WRITE8_MEMBER(fromance_state::fromance_crtc_register_w)
{
	m_crtc_register = data;
}



/*************************************
 *
 *  Sprite routines (Pipe Dream)
 *
 *************************************/

static void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int draw_priority )
{
	fromance_state *state = screen.machine().driver_data<fromance_state>();
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	const rectangle &visarea = screen.visible_area();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < state->m_spriteram_size; offs += 8)
	{
		int data2 = spriteram[offs + 4] | (spriteram[offs + 5] << 8);
		int priority = (data2 >> 4) & 1;

		/* turns out the sprites are the same as in aerofgt.c */
		if ((data2 & 0x80) && priority == draw_priority)
		{
			int data0 = spriteram[offs + 0] | (spriteram[offs + 1] << 8);
			int data1 = spriteram[offs + 2] | (spriteram[offs + 3] << 8);
			int data3 = spriteram[offs + 6] | (spriteram[offs + 7] << 8);
			int code = data3 & 0xfff;
			int color = data2 & 0x0f;
			int y = (data0 & 0x1ff) - 6;
			int x = (data1 & 0x1ff) - 13;
			int yzoom = (data0 >> 12) & 15;
			int xzoom = (data1 >> 12) & 15;
			int zoomed = (xzoom | yzoom);
			int ytiles = ((data2 >> 12) & 7) + 1;
			int xtiles = ((data2 >> 8) & 7) + 1;
			int yflip = (data2 >> 15) & 1;
			int xflip = (data2 >> 11) & 1;
			int xt, yt;

			/* compute the zoom factor -- stolen from aerofgt.c */
			xzoom = 16 - zoomtable[xzoom] / 8;
			yzoom = 16 - zoomtable[yzoom] / 8;

			/* wrap around */
			if (x > visarea.max_x)
				x -= 0x200;
			if (y > visarea.max_y)
				y -= 0x200;

			/* flip ? */
			if (state->m_flipscreen)
			{
				y = visarea.max_y - y - 16 * ytiles - 4;
				x = visarea.max_x - x - 16 * xtiles - 24;
				xflip=!xflip;
				yflip=!yflip;
			}

			/* normal case */
			if (!xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 0,
									x + xt * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 0,
									x + xt * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* xflipped case */
			else if (xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* yflipped case */
			else if (!xflip && yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 1,
									x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 0, 1,
									x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* x & yflipped case */
			else
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine().gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}
		}
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( fromance )
{
	fromance_state *state = screen.machine().driver_data<fromance_state>();

	state->m_bg_tilemap->set_scrollx(0, state->m_scrollx[0]);
	state->m_bg_tilemap->set_scrolly(0, state->m_scrolly[0]);
	state->m_fg_tilemap->set_scrollx(0, state->m_scrollx[1]);
	state->m_fg_tilemap->set_scrolly(0, state->m_scrolly[1]);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( pipedrm )
{
	fromance_state *state = screen.machine().driver_data<fromance_state>();

	/* there seems to be no logical mapping for the X scroll register -- maybe it's gone */
	state->m_bg_tilemap->set_scrolly(0, state->m_scrolly[1]);
	state->m_fg_tilemap->set_scrolly(0, state->m_scrolly[0]);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen, bitmap, cliprect, 0);
	draw_sprites(screen, bitmap, cliprect, 1);
	return 0;
}
