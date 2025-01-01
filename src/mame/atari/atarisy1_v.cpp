// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari System 1 hardware

****************************************************************************/

#include "emu.h"
#include "atarisy1.h"
#include "cpu/m68000/m68000.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

/* the color and remap PROMs are mapped as follows */
static constexpr unsigned PROM1_BANK_4         = 0x80;        /* active low */
static constexpr unsigned PROM1_BANK_3         = 0x40;        /* active low */
static constexpr unsigned PROM1_BANK_2         = 0x20;        /* active low */
static constexpr unsigned PROM1_BANK_1         = 0x10;        /* active low */
static constexpr unsigned PROM1_OFFSET_MASK    = 0x0f;        /* postive logic */

static constexpr unsigned PROM2_BANK_6_OR_7    = 0x80;        /* active low */
static constexpr unsigned PROM2_BANK_5         = 0x40;        /* active low */
static constexpr unsigned PROM2_PLANE_5_ENABLE = 0x20;        /* active high */
static constexpr unsigned PROM2_PLANE_4_ENABLE = 0x10;        /* active high */
static constexpr unsigned PROM2_PF_COLOR_MASK  = 0x0f;        /* negative logic */
static constexpr unsigned PROM2_BANK_7         = 0x08;        /* active low, plus PROM2_BANK_6_OR_7 low as well */
static constexpr unsigned PROM2_MO_COLOR_MASK  = 0x07;        /* negative logic */



/*************************************
 *
 *  Statics
 *
 *************************************/

static const gfx_layout objlayout_4bpp =
{
	8,8,    /* 8*8 sprites */
	4096,   /* 4096 of them */
	4,      /* 4 bits per pixel */
	{ 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every sprite takes 8 consecutive bytes */
};

static const gfx_layout objlayout_5bpp =
{
	8,8,    /* 8*8 sprites */
	4096,   /* 4096 of them */
	5,      /* 5 bits per pixel */
	{ 4*8*0x10000, 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every sprite takes 8 consecutive bytes */
};

static const gfx_layout objlayout_6bpp =
{
	8,8,    /* 8*8 sprites */
	4096,   /* 4096 of them */
	6,      /* 6 bits per pixel */
	{ 5*8*0x10000, 4*8*0x10000, 3*8*0x10000, 2*8*0x10000, 1*8*0x10000, 0*8*0x10000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every sprite takes 8 consecutive bytes */
};



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarisy1_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0x3ff;
	int const color = (data >> 10) & 0x07;
	int const opaque = data & 0x2000;
	tileinfo.set(0, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarisy1_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	uint16_t const lookup = m_playfield_lookup[((data >> 8) & 0x7f) | (m_playfield_tile_bank << 7)];
	int const gfxindex = (lookup >> 8) & 15;
	int const code = ((lookup & 0xff) << 8) | (data & 0xff);
	int const color = 0x20 + (((lookup >> 12) & 15) << m_bank_color_shift[gfxindex]);
	tileinfo.set(gfxindex, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Generic video system start
 *
 *************************************/

const atari_motion_objects_config atarisy1_state::s_mob_config =
{
	0,                  /* index to which gfx system */
	8,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	1,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	0,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0x38,               /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
	0,                  /* transparent pen index */

	{{ 0,0,0,0x003f }}, /* mask for the link */
	{{ 0,0xffff,0,0 }}, /* mask for the code index */
	{{ 0,0xff00,0,0 }}, /* mask for the color */
	{{ 0,0,0x3fe0,0 }}, /* mask for the X position */
	{{ 0x3fe0,0,0,0 }}, /* mask for the Y position */
	{{ 0 }},            /* mask for the width, in tiles*/
	{{ 0x000f,0,0,0 }}, /* mask for the height, in tiles */
	{{ 0x8000,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0,0,0x8000,0 }}, /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0,0xffff,0,0 }}, /* mask for the special value */
	0xffff              /* resulting value to indicate "special" */
};

void atarisy1_state::video_start()
{
	// first decode the graphics
	uint16_t motable[256];
	decode_gfx(m_playfield_lookup, motable);

	// modify the motion object code lookup
	std::vector<uint32_t> &codelookup = m_mob->code_lookup();
	for (unsigned int i = 0; i < codelookup.size(); i++)
		codelookup[i] = (i & 0xff) | ((motable[i >> 8] & 0xff) << 8);

	// modify the motion object color and gfx lookups
	std::vector<uint8_t> &colorlookup = m_mob->color_lookup();
	std::vector<uint8_t> &gfxlookup = m_mob->gfx_lookup();
	for (unsigned int i = 0; i < colorlookup.size(); i++)
	{
		colorlookup[i] = ((motable[i] >> 12) & 15) << 1;
		gfxlookup[i] = (motable[i] >> 8) & 15;
	}

	// reset the statics
	m_mob->set_yscroll(256);
	m_next_timer_scanline = -1;
	m_scanline_int_state = false;
	m_bankselect = 0xff;
	m_playfield_tile_bank = 0;

	// save state
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_priority_pens));
	save_item(NAME(m_next_timer_scanline));
	save_item(NAME(m_scanline_int_state));
	save_item(NAME(m_bankselect));
}



/*************************************
 *
 *  Graphics bank selection
 *
 *************************************/

void atarisy1_state::bankselect_w(uint8_t data)
{
	uint8_t const oldselect = m_bankselect;
	uint8_t const newselect = data;
	int const scanline = m_screen->vpos();

	// update memory
	uint8_t const diff = oldselect ^ newselect;

	// sound CPU reset
	if (BIT(diff, 7))
	{
		m_outlatch->clear_w(BIT(newselect, 7));
		m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(newselect, 7) ? CLEAR_LINE : ASSERT_LINE);
		if (!BIT(newselect, 7))
		{
			m_mainlatch->acknowledge_w();
			if (m_via.found())
				m_via->reset();
		}
	}

	// if MO or playfield banks change, force a partial update
	if (diff & 0x3c)
		m_screen->update_partial(scanline);

	// motion object bank select
	m_mob->set_bank((newselect >> 3) & 7);
	update_timers(scanline);

	// playfield bank select
	if (BIT(diff, 2))
	{
		m_playfield_tile_bank = BIT(newselect, 2);
		m_playfield_tilemap->mark_all_dirty();
	}

	// stash the new value
	m_bankselect = newselect;
}



/*************************************
 *
 *  Playfield priority pens
 *
 *************************************/

void atarisy1_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldpens = m_playfield_priority_pens;
	uint16_t newpens = oldpens;

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newpens);
	if (oldpens != newpens)
		m_screen->update_partial(m_screen->vpos());
	m_playfield_priority_pens = newpens;
}



/*************************************
 *
 *  Playfield horizontal scroll
 *
 *************************************/

void atarisy1_state::xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldscroll = *m_xscroll;
	uint16_t newscroll = oldscroll;

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newscroll);
	if (oldscroll != newscroll)
		m_screen->update_partial(m_screen->vpos());

	/* set the new scroll value */
	m_playfield_tilemap->set_scrollx(0, newscroll);

	/* update the data */
	*m_xscroll = newscroll;
}



/*************************************
 *
 *  Playfield vertical scroll
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_state::reset_yscroll_callback)
{
	m_playfield_tilemap->set_scrolly(0, param);
}


void atarisy1_state::yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t const oldscroll = *m_yscroll;
	uint16_t newscroll = oldscroll;
	int const scanline = m_screen->vpos();

	/* force a partial update in case this changes mid-screen */
	COMBINE_DATA(&newscroll);
	m_screen->update_partial(scanline);

	/* because this latches a new value into the scroll base,
	   we need to adjust for the scanline */
	int adjusted_scroll = newscroll;
	if (scanline <= m_screen->visible_area().bottom())
		adjusted_scroll -= (scanline + 1);
	m_playfield_tilemap->set_scrolly(0, adjusted_scroll);

	/* but since we've adjusted it, we must reset it to the normal value
	   once we hit scanline 0 again */
	m_yscroll_reset_timer->adjust(m_screen->time_until_pos(0), newscroll);

	/* update the data */
	*m_yscroll = newscroll;
}



/*************************************
 *
 *  Sprite RAM write handler
 *
 *************************************/

void atarisy1_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const active_bank = m_mob->bank();
	uint16_t *spriteram = m_mob->spriteram();
	int const oldword = spriteram[offset];
	int newword = oldword;
	COMBINE_DATA(&newword);

	/* if the data changed, and it modified the live sprite bank, do some extra work */
	if (oldword != newword && (offset >> 8) == active_bank)
	{
		/* if modifying a timer, beware */
		if (((offset & 0xc0) == 0x00 && spriteram[offset | 0x40] == 0xffff) ||
			((offset & 0xc0) == 0x40 && (newword == 0xffff || oldword == 0xffff)))
		{
			/* if the timer is in the active bank, update the display list */
			spriteram[offset] = data;
			update_timers(m_screen->vpos());
		}

		/* if we're about to modify data in the active sprite bank, make sure the video is up-to-date */
		/* Road Runner needs this to work; note the +2 kludge -- +1 would be correct since the video */
		/* renders the next scanline's sprites to the line buffers, but Road Runner still glitches */
		/* without the extra +1 */
		else
			m_screen->update_partial(m_screen->vpos() + 2);
	}

	/* let the MO handler do the basic work */
	spriteram[offset] = data;
}



/*************************************
 *
 *  MO interrupt handlers
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_state::int3off_callback)
{
	// clear the state
	m_scanline_int_state = false;
	m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_state::int3_callback)
{
	int const scanline = param;

	// update the state
	m_scanline_int_state = true;
	m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);

	// set a timer to turn it off
	m_int3off_timer->adjust(m_screen->scan_period());

	// determine the time of the next one
	m_next_timer_scanline = -1;
	update_timers(scanline);
}



/*************************************
 *
 *  MO interrupt state read
 *
 *************************************/

uint16_t atarisy1_state::int3state_r()
{
	return m_scanline_int_state ? 0x0080 : 0x0000;
}



/*************************************
 *
 *  Timer updater
 *
 *************************************/

void atarisy1_state::update_timers(int scanline)
{
}

void atarisy1r_state::update_timers(int scanline)
{
	int const offset = m_mob->bank() * 64 * 4;
	int link = 0, best = scanline, found = 0;
	uint8_t spritevisit[64];

	/* track which ones we've visited */
	memset(spritevisit, 0, sizeof(spritevisit));

	/* walk the list until we loop */
	while (!spritevisit[link])
	{
		/* timers are indicated by 0xffff in entry 2 */
		if (m_mob->spriteram()[offset + link + 0x40] == 0xffff)
		{
			int const data = m_mob->spriteram()[offset + link];
			int const vsize = (data & 15) + 1;
			int const ypos = (256 - (data >> 5) - vsize * 8 - 1) & 0x1ff;

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
		link = m_mob->spriteram()[offset + link + 0xc0] & 0x3f;
	}

	/* if nothing was found, use scanline -1 */
	if (!found)
		best = -1;

	/* update the timer */
	if (best != m_next_timer_scanline)
	{
		m_next_timer_scanline = best;

		/* set a new one */
		if (best != -1)
			m_scanline_timer->adjust(m_screen->time_until_pos(best), best);
		else
			m_scanline_timer->reset();
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t atarisy1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* high priority MO? */
					if (mo[x] & atari_motion_objects_device::PRIORITY_MASK)
					{
						/* only gets priority if MO pen is not 1 */
						if ((mo[x] & 0x0f) != 1)
							pf[x] = 0x300 + ((pf[x] & 0x0f) << 4) + (mo[x] & 0x0f);
					}

					/* low priority */
					else
					{
						/* priority pens for playfield color 0 */
						if ((pf[x] & 0xf8) != 0 || !(m_playfield_priority_pens & (1 << (pf[x] & 0x07))))
							pf[x] = mo[x];
					}
				}
		}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

void atarisy1_state::decode_gfx(uint16_t *pflookup, uint16_t *molookup)
{
	uint8_t const *prom1 = &memregion("proms")->as_u8(0x000);
	uint8_t const *prom2 = &memregion("proms")->as_u8(0x200);

	/* reset the globals */
	memset(&m_bank_gfx[0][0], 0, sizeof(m_bank_gfx));

	/* loop for two sets of objects */
	for (int obj = 0; obj < 2; obj++)
	{
		/* loop for 256 objects in the set */
		for (int i = 0; i < 256; i++, prom1++, prom2++)
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
			bank = get_bank(*prom1, *prom2, bpp);

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

int atarisy1_state::get_bank(uint8_t prom1, uint8_t prom2, int bpp)
{
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
	if (m_bank_gfx[bpp - 4][bank_index])
		return m_bank_gfx[bpp - 4][bank_index];

	/* if the bank is out of range, call it 0 */
	memory_region *tiles = memregion("tiles");
	if (0x80000 * (bank_index - 1) >= tiles->bytes())
		return 0;

	/* don't have one? let's make it ... first find any empty slot */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;
	assert(gfx_index != MAX_GFX_ELEMENTS);

	/* decode the graphics */
	const uint8_t *srcdata = &tiles->as_u8(0x80000 * (bank_index - 1));
	switch (bpp)
	{
	case 4:
		m_gfxdecode->set_gfx(gfx_index,std::make_unique<gfx_element>(m_palette, objlayout_4bpp, srcdata, 0, 0x40, 256));
		break;

	case 5:
		m_gfxdecode->set_gfx(gfx_index,std::make_unique<gfx_element>(m_palette, objlayout_5bpp, srcdata, 0, 0x40, 256));
		break;

	case 6:
		m_gfxdecode->set_gfx(gfx_index,std::make_unique<gfx_element>(m_palette, objlayout_6bpp, srcdata, 0, 0x40, 256));
		break;

	default:
		fatalerror("Unsupported bpp\n");
	}

	/* set the color information */
	m_gfxdecode->gfx(gfx_index)->set_granularity(8);
	m_bank_color_shift[gfx_index] = bpp - 3;

	/* set the entry and return it */
	return m_bank_gfx[bpp - 4][bank_index] = gfx_index;
}
