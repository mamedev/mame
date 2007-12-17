/***************************************************************************

  Mad Motor video emulation - Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "driver.h"

UINT16 *madmotor_pf1_rowscroll;
UINT16 *madmotor_pf1_data,*madmotor_pf2_data,*madmotor_pf3_data;

static UINT16 madmotor_pf1_control[16];
static UINT16 madmotor_pf2_control[16];
static UINT16 madmotor_pf3_control[16];

static int flipscreen;
static tilemap *madmotor_pf1_tilemap,*madmotor_pf2_tilemap,*madmotor_pf3_tilemap,*madmotor_pf3a_tilemap;




/* 512 by 512 playfield, 8 by 8 tiles */
static TILEMAP_MAPPER( pf1_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	int tile,color;

	tile=madmotor_pf1_data[tile_index];
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

/* 512 by 512 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf2_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x10) << 4) + ((col & 0x10) << 5);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	int tile,color;

	tile=madmotor_pf2_data[tile_index];
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

/* 512 by 1024 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf3_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x30) << 4) + ((col & 0x10) << 6);
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	int tile,color;

	tile=madmotor_pf3_data[tile_index];
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

/* 2048 by 256 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf3a_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4);
}

static TILE_GET_INFO( get_pf3a_tile_info )
{
	int tile,color;

	tile=madmotor_pf3_data[tile_index];
	color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

/******************************************************************************/

VIDEO_START( madmotor )
{
	madmotor_pf1_tilemap = tilemap_create(get_pf1_tile_info, pf1_scan, TILEMAP_TYPE_PEN, 8, 8, 64,64);
	madmotor_pf2_tilemap = tilemap_create(get_pf2_tile_info, pf2_scan, TILEMAP_TYPE_PEN,16,16, 32,32);
	madmotor_pf3_tilemap = tilemap_create(get_pf3_tile_info, pf3_scan, TILEMAP_TYPE_PEN,     16,16, 32,64);
	madmotor_pf3a_tilemap= tilemap_create(get_pf3a_tile_info,pf3a_scan,TILEMAP_TYPE_PEN,     16,16,128,16);

	tilemap_set_transparent_pen(madmotor_pf1_tilemap,0);
	tilemap_set_transparent_pen(madmotor_pf2_tilemap,0);
	tilemap_set_scroll_rows(madmotor_pf1_tilemap,512);
}

/******************************************************************************/

READ16_HANDLER( madmotor_pf1_data_r )
{
	return madmotor_pf1_data[offset];
}

READ16_HANDLER( madmotor_pf2_data_r )
{
	return madmotor_pf2_data[offset];
}

READ16_HANDLER( madmotor_pf3_data_r )
{
	return madmotor_pf3_data[offset];
}

WRITE16_HANDLER( madmotor_pf1_data_w )
{
	COMBINE_DATA(&madmotor_pf1_data[offset]);
	tilemap_mark_tile_dirty(madmotor_pf1_tilemap,offset);
}

WRITE16_HANDLER( madmotor_pf2_data_w )
{
	COMBINE_DATA(&madmotor_pf2_data[offset]);
	tilemap_mark_tile_dirty(madmotor_pf2_tilemap,offset);
}

WRITE16_HANDLER( madmotor_pf3_data_w )
{
	COMBINE_DATA(&madmotor_pf3_data[offset]);

	/* Mark the dirty position on the 512 x 1024 version */
	tilemap_mark_tile_dirty(madmotor_pf3_tilemap,offset);

	/* Mark the dirty position on the 2048 x 256 version */
	tilemap_mark_tile_dirty(madmotor_pf3a_tilemap,offset);
}

WRITE16_HANDLER( madmotor_pf1_control_w )
{
	COMBINE_DATA(&madmotor_pf1_control[offset]);
}

WRITE16_HANDLER( madmotor_pf2_control_w )
{
	COMBINE_DATA(&madmotor_pf2_control[offset]);
}

WRITE16_HANDLER( madmotor_pf3_control_w )
{
	COMBINE_DATA(&madmotor_pf3_control[offset]);
}

READ16_HANDLER( madmotor_pf1_rowscroll_r )
{
	return madmotor_pf1_rowscroll[offset];
}

WRITE16_HANDLER( madmotor_pf1_rowscroll_w )
{
	COMBINE_DATA(&madmotor_pf1_rowscroll[offset]);
}

/******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int pri_mask,int pri_val)
{
	int offs;

	offs = 0;
	while (offs < spriteram_size/2)
	{
		int sx,sy,code,color,w,h,flipx,flipy,incy,flash,mult,x,y;

		sy = spriteram16[offs];
		sx = spriteram16[offs+2];
		color = sx >> 12;

		flash=sx&0x800;

		flipx = sy & 0x2000;
		flipy = sy & 0x4000;
		h = (1 << ((sy & 0x1800) >> 11));	/* 1x, 2x, 4x, 8x height */
		w = (1 << ((sy & 0x0600) >>  9));	/* 1x, 2x, 4x, 8x width */
		/* multi width used only on the title screen? */

		code = spriteram16[offs+1] & 0x1fff;

		sx = sx & 0x01ff;
		sy = sy & 0x01ff;
		if (sx >= 256) sx -= 512;
		if (sy >= 256) sy -= 512;
		sx = 240 - sx;
		sy = 240 - sy;

		code &= ~(h-1);
		if (flipy)
			incy = -1;
		else
		{
			code += h-1;
			incy = 1;
		}

		if (flipscreen) {
			sy=240-sy;
			sx=240-sx;
			if (flipx) flipx=0; else flipx=1;
			if (flipy) flipy=0; else flipy=1;
			mult=16;
		}
		else mult=-16;

		for (x = 0;x < w;x++)
		{
			for (y = 0;y < h;y++)
			{
				if ((color & pri_mask) == pri_val &&
							(!flash || (cpu_getcurrentframe() & 1)))
					drawgfx(bitmap,machine->gfx[3],
							code - y * incy + h * x,
							color,
							flipx,flipy,
							sx + mult * x,sy + mult * y,
							cliprect,TRANSPARENCY_PEN,0);
			}

			offs += 4;
			if (offs >= spriteram_size/2 ||
					spriteram16[offs] & 0x8000)	// seems the expected behaviour on the title screen
				 break;
		}
	}
}


/******************************************************************************/

VIDEO_UPDATE( madmotor )
{
	int offs;

	/* Update flipscreen */
	if (madmotor_pf1_control[0]&0x80)
		flipscreen=1;
	else
		flipscreen=0;
	tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Setup scroll registers */
	for (offs = 0;offs < 512;offs++)
		tilemap_set_scrollx( madmotor_pf1_tilemap,offs, madmotor_pf1_control[0x08] + madmotor_pf1_rowscroll[0x200+offs] );
	tilemap_set_scrolly( madmotor_pf1_tilemap,0, madmotor_pf1_control[0x09] );
	tilemap_set_scrollx( madmotor_pf2_tilemap,0, madmotor_pf2_control[0x08] );
	tilemap_set_scrolly( madmotor_pf2_tilemap,0, madmotor_pf2_control[0x09] );
	tilemap_set_scrollx( madmotor_pf3_tilemap,0, madmotor_pf3_control[0x08] );
	tilemap_set_scrolly( madmotor_pf3_tilemap,0, madmotor_pf3_control[0x09] );
	tilemap_set_scrollx( madmotor_pf3a_tilemap,0, madmotor_pf3_control[0x08] );
	tilemap_set_scrolly( madmotor_pf3a_tilemap,0, madmotor_pf3_control[0x09] );

	/* Draw playfields & sprites */
	if (madmotor_pf3_control[0x03]==2)
		tilemap_draw(bitmap,cliprect,madmotor_pf3_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,madmotor_pf3a_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,madmotor_pf2_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect,0x00,0x00);
	tilemap_draw(bitmap,cliprect,madmotor_pf1_tilemap,0,0);
	return 0;
}
