/***************************************************************************

    Atari ThunderJaws hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "includes/thunderj.h"



/*************************************
 *
 *  Globals
 *
 *************************************/

UINT8 thunderj_alpha_tile_bank;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	UINT16 data = atarigen_alpha[tile_index];
	int code = ((data & 0x200) ? (thunderj_alpha_tile_bank * 0x200) : 0) + (data & 0x1ff);
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data1 = atarigen_playfield[tile_index];
	UINT16 data2 = atarigen_playfield_upper[tile_index] & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO(0, code, color, (data1 >> 15) & 1);
	tileinfo->category = (data2 >> 4) & 3;
}


static TILE_GET_INFO( get_playfield2_tile_info )
{
	UINT16 data1 = atarigen_playfield2[tile_index];
	UINT16 data2 = atarigen_playfield_upper[tile_index] >> 8;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO(0, code, color, (data1 >> 15) & 1);
	tileinfo->category = (data2 >> 4) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( thunderj )
{
	static const struct atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		1,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x03ff,0,0,0 }},	/* mask for the link */
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
		{{ 0,0,0x0070,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		NULL				/* callback routine for special entries */
	};

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 8,8, 64,64);

	/* initialize the second playfield */
	atarigen_playfield2_tilemap = tilemap_create(get_playfield2_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 8,8, 64,64);
	tilemap_set_transparent_pen(atarigen_playfield2_tilemap, 0);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	atarigen_alpha_tilemap = tilemap_create(get_alpha_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
	tilemap_set_transparent_pen(atarigen_alpha_tilemap, 0);
}



/*************************************
 *
 *  Mark high palette bits starting
 *  at the given X,Y and continuing
 *  until a stop or the end of line
 *
 *************************************/

void thunderj_mark_high_palette(mame_bitmap *bitmap, UINT16 *pf, UINT16 *mo, int x, int y)
{
	#define START_MARKER	((4 << ATARIMO_PRIORITY_SHIFT) | 2)
	#define END_MARKER		((4 << ATARIMO_PRIORITY_SHIFT) | 4)
	int offnext = 0;

	for ( ; x < bitmap->width; x++)
	{
		pf[x] |= 0x400;
		if (offnext && (mo[x] & START_MARKER) != START_MARKER)
			break;
		offnext = ((mo[x] & END_MARKER) == END_MARKER);
	}
 }



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( thunderj )
{
	struct atarimo_rect_list rectlist;
	mame_bitmap *mobitmap;
	int x, y, r;

	/* draw the playfield */
	fillbitmap(priority_bitmap, 0, cliprect);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 0, 0x00);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 1, 0x01);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 2, 0x02);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 3, 0x03);
	tilemap_draw(bitmap, cliprect, atarigen_playfield2_tilemap, 0, 0x80);
	tilemap_draw(bitmap, cliprect, atarigen_playfield2_tilemap, 1, 0x84);
	tilemap_draw(bitmap, cliprect, atarigen_playfield2_tilemap, 2, 0x88);
	tilemap_draw(bitmap, cliprect, atarigen_playfield2_tilemap, 3, 0x8c);

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
                     *      PF/M=MPX0*!MPX1*!MPX2*!MPX3*!MPX4*!MPX5*!MPX6*!MPX7
                     *          +PFX3*PFX8*PFX9*!MPR0
                     *          +PFX3*PFX8*!MPR0*!MPR1
                     *          +PFX3*PFX9*!MPR1
                     *
                     *      --- CS1 is 1 if the playfield should be displayed
                     *      CS1=PF/M*!ALBG*!APIX0*!APIX1
                     *         +!MPX0*!MPX1*!MPX2*!MPX3*!ALBG*!APIX0*!APIX1
                     *
                     *      --- CS0 is 1 if the MOs should be displayed
                     *      CS0=!PF/M*MPX0*!ALBG*!APIX0*!APIX1
                     *         +!PF/M*MPX1*!ALBG*!APIX0*!APIX1
                     *         +!PF/M*MPX2*!ALBG*!APIX0*!APIX1
                     *         +!PF/M*MPX3*!ALBG*!APIX0*!APIX1
                     *
                     *      --- CRA10 is the 0x200 bit of the color RAM index; set if pf is displayed
                     *      CRA10:=CS1
                     *
                     *      --- CRA9 is the 0x100 bit of the color RAM index; set if mo is displayed
                     *          or if the playfield selected is playfield #2
                     *      CRA9:=PFXS*CS1
                     *          +!CS1*CS0
                     *
                     *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
                     *      CRA8:=CS1*PFX7
                     *          +!CS1*MPX7*CS0
                     *          +!CS1*!CS0*ALC4
                     *
                     *      CRA7:=CS1*PFX6
                     *          +!CS1*MPX6*CS0
                     *
                     *      CRA6:=CS1*PFX5
                     *          +MPX5*!CS1*CS0
                     *          +!CS1*!CS0*ALC3
                     *
                     *      CRA5:=CS1*PFX4
                     *          +MPX4*!CS1*CS0
                     *          +!CS1*ALC2*!CS0
                     *
                     *      CRA4:=CS1*PFX3
                     *          +!CS1*MPX3*CS0
                     *          +!CS1*!CS0*ALC1
                     *
                     *      CRA3:=CS1*PFX2
                     *          +MPX2*!CS1*CS0
                     *          +!CS1*!CS0*ALC0
                     *
                     *      CRA2:=CS1*PFX1
                     *          +MPX1*!CS1*CS0
                     *          +!CS1*!CS0*APIX1
                     *
                     *      CRA1:=CS1*PFX0
                     *          +MPX0*!CS1*CS0
                     *          +!CS1*!CS0*APIX0
                     */
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;
					int pfm = 0;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* determine pf/m signal */
					if ((mo[x] & 0xff) == 1)
						pfm = 1;
					else if (pf[x] & 8)
					{
						int pfpriority = (pri[x] & 0x80) ? ((pri[x] >> 2) & 3) : (pri[x] & 3);
						if (((pfpriority == 3) && !(mopriority & 1)) ||
							((pfpriority & 1) && (mopriority == 0)) ||
							((pfpriority & 2) && !(mopriority & 2)))
							pfm = 1;
					}

					/* if pfm is low, we display the mo */
					if (!pfm)
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, atarigen_alpha_tilemap, 0, 0);

	/* now go back and process the upper bit of MO priority */
	rectlist.rect -= rectlist.numrects;
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* upper bit of MO priority might mean palette kludges */
					if (mopriority & 4)
					{
						/* if bit 2 is set, start setting high palette bits */
						if (mo[x] & 2)
							thunderj_mark_high_palette(bitmap, pf, mo, x, y);
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
