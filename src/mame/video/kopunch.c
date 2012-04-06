#include "emu.h"
#include "includes/kopunch.h"


PALETTE_INIT( kopunch )
{
	int i;

	color_prom += 24;	/* first 24 colors are black */
	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

WRITE8_MEMBER(kopunch_state::kopunch_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kopunch_state::kopunch_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kopunch_state::kopunch_scroll_x_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(kopunch_state::kopunch_scroll_y_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE8_MEMBER(kopunch_state::kopunch_gfxbank_w)
{
	if (m_gfxbank != (data & 0x07))
	{
		m_gfxbank = data & 0x07;
		m_bg_tilemap->mark_all_dirty();
	}

	m_bg_tilemap->set_flip((data & 0x08) ? TILEMAP_FLIPY : 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	kopunch_state *state = machine.driver_data<kopunch_state>();
	int code = state->m_videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	kopunch_state *state = machine.driver_data<kopunch_state>();
	int code = (state->m_videoram2[tile_index] & 0x7f) + 128 * state->m_gfxbank;

	SET_TILE_INFO(1, code, 0, 0);
}

VIDEO_START( kopunch )
{
	kopunch_state *state = machine.driver_data<kopunch_state>();
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8,  8, 32, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 16, 16);

	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap->set_scrolldx(16, 16);
}

SCREEN_UPDATE_IND16( kopunch )
{
	kopunch_state *state = screen.machine().driver_data<kopunch_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
