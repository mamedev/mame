/***************************************************************************

    Atari Cyberball hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


#define SCREEN_WIDTH		(42*16)



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	cyberbal_state *state = machine->driver_data<cyberbal_state>();
	UINT16 data = state->alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO(2, code, color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_alpha2_tile_info )
{
	cyberbal_state *state = machine->driver_data<cyberbal_state>();
	UINT16 data = state->alpha2[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO(2, code, 0x80 | color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	cyberbal_state *state = machine->driver_data<cyberbal_state>();
	UINT16 data = state->playfield[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_playfield2_tile_info )
{
	cyberbal_state *state = machine->driver_data<cyberbal_state>();
	UINT16 data = state->playfield2[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO(0, code, 0x80 | color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void video_start_cyberbal_common(running_machine* machine, int screens)
{
	static const atarimo_desc mo0desc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		1,					/* does the neighbor bit affect the next object? */
		1024,				/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x600,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0x07f8,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0x7fff,0,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0,0x000f }},	/* mask for the color */
		{{ 0,0,0,0xffc0 }},	/* mask for the X position */
		{{ 0,0xff80,0,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0,0x000f,0,0 }},	/* mask for the height, in tiles */
		{{ 0x8000,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0,0,0,0x0010 }},	/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};

	static const atarimo_desc mo1desc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		1,					/* does the neighbor bit affect the next object? */
		1024,				/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0xe00,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0x07f8,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0x7fff,0,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0,0x000f }},	/* mask for the color */
		{{ 0,0,0,0xffc0 }},	/* mask for the X position */
		{{ 0,0xff80,0,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0,0x000f,0,0 }},	/* mask for the height, in tiles */
		{{ 0x8000,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0,0,0,0x0010 }},	/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};
	cyberbal_state *state = machine->driver_data<cyberbal_state>();

	/* set the slip variables */
	atarimo_0_slipram = &state->current_slip[0];
	atarimo_1_slipram = &state->current_slip[1];

	/* initialize the playfield */
	state->playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  16,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &mo0desc);

	/* initialize the alphanumerics */
	state->alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  16,8, 64,32);
	tilemap_set_transparent_pen(state->alpha_tilemap, 0);

	/* allocate the second screen if necessary */
	if (screens == 2)
	{
		/* initialize the playfield */
		state->playfield2_tilemap = tilemap_create(machine, get_playfield2_tile_info, tilemap_scan_rows,  16,8, 64,64);
		tilemap_set_scrollx(state->playfield2_tilemap, 0, 0);

		/* initialize the motion objects */
		atarimo_init(machine, 1, &mo1desc);

		/* initialize the alphanumerics */
		state->alpha2_tilemap = tilemap_create(machine, get_alpha2_tile_info, tilemap_scan_rows,  16,8, 64,32);
		tilemap_set_scrollx(state->alpha2_tilemap, 0, 0);
		tilemap_set_transparent_pen(state->alpha2_tilemap, 0);
	}

	/* save states */
	state_save_register_global_array(machine, state->current_slip);
	state_save_register_global_array(machine, state->playfield_palette_bank);
	state_save_register_global_array(machine, state->playfield_xscroll);
	state_save_register_global_array(machine, state->playfield_yscroll);
}


VIDEO_START( cyberbal )
{
	video_start_cyberbal_common(machine, 2);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 4);
	atarimo_set_xscroll(1, 4);
}


VIDEO_START( cyberbal2p )
{
	video_start_cyberbal_common(machine, 1);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 5);
}



/*************************************
 *
 *  Palette tweaker
 *
 *************************************/

INLINE void set_palette_entry(running_machine *machine, int entry, UINT16 value)
{
	int r, g, b;

	r = ((value >> 9) & 0x3e) | ((value >> 15) & 1);
	g = ((value >> 4) & 0x3e) | ((value >> 15) & 1);
	b = ((value << 1) & 0x3e) | ((value >> 15) & 1);

	palette_set_color_rgb(machine, entry, pal6bit(r), pal6bit(g), pal6bit(b));
}



/*************************************
 *
 *  Palette RAM write handlers
 *
 *************************************/

WRITE16_HANDLER( cyberbal_paletteram_0_w )
{
	cyberbal_state *state = space->machine->driver_data<cyberbal_state>();
	COMBINE_DATA(&state->paletteram_0[offset]);
	set_palette_entry(space->machine, offset, state->paletteram_0[offset]);
}

READ16_HANDLER( cyberbal_paletteram_0_r )
{
	cyberbal_state *state = space->machine->driver_data<cyberbal_state>();
	return state->paletteram_0[offset];
}


WRITE16_HANDLER( cyberbal_paletteram_1_w )
{
	cyberbal_state *state = space->machine->driver_data<cyberbal_state>();
	COMBINE_DATA(&state->paletteram_1[offset]);
	set_palette_entry(space->machine, offset + 0x800, state->paletteram_1[offset]);
}

READ16_HANDLER( cyberbal_paletteram_1_r )
{
	cyberbal_state *state = space->machine->driver_data<cyberbal_state>();
	return state->paletteram_1[offset];
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void cyberbal_scanline_update(screen_device &screen, int scanline)
{
	cyberbal_state *state = screen.machine->driver_data<cyberbal_state>();
	int i;
	screen_device *update_screen;

	/* loop over screens */
	for (i = 0, update_screen = screen.machine->first_screen(); update_screen != NULL; i++, update_screen = update_screen->next_screen())
	{
		UINT16 *vram = i ? state->alpha2 : state->alpha;
		UINT16 *base = &vram[((scanline - 8) / 8) * 64 + 47];

		/* keep in range */
		if (base < vram)
			base += 0x800;
		else if (base >= &vram[0x800])
			return;

		/* update the current parameters */
		if (!(base[3] & 1))
		{
			if (((base[3] >> 1) & 7) != state->playfield_palette_bank[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				state->playfield_palette_bank[i] = (base[3] >> 1) & 7;
				tilemap_set_palette_offset(i ? state->playfield2_tilemap : state->playfield_tilemap, state->playfield_palette_bank[i] << 8);
			}
		}
		if (!(base[4] & 1))
		{
			int newscroll = 2 * (((base[4] >> 7) + 4) & 0x1ff);
			if (newscroll != state->playfield_xscroll[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				tilemap_set_scrollx(i ? state->playfield2_tilemap : state->playfield_tilemap, 0, newscroll);
				state->playfield_xscroll[i] = newscroll;
			}
		}
		if (!(base[5] & 1))
		{
			/* a new vscroll latches the offset into a counter; we must adjust for this */
			int newscroll = ((base[5] >> 7) - (scanline)) & 0x1ff;
			if (newscroll != state->playfield_yscroll[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				tilemap_set_scrolly(i ? state->playfield2_tilemap : state->playfield_tilemap, 0, newscroll);
				state->playfield_yscroll[i] = newscroll;
			}
		}
		if (!(base[7] & 1))
		{
			if (state->current_slip[i] != base[7])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				state->current_slip[i] = base[7];
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

static void update_one_screen(screen_device &screen, bitmap_t *bitmap, const rectangle *cliprect)
{
	cyberbal_state *state = screen.machine->driver_data<cyberbal_state>();
	atarimo_rect_list rectlist;
	rectangle tempclip = *cliprect;
	bitmap_t *mobitmap;
	int x, y, r, mooffset, temp;
	rectangle visarea = screen.visible_area();

	/* for 2p games, the left screen is the main screen */
	device_t *left_screen = screen.machine->device("lscreen");
	if (left_screen == NULL)
		left_screen = screen.machine->device("screen");

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, (&screen == left_screen) ? state->playfield_tilemap : state->playfield2_tilemap, 0, 0);

	/* draw the MOs -- note some kludging to get this to work correctly for 2 screens */
	mooffset = 0;
	tempclip.min_x -= mooffset;
	tempclip.max_x -= mooffset;
	temp = visarea.max_x;
	if (temp > SCREEN_WIDTH)
		visarea.max_x /= 2;
	mobitmap = atarimo_render((&screen == left_screen) ? 0 : 1, cliprect, &rectlist);
	tempclip.min_x += mooffset;
	tempclip.max_x += mooffset;
	visarea.max_x = temp;

	/* draw and merge the MO */
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y + mooffset;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* not verified: logic is all controlled in a PAL
                    */
					pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, (&screen == left_screen) ? state->alpha_tilemap : state->alpha2_tilemap, 0, 0);
}


VIDEO_UPDATE( cyberbal )
{
	update_one_screen(*screen, bitmap, cliprect);
	return 0;
}
