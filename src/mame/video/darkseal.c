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

#include "driver.h"

UINT16 *darkseal_pf12_row,*darkseal_pf34_row;
UINT16 *darkseal_pf1_data,*darkseal_pf2_data,*darkseal_pf3_data;

static UINT16 darkseal_control_0[8];
static UINT16 darkseal_control_1[8];

static tilemap *pf1_tilemap,*pf2_tilemap,*pf3_tilemap;
static int flipscreen;

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

static TILE_GET_INFO( get_bg_tile_info2 ) { get_bg_tile_info(machine,tileinfo,tile_index,1,darkseal_pf2_data); }
static TILE_GET_INFO( get_bg_tile_info3 ) { get_bg_tile_info(machine,tileinfo,tile_index,2,darkseal_pf3_data); }

static TILE_GET_INFO( get_fg_tile_info )
{
	int tile=darkseal_pf1_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

/******************************************************************************/

static void update_24bitcol(int offset)
{
	int r,g,b;

	r = (paletteram16[offset] >> 0) & 0xff;
	g = (paletteram16[offset] >> 8) & 0xff;
	b = (paletteram16_2[offset] >> 0) & 0xff;

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( darkseal_palette_24bit_rg_w )
{
	COMBINE_DATA(&paletteram16[offset]);
	update_24bitcol(offset);
}

WRITE16_HANDLER( darkseal_palette_24bit_b_w )
{
	COMBINE_DATA(&paletteram16_2[offset]);
	update_24bitcol(offset);
}

/******************************************************************************/

static void draw_sprites(running_machine* machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = buffered_spriteram16[offs+1] & 0x1fff;
		if (!sprite) continue;

		y = buffered_spriteram16[offs];
		x = buffered_spriteram16[offs+2];

		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		colour = (x >> 9) &0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x>256) continue; /* Speedup */

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

/******************************************************************************/

WRITE16_HANDLER( darkseal_pf1_data_w )
{
	COMBINE_DATA(&darkseal_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf2_data_w )
{
	COMBINE_DATA(&darkseal_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf3_data_w )
{
	COMBINE_DATA(&darkseal_pf3_data[offset]);
	tilemap_mark_tile_dirty(pf3_tilemap,offset);
}

WRITE16_HANDLER( darkseal_pf3b_data_w ) /* Mirror */
{
	darkseal_pf3_data_w(offset+0x800,data,mem_mask);
}

WRITE16_HANDLER( darkseal_control_0_w )
{
	COMBINE_DATA(&darkseal_control_0[offset]);
}

WRITE16_HANDLER( darkseal_control_1_w )
{
	COMBINE_DATA(&darkseal_control_1[offset]);
}

/******************************************************************************/

VIDEO_START( darkseal )
{
	pf1_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	pf2_tilemap = tilemap_create(get_bg_tile_info2,darkseal_scan,    TILEMAP_TYPE_PEN,16,16,64,64);
	pf3_tilemap = tilemap_create(get_bg_tile_info3,darkseal_scan,    TILEMAP_TYPE_PEN,     16,16,64,64);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
}

/******************************************************************************/

VIDEO_UPDATE( darkseal )
{
	flipscreen=!(darkseal_control_0[0]&0x80);
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Update scroll registers */
	tilemap_set_scrollx( pf1_tilemap,0, darkseal_control_1[3] );
	tilemap_set_scrolly( pf1_tilemap,0, darkseal_control_1[4] );
	tilemap_set_scrollx( pf2_tilemap,0, darkseal_control_1[1]);
	tilemap_set_scrolly( pf2_tilemap,0, darkseal_control_1[2] );

	if (darkseal_control_0[6]&0x4000) { /* Rowscroll enable */
		int offs,scrollx=darkseal_control_0[3];

		tilemap_set_scroll_rows(pf3_tilemap,512);
		for (offs = 0;offs < 512;offs++)
			tilemap_set_scrollx( pf3_tilemap,offs, scrollx + darkseal_pf34_row[offs+0x40] );
	}
	else {
		tilemap_set_scroll_rows(pf3_tilemap,1);
		tilemap_set_scrollx( pf3_tilemap,0, darkseal_control_0[3] );
	}
	tilemap_set_scrolly( pf3_tilemap,0, darkseal_control_0[4] );

	tilemap_draw(bitmap,cliprect,pf3_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	return 0;
}

/******************************************************************************/
