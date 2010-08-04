/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/atarig1.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();
	UINT16 data = state->alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();
	UINT16 data = state->playfield[tile_index];
	int code = (state->playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig1 )
{
	static const atarirle_desc modesc_hydra =
	{
		"gfx3",		/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		0,			/* left clip coordinate */
		255,		/* right clip coordinate */

		0x200,		/* base palette entry */
		0x100,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x00f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0x00ff,0,0 }},	/* mask for the order */
		{{ 0 }},					/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};

	static const atarirle_desc modesc_pitfight =
	{
		"gfx3",		/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		40,			/* left clip coordinate */
		295,		/* right clip coordinate */

		0x200,		/* base palette entry */
		0x100,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x00f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0,0x00ff,0 }},	/* mask for the order */
		{{ 0 }},					/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};
	atarig1_state *state = machine->driver_data<atarig1_state>();

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x10);

	/* initialize the playfield */
	state->playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  8,8, 64,64);

	/* initialize the motion objects */
	atarirle_init(machine, 0, state->is_pitfight ? &modesc_pitfight : &modesc_hydra);

	/* initialize the alphanumerics */
	state->alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->alpha_tilemap, 0);

	/* reset statics */
	state->pfscroll_xoffset = state->is_pitfight ? 2 : 0;

	/* state saving */
	state_save_register_global(machine, state->current_control);
	state_save_register_global(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->playfield_xscroll);
	state_save_register_global(machine, state->playfield_yscroll);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig1_mo_control_w )
{
	atarig1_state *state = space->machine->driver_data<atarig1_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space->machine->primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->current_control);
}


void atarig1_scanline_update(screen_device &screen, int scanline)
{
	atarig1_state *state = screen.machine->driver_data<atarig1_state>();
	UINT16 *base = &state->alpha[(scanline / 8) * 64 + 48];
	int i;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->alpha[0x800])
		return;
	screen.update_partial(MAX(scanline - 1, 0));

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		/* first word controls horizontal scroll */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) + state->pfscroll_xoffset) & 0x1ff;
			if (newscroll != state->playfield_xscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				tilemap_set_scrollx(state->playfield_tilemap, 0, newscroll);
				state->playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != state->playfield_yscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				tilemap_set_scrolly(state->playfield_tilemap, 0, newscroll);
				state->playfield_yscroll = newscroll;
			}
			if (newbank != state->playfield_tile_bank)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
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

VIDEO_UPDATE( atarig1 )
{
	atarig1_state *state = screen->machine->driver_data<atarig1_state>();

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 0, 0);

	/* copy the motion objects on top */
	copybitmap_trans(bitmap, atarirle_get_vram(0, 0), 0, 0, 0, 0, cliprect, 0);

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->alpha_tilemap, 0, 0);
	return 0;
}
