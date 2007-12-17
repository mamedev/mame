/*
Car Jamboree
Omori Electric CAD (OEC) 1981
*/

#include "driver.h"

static tilemap *carjmbre_tilemap;

static UINT8 carjmbre_flipscreen;
static UINT16 carjmbre_bgcolor;

PALETTE_INIT( carjmbre )
{
	int i,bit0,bit1,bit2,r,g,b;

	for (i = 0;i < machine->drv->total_colors; i++)
	{
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
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( carjmbre_flipscreen_w )
{
	carjmbre_flipscreen = data?(TILEMAP_FLIPX|TILEMAP_FLIPY):0;
	tilemap_set_flip( ALL_TILEMAPS,carjmbre_flipscreen );
}

WRITE8_HANDLER( carjmbre_bgcolor_w )
{
	int oldbg,i;

	oldbg=carjmbre_bgcolor;

	carjmbre_bgcolor&=0xff00>>(offset*8);
	carjmbre_bgcolor|=((~data)&0xff)<<(offset*8);

	if(oldbg!=carjmbre_bgcolor)
	{
		memset(dirtybuffer,1,videoram_size);

		for (i=0;i<64;i+=4)
			palette_set_color_rgb(Machine, i, (carjmbre_bgcolor&0xff)*0x50, (carjmbre_bgcolor&0xff)*0x50, (carjmbre_bgcolor&0xff)!=0?0x50:0);
	}
}

static TILE_GET_INFO( get_carjmbre_tile_info ){
	UINT32 tile_number = videoram[tile_index] & 0xFF;
	UINT8 attr  = videoram[tile_index+0x400];
	tile_number += (attr & 0x80) << 1; /* bank */
	SET_TILE_INFO(
			0,
			tile_number,
			(attr&0x7),
			0);
}

WRITE8_HANDLER( carjmbre_videoram_w ){
	videoram[offset] = data;
	tilemap_mark_tile_dirty(carjmbre_tilemap,offset&0x3ff);
}



VIDEO_START( carjmbre )
{

	carjmbre_tilemap = tilemap_create( get_carjmbre_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );

	state_save_register_global(carjmbre_flipscreen);
	state_save_register_global(carjmbre_bgcolor);
}

VIDEO_UPDATE( carjmbre )
{
	int offs,troffs,sx,sy,flipx,flipy;

	//colorram
	//76543210
	//x------- graphic bank
	//-xxx---- unused
	//----x--- ?? probably colour, only used for ramp and pond
	//-----xxx colour

	tilemap_draw( bitmap,cliprect,carjmbre_tilemap,0,0 );

	//spriteram[offs]
	//+0       y pos
	//+1       sprite number
	//+2
	//76543210
	//x------- flipy
	//-x------ flipx
	//--xx---- unused
	//----x--- ?? probably colour
	//-----xxx colour
	//+3       x pos
	for (offs = spriteram_size-4; offs >= 0; offs-=4)
	{
		//before copying the sprites to spriteram the game reorders the first
		//sprite to last, sprite ordering is incorrect if this isn't undone
		troffs=(offs-4+spriteram_size)%spriteram_size;

		//unused sprites are marked with ypos <= 0x02 (or >= 0xfd if screen flipped)
		if (spriteram[troffs] > 0x02 && spriteram[troffs] < 0xfd)
		{
			{
				sx = spriteram[troffs+3]-7;
				sy = 241-spriteram[troffs];
				flipx = (spriteram[troffs+2]&0x40)>>6;
				flipy = (spriteram[troffs+2]&0x80)>>7;

				if (carjmbre_flipscreen)
				{
					sx = (256+(226-sx))%256;
					sy = 242-sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx(bitmap,machine->gfx[1],
						spriteram[troffs+1],
						spriteram[troffs+2]&0x07,
						flipx,flipy,
						sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
	return 0;
}
