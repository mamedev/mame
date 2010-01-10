/*
Super Cross II (JPN Ver.)
(c)1986 GM Shoji
*/

#include "emu.h"

static tilemap_t *sprcros2_bgtilemap, *sprcros2_fgtilemap;
UINT8 *sprcros2_fgvideoram, *sprcros2_spriteram, *sprcros2_bgvideoram;
size_t sprcros2_spriteram_size;
extern UINT8 sprcros2_m_port7;

PALETTE_INIT( sprcros2 )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0xb8 * bit1;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* bg */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | ((color_prom[i + 0x100] & 0x0f) << 4);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites & fg */
	for (i = 0x100; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i + 0x100];
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( sprcros2_fgvideoram_w )
{
	sprcros2_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(sprcros2_fgtilemap,offset&0x3ff);
}

WRITE8_HANDLER( sprcros2_bgvideoram_w )
{
	sprcros2_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(sprcros2_bgtilemap,offset&0x3ff);
}

WRITE8_HANDLER( sprcros2_bgscrollx_w )
{
	if(sprcros2_m_port7&0x02)
		tilemap_set_scrollx(sprcros2_bgtilemap,0,0x100-data);
	else
		tilemap_set_scrollx(sprcros2_bgtilemap,0,data);
}

WRITE8_HANDLER( sprcros2_bgscrolly_w )
{
	tilemap_set_scrolly(sprcros2_bgtilemap,0,data);
}

static TILE_GET_INFO( get_sprcros2_bgtile_info )
{
	UINT32 tile_number = sprcros2_bgvideoram[tile_index];
	UINT8 attr = sprcros2_bgvideoram[tile_index+0x400];

	//attr
	//76543210
	//xxxx---- colour
	//----x--- flipx
	//-----xxx tile bank

	tile_number += (attr&0x07)<<8;

	SET_TILE_INFO(
			0,
			tile_number,
			(attr&0xf0)>>4,
			(attr&0x08)?TILE_FLIPX:0);
}

static TILE_GET_INFO( get_sprcros2_fgtile_info )
{
	UINT32 tile_number = sprcros2_fgvideoram[tile_index];
	UINT8 attr = sprcros2_fgvideoram[tile_index+0x400];
	int color = (attr&0xfc)>>2;

	tileinfo->group = color;

	//attr
	//76543210
	//xxxxxx-- colour
	//------xx tile bank

	tile_number += (attr&0x03)<<8;

	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
}

VIDEO_START( sprcros2 )
{
	sprcros2_bgtilemap = tilemap_create( machine, get_sprcros2_bgtile_info,tilemap_scan_rows,8,8,32,32 );
	sprcros2_fgtilemap = tilemap_create( machine, get_sprcros2_fgtile_info,tilemap_scan_rows,8,8,32,32 );

	colortable_configure_tilemap_groups(machine->colortable, sprcros2_fgtilemap, machine->gfx[2], 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs,sx,sy,color,flipx,flipy;

	for (offs = sprcros2_spriteram_size-4; offs >= 0; offs -= 4)
	{
		if(sprcros2_spriteram[offs])
		{

			//offs
			//76543210
			//x------- unused
			//-xxxxxxx sprite number
			//offs+1
			//76543210
			//xx------ unused
			//--xxx--- colour (6/7 unused and blank in prom)
			//-----x-- unused
			//------x- flipx
			//-------x unused
			//offs+2   y pos
			//offs+3   x pos

			sx = ((sprcros2_spriteram[offs+3]+0x10)%0x100)-0x10;
			sy = 225-(((sprcros2_spriteram[offs+2]+0x10)%0x100)-0x10);
			color = (sprcros2_spriteram[offs+1]&0x38)>>3;
			flipx = sprcros2_spriteram[offs+1]&0x02;
			flipy = 0;

			if (sprcros2_m_port7&0x02)
			{
				sx = 224-sx;
				sy = 224-sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap,cliprect,machine->gfx[1],
				sprcros2_spriteram[offs],
				color,
				flipx,flipy,
				sx,sy,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
		}
	}
}

VIDEO_UPDATE( sprcros2 )
{
	tilemap_draw( bitmap,cliprect,sprcros2_bgtilemap,0,0 );
	draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw( bitmap,cliprect,sprcros2_fgtilemap,0,0 );
	return 0;
}
