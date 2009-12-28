/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


UINT8 *mrdo_bgvideoram,*mrdo_fgvideoram;
static tilemap_t *bg_tilemap,*fg_tilemap;
static int flipscreen;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mr. Do! has two 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROMs are connected to the RGB output this way:

  U2:
  bit 7 -- unused
        -- unused
        -- 100 ohm resistor  -diode- BLUE
        --  75 ohm resistor  -diode- BLUE
        -- 100 ohm resistor  -diode- GREEN
        --  75 ohm resistor  -diode- GREEN
        -- 100 ohm resistor  -diode- RED
  bit 0 --  75 ohm resistor  -diode- RED

  T2:
  bit 7 -- unused
        -- unused
        -- 150 ohm resistor  -diode- BLUE
        -- 120 ohm resistor  -diode- BLUE
        -- 150 ohm resistor  -diode- GREEN
        -- 120 ohm resistor  -diode- GREEN
        -- 150 ohm resistor  -diode- RED
  bit 0 -- 120 ohm resistor  -diode- RED

  200 ohm pulldown on all three components

***************************************************************************/
PALETTE_INIT( mrdo )
{
	int i;

	const int R1 = 150;
	const int R2 = 120;
	const int R3 = 100;
	const int R4 = 75;
	const int pull = 220;
	float pot[16];
	int weight[16];
	const float potadjust = 0.7f;	/* diode voltage drop */

	for (i = 0x0f; i >= 0; i--)
	{
		float par = 0;

		if (i & 1) par += 1.0f/(float)R1;
		if (i & 2) par += 1.0f/(float)R2;
		if (i & 4) par += 1.0f/(float)R3;
		if (i & 8) par += 1.0f/(float)R4;
		if (par)
		{
			par = 1/par;
			pot[i] = pull/(pull+par) - potadjust;
		}
		else pot[i] = 0;

		weight[i] = 0xff * pot[i] / pot[0x0f];
	}

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	for (i = 0; i < 0x100; i++)
	{
		int a1,a2;
		int bits0, bits2;
		int r, g, b;

		a1 = ((i >> 3) & 0x1c) + (i & 0x03) + 0x20;
		a2 = ((i >> 0) & 0x1c) + (i & 0x03);

		/* red component */
		bits0 = (color_prom[a1] >> 0) & 0x03;
		bits2 = (color_prom[a2] >> 0) & 0x03;
		r = weight[bits0 + (bits2 << 2)];

		/* green component */
		bits0 = (color_prom[a1] >> 2) & 0x03;
		bits2 = (color_prom[a2] >> 2) & 0x03;
		g = weight[bits0 + (bits2 << 2)];

		/* blue component */
		bits0 = (color_prom[a1] >> 4) & 0x03;
		bits2 = (color_prom[a2] >> 4) & 0x03;
		b = weight[bits0 + (bits2 << 2)];

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	/* characters */
	for (i = 0; i < 0x100; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* sprites */
	for (i = 0x100; i < 0x140; i++)
	{
		UINT8 ctabentry = color_prom[(i - 0x100) & 0x1f];

		if ((i - 0x100) & 0x20)
			ctabentry >>= 4;		/* high 4 bits are for sprite color n + 8 */
		else
			ctabentry &= 0x0f;	/* low 4 bits are for sprite color n */

		colortable_entry_set_value(machine->colortable, i, ctabentry + ((ctabentry & 0x0c) << 3));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 attr = mrdo_bgvideoram[tile_index];
	SET_TILE_INFO(
			1,
			mrdo_bgvideoram[tile_index+0x400] + ((attr & 0x80) << 1),
			attr & 0x3f,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 attr = mrdo_fgvideoram[tile_index];
	SET_TILE_INFO(
			0,
			mrdo_fgvideoram[tile_index+0x400] + ((attr & 0x80) << 1),
			attr & 0x3f,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mrdo )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,32,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(bg_tilemap, 0, 56);
	tilemap_set_scrolldx(fg_tilemap, 0, 56);
	tilemap_set_scrolldy(bg_tilemap, 0, 6);
	tilemap_set_scrolldy(fg_tilemap, 0, 6);

    state_save_register_global(machine, flipscreen);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mrdo_bgvideoram_w )
{
	mrdo_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( mrdo_fgvideoram_w )
{
	mrdo_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}


WRITE8_HANDLER( mrdo_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap,0,data);
}

WRITE8_HANDLER( mrdo_scrolly_w )
{
	/* This is NOT affected by flipscreen (so stop it happening) */

	if (flipscreen) tilemap_set_scrolly(bg_tilemap,0,((256-data) & 0xff));
	else tilemap_set_scrolly(bg_tilemap,0,data);
}


WRITE8_HANDLER( mrdo_flipscreen_w )
{
	/* bits 1-3 control the playfield priority, but they are not used by */
	/* Mr. Do! so we don't emulate them */

	flipscreen = data & 0x01;
	tilemap_set_flip_all(space->machine,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;


	for (offs = machine->generic.spriteram_size - 4;offs >= 0;offs -= 4)
	{
		if (spriteram[offs + 1] != 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					spriteram[offs],spriteram[offs + 2] & 0x0f,
					spriteram[offs + 2] & 0x10,spriteram[offs + 2] & 0x20,
					spriteram[offs + 3],256 - spriteram[offs + 1],0);
		}
	}
}

VIDEO_UPDATE( mrdo )
{
	bitmap_fill(bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
