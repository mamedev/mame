/***************************************************************************

    System 16 / 18 bootleg video

    Bugs to check against real HW

    System16A Tilemap bootlegs

    - Shinobi (Datsu bootleg) has a black bar down the right hand side,
      which turns red on the high score table.  This is in the Text layer.
      According to other games this should be the correct alignment for
      this bootleg HW.

    - After inserting a coin in Passing Shot (2p version) the layers are
      misaligned by 1 pixel (happens on non-bootlegs too)

    - Causing a 'fault' in the Passing Shot 2p bootleg (not hitting the ball
      on your serve) causes the tilemaps to be erased and not updated
      properly (mirroring?, bootleg protection?, missed case in bootleg?)

    System18 Tilemap bootlegs

    - Alien Storm Credit text for Player 1 is not displayed after inserting
      a coin.

***************************************************************************/
#include "emu.h"
#include "includes/system16.h"
#include "video/resnet.h"
#include "video/segaic16.h"


static void setup_system16_bootleg_spritebanking( running_machine* machine )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	if (state->spritebank_type == 1)
	{
		static const UINT8 default_banklist[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		int i;
		for (i = 0; i < 16; i++)
			segaic16_sprites_set_bank(machine, 0, i, default_banklist[i]);
	}
	else
	{
		static const UINT8 alternate_banklist[] = { 0,255,255,255, 255,255,255,3, 255,255,255,2, 255,1,0,255 };
		int i;
		for (i = 0; i < 16; i++)
			segaic16_sprites_set_bank(machine, 0, i, alternate_banklist[i]);

	}

}


#define MAXCOLOURS 0x2000 /* 8192 */


/***************************************************************************/

static TILEMAP_MAPPER( sys16_bg_map )
{
	int page = 0;
	if (row < 32)
	{
		/* top */
		if (col < 64)
			page = 0;
		else
			page = 1;
	}
	else
	{
		/* bottom */
		if (col < 64)
			page = 2;
		else
			page = 3;
	}

	row = row % 32;
	col = col % 64;
	return page * 64 * 32 + row * 64 + col;
}

static TILEMAP_MAPPER( sys16_text_map )
{
	return row * 64 + col + (64 - 40);
}


/*
    Color generation details

    Each color is made up of 5 bits, connected through one or more resistors like so:

    Bit 0 = 1 x 3.9K ohm
    Bit 1 = 1 x 2.0K ohm
    Bit 2 = 1 x 1.0K ohm
    Bit 3 = 2 x 1.0K ohm
    Bit 4 = 4 x 1.0K ohm

    Another data bit is connected by a tristate buffer to the color output through a 470 ohm resistor.
    The buffer allows the resistor to have no effect (tristate), halve brightness (pull-down) or double brightness (pull-up).
    The data bit source is a PPI pin in some of the earlier hardware (Hang-On, Pre-System 16) or bit 15 of each
    color RAM entry (Space Harrier, System 16B and most later boards).
*/

static const int resistances_normal[6] = {3900, 2000, 1000, 1000/2, 1000/4, 0};
static const int resistances_sh[6] = {3900, 2000, 1000, 1000/2, 1000/4, 470};

#ifdef UNUSED_CODE
WRITE16_HANDLER( sys16_paletteram_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	UINT16 newword;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];

	/*  sBGR BBBB GGGG RRRR */
	/*  x000 4321 4321 4321 */
	{
		int r, g, b, rs, gs, bs, rh, gh, bh;
		int r0 = (newword >> 12) & 1;
		int r1 = (newword >>  0) & 1;
		int r2 = (newword >>  1) & 1;
		int r3 = (newword >>  2) & 1;
		int r4 = (newword >>  3) & 1;
		int g0 = (newword >> 13) & 1;
		int g1 = (newword >>  4) & 1;
		int g2 = (newword >>  5) & 1;
		int g3 = (newword >>  6) & 1;
		int g4 = (newword >>  7) & 1;
		int b0 = (newword >> 14) & 1;
		int b1 = (newword >>  8) & 1;
		int b2 = (newword >>  9) & 1;
		int b3 = (newword >> 10) & 1;
		int b4 = (newword >> 11) & 1;

		/* Normal colors */
		r = combine_6_weights(state->weights[0][0], r0, r1, r2, r3, r4, 0);
		g = combine_6_weights(state->weights[0][1], g0, g1, g2, g3, g4, 0);
		b = combine_6_weights(state->weights[0][2], b0, b1, b2, b3, b4, 0);

		/* Shadow colors */
		rs = combine_6_weights(state->weights[1][0], r0, r1, r2, r3, r4, 0);
		gs = combine_6_weights(state->weights[1][1], g0, g1, g2, g3, g4, 0);
		bs = combine_6_weights(state->weights[1][2], b0, b1, b2, b3, b4, 0);

		/* Highlight colors */
		rh = combine_6_weights(state->weights[1][0], r0, r1, r2, r3, r4, 1);
		gh = combine_6_weights(state->weights[1][1], g0, g1, g2, g3, g4, 1);
		bh = combine_6_weights(state->weights[1][2], b0, b1, b2, b3, b4, 1);

		palette_set_color(space->machine, offset, MAKE_RGB(r, g, b) );

		palette_set_color(space->machine, offset + space->machine->config->total_colors/2, MAKE_RGB(rs,gs,bs));
	}
}
#endif

static void update_page( running_machine *machine )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	int all_dirty = 0;
	int i, offset;

	if (state->old_tile_bank1 != state->tile_bank1)
	{
		all_dirty = 1;
		state->old_tile_bank1 = state->tile_bank1;
	}

	if (state->old_tile_bank0 != state->tile_bank0)
	{
		all_dirty = 1;
		state->old_tile_bank0 = state->tile_bank0;
		tilemap_mark_all_tiles_dirty(state->text_layer);
	}

	if (all_dirty)
	{
		tilemap_mark_all_tiles_dirty(state->background);
		tilemap_mark_all_tiles_dirty(state->foreground);

		if (state->system18)
		{
			tilemap_mark_all_tiles_dirty(state->background2);
			tilemap_mark_all_tiles_dirty(state->foreground2);
		}
	}
	else {
		for (i = 0; i < 4; i++)
		{
			int page0 = 64 * 32 * i;
			if (state->old_bg_page[i] != state->bg_page[i])
			{
				state->old_bg_page[i] = state->bg_page[i];
				for (offset = page0; offset < page0 + 64 * 32; offset++)
				{
					tilemap_mark_tile_dirty(state->background, offset);
				}
			}

			if (state->old_fg_page[i] != state->fg_page[i])
			{
				state->old_fg_page[i] = state->fg_page[i];
				for (offset = page0; offset < page0 + 64 * 32; offset++)
				{
					tilemap_mark_tile_dirty(state->foreground, offset);
				}
			}

			if (state->system18)
			{
				if (state->old_bg2_page[i] != state->bg2_page[i])
				{
					state->old_bg2_page[i] = state->bg2_page[i];
					for (offset = page0; offset < page0 + 64 * 32; offset++)
					{
						tilemap_mark_tile_dirty(state->background2, offset);
					}
				}

				if (state->old_fg2_page[i] != state->fg2_page[i])
				{
					state->old_fg2_page[i] = state->fg2_page[i];
					for (offset = page0; offset < page0 + 64 * 32; offset++)
					{
						tilemap_mark_tile_dirty(state->foreground2, offset);
					}
				}
			}
		}
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	const UINT16 *source = 64 * 32 * state->bg_page[tile_index / (64 * 32)] + state->tileram;
	int data = source[tile_index%(64*32)];
	int tile_number = (data & 0xfff) + 0x1000 * ((data & state->tilebank_switch) ? state->tile_bank1 : state->tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	const UINT16 *source = 64 * 32 * state->fg_page[tile_index / (64 * 32)] + state->tileram;
	int data = source[tile_index % (64 * 32)];
	int tile_number = (data & 0xfff) + 0x1000 * ((data & state->tilebank_switch) ? state->tile_bank1 : state->tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	const UINT16 *source = 64 * 32 * state->bg2_page[tile_index / (64 * 32)] + state->tileram;
	int data = source[tile_index % (64 * 32)];
	int tile_number = (data & 0xfff) + 0x1000 * ((data & 0x1000) ? state->tile_bank1 : state->tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

static TILE_GET_INFO( get_fg2_tile_info )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	const UINT16 *source = 64 * 32 * state->fg2_page[tile_index / (64 * 32)] + state->tileram;
	int data = source[tile_index % (64 * 32)];
	int tile_number = (data & 0xfff) + 0x1000 * ((data & 0x1000) ? state->tile_bank1 : state->tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

WRITE16_HANDLER( sys16_tileram_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	UINT16 oldword = state->tileram[offset];

	COMBINE_DATA(&state->tileram[offset]);

	if (oldword != state->tileram[offset])
	{
		int page = offset / (64 * 32);
		offset = offset % (64 * 32);

		if (state->bg_page[0] == page) tilemap_mark_tile_dirty(state->background, offset + 64 * 32 * 0);
		if (state->bg_page[1] == page) tilemap_mark_tile_dirty(state->background, offset + 64 * 32 * 1);
		if (state->bg_page[2] == page) tilemap_mark_tile_dirty(state->background, offset + 64 * 32 * 2);
		if (state->bg_page[3] == page) tilemap_mark_tile_dirty(state->background, offset + 64 * 32 * 3);

		if (state->fg_page[0] == page) tilemap_mark_tile_dirty(state->foreground, offset + 64 * 32 * 0);
		if (state->fg_page[1] == page) tilemap_mark_tile_dirty(state->foreground, offset + 64 * 32 * 1);
		if (state->fg_page[2] == page) tilemap_mark_tile_dirty(state->foreground, offset + 64 * 32 * 2);
		if (state->fg_page[3] == page) tilemap_mark_tile_dirty(state->foreground, offset + 64 * 32 * 3);

		if (state->system18)
		{
			if (state->bg2_page[0] == page) tilemap_mark_tile_dirty(state->background2, offset + 64 * 32 * 0);
			if (state->bg2_page[1] == page) tilemap_mark_tile_dirty(state->background2, offset + 64 * 32 * 1);
			if (state->bg2_page[2] == page) tilemap_mark_tile_dirty(state->background2, offset + 64 * 32 * 2);
			if (state->bg2_page[3] == page) tilemap_mark_tile_dirty(state->background2, offset + 64 * 32 * 3);

			if (state->fg2_page[0] == page) tilemap_mark_tile_dirty(state->foreground2, offset + 64 * 32 * 0);
			if (state->fg2_page[1] == page) tilemap_mark_tile_dirty(state->foreground2, offset + 64 * 32 * 1);
			if (state->fg2_page[2] == page) tilemap_mark_tile_dirty(state->foreground2, offset + 64 * 32 * 2);
			if (state->fg2_page[3] == page) tilemap_mark_tile_dirty(state->foreground2, offset + 64 * 32 * 3);
		}
	}
}

/***************************************************************************/

static TILE_GET_INFO( get_text_tile_info )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	const UINT16 *source = state->textram;
	int tile_number = source[tile_index];
	int pri = tile_number >> 8;

	if (!state->shinobl_kludge)
	{
		SET_TILE_INFO(
				0,
				(tile_number & 0x1ff) + state->tile_bank0 * 0x1000,
				(tile_number >> 9) % 8,
				0);
	}
	else
	{
		SET_TILE_INFO(
				0,
				(tile_number & 0xff)  + state->tile_bank0 * 0x1000,
				(tile_number >> 8) % 8,
				0);
	}

	if (pri >= state->textlayer_lo_min && pri <= state->textlayer_lo_max)
		tileinfo->category = 1;
	if (pri >= state->textlayer_hi_min && pri <= state->textlayer_hi_max)
		tileinfo->category = 0;
}

WRITE16_HANDLER( sys16_textram_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;

	COMBINE_DATA(&state->textram[offset]);
	tilemap_mark_tile_dirty(state->text_layer, offset);
}

/***************************************************************************/

VIDEO_START( system16 )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, state->weights[0][0], 0, 0,
		6, resistances_normal, state->weights[0][1], 0, 0,
		6, resistances_normal, state->weights[0][2], 0, 0
		);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, state->weights[1][0], 0, 0,
		6, resistances_sh, state->weights[1][1], 0, 0,
		6, resistances_sh, state->weights[1][2], 0, 0
		);

	if (!state->bg1_trans)
		state->background = tilemap_create(machine, get_bg_tile_info, sys16_bg_map,
			8,8,
			64*2,32*2 );
	else
		state->background = tilemap_create(machine, get_bg_tile_info, sys16_bg_map,
			8,8,
			64*2,32*2 );

	state->foreground = tilemap_create(machine, get_fg_tile_info, sys16_bg_map,
		8,8,
		64*2,32*2 );

	state->text_layer = tilemap_create(machine, get_text_tile_info, sys16_text_map,
		8,8,
		40,28 );

	{
		if (state->bg1_trans) tilemap_set_transparent_pen(state->background, 0);
		tilemap_set_transparent_pen(state->foreground, 0);
		tilemap_set_transparent_pen(state->text_layer, 0);

		state->tile_bank0 = 0;
		state->tile_bank1 = 1;

		state->fg_scrollx = 0;
		state->fg_scrolly = 0;

		state->bg_scrollx = 0;
		state->bg_scrolly = 0;

		state->refreshenable = 1;

		/* common defaults */
		state->tilebank_switch = 0x1000;

		// Defaults for sys16 games
		state->textlayer_lo_min = 0;
		state->textlayer_lo_max = 0x7f;
		state->textlayer_hi_min = 0x80;
		state->textlayer_hi_max = 0xff;

		state->system18 = 0;
	}

	segaic16_palette_init(0x800);
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_16B, 0x400, state->sprite_xoffs);
	setup_system16_bootleg_spritebanking(machine);


}

VIDEO_START( system18old )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	VIDEO_START_CALL(system16);

	state->bg1_trans = 1;

	state->background2 = tilemap_create(machine, get_bg2_tile_info, sys16_bg_map,
		8,8,
		64*2,32*2 );

	state->foreground2 = tilemap_create(machine, get_fg2_tile_info, sys16_bg_map,
		8,8,
		64*2,32*2 );

	tilemap_set_transparent_pen(state->foreground2, 0);

	if (state->splittab_fg_x)
	{
		tilemap_set_scroll_rows(state->foreground , 64);
		tilemap_set_scroll_rows(state->foreground2 , 64);
	}

	if (state->splittab_bg_x)
	{
		tilemap_set_scroll_rows(state->background , 64);
		tilemap_set_scroll_rows(state->background2 , 64);
	}

	state->textlayer_lo_min = 0;
	state->textlayer_lo_max = 0x1f;
	state->textlayer_hi_min = 0x20;
	state->textlayer_hi_max = 0xff;

	state->system18 = 1;
}


/*****************************************************************************************
 System 16A bootleg video

 The System16A bootlegs have extra RAM for 2 tilemaps.  The game code copies data from
 the usual 'tilemap ram' area to this new RAM and sets scroll registers as appropriate
 using additional registers not present on the original System 16A hardware.

 For some unknown reason the 2p Passing Shot bootleg end up blanking this area at times,
 this could be an emulation flaw or a problem with the original bootlegs.  Inserting a
 coin at the incorrect time can also cause missing graphics on the initial entry screen.
 See note at top of driver

 Sprites:
  ToDo

*****************************************************************************************/


static TILE_GET_INFO( get_s16a_bootleg_tile_infotxt )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	int data, tile_number;

	data = state->textram[tile_index];
	tile_number = data & 0x1ff;

	SET_TILE_INFO(
			0,
			tile_number,
			((data >> 9) & 0x7),
			0);
}

static TILE_GET_INFO( get_s16a_bootleg_tile_info0 )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	int data, tile_number;

	data = state->bg0_tileram[tile_index];
	tile_number = data & 0x1fff;


	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

static TILE_GET_INFO( get_s16a_bootleg_tile_info1 )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;
	int data, tile_number;

	data = state->bg1_tileram[tile_index];
	tile_number = data & 0x1fff;

	SET_TILE_INFO(
			0,
			tile_number,
			(data >> 6) & 0x7f,
			0);
}

WRITE16_HANDLER( s16a_bootleg_bgscrolly_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	state->bg_scrolly = data;
}

WRITE16_HANDLER( s16a_bootleg_bgscrollx_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	state->bg_scrollx = data;
}

WRITE16_HANDLER( s16a_bootleg_fgscrolly_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	state->fg_scrolly = data;
}

WRITE16_HANDLER( s16a_bootleg_fgscrollx_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;
	state->fg_scrollx = data;
}

WRITE16_HANDLER( s16a_bootleg_tilemapselect_w )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)space->machine->driver_data;

	COMBINE_DATA(&state->tilemapselect);
	//printf("system16 bootleg tilemapselect %04x\n", state->tilemapselect);
}


VIDEO_START( s16a_bootleg )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, state->weights[0][0], 0, 0,
		6, resistances_normal, state->weights[0][1], 0, 0,
		6, resistances_normal, state->weights[0][2], 0, 0
		);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, state->weights[1][0], 0, 0,
		6, resistances_sh, state->weights[1][1], 0, 0,
		6, resistances_sh, state->weights[1][2], 0, 0
		);



	state->text_tilemap = tilemap_create(machine, get_s16a_bootleg_tile_infotxt, tilemap_scan_rows, 8,8, 64,32 );

	// the system16a bootlegs have simple tilemaps instead of the paged system
	state->bg_tilemaps[0] = tilemap_create(machine, get_s16a_bootleg_tile_info0, tilemap_scan_rows, 8,8, 64,32 );
	state->bg_tilemaps[1] = tilemap_create(machine, get_s16a_bootleg_tile_info1, tilemap_scan_rows, 8,8, 64,32 );

	tilemap_set_transparent_pen(state->text_tilemap, 0);
	tilemap_set_transparent_pen(state->bg_tilemaps[0], 0);
	tilemap_set_transparent_pen(state->bg_tilemaps[1], 0);

	segaic16_palette_init(0x800);

}

VIDEO_START( s16a_bootleg_wb3bl )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	VIDEO_START_CALL(s16a_bootleg);
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_16A_BOOTLEG_WB3BL, 0x400, state->sprite_xoffs);
	setup_system16_bootleg_spritebanking(machine);
}

VIDEO_START( s16a_bootleg_shinobi )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	VIDEO_START_CALL(s16a_bootleg);
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_16A_BOOTLEG_SHINOBLD, 0x400, state->sprite_xoffs);
	setup_system16_bootleg_spritebanking(machine);
}

VIDEO_START( s16a_bootleg_passsht )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)machine->driver_data;

	VIDEO_START_CALL(s16a_bootleg);
	segaic16_sprites_init(machine, 0, SEGAIC16_SPRITES_16A_BOOTLEG_PASSHTBL, 0x400, state->sprite_xoffs);
	setup_system16_bootleg_spritebanking(machine);
}

// Passing Shot (2 player), Shinobi (Datsu), Wonderboy 3
VIDEO_UPDATE( s16a_bootleg )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)screen->machine->driver_data;

	// passing shot
	int offset_txtx = 192;
	int offset_txty = 0;
	int offset_bg1x = 190;
	int offset_bg1y = 0;
	int offset_bg0x = 187;
	int offset_bg0y = 0;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	tilemap_mark_all_tiles_dirty(state->bg_tilemaps[0]);
	tilemap_mark_all_tiles_dirty(state->bg_tilemaps[1]);
	tilemap_mark_all_tiles_dirty(state->text_tilemap);

	tilemap_set_scrollx(state->text_tilemap, 0, offset_txtx);
	tilemap_set_scrolly(state->text_tilemap, 0, offset_txty);

	if ((state->tilemapselect & 0xff) == 0x12)
	{
		tilemap_set_scrollx(state->bg_tilemaps[1], 0, state->bg_scrollx + offset_bg1x);
		tilemap_set_scrolly(state->bg_tilemaps[1], 0, state->bg_scrolly + offset_bg1y + state->back_yscroll);
		tilemap_set_scrollx(state->bg_tilemaps[0], 0, state->fg_scrollx + offset_bg0x);
		tilemap_set_scrolly(state->bg_tilemaps[0], 0, state->fg_scrolly + offset_bg0y + state->fore_yscroll);

		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[0], TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[1], 0, 0);

		tilemap_set_scrolly(state->text_tilemap, 0, state->text_yscroll);

		tilemap_draw(bitmap, cliprect, state->text_tilemap, 0, 0);
	}
	else if ((state->tilemapselect & 0xff) == 0x21)
	{
		tilemap_set_scrollx(state->bg_tilemaps[0], 0, state->bg_scrollx + 187 );
		tilemap_set_scrolly(state->bg_tilemaps[0], 0, state->bg_scrolly + state->back_yscroll );
		tilemap_set_scrollx(state->bg_tilemaps[1], 0, state->fg_scrollx + 187 );
		tilemap_set_scrolly(state->bg_tilemaps[1], 0, state->fg_scrolly + 1 + state->fore_yscroll );

		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[1], TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[0], 0, 0);

		tilemap_set_scrolly(state->text_tilemap, 0, state->text_yscroll);

		tilemap_draw(bitmap, cliprect, state->text_tilemap, 0, 0);
	}

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);

	return 0;
}

/* The Passing Shot 4 Player bootleg has weird scroll registers (different offsets, ^0x7 xor) */
VIDEO_UPDATE( s16a_bootleg_passht4b )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)screen->machine->driver_data;

	// passing shot
	int offset_txtx = 192;
	int offset_txty = 0;
	int offset_bg1x = 3;
	int offset_bg1y = 32;
	int offset_bg0x = 5;
	int offset_bg0y = 32;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	tilemap_mark_all_tiles_dirty(state->bg_tilemaps[0]);
	tilemap_mark_all_tiles_dirty(state->bg_tilemaps[1]);
	tilemap_mark_all_tiles_dirty(state->text_tilemap);

	tilemap_set_scrollx(state->text_tilemap, 0, offset_txtx);
	tilemap_set_scrolly(state->text_tilemap, 0, offset_txty);

	if ((state->tilemapselect & 0xff) == 0x12)
	{
		tilemap_set_scrollx(state->bg_tilemaps[1], 0, (state->bg_scrollx ^ 0x7) + offset_bg1x);
		tilemap_set_scrolly(state->bg_tilemaps[1], 0, state->bg_scrolly + offset_bg1y);
		tilemap_set_scrollx(state->bg_tilemaps[0], 0, (state->fg_scrollx ^ 0x7) + offset_bg0x);
		tilemap_set_scrolly(state->bg_tilemaps[0], 0, state->fg_scrolly + offset_bg0y);

		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[0], TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemaps[1], 0, 0);
		tilemap_draw(bitmap, cliprect, state->text_tilemap, 0, 0);
	}

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);

	return 0;
}


/***************************************************************************/

VIDEO_UPDATE( system16 )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)screen->machine->driver_data;

	if (!state->refreshenable)
	{
		bitmap_fill(bitmap, cliprect, 0);
		return 0;
	}

	update_page(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_set_scrollx(state->background, 0, -320 - state->bg_scrollx);
	tilemap_set_scrolly(state->background, 0, -256 + state->bg_scrolly + state->back_yscroll);
	tilemap_set_scrollx(state->foreground, 0, -320 - state->fg_scrollx);
	tilemap_set_scrolly(state->foreground, 0, -256 + state->fg_scrolly + state->fore_yscroll);

	tilemap_set_scrollx(state->text_layer, 0, 0);
	tilemap_set_scrolly(state->text_layer, 0, 0 + state->text_yscroll);

	/* Background */
	tilemap_draw(bitmap, cliprect, state->background, TILEMAP_DRAW_OPAQUE, 0x00);

	/* Foreground */
	tilemap_draw(bitmap, cliprect, state->foreground, 0, 0x03);
	tilemap_draw(bitmap, cliprect, state->foreground, 1, 0x07);


	/* Text Layer */
	if (state->textlayer_lo_max != 0)
	{
		tilemap_draw(bitmap, cliprect, state->text_layer, 1, 7);// needed for Body Slam
	}

	tilemap_draw(bitmap, cliprect, state->text_layer, 0, 0xf);

	//draw_sprites(screen->machine, bitmap, cliprect,0);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}


VIDEO_UPDATE( system18old )
{
	segas1x_bootleg_state *state = (segas1x_bootleg_state *)screen->machine->driver_data;

	if (!state->refreshenable)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	update_page(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	bitmap_fill(bitmap,cliprect,0);

	tilemap_draw(bitmap, cliprect, state->background, TILEMAP_DRAW_OPAQUE, 0);
	tilemap_draw(bitmap, cliprect, state->background, TILEMAP_DRAW_OPAQUE | 1, 0);	//??
	tilemap_draw(bitmap, cliprect, state->background, TILEMAP_DRAW_OPAQUE | 2, 0);	//??
	tilemap_draw(bitmap, cliprect, state->background, 1, 0x1);
	tilemap_draw(bitmap, cliprect, state->background, 2, 0x3);

	tilemap_draw(bitmap, cliprect, state->foreground, 0, 0x3);
	tilemap_draw(bitmap, cliprect, state->foreground, 1, 0x7);

	tilemap_draw(bitmap, cliprect, state->text_layer, 1, 0x7);
	tilemap_draw(bitmap, cliprect, state->text_layer, 0, 0xf);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);

	return 0;
}
