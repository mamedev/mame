/***************************************************************************

    Atari G42 hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank

****************************************************************************/


#include "driver.h"
#include "machine/atarigen.h"
#include "atarig42.h"



/*************************************
 *
 *  Globals we own
 *
 *************************************/

UINT16 atarig42_playfield_base;
UINT16 atarig42_motion_object_base;
UINT16 atarig42_motion_object_mask;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT16 current_control;
static UINT8 playfield_tile_bank;
static UINT8 playfield_color_bank;
static UINT16 playfield_xscroll;
static UINT16 playfield_yscroll;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	UINT16 data = atarigen_alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data = atarigen_playfield[tile_index];
	int code = (playfield_tile_bank << 12) | (data & 0xfff);
	int color = (atarig42_playfield_base >> 5) + ((playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
	tileinfo->category = (playfield_color_bank >> 2) & 7;
}


static TILEMAP_MAPPER( atarig42_playfield_scan )
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig42 )
{
	static const struct atarirle_desc modesc =
	{
		REGION_GFX3,/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		0,			/* left clip coordinate */
		0,			/* right clip coordinate */

		0x000,		/* base palette entry */
		0x400,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x03f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0,0x00ff,0 }},	/* mask for the order */
		{{ 0,0x0e00,0,0,0,0,0,0 }},	/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};
	struct atarirle_desc adjusted_modesc = modesc;
	int i;

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x30);

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, atarig42_playfield_scan, TILEMAP_TYPE_PEN, 8,8, 128,64);

	/* initialize the motion objects */
	adjusted_modesc.palettebase = atarig42_motion_object_base;
	for (i = 0; i < 8; i++)
		adjusted_modesc.colormask.data[i] &= atarig42_motion_object_mask;
	atarirle_init(0, &adjusted_modesc);

	/* initialize the alphanumerics */
	atarigen_alpha_tilemap = tilemap_create(get_alpha_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
	tilemap_set_transparent_pen(atarigen_alpha_tilemap, 0);

	/* reset statics */
	current_control = 0;
	playfield_tile_bank = 0;
	playfield_color_bank = 0;
	playfield_xscroll = 0;
	playfield_yscroll = 0;
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig42_mo_control_w )
{
	logerror("MOCONT = %d (scan = %d)\n", data, video_screen_get_vpos(0));

	/* set the control value */
	COMBINE_DATA(&current_control);
}


void atarig42_scanline_update(running_machine *machine, int scrnum, int scanline)
{
	UINT16 *base = &atarigen_alpha[(scanline / 8) * 64 + 48];
	int i;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &atarigen_alpha[0x800])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = (word >> 5) & 0x3ff;
			int newbank = word & 0x1f;
			if (newscroll != playfield_xscroll)
			{
				video_screen_update_partial(0, scanline + i - 1);
				tilemap_set_scrollx(atarigen_playfield_tilemap, 0, newscroll);
				playfield_xscroll = newscroll;
			}
			if (newbank != playfield_color_bank)
			{
				video_screen_update_partial(0, scanline + i - 1);
				tilemap_mark_all_tiles_dirty(atarigen_playfield_tilemap);
				playfield_color_bank = newbank;
			}
		}

		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != playfield_yscroll)
			{
				video_screen_update_partial(0, scanline + i - 1);
				tilemap_set_scrolly(atarigen_playfield_tilemap, 0, newscroll);
				playfield_yscroll = newscroll;
			}
			if (newbank != playfield_tile_bank)
			{
				video_screen_update_partial(0, scanline + i - 1);
				tilemap_mark_all_tiles_dirty(atarigen_playfield_tilemap);
				playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( atarig42 )
{
	/* draw the playfield */
	fillbitmap(priority_bitmap, 0, cliprect);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 1, 1);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 2, 2);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 3, 3);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 4, 4);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 5, 5);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 6, 6);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 7, 7);

	/* copy the motion objects on top */
	{
		mame_bitmap *mo_bitmap = atarirle_get_vram(0, 0);
		int left	= cliprect->min_x;
		int top		= cliprect->min_y;
		int right	= cliprect->max_x + 1;
		int bottom	= cliprect->max_y + 1;
		int x, y;

		/* now blend with the playfield */
		for (y = top; y < bottom; y++)
		{
			UINT16 *pf = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			UINT16 *mo = (UINT16 *)mo_bitmap->base + y * mo_bitmap->rowpixels;
			UINT8 *pri = (UINT8 *)priority_bitmap->base + priority_bitmap->rowpixels * y;
			for (x = left; x < right; x++)
				if (mo[x])
				{
					int pfpri = pri[x];
					int mopri = mo[x] >> ATARIMO_PRIORITY_SHIFT;
					if (mopri >= pfpri)
						pf[x] = mo[x] & ATARIRLE_DATA_MASK;
				}
		}
	}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, atarigen_alpha_tilemap, 0, 0);
	return 0;
}
