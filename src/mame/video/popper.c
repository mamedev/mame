/*
Popper
Omori Electric CAD (OEC) 1983
*/

#include "driver.h"

static tilemap *popper_p123_tilemap, *popper_p0_tilemap, *popper_ol_p123_tilemap, *popper_ol_p0_tilemap;
UINT8 *popper_videoram, *popper_attribram, *popper_ol_videoram, *popper_ol_attribram, *popper_spriteram;
size_t popper_spriteram_size;
static INT32 popper_flipscreen, popper_e002, popper_gfx_bank;
static rectangle tilemap_clip;

PALETTE_INIT( popper )
{
	int i;

	for (i = 0;i < machine->drv->total_colors; i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x97 * bit1 + 0x68 * bit2;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( popper_ol_videoram_w )
{
	popper_ol_videoram[offset] = data;
	tilemap_mark_tile_dirty(popper_ol_p123_tilemap,offset);
	tilemap_mark_tile_dirty(popper_ol_p0_tilemap,offset);
}

WRITE8_HANDLER( popper_videoram_w )
{
	popper_videoram[offset] = data;
	tilemap_mark_tile_dirty(popper_p123_tilemap,offset);
	tilemap_mark_tile_dirty(popper_p0_tilemap,offset);
}

WRITE8_HANDLER( popper_ol_attribram_w )
{
	popper_ol_attribram[offset] = data;
	tilemap_mark_tile_dirty(popper_ol_p123_tilemap,offset);
	tilemap_mark_tile_dirty(popper_ol_p0_tilemap,offset);
}

WRITE8_HANDLER( popper_attribram_w )
{
	popper_attribram[offset] = data;
	tilemap_mark_tile_dirty(popper_p123_tilemap,offset);
	tilemap_mark_tile_dirty(popper_p0_tilemap,offset);
}

WRITE8_HANDLER( popper_flipscreen_w )
{
	popper_flipscreen = data;
	tilemap_set_flip( ALL_TILEMAPS,popper_flipscreen?(TILEMAP_FLIPX|TILEMAP_FLIPY):0 );

	tilemap_clip = Machine->screen[0].visarea;

	if (popper_flipscreen)
		tilemap_clip.min_x=tilemap_clip.max_x-15;
	else
		tilemap_clip.max_x=15;
}

WRITE8_HANDLER( popper_e002_w )
{
	popper_e002 = data;
}

WRITE8_HANDLER( popper_gfx_bank_w )
{
	if (popper_gfx_bank != data)
	{
		popper_gfx_bank = data;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_popper_p123_tile_info )
{
	UINT32 tile_number = popper_videoram[tile_index];
	UINT8 attr  = popper_attribram[tile_index];
	tile_number += popper_gfx_bank << 8;

	SET_TILE_INFO(
			0,
			tile_number,
			(attr&0xf),
			0);
	tileinfo->group = (attr & 0x80)>>7;
}

static TILE_GET_INFO( get_popper_p0_tile_info )
{
	UINT32 tile_number = popper_videoram[tile_index];
	UINT8 attr = popper_attribram[tile_index];
	tile_number += popper_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo->group = (attr&0x70) ? ((attr & 0x80)>>7) : 0;

	SET_TILE_INFO(
			0,
			tile_number,
			((attr&0x70)>>4)+8,
			0);
}

static TILE_GET_INFO( get_popper_ol_p123_tile_info )
{
	UINT32 tile_number = popper_ol_videoram[tile_index];
	UINT8 attr  = popper_ol_attribram[tile_index];
	tile_number += popper_gfx_bank << 8;

	SET_TILE_INFO(
			0,
			tile_number,
			(attr&0xf),
			0);
	tileinfo->group = (attr & 0x80)>>7;
}

static TILE_GET_INFO( get_popper_ol_p0_tile_info )
{
	UINT32 tile_number = popper_ol_videoram[tile_index];
	UINT8 attr = popper_ol_attribram[tile_index];
	tile_number += popper_gfx_bank << 8;

	//pen 0 only in front if colour set as well
	tileinfo->group = (attr&0x70) ? ((attr & 0x80)>>7) : 0;

	SET_TILE_INFO(
			0,
			tile_number,
			((attr&0x70)>>4)+8,
			0);
}

VIDEO_START( popper )
{
	popper_p123_tilemap    = tilemap_create( get_popper_p123_tile_info,   tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,33,32 );
	popper_p0_tilemap      = tilemap_create( get_popper_p0_tile_info,     tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,33,32 );
	popper_ol_p123_tilemap = tilemap_create( get_popper_ol_p123_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,2 ,32 );
	popper_ol_p0_tilemap   = tilemap_create( get_popper_ol_p0_tile_info,  tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,2 ,32 );

	tilemap_set_transmask(popper_p123_tilemap,   0,0x0f,0x01);
	tilemap_set_transmask(popper_p123_tilemap,   1,0x01,0x0f);
	tilemap_set_transmask(popper_p0_tilemap,     0,0x0f,0x0e);
	tilemap_set_transmask(popper_p0_tilemap,     1,0x0e,0x0f);
	tilemap_set_transmask(popper_ol_p123_tilemap,0,0x0f,0x01);
	tilemap_set_transmask(popper_ol_p123_tilemap,1,0x01,0x0f);
	tilemap_set_transmask(popper_ol_p0_tilemap,  0,0x0f,0x0e);
	tilemap_set_transmask(popper_ol_p0_tilemap,  1,0x0e,0x0f);

	tilemap_clip = machine->screen[0].visarea;

	state_save_register_global(popper_flipscreen);
//  state_save_register_global(popper_e002);
	state_save_register_global(popper_gfx_bank);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs,sx,sy,flipx,flipy;

	for (offs = 0; offs < popper_spriteram_size-4; offs += 4)
	{
		//if y position is in the current strip
		if(popper_spriteram[offs+1] && (((popper_spriteram[offs]+(popper_flipscreen?2:0))&0xf0) == (0x0f-offs/0x80)<<4))
		{
			//offs     y pos
			//offs+1   sprite number
			//offs+2
			//76543210
			//x------- flipy
			//-x------ flipx
			//--xx---- unused
			//----xxxx colour
			//offs+3   x pos

			sx = popper_spriteram[offs+3];
			sy = 240-popper_spriteram[offs];
			flipx = (popper_spriteram[offs+2]&0x40)>>6;
			flipy = (popper_spriteram[offs+2]&0x80)>>7;

			if (popper_flipscreen)
			{
				sx = 248 - sx;
				sy = 242 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap,machine->gfx[1],
					popper_spriteram[offs+1],
					(popper_spriteram[offs+2]&0x0f),
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( popper )
{
	rectangle finalclip = tilemap_clip;
	sect_rect(&finalclip, cliprect);

	//attribram
	//76543210
	//x------- draw over sprites
	//-xxx---- colour for pen 0 (from second prom?)
	//----xxxx colour for pens 1,2,3

	tilemap_draw( bitmap,cliprect,popper_p123_tilemap,     TILEMAP_DRAW_LAYER1,0 );
	tilemap_draw( bitmap,cliprect,popper_p0_tilemap,       TILEMAP_DRAW_LAYER1,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p123_tilemap,TILEMAP_DRAW_LAYER1,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p0_tilemap,  TILEMAP_DRAW_LAYER1,0 );

	draw_sprites(machine, bitmap,cliprect);

	tilemap_draw( bitmap,cliprect,popper_p123_tilemap,     TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,cliprect,popper_p0_tilemap,       TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p123_tilemap,TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p0_tilemap,  TILEMAP_DRAW_LAYER0,0 );
	return 0;
}
