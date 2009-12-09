/***************************************************************************

    Atari Food Fight hardware

****************************************************************************/

#include "driver.h"
#include "foodf.h"
#include "video/resnet.h"


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_playfield_tile_info )
{
	foodf_state *state = (foodf_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield[tile_index];
	int code = (data & 0xff) | ((data >> 7) & 0x100);
	int color = (data >> 8) & 0x3f;
	SET_TILE_INFO(0, code, color, state->playfield_flip ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( foodf )
{
	static const int resistances[3] = { 1000, 470, 220 };
	foodf_state *state = (foodf_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_cols,  8,8, 32,32);
	tilemap_set_transparent_pen(state->atarigen.playfield_tilemap, 0);

	/* adjust the playfield for the 8 pixel offset */
	tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, -8);
	state_save_register_global(machine, state->playfield_flip);

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], state->rweights, 0, 0,
			3,	&resistances[0], state->gweights, 0, 0,
			2,	&resistances[1], state->bweights, 0, 0);
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

void foodf_set_flip(foodf_state *state, int flip)
{
	if (flip != state->playfield_flip)
	{
		state->playfield_flip = flip;
		tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
	}
}



/*************************************
 *
 *  Palette RAM write
 *
 *************************************/

WRITE16_HANDLER( foodf_paletteram_w )
{
	foodf_state *state = (foodf_state *)space->machine->driver_data;
	int newword, r, g, b, bit0, bit1, bit2;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];

	/* only the bottom 8 bits are used */
	/* red component */
	bit0 = (newword >> 0) & 0x01;
	bit1 = (newword >> 1) & 0x01;
	bit2 = (newword >> 2) & 0x01;
	r = combine_3_weights(state->rweights, bit0, bit1, bit2);

	/* green component */
	bit0 = (newword >> 3) & 0x01;
	bit1 = (newword >> 4) & 0x01;
	bit2 = (newword >> 5) & 0x01;
	g = combine_3_weights(state->gweights, bit0, bit1, bit2);

	/* blue component */
	bit0 = (newword >> 6) & 0x01;
	bit1 = (newword >> 7) & 0x01;
	b = combine_2_weights(state->bweights, bit0, bit1);

	palette_set_color(space->machine, offset, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( foodf )
{
	foodf_state *state = (foodf_state *)screen->machine->driver_data;
	int offs;
	const gfx_element *gfx = screen->machine->gfx[1];
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;
	UINT16 *spriteram16 = screen->machine->generic.spriteram.u16;

	/* first draw the playfield opaquely */
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, TILEMAP_DRAW_OPAQUE, 0);

	/* then draw the non-transparent parts with a priority of 1 */
	bitmap_fill(priority_bitmap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 0, 1);

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

		pdrawgfx_transpen(bitmap, cliprect, gfx, pict, color, hflip, vflip,
				xpos, ypos, priority_bitmap, pri * 2, 0);

		/* draw again with wraparound (needed to get the end of level animation right) */
		pdrawgfx_transpen(bitmap, cliprect, gfx, pict, color, hflip, vflip,
				xpos - 256, ypos, priority_bitmap, pri * 2, 0);
	}

	return 0;
}
