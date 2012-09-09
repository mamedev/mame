/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "emu.h"
#include "includes/homerun.h"


#define half_screen 116

/**************************************************************************/

WRITE8_MEMBER(homerun_state::homerun_scrollhi_w)
{
	// d0: scroll y high bit
	// d1: scroll x high bit
	// other bits: ?
	m_scrolly = (m_scrolly & 0xff) | (data << 8 & 0x100);
	m_scrollx = (m_scrollx & 0xff) | (data << 7 & 0x100);
}

WRITE8_MEMBER(homerun_state::homerun_scrolly_w)
{
	m_scrolly = (m_scrolly & 0xff00) | data;
}

WRITE8_MEMBER(homerun_state::homerun_scrollx_w)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

WRITE8_DEVICE_HANDLER(homerun_banking_w)
{
	homerun_state *state = device->machine().driver_data<homerun_state>();
	if (device->machine().primary_screen->vpos() > half_screen)
		state->m_gc_down = data & 3;
	else
		state->m_gc_up = data & 3;

	state->m_tilemap->mark_all_dirty();

	data >>= 5;
	state->membank("bank1")->set_entry(data & 0x07);
}

WRITE8_MEMBER(homerun_state::homerun_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(homerun_state::homerun_color_w)
{
	int r, g, b;
	int bit0, bit1, bit2;
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = 0;
	bit1 = (data >> 6) & 0x01;
	bit2 = (data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	palette_set_color(machine(), offset, MAKE_RGB(r,g,b));
}


/**************************************************************************/

TILE_GET_INFO_MEMBER(homerun_state::get_homerun_tile_info)
{
	int tileno = (m_videoram[tile_index]) + ((m_videoram[tile_index + 0x1000] & 0x38) << 5) + ((m_gfx_ctrl & 1) << 11);
	int palno = (m_videoram[tile_index + 0x1000] & 0x07);

	SET_TILE_INFO_MEMBER(0, tileno, palno, 0);
}


VIDEO_START( homerun )
{
	homerun_state *state = machine.driver_data<homerun_state>();
	state->m_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(homerun_state::get_homerun_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	homerun_state *state = machine.driver_data<homerun_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram.bytes() - 4; offs >=0; offs -= 4)
	{
		int code, color, sx, sy, flipx, flipy;
		sx = spriteram[offs + 3];
		sy = spriteram[offs + 0] - 16;
		code = (spriteram[offs + 1]) + ((spriteram[offs + 2] & 0x8) << 5) + (state->m_gfx_ctrl << 9);
		color = (spriteram[offs + 2] & 0x7) + 8 ;
		flipx=(spriteram[offs + 2] & 0x40) ;
		flipy=(spriteram[offs + 2] & 0x80) ;
		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16(homerun)
{
	homerun_state *state = screen.machine().driver_data<homerun_state>();
	rectangle myclip = cliprect;

	/* upper part */
	state->m_tilemap->set_scrolly(0, state->m_scrolly);
	state->m_tilemap->set_scrollx(0, state->m_scrollx);

	myclip.max_y /= 2;
	state->m_gfx_ctrl = state->m_gc_up;
	state->m_tilemap->draw(bitmap, myclip, 0, 0);
	draw_sprites(screen.machine(), bitmap, myclip);

	/* lower part */
	myclip.min_y += myclip.max_y;
	myclip.max_y *= 2;
	state->m_gfx_ctrl = state->m_gc_down;
	state->m_tilemap->draw(bitmap, myclip, 0, 0);
	draw_sprites(screen.machine(), bitmap, myclip);

	state->m_gc_down = state->m_gc_up;
	return 0;
}
