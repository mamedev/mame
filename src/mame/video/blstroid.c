/***************************************************************************

    Atari Blasteroids hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "includes/blstroid.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(blstroid_state::get_playfield_tile_info)
{
	UINT16 data = m_playfield[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 13) & 0x07;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(blstroid_state,blstroid)
{
	static const atarimo_desc modesc =
	{
		1,                  /* index to which gfx system */
		1,                  /* number of motion object banks */
		1,                  /* are the entries linked? */
		0,                  /* are the entries split? */
		0,                  /* render in reverse order? */
		0,                  /* render in swapped X/Y order? */
		0,                  /* does the neighbor bit affect the next object? */
		0,                  /* pixels per SLIP entry (0 for no-slip) */
		0,                  /* pixel offset for SLIPs */
		0,                  /* maximum number of links to visit/scanline (0=all) */

		0x000,              /* base palette entry */
		0x100,              /* maximum number of colors */
		0,                  /* transparent pen index */

		{{ 0,0,0x0ff8,0 }}, /* mask for the link */
		{{ 0 }},            /* mask for the graphics bank */
		{{ 0,0x3fff,0,0 }}, /* mask for the code index */
		{{ 0 }},            /* mask for the upper code index */
		{{ 0,0,0,0x000f }}, /* mask for the color */
		{{ 0,0,0,0xffc0 }}, /* mask for the X position */
		{{ 0xff80,0,0,0 }}, /* mask for the Y position */
		{{ 0 }},            /* mask for the width, in tiles*/
		{{ 0x000f,0,0,0 }}, /* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
		{{ 0,0x4000,0,0 }}, /* mask for the vertical flip */
		{{ 0 }},            /* mask for the priority */
		{{ 0 }},            /* mask for the neighbor */
		{{ 0 }},            /* mask for absolute coordinates */

		{{ 0 }},            /* mask for the special value */
		0,                  /* resulting value to indicate "special" */
		0                   /* callback routine for special entries */
	};

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(blstroid_state::get_playfield_tile_info),this), TILEMAP_SCAN_ROWS,  16,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

TIMER_CALLBACK_MEMBER(blstroid_state::irq_off)
{
	/* clear the interrupt */
	address_space &space = subdevice("maincpu")->memory().space(AS_PROGRAM);
	scanline_int_ack_w(space, 0, 0);
}


TIMER_CALLBACK_MEMBER(blstroid_state::irq_on)
{
	/* generate the interrupt */
	scanline_int_gen(*subdevice("maincpu"));
	update_interrupts();
}


void blstroid_state::scanline_update(screen_device &screen, int scanline)
{
	int offset = (scanline / 8) * 64 + 40;

	/* check for interrupts */
	if (offset < 0x1000)
		if (m_playfield[offset] & 0x8000)
		{
			int width, vpos;
			attotime period_on;
			attotime period_off;

			/* FIXME: - the only thing this IRQ does it tweak the starting MO link */
			/* unfortunately, it does it too early for the given MOs! */
			/* perhaps it is not actually hooked up on the real PCB... */
			return;

			/* set a timer to turn the interrupt on at HBLANK of the 7th scanline */
			/* and another to turn it off one scanline later */
			width = screen.width();
			vpos  = screen.vpos();
			period_on  = screen.time_until_pos(vpos + 7, width * 0.9);
			period_off = screen.time_until_pos(vpos + 8, width * 0.9);

			screen.machine().scheduler().timer_set(period_on, timer_expired_delegate(FUNC(blstroid_state::irq_on), this));
			screen.machine().scheduler().timer_set(period_off, timer_expired_delegate(FUNC(blstroid_state::irq_off), this));
		}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 blstroid_state::screen_update_blstroid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);

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
					/* verified via schematics

					    priority address = HPPPMMMM
					*/
					int priaddr = ((pf[x] & 8) << 4) | (pf[x] & 0x70) | ((mo[x] & 0xf0) >> 4);
					if (m_priorityram[priaddr] & 1)
						pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
