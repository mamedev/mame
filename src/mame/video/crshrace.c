#include "emu.h"
#include "video/konicdev.h"
#include "includes/crshrace.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info1 )
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	int code = state->m_videoram1[tile_index];

	SET_TILE_INFO(1, (code & 0xfff) + (state->m_roz_bank << 12), code >> 12, 0);
}

static TILE_GET_INFO( get_tile_info2 )
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	int code = state->m_videoram2[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( crshrace )
{
	crshrace_state *state = machine.driver_data<crshrace_state>();

	state->m_tilemap1 = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_tilemap2 = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 8, 8, 64, 64);

	state->m_tilemap1->set_transparent_pen(0x0f);
	state->m_tilemap2->set_transparent_pen(0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(crshrace_state::crshrace_videoram1_w)
{

	COMBINE_DATA(&m_videoram1[offset]);
	m_tilemap1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crshrace_state::crshrace_videoram2_w)
{

	COMBINE_DATA(&m_videoram2[offset]);
	m_tilemap2->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crshrace_state::crshrace_roz_bank_w)
{

	if (ACCESSING_BITS_0_7)
	{
		if (m_roz_bank != (data & 0xff))
		{
			m_roz_bank = data & 0xff;
			m_tilemap1->mark_all_dirty();
		}
	}
}


WRITE16_MEMBER(crshrace_state::crshrace_gfxctrl_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_gfxctrl = data & 0xdf;
		m_flipscreen = data & 0x20;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	UINT16 *buffered_spriteram = state->m_spriteram->buffer();
	UINT16 *buffered_spriteram_2 = state->m_spriteram2->buffer();
	int offs;

	offs = 0;
	while (offs < 0x0400 && (buffered_spriteram[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
		/* table hand made by looking at the ship explosion in aerofgt attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		attr_start = 4 * (buffered_spriteram[offs++] & 0x03ff);

		ox = buffered_spriteram[attr_start + 1] & 0x01ff;
		xsize = (buffered_spriteram[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (buffered_spriteram[attr_start + 1] & 0xf000) >> 12;
		oy = buffered_spriteram[attr_start + 0] & 0x01ff;
		ysize = (buffered_spriteram[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (buffered_spriteram[attr_start + 0] & 0xf000) >> 12;
		flipx = buffered_spriteram[attr_start + 2] & 0x4000;
		flipy = buffered_spriteram[attr_start + 2] & 0x8000;
		color = (buffered_spriteram[attr_start + 2] & 0x1f00) >> 8;
		map_start = buffered_spriteram[attr_start + 3] & 0x7fff;

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		if (buffered_spriteram[attr_start + 2] & 0x20ff) color = machine.rand();

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				code = buffered_spriteram_2[map_start & 0x7fff];
				map_start++;

				if (state->m_flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							color,
							flipx,flipy,
							sx,sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
			}
		}
	}
}


static void draw_bg( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	state->m_tilemap2->draw(bitmap, cliprect, 0, 0);
}


static void draw_fg(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	crshrace_state *state = machine.driver_data<crshrace_state>();
	k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_tilemap1, 0, 0, 1);
}


SCREEN_UPDATE_IND16( crshrace )
{
	crshrace_state *state = screen.machine().driver_data<crshrace_state>();

	if (state->m_gfxctrl & 0x04)	/* display disable? */
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	bitmap.fill(0x1ff, cliprect);

	switch (state->m_gfxctrl & 0xfb)
	{
		case 0x00:	/* high score screen */
			draw_sprites(screen.machine(), bitmap, cliprect);
			draw_bg(screen.machine(), bitmap, cliprect);
			draw_fg(screen.machine(), bitmap, cliprect);
			break;
		case 0x01:
		case 0x02:
			draw_bg(screen.machine(), bitmap, cliprect);
			draw_fg(screen.machine(), bitmap, cliprect);
			draw_sprites(screen.machine(), bitmap, cliprect);
			break;
		default:
			popmessage("gfxctrl = %02x", state->m_gfxctrl);
			break;
	}
	return 0;
}

SCREEN_VBLANK( crshrace )
{
	crshrace_state *state = screen.machine().driver_data<crshrace_state>();
	state->m_spriteram->vblank_copy_rising(screen, vblank_on);
	state->m_spriteram2->vblank_copy_rising(screen, vblank_on);
}
