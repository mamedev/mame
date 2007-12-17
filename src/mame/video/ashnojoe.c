/********************************************************************

    Ashita no Joe (Success Joe) [Wave]
    video hardware emulation

*********************************************************************/

#include "driver.h"

UINT16 *ashnojoetileram16, *ashnojoetileram16_2, *ashnojoetileram16_3, *ashnojoetileram16_4, *ashnojoetileram16_5, *ashnojoetileram16_6, *ashnojoetileram16_7;
static tilemap *joetilemap, *joetilemap2, *joetilemap3, *joetilemap4, *joetilemap5, *joetilemap6, *joetilemap7;

UINT16 *ashnojoe_tilemap_reg;

static TILE_GET_INFO( get_joe_tile_info )
{
	int code = ashnojoetileram16 [tile_index];

	SET_TILE_INFO(
			2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

static TILE_GET_INFO( get_joe_tile_info_2 )
{
	int code = ashnojoetileram16_2 [tile_index*2];
	int attr = ashnojoetileram16_2 [tile_index*2+1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_3 )
{
	int code = ashnojoetileram16_3 [tile_index];

	SET_TILE_INFO(
			0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_4 )
{
	int code = ashnojoetileram16_4 [tile_index];

	SET_TILE_INFO(
			1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_5 )
{
	int code = ashnojoetileram16_5 [tile_index*2];
	int attr = ashnojoetileram16_5 [tile_index*2+1];

	SET_TILE_INFO(
			4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

static TILE_GET_INFO( get_joe_tile_info_6 )
{
	int code = ashnojoetileram16_6 [tile_index*2];
	int attr = ashnojoetileram16_6 [tile_index*2+1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


static TILE_GET_INFO( get_joe_tile_info_7 )
{
	int code = ashnojoetileram16_7 [tile_index*2];
	int attr = ashnojoetileram16_7 [tile_index*2+1];

	SET_TILE_INFO(
			3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}

WRITE16_HANDLER( ashnojoe_tileram_w )
{
	ashnojoetileram16[offset] = data;
	tilemap_mark_tile_dirty(joetilemap,offset);
}


WRITE16_HANDLER( ashnojoe_tileram2_w )
{
	ashnojoetileram16_2[offset] = data;
	tilemap_mark_tile_dirty(joetilemap2,offset/2);
}

WRITE16_HANDLER( ashnojoe_tileram3_w )
{
	ashnojoetileram16_3[offset] = data;
	tilemap_mark_tile_dirty(joetilemap3,offset);
}

WRITE16_HANDLER( ashnojoe_tileram4_w )
{
	ashnojoetileram16_4[offset] = data;
	tilemap_mark_tile_dirty(joetilemap4,offset);
}

WRITE16_HANDLER( ashnojoe_tileram5_w )
{
	ashnojoetileram16_5[offset] = data;
	tilemap_mark_tile_dirty(joetilemap5,offset/2);
}

WRITE16_HANDLER( ashnojoe_tileram6_w )
{
	ashnojoetileram16_6[offset] = data;
	tilemap_mark_tile_dirty(joetilemap6,offset/2);
}

WRITE16_HANDLER( ashnojoe_tileram7_w )
{
	ashnojoetileram16_7[offset] = data;
	tilemap_mark_tile_dirty(joetilemap7,offset/2);
}

WRITE16_HANDLER( joe_tilemaps_xscroll_w )
{
	switch( offset )
	{
	case 0:
		tilemap_set_scrollx(joetilemap3,0,data);
		break;
	case 1:
		tilemap_set_scrollx(joetilemap5,0,data);
		break;
	case 2:
		tilemap_set_scrollx(joetilemap2,0,data);
		break;
	case 3:
		tilemap_set_scrollx(joetilemap4,0,data);
		break;
	case 4:
		tilemap_set_scrollx(joetilemap6,0,data);
		tilemap_set_scrollx(joetilemap7,0,data);
		break;
	}
}

WRITE16_HANDLER( joe_tilemaps_yscroll_w )
{
	switch( offset )
	{
	case 0:
		tilemap_set_scrolly(joetilemap3,0,data);
		break;
	case 1:
		tilemap_set_scrolly(joetilemap5,0,data);
		break;
	case 2:
		tilemap_set_scrolly(joetilemap2,0,data);
		break;
	case 3:
		tilemap_set_scrolly(joetilemap4,0,data);
		break;
	case 4:
		tilemap_set_scrolly(joetilemap6,0,data);
		tilemap_set_scrolly(joetilemap7,0,data);
		break;
	}
}

VIDEO_START( ashnojoe )
{
	joetilemap  =  tilemap_create(get_joe_tile_info,  tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	joetilemap2 =  tilemap_create(get_joe_tile_info_2,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	joetilemap3 =  tilemap_create(get_joe_tile_info_3,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	joetilemap4 =  tilemap_create(get_joe_tile_info_4,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,64);
	joetilemap5 =  tilemap_create(get_joe_tile_info_5,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,32,32);
	joetilemap6 =  tilemap_create(get_joe_tile_info_6,tilemap_scan_rows,TILEMAP_TYPE_PEN,		16,16,32,32);
	joetilemap7 =  tilemap_create(get_joe_tile_info_7,tilemap_scan_rows,TILEMAP_TYPE_PEN,		16,16,32,32);

	tilemap_set_transparent_pen(joetilemap, 15);
	tilemap_set_transparent_pen(joetilemap2,15);
	tilemap_set_transparent_pen(joetilemap3,15);
	tilemap_set_transparent_pen(joetilemap4,15);
	tilemap_set_transparent_pen(joetilemap5,15);
}

VIDEO_UPDATE( ashnojoe )
{

//  ashnojoe_tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(ashnojoe_tilemap_reg[0] & 1);

	if(ashnojoe_tilemap_reg[0] & 0x02)
		tilemap_draw(bitmap,cliprect,joetilemap7,0,0);
	else
		tilemap_draw(bitmap,cliprect,joetilemap6,0,0);

	tilemap_draw(bitmap,cliprect,joetilemap4,0,0);
	tilemap_draw(bitmap,cliprect,joetilemap2,0,0);
	tilemap_draw(bitmap,cliprect,joetilemap5,0,0);
	tilemap_draw(bitmap,cliprect,joetilemap3,0,0);
	tilemap_draw(bitmap,cliprect,joetilemap, 0,0);

	return 0;
}
