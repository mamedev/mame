/***************************************************************************

    Blue Print

***************************************************************************/

#include "emu.h"
#include "includes/blueprnt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Blue Print doesn't have color PROMs. For sprites, the ROM data is directly
  converted into colors; for characters, it is converted through the color
  code (bits 0-2 = RBG for 01 pixels, bits 3-5 = RBG for 10 pixels, 00 pixels
  always black, 11 pixels use the OR of bits 0-2 and 3-5. Bit 6 is intensity
  control)

***************************************************************************/

void blueprnt_state::palette_init()
{
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		UINT8 pen;
		int r, g, b;

		if (i < 0x200)
			/* characters */
			pen = ((i & 0x100) >> 5) |
				  ((i & 0x002) ? ((i & 0x0e0) >> 5) : 0) |
				  ((i & 0x001) ? ((i & 0x01c) >> 2) : 0);
		else
			/* sprites */
			pen = i - 0x200;

		r = ((pen >> 0) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		g = ((pen >> 2) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		b = ((pen >> 1) & 1) * ((pen & 0x08) ? 0xbf : 0xff);

		palette_set_color(machine(), i, MAKE_RGB(r, g, b));
	}
}

WRITE8_MEMBER(blueprnt_state::blueprnt_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(blueprnt_state::blueprnt_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);

	offset+=32;
	offset &=0x3ff;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(blueprnt_state::blueprnt_flipscreen_w)
{

	flip_screen_set(~data & 0x02);

	if (m_gfx_bank != ((data & 0x04) >> 2))
	{
		m_gfx_bank = ((data & 0x04) >> 2);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(blueprnt_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 256 * m_gfx_bank;
	int color = attr & 0x7f;

	tileinfo.category = (attr & 0x80) ? 1 : 0;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

// really not sure about this but Grasspin doesn't write the tilebank
// or flipscreen after startup...  this certainly doesn't work for 'Saturn'
TILE_GET_INFO_MEMBER(blueprnt_state::get_bg_tile_info_grasspin)
{
	int attr = m_colorram[tile_index] & 0x3f;
	attr |= m_colorram[(tile_index+32)&0x3ff] & 0xc0;  // from the next row?


	int code = m_videoram[tile_index];


	int color = attr & 0x7f;

	tileinfo.category = (attr & 0x80) ? 1 : 0;

	if ((attr & 0x40)) code  += m_gfx_bank * 0x100;
	else code &=0xff;
	
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



VIDEO_START_MEMBER(blueprnt_state,blueprnt)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(blueprnt_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_X, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_gfx_bank));
}

VIDEO_START_MEMBER(blueprnt_state,grasspin)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(blueprnt_state::get_bg_tile_info_grasspin),this), TILEMAP_SCAN_COLS_FLIP_X, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_gfx_bank));
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	blueprnt_state *state = machine.driver_data<blueprnt_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int code = state->m_spriteram[offs + 1];
		int sx = state->m_spriteram[offs + 3];
		int sy = 240 - state->m_spriteram[offs];
		int flipx = state->m_spriteram[offs + 2] & 0x40;
		int flipy = state->m_spriteram[offs + 2 - 4] & 0x80;	// -4? Awkward, isn't it?

		if (state->flip_screen())
		{
			sx = 248 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		// sprites are slightly misplaced, regardless of the screen flip
		drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, 0, flipx, flipy, 2 + sx, sy - 1, 0);
	}
}

UINT32 blueprnt_state::screen_update_blueprnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	if (flip_screen())
		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, m_scrollram[32 - i]);
	else
		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, m_scrollram[30 - i]);

	bitmap.fill(get_black_pen(machine()), cliprect);
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(machine(), bitmap, cliprect);
	m_bg_tilemap->draw(bitmap, cliprect, 1, 0);
	return 0;
}
