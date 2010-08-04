/***************************************************************************

    Atari G42 hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank

****************************************************************************/


#include "emu.h"
#include "machine/atarigen.h"
#include "includes/atarig42.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarig42_state *state = machine->driver_data<atarig42_state>();
	UINT16 data = state->alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarig42_state *state = machine->driver_data<atarig42_state>();
	UINT16 data = state->playfield[tile_index];
	int code = (state->playfield_tile_bank << 12) | (data & 0xfff);
	int color = (state->playfield_base >> 5) + ((state->playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
	tileinfo->category = (state->playfield_color_bank >> 2) & 7;
}


static TILEMAP_MAPPER( atarig42_playfield_scan )
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig42 )
{
	static const atarirle_desc modesc =
	{
		"gfx3",		/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		0,			/* left clip coordinate */
		0,			/* right clip coordinate */

		0x000,		/* base palette entry */
		0x400,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x03f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0,0x00ff,0 }},	/* mask for the order */
		{{ 0,0x0e00,0,0,0,0,0,0 }},	/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};
	atarig42_state *state = machine->driver_data<atarig42_state>();
	atarirle_desc adjusted_modesc = modesc;
	int i;

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x30);

	/* initialize the playfield */
	state->playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, atarig42_playfield_scan,  8,8, 128,64);

	/* initialize the motion objects */
	adjusted_modesc.palettebase = state->motion_object_base;
	for (i = 0; i < 8; i++)
		adjusted_modesc.colormask.data[i] &= state->motion_object_mask;
	atarirle_init(machine, 0, &adjusted_modesc);

	/* initialize the alphanumerics */
	state->alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->alpha_tilemap, 0);

	/* save states */
	state_save_register_global(machine, state->current_control);
	state_save_register_global(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->playfield_color_bank);
	state_save_register_global(machine, state->playfield_xscroll);
	state_save_register_global(machine, state->playfield_yscroll);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig42_mo_control_w )
{
	atarig42_state *state = space->machine->driver_data<atarig42_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space->machine->primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->current_control);
}


void atarig42_scanline_update(screen_device &screen, int scanline)
{
	atarig42_state *state = screen.machine->driver_data<atarig42_state>();
	UINT16 *base = &state->alpha[(scanline / 8) * 64 + 48];
	int i;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->alpha[0x800])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = (word >> 5) & 0x3ff;
			int newbank = word & 0x1f;
			if (newscroll != state->playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_set_scrollx(state->playfield_tilemap, 0, newscroll);
				state->playfield_xscroll = newscroll;
			}
			if (newbank != state->playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_mark_all_tiles_dirty(state->playfield_tilemap);
				state->playfield_color_bank = newbank;
			}
		}

		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != state->playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_set_scrolly(state->playfield_tilemap, 0, newscroll);
				state->playfield_yscroll = newscroll;
			}
			if (newbank != state->playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_mark_all_tiles_dirty(state->playfield_tilemap);
				state->playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( atarig42 )
{
	atarig42_state *state = screen->machine->driver_data<atarig42_state>();
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;

	/* draw the playfield */
	bitmap_fill(priority_bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 1, 1);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 2, 2);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 3, 3);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 4, 4);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 5, 5);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 6, 6);
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 7, 7);

	/* copy the motion objects on top */
	{
		bitmap_t *mo_bitmap = atarirle_get_vram(0, 0);
		int left	= cliprect->min_x;
		int top		= cliprect->min_y;
		int right	= cliprect->max_x + 1;
		int bottom	= cliprect->max_y + 1;
		int x, y;

		/* now blend with the playfield */
		for (y = top; y < bottom; y++)
		{
			UINT16 *pf = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			UINT16 *mo = (UINT16 *)mo_bitmap->base + y * mo_bitmap->rowpixels;
			UINT8 *pri = (UINT8 *)priority_bitmap->base + priority_bitmap->rowpixels * y;
			for (x = left; x < right; x++)
				if (mo[x])
				{
					int pfpri = pri[x];
					int mopri = mo[x] >> ATARIMO_PRIORITY_SHIFT;
					if (mopri >= pfpri)
						pf[x] = mo[x] & ATARIRLE_DATA_MASK;
				}
		}
	}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->alpha_tilemap, 0, 0);
	return 0;
}
