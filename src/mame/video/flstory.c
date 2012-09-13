/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/flstory.h"

TILE_GET_INFO_MEMBER(flstory_state::get_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	int flags = TILE_FLIPYX((attr & 0x18) >> 3);
	tileinfo.category = (attr & 0x20) >> 5;
	tileinfo.group = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(
			0,
			tile_number,
			attr & 0x0f,
			flags);
}

TILE_GET_INFO_MEMBER(flstory_state::victnine_get_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = ((attr & 0x38) << 5) + code;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(
			0,
			tile_number,
			attr & 0x07,
			flags);
}

TILE_GET_INFO_MEMBER(flstory_state::get_rumba_tile_info)
{
	int code = m_videoram[tile_index * 2];
	int attr = m_videoram[tile_index * 2 + 1];
	int tile_number = code + ((attr & 0xc0) << 2) + 0x400 + 0x800 * m_char_bank;
	int col = (attr & 0x0f);

	tileinfo.category = (attr & 0x20) >> 5;
	tileinfo.group = (attr & 0x20) >> 5;
	SET_TILE_INFO_MEMBER(
			0,
			tile_number,
			col,
			0);
}

VIDEO_START_MEMBER(flstory_state,flstory)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(flstory_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_generic_paletteram_8.allocate(0x200);
	m_generic_paletteram2_8.allocate(0x200);
}

VIDEO_START_MEMBER(flstory_state,rumba)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(flstory_state::get_rumba_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_bg_tilemap->set_transparent_pen(15);
	m_bg_tilemap->set_transmask(0, 0x3fff, 0xc000); /* split type 0 has pens 0-13 transparent in front half */
	m_bg_tilemap->set_transmask(1, 0x8000, 0x7fff); /* split type 1 has pen 15 transparent in front half */
	m_bg_tilemap->set_scroll_cols(32);

	m_generic_paletteram_8.allocate(0x200);
	m_generic_paletteram2_8.allocate(0x200);
}

VIDEO_START_MEMBER(flstory_state,victnine)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(flstory_state::victnine_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	m_generic_paletteram_8.allocate(0x200);
	m_generic_paletteram2_8.allocate(0x200);
}

WRITE8_MEMBER(flstory_state::flstory_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(flstory_state::flstory_palette_w)
{
	if (offset & 0x100)
		paletteram_xxxxBBBBGGGGRRRR_byte_split_hi_w(space, (offset & 0xff) + (m_palette_bank << 8),data);
	else
		paletteram_xxxxBBBBGGGGRRRR_byte_split_lo_w(space, (offset & 0xff) + (m_palette_bank << 8),data);
}

READ8_MEMBER(flstory_state::flstory_palette_r)
{
	if (offset & 0x100)
		return m_generic_paletteram2_8[ (offset & 0xff) + (m_palette_bank << 8) ];
	else
		return m_generic_paletteram_8  [ (offset & 0xff) + (m_palette_bank << 8) ];
}

WRITE8_MEMBER(flstory_state::flstory_gfxctrl_w)
{
	if (m_gfxctrl == data)
		return;
	m_gfxctrl = data;

	m_flipscreen = (~data & 0x01);
	if (m_char_bank != ((data & 0x10) >> 4))
	{
		m_char_bank = (data & 0x10) >> 4;
		m_bg_tilemap->mark_all_dirty();
	}
	m_palette_bank = (data & 0x20) >> 5;

	flip_screen_set(m_flipscreen);

//popmessage("%04x: gfxctrl = %02x\n", space.device().safe_pc(), data);

}

READ8_MEMBER(flstory_state::victnine_gfxctrl_r)
{
	return m_gfxctrl;
}

WRITE8_MEMBER(flstory_state::victnine_gfxctrl_w)
{
	if (m_gfxctrl == data)
		return;
	m_gfxctrl = data;

	m_palette_bank = (data & 0x20) >> 5;

	if (data & 0x04)
	{
		m_flipscreen = (data & 0x01);
		flip_screen_set(m_flipscreen);
	}

//popmessage("%04x: gfxctrl = %02x\n", space.device().safe_pc(), data);

}

WRITE8_MEMBER(flstory_state::flstory_scrlram_w)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}


static void flstory_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	flstory_state *state = machine.driver_data<flstory_state>();
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->m_spriteram[state->m_spriteram.bytes() - 1 - i];
		int offs = (pr & 0x1f) * 4;

		if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = state->m_spriteram[offs + 2] + ((state->m_spriteram[offs + 1] & 0x30) << 4);
			sx = state->m_spriteram[offs + 3];
			sy = state->m_spriteram[offs + 0];

			if (state->m_flipscreen)
			{
				sx = (240 - sx) & 0xff ;
				sy = sy - 1 ;
			}
			else
				sy = 240 - sy - 1 ;

			flipx = ((state->m_spriteram[offs + 1] & 0x40) >> 6) ^ state->m_flipscreen;
			flipy = ((state->m_spriteram[offs + 1] & 0x80) >> 7) ^ state->m_flipscreen;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					code,
					state->m_spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
						code,
						state->m_spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

SCREEN_UPDATE_IND16( flstory )
{
	flstory_state *state = screen.machine().driver_data<flstory_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);
	flstory_draw_sprites(screen.machine(), bitmap, cliprect, 0x00);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	flstory_draw_sprites(screen.machine(), bitmap, cliprect, 0x80);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}

static void victnine_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	flstory_state *state = machine.driver_data<flstory_state>();
	int i;

	for (i = 0; i < 0x20; i++)
	{
		int pr = state->m_spriteram[state->m_spriteram.bytes() - 1 - i];
		int offs = (pr & 0x1f) * 4;

		//if ((pr & 0x80) == pri)
		{
			int code, sx, sy, flipx, flipy;

			code = state->m_spriteram[offs + 2] + ((state->m_spriteram[offs + 1] & 0x20) << 3);
			sx = state->m_spriteram[offs + 3];
			sy = state->m_spriteram[offs + 0];

			if (state->m_flipscreen)
			{
				sx = (240 - sx + 1) & 0xff ;
				sy = sy + 1 ;
			}
			else
				sy = 240 - sy + 1 ;

			flipx = ((state->m_spriteram[offs + 1] & 0x40) >> 6) ^ state->m_flipscreen;
			flipy = ((state->m_spriteram[offs + 1] & 0x80) >> 7) ^ state->m_flipscreen;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					code,
					state->m_spriteram[offs + 1] & 0x0f,
					flipx,flipy,
					sx,sy,15);
			/* wrap around */
			if (sx > 240)
				drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
						code,
						state->m_spriteram[offs + 1] & 0x0f,
						flipx,flipy,
						sx-256,sy,15);
		}
	}
}

SCREEN_UPDATE_IND16( victnine )
{
	flstory_state *state = screen.machine().driver_data<flstory_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	victnine_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( rumba )
{
	flstory_state *state = screen.machine().driver_data<flstory_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);
	victnine_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	victnine_draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
