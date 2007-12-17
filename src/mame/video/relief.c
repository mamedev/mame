/***************************************************************************

    Atari "Round" hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "relief.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data1 = atarigen_playfield[tile_index];
	UINT16 data2 = atarigen_playfield_upper[tile_index] & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x20 + (data2 & 0x0f);
	SET_TILE_INFO(0, code, color, (data1 >> 15) & 1);
}


static TILE_GET_INFO( get_playfield2_tile_info )
{
	UINT16 data1 = atarigen_playfield2[tile_index];
	UINT16 data2 = atarigen_playfield_upper[tile_index] >> 8;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( relief )
{
	static const struct atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
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
		0					/* callback routine for special entries */
	};

	/* blend the MO graphics */
	atarigen_blend_gfx(machine, 1, 2, 0x0f, 0x10);

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 8,8, 64,64);

	/* initialize the second playfield */
	atarigen_playfield2_tilemap = tilemap_create(get_playfield2_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 8,8, 64,64);
	tilemap_set_transparent_pen(atarigen_playfield2_tilemap, 0);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( relief )
{
	struct atarimo_rect_list rectlist;
	mame_bitmap *mobitmap;
	int x, y, r;

	/* draw the playfield */
	fillbitmap(priority_bitmap, 0, cliprect);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, atarigen_playfield2_tilemap, 0, 1);

	/* draw and merge the MO */
	mobitmap = atarimo_render(machine, 0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			UINT8 *pri = (UINT8 *)priority_bitmap->base + priority_bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* verified from the GALs on the real PCB; equations follow
                     *
                     *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
                     *      PF/M = PFXS
                     *
                     *      --- CS0 is set to 1 if the MO is transparent
                     *      CS0=!MPX0*!MPX1*!MPX2*!MPX3
                     *
                     *      --- CS1 is 1 to select playfield pixels or 0 to select MO pixels
                     *      !CS1=MPX5*MPX6*MPX7*!CS0
                     *          +!MPX4*MPX5*MPX6*MPX7
                     *          +PFXS*!CS0
                     *          +!MPX4*PFXS
                     *
                     *      --- CRA10 is the 0x200 bit of the color RAM index; set for the top playfield only
                     *      CRA10:=CS1*PFXS
                     *
                     *      --- CRA9 is the 0x100 bit of the color RAM index; set for MOs only
                     *      !CA9:=CS1
                     *
                     *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
                     */
					int cs0 = 0;
					int cs1 = 1;

					/* compute the CS0 signal */
					cs0 = ((mo[x] & 0x0f) == 0);

					/* compute the CS1 signal */
					if ((!cs0 && (mo[x] & 0xe0) == 0xe0) ||
						((mo[x] & 0xf0) == 0xe0) ||
						(!pri[x] && !cs0) ||
						(!pri[x] && !(mo[x] & 0x10)))
						cs1 = 0;

					/* MO is displayed if cs1 == 0 */
					if (!cs1)
						pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
