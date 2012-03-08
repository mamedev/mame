#include "emu.h"
#include "includes/namcos2.h" /* for game-specific hacks */
#include "includes/namcoic.h"

/**************************************************************************************/
static int mPalXOR;		/* XOR'd with palette select register; needed for System21 */

static struct
{
	UINT16 control[0x40/2];
	/**
     * [0x1] 0x02/2 tilemap#0.scrollx
     * [0x3] 0x06/2 tilemap#0.scrolly
     * [0x5] 0x0a/2 tilemap#1.scrollx
     * [0x7] 0x0e/2 tilemap#1.scrolly
     * [0x9] 0x12/2 tilemap#2.scrollx
     * [0xb] 0x16/2 tilemap#2.scrolly
     * [0xd] 0x1a/2 tilemap#3.scrollx
     * [0xf] 0x1e/2 tilemap#3.scrolly
     * 0x20/2 priority
     * 0x30/2 color
     */
	tilemap_t *tmap[6];
	UINT16 *videoram;
	int gfxbank;
	UINT8 *maskBaseAddr;
	void (*cb)( running_machine &machine, UINT16 code, int *gfx, int *mask);
} mTilemapInfo;

void namco_tilemap_invalidate( void )
{
	int i;
	for( i=0; i<6; i++ )
	{
		mTilemapInfo.tmap[i]->mark_all_dirty();
	}
} /* namco_tilemap_invalidate */

INLINE void get_tile_info(running_machine &machine,tile_data &tileinfo,int tile_index,UINT16 *vram)
{
	int tile, mask;
	mTilemapInfo.cb( machine, vram[tile_index], &tile, &mask );
	tileinfo.mask_data = mTilemapInfo.maskBaseAddr+mask*8;
	SET_TILE_INFO(mTilemapInfo.gfxbank,tile,0,0);
} /* get_tile_info */

static TILE_GET_INFO( get_tile_info0 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x0000]); }
static TILE_GET_INFO( get_tile_info1 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x1000]); }
static TILE_GET_INFO( get_tile_info2 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x2000]); }
static TILE_GET_INFO( get_tile_info3 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x3000]); }
static TILE_GET_INFO( get_tile_info4 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x4008]); }
static TILE_GET_INFO( get_tile_info5 ) { get_tile_info(machine,tileinfo,tile_index,&mTilemapInfo.videoram[0x4408]); }

void
namco_tilemap_init( running_machine &machine, int gfxbank, void *maskBaseAddr,
	void (*cb)( running_machine &machine, UINT16 code, int *gfx, int *mask) )
{
	int i;
	mTilemapInfo.gfxbank = gfxbank;
	mTilemapInfo.maskBaseAddr = (UINT8 *)maskBaseAddr;
	mTilemapInfo.cb = cb;
	mTilemapInfo.videoram = auto_alloc_array(machine, UINT16,  0x10000 );

		/* four scrolling tilemaps */
		mTilemapInfo.tmap[0] = tilemap_create(machine, get_tile_info0,tilemap_scan_rows,8,8,64,64);
		mTilemapInfo.tmap[1] = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,8,8,64,64);
		mTilemapInfo.tmap[2] = tilemap_create(machine, get_tile_info2,tilemap_scan_rows,8,8,64,64);
		mTilemapInfo.tmap[3] = tilemap_create(machine, get_tile_info3,tilemap_scan_rows,8,8,64,64);

		/* two non-scrolling tilemaps */
		mTilemapInfo.tmap[4] = tilemap_create(machine, get_tile_info4,tilemap_scan_rows,8,8,36,28);
		mTilemapInfo.tmap[5] = tilemap_create(machine, get_tile_info5,tilemap_scan_rows,8,8,36,28);

		/* define offsets for scrolling */
		for( i=0; i<4; i++ )
		{
			static const int adj[4] = { 4,2,1,0 };
			int dx = 44+adj[i];
			mTilemapInfo.tmap[i]->set_scrolldx( -dx, -(-384-dx) );
			mTilemapInfo.tmap[i]->set_scrolldy( -24, 288 );
		}

		mTilemapInfo.tmap[4]->set_scrolldx( 0, 96 );
		mTilemapInfo.tmap[4]->set_scrolldy( 0, 40 );

		mTilemapInfo.tmap[5]->set_scrolldx( 0, 96 );
		mTilemapInfo.tmap[5]->set_scrolldy( 0, 40 );

} /* namco_tilemap_init */

void
namco_tilemap_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int i;
	for( i=0; i<6; i++ )
	{
		/* note: priority is only in range 0..7, but Point Blank uses 0xf to hide a layer */
		if( (mTilemapInfo.control[0x20/2+i]&0xf) == pri )
		{
			int color = mTilemapInfo.control[0x30/2+i] & 0x07;
			mTilemapInfo.tmap[i]->set_palette_offset( color*256 );
			mTilemapInfo.tmap[i]->draw(bitmap,cliprect,0,0);
		}
	}
} /* namco_tilemap_draw */

static void
SetTilemapVideoram( int offset, UINT16 newword )
{
	mTilemapInfo.videoram[offset] = newword;
	if( offset<0x4000 )
	{
		mTilemapInfo.tmap[offset>>12]->mark_tile_dirty(offset&0xfff);
	}
	else if( offset>=0x8010/2 && offset<0x87f0/2 )
	{ /* fixed plane#1 */
		offset-=0x8010/2;
		mTilemapInfo.tmap[4]->mark_tile_dirty( offset );
	}
	else if( offset>=0x8810/2 && offset<0x8ff0/2 )
	{ /* fixed plane#2 */
		offset-=0x8810/2;
		mTilemapInfo.tmap[5]->mark_tile_dirty( offset );
	}
} /* SetTilemapVideoram */

WRITE16_HANDLER( namco_tilemapvideoram16_w )
{
	UINT16 newword = mTilemapInfo.videoram[offset];
	COMBINE_DATA( &newword );
	SetTilemapVideoram( offset, newword );
}

READ16_HANDLER( namco_tilemapvideoram16_r )
{
	return mTilemapInfo.videoram[offset];
}

static void
SetTilemapControl( int offset, UINT16 newword )
{
	mTilemapInfo.control[offset] = newword;
	if( offset<0x20/2 )
	{
		if( offset == 0x02/2 )
		{
			/* all planes are flipped X+Y from D15 of this word */
			int attrs = (newword & 0x8000)?(TILEMAP_FLIPX|TILEMAP_FLIPY):0;
			int i;
			for( i=0; i<=5; i++ )
			{
				mTilemapInfo.tmap[i]->set_flip(attrs);
			}
		}
	}
	newword &= 0x1ff;
	if( mTilemapInfo.control[0x02/2]&0x8000 )
	{
		newword = -newword;
	}
	switch( offset )
	{
	case 0x02/2:
		mTilemapInfo.tmap[0]->set_scrollx( 0, newword );
		break;
	case 0x06/2:
		mTilemapInfo.tmap[0]->set_scrolly( 0, newword );
		break;
	case 0x0a/2:
		mTilemapInfo.tmap[1]->set_scrollx( 0, newword );
		break;
	case 0x0e/2:
		mTilemapInfo.tmap[1]->set_scrolly( 0, newword );
		break;
	case 0x12/2:
		mTilemapInfo.tmap[2]->set_scrollx( 0, newword );
		break;
	case 0x16/2:
		mTilemapInfo.tmap[2]->set_scrolly( 0, newword );
		break;
	case 0x1a/2:
		mTilemapInfo.tmap[3]->set_scrollx( 0, newword );
		break;
	case 0x1e/2:
		mTilemapInfo.tmap[3]->set_scrolly( 0, newword );
		break;
	}
} /* SetTilemapControl */

WRITE16_HANDLER( namco_tilemapcontrol16_w )
{
	UINT16 newword = mTilemapInfo.control[offset];
	COMBINE_DATA( &newword );
	SetTilemapControl( offset, newword );
}

READ16_HANDLER( namco_tilemapcontrol16_r )
{
	return mTilemapInfo.control[offset];
}

READ32_HANDLER( namco_tilemapvideoram32_r )
{
	offset *= 2;
	return (mTilemapInfo.videoram[offset]<<16)|mTilemapInfo.videoram[offset+1];
}

WRITE32_HANDLER( namco_tilemapvideoram32_w )
{
	UINT32 v;
	offset *=2;
	v = (mTilemapInfo.videoram[offset]<<16)|mTilemapInfo.videoram[offset+1];
	COMBINE_DATA(&v);
	SetTilemapVideoram( offset, v>>16 );
	SetTilemapVideoram( offset+1, v&0xffff );
}

READ32_HANDLER( namco_tilemapcontrol32_r )
{
	offset *= 2;
	return (mTilemapInfo.control[offset]<<16)|mTilemapInfo.control[offset+1];
}

WRITE32_HANDLER( namco_tilemapcontrol32_w )
{
	UINT32 v;
	offset *=2;
	v = (mTilemapInfo.control[offset]<<16)|mTilemapInfo.control[offset+1];
	COMBINE_DATA(&v);
	SetTilemapControl( offset, v>>16 );
	SetTilemapControl( offset+1, v&0xffff );
}

READ32_HANDLER( namco_tilemapcontrol32_le_r )
{
	offset *= 2;
	return (mTilemapInfo.control[offset+1]<<16)|mTilemapInfo.control[offset];
}

WRITE32_HANDLER( namco_tilemapcontrol32_le_w )
{
	UINT32 v;
	offset *=2;
	v = (mTilemapInfo.control[offset+1]<<16)|mTilemapInfo.control[offset];
	COMBINE_DATA(&v);
	SetTilemapControl( offset+1, v>>16 );
	SetTilemapControl( offset, v&0xffff );
}

READ32_HANDLER( namco_tilemapvideoram32_le_r )
{
	offset *= 2;
	return (mTilemapInfo.videoram[offset+1]<<16)|mTilemapInfo.videoram[offset];
}

WRITE32_HANDLER( namco_tilemapvideoram32_le_w )
{
	UINT32 v;
	offset *=2;
	v = (mTilemapInfo.videoram[offset+1]<<16)|mTilemapInfo.videoram[offset];
	COMBINE_DATA(&v);
	SetTilemapVideoram( offset+1, v>>16 );
	SetTilemapVideoram( offset, v&0xffff );
}

/**************************************************************************************/

static void zdrawgfxzoom(
		bitmap_ind16 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	if (!scalex || !scaley) return;
	if (dest_bmp.bpp() == 16)
	{
		if( gfx )
		{
			int shadow_offset = (gfx->machine().config().m_video_attributes&VIDEO_HAS_SHADOWS)?gfx->machine().total_colors():0;
			const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
			const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
			int sprite_screen_height = (scaley*gfx->height+0x8000)>>16;
			int sprite_screen_width = (scalex*gfx->width+0x8000)>>16;
			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width<<16)/sprite_screen_width;
				int dy = (gfx->height<<16)/sprite_screen_height;

				int ex = sx+sprite_screen_width;
				int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;

				if( flipx )
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if( flipy )
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if( sx < clip.min_x)
				{ /* clip left */
					int pixels = clip.min_x-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if( sy < clip.min_y )
				{ /* clip top */
					int pixels = clip.min_y-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				if( ex > clip.max_x+1 )
				{ /* clip right */
					int pixels = ex-clip.max_x-1;
					ex -= pixels;
				}
				if( ey > clip.max_y+1 )
				{ /* clip bottom */
					int pixels = ey-clip.max_y-1;
					ey -= pixels;
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;
					bitmap_ind8 &priority_bitmap = gfx->machine().priority_bitmap;
					if( priority_bitmap.valid() )
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
							UINT16 *dest = &dest_bmp.pix16(y);
							UINT8 *pri = &priority_bitmap.pix8(y);
							int x, x_index = x_index_base;
							if( mPalXOR )
							{
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != 0xff )
									{
										if( pri[x]<=zpos )
										{
											switch( c )
											{
											case 0:
												dest[x] = 0x4000|(dest[x]&0x1fff);
												break;
											case 1:
												dest[x] = 0x6000|(dest[x]&0x1fff);
												break;
											default:
												dest[x] = pal[c];
												break;
											}
											pri[x] = zpos;
										}
									}
									x_index += dx;
								}
								y_index += dy;
							}
							else
							{
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != 0xff )
									{
										if( pri[x]<=zpos )
										{
											if( color == 0xf && c==0xfe && shadow_offset )
											{
												dest[x] |= shadow_offset;
											}
											else
											{
												dest[x] = pal[c];
											}
											pri[x] = zpos;
										}
									}
									x_index += dx;
								}
								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
} /* zdrawgfxzoom */

static void zdrawgfxzoom(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	/* nop */
}

void
namcos2_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control )
{
	int offset = (control & 0x000f) * (128*4);
	int loop;
	if( pri==0 )
	{
		machine.priority_bitmap.fill(0, cliprect );
	}
	for( loop=0; loop < 128; loop++ )
	{
		/****************************************
        * word#0
        *   Sprite Y position           D00-D08
        *   Sprite Size 16/32           D09
        *   Sprite Size Y               D10-D15
        *
        * word#1
        *   Sprite Quadrant             D00-D01
        *   Sprite Number               D02-D12
        *   Sprite ROM Bank select      D13
        *   Sprite flip X               D14
        *   Sprite flip Y               D15
        *
        * word#2
        *   Sprite X position           D00-D10
        *
        * word#3
        *   Sprite priority             D00-D02
        *   Sprite colour index         D04-D07
        *   Sprite Size X               D10-D15
        */
		int word3 = namcos2_sprite_ram[offset+(loop*4)+3];
		if( (word3&0xf)==pri )
		{
			int word0 = namcos2_sprite_ram[offset+(loop*4)+0];
			int word1 = namcos2_sprite_ram[offset+(loop*4)+1];
			int offset4 = namcos2_sprite_ram[offset+(loop*4)+2];

			int sizey=((word0>>10)&0x3f)+1;
			int sizex=(word3>>10)&0x3f;

			if((word0&0x0200)==0) sizex>>=1;

			if((sizey-1) && sizex )
			{
				int color  = (word3>>4)&0x000f;
				int sprn   = (word1>>2)&0x7ff;
				int rgn    = (word1&0x2000)?1:0;
				int ypos   = (0x1ff-(word0&0x01ff))-0x50+0x02;
				int xpos   = (offset4&0x03ff)-0x50+0x07;
				int flipy  = word1&0x8000;
				int flipx  = word1&0x4000;
				int scalex = (sizex<<16)/((word0&0x0200)?0x20:0x10);
				int scaley = (sizey<<16)/((word0&0x0200)?0x20:0x10);
				if(scalex && scaley)
				{
					gfx_element *gfx = machine.gfx[rgn];

					if( (word0&0x0200)==0 )
						gfx_element_set_source_clip(gfx, (word1&0x0001) ? 16 : 0, 16, (word1&0x0002) ? 16 : 0, 16);
					else
						gfx_element_set_source_clip(gfx, 0, 32, 0, 32);

					zdrawgfxzoom(
						bitmap,
						cliprect,
						gfx,
						sprn,
						color,
						flipx,flipy,
						xpos,ypos,
						scalex,scaley,
						loop );
				}
			}
		}
	}
} /* namcos2_draw_sprites */

void
namcos2_draw_sprites_metalhawk(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	/**
     * word#0
     *  xxxxxx---------- ysize
     *  ------x--------- sprite tile size
     *  -------xxxxxxxxx screeny
     *
     * word#1
     *  --x------------- bank
     *  ----xxxxxxxxxxxx tile
     *
     * word#2 (unused)
     *
     * word#3
     *  xxxxxx---------- xsize
     *  ------xxxxxxxxxx screenx
     *
     * word#4 (unused)
     * word#5 (unused)
     *
     * word#6 (orientation)
     *  ---------------x rot90
     *  --------------x- flipx
     *  -------------x-- flipy
     *  ------------x--- tile size
     *
     * word#7
     *  ------------xxxx priority
     *  --------xxxx---- color
     *  x--------------- unknown
     */
	const UINT16 *pSource = namcos2_sprite_ram;
	rectangle rect;
	int loop;
	if( pri==0 )
	{
		machine.priority_bitmap.fill(0, cliprect );
	}
	for( loop=0; loop < 128; loop++ )
	{
		int ypos  = pSource[0];
		int tile  = pSource[1];
		int xpos  = pSource[3];
		int flags = pSource[6];
		int attrs = pSource[7];
		int sizey = ((ypos>>10)&0x3f)+1;
		int sizex = (xpos>>10)&0x3f;
		int sprn  = (tile>>2)&0x7ff;

		if( tile&0x2000 )
		{
			sprn&=0x3ff;
		}
		else
		{
			sprn|=0x400;
		}

		if( (sizey-1) && sizex && (attrs&0xf)==pri )
		{
			int bBigSprite = (flags&8);
			int color = (attrs>>4)&0xf;
			int sx = (xpos&0x03ff)-0x50+0x07;
			int sy = (0x1ff-(ypos&0x01ff))-0x50+0x02;
			int flipx = flags&2;
			int flipy = flags&4;
			int scalex = (sizex<<16)/(bBigSprite?0x20:0x10);
			int scaley = (sizey<<16)/(bBigSprite?0x20:0x10);

			if( (flags&0x01) )
			{ /* swap xy */
				sprn |= 0x800;
			}

			if( bBigSprite )
			{
				if( sizex < 0x20 )
				{
					sx -= (0x20-sizex)/0x8;
				}
				if( sizey < 0x20 )
				{
					sy += (0x20-sizey)/0xC;
				}
			}

			/* Set the clipping rect to mask off the other portion of the sprite */
			rect.set(sx, sx+(sizex-1), sy, sy+(sizey-1));
			rect &= cliprect;

			if( !bBigSprite )
			{
				sizex = 16;
				sizey = 16;
				scalex = 1<<16;
				scaley = 1<<16;

				sx -= (tile&1)?16:0;
				sy -= (tile&2)?16:0;

				rect.set(sx, sx+(sizex-1), sy, sy+(sizey-1));
				rect.min_x += (tile&1)?16:0;
				rect.max_x += (tile&1)?16:0;
				rect.min_y += (tile&2)?16:0;
				rect.max_y += (tile&2)?16:0;
			}
			zdrawgfxzoom(
				bitmap,
				rect,
				machine.gfx[0],
				sprn, color,
				flipx,flipy,
				sx,sy,
				scalex, scaley,
				loop );
		}
		pSource += 8;
	}
} /* namcos2_draw_sprites_metalhawk */

/**************************************************************************************/

static UINT16 mSpritePos[4];
static UINT16 *m_spriteram;

WRITE16_HANDLER( namco_spritepos16_w )
{
	COMBINE_DATA( &mSpritePos[offset] );
}
READ16_HANDLER( namco_spritepos16_r )
{
	return mSpritePos[offset];
}

WRITE32_HANDLER( namco_spritepos32_w )
{
	UINT32 v;
	offset *= 2;
	v = (mSpritePos[offset]<<16)|mSpritePos[offset+1];
	COMBINE_DATA( &v );
	mSpritePos[offset+0] = v>>16;
	mSpritePos[offset+1] = v&0xffff;
}
READ32_HANDLER( namco_spritepos32_r )
{
	offset *= 2;
	return (mSpritePos[offset]<<16)|mSpritePos[offset+1];
}

INLINE UINT8
nth_byte16( const UINT16 *pSource, int which )
{
	UINT16 data = pSource[which/2];
	if( which&1 )
	{
		return data&0xff;
	}
	else
	{
		return data>>8;
	}
} /* nth_byte16 */

/* nth_word32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of 16 bit words.
 */
#ifdef UNUSED_FUNCTION
INLINE UINT16
nth_word32( const UINT32 *pSource, int which )
{
	UINT32 data = pSource[which/2];
	if( which&1 )
	{
		return data&0xffff;
	}
	else
	{
		return data>>16;
	}
} /* nth_word32 */
#endif

/* nth_byte32 is a general-purpose utility function, which allows us to
 * read from 32-bit aligned memory as if it were an array of bytes.
 */
#ifdef UNUSED_FUNCTION
INLINE UINT8
nth_byte32( const UINT32 *pSource, int which )
{
		UINT32 data = pSource[which/4];
		switch( which&3 )
		{
		case 0: return data>>24;
		case 1: return (data>>16)&0xff;
		case 2: return (data>>8)&0xff;
		default: return data&0xff;
		}
} /* nth_byte32 */
#endif

/**************************************************************************************************************/

static int (*mpCodeToTile)( running_machine &machine, int code ); /* sprite banking callback */
static int mGfxC355;	/* gfx bank for sprites */

/**
 * 0x00000 sprite attr (page0)
 * 0x02000 sprite list (page0)
 *
 * 0x02400 window attributes
 * 0x04000 format
 * 0x08000 tile
 * 0x10000 sprite attr (page1)
 * 0x14000 sprite list (page1)
 */
template<class _BitmapClass>
static void
draw_spriteC355(running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, const UINT16 *pSource, int pri, int zpos )
{
	UINT16 *spriteram16 = m_spriteram;
	unsigned screen_height_remaining, screen_width_remaining;
	unsigned source_height_remaining, source_width_remaining;
	int hpos,vpos;
	UINT16 hsize,vsize;
	UINT16 palette;
	UINT16 linkno;
	UINT16 offset;
	UINT16 format;
	int tile_index;
	int num_cols,num_rows;
	int dx,dy;
	int row,col;
	int sx,sy,tile;
	int flipx,flipy;
	UINT32 zoomx, zoomy;
	int tile_screen_width;
	int tile_screen_height;
	const UINT16 *spriteformat16 = &spriteram16[0x4000/2];
	const UINT16 *spritetile16   = &spriteram16[0x8000/2];
	int color;
	const UINT16 *pWinAttr;
	rectangle clip;
	int xscroll, yscroll;

	/**
     * ----xxxx-------- window select
     * --------xxxx---- priority
     * ------------xxxx palette select
     */
	palette = pSource[6];
	if( pri != ((palette>>4)&0xf) )
	{
		return;
	}

	linkno		= pSource[0]; /* LINKNO */
	offset		= pSource[1]; /* OFFSET */
	hpos		= pSource[2]; /* HPOS       0x000..0x7ff (signed) */
	vpos		= pSource[3]; /* VPOS       0x000..0x7ff (signed) */
	hsize		= pSource[4]; /* HSIZE      max 0x3ff pixels */
	vsize		= pSource[5]; /* VSIZE      max 0x3ff pixels */
	/* pSource[6] contains priority/palette */
	/* pSource[7] is used in Lucky & Wild, possibly for sprite-road priority */

	if( linkno*4>=0x4000/2 ) return; /* avoid garbage memory reads */

	xscroll = (INT16)mSpritePos[1];
	yscroll = (INT16)mSpritePos[0];

//  xscroll &= 0x3ff; if( xscroll & 0x200 ) xscroll |= ~0x3ff;
	xscroll &= 0x1ff; if( xscroll & 0x100 ) xscroll |= ~0x1ff;
	yscroll &= 0x1ff; if( yscroll & 0x100 ) yscroll |= ~0x1ff;

	if( bitmap.width() > 384 )
	{ /* Medium Resolution: System21 adjust */
			xscroll = (INT16)mSpritePos[1];
			xscroll &= 0x3ff; if( xscroll & 0x200 ) xscroll |= ~0x3ff;
			if( yscroll<0 )
			{ /* solvalou */
				yscroll += 0x20;
			}
			yscroll += 0x10;
	}
	else
	{
		if ((namcos2_gametype == NAMCOFL_SPEED_RACER) || (namcos2_gametype == NAMCOFL_FINAL_LAP_R))
		{ /* Namco FL: don't adjust and things line up fine */
		}
		else
		{ /* Namco NB1, Namco System 2 */
			xscroll += 0x26;
			yscroll += 0x19;
		}
	}

	hpos -= xscroll;
	vpos -= yscroll;
	pWinAttr = &spriteram16[0x2400/2+((palette>>8)&0xf)*4];
	clip.set(pWinAttr[0] - xscroll, pWinAttr[1] - xscroll, pWinAttr[2] - yscroll, pWinAttr[3] - yscroll);
	clip &= cliprect;
	hpos&=0x7ff; if( hpos&0x400 ) hpos |= ~0x7ff; /* sign extend */
	vpos&=0x7ff; if( vpos&0x400 ) vpos |= ~0x7ff; /* sign extend */

	tile_index		= spriteformat16[linkno*4+0];
	format			= spriteformat16[linkno*4+1];
	dx				= spriteformat16[linkno*4+2];
	dy				= spriteformat16[linkno*4+3];
	num_cols		= (format>>4)&0xf;
	num_rows		= (format)&0xf;

	if( num_cols == 0 ) num_cols = 0x10;
	flipx = (hsize&0x8000)?1:0;
	hsize &= 0x3ff;//0x1ff;
	if( hsize == 0 ) return;
	zoomx = (hsize<<16)/(num_cols*16);
	dx = (dx*zoomx+0x8000)>>16;
	if( flipx )
	{
		hpos += dx;
	}
	else
	{
		hpos -= dx;
	}

	if( num_rows == 0 ) num_rows = 0x10;
	flipy = (vsize&0x8000)?1:0;
	vsize &= 0x3ff;
	if( vsize == 0 ) return;
	zoomy = (vsize<<16)/(num_rows*16);
	dy = (dy*zoomy+0x8000)>>16;
	if( flipy )
	{
		vpos += dy;
	}
	else
	{
		vpos -= dy;
	}

	color = (palette&0xf)^mPalXOR;

	source_height_remaining = num_rows*16;
	screen_height_remaining = vsize;
	sy = vpos;
	for( row=0; row<num_rows; row++ )
	{
		tile_screen_height = 16*screen_height_remaining/source_height_remaining;
		zoomy = (screen_height_remaining<<16)/source_height_remaining;
		if( flipy )
		{
			sy -= tile_screen_height;
		}
		source_width_remaining = num_cols*16;
		screen_width_remaining = hsize;
		sx = hpos;
		for( col=0; col<num_cols; col++ )
		{
			tile_screen_width = 16*screen_width_remaining/source_width_remaining;
			zoomx = (screen_width_remaining<<16)/source_width_remaining;
			if( flipx )
			{
				sx -= tile_screen_width;
			}
			tile = spritetile16[tile_index++];
			if( (tile&0x8000)==0 )
			{
				zdrawgfxzoom(
					bitmap,
					clip,
					machine.gfx[mGfxC355],
					mpCodeToTile(machine, tile) + offset,
					color,
					flipx,flipy,
					sx,sy,
					zoomx, zoomy, zpos );
			}
			if( !flipx )
			{
				sx += tile_screen_width;
			}
			screen_width_remaining -= tile_screen_width;
			source_width_remaining -= 16;
		} /* next col */
		if( !flipy )
		{
			sy += tile_screen_height;
		}
		screen_height_remaining -= tile_screen_height;
		source_height_remaining -= 16;
	} /* next row */
} /* draw_spriteC355 */


static int DefaultCodeToTile( running_machine &machine, int code )
{
	return code;
}

void
namco_obj_init( running_machine &machine, int gfxbank, int palXOR, int (*codeToTile)( running_machine &machine, int code ) )
{
	mGfxC355 = gfxbank;
	mPalXOR = palXOR;
	if( codeToTile )
	{
		mpCodeToTile = codeToTile;
	}
	else
	{
		mpCodeToTile = DefaultCodeToTile;
	}
	m_spriteram = auto_alloc_array(machine, UINT16, 0x20000/2);
	memset( m_spriteram, 0, 0x20000 ); /* needed for Nebulas Ray */
	memset( mSpritePos,0x00,sizeof(mSpritePos) );
} /* namcosC355_init */

template<class _BitmapClass>
static void
DrawObjectList(running_machine &machine,
		_BitmapClass &bitmap,
		const rectangle &cliprect,
		int pri,
		const UINT16 *pSpriteList16,
		const UINT16 *pSpriteTable )
{
	int i;
	/* draw the sprites */
	for( i=0; i<256; i++ )
	{
		UINT16 which = pSpriteList16[i];
		draw_spriteC355(machine, bitmap, cliprect, &pSpriteTable[(which&0xff)*8], pri, i );
		if( which&0x100 ) break;
	}
} /* DrawObjectList */

void
namco_obj_draw(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	if( pri==0 )
	{
		machine.priority_bitmap.fill(0, cliprect );
	}
//  if( offs==0 )
	{ /* boot */
		DrawObjectList(machine, bitmap,cliprect,pri,&m_spriteram[0x02000/2], &m_spriteram[0x00000/2] );
	}
//  else
	{
		DrawObjectList(machine, bitmap,cliprect,pri,&m_spriteram[0x14000/2], &m_spriteram[0x10000/2] );
	}
} /* namco_obj_draw */

void
namco_obj_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri )
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	if( pri==0 )
	{
		machine.priority_bitmap.fill(0, cliprect );
	}
//  if( offs==0 )
	{ /* boot */
		DrawObjectList(machine, bitmap,cliprect,pri,&m_spriteram[0x02000/2], &m_spriteram[0x00000/2] );
	}
//  else
	{
		DrawObjectList(machine, bitmap,cliprect,pri,&m_spriteram[0x14000/2], &m_spriteram[0x10000/2] );
	}
} /* namco_obj_draw */

WRITE16_HANDLER( namco_obj16_w )
{
	COMBINE_DATA( &m_spriteram[offset] );
} /* namco_obj16_w */

READ16_HANDLER( namco_obj16_r )
{
	return m_spriteram[offset];
} /* namco_obj16_r */

WRITE32_HANDLER( namco_obj32_w )
{
	UINT16 *spriteram16 = m_spriteram;
	UINT32 v;
	offset *= 2;
	v = (spriteram16[offset]<<16)|spriteram16[offset+1];
	COMBINE_DATA( &v );
	spriteram16[offset] = v>>16;
	spriteram16[offset+1] = v&0xffff;
} /* namco_obj32_w */

READ32_HANDLER( namco_obj32_r )
{
	UINT16 *spriteram16 = m_spriteram;
	offset *= 2;
	return (spriteram16[offset]<<16)|spriteram16[offset+1];
} /* namco_obj32_r */

WRITE32_HANDLER( namco_obj32_le_w )
{
	UINT16 *spriteram16 = m_spriteram;
	UINT32 v;
	offset *= 2;
	v = (spriteram16[offset+1]<<16)|spriteram16[offset];
	COMBINE_DATA( &v );
	spriteram16[offset+1] = v>>16;
	spriteram16[offset] = v&0xffff;
} /* namco_obj32_w */

READ32_HANDLER( namco_obj32_le_r )
{
	UINT16 *spriteram16 = m_spriteram;
	offset *= 2;
	return (spriteram16[offset+1]<<16)|spriteram16[offset];
} /* namco_obj32_r */

/**************************************************************************************************************/

/**
 * Advanced rotate-zoom chip manages two layers.
 * Each layer uses a designated subset of a master 256x256 tile tilemap (4096x4096 pixels).
 * Each layer has configurable color and tile banking.
 * ROZ attributes may be specified independently for each scanline.
 *
 * Used by:
 *  Namco NB2 - The Outfoxies, Mach Breakers
 *  Namco System 2 - Metal Hawk, Lucky and Wild
 *  Namco System FL - Final Lap R, Speed Racer
 */
#define ROZ_TILEMAP_COUNT 2
static tilemap_t *mRozTilemap[ROZ_TILEMAP_COUNT];
static UINT16 *rozbank16;
static UINT16 *rozvideoram16;
static UINT16 *rozcontrol16;
static int mRozGfxBank;
static const char * mRozMaskRegion;

/**
 * Graphics ROM addressing varies across games.
 */
static void
roz_get_info( running_machine &machine, tile_data &tileinfo, int tile_index, int which)
{
	UINT16 tile = rozvideoram16[tile_index];
	int bank, mangle;

	switch( namcos2_gametype )
	{
	case NAMCONB2_MACH_BREAKERS:
		bank = nth_byte16( &rozbank16[which*8/2], (tile>>11)&0x7 );
		tile = (tile&0x7ff)|(bank*0x800);
		mangle = tile;
		break;

	case NAMCONB2_OUTFOXIES:
		bank = nth_byte16( &rozbank16[which*8/2], (tile>>11)&0x7 );
		tile = (tile&0x7ff)|(bank*0x800);
		mangle = tile&~(0x50);
		if( tile&0x10 ) mangle |= 0x40;
		if( tile&0x40 ) mangle |= 0x10;
		break;

	case NAMCOS2_LUCKY_AND_WILD:
		mangle	= tile&0x01ff;
		tile &= 0x3fff;
		switch( tile>>9 )
		{
		case 0x00: mangle |= 0x1c00; break;
		case 0x01: mangle |= 0x0800; break;
		case 0x02: mangle |= 0x0000; break;

		case 0x08: mangle |= 0x1e00; break;
		case 0x09: mangle |= 0x0a00; break;
		case 0x0a: mangle |= 0x0200; break;

		case 0x10: mangle |= 0x2000; break;
		case 0x11: mangle |= 0x0c00; break;
		case 0x12: mangle |= 0x0400; break;

		case 0x18: mangle |= 0x2200; break;
		case 0x19: mangle |= 0x0e00; break;
		case 0x1a: mangle |= 0x0600; break;
		}
		break;

	case NAMCOS2_METAL_HAWK:
		mangle = tile&0x01ff;
		if( tile&0x1000 ) mangle |= 0x0200;
		if( tile&0x0200 ) mangle |= 0x0400;
		if( tile&0x0400 ) mangle |= 0x0800;
		if( tile&0x0800 ) mangle |= 0x1000;
		tile &= 0x3fff; /* cap mask offset */
		break;

	default:
	case NAMCOFL_SPEED_RACER:
	case NAMCOFL_FINAL_LAP_R:
		mangle = tile;
		tile &= 0x3fff; /* cap mask offset */
		break;
	}
	SET_TILE_INFO( mRozGfxBank,mangle,0/*color*/,0/*flag*/ );
	tileinfo.mask_data = 32*tile + (UINT8 *)machine.region( mRozMaskRegion )->base();
} /* roz_get_info */

static
TILE_GET_INFO( roz_get_info0 )
{
	roz_get_info( machine,tileinfo,tile_index,0 );
} /* roz_get_info0 */

static
TILE_GET_INFO( roz_get_info1 )
{
	roz_get_info( machine,tileinfo,tile_index,1 );
} /* roz_get_info1 */

static
TILEMAP_MAPPER( namco_roz_scan )
{
	if( col>=128 )
	{
		col %= 128;
		row += 256;
	}
	return row*128+col;
} /* namco_roz_scan*/

void
namco_roz_init( running_machine &machine, int gfxbank, const char * maskregion )
{
	int i;
	static const tile_get_info_func roz_info[ROZ_TILEMAP_COUNT] =
	{
		roz_get_info0,
		roz_get_info1
	};

	mRozGfxBank = gfxbank;
	mRozMaskRegion = maskregion;

	rozbank16 = auto_alloc_array(machine, UINT16, 0x10/2);
	rozvideoram16 = auto_alloc_array(machine, UINT16, 0x20000/2);
	rozcontrol16 = auto_alloc_array(machine, UINT16, 0x20/2);

		for( i=0; i<ROZ_TILEMAP_COUNT; i++ )
		{
			mRozTilemap[i] = tilemap_create(machine,
				roz_info[i],
				namco_roz_scan,
				16,16,
				256,256 );
		}
} /* namco_roz_init */

struct RozParam
{
	UINT32 left, top, size;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int color,priority;
};

static void
UnpackRozParam( const UINT16 *pSource, struct RozParam *pRozParam )
{
	const int xoffset = 36, yoffset = 3;

	/**
     * x-------.-------- disable layer
     * ----x---.-------- wrap?
     * ------xx.-------- size
     * --------.xxxx---- priority
     * --------.----xxxx color
     */
	UINT16 temp = pSource[1];
	pRozParam->size     = 512<<((temp&0x0300)>>8);
	if ((namcos2_gametype == NAMCOFL_SPEED_RACER) || (namcos2_gametype == NAMCOFL_FINAL_LAP_R))
	{
		pRozParam->color    = (temp&0x0007)*256;
	}
	else
	{
	pRozParam->color    = (temp&0x000f)*256;
	}
	pRozParam->priority = (temp&0x00f0)>>4;

	temp = pSource[2];
	pRozParam->left = (temp&0x7000)>>3;
	if( temp&0x8000 ) temp |= 0xf000; else temp&=0x0fff; /* sign extend */
	pRozParam->incxx = (INT16)temp;

	temp = pSource[3];
	pRozParam->top = (temp&0x7000)>>3;
	if( temp&0x8000 ) temp |= 0xf000; else temp&=0x0fff; /* sign extend */
	pRozParam->incxy =  (INT16)temp;

	temp = pSource[4];
	if( temp&0x8000 ) temp |= 0xf000; else temp&=0x0fff; /* sign extend */
	pRozParam->incyx =  (INT16)temp;

	temp = pSource[5];
	if( temp&0x8000 ) temp |= 0xf000; else temp&=0x0fff; /* sign extend */
	pRozParam->incyy =  (INT16)temp;

	pRozParam->startx = (INT16)pSource[6];
	pRozParam->starty = (INT16)pSource[7];
	pRozParam->startx <<= 4;
	pRozParam->starty <<= 4;

	pRozParam->startx += xoffset * pRozParam->incxx + yoffset * pRozParam->incyx;
	pRozParam->starty += xoffset * pRozParam->incxy + yoffset * pRozParam->incyy;

	/* normalize */
	pRozParam->startx <<= 8;
	pRozParam->starty <<= 8;
	pRozParam->incxx <<= 8;
	pRozParam->incxy <<= 8;
	pRozParam->incyx <<= 8;
	pRozParam->incyy <<= 8;
} /* UnpackRozParam */

static void
DrawRozHelper(
	bitmap_ind16 &bitmap,
	tilemap_t *tmap,
	const rectangle &clip,
	const struct RozParam *rozInfo )
{

	if( (bitmap.bpp() == 16) &&
	    (namcos2_gametype != NAMCOFL_SPEED_RACER) &&
	    (namcos2_gametype != NAMCOFL_FINAL_LAP_R))
	{
		UINT32 size_mask = rozInfo->size-1;
		bitmap_ind16 &srcbitmap = tmap->pixmap();
		bitmap_ind8 &flagsbitmap = tmap->flagsmap();
		UINT32 startx = rozInfo->startx + clip.min_x * rozInfo->incxx + clip.min_y * rozInfo->incyx;
		UINT32 starty = rozInfo->starty + clip.min_x * rozInfo->incxy + clip.min_y * rozInfo->incyy;
		int sx = clip.min_x;
		int sy = clip.min_y;
		while( sy <= clip.max_y )
		{
			int x = sx;
			UINT32 cx = startx;
			UINT32 cy = starty;
			UINT16 *dest = &bitmap.pix16(sy, sx);
			while( x <= clip.max_x )
			{
				UINT32 xpos = (((cx>>16)&size_mask) + rozInfo->left)&0xfff;
				UINT32 ypos = (((cy>>16)&size_mask) + rozInfo->top)&0xfff;
				if( flagsbitmap.pix8(ypos, xpos)&TILEMAP_PIXEL_LAYER0 )
				{
					*dest = srcbitmap.pix16(ypos, xpos)+rozInfo->color;
				}
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
		tmap->set_palette_offset( rozInfo->color );

		tmap->draw_roz(
			bitmap,
			clip,
			rozInfo->startx, rozInfo->starty,
			rozInfo->incxx, rozInfo->incxy,
			rozInfo->incyx, rozInfo->incyy,
			1,0,0); // wrap, flags, pri
	}
} /* DrawRozHelper */

static void
DrawRozScanline( bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect )
{
	if( line>=cliprect.min_y && line<=cliprect.max_y )
	{
		struct RozParam rozInfo;
		rectangle clip;
		int row = line/8;
		int offs = row*0x100+(line&7)*0x10 + 0xe080;
		UINT16 *pSource = &rozvideoram16[offs/2];
		if( (pSource[1]&0x8000)==0 )
		{
			UnpackRozParam( pSource, &rozInfo );
			if( pri == rozInfo.priority )
			{
				clip.set(0, bitmap.width()-1, line, line);
				clip &= cliprect;

				DrawRozHelper( bitmap, mRozTilemap[which], clip, &rozInfo );
			} /* priority */
		} /* enabled */
	}
} /* DrawRozScanline */

void
namco_roz_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int mode = rozcontrol16[0]; /* 0x8000 or 0x1000 */
	int which, special = 1;

	if ((namcos2_gametype == NAMCOFL_SPEED_RACER) || (namcos2_gametype == NAMCOFL_FINAL_LAP_R))
	{
		special = 0;
	}

	for( which=1; which>=0; which-- )
	{
		const UINT16 *pSource = &rozcontrol16[which*8];
		UINT16 attrs = pSource[1];
		if( (attrs&0x8000)==0 )
		{ /* layer is enabled */
			if( which==special && mode==0x8000 )
			{ /* second ROZ layer is configured to use per-scanline registers */
				int line;
				for( line=0; line<224; line++ )
				{
					DrawRozScanline( bitmap, line, which, pri, cliprect/*, tmap*/ );
				}
			}
			else
			{
				struct RozParam rozInfo;
				UnpackRozParam( pSource, &rozInfo );
				if( rozInfo.priority == pri )
				{
					DrawRozHelper( bitmap, mRozTilemap[which], cliprect, &rozInfo );
				} /* roz_pri==pri */
			}
		} /* enable */
	} /* which */
} /* namco_roz_draw */

READ16_HANDLER( namco_rozcontrol16_r )
{
	return rozcontrol16[offset];
} /* namco_rozcontrol16_r */

WRITE16_HANDLER( namco_rozcontrol16_w )
{
	COMBINE_DATA( &rozcontrol16[offset] );
} /* namco_rozcontrol16_w */

#ifdef UNUSED_FUNCTION
READ16_HANDLER( namco_rozbank16_r )
{
	return rozbank16[offset];
} /* namco_rozbank16_r */

WRITE16_HANDLER( namco_rozbank16_w )
{
	UINT16 old_data = rozbank16[offset];
	COMBINE_DATA( &rozbank16[offset] );
	if( rozbank16[offset]!=old_data )
	{
		int i;
		for( i=0; i<ROZ_TILEMAP_COUNT; i++ )
		{
			mRozTilemap[i]->mark_all_dirty();
		}
	}
} /* namco_rozbank16_w */
#endif

static void
writerozvideo( int offset, UINT16 data )
{
	int i;
	rozvideoram16[offset] = data;
	for( i=0; i<ROZ_TILEMAP_COUNT; i++ )
	{
		mRozTilemap[i]->mark_tile_dirty( offset );
	}
} /* writerozvideo */

READ16_HANDLER( namco_rozvideoram16_r )
{
	return rozvideoram16[offset];
} /* namco_rozvideoram16_r */

WRITE16_HANDLER( namco_rozvideoram16_w )
{
	UINT16 v = rozvideoram16[offset];
	COMBINE_DATA( &v );
	writerozvideo( offset, v );
} /* namco_rozvideoram16_w */

READ32_HANDLER( namco_rozcontrol32_r )
{
	offset *= 2;
	return (rozcontrol16[offset]<<16)|rozcontrol16[offset+1];
} /* namco_rozcontrol32_r */

WRITE32_HANDLER( namco_rozcontrol32_w )
{
	UINT32 v;
	offset *=2;
	v = (rozcontrol16[offset]<<16)|rozcontrol16[offset+1];
	COMBINE_DATA(&v);
	rozcontrol16[offset] = v>>16;
	rozcontrol16[offset+1] = v&0xffff;
} /* namco_rozcontrol32_w */

READ32_HANDLER( namco_rozcontrol32_le_r )
{
	offset *= 2;
	return (rozcontrol16[offset]<<16)|rozcontrol16[offset+1];
} /* namco_rozcontrol32_le_r */

WRITE32_HANDLER( namco_rozcontrol32_le_w )
{
	UINT32 v;
	offset *=2;
	v = (rozcontrol16[offset+1]<<16)|rozcontrol16[offset];
	COMBINE_DATA(&v);
	rozcontrol16[offset+1] = v>>16;
	rozcontrol16[offset] = v&0xffff;
} /* namco_rozcontrol32_le_w */

READ32_HANDLER( namco_rozbank32_r )
{
	offset *= 2;
	return (rozbank16[offset]<<16)|rozbank16[offset+1];
} /* namco_rozbank32_r */

WRITE32_HANDLER( namco_rozbank32_w )
{
	UINT32 v;
	offset *=2;
	v = (rozbank16[offset]<<16)|rozbank16[offset+1];
	COMBINE_DATA(&v);
	rozbank16[offset] = v>>16;
	rozbank16[offset+1] = v&0xffff;
} /* namco_rozbank32_w */

READ32_HANDLER( namco_rozvideoram32_r )
{
	offset *= 2;
	return (rozvideoram16[offset]<<16)|rozvideoram16[offset+1];
} /* namco_rozvideoram32_r */

WRITE32_HANDLER( namco_rozvideoram32_w )
{
	UINT32 v;
	offset *= 2;
	v = (rozvideoram16[offset]<<16)|rozvideoram16[offset+1];
	COMBINE_DATA( &v );
	writerozvideo(offset,v>>16);
	writerozvideo(offset+1,v&0xffff);
} /* namco_rozvideoram32_w */

READ32_HANDLER( namco_rozvideoram32_le_r )
{
	offset *= 2;
	return (rozvideoram16[offset+1]<<16)|rozvideoram16[offset];
} /* namco_rozvideoram32_le_r */

WRITE32_HANDLER( namco_rozvideoram32_le_w )
{
	UINT32 v;
	offset *= 2;
	v = (rozvideoram16[offset+1]<<16)|rozvideoram16[offset];
	COMBINE_DATA( &v );
	writerozvideo(offset+1,v>>16);
	writerozvideo(offset,v&0xffff);
} /* namco_rozvideoram32_le_w */

/**************************************************************************************************************/
/*
    Land Line Buffer
    Land Generator
        0xf,0x7,0xe,0x6,0xd,0x5,0xc,0x4,
        0xb,0x3,0xa,0x2,0x9,0x1,0x8,0x0

*/

/* Preliminary!  The road circuitry is identical for all the driving games.
 *
 * There are several chunks of RAM
 *
 *  Road Tilemap:
 *      0x00000..0x0ffff    64x512 tilemap
 *
 *  Road Tiles:
 *      0x10000..0x1f9ff    16x16x2bpp tiles
 *
 *
 *  Line Attributes:
 *
 *      0x1fa00..0x1fbdf    xxx- ---- ---- ----     priority
 *                          ---- xxxx xxxx xxxx     xscroll
 *
 *      0x1fbfe             horizontal adjust?
 *                          0x0017
 *                          0x0018 (Final Lap3)
 *
 *      0x1fc00..0x1fddf    selects line in source bitmap
 *      0x1fdfe             yscroll
 *
 *      0x1fe00..0x1ffdf    ---- --xx xxxx xxxx     zoomx
 *      0x1fffd             always 0xffff 0xffff?
 */
static UINT16 *mpRoadRAM; /* at 0x880000 in Final Lap; at 0xa00000 in Lucky&Wild */
static int mRoadGfxBank;
static tilemap_t *mpRoadTilemap;
static pen_t mRoadTransparentColor;
static int mbRoadNeedTransparent;

#define ROAD_COLS			64
#define ROAD_ROWS			512
#define ROAD_TILE_SIZE		16
#define ROAD_TILEMAP_WIDTH	(ROAD_TILE_SIZE*ROAD_COLS)
#define ROAD_TILEMAP_HEIGHT (ROAD_TILE_SIZE*ROAD_ROWS)

#define ROAD_TILE_COUNT_MAX	(0xfa00/0x40) /* 0x3e8 */
#define WORDS_PER_ROAD_TILE (0x40/2)

static const gfx_layout RoadTileLayout =
{
	ROAD_TILE_SIZE,	ROAD_TILE_SIZE,
	ROAD_TILE_COUNT_MAX,
	2,
	{ NATIVE_ENDIAN_VALUE_LE_BE(8,0), NATIVE_ENDIAN_VALUE_LE_BE(0,8) },
	{/* x offset */
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17
	},
	{/* y offset */
		0x000,0x020,0x040,0x060,0x080,0x0a0,0x0c0,0x0e0,
		0x100,0x120,0x140,0x160,0x180,0x1a0,0x1c0,0x1e0
	},
	0x200, /* offset to next tile */
};

static TILE_GET_INFO( get_road_info )
{
	UINT16 data = mpRoadRAM[tile_index];
	/* ------xx xxxxxxxx tile number
     * xxxxxx-- -------- palette select
     */
	int tile = (data&0x3ff);
	int color = (data>>10);

	SET_TILE_INFO( mRoadGfxBank, tile, color , 0 );
} /* get_road_info */

READ16_HANDLER( namco_road16_r )
{
	return mpRoadRAM[offset];
}

WRITE16_HANDLER( namco_road16_w )
{
	COMBINE_DATA( &mpRoadRAM[offset] );
	if( offset<0x10000/2 )
	{
		mpRoadTilemap->mark_tile_dirty( offset );
	}
	else
	{
		offset -= 0x10000/2;
		gfx_element_mark_dirty(space->machine().gfx[mRoadGfxBank], offset/WORDS_PER_ROAD_TILE);
	}
}

void
namco_road_init(running_machine &machine, int gfxbank )
{
	gfx_element *pGfx;

	mbRoadNeedTransparent = 0;
	mRoadGfxBank = gfxbank;

	mpRoadRAM = auto_alloc_array(machine, UINT16, 0x20000/2);

	pGfx = gfx_element_alloc( machine, &RoadTileLayout, 0x10000+(UINT8 *)mpRoadRAM, 0x3f, 0xf00);

	machine.gfx[gfxbank] = pGfx;
	mpRoadTilemap = tilemap_create(machine,
		get_road_info,tilemap_scan_rows,
		ROAD_TILE_SIZE,ROAD_TILE_SIZE,
		ROAD_COLS,ROAD_ROWS);

	state_save_register_global_pointer(machine, mpRoadRAM,   0x20000 / 2);
} /* namco_road_init */

void
namco_road_set_transparent_color(pen_t pen)
{
	mbRoadNeedTransparent = 1;
	mRoadTransparentColor = pen;
}

void
namco_road_draw(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	const UINT8 *clut = (const UINT8 *)machine.region("user3")->base();
	unsigned yscroll;
	int i;

	bitmap_ind16 &SourceBitmap = mpRoadTilemap->pixmap();
	yscroll = mpRoadRAM[0x1fdfe/2];

	for( i=cliprect.min_y; i<=cliprect.max_y; i++ )
	{
		int screenx	= mpRoadRAM[0x1fa00/2+i+15];
		if( pri == ((screenx&0xf000)>>12) )
		{
			unsigned zoomx	= mpRoadRAM[0x1fe00/2+i+15]&0x3ff;
			if( zoomx )
			{
				unsigned sourcey = mpRoadRAM[0x1fc00/2+i+15]+yscroll;
				const UINT16 *pSourceGfx = &SourceBitmap.pix16(sourcey&(ROAD_TILEMAP_HEIGHT-1));
				unsigned dsourcex = (ROAD_TILEMAP_WIDTH<<16)/zoomx;
				if( dsourcex )
				{
					UINT16 *pDest = &bitmap.pix16(i);
					unsigned sourcex = 0;
					int numpixels = (44*ROAD_TILE_SIZE<<16)/dsourcex;
					int clipPixels;

					screenx &= 0x0fff; /* mask off priority bits */
					if( screenx&0x0800 )
					{
						/* sign extend */
						screenx |= ~0x7ff;
					}

					/* adjust the horizontal placement */
					screenx -= 64; /*needs adjustment to left*/

					clipPixels = cliprect.min_x - screenx;
					if( clipPixels>0 )
					{ /* crop left */
						numpixels -= clipPixels;
						sourcex += dsourcex*clipPixels;
						screenx = cliprect.min_x;
					}

					clipPixels = (screenx+numpixels) - (cliprect.max_x+1);
					if( clipPixels>0 )
					{ /* crop right */
						numpixels -= clipPixels;
					}

					/* TBA: work out palette mapping for Final Lap, Suzuka */

					/* BUT: support transparent color for Thunder Ceptor */
					if( mbRoadNeedTransparent )
					{
						while( numpixels-- > 0 )
						{
							int pen = pSourceGfx[sourcex>>16];

							if(colortable_entry_get_value(machine.colortable, pen) != mRoadTransparentColor)
							{
								if( clut )
								{
									pen = (pen&~0xff)|clut[pen&0xff];
								}
								pDest[screenx] = pen;
							}
							screenx++;
							sourcex += dsourcex;
						}
					}
					else
					{
						while( numpixels-- > 0 )
						{
							int pen = pSourceGfx[sourcex>>16];
							if( clut )
							{
								pen = (pen&~0xff)|clut[pen&0xff];
							}
							pDest[screenx++] = pen;
							sourcex += dsourcex;
						}
					}
				} /* dsourcex!=0 */
			} /* zoomx!=0 */
		} /* priority */
	} /* next scanline */
} /* namco_road_draw */
