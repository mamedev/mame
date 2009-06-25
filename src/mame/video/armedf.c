#include "driver.h"
#include "includes/armedf.h"


UINT16 armedf_vreg;

UINT16 *terraf_text_videoram;
UINT16 *armedf_bg_videoram;
UINT16 *armedf_fg_videoram;
UINT16 *legion_cmd;
static UINT16 armedf_fg_scrollx,armedf_fg_scrolly;

static UINT16 terraf_scroll_msb;

static tilemap *bg_tilemap, *fg_tilemap;
tilemap *armedf_tx_tilemap;

static int scroll_type,sprite_offy, mcu_mode, old_mcu_mode;

void armedf_setgfxtype( int type )
{
	scroll_type = type;
	mcu_mode = 0;
	old_mcu_mode = 0;
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( armedf_scan )
{ /* col: 0..63; row: 0..31 */
	switch( scroll_type )
	{
	case 1: /* armed formation */
		return col*32+row;

	case 3: /* legion */
	case 6: /* legiono */
		return (col&0x1f)*32+row+0x800*(col/32);

	default:
		return 32*(31-row)+(col&0x1f)+0x800*(col/32);
	}
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int tile_number = terraf_text_videoram[tile_index]&0xff;
	int attributes;

	if( scroll_type == 1 )
	{
		attributes = terraf_text_videoram[tile_index+0x800]&0xff;
	}
	else
	{
		attributes = terraf_text_videoram[tile_index+0x400]&0xff;
	}
	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int data = armedf_fg_videoram[tile_index];
	SET_TILE_INFO(
			1,
			data&0x7ff,
			data>>11,
			0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	int data = armedf_bg_videoram[tile_index];
	SET_TILE_INFO(
			2,
			data&0x3ff,
			data>>11,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( armedf )
{
	if( scroll_type == 4 || /* cclimbr2 */
		scroll_type == 3 || /* legion */
		scroll_type == 6 )  /* legiono */
	{
		sprite_offy = 0;
	}
	else
	{
		sprite_offy = 128;
	}

	//bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,16,16,64,32);
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,16,16,64,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,16,16,64,32);
	armedf_tx_tilemap = tilemap_create(machine, get_tx_tile_info,armedf_scan,8,8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0xf);
	tilemap_set_transparent_pen(armedf_tx_tilemap,0xf);
	tilemap_set_transparent_pen(bg_tilemap,0xf);

	if( scroll_type!=1 )
	{
		tilemap_set_scrollx(armedf_tx_tilemap,0,-128);
	}
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( armedf_text_videoram_w )
{
	COMBINE_DATA(&terraf_text_videoram[offset]);
	if( scroll_type == 1 )
	{
		tilemap_mark_tile_dirty(armedf_tx_tilemap,offset & 0x7ff);
	}
	else
	{
		tilemap_mark_tile_dirty(armedf_tx_tilemap,offset & 0xbff);
	}

/*  if (offset<0x10)
        logerror("%04x %04x %04x %04x %04x %04x %04x %04x-%04x %04x %04x %04x %04x %04x %04x %04x (%04x)\n",
            terraf_text_videoram[0], terraf_text_videoram[1], terraf_text_videoram[2],
            terraf_text_videoram[3], terraf_text_videoram[4], terraf_text_videoram[5],
            terraf_text_videoram[6], terraf_text_videoram[7], terraf_text_videoram[8],
            terraf_text_videoram[9], terraf_text_videoram[10], terraf_text_videoram[11],
            terraf_text_videoram[12], terraf_text_videoram[13], terraf_text_videoram[14],
            terraf_text_videoram[15], offset);*/
}


WRITE16_HANDLER( armedf_fg_videoram_w )
{
	COMBINE_DATA(&armedf_fg_videoram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE16_HANDLER( armedf_bg_videoram_w )
{
	COMBINE_DATA(&armedf_bg_videoram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static int waiting_msb;

WRITE16_HANDLER( terraf_fg_scrollx_w )
{
	if (ACCESSING_BITS_8_15)
	{
		armedf_fg_scrollx = data >> 8;
		waiting_msb = 1;
	}
}

WRITE16_HANDLER( terraf_fg_scrolly_w )
{
	if (ACCESSING_BITS_8_15)
	{
		if (waiting_msb)
			terraf_scroll_msb = data >> 8;
		else
			armedf_fg_scrolly = data >> 8;
	}
}

WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w )
{
	if (ACCESSING_BITS_8_15)
		waiting_msb = 0;
}

WRITE16_HANDLER( armedf_fg_scrollx_w )
{
	COMBINE_DATA(&armedf_fg_scrollx);
}

WRITE16_HANDLER( armedf_fg_scrolly_w )
{
	COMBINE_DATA(&armedf_fg_scrolly);
}

WRITE16_HANDLER( armedf_bg_scrollx_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrollx(bg_tilemap,0,scroll);
}

WRITE16_HANDLER( armedf_bg_scrolly_w )
{
	static UINT16 scroll;
	COMBINE_DATA(&scroll);
	tilemap_set_scrolly(bg_tilemap,0,scroll);
}

WRITE16_HANDLER( armedf_mcu_cmd )
{
	COMBINE_DATA(&mcu_mode);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	int offs;

	for (offs = 0;offs < spriteram_size/2;offs += 4)
	{
		int code = buffered_spriteram16[offs+1]; /* ??YX?TTTTTTTTTTT */
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color = (buffered_spriteram16[offs+2]>>8)&0x1f;
		int sx = buffered_spriteram16[offs+3];
		int sy = sprite_offy+240-(buffered_spriteram16[offs+0]&0x1ff);

		if (flip_screen_get(machine)) {
			sx = 320 - sx + 176;	/* don't ask where 176 comes from, just tried it out */
			sy = 240 - sy + 1;		/* don't ask where 1 comes from, just tried it out */
			flipx = !flipx;			/* the values seem to result in pixel-correct placement */
			flipy = !flipy;			/* in all the games supported by this driver */
		}

		if (((buffered_spriteram16[offs+0] & 0x3000) >> 12) == priority)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
				code & 0xfff,
				color,
 				flipx,flipy,
				sx,sy,15);
		}
	}
}



VIDEO_UPDATE( armedf )
{
	int sprite_enable = armedf_vreg & 0x200;

	tilemap_set_enable( bg_tilemap, armedf_vreg&0x800 );
	tilemap_set_enable( fg_tilemap, armedf_vreg&0x400 );
	tilemap_set_enable( armedf_tx_tilemap, armedf_vreg&0x100 );

	if ((scroll_type == 0)||(scroll_type == 5 )) {
		if (old_mcu_mode!=mcu_mode) {
			if ((mcu_mode&0x000f)==0x0004) {		// transparent tx
				tilemap_set_transparent_pen(armedf_tx_tilemap, 0x0f);
				tilemap_mark_all_tiles_dirty( armedf_tx_tilemap );
				//logerror("? Transparent TX 0x0f\n");
			}
			if ((mcu_mode&0x000f)==0x000f) {		// opaque tx
				tilemap_set_transparent_pen(armedf_tx_tilemap, 0x10);
				tilemap_mark_all_tiles_dirty( armedf_tx_tilemap );
				//logerror("? Opaque TX\n");
			}

			old_mcu_mode = mcu_mode;
			//logerror("MCU Change => %04x\n",mcu_mode);
		}
	}

	switch (scroll_type)
	{
		case 0: /* terra force */
			tilemap_set_scrollx( fg_tilemap, 0, armedf_fg_scrolly + ((terraf_scroll_msb>>4)&3)*256 );
			tilemap_set_scrolly( fg_tilemap, 0, armedf_fg_scrollx + ((terraf_scroll_msb)&3)*256 );
			break;

		case 1: /* armed formation */
			tilemap_set_scrollx( fg_tilemap, 0, armedf_fg_scrollx );
			tilemap_set_scrolly( fg_tilemap, 0, armedf_fg_scrolly );
			break;

		case 6: /* legiono */
			tilemap_set_scrollx( fg_tilemap, 0, (legion_cmd[13] & 0xff) | ((legion_cmd[14] & 0x3)<<8) );
			tilemap_set_scrolly( fg_tilemap, 0, (legion_cmd[11] & 0xff) | ((legion_cmd[12] & 0x3)<<8) );
			break;
		case 2: /* kodure ookami */
		case 3:
		case 4: /* crazy climber 2 */
			{
				int scrollx,scrolly;

				/* scrolling is handled by the protection mcu */
				scrollx = (terraf_text_videoram[13] & 0xff) | (terraf_text_videoram[14] << 8);
				scrolly = (terraf_text_videoram[11] & 0xff) | (terraf_text_videoram[12] << 8);
				tilemap_set_scrollx( fg_tilemap, 0, scrollx);
				tilemap_set_scrolly( fg_tilemap, 0, scrolly);

			}
			break;
		case 5: /* terra force (US) */
			tilemap_set_scrollx( fg_tilemap, 0, (terraf_text_videoram[13] & 0xff) | ((terraf_text_videoram[14] & 0x3)<<8) );
			tilemap_set_scrolly( fg_tilemap, 0, (terraf_text_videoram[11] & 0xff) | ((terraf_text_videoram[12] & 0x3)<<8) );
			break;

	}


	bitmap_fill( bitmap, cliprect , 0xff);
	if (armedf_vreg & 0x0800) tilemap_draw( bitmap, cliprect, bg_tilemap, 0, 0);
	/*if( armedf_vreg & 0x0800 )
    {
        tilemap_draw( bitmap, cliprect, bg_tilemap, 0, 0);
    }
    else
    {
        bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine)&0x0f);
    }*/

	if ((mcu_mode&0x0030)==0x0030) tilemap_draw( bitmap, cliprect, armedf_tx_tilemap, 0, 0);
	if( sprite_enable ) draw_sprites(screen->machine, bitmap, cliprect, 2 );
	if ((mcu_mode&0x0030)==0x0020) tilemap_draw( bitmap, cliprect, armedf_tx_tilemap, 0, 0);
	tilemap_draw( bitmap, cliprect, fg_tilemap, 0, 0);
	if ((mcu_mode&0x0030)==0x0010) tilemap_draw( bitmap, cliprect, armedf_tx_tilemap, 0, 0);
	if( sprite_enable ) draw_sprites(screen->machine, bitmap, cliprect, 1 );
	if ((mcu_mode&0x0030)==0x0000) tilemap_draw( bitmap, cliprect, armedf_tx_tilemap, 0, 0);
	if( sprite_enable ) draw_sprites(screen->machine, bitmap, cliprect, 0 );

	return 0;
}

VIDEO_EOF( armedf )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space,0,0,0xffff);
}
