/*
Popper
Omori Electric CAD (OEC) 1983
*/

#include "driver.h"
#include "video/resnet.h"

static tilemap *popper_p123_tilemap, *popper_p0_tilemap, *popper_ol_p123_tilemap, *popper_ol_p0_tilemap;
UINT8 *popper_videoram, *popper_attribram, *popper_ol_videoram, *popper_ol_attribram, *popper_spriteram;
size_t popper_spriteram_size;
static INT32 popper_flipscreen, popper_e002, popper_gfx_bank;
static rectangle tilemap_clip;


/***************************************************************************
 *
 * Color guns - from schematics
 *
 ***************************************************************************/

static const res_net_decode_info popper_decode_info =
{
	1,		// there may be two proms needed to construct color
	0,		// start at 0
	63,	// end at 255
	//  R,   G,   B,
	{   0,   0,   0, },		// offsets
	{   0,   3,   6, },		// shifts
	{0x07,0x07,0x03, }	    // masks
};

static const res_net_info popper_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

/***************************************************************************
 *
 * PALETTE_INIT
 *
 ***************************************************************************/

PALETTE_INIT( popper )
{
	rgb_t	*rgb;

	rgb = compute_res_net_all(color_prom, &popper_decode_info, &popper_net_info);
	palette_set_colors(machine, 0, rgb, 64);
	palette_normalize_range(machine->palette, 0, 63, 0, 255);
	free(rgb);
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
	tilemap_set_flip_all( space->machine,popper_flipscreen?(TILEMAP_FLIPX|TILEMAP_FLIPY):0 );

	tilemap_clip = *video_screen_get_visible_area(space->machine->primary_screen);

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
		tilemap_mark_all_tiles_dirty_all(space->machine);
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
	popper_p123_tilemap    = tilemap_create( machine, get_popper_p123_tile_info,   tilemap_scan_cols,8,8,33,32 );
	popper_p0_tilemap      = tilemap_create( machine, get_popper_p0_tile_info,     tilemap_scan_cols,8,8,33,32 );
	popper_ol_p123_tilemap = tilemap_create( machine, get_popper_ol_p123_tile_info,tilemap_scan_cols,8,8,2 ,32 );
	popper_ol_p0_tilemap   = tilemap_create( machine, get_popper_ol_p0_tile_info,  tilemap_scan_cols,8,8,2 ,32 );

	tilemap_set_transmask(popper_p123_tilemap,   0,0x0f,0x01);
	tilemap_set_transmask(popper_p123_tilemap,   1,0x01,0x0f);
	tilemap_set_transmask(popper_p0_tilemap,     0,0x0f,0x0e);
	tilemap_set_transmask(popper_p0_tilemap,     1,0x0e,0x0f);
	tilemap_set_transmask(popper_ol_p123_tilemap,0,0x0f,0x01);
	tilemap_set_transmask(popper_ol_p123_tilemap,1,0x01,0x0f);
	tilemap_set_transmask(popper_ol_p0_tilemap,  0,0x0f,0x0e);
	tilemap_set_transmask(popper_ol_p0_tilemap,  1,0x0e,0x0f);

	tilemap_clip = *video_screen_get_visible_area(machine->primary_screen);

	state_save_register_global(machine, popper_flipscreen);
//  state_save_register_global(machine, popper_e002);
	state_save_register_global(machine, popper_gfx_bank);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
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

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					popper_spriteram[offs+1],
					(popper_spriteram[offs+2]&0x0f),
					flipx,flipy,
					sx,sy,0);
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

	draw_sprites(screen->machine, bitmap,cliprect);

	tilemap_draw( bitmap,cliprect,popper_p123_tilemap,     TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,cliprect,popper_p0_tilemap,       TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p123_tilemap,TILEMAP_DRAW_LAYER0,0 );
	tilemap_draw( bitmap,&finalclip,popper_ol_p0_tilemap,  TILEMAP_DRAW_LAYER0,0 );
	return 0;
}
