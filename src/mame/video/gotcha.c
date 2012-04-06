#include "emu.h"
#include "includes/gotcha.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( gotcha_tilemap_scan )
{
	return (col & 0x1f) | (row << 5) | ((col & 0x20) << 5);
}

INLINE void get_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index ,UINT16 *vram, int color_offs)
{
	gotcha_state *state = machine.driver_data<gotcha_state>();
	UINT16 data = vram[tile_index];
	int code = (data & 0x3ff) | (state->m_gfxbank[(data & 0x0c00) >> 10] << 10);

	SET_TILE_INFO(0, code, (data >> 12) + color_offs, 0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	gotcha_state *state = machine.driver_data<gotcha_state>();
	get_tile_info(machine, tileinfo, tile_index, state->m_fgvideoram, 0);
}

static TILE_GET_INFO( bg_get_tile_info )
{
	gotcha_state *state = machine.driver_data<gotcha_state>();
	get_tile_info(machine, tileinfo, tile_index, state->m_bgvideoram, 16);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gotcha )
{
	gotcha_state *state = machine.driver_data<gotcha_state>();
	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info, gotcha_tilemap_scan, 16, 16, 64, 32);
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info, gotcha_tilemap_scan, 16, 16, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_fg_tilemap->set_scrolldx(-1, 0);
	state->m_bg_tilemap->set_scrolldx(-5, 0);
}


WRITE16_MEMBER(gotcha_state::gotcha_fgvideoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gotcha_state::gotcha_bgvideoram_w)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(gotcha_state::gotcha_gfxbank_select_w)
{
	if (ACCESSING_BITS_8_15)
		m_banksel = (data & 0x0300) >> 8;
}

WRITE16_MEMBER(gotcha_state::gotcha_gfxbank_w)
{
	if (ACCESSING_BITS_8_15)
	{
		if (m_gfxbank[m_banksel] != ((data & 0x0f00) >> 8))
		{
			m_gfxbank[m_banksel] = (data & 0x0f00) >> 8;
			machine().tilemap().mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(gotcha_state::gotcha_scroll_w)
{
	COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrollx(0, m_scroll[0]); break;
		case 1: m_fg_tilemap->set_scrolly(0, m_scroll[1]); break;
		case 2: m_bg_tilemap->set_scrollx(0, m_scroll[2]); break;
		case 3: m_bg_tilemap->set_scrolly(0, m_scroll[3]); break;
	}
}




static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gotcha_state *state = machine.driver_data<gotcha_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 4)
	{
		int sx, sy, code, color, flipx, flipy, height, y;

		sx = spriteram[offs + 2];
		sy = spriteram[offs + 0];
		code = spriteram[offs + 1];
		color = spriteram[offs + 2] >> 9;
		height = 1 << ((spriteram[offs + 0] & 0x0600) >> 9);
		flipx = spriteram[offs + 0] & 0x2000;
		flipy = spriteram[offs + 0] & 0x4000;

		for (y = 0; y < height; y++)
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					code + (flipy ? height-1 - y : y),
					color,
					flipx,flipy,
					0x140-5 - ((sx + 0x10) & 0x1ff),0x100+1 - ((sy + 0x10 * (height - y)) & 0x1ff),0);
		}
	}
}


SCREEN_UPDATE_IND16( gotcha )
{
	gotcha_state *state = screen.machine().driver_data<gotcha_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
