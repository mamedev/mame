/*******************************************************************************

Dr. Micro (c) 1983 Sanritsu

Video hardware
        driver by Uki

*******************************************************************************/

#include "emu.h"
#include "includes/drmicro.h"


/****************************************************************************/

WRITE8_MEMBER(drmicro_state::drmicro_videoram_w)
{
	m_videoram[offset] = data;

	if (offset < 0x800)
		m_bg2->mark_tile_dirty((offset & 0x3ff));
	else
		m_bg1->mark_tile_dirty((offset & 0x3ff));
}


/****************************************************************************/

static TILE_GET_INFO( get_bg1_tile_info )
{
	drmicro_state *state = machine.driver_data<drmicro_state>();
	int code, col, flags;

	code = state->m_videoram[tile_index + 0x0800];
	col = state->m_videoram[tile_index + 0x0c00];

	code += (col & 0xc0) << 2;
	flags = ((col & 0x20) ? TILEMAP_FLIPY : 0) | ((col & 0x10) ? TILEMAP_FLIPX : 0);
	col &= 0x0f;

	SET_TILE_INFO( 0, code, col, flags);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	drmicro_state *state = machine.driver_data<drmicro_state>();
	int code, col, flags;

	code = state->m_videoram[tile_index + 0x0000];
	col = state->m_videoram[tile_index + 0x0400];

	code += (col & 0xc0) << 2;
	flags = ((col & 0x20) ? TILEMAP_FLIPY : 0) | ((col & 0x10) ? TILEMAP_FLIPX : 0);
	col &= 0x0f;

	SET_TILE_INFO( 1, code, col, flags);
}

/****************************************************************************/

PALETTE_INIT( drmicro )
{
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

VIDEO_START( drmicro)
{
	drmicro_state *state = machine.driver_data<drmicro_state>();

	state->m_videoram = auto_alloc_array(machine, UINT8, 0x1000);
	state->save_pointer(NAME(state->m_videoram), 0x1000);

	state->m_bg1 = tilemap_create(machine, get_bg1_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg2 = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bg2->set_transparent_pen(0);
}

SCREEN_UPDATE_IND16( drmicro )
{
	drmicro_state *state = screen.machine().driver_data<drmicro_state>();
	int offs, adr, g;
	int chr, col, attr;
	int x, y, fx, fy;

	state->m_bg1->draw(bitmap, cliprect, 0, 0);
	state->m_bg2->draw(bitmap, cliprect, 0, 0);

	/* draw sprites */
	for (g = 0; g < 2; g++)
	{
		adr = 0x800 * g;

		for (offs = 0x00; offs < 0x20; offs += 4)
		{
			x = state->m_videoram[offs + adr + 3];
			y = state->m_videoram[offs + adr + 0];
			attr = state->m_videoram[offs + adr + 2];
			chr = state->m_videoram[offs + adr + 1];

			fx = (chr & 0x01) ^ state->m_flipscreen;
			fy = ((chr & 0x02) >> 1) ^ state->m_flipscreen;

			chr = (chr >> 2) | (attr & 0xc0);

			col = (attr & 0x0f) + 0x00;

			if (!state->m_flipscreen)
				y = (240 - y) & 0xff;
			else
				x = (240 - x) & 0xff;

			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[3-g],
					chr,
					col,
					fx,fy,
					x,y,0);

			if (x > 240)
			{
				drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[3-g],
						chr,
						col,
						fx,fy,
						x-256,y,0);
			}
		}
	}
	return 0;
}
