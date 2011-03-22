/***************************************************************************

   Dark Seal Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

Data East custom chip 55:  Generates two playfields, playfield 1 is underneath
playfield 2.  Dark Seal uses two of these chips.  1 playfield is _always_ off
in this game.

    16 bytes of control registers per chip.

    Word 0:
        Mask 0x0080: Flip screen
        Mask 0x007f: ?
    Word 2:
        Mask 0xffff: Playfield 2 X scroll (top playfield)
    Word 4:
        Mask 0xffff: Playfield 2 Y scroll (top playfield)
    Word 6:
        Mask 0xffff: Playfield 1 X scroll (bottom playfield)
    Word 8:
        Mask 0xffff: Playfield 1 Y scroll (bottom playfield)
    Word 0xa:
        Mask 0xc000: Playfield 1 shape??
        Mask 0x3000: Playfield 1 rowscroll style (maybe mask 0x3800??)
        Mask 0x0300: Playfield 1 colscroll style (maybe mask 0x0700??)?

        Mask 0x00c0: Playfield 2 shape??
        Mask 0x0030: Playfield 2 rowscroll style (maybe mask 0x0038??)
        Mask 0x0003: Playfield 2 colscroll style (maybe mask 0x0007??)?
    Word 0xc:
        Mask 0x8000: Playfield 1 is 8*8 tiles else 16*16
        Mask 0x4000: Playfield 1 rowscroll enabled
        Mask 0x2000: Playfield 1 colscroll enabled
        Mask 0x1f00: ?

        Mask 0x0080: Playfield 2 is 8*8 tiles else 16*16
        Mask 0x0040: Playfield 2 rowscroll enabled
        Mask 0x0020: Playfield 2 colscroll enabled
        Mask 0x001f: ?
    Word 0xe:
        ??

Locations 0 & 0xe are mostly unknown:

                             0      14
Caveman Ninja (bottom):     0053    1100 (changes to 1111 later)
Caveman Ninja (top):        0010    0081
Two Crude (bottom):         0053    0000
Two Crude (top):            0010    0041
Dark Seal (bottom):         0010    0000
Dark Seal (top):            0053    4101
Tumblepop:                  0010    0000
Super Burger Time:          0010    0000

Location 0xe looks like it could be a mirror of another byte..

**************************************************************************

Sprites - Data East custom chip 52

    8 bytes per sprite, unknowns bits seem unused.

    Word 0:
        Mask 0x8000 - ?
        Mask 0x4000 - Y flip
        Mask 0x2000 - X flip
        Mask 0x1000 - Sprite flash
        Mask 0x0800 - ?
        Mask 0x0600 - Sprite height (1x, 2x, 4x, 8x)
        Mask 0x01ff - Y coordinate

    Word 2:
        Mask 0xffff - Sprite number

    Word 4:
        Mask 0x8000 - ?
        Mask 0x4000 - Sprite is drawn beneath top 8 pens of playfield 4
        Mask 0x3e00 - Colour (32 palettes, most games only use 16)
        Mask 0x01ff - X coordinate

    Word 6:
        Always unused.

***************************************************************************/

#include "emu.h"
#include "includes/darkseal.h"
#include "video/decospr.h"


/***************************************************************************/

/* Function for all 16x16 1024x1024 layers */
static TILEMAP_MAPPER( darkseal_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
}

INLINE void get_bg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int gfx_bank,UINT16 *gfx_base)
{
	int tile,color;

	tile=gfx_base[tile_index];
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(
			gfx_bank,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_bg_tile_info2 )
{
	darkseal_state *state = machine->driver_data<darkseal_state>();
	get_bg_tile_info(machine,tileinfo,tile_index,1,state->pf2_data);
}

static TILE_GET_INFO( get_bg_tile_info3 )
{
	darkseal_state *state = machine->driver_data<darkseal_state>();
	get_bg_tile_info(machine,tileinfo,tile_index,2,state->pf3_data);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	darkseal_state *state = machine->driver_data<darkseal_state>();
	int tile=state->pf1_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

/******************************************************************************/

static void update_24bitcol(running_machine *machine, int offset)
{
	int r,g,b;

	r = (machine->generic.paletteram.u16[offset] >> 0) & 0xff;
	g = (machine->generic.paletteram.u16[offset] >> 8) & 0xff;
	b = (machine->generic.paletteram2.u16[offset] >> 0) & 0xff;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( darkseal_palette_24bit_rg_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	update_24bitcol(space->machine, offset);
}

WRITE16_HANDLER( darkseal_palette_24bit_b_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram2.u16[offset]);
	update_24bitcol(space->machine, offset);
}

/******************************************************************************/

/******************************************************************************/

WRITE16_HANDLER( darkseal_pf1_data_w )
{
	darkseal_state *state = space->machine->driver_data<darkseal_state>();
	COMBINE_DATA(&state->pf1_data[offset]);
	tilemap_mark_tile_dirty(state->pf1_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf2_data_w )
{
	darkseal_state *state = space->machine->driver_data<darkseal_state>();
	COMBINE_DATA(&state->pf2_data[offset]);
	tilemap_mark_tile_dirty(state->pf2_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf3_data_w )
{
	darkseal_state *state = space->machine->driver_data<darkseal_state>();
	COMBINE_DATA(&state->pf3_data[offset]);
	tilemap_mark_tile_dirty(state->pf3_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf3b_data_w ) /* Mirror */
{
	darkseal_pf3_data_w(space,offset+0x800,data,mem_mask);
}

WRITE16_HANDLER( darkseal_control_0_w )
{
	darkseal_state *state = space->machine->driver_data<darkseal_state>();
	COMBINE_DATA(&state->control_0[offset]);
}

WRITE16_HANDLER( darkseal_control_1_w )
{
	darkseal_state *state = space->machine->driver_data<darkseal_state>();
	COMBINE_DATA(&state->control_1[offset]);
}

/******************************************************************************/

VIDEO_START( darkseal )
{
	darkseal_state *state = machine->driver_data<darkseal_state>();
	state->pf1_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8,64,64);
	state->pf2_tilemap = tilemap_create(machine, get_bg_tile_info2,darkseal_scan,    16,16,64,64);
	state->pf3_tilemap = tilemap_create(machine, get_bg_tile_info3,darkseal_scan,         16,16,64,64);

	tilemap_set_transparent_pen(state->pf1_tilemap,0);
	tilemap_set_transparent_pen(state->pf2_tilemap,0);
}

/******************************************************************************/

SCREEN_UPDATE( darkseal )
{
	darkseal_state *state = screen->machine->driver_data<darkseal_state>();
	state->flipscreen=!(state->control_0[0]&0x80);
	tilemap_set_flip_all(screen->machine,state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Update scroll registers */
	tilemap_set_scrollx( state->pf1_tilemap,0, state->control_1[3] );
	tilemap_set_scrolly( state->pf1_tilemap,0, state->control_1[4] );
	tilemap_set_scrollx( state->pf2_tilemap,0, state->control_1[1]);
	tilemap_set_scrolly( state->pf2_tilemap,0, state->control_1[2] );

	if (state->control_0[6]&0x4000) { /* Rowscroll enable */
		int offs,scrollx=state->control_0[3];

		tilemap_set_scroll_rows(state->pf3_tilemap,512);
		for (offs = 0;offs < 512;offs++)
			tilemap_set_scrollx( state->pf3_tilemap,offs, scrollx + state->pf34_row[offs+0x40] );
	}
	else {
		tilemap_set_scroll_rows(state->pf3_tilemap,1);
		tilemap_set_scrollx( state->pf3_tilemap,0, state->control_0[3] );
	}
	tilemap_set_scrolly( state->pf3_tilemap,0, state->control_0[4] );

	tilemap_draw(bitmap,cliprect,state->pf3_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->pf2_tilemap,0,0);

	screen->machine->device<decospr_device>("spritegen")->draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u16, 0x400);

	tilemap_draw(bitmap,cliprect,state->pf1_tilemap,0,0);
	return 0;
}

/******************************************************************************/
