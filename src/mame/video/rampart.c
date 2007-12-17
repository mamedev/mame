/***************************************************************************

    Atari Rampart hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "rampart.h"



/*************************************
 *
 *  Globals we own
 *
 *************************************/

UINT16 *rampart_bitmap;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 *pfdirty;
static mame_bitmap *pfbitmap;
static int xdim, ydim;



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( rampart )
{
	static const struct atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x00ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x0007 }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0,					/* callback routine for special entries */
	};

	/* initialize the playfield */
	rampart_bitmap_init(machine, 43*8, 30*8);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* set the intial scroll offset */
	atarimo_set_xscroll(0, -4);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( rampart )
{
	struct atarimo_rect_list rectlist;
	mame_bitmap *mobitmap;
	int x, y, r;

	/* draw the playfield */
	rampart_bitmap_render(machine, bitmap, cliprect);

	/* draw and merge the MO */
	mobitmap = atarimo_render(machine, 0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* not yet verified
                    */
					pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}



/*************************************
 *
 *  Bitmap initialization
 *
 *************************************/

void rampart_bitmap_init(running_machine *machine, int _xdim, int _ydim)
{
	/* set the dimensions */
	xdim = _xdim;
	ydim = _ydim;

	/* allocate dirty map */
	pfdirty = auto_malloc(sizeof(pfdirty[0]) * ydim);
	memset(pfdirty, 1, sizeof(pfdirty[0]) * ydim);

	/* allocate playfield bitmap */
	pfbitmap = auto_bitmap_alloc(xdim, ydim, machine->screen[0].format);
}



/*************************************
 *
 *  Bitmap RAM write handler
 *
 *************************************/

WRITE16_HANDLER( rampart_bitmap_w )
{
	int oldword = rampart_bitmap[offset];
	int newword = oldword;
	int x, y;

	COMBINE_DATA(&newword);
	if (oldword != newword)
	{
		rampart_bitmap[offset] = newword;

		/* track color usage */
		x = offset % 256;
		y = offset / 256;
		if (x < xdim && y < ydim)
			pfdirty[y] = 1;
	}
}



/*************************************
 *
 *  Bitmap rendering
 *
 *************************************/

void rampart_bitmap_render(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int x, y;

	/* update any dirty scanlines */
	for (y = 0; y < ydim; y++)
		if (pfdirty[y])
		{
			const UINT16 *src = &rampart_bitmap[256 * y];
			UINT8 scanline[512];
			UINT8 *dst = scanline;

			/* regenerate the line */
			for (x = 0; x < xdim / 2; x++)
			{
				int bits = *src++;
				*dst++ = bits >> 8;
				*dst++ = bits;
			}
			pfdirty[y] = 0;

			/* draw it */
			draw_scanline8(pfbitmap, 0, y, xdim, scanline, machine->pens, -1);
		}

	/* copy the cached bitmap */
	copybitmap(bitmap, pfbitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_NONE, 0);
}
