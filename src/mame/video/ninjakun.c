#include "driver.h"
#include "sound/ay8910.h"


UINT8 *ninjakun_fg_videoram;
UINT8 *ninjakun_bg_videoram;
static tilemap *bg_tilemap, *fg_tilemap;
static UINT8 flipscreen;
static UINT8 ninjakun_xscroll,ninjakun_yscroll;

static UINT8 ninjakun_io_8000_ctrl[4];
// static UINT8 old_scroll;

/*******************************************************************************
 Tilemap Callbacks
*******************************************************************************/

static TILE_GET_INFO( get_fg_tile_info ){
	UINT32 tile_number = ninjakun_fg_videoram[tile_index] & 0xFF;
	UINT8 attr  = ninjakun_fg_videoram[tile_index+0x400];
	tile_number += (attr & 0x20) << 3; /* bank */
	SET_TILE_INFO(
			0,
			tile_number,
			(attr&0xf),
			0);
}

static TILE_GET_INFO( get_bg_tile_info ){
	UINT32 tile_number = ninjakun_bg_videoram[tile_index] & 0xFF;
	UINT8 attr  = ninjakun_bg_videoram[tile_index+0x400];
	tile_number += (attr & 0xC0) << 2; /* bank */
	SET_TILE_INFO(
			1,
			tile_number,
			(attr&0xf),
			0);
}

WRITE8_HANDLER( ninjakun_fg_videoram_w ){
	ninjakun_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset&0x3ff);
}

WRITE8_HANDLER( ninjakun_bg_videoram_w ){

	int y = (offset + ((ninjakun_yscroll & 0xf8) << 2) ) & 0x3e0;
	int x = (offset + (ninjakun_xscroll >> 3) ) & 0x1f;
	int offs = x+y+(offset & 0x400);

	ninjakun_bg_videoram[offs] = data;
	tilemap_mark_tile_dirty(bg_tilemap,x+y);
}

READ8_HANDLER( ninjakun_bg_videoram_r )
{
	int y = (offset + ((ninjakun_yscroll & 0xf8) << 2) ) & 0x3e0;
	int x = (offset + (ninjakun_xscroll >> 3) ) & 0x1f;
	int offs = x+y+(offset & 0x400);

	return ninjakun_bg_videoram[offs];
}

/******************************************************************************/

WRITE8_HANDLER( ninjakun_flipscreen_w ){
	flipscreen = data?(TILEMAP_FLIPX|TILEMAP_FLIPY):0;
	tilemap_set_flip( ALL_TILEMAPS,flipscreen );
}

READ8_HANDLER( ninjakun_io_8000_r ){
	switch( offset ){
	case 0: /* control */
		return AY8910_read_port_0_r( 0 );

	case 1: /* input read */
		switch( ninjakun_io_8000_ctrl[0] ){
		case 0xf:
			return readinputport(4);
		case 0xe:
			return readinputport(3);
		default:
			return ninjakun_io_8000_ctrl[1];
		}
		break;

	case 2: /* control */
		return AY8910_read_port_1_r( 0 );

	case 3: /* data */
		return ninjakun_io_8000_ctrl[3];
	}

//  logerror("PC=%04x; RAM[0x800%d]\n",activecpu_get_pc(),offset);
	return 0xFF;
}

/* static void handle_scrolly( UINT8 new_scroll ){ */

/*  HACK!!!
**
**  New rows are always written at fixed locations above and below the background
**  tilemaps, rather than at the logical screen boundaries with respect to scrolling.
**
**  I don't know how this is handled by the actual NinjaKun hardware, but the
**  following is a crude approximation, yielding a playable game.
*/

/*
    int old_row = old_scroll/8;
    int new_row = new_scroll/8;
    int i;
    if( new_scroll!=old_scroll ){
        tilemap_set_scrolly( bg_tilemap, 0, new_scroll&7 );

        if ((new_row == ((old_row - 1) & 0xff)) || ((!old_row) && (new_row == 0x1f)))
        {
            for( i=0x400-0x21; i>=0; i-- ){
                ninjakun_bg_videoram_w( i+0x20, ninjakun_bg_videoram[i] );
            }
        }
        else if ((new_row == ((old_row + 1) & 0xff)) || ((old_row == 0x1f) && (!new_row)))
        {
            for( i=0x20; i<0x400; i++ ){
                ninjakun_bg_videoram_w( i-0x20, ninjakun_bg_videoram[i] );
            }
        }

        old_scroll = new_scroll;
    }
}
*/


WRITE8_HANDLER( ninjakun_io_8000_w ){
	switch( offset ){
	case 0x0: /* control#1 */
		ninjakun_io_8000_ctrl[0] = data;
		AY8910_control_port_0_w( 0, data );
		break;

	case 0x1: /* data#1 */
		ninjakun_io_8000_ctrl[1] = data;
		switch( ninjakun_io_8000_ctrl[0] ){
		default:
			AY8910_write_port_0_w( 0,data );
			break;
		}
		break;

	case 0x2: /* control#2 */
		ninjakun_io_8000_ctrl[2] = data;
		AY8910_control_port_1_w( 0, data );
		break;

	case 0x3: /* data#2 */
		ninjakun_io_8000_ctrl[3] = data;
		switch( ninjakun_io_8000_ctrl[2] ){
		case 0xf:
				tilemap_set_scrolly( bg_tilemap, 0, data );
				ninjakun_yscroll = data;
			break;
		case 0xe:
			if (flipscreen == 0)
				tilemap_set_scrollx( bg_tilemap, 0, data-7 );
			else
				tilemap_set_scrollx( bg_tilemap, 0, data );
				ninjakun_xscroll = data;
			break;
		default:
			AY8910_write_port_1_w( 0,data );
		}
		break;
	}
}

WRITE8_HANDLER( ninjakun_paletteram_w )
{
	int i;

	paletteram_BBGGRRII_w(offset,data);

	if (offset > 15)
		return;

	if (offset != 1)
	{
		for (i=0; i<16; i++)
		{
			paletteram_BBGGRRII_w(0x200+offset+i*16,data);
		}
	}
	paletteram_BBGGRRII_w(0x200+offset*16+1,data);
}

/*******************************************************************************
 Video Hardware Functions
********************************************************************************
 vh_start / vh_refresh
*******************************************************************************/

VIDEO_START( ninjakun ){
    fg_tilemap = tilemap_create( get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );
	bg_tilemap = tilemap_create( get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );
	tilemap_set_transparent_pen( fg_tilemap,0 );

	/* Save State Support */

	state_save_register_global_array(ninjakun_io_8000_ctrl);
	state_save_register_global(flipscreen);
//  state_save_register_global(old_scroll);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect ){
	const UINT8 *source = spriteram;
	const UINT8 *finish = source+0x800;

	const gfx_element *gfx = machine->gfx[2];

	while( source<finish ){
		int tile_number = source[0];
		int sx = source[1];
		int sy = source[2];
		int attr = source[3];
		int flipx = attr&0x10;
		int flipy = attr&0x20;
		int color = attr&0x0f;

		if( flipscreen ){
			sx = 240-sx;
			sy = 240-sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(
			bitmap,
			gfx,
			tile_number,
			color,
			flipx,flipy,
			sx,sy,
			cliprect,
			TRANSPARENCY_PEN,0
		);
		if (sx>240)
			drawgfx(
				bitmap,
				gfx,
				tile_number,
				color,
				flipx,flipy,
				sx-256,sy,
				cliprect,
				TRANSPARENCY_PEN,0
			);

		source+=0x20;
	}
}

VIDEO_UPDATE( ninjakun )
{
	int offs,chr,col,px,py,x,y;

	tilemap_draw( bitmap,cliprect,bg_tilemap,0,0 );
	tilemap_draw( bitmap,cliprect,fg_tilemap,0,0 );
	draw_sprites( machine, bitmap,cliprect );

	for (y=4; y<28; y++)
	{
		for (x=0; x<32; x++)
		{
			offs = y*32+x;
			chr = ninjakun_fg_videoram[offs];
			col = ninjakun_fg_videoram[offs + 0x400];
			chr +=  (col & 0x20) << 3;

			if ((col & 0x10) == 0)
			{

				if (flipscreen==0)
				{
					px = 8*x;
					py = 8*y;
				}
				else
				{
					px = 248-8*x;
					py = 248-8*y;
				}

				drawgfx(bitmap,machine->gfx[0],
					chr,
					col & 0x0f,
					flipscreen,flipscreen,
					px,py,
					cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}


	return 0;
}
