/***************************************************************************

    Atari Food Fight hardware

****************************************************************************/

#include "driver.h"
#include "machine/atarigen.h"
#include "foodf.h"
#include "video/resnet.h"


static double rweights[3], gweights[3], bweights[2];
static UINT8 playfield_flip;



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data = atarigen_playfield[tile_index];
	int code = (data & 0xff) | ((data >> 7) & 0x100);
	int color = (data >> 8) & 0x3f;
	SET_TILE_INFO(0, code, color, playfield_flip ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( foodf )
{
	static const int resistances[3] = { 1000, 470, 220 };

	/* initialize the playfield */
	atarigen_playfield_tilemap = tilemap_create(get_playfield_tile_info, tilemap_scan_cols, TILEMAP_TYPE_PEN, 8,8, 32,32);
	tilemap_set_transparent_pen(atarigen_playfield_tilemap, 0);

	/* adjust the playfield for the 8 pixel offset */
	tilemap_set_scrollx(atarigen_playfield_tilemap, 0, -8);
	playfield_flip = 0;
	state_save_register_global(playfield_flip);

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], rweights, 0, 0,
			3,	&resistances[0], gweights, 0, 0,
			2,	&resistances[1], bweights, 0, 0);
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

void foodf_set_flip(int flip)
{
	if (flip != playfield_flip)
	{
		playfield_flip = flip;
		tilemap_mark_all_tiles_dirty(atarigen_playfield_tilemap);
	}
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE16_HANDLER( foodf_paletteram_w )
{
	int newword, r, g, b, bit0, bit1, bit2;

	COMBINE_DATA(&paletteram16[offset]);
	newword = paletteram16[offset];

	/* only the bottom 8 bits are used */
	/* red component */
	bit0 = (newword >> 0) & 0x01;
	bit1 = (newword >> 1) & 0x01;
	bit2 = (newword >> 2) & 0x01;
	r = combine_3_weights(rweights, bit0, bit1, bit2);

	/* green component */
	bit0 = (newword >> 3) & 0x01;
	bit1 = (newword >> 4) & 0x01;
	bit2 = (newword >> 5) & 0x01;
	g = combine_3_weights(gweights, bit0, bit1, bit2);

	/* blue component */
	bit0 = (newword >> 6) & 0x01;
	bit1 = (newword >> 7) & 0x01;
	b = combine_2_weights(bweights, bit0, bit1);

	palette_set_color(Machine, offset, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( foodf )
{
	int offs;

	/* first draw the playfield opaquely */
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, TILEMAP_DRAW_OPAQUE, 0);

	/* then draw the non-transparent parts with a priority of 1 */
	fillbitmap(priority_bitmap, 0, 0);
	tilemap_draw(bitmap, cliprect, atarigen_playfield_tilemap, 0, 1);

	/* draw the motion objects front-to-back */
	for (offs = 0x80-2; offs >= 0x20; offs -= 2)
	{
		int data1 = spriteram16[offs];
		int data2 = spriteram16[offs+1];

		int pict = data1 & 0xff;
		int color = (data1 >> 8) & 0x1f;
		int xpos = (data2 >> 8) & 0xff;
		int ypos = (0xff - data2 - 16) & 0xff;
		int hflip = (data1 >> 15) & 1;
		int vflip = (data1 >> 14) & 1;
		int pri = (data1 >> 13) & 1;

		pdrawgfx(bitmap, machine->gfx[1], pict, color, hflip, vflip,
				xpos, ypos, cliprect, TRANSPARENCY_PEN, 0, pri * 2);

		/* draw again with wraparound (needed to get the end of level animation right) */
		pdrawgfx(bitmap, machine->gfx[1], pict, color, hflip, vflip,
				xpos - 256, ypos, cliprect, TRANSPARENCY_PEN, 0, pri * 2);
	}

	return 0;
}
