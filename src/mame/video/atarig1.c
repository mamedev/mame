/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "atarig1.h"



/*************************************
 *
 *  Globals we own
 *
 *************************************/

UINT8 atarig1_pitfight;



/*************************************
 *
 *  Statics
 *
 *************************************/

static int pfscroll_xoffset;
static UINT16 current_control;
static UINT8 playfield_tile_bank;
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
	int color = (data >> 12) & 7;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig1 )
{
	static const struct atarirle_desc modesc_hydra =
	{
		REGION_GFX3,/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		0,			/* left clip coordinate */
		255,		/* right clip coordinate */

		0x200,		/* base palette entry */
		0x100,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x00f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0x00ff,0,0 }},	/* mask for the order */
		{{ 0 }},					/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};

	static const struct atarirle_desc modesc_pitfight =
	{
		REGION_GFX3,/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		40,			/* left clip coordinate */
		295,		/* right clip coordinate */

		0x200,		/* base palette entry */
		0x100,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x00f0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0,0x00ff,0 }},	/* mask for the order */
		{{ 0 }},					/* mask for the priority */
		{{ 0 }}						/* mask for the VRAM target */
	};

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x10);

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,64);

	/* initialize the motion objects */
	atarirle_init(0, atarig1_pitfight ? &modesc_pitfight : &modesc_hydra);

	/* initialize the alphanumerics */
	atarigen_alpha_tilemap = tilemap_create(get_alpha_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
	tilemap_set_transparent_pen(atarigen_alpha_tilemap, 0);

	/* reset statics */
	current_control = 0;
	pfscroll_xoffset = atarig1_pitfight ? 2 : 0;
	playfield_tile_bank = 0;
	playfield_xscroll = 0;
	playfield_yscroll = 0;
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig1_mo_control_w )
{
	logerror("MOCONT = %d (scan = %d)\n", data, video_screen_get_vpos(0));

	/* set the control value */
	COMBINE_DATA(&current_control);
}


void atarig1_scanline_update(running_machine *machine, int scrnum, int scanline)
{
	UINT16 *base = &atarigen_alpha[(scanline / 8) * 64 + 48];
	int i;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &atarigen_alpha[0x800])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		/* first word controls horizontal scroll */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) + pfscroll_xoffset) & 0x1ff;
			if (newscroll != playfield_xscroll)
			{
				video_screen_update_partial(0, scanline + i - 1);
				tilemap_set_scrollx(atarigen_playfield_tilemap, 0, newscroll);
				playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
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

VIDEO_UPDATE( atarig1 )
{
	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 0, 0);

	/* copy the motion objects on top */
	copybitmap(bitmap, atarirle_get_vram(0, 0), 0, 0, 0, 0, cliprect, TRANSPARENCY_PEN, 0);

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, atarigen_alpha_tilemap, 0, 0);
	return 0;
}
