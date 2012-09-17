/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/vulgus.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void vulgus_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	machine().colortable = colortable_alloc(machine(), 256);

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[256] >> 0) & 0x01;
		bit1 = (color_prom[256] >> 1) & 0x01;
		bit2 = (color_prom[256] >> 2) & 0x01;
		bit3 = (color_prom[256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*256] >> 0) & 0x01;
		bit1 = (color_prom[2*256] >> 1) & 0x01;
		bit2 = (color_prom[2*256] >> 2) & 0x01;
		bit3 = (color_prom[2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(machine().colortable,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*256;
	/* color_prom now points to the beginning of the lookup table */


	/* characters use colors 32-47 (?) */
	for (i = 0;i < machine().gfx[0]->colors() * machine().gfx[0]->granularity();i++)
		colortable_entry_set_value(machine().colortable, machine().gfx[0]->colorbase() + i, 32 + *color_prom++);

	/* sprites use colors 16-31 */
	for (i = 0;i < machine().gfx[2]->colors() * machine().gfx[2]->granularity();i++)
		colortable_entry_set_value(machine().colortable, machine().gfx[2]->colorbase() + i, 16 + *color_prom++);

	/* background tiles use colors 0-15, 64-79, 128-143, 192-207 in four banks */
	for (i = 0;i < machine().gfx[1]->colors() * machine().gfx[1]->granularity() / 4;i++)
	{
		colortable_entry_set_value(machine().colortable, machine().gfx[1]->colorbase() + 0*32*8 + i, *color_prom);
		colortable_entry_set_value(machine().colortable, machine().gfx[1]->colorbase() + 1*32*8 + i, *color_prom + 64);
		colortable_entry_set_value(machine().colortable, machine().gfx[1]->colorbase() + 2*32*8 + i, *color_prom + 128);
		colortable_entry_set_value(machine().colortable, machine().gfx[1]->colorbase() + 3*32*8 + i, *color_prom + 192);
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(vulgus_state::get_fg_tile_info)
{
	int code, color;

	code = m_fgvideoram[tile_index];
	color = m_fgvideoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(
			0,
			code + ((color & 0x80) << 1),
			color & 0x3f,
			0);
	tileinfo.group = color & 0x3f;
}

TILE_GET_INFO_MEMBER(vulgus_state::get_bg_tile_info)
{
	int code, color;

	code = m_bgvideoram[tile_index];
	color = m_bgvideoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(
			1,
			code + ((color & 0x80) << 1),
			(color & 0x1f) + (0x20 * m_palette_bank),
			TILE_FLIPYX((color & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void vulgus_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(vulgus_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(vulgus_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,16,16,32,32);

	colortable_configure_tilemap_groups(machine().colortable, m_fg_tilemap, machine().gfx[0], 47);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(vulgus_state::vulgus_fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(vulgus_state::vulgus_bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


WRITE8_MEMBER(vulgus_state::vulgus_c804_w)
{
	/* bits 0 and 1 are coin counters */
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);

	/* bit 7 flips screen */
	flip_screen_set(data & 0x80);
}


WRITE8_MEMBER(vulgus_state::vulgus_palette_bank_w)
{
	if (m_palette_bank != (data & 3))
	{
		m_palette_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	vulgus_state *state = machine.driver_data<vulgus_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;


	for (offs = state->m_spriteram.bytes() - 4;offs >= 0;offs -= 4)
	{
		int code,i,col,sx,sy,dir;


		code = spriteram[offs];
		col = spriteram[offs + 1] & 0x0f;
		sx = spriteram[offs + 3];
		sy = spriteram[offs + 2];
		dir = 1;
		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			dir = -1;
		}

		i = (spriteram[offs + 1] & 0xc0) >> 6;
		if (i == 2) i = 3;

		do
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
					code + i,
					col,
					state->flip_screen(),state->flip_screen(),
					sx, sy + 16 * i * dir,15);

			/* draw again with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
					code + i,
					col,
					state->flip_screen(),state->flip_screen(),
					sx, sy + 16 * i * dir -  dir * 256,15);
			i--;
		} while (i >= 0);
	}
}

UINT32 vulgus_state::screen_update_vulgus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scroll_low[1] + 256 * m_scroll_high[1]);
	m_bg_tilemap->set_scrolly(0, m_scroll_low[0] + 256 * m_scroll_high[0]);

	m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
