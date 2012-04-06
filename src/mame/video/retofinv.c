/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/retofinv.h"


PALETTE_INIT( retofinv )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;


	/* fg chars (1bpp) */
	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry;

		if (i & 0x01)
			ctabentry = i >> 1;
		else
			ctabentry = 0;

		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* sprites and bg tiles */
	for (i = 0; i < 0x800; i++)
	{
		UINT8 ctabentry = BITSWAP8(color_prom[i],4,5,6,7,3,2,1,0);
		colortable_entry_set_value(machine.colortable, i + 0x200, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	row += 2;
	col -= 2;
	if (col  & 0x20)
		return ((col & 0x1f) << 5) + row;
	else
		return (row << 5) + col;
}

static TILE_GET_INFO( bg_get_tile_info )
{
	retofinv_state *state = machine.driver_data<retofinv_state>();
	SET_TILE_INFO(
			2,
			state->m_bg_videoram[tile_index] + 256 * state->m_bg_bank,
			state->m_bg_videoram[0x400 + tile_index] & 0x3f,
			0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	retofinv_state *state = machine.driver_data<retofinv_state>();
	int color = state->m_fg_videoram[0x400 + tile_index];

	tileinfo.group = color;

	SET_TILE_INFO(
			0,
			state->m_fg_videoram[tile_index] + 256 * state->m_fg_bank,
			color,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( retofinv )
{
	retofinv_state *state = machine.driver_data<retofinv_state>();
	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan,8,8,36,28);
	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan,8,8,36,28);

	colortable_configure_tilemap_groups(machine.colortable, state->m_fg_tilemap, machine.gfx[0], 0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(retofinv_state::retofinv_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(retofinv_state::retofinv_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(retofinv_state::retofinv_gfx_ctrl_w)
{
	switch (offset)
	{
		case 0:
			flip_screen_set(machine(), data & 1);
			break;

		case 1:
			if (m_fg_bank != (data & 1))
			{
				m_fg_bank = data & 1;
				m_fg_tilemap->mark_all_dirty();
			}
			break;

		case 2:
			if (m_bg_bank != (data & 1))
			{
				m_bg_bank = data & 1;
				m_bg_tilemap->mark_all_dirty();
			}
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap)
{
	retofinv_state *state = machine.driver_data<retofinv_state>();
	UINT8 *spriteram = state->m_sharedram + 0x0780;
	UINT8 *spriteram_2 = state->m_sharedram + 0x0f80;
	UINT8 *spriteram_3 = state->m_sharedram + 0x1780;
	int offs;
	const rectangle spritevisiblearea(2*8, 34*8-1, 0*8, 28*8-1);

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs];
		int color = spriteram[offs+1] & 0x3f;
		int sx = ((spriteram_2[offs+1] << 1) + ((spriteram_3[offs+1] & 0x80) >> 7)) - 39;
		int sy = 256 - ((spriteram_2[offs] << 1) + ((spriteram_3[offs] & 0x80) >> 7)) + 1;
		/* not sure about the flipping, it's hardly ever used (mostly for shots) */
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizey = (spriteram_3[offs] & 0x04) >> 2;
		int sizex = (spriteram_3[offs] & 0x08) >> 3;
		int x,y;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen_get(machine))
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;	// fix wraparound

		for (y = 0;y <= sizey;y++)
		{
			for (x = 0;x <= sizex;x++)
			{
				drawgfx_transmask(bitmap,spritevisiblearea,machine.gfx[1],
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx,flipy,
					sx + 16*x,sy + 16*y,
					colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0xff));
			}
		}
	}
}



SCREEN_UPDATE_IND16( retofinv )
{
	retofinv_state *state = screen.machine().driver_data<retofinv_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
