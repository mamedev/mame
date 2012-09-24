/***************************************************************************

    Atari System 2 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/slapstic.h"
#include "includes/atarisy2.h"



/*************************************
 *
 *  Prototypes
 *
 *************************************/





/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarisy2_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = data & 0x3ff;
	int color = (data >> 13) & 0x07;
	SET_TILE_INFO_MEMBER(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(atarisy2_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = m_playfield_tile_bank[(data >> 10) & 1] + (data & 0x3ff);
	int color = (data >> 11) & 7;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
	tileinfo.category = (~data >> 14) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(atarisy2_state,atarisy2)
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		0,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x00,				/* base palette entry */
		0x40,				/* maximum number of colors */
		15,					/* transparent pen index */

		{{ 0,0,0,0x07f8 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x07ff,0,0 }},	/* mask for the code index */
		{{ 0x0007,0,0,0 }},	/* mask for the upper code index */
		{{ 0,0,0,0x3000 }},	/* mask for the color */
		{{ 0,0,0xffc0,0 }},	/* mask for the X position */
		{{ 0x7fc0,0,0,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0,0x3800,0,0 }},	/* mask for the height, in tiles */
		{{ 0,0x4000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0,0xc000 }},	/* mask for the priority */
		{{ 0,0x8000,0,0 }},	/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};

	/* initialize banked memory */
	m_alpha.set_target(&m_vram[0x0000], 0x2000);
	m_playfield.set_target(&m_vram[0x2000], 0x2000);

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarisy2_state::get_playfield_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 128,64);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);

	/* initialize the alphanumerics */
	m_alpha_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarisy2_state::get_alpha_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 64,48);
	m_alpha_tilemap->set_transparent_pen(0);

	/* reset the statics */
	m_yscroll_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(atarisy2_state::reset_yscroll_callback),this));
	m_videobank = 0;

	/* save states */
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_videobank));
	save_item(NAME(m_vram));
}



/*************************************
 *
 *  Scroll/playfield bank write
 *
 *************************************/

WRITE16_HANDLER( atarisy2_xscroll_w )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	UINT16 oldscroll = *state->m_xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	state->m_playfield_tilemap->set_scrollx(0, newscroll >> 6);

	/* update the playfield banking */
	if (state->m_playfield_tile_bank[0] != (newscroll & 0x0f) * 0x400)
	{
		state->m_playfield_tile_bank[0] = (newscroll & 0x0f) * 0x400;
		state->m_playfield_tilemap->mark_all_dirty();
	}

	/* update the data */
	*state->m_xscroll = newscroll;
}


TIMER_CALLBACK_MEMBER(atarisy2_state::reset_yscroll_callback)
{
	m_playfield_tilemap->set_scrolly(0, param);
}


WRITE16_HANDLER( atarisy2_yscroll_w )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	UINT16 oldscroll = *state->m_yscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());

	/* if bit 4 is zero, the scroll value is clocked in right away */
	if (!(newscroll & 0x10))
		state->m_playfield_tilemap->set_scrolly(0, (newscroll >> 6) - space.machine().primary_screen->vpos());
	else
		state->m_yscroll_reset_timer->adjust(space.machine().primary_screen->time_until_pos(0), newscroll >> 6);

	/* update the playfield banking */
	if (state->m_playfield_tile_bank[1] != (newscroll & 0x0f) * 0x400)
	{
		state->m_playfield_tile_bank[1] = (newscroll & 0x0f) * 0x400;
		state->m_playfield_tilemap->mark_all_dirty();
	}

	/* update the data */
	*state->m_yscroll = newscroll;
}



/*************************************
 *
 *  Palette RAM write handler
 *
 *************************************/

WRITE16_HANDLER( atarisy2_paletteram_w )
{
	static const int intensity_table[16] =
	{
		#define ZB 115
		#define Z3 78
		#define Z2 37
		#define Z1 17
		#define Z0 9
		0, ZB+Z0, ZB+Z1, ZB+Z1+Z0, ZB+Z2, ZB+Z2+Z0, ZB+Z2+Z1, ZB+Z2+Z1+Z0,
		ZB+Z3, ZB+Z3+Z0, ZB+Z3+Z1, ZB+Z3+Z1+Z0,ZB+ Z3+Z2, ZB+Z3+Z2+Z0, ZB+Z3+Z2+Z1, ZB+Z3+Z2+Z1+Z0
	};
	static const int color_table[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xe, 0xf, 0xf };

	int newword, inten, red, green, blue;

	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);
	newword = state->m_generic_paletteram_16[offset];

	inten = intensity_table[newword & 15];
	red = (color_table[(newword >> 12) & 15] * inten) >> 4;
	green = (color_table[(newword >> 8) & 15] * inten) >> 4;
	blue = (color_table[(newword >> 4) & 15] * inten) >> 4;
	palette_set_color(space.machine(), offset, MAKE_RGB(red, green, blue));
}



/*************************************
 *
 *  Video RAM bank read/write handlers
 *
 *************************************/

READ16_HANDLER( atarisy2_slapstic_r )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	int result = state->m_slapstic_base[offset];
	slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	state->m_videobank = slapstic_tweak(space, 0x1234) * 0x1000;
	return result;
}


WRITE16_HANDLER( atarisy2_slapstic_w )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();

	slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	state->m_videobank = slapstic_tweak(space, 0x1234) * 0x1000;
}



/*************************************
 *
 *  Video RAM read/write handlers
 *
 *************************************/

READ16_HANDLER( atarisy2_videoram_r )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	int offs = offset | state->m_videobank;
	if (offs >= 0xc00 && offs < 0x1000)
	{
		return atarimo_0_spriteram_r(space, offs - 0x0c00, mem_mask);
	}

	return state->m_vram[offs];
}


WRITE16_HANDLER( atarisy2_videoram_w )
{
	atarisy2_state *state = space.machine().driver_data<atarisy2_state>();
	int offs = offset | state->m_videobank;

	/* alpharam? */
	if (offs < 0x0c00)
	{
		COMBINE_DATA(&state->m_alpha[offs]);
		state->m_alpha_tilemap->mark_tile_dirty(offs);
	}

	/* spriteram? */
	else if (offs < 0x1000)
	{
		/* force an update if the link of object 0 is about to change */
		if (offs == 0x0c03)
			space.machine().primary_screen->update_partial(space.machine().primary_screen->vpos());
		atarimo_0_spriteram_w(space, offs - 0x0c00, data, mem_mask);
	}

	/* playfieldram? */
	else if (offs >= 0x2000)
	{
		offs -= 0x2000;
		COMBINE_DATA(&state->m_playfield[offs]);
		state->m_playfield_tilemap->mark_tile_dirty(offs);
	}

	/* generic case */
	else
	{
		COMBINE_DATA(&state->m_vram[offs]);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 atarisy2_state::screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(bitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(bitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(bitmap, cliprect, 3, 3);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x] != 0x0f)
				{
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* high priority PF? */
					if ((mopriority + pri[x]) & 2)
					{
						/* only gets priority if PF pen is less than 8 */
						if (!(pf[x] & 0x08))
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
					}

					/* low priority */
					else
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					/* erase behind ourselves */
					mo[x] = 0x0f;
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
