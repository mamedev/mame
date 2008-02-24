/* video hardware for Namco System II */

#include "driver.h"
#include "deprecat.h"
#include "namcos2.h"
#include "namcoic.h"

UINT16 *namcos2_sprite_ram;
UINT16 *namcos2_68k_palette_ram;
size_t namcos2_68k_palette_size;
//size_t namcos2_68k_roz_ram_size;
UINT16 *namcos2_68k_roz_ram;

static UINT16 namcos2_68k_roz_ctrl[0x8];
static tilemap *tilemap_roz;

static void
TilemapCB( UINT16 code, int *tile, int *mask )
{
	*mask = code;

	switch( namcos2_gametype )
	{
	case NAMCOS2_FINAL_LAP_2:
	case NAMCOS2_FINAL_LAP_3:
		*tile = (code&0x07ff)|((code&0x4000)>>3)|((code&0x3800)<<1);
		break;

	default:
		/* The order of bits needs to be corrected to index the right tile  14 15 11 12 13 */
		*tile = (code&0x07ff)|((code&0xc000)>>3)|((code&0x3800)<<2);
		break;
	}
} /* TilemapCB */

/**
 * namcos2_gfx_ctrl selects a bank of 128 sprites within spriteram
 *
 * namcos2_gfx_ctrl also supplies palette and priority information that is applied to the output of the
 *                  Namco System 2 ROZ chip
 *
 * -xxx ---- ---- ---- roz priority
 * ---- xxxx ---- ---- roz palette
 * ---- ---- xxxx ---- always zero?
 * ---- ---- ---- xxxx sprite bank
 */
static UINT16 namcos2_gfx_ctrl;

READ16_HANDLER( namcos2_gfx_ctrl_r )
{
	return namcos2_gfx_ctrl;
} /* namcos2_gfx_ctrl_r */

WRITE16_HANDLER( namcos2_gfx_ctrl_w )
{
	COMBINE_DATA(&namcos2_gfx_ctrl);
} /* namcos2_gfx_ctrl_w */

static TILE_GET_INFO( get_tile_info_roz )
{
	int tile = namcos2_68k_roz_ram[tile_index];
	SET_TILE_INFO(3,tile,0/*color*/,0);
} /* get_tile_info_roz */

struct RozParam
{
	UINT32 size;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int color;
	int wrap;
};

static void
DrawRozHelper(
	mame_bitmap *bitmap,
	tilemap *tmap,
	const rectangle *clip,
	const struct RozParam *rozInfo )
{
	tilemap_set_palette_offset( tmap, rozInfo->color );

	if( bitmap->bpp == 16 )
	{
		UINT32 size_mask = rozInfo->size-1;
		mame_bitmap *srcbitmap = tilemap_get_pixmap( tmap );
		mame_bitmap *flagsbitmap = tilemap_get_flagsmap( tmap );
		UINT32 startx = rozInfo->startx + clip->min_x * rozInfo->incxx + clip->min_y * rozInfo->incyx;
		UINT32 starty = rozInfo->starty + clip->min_x * rozInfo->incxy + clip->min_y * rozInfo->incyy;
		int sx = clip->min_x;
		int sy = clip->min_y;
		while( sy <= clip->max_y )
		{
			int x = sx;
			UINT32 cx = startx;
			UINT32 cy = starty;
			UINT16 *dest = BITMAP_ADDR16(bitmap, sy, sx);
			while( x <= clip->max_x )
			{
				UINT32 xpos = (cx>>16);
				UINT32 ypos = (cy>>16);
				if( rozInfo->wrap )
				{
					xpos &= size_mask;
					ypos &= size_mask;
				}
				else if( xpos>rozInfo->size || ypos>=rozInfo->size )
				{
					goto L_SkipPixel;
				}

				if( *BITMAP_ADDR16(flagsbitmap, ypos, xpos)&TILEMAP_PIXEL_LAYER0 )
				{
					*dest = *BITMAP_ADDR16(srcbitmap, ypos, xpos)+rozInfo->color;
				}
L_SkipPixel:
				cx += rozInfo->incxx;
				cy += rozInfo->incxy;
				x++;
				dest++;
			} /* next x */
			startx += rozInfo->incyx;
			starty += rozInfo->incyy;
			sy++;
		} /* next y */
	}
	else
	{
		tilemap_draw_roz(
			bitmap,
			clip,
			tmap,
			rozInfo->startx, rozInfo->starty,
			rozInfo->incxx, rozInfo->incxy,
			rozInfo->incyx, rozInfo->incyy,
			rozInfo->wrap,0,0); // wrap, flags, pri
	}
} /* DrawRozHelper */

static void
DrawROZ(mame_bitmap *bitmap,const rectangle *cliprect)
{
	const int xoffset = 38,yoffset = 0;
	struct RozParam rozParam;

	rozParam.color = (namcos2_gfx_ctrl & 0x0f00);
	rozParam.incxx  = (INT16)namcos2_68k_roz_ctrl[0];
	rozParam.incxy  = (INT16)namcos2_68k_roz_ctrl[1];
	rozParam.incyx  = (INT16)namcos2_68k_roz_ctrl[2];
	rozParam.incyy  = (INT16)namcos2_68k_roz_ctrl[3];
	rozParam.startx = (INT16)namcos2_68k_roz_ctrl[4];
	rozParam.starty = (INT16)namcos2_68k_roz_ctrl[5];
	rozParam.size = 2048;
	rozParam.wrap = 1;


	switch( namcos2_68k_roz_ctrl[7] )
	{
	case 0x4400: /* (2048x2048) */
		break;

	case 0x4488: /* attract mode */
		rozParam.wrap = 0;
		break;

	case 0x44cc: /* stage1 demo */
		rozParam.wrap = 0;
		break;

	case 0x44ee: /* (256x256) used in Dragon Saber */
		rozParam.wrap = 0;
		rozParam.size = 256;
		break;
	}

	rozParam.startx <<= 4;
	rozParam.starty <<= 4;
	rozParam.startx += xoffset * rozParam.incxx + yoffset * rozParam.incyx;
	rozParam.starty += xoffset * rozParam.incxy + yoffset * rozParam.incyy;

	rozParam.startx<<=8;
	rozParam.starty<<=8;
	rozParam.incxx<<=8;
	rozParam.incxy<<=8;
	rozParam.incyx<<=8;
	rozParam.incyy<<=8;

	DrawRozHelper( bitmap, tilemap_roz, cliprect, &rozParam );
}

READ16_HANDLER(namcos2_68k_roz_ctrl_r)
{
	return namcos2_68k_roz_ctrl[offset];
}

WRITE16_HANDLER( namcos2_68k_roz_ctrl_w )
{
	COMBINE_DATA(&namcos2_68k_roz_ctrl[offset]);
}

READ16_HANDLER( namcos2_68k_roz_ram_r )
{
	return namcos2_68k_roz_ram[offset];
}

WRITE16_HANDLER( namcos2_68k_roz_ram_w )
{
	COMBINE_DATA(&namcos2_68k_roz_ram[offset]);
	tilemap_mark_tile_dirty(tilemap_roz,offset);
//      if( input_code_pressed(KEYCODE_Q) )
//      {
//          DEBUGGER_BREAK;
//      }
}

/**************************************************************************/

READ16_HANDLER( namcos2_68k_video_palette_r )
{
	if( (offset&0x1800) == 0x1800 )
	{
		offset &= 0x180f;
		if( offset == 0x180d ) return 0xff;
		if( offset == 0x180f ) return 0xff;
	}
	return namcos2_68k_palette_ram[offset];
} /* namcos2_68k_video_palette_r */

WRITE16_HANDLER( namcos2_68k_video_palette_w )
{
	if( (offset&0x1800) == 0x1800 )
	{
		offset &= 0x180f;
		if( ACCESSING_LSB )
		{
			namcos2_68k_palette_ram[offset] = data;
		}
		else
		{
			namcos2_68k_palette_ram[offset] = data>>8;
		}
	}
	else
	{
		COMBINE_DATA(&namcos2_68k_palette_ram[offset]);
	}
} /* namcos2_68k_video_palette_w */

static UINT16
GetPaletteRegister( int which )
{
	const UINT16 *source = &namcos2_68k_palette_ram[0x3000/2];
	return ((source[which*2]&0xff)<<8) | (source[which*2+1]&0xff);
}

int
namcos2_GetPosIrqScanline( void )
{
	/* PaleteRegister(4)? used by Finest Hour; pc=0x356e */
	int scanline = GetPaletteRegister(5) - 34;
	if( scanline<0 )
	{
		scanline = 0;
	}
	else if( scanline > Machine->screen[0].height )
	{
		scanline = Machine->screen[0].height;
	}
	return scanline;
} /* namcos2_GetPosIrqScanline */

static void
UpdatePalette( void )
{
	int bank;
	for( bank=0; bank<0x20; bank++ )
	{
		int pen = bank*256;
		int offset = ((pen & 0x1800) << 2) | (pen & 0x07ff);
		int i;
		for( i=0; i<256; i++ )
		{
			int r = namcos2_68k_palette_ram[offset | 0x0000] & 0x00ff;
			int g = namcos2_68k_palette_ram[offset | 0x0800] & 0x00ff;
			int b = namcos2_68k_palette_ram[offset | 0x1000] & 0x00ff;
			palette_set_color(Machine,pen++,MAKE_RGB(r,g,b));
			offset++;
		}
	}
} /* UpdatePalette */

/**************************************************************************/

static void
DrawSpriteInit( void )
{
	int i;
	/* set table for sprite color == 0x0f */
	for(i = 0;i <= 253;i++)
	{
		gfx_drawmode_table[i] = DRAWMODE_SOURCE;
	}
	gfx_drawmode_table[254] = DRAWMODE_SHADOW;
	gfx_drawmode_table[255] = DRAWMODE_NONE;
	for( i = 0; i<16*256; i++ )
	{
		Machine->shadow_table[i] = i+0x2000;
	}
}

WRITE16_HANDLER( namcos2_sprite_ram_w )
{
	COMBINE_DATA(&namcos2_sprite_ram[offset]);
}

READ16_HANDLER( namcos2_sprite_ram_r )
{
	return namcos2_sprite_ram[offset];
}

/**************************************************************************/

VIDEO_START( namcos2 )
{
	namco_tilemap_init(2,memory_region(REGION_GFX4),TilemapCB);
	tilemap_roz = tilemap_create(get_tile_info_roz,tilemap_scan_rows,8,8,256,256);
	tilemap_set_transparent_pen(tilemap_roz,0xff);
	DrawSpriteInit();
}

static void
ApplyClip( rectangle *clip, const rectangle *cliprect )
{
	clip->min_x = GetPaletteRegister(0) - 0x4a;
	clip->max_x = GetPaletteRegister(1) - 0x4a - 1;
	clip->min_y = GetPaletteRegister(2) - 0x21;
	clip->max_y = GetPaletteRegister(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	if( clip->min_x < cliprect->min_x ){ clip->min_x = cliprect->min_x; }
	if( clip->min_y < cliprect->min_y ){ clip->min_y = cliprect->min_y; }
	if( clip->max_x > cliprect->max_x ){ clip->max_x = cliprect->max_x; }
	if( clip->max_y > cliprect->max_y ){ clip->max_y = cliprect->max_y; }
} /* ApplyClip */

VIDEO_UPDATE( namcos2_default )
{
	rectangle clip;
	int pri;

	UpdatePalette();
	fillbitmap( bitmap, get_black_pen(machine), cliprect );
	ApplyClip( &clip, cliprect );

	/* HACK: enable ROZ layer only if it has priority > 0 */
	tilemap_set_enable(tilemap_roz,(namcos2_gfx_ctrl & 0x7000) ? 1 : 0);

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );

			if( ((namcos2_gfx_ctrl & 0x7000) >> 12)==pri/2 )
			{
				DrawROZ(bitmap,&clip);
			}
			namcos2_draw_sprites(machine, bitmap, &clip, pri/2, namcos2_gfx_ctrl );
		}
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( finallap )
{
	namco_tilemap_init(2,memory_region(REGION_GFX4),TilemapCB);
	DrawSpriteInit();
	namco_road_init(machine, 3);
}

VIDEO_UPDATE( finallap )
{
	rectangle clip;
	int pri;

	UpdatePalette();
	fillbitmap( bitmap, get_black_pen(machine), cliprect );
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_road_draw(machine, bitmap,&clip,pri );
		namcos2_draw_sprites(machine, bitmap,&clip,pri,namcos2_gfx_ctrl );
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( luckywld )
{
	namco_tilemap_init(2,memory_region(REGION_GFX4),TilemapCB);
	namco_obj_init( 0, 0x0, NULL );
	if( namcos2_gametype==NAMCOS2_LUCKY_AND_WILD )
	{
		namco_roz_init( 1, REGION_GFX5 );
	}
	if( namcos2_gametype!=NAMCOS2_STEEL_GUNNER_2 )
	{
		namco_road_init(machine, 3);
	}
} /* luckywld */

VIDEO_UPDATE( luckywld )
{
	rectangle clip;
	int pri;

	UpdatePalette();
	fillbitmap( bitmap, get_black_pen(machine), cliprect );
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_road_draw(machine, bitmap,&clip,pri );
		if( namcos2_gametype==NAMCOS2_LUCKY_AND_WILD )
		{
			namco_roz_draw( bitmap, &clip, pri );
		}
		namco_obj_draw(machine, bitmap, &clip, pri );
	}
	return 0;
}

/**************************************************************************/

VIDEO_START( sgunner )
{
	namco_tilemap_init(2,memory_region(REGION_GFX4),TilemapCB);
	namco_obj_init( 0, 0x0, NULL );
}

VIDEO_UPDATE( sgunner )
{
	rectangle clip;
	int pri;

	UpdatePalette();
	fillbitmap( bitmap, get_black_pen(machine), cliprect );
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<8; pri++ )
	{
		namco_tilemap_draw( bitmap, &clip, pri );
		namco_obj_draw(machine, bitmap, &clip, pri );
	}
	return 0;
}


/**************************************************************************/

VIDEO_START( metlhawk )
{
	namco_tilemap_init(2,memory_region(REGION_GFX4),TilemapCB);
	namco_roz_init( 1, REGION_GFX5 );
}

VIDEO_UPDATE( metlhawk )
{
	rectangle clip;
	int pri;

	UpdatePalette();
	fillbitmap( bitmap, get_black_pen(machine), cliprect );
	ApplyClip( &clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( bitmap, &clip, pri/2 );
		}
		namco_roz_draw( bitmap, &clip, pri );
		namcos2_draw_sprites_metalhawk(machine, bitmap,&clip,pri );
	}
	return 0;
}
