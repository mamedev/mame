/***************************************************************************

    Atari Cyberball hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "includes/cyberbal.h"


#define SCREEN_WIDTH		(42*16)



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(cyberbal_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_state::get_alpha2_tile_info)
{
	UINT16 data = m_alpha2[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, 0x80 | color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}


TILE_GET_INFO_MEMBER(cyberbal_state::get_playfield2_tile_info)
{
	UINT16 data = m_playfield2[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, 0x80 | color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void video_start_cyberbal_common(running_machine &machine, int screens)
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
	cyberbal_state *state = machine.driver_data<cyberbal_state>();

	/* initialize the playfield */
	state->m_playfield_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(cyberbal_state::get_playfield_tile_info),state), TILEMAP_SCAN_ROWS,  16,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &mo0desc);
	atarimo_set_slipram(0, &state->m_current_slip[0]);

	/* initialize the alphanumerics */
	state->m_alpha_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(cyberbal_state::get_alpha_tile_info),state), TILEMAP_SCAN_ROWS,  16,8, 64,32);
	state->m_alpha_tilemap->set_transparent_pen(0);

	/* allocate the second screen if necessary */
	if (screens == 2)
	{
		/* initialize the playfield */
		state->m_playfield2_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(cyberbal_state::get_playfield2_tile_info),state), TILEMAP_SCAN_ROWS,  16,8, 64,64);
		state->m_playfield2_tilemap->set_scrollx(0, 0);

		/* initialize the motion objects */
		atarimo_init(machine, 1, &mo1desc);
		atarimo_set_slipram(1, &state->m_current_slip[1]);

		/* initialize the alphanumerics */
		state->m_alpha2_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(cyberbal_state::get_alpha2_tile_info),state), TILEMAP_SCAN_ROWS,  16,8, 64,32);
		state->m_alpha2_tilemap->set_scrollx(0, 0);
		state->m_alpha2_tilemap->set_transparent_pen(0);
	}

	/* save states */
	state->save_item(NAME(state->m_current_slip));
	state->save_item(NAME(state->m_playfield_palette_bank));
	state->save_item(NAME(state->m_playfield_xscroll));
	state->save_item(NAME(state->m_playfield_yscroll));
}


VIDEO_START_MEMBER(cyberbal_state,cyberbal)
{
	video_start_cyberbal_common(machine(), 2);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 4);
	atarimo_set_xscroll(1, 4);
}


VIDEO_START_MEMBER(cyberbal_state,cyberbal2p)
{
	video_start_cyberbal_common(machine(), 1);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 5);
}



/*************************************
 *
 *  Palette tweaker
 *
 *************************************/

INLINE void set_palette_entry(running_machine &machine, int entry, UINT16 value)
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
	cyberbal_state *state = space.machine().driver_data<cyberbal_state>();
	COMBINE_DATA(&state->m_paletteram_0[offset]);
	set_palette_entry(space.machine(), offset, state->m_paletteram_0[offset]);
}

READ16_HANDLER( cyberbal_paletteram_0_r )
{
	cyberbal_state *state = space.machine().driver_data<cyberbal_state>();
	return state->m_paletteram_0[offset];
}


WRITE16_HANDLER( cyberbal_paletteram_1_w )
{
	cyberbal_state *state = space.machine().driver_data<cyberbal_state>();
	COMBINE_DATA(&state->m_paletteram_1[offset]);
	set_palette_entry(space.machine(), offset + 0x800, state->m_paletteram_1[offset]);
}

READ16_HANDLER( cyberbal_paletteram_1_r )
{
	cyberbal_state *state = space.machine().driver_data<cyberbal_state>();
	return state->m_paletteram_1[offset];
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void cyberbal_scanline_update(screen_device &screen, int scanline)
{
	cyberbal_state *state = screen.machine().driver_data<cyberbal_state>();
	int i;
	screen_device *update_screen;

	/* loop over screens */
	screen_device_iterator iter(screen.machine().root_device());
	for (i = 0, update_screen = iter.first(); update_screen != NULL; i++, update_screen = iter.next())
	{
		UINT16 *vram = i ? state->m_alpha2 : state->m_alpha;
		UINT16 *base = &vram[((scanline - 8) / 8) * 64 + 47];

		/* keep in range */
		if (base < vram)
			base += 0x800;
		else if (base >= &vram[0x800])
			return;

		/* update the current parameters */
		if (!(base[3] & 1))
		{
			if (((base[3] >> 1) & 7) != state->m_playfield_palette_bank[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				state->m_playfield_palette_bank[i] = (base[3] >> 1) & 7;
				(i ? state->m_playfield2_tilemap : state->m_playfield_tilemap)->set_palette_offset(state->m_playfield_palette_bank[i] << 8);
			}
		}
		if (!(base[4] & 1))
		{
			int newscroll = 2 * (((base[4] >> 7) + 4) & 0x1ff);
			if (newscroll != state->m_playfield_xscroll[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				(i ? state->m_playfield2_tilemap : state->m_playfield_tilemap)->set_scrollx(0, newscroll);
				state->m_playfield_xscroll[i] = newscroll;
			}
		}
		if (!(base[5] & 1))
		{
			/* a new vscroll latches the offset into a counter; we must adjust for this */
			int newscroll = ((base[5] >> 7) - (scanline)) & 0x1ff;
			if (newscroll != state->m_playfield_yscroll[i])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				(i ? state->m_playfield2_tilemap : state->m_playfield_tilemap)->set_scrolly(0, newscroll);
				state->m_playfield_yscroll[i] = newscroll;
			}
		}
		if (!(base[7] & 1))
		{
			if (state->m_current_slip[i] != base[7])
			{
				if (scanline > 0)
					update_screen->update_partial(scanline - 1);
				state->m_current_slip[i] = base[7];
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

static UINT32 update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index)
{
	cyberbal_state *state = screen.machine().driver_data<cyberbal_state>();
	atarimo_rect_list rectlist;
	rectangle tempclip = cliprect;
	bitmap_ind16 *mobitmap;
	int x, y, r, mooffset, temp;
	rectangle visarea = screen.visible_area();

	/* draw the playfield */
	((index == 0) ? state->m_playfield_tilemap : state->m_playfield2_tilemap)->draw(bitmap, cliprect, 0, 0);

	/* draw the MOs -- note some kludging to get this to work correctly for 2 screens */
	mooffset = 0;
	tempclip.min_x -= mooffset;
	tempclip.max_x -= mooffset;
	temp = visarea.max_x;
	if (temp > SCREEN_WIDTH)
		visarea.max_x /= 2;
	mobitmap = atarimo_render((index == 0) ? 0 : 1, cliprect, &rectlist);
	tempclip.min_x += mooffset;
	tempclip.max_x += mooffset;
	visarea.max_x = temp;

	/* draw and merge the MO */
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y) + mooffset;
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
	((index == 0) ? state->m_alpha_tilemap : state->m_alpha2_tilemap)->draw(bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 cyberbal_state::screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, 0);
}

UINT32 cyberbal_state::screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, 1);
}

UINT32 cyberbal_state::screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return update_one_screen(screen, bitmap, cliprect, 0);
}
