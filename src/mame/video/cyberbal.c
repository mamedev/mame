/***************************************************************************

    Atari Cyberball hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "cyberbal.h"


#define SCREEN_WIDTH		(42*16)



/*************************************
 *
 *  Globals we own
 *
 *************************************/

UINT16 *cyberbal_paletteram_0;
UINT16 *cyberbal_paletteram_1;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 current_screen;
static UINT8 total_screens;
static UINT16 current_slip[2];
static UINT8 playfield_palette_bank[2];
static UINT16 playfield_xscroll[2];
static UINT16 playfield_yscroll[2];



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	UINT16 data = atarigen_alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO(2, code, color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_alpha2_tile_info )
{
	UINT16 data = atarigen_alpha2[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x07;
	SET_TILE_INFO(2, code, 0x80 | color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data = atarigen_playfield[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}


static TILE_GET_INFO( get_playfield2_tile_info )
{
	UINT16 data = atarigen_playfield2[tile_index];
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
	static const struct atarimo_desc mo0desc =
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

	static const struct atarimo_desc mo1desc =
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

	/* set the slip variables */
	atarimo_0_slipram = &current_slip[0];
	atarimo_1_slipram = &current_slip[1];

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &mo0desc);

	/* initialize the alphanumerics */
	atarigen_alpha_tilemap = tilemap_create(get_alpha_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,8, 64,32);
	tilemap_set_transparent_pen(atarigen_alpha_tilemap, 0);

	/* allocate the second screen if necessary */
	if (screens == 2)
	{
		/* initialize the playfield */
		atarigen_playfield2_tilemap = tilemap_create(get_playfield2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,8, 64,64);
		tilemap_set_scrollx(atarigen_playfield2_tilemap, 0, 0);

		/* initialize the motion objects */
		atarimo_init(machine, 1, &mo1desc);

		/* initialize the alphanumerics */
		atarigen_alpha2_tilemap = tilemap_create(get_alpha2_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 16,8, 64,32);
		tilemap_set_scrollx(atarigen_alpha2_tilemap, 0, 0);
		tilemap_set_transparent_pen(atarigen_alpha2_tilemap, 0);
	}

	/* reset statics */
	current_slip[0] = 0;
	current_slip[1] = 0;
	total_screens = screens;
	current_screen = 0;
}


VIDEO_START( cyberbal )
{
	video_start_cyberbal_common(machine, 2);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 4);
	atarimo_set_xscroll(1, 4);
}


VIDEO_START( cyberb2p )
{
	video_start_cyberbal_common(machine, 1);

	/* adjust the sprite positions */
	atarimo_set_xscroll(0, 5);
}



/*************************************
 *
 *  Screen switcher
 *
 *************************************/

void cyberbal_set_screen(int which)
{
	if (which < total_screens)
		current_screen = which;
}



/*************************************
 *
 *  Palette tweaker
 *
 *************************************/

INLINE void set_palette_entry(int entry, UINT16 value)
{
	int r, g, b;

	r = ((value >> 9) & 0x3e) | ((value >> 15) & 1);
	g = ((value >> 4) & 0x3e) | ((value >> 15) & 1);
	b = ((value << 1) & 0x3e) | ((value >> 15) & 1);

	palette_set_color_rgb(Machine, entry, pal6bit(r), pal6bit(g), pal6bit(b));
}



/*************************************
 *
 *  Palette RAM write handlers
 *
 *************************************/

WRITE16_HANDLER( cyberbal_paletteram_0_w )
{
	COMBINE_DATA(&cyberbal_paletteram_0[offset]);
	set_palette_entry(offset, cyberbal_paletteram_0[offset]);
}

READ16_HANDLER( cyberbal_paletteram_0_r )
{
	return cyberbal_paletteram_0[offset];
}


WRITE16_HANDLER( cyberbal_paletteram_1_w )
{
	COMBINE_DATA(&cyberbal_paletteram_1[offset]);
	set_palette_entry(offset + 0x800, cyberbal_paletteram_1[offset]);
}

READ16_HANDLER( cyberbal_paletteram_1_r )
{
	return cyberbal_paletteram_1[offset];
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void cyberbal_scanline_update(running_machine *machine, int scrnum, int scanline)
{
	int i;

	/* loop over screens */
	for (i = 0; i < total_screens; i++)
	{
		UINT16 *vram = i ? atarigen_alpha2 : atarigen_alpha;
		UINT16 *base = &vram[((scanline - 8) / 8) * 64 + 47];
		int update_screen = i;

		/* keep in range */
		if (base < vram)
			base += 0x800;
		else if (base >= &vram[0x800])
			return;

		/* update the current parameters */
		if (!(base[3] & 1))
		{
			if (((base[3] >> 1) & 7) != playfield_palette_bank[i])
			{
				video_screen_update_partial(update_screen, scanline - 1);
				playfield_palette_bank[i] = (base[3] >> 1) & 7;
				tilemap_set_palette_offset(i ? atarigen_playfield2_tilemap : atarigen_playfield_tilemap, playfield_palette_bank[i] << 8);
			}
		}
		if (!(base[4] & 1))
		{
			int newscroll = 2 * (((base[4] >> 7) + 4) & 0x1ff);
			if (newscroll != playfield_xscroll[i])
			{
				video_screen_update_partial(update_screen, scanline - 1);
				tilemap_set_scrollx(i ? atarigen_playfield2_tilemap : atarigen_playfield_tilemap, 0, newscroll);
				playfield_xscroll[i] = newscroll;
			}
		}
		if (!(base[5] & 1))
		{
			/* a new vscroll latches the offset into a counter; we must adjust for this */
			int newscroll = ((base[5] >> 7) - (scanline)) & 0x1ff;
			if (newscroll != playfield_yscroll[i])
			{
				video_screen_update_partial(update_screen, scanline - 1);
				tilemap_set_scrolly(i ? atarigen_playfield2_tilemap : atarigen_playfield_tilemap, 0, newscroll);
				playfield_yscroll[i] = newscroll;
			}
		}
		if (!(base[7] & 1))
		{
			if (current_slip[i] != base[7])
			{
				video_screen_update_partial(update_screen, scanline - 1);
				current_slip[i] = base[7];
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

static void update_one_screen(running_machine* machine, int screen, mame_bitmap *bitmap, const rectangle *cliprect)
{
	struct atarimo_rect_list rectlist;
	rectangle tempclip = *cliprect;
	mame_bitmap *mobitmap;
	int x, y, r, mooffset, temp;

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, screen ? atarigen_playfield2_tilemap : atarigen_playfield_tilemap, 0, 0);

	/* draw the MOs -- note some kludging to get this to work correctly for 2 screens */
	mooffset = 0;
	tempclip.min_x -= mooffset;
	tempclip.max_x -= mooffset;
	temp = machine->screen[screen].visarea.max_x;
	if (temp > SCREEN_WIDTH)
		machine->screen[screen].visarea.max_x /= 2;
	mobitmap = atarimo_render(machine, screen, cliprect, &rectlist);
	tempclip.min_x += mooffset;
	tempclip.max_x += mooffset;
	machine->screen[screen].visarea.max_x = temp;

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
	tilemap_draw(bitmap, cliprect, screen ? atarigen_alpha2_tilemap : atarigen_alpha_tilemap, 0, 0);
}


VIDEO_UPDATE( cyberbal )
{
	update_one_screen(machine, screen, bitmap, cliprect);
	return 0;
}
