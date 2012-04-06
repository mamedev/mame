/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/trackfld.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Track 'n Field has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( trackfld )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

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
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	/* characters */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}
}

WRITE8_MEMBER(trackfld_state::trackfld_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(trackfld_state::trackfld_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(trackfld_state::trackfld_flipscreen_w)
{
	if (flip_screen_get(machine()) != data)
	{
		flip_screen_set(machine(), data);
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(trackfld_state::atlantol_gfxbank_w)
{
	if (data & 1)
	{
		/* male / female sprites switch */
		if ((m_old_gfx_bank == 1 && (data & 1) == 1) || (m_old_gfx_bank == 0 && (data & 1) == 1))
			m_sprite_bank2 = 0x200;
		else
			m_sprite_bank2 = 0;

		m_sprite_bank1 = 0;
		m_old_gfx_bank = data & 1;
	}
	else
	{
		/* male / female sprites switch */
		if ((m_old_gfx_bank == 0 && (data & 1) == 0) || (m_old_gfx_bank == 1 && (data & 1) == 0))
			m_sprite_bank2 = 0;
		else
			m_sprite_bank2 = 0x200;

		m_sprite_bank1 = 0;
		m_old_gfx_bank = data & 1;
	}

	if ((data & 3) == 3)
	{
		if (m_sprite_bank2)
			m_sprite_bank1 = 0x500;
		else
			m_sprite_bank1 = 0x300;
	}
	else if ((data & 3) == 2)
	{
		if (m_sprite_bank2)
			m_sprite_bank1 = 0x300;
		else
			m_sprite_bank1 = 0x100;
	}

	if (m_bg_bank != (data & 0x8))
	{
		m_bg_bank = data & 0x8;
		m_bg_tilemap->mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	trackfld_state *state = machine.driver_data<trackfld_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + 4 * (attr & 0xc0);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	if (state->m_bg_bank)
		code |= 0x400;

	SET_TILE_INFO(1, code, color, flags);
}

VIDEO_START( trackfld )
{
	trackfld_state *state = machine.driver_data<trackfld_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_bg_tilemap->set_scroll_rows(32);
	state->m_sprites_gfx_banked = 0;
}


VIDEO_START( atlantol )
{
	trackfld_state *state = machine.driver_data<trackfld_state>();
	VIDEO_START_CALL( trackfld );
	state->m_sprites_gfx_banked = 1;
}



static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	trackfld_state *state = machine.driver_data<trackfld_state>();
	UINT8 *spriteram = state->m_spriteram;
	UINT8 *spriteram_2 = state->m_spriteram2;
	int offs;

	for (offs = state->m_spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram_2[offs];
		int code = spriteram[offs + 1];
		int color = attr & 0x0f;
		if (!state->m_sprites_gfx_banked)
			if (attr&1) code|=0x100; // extra tile# bit for the yiear conversion, trackfld doesn't have this many sprites so it will just get masked
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs] - 1;
		int sy = 240 - spriteram_2[offs + 1];

		if (flip_screen_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustement must be done AFTER handling flip screen, thus */
		/* proving that this is a hardware related "feature" */
		sy += 1;

		// to fix the title screen in yieartf it would have to be like this, the same as yiear.c, this should be verified on the hw
		//
		//if (offs < 0x26)
		//{
		//  sy++;   /* fix title screen & garbage at the bottom of the screen */
		//}


		drawgfx_transmask(bitmap, cliprect,
			machine.gfx[0],
			code + state->m_sprite_bank1 + state->m_sprite_bank2, color,
			flipx, flipy,
			sx, sy,
			colortable_get_transpen_mask(machine.colortable, machine.gfx[0], color, 0));

		/* redraw with wraparound */
		drawgfx_transmask(bitmap,cliprect,
			machine.gfx[0],
			code + state->m_sprite_bank1 + state->m_sprite_bank2, color,
			flipx, flipy,
			sx - 256, sy,
			colortable_get_transpen_mask(machine.colortable, machine.gfx[0], color, 0));
	}
}



SCREEN_UPDATE_IND16( trackfld )
{
	trackfld_state *state = screen.machine().driver_data<trackfld_state>();
	int row, scrollx;

	for (row = 0; row < 32; row++)
	{
		scrollx = state->m_scroll[row] + 256 * (state->m_scroll2[row] & 0x01);
		if (flip_screen_get(screen.machine())) scrollx = -scrollx;
		state->m_bg_tilemap->set_scrollx(row, scrollx);
	}

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

