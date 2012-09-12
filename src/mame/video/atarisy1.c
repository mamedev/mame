/***************************************************************************

    Atari System 1 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarimo.h"
#include "includes/atarisy1.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

/* the color and remap PROMs are mapped as follows */
#define PROM1_BANK_4			0x80		/* active low */
#define PROM1_BANK_3			0x40		/* active low */
#define PROM1_BANK_2			0x20		/* active low */
#define PROM1_BANK_1			0x10		/* active low */
#define PROM1_OFFSET_MASK		0x0f		/* postive logic */

#define PROM2_BANK_6_OR_7		0x80		/* active low */
#define PROM2_BANK_5			0x40		/* active low */
#define PROM2_PLANE_5_ENABLE	0x20		/* active high */
#define PROM2_PLANE_4_ENABLE	0x10		/* active high */
#define PROM2_PF_COLOR_MASK		0x0f		/* negative logic */
#define PROM2_BANK_7			0x08		/* active low, plus PROM2_BANK_6_OR_7 low as well */
#define PROM2_MO_COLOR_MASK		0x07		/* negative logic */



/*************************************
 *
 *  Statics
 *
 *************************************/

static const gfx_layout objlayout_4bpp =
{
	8,8,	/* 8*8 sprites */
	4096,	/* 4096 of them */
	4,		/* 4 bits per pixel */
	{ 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8		/* every sprite takes 8 consecutive bytes */
};

static const gfx_layout objlayout_5bpp =
{
	8,8,	/* 8*8 sprites */
	4096,	/* 4096 of them */
	5,		/* 5 bits per pixel */
	{ 4*8*0x10000, 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8		/* every sprite takes 8 consecutive bytes */
};

static const gfx_layout objlayout_6bpp =
{
	8,8,	/* 8*8 sprites */
	4096,	/* 4096 of them */
	6,		/* 6 bits per pixel */
	{ 5*8*0x10000, 4*8*0x10000, 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8		/* every sprite takes 8 consecutive bytes */
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void update_timers(running_machine &machine, int scanline);
static void decode_gfx(running_machine &machine, UINT16 *pflookup, UINT16 *molookup);
static int get_bank(running_machine &machine, UINT8 prom1, UINT8 prom2, int bpp);



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarisy1_state::get_alpha_tile_info)
{
	UINT16 data = m_alpha[tile_index];
	int code = data & 0x3ff;
	int color = (data >> 10) & 0x07;
	int opaque = data & 0x2000;
	SET_TILE_INFO_MEMBER(0, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarisy1_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	UINT16 lookup = m_playfield_lookup[((data >> 8) & 0x7f) | (m_playfield_tile_bank << 7)];
	int gfxindex = (lookup >> 8) & 15;
	int code = ((lookup & 0xff) << 8) | (data & 0xff);
	int color = 0x20 + (((lookup >> 12) & 15) << m_bank_color_shift[gfxindex]);
	SET_TILE_INFO_MEMBER(gfxindex, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Generic video system start
 *
 *************************************/

VIDEO_START( atarisy1 )
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		8,					/* number of motion object banks */
		1,					/* are the entries linked? */
		1,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		0,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0x38,				/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0,0x003f }},	/* mask for the link */
		{{ 0,0xff00,0,0 }},	/* mask for the graphics bank */
		{{ 0,0xffff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0xff00,0,0 }},	/* mask for the color */
		{{ 0,0,0x3fe0,0 }},	/* mask for the X position */
		{{ 0x3fe0,0,0,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0x000f,0,0,0 }},	/* mask for the height, in tiles */
		{{ 0x8000,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0x8000,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0,0xffff,0,0 }},	/* mask for the special value */
		0xffff,				/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};

	atarisy1_state *state = machine.driver_data<atarisy1_state>();
	UINT16 motable[256];
	UINT16 *codelookup;
	UINT8 *colorlookup, *gfxlookup;
	int i, size;

	/* first decode the graphics */
	decode_gfx(machine, state->m_playfield_lookup, motable);

	/* initialize the playfield */
	state->m_playfield_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(atarisy1_state::get_playfield_tile_info),state), TILEMAP_SCAN_ROWS,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->m_alpha_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(atarisy1_state::get_alpha_tile_info),state), TILEMAP_SCAN_ROWS,  8,8, 64,32);
	state->m_alpha_tilemap->set_transparent_pen(0);

	/* modify the motion object code lookup */
	codelookup = atarimo_get_code_lookup(0, &size);
	for (i = 0; i < size; i++)
		codelookup[i] = (i & 0xff) | ((motable[i >> 8] & 0xff) << 8);

	/* modify the motion object color and gfx lookups */
	colorlookup = atarimo_get_color_lookup(0, &size);
	gfxlookup = atarimo_get_gfx_lookup(0, &size);
	for (i = 0; i < size; i++)
	{
		colorlookup[i] = ((motable[i] >> 12) & 15) << 1;
		gfxlookup[i] = (motable[i] >> 8) & 15;
	}

	/* reset the statics */
	atarimo_set_yscroll(0, 256);
	state->m_next_timer_scanline = -1;

	/* save state */
	state->save_item(NAME(state->m_playfield_tile_bank));
	state->save_item(NAME(state->m_playfield_priority_pens));
	state->save_item(NAME(state->m_next_timer_scanline));
}



/*************************************
 *
 *  Graphics bank selection
 *
 *************************************/

WRITE16_HANDLER( atarisy1_bankselect_w )
{
	atarisy1_state *state = space->machine().driver_data<atarisy1_state>();
	UINT16 oldselect = *state->m_bankselect;
	UINT16 newselect = oldselect, diff;
	int scanline = space->machine().primary_screen->vpos();

	/* update memory */
	COMBINE_DATA(&newselect);
	diff = oldselect ^ newselect;

	/* sound CPU reset */
	if (diff & 0x0080)
	{
		space->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, (newselect & 0x0080) ? CLEAR_LINE : ASSERT_LINE);
		if (!(newselect & 0x0080)) atarigen_sound_reset(space->machine());
	}

	/* if MO or playfield banks change, force a partial update */
	if (diff & 0x003c)
		space->machine().primary_screen->update_partial(scanline);

	/* motion object bank select */
	atarimo_set_bank(0, (newselect >> 3) & 7);
	update_timers(space->machine(), scanline);

	/* playfield bank select */
	if (diff & 0x0004)
	{
		state->m_playfield_tile_bank = (newselect >> 2) & 1;
		state->m_playfield_tilemap->mark_all_dirty();
	}

	/* stash the new value */
	*state->m_bankselect = newselect;
}



/*************************************
 *
 *  Playfield priority pens
 *
 *************************************/

WRITE16_HANDLER( atarisy1_priority_w )
{
	atarisy1_state *state = space->machine().driver_data<atarisy1_state>();
	UINT16 oldpens = state->m_playfield_priority_pens;
	UINT16 newpens = oldpens;

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newpens);
	if (oldpens != newpens)
		space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());
	state->m_playfield_priority_pens = newpens;
}



/*************************************
 *
 *  Playfield horizontal scroll
 *
 *************************************/

WRITE16_HANDLER( atarisy1_xscroll_w )
{
	atarisy1_state *state = space->machine().driver_data<atarisy1_state>();
	UINT16 oldscroll = *state->m_xscroll;
	UINT16 newscroll = oldscroll;

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newscroll);
	if (oldscroll != newscroll)
		space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos());

	/* set the new scroll value */
	state->m_playfield_tilemap->set_scrollx(0, newscroll);

	/* update the data */
	*state->m_xscroll = newscroll;
}



/*************************************
 *
 *  Playfield vertical scroll
 *
 *************************************/

TIMER_DEVICE_CALLBACK( atarisy1_reset_yscroll_callback )
{
	atarisy1_state *state = timer.machine().driver_data<atarisy1_state>();
	state->m_playfield_tilemap->set_scrolly(0, param);
}


WRITE16_HANDLER( atarisy1_yscroll_w )
{
	atarisy1_state *state = space->machine().driver_data<atarisy1_state>();
	UINT16 oldscroll = *state->m_yscroll;
	UINT16 newscroll = oldscroll;
	int scanline = space->machine().primary_screen->vpos();
	int adjusted_scroll;

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newscroll);
	space->machine().primary_screen->update_partial(scanline);

	/* because this latches a new value into the scroll base,
       we need to adjust for the scanline */
	adjusted_scroll = newscroll;
	if (scanline <= space->machine().primary_screen->visible_area().max_y)
		adjusted_scroll -= (scanline + 1);
	state->m_playfield_tilemap->set_scrolly(0, adjusted_scroll);

	/* but since we've adjusted it, we must reset it to the normal value
       once we hit scanline 0 again */
	state->m_yscroll_reset_timer->adjust(space->machine().primary_screen->time_until_pos(0), newscroll);

	/* update the data */
	*state->m_yscroll = newscroll;
}



/*************************************
 *
 *  Sprite RAM write handler
 *
 *************************************/

WRITE16_HANDLER( atarisy1_spriteram_w )
{
	int active_bank = atarimo_get_bank(0);
	int oldword = atarimo_0_spriteram_r(space, offset, mem_mask);
	int newword = oldword;
	COMBINE_DATA(&newword);

	/* if the data changed, and it modified the live sprite bank, do some extra work */
	if (oldword != newword && (offset >> 8) == active_bank)
	{
		/* if modifying a timer, beware */
		if (((offset & 0xc0) == 0x00 && atarimo_0_spriteram_r(space, offset | 0x40, mem_mask) == 0xffff) ||
		    ((offset & 0xc0) == 0x40 && (newword == 0xffff || oldword == 0xffff)))
		{
			/* if the timer is in the active bank, update the display list */
			atarimo_0_spriteram_w(space, offset, data, 0xffff);
			update_timers(space->machine(), space->machine().primary_screen->vpos());
		}

		/* if we're about to modify data in the active sprite bank, make sure the video is up-to-date */
		/* Road Runner needs this to work; note the +2 kludge -- +1 would be correct since the video */
		/* renders the next scanline's sprites to the line buffers, but Road Runner still glitches */
		/* without the extra +1 */
		else
			space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos() + 2);
	}

	/* let the MO handler do the basic work */
	atarimo_0_spriteram_w(space, offset, data, 0xffff);
}



/*************************************
 *
 *  MO interrupt handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK( atarisy1_int3off_callback )
{
	address_space *space = timer.machine().device("maincpu")->memory().space(AS_PROGRAM);

	/* clear the state */
	atarigen_scanline_int_ack_w(space, 0, 0, 0xffff);
}


TIMER_DEVICE_CALLBACK( atarisy1_int3_callback )
{
	atarisy1_state *state = timer.machine().driver_data<atarisy1_state>();
	int scanline = param;

	/* update the state */
	atarigen_scanline_int_gen(timer.machine().device("maincpu"));

	/* set a timer to turn it off */
	state->m_int3off_timer->adjust(timer.machine().primary_screen->scan_period());

	/* determine the time of the next one */
	state->m_next_timer_scanline = -1;
	update_timers(timer.machine(), scanline);
}



/*************************************
 *
 *  MO interrupt state read
 *
 *************************************/

READ16_HANDLER( atarisy1_int3state_r )
{
	atarigen_state *atarigen = space->machine().driver_data<atarigen_state>();
	return atarigen->m_scanline_int_state ? 0x0080 : 0x0000;
}



/*************************************
 *
 *  Timer updater
 *
 *************************************/

static void update_timers(running_machine &machine, int scanline)
{
	atarisy1_state *state = machine.driver_data<atarisy1_state>();
	address_space *space = NULL;
	UINT16 mem_mask = 0xffff;
	int offset = atarimo_get_bank(0) * 64 * 4;
	int link = 0, best = scanline, found = 0;
	UINT8 spritevisit[64];

	/* track which ones we've visited */
	memset(spritevisit, 0, sizeof(spritevisit));

	/* walk the list until we loop */
	while (!spritevisit[link])
	{
		/* timers are indicated by 0xffff in entry 2 */
		if (atarimo_0_spriteram_r(space, offset + link + 0x40, mem_mask) == 0xffff)
		{
			int data = atarimo_0_spriteram_r(space, offset + link, mem_mask);
			int vsize = (data & 15) + 1;
			int ypos = (256 - (data >> 5) - vsize * 8 - 1) & 0x1ff;

			/* note that we found something */
			found = 1;

			/* is this a better entry than the best so far? */
			if (best <= scanline)
			{
				if ((ypos <= scanline && ypos < best) || ypos > scanline)
					best = ypos;
			}
			else
			{
				if (ypos < best)
					best = ypos;
			}
		}

		/* link to the next */
		spritevisit[link] = 1;
		link = atarimo_0_spriteram_r(space, offset + link + 0xc0, mem_mask) & 0x3f;
	}

	/* if nothing was found, use scanline -1 */
	if (!found)
		best = -1;

	/* update the timer */
	if (best != state->m_next_timer_scanline)
	{
		state->m_next_timer_scanline = best;

		/* set a new one */
		if (best != -1)
			state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(best), best);
		else
			state->m_scanline_timer->reset();
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( atarisy1 )
{
	atarisy1_state *state = screen.machine().driver_data<atarisy1_state>();
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	state->m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* high priority MO? */
					if (mo[x] & ATARIMO_PRIORITY_MASK)
					{
						/* only gets priority if MO pen is not 1 */
						if ((mo[x] & 0x0f) != 1)
							pf[x] = 0x300 + ((pf[x] & 0x0f) << 4) + (mo[x] & 0x0f);
					}

					/* low priority */
					else
					{
						/* priority pens for playfield color 0 */
						if ((pf[x] & 0xf8) != 0 || !(state->m_playfield_priority_pens & (1 << (pf[x] & 0x07))))
							pf[x] = mo[x];
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	state->m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}



/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static void decode_gfx(running_machine &machine, UINT16 *pflookup, UINT16 *molookup)
{
	atarisy1_state *state = machine.driver_data<atarisy1_state>();
	UINT8 *prom1 = &state->memregion("proms")->u8(0x000);
	UINT8 *prom2 = &state->memregion("proms")->u8(0x200);
	int obj, i;

	/* reset the globals */
	memset(&state->m_bank_gfx[0][0], 0, sizeof(state->m_bank_gfx));

	/* loop for two sets of objects */
	for (obj = 0; obj < 2; obj++)
	{
		/* loop for 256 objects in the set */
		for (i = 0; i < 256; i++, prom1++, prom2++)
		{
			int bank, bpp, color, offset;

			/* determine the bpp */
			bpp = 4;
			if (*prom2 & PROM2_PLANE_4_ENABLE)
			{
				bpp = 5;
				if (*prom2 & PROM2_PLANE_5_ENABLE)
					bpp = 6;
			}

			/* determine the offset */
			offset = *prom1 & PROM1_OFFSET_MASK;

			/* determine the bank */
			bank = get_bank(machine, *prom1, *prom2, bpp);

			/* set the value */
			if (obj == 0)
			{
				/* playfield case */
				color = (~*prom2 & PROM2_PF_COLOR_MASK) >> (bpp - 4);
				if (bank == 0)
				{
					bank = 1;
					offset = color = 0;
				}
				pflookup[i] = offset | (bank << 8) | (color << 12);
			}
			else
			{
				/* motion objects (high bit ignored) */
				color = (~*prom2 & PROM2_MO_COLOR_MASK) >> (bpp - 4);
				molookup[i] = offset | (bank << 8) | (color << 12);
			}
		}
	}
}



/*************************************
 *
 *  Graphics bank mapping
 *
 *************************************/

static int get_bank(running_machine &machine, UINT8 prom1, UINT8 prom2, int bpp)
{
	atarisy1_state *state = machine.driver_data<atarisy1_state>();
	const UINT8 *srcdata;
	int bank_index, gfx_index;

	/* determine the bank index */
	if ((prom1 & PROM1_BANK_1) == 0)
		bank_index = 1;
	else if ((prom1 & PROM1_BANK_2) == 0)
		bank_index = 2;
	else if ((prom1 & PROM1_BANK_3) == 0)
		bank_index = 3;
	else if ((prom1 & PROM1_BANK_4) == 0)
		bank_index = 4;
	else if ((prom2 & PROM2_BANK_5) == 0)
		bank_index = 5;
	else if ((prom2 & PROM2_BANK_6_OR_7) == 0)
	{
		if ((prom2 & PROM2_BANK_7) == 0)
			bank_index = 7;
		else
			bank_index = 6;
	}
	else
		return 0;

	/* find the bank */
	if (state->m_bank_gfx[bpp - 4][bank_index])
		return state->m_bank_gfx[bpp - 4][bank_index];

	/* if the bank is out of range, call it 0 */
	memory_region *tiles = machine.root_device().memregion("tiles");
	if (0x80000 * (bank_index - 1) >= tiles->bytes())
		return 0;

	/* don't have one? let's make it ... first find any empty slot */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine.gfx[gfx_index] == NULL)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	srcdata = &tiles->u8(0x80000 * (bank_index - 1));
	switch (bpp)
	{
	case 4:
		machine.gfx[gfx_index] = auto_alloc(machine, gfx_element(machine, objlayout_4bpp, srcdata, 0x40, 256));
		break;

	case 5:
		machine.gfx[gfx_index] = auto_alloc(machine, gfx_element(machine, objlayout_5bpp, srcdata, 0x40, 256));
		break;

	case 6:
		machine.gfx[gfx_index] = auto_alloc(machine, gfx_element(machine, objlayout_6bpp, srcdata, 0x40, 256));
		break;

	default:
		fatalerror("Unsupported bpp\n");
	}

	/* set the color information */
	machine.gfx[gfx_index]->set_granularity(8);
	state->m_bank_color_shift[gfx_index] = bpp - 3;

	/* set the entry and return it */
	return state->m_bank_gfx[bpp - 4][bank_index] = gfx_index;
}
