#include "emu.h"
#include "includes/cbasebal.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	cbasebal_state *state = machine.driver_data<cbasebal_state>();
	UINT8 attr = state->m_scrollram[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			state->m_scrollram[2 * tile_index] + ((attr & 0x07) << 8) + 0x800 * state->m_tilebank,
			(attr & 0xf0) >> 4,
			(attr & 0x08) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	cbasebal_state *state = machine.driver_data<cbasebal_state>();
	UINT8 attr = state->m_textram[tile_index + 0x800];
	SET_TILE_INFO(
			0,
			state->m_textram[tile_index] + ((attr & 0xf0) << 4),
			attr & 0x07,
			(attr & 0x08) ? TILE_FLIPX : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( cbasebal )
{
	cbasebal_state *state = machine.driver_data<cbasebal_state>();

	state->m_textram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_scrollram = auto_alloc_array(machine, UINT8, 0x1000);

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(3);

	state->save_pointer(NAME(state->m_textram), 0x1000);
	state->save_pointer(NAME(state->m_scrollram), 0x1000);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(cbasebal_state::cbasebal_textram_w)
{

	m_textram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

READ8_MEMBER(cbasebal_state::cbasebal_textram_r)
{
	return m_textram[offset];
}

WRITE8_MEMBER(cbasebal_state::cbasebal_scrollram_w)
{

	m_scrollram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

READ8_MEMBER(cbasebal_state::cbasebal_scrollram_r)
{
	return m_scrollram[offset];
}

WRITE8_MEMBER(cbasebal_state::cbasebal_gfxctrl_w)
{

	/* bit 0 is unknown - toggles continuously */

	/* bit 1 is flip screen */
	m_flipscreen = data & 0x02;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* bit 2 is unknown - unused? */

	/* bit 3 is tile bank */
	if (m_tilebank != ((data & 0x08) >> 3))
	{
		m_tilebank = (data & 0x08) >> 3;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 4 is sprite bank */
	m_spritebank = (data & 0x10) >> 4;

	/* bits 5 is text enable */
	m_text_on = ~data & 0x20;

	/* bits 6-7 are bg/sprite enable (don't know which is which) */
	m_bg_on = ~data & 0x40;
	m_obj_on = ~data & 0x80;

	/* other bits unknown, but used */
}

WRITE8_MEMBER(cbasebal_state::cbasebal_scrollx_w)
{
	m_scroll_x[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll_x[0] + 256 * m_scroll_x[1]);
}

WRITE8_MEMBER(cbasebal_state::cbasebal_scrolly_w)
{
	m_scroll_y[offset] = data;
	m_bg_tilemap->set_scrolly(0, m_scroll_y[0] + 256 * m_scroll_y[1]);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	cbasebal_state *state = machine.driver_data<cbasebal_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs, sx, sy;

	/* the last entry is not a sprite, we skip it otherwise spang shows a bubble */
	/* moving diagonally across the screen */
	for (offs = state->m_spriteram_size - 8; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs];
		int attr = spriteram[offs + 1];
		int color = attr & 0x07;
		int flipx = attr & 0x08;
		sx = spriteram[offs + 3] + ((attr & 0x10) << 4);
		sy = ((spriteram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;
		code += state->m_spritebank * 0x800;

		if (state->m_flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
				code,
				color,
				flipx,state->m_flipscreen,
				sx,sy,15);
	}
}

SCREEN_UPDATE_IND16( cbasebal )
{
	cbasebal_state *state = screen.machine().driver_data<cbasebal_state>();

	if (state->m_bg_on)
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	else
		bitmap.fill(768, cliprect);

	if (state->m_obj_on)
		draw_sprites(screen.machine(), bitmap, cliprect);

	if (state->m_text_on)
		state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
