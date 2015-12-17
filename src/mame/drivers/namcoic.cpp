// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "includes/namcos2.h" /* for game-specific hacks */
#include "includes/namcoic.h"


/**************************************************************************************/
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
	std::unique_ptr<UINT16[]> videoram;
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

inline void namcos2_shared_state::namcoic_get_tile_info(tile_data &tileinfo,int tile_index,UINT16 *vram)
{
	int tile, mask;
	mTilemapInfo.cb( machine(), vram[tile_index], &tile, &mask );
	tileinfo.mask_data = mTilemapInfo.maskBaseAddr+mask*8;
	SET_TILE_INFO_MEMBER(mTilemapInfo.gfxbank,tile,0,0);
} /* get_tile_info */

TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info0 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x0000]); }
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info1 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x1000]); }
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info2 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x2000]); }
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info3 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x3000]); }
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info4 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x4008]); }
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info5 ) { namcoic_get_tile_info(tileinfo,tile_index,&mTilemapInfo.videoram[0x4408]); }

void namcos2_shared_state::namco_tilemap_init( int gfxbank, void *maskBaseAddr,
	void (*cb)( running_machine &machine, UINT16 code, int *gfx, int *mask) )
{
	int i;
	mTilemapInfo.gfxbank = gfxbank;
	mTilemapInfo.maskBaseAddr = (UINT8 *)maskBaseAddr;
	mTilemapInfo.cb = cb;
	mTilemapInfo.videoram = std::make_unique<UINT16[]>( 0x10000 );

		/* four scrolling tilemaps */
		mTilemapInfo.tmap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info0),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		mTilemapInfo.tmap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info1),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		mTilemapInfo.tmap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info2),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		mTilemapInfo.tmap[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info3),this),TILEMAP_SCAN_ROWS,8,8,64,64);

		/* two non-scrolling tilemaps */
		mTilemapInfo.tmap[4] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info4),this),TILEMAP_SCAN_ROWS,8,8,36,28);
		mTilemapInfo.tmap[5] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info5),this),TILEMAP_SCAN_ROWS,8,8,36,28);

		/* define offsets for scrolling */
		for( i=0; i<4; i++ )
		{
			static const int adj[4] = { 4,2,1,0 };
			int dx = 44+adj[i];
			mTilemapInfo.tmap[i]->set_scrolldx( -dx, 288+dx );
			mTilemapInfo.tmap[i]->set_scrolldy( -24, 224+24 );
		}
} /* namco_tilemap_init */

void
namco_tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int i;
	for( i=0; i<6; i++ )
	{
		/* note: priority is only in range 0..7, but Point Blank uses 0xf to hide a layer */
		if( (mTilemapInfo.control[0x20/2+i]&0xf) == pri )
		{
			int color = mTilemapInfo.control[0x30/2+i] & 0x07;
			mTilemapInfo.tmap[i]->set_palette_offset( color*256 );
			mTilemapInfo.tmap[i]->draw(screen, bitmap,cliprect,0,0);
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

WRITE16_MEMBER( namcos2_shared_state::c123_tilemap_videoram_w )
{
	UINT16 newword = mTilemapInfo.videoram[offset];
	COMBINE_DATA( &newword );
	SetTilemapVideoram( offset, newword );
}

READ16_MEMBER( namcos2_shared_state::c123_tilemap_videoram_r )
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

WRITE16_MEMBER( namcos2_shared_state::c123_tilemap_control_w )
{
	UINT16 newword = mTilemapInfo.control[offset];
	COMBINE_DATA( &newword );
	SetTilemapControl( offset, newword );
}

READ16_MEMBER( namcos2_shared_state::c123_tilemap_control_r )
{
	return mTilemapInfo.control[offset];
}


/**************************************************************************************/

void namcos2_shared_state::zdrawgfxzoom(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	if (!scalex || !scaley) return;
	if (dest_bmp.bpp() == 16)
	{
		if( gfx )
		{
			int shadow_offset = (m_palette->shadows_enabled())?m_palette->entries():0;
			const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const UINT8 *source_base = gfx->get_data(code % gfx->elements());
			int sprite_screen_height = (scaley*gfx->height()+0x8000)>>16;
			int sprite_screen_width = (scalex*gfx->width()+0x8000)>>16;
			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width()<<16)/sprite_screen_width;
				int dy = (gfx->height()<<16)/sprite_screen_height;

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
					bitmap_ind8 &priority_bitmap = screen.priority();
					if( priority_bitmap.valid() )
					{
						for( y=sy; y<ey; y++ )
						{
							const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
							UINT16 *dest = &dest_bmp.pix16(y);
							UINT8 *pri = &priority_bitmap.pix8(y);
							int x, x_index = x_index_base;
							if( m_c355_obj_palxor )
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

void namcos2_shared_state::zdrawgfxzoom(
		screen_device &screen,
		bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	/* nop */
}

void
namcos2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control )
{
	int offset = (control & 0x000f) * (128*4);
	int loop;
	if( pri==0 )
	{
		screen.priority().fill(0, cliprect );
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
		int word3 = m_spriteram[offset+(loop*4)+3];
		if( (word3&0xf)==pri )
		{
			int word0 = m_spriteram[offset+(loop*4)+0];
			int word1 = m_spriteram[offset+(loop*4)+1];
			int offset4 = m_spriteram[offset+(loop*4)+2];

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
					gfx_element *gfx = m_gfxdecode->gfx(rgn);

					if( (word0&0x0200)==0 )
						gfx->set_source_clip((word1&0x0001) ? 16 : 0, 16, (word1&0x0002) ? 16 : 0, 16);
					else
						gfx->set_source_clip(0, 32, 0, 32);

					zdrawgfxzoom(
						screen,
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

void namcos2_state::draw_sprites_metalhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
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
	const UINT16 *pSource = m_spriteram;
	rectangle rect;
	int loop;
	if( pri==0 )
	{
		screen.priority().fill(0, cliprect );
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
				screen,
				bitmap,
				rect,
				m_gfxdecode->gfx(0),
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

WRITE16_MEMBER( namcos2_shared_state::c355_obj_position_w )
{
	COMBINE_DATA(&m_c355_obj_position[offset]);
}
READ16_MEMBER( namcos2_shared_state::c355_obj_position_r )
{
	return m_c355_obj_position[offset];
}

static inline UINT8
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
static inline UINT16
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
static inline UINT8
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
void namcos2_shared_state::c355_obj_draw_sprite(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, const UINT16 *pSource, int pri, int zpos )
{
	UINT16 *spriteram16 = m_c355_obj_ram;
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

	linkno      = pSource[0]; /* LINKNO */
	offset      = pSource[1]; /* OFFSET */
	hpos        = pSource[2]; /* HPOS       0x000..0x7ff (signed) */
	vpos        = pSource[3]; /* VPOS       0x000..0x7ff (signed) */
	hsize       = pSource[4]; /* HSIZE      max 0x3ff pixels */
	vsize       = pSource[5]; /* VSIZE      max 0x3ff pixels */
	/* pSource[6] contains priority/palette */
	/* pSource[7] is used in Lucky & Wild, possibly for sprite-road priority */

	if( linkno*4>=0x4000/2 ) return; /* avoid garbage memory reads */

	xscroll = (INT16)m_c355_obj_position[1];
	yscroll = (INT16)m_c355_obj_position[0];

//  xscroll &= 0x3ff; if( xscroll & 0x200 ) xscroll |= ~0x3ff;
	xscroll &= 0x1ff; if( xscroll & 0x100 ) xscroll |= ~0x1ff;
	yscroll &= 0x1ff; if( yscroll & 0x100 ) yscroll |= ~0x1ff;

	if( bitmap.width() > 384 )
	{ /* Medium Resolution: System21 adjust */
			xscroll = (INT16)m_c355_obj_position[1];
			xscroll &= 0x3ff; if( xscroll & 0x200 ) xscroll |= ~0x3ff;
			if( yscroll<0 )
			{ /* solvalou */
				yscroll += 0x20;
			}
			yscroll += 0x10;
	}
	else
	{
		if ((m_gametype == NAMCOFL_SPEED_RACER) || (m_gametype == NAMCOFL_FINAL_LAP_R))
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

	tile_index      = spriteformat16[linkno*4+0];
	format          = spriteformat16[linkno*4+1];
	dx              = spriteformat16[linkno*4+2];
	dy              = spriteformat16[linkno*4+3];
	num_cols        = (format>>4)&0xf;
	num_rows        = (format)&0xf;

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

	color = (palette&0xf)^m_c355_obj_palxor;

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
					screen,
					bitmap,
					clip,
					m_gfxdecode->gfx(m_c355_obj_gfxbank),
					m_c355_obj_code2tile(tile) + offset,
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
}


int namcos2_shared_state::c355_obj_default_code2tile(int code)
{
	return code;
}

void namcos2_shared_state::c355_obj_init(int gfxbank, int pal_xor, c355_obj_code2tile_delegate code2tile)
{
	m_c355_obj_gfxbank = gfxbank;
	m_c355_obj_palxor = pal_xor;
	if (!code2tile.isnull())
		m_c355_obj_code2tile = code2tile;
	else
		m_c355_obj_code2tile = c355_obj_code2tile_delegate(FUNC(namcos2_shared_state::c355_obj_default_code2tile), this);

	memset(m_c355_obj_ram, 0, sizeof(m_c355_obj_ram)); // needed for Nebulas Ray
	memset(m_c355_obj_position, 0, sizeof(m_c355_obj_position));
}

template<class _BitmapClass>
void namcos2_shared_state::c355_obj_draw_list(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, const UINT16 *pSpriteList16, const UINT16 *pSpriteTable)
{
	int i;
	/* draw the sprites */
	for( i=0; i<256; i++ )
	{
		UINT16 which = pSpriteList16[i];
		c355_obj_draw_sprite(screen, bitmap, cliprect, &pSpriteTable[(which&0xff)*8], pri, i );
		if( which&0x100 ) break;
	}
}

void namcos2_shared_state::c355_obj_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	if (pri == 0)
		screen.priority().fill(0, cliprect);

//  if (offs == 0)  // boot
		c355_obj_draw_list(screen, bitmap, cliprect, pri, &m_c355_obj_ram[0x02000/2], &m_c355_obj_ram[0x00000/2]);
//  else
		c355_obj_draw_list(screen, bitmap, cliprect, pri, &m_c355_obj_ram[0x14000/2], &m_c355_obj_ram[0x10000/2]);
}

void namcos2_shared_state::c355_obj_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	if (pri == 0)
		screen.priority().fill(0, cliprect);

//  if (offs == 0)  // boot
		c355_obj_draw_list(screen, bitmap, cliprect, pri, &m_c355_obj_ram[0x02000/2], &m_c355_obj_ram[0x00000/2]);
//  else
		c355_obj_draw_list(screen, bitmap, cliprect, pri, &m_c355_obj_ram[0x14000/2], &m_c355_obj_ram[0x10000/2]);
}

WRITE16_MEMBER( namcos2_shared_state::c355_obj_ram_w )
{
	COMBINE_DATA(&m_c355_obj_ram[offset]);
}

READ16_MEMBER( namcos2_shared_state::c355_obj_ram_r )
{
	return m_c355_obj_ram[offset];
}

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

/**
 * Graphics ROM addressing varies across games.
 */
void namcos2_shared_state::c169_roz_get_info(tile_data &tileinfo, int tile_index, int which)
{
	UINT16 tile = m_c169_roz_videoram[tile_index];
	int bank, mangle;

	switch (m_gametype)
	{
		case NAMCONB2_MACH_BREAKERS:
			bank = nth_byte16(&m_c169_roz_bank[which * 8 / 2], (tile >> 11) & 0x7);
			tile = (tile & 0x7ff) | (bank * 0x800);
			mangle = tile;
			break;

		case NAMCONB2_OUTFOXIES:
			bank = nth_byte16(&m_c169_roz_bank[which * 8 / 2], (tile >> 11) & 0x7);
			tile = (tile & 0x7ff) | (bank * 0x800);
			mangle = tile & ~0x50;
			if (tile & 0x10) mangle |= 0x40;
			if (tile & 0x40) mangle |= 0x10;
			break;

		case NAMCOS2_LUCKY_AND_WILD:
			mangle = tile & 0x01ff;
			tile &= 0x3fff;
			switch (tile >> 9)
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
			mangle = tile & 0x01ff;
			if (tile & 0x1000) mangle |= 0x0200;
			if (tile & 0x0200) mangle |= 0x0400;
			if (tile & 0x0400) mangle |= 0x0800;
			if (tile & 0x0800) mangle |= 0x1000;
			tile &= 0x3fff; // cap mask offset
			break;

		default:
		case NAMCOFL_SPEED_RACER:
		case NAMCOFL_FINAL_LAP_R:
			mangle = tile;
			tile &= 0x3fff; // cap mask offset
			break;
	}
	SET_TILE_INFO_MEMBER(m_c169_roz_gfxbank, mangle, 0/*color*/, 0/*flag*/);
	tileinfo.mask_data = m_c169_roz_mask + 32*tile;
}

TILE_GET_INFO_MEMBER( namcos2_shared_state::c169_roz_get_info0 )
{
	c169_roz_get_info(tileinfo, tile_index, 0);
}

TILE_GET_INFO_MEMBER( namcos2_shared_state::c169_roz_get_info1 )
{
	c169_roz_get_info(tileinfo, tile_index, 1);
}

TILEMAP_MAPPER_MEMBER( namcos2_shared_state::c169_roz_mapper )
{
	if (col >= 128)
	{
		col %= 128;
		row += 256;
	}
	return row * 128 + col;
}

void namcos2_shared_state::c169_roz_init(int gfxbank, const char *maskregion)
{
	m_c169_roz_gfxbank = gfxbank;
	m_c169_roz_mask = memregion(maskregion)->base();

	m_c169_roz_tilemap[0] = &machine().tilemap().create(m_gfxdecode,
		tilemap_get_info_delegate(FUNC(namcos2_shared_state::c169_roz_get_info0), this),
		tilemap_mapper_delegate(FUNC(namcos2_shared_state::c169_roz_mapper), this),
		16,16,
		256,256);

	m_c169_roz_tilemap[1] = &machine().tilemap().create(m_gfxdecode,
		tilemap_get_info_delegate(FUNC(namcos2_shared_state::c169_roz_get_info1), this),
		tilemap_mapper_delegate(FUNC(namcos2_shared_state::c169_roz_mapper), this),
		16,16,
		256,256);

}

void namcos2_shared_state::c169_roz_unpack_params(const UINT16 *source, roz_parameters &params)
{
	const int xoffset = 36, yoffset = 3;

	/**
	 * x-------.-------- disable layer
	 * ----x---.-------- wrap?
	 * ------xx.-------- size
	 * --------.xxxx---- priority
	 * --------.----xxxx color
	 */

	UINT16 temp = source[1];
	params.size = 512 << ((temp & 0x0300) >> 8);
	if (m_gametype == NAMCOFL_SPEED_RACER || m_gametype == NAMCOFL_FINAL_LAP_R)
		params.color = (temp & 0x0007) * 256;
	else
		params.color = (temp & 0x000f) * 256;
	params.priority = (temp & 0x00f0) >> 4;

	temp = source[2];
	params.left = (temp & 0x7000) >> 3;
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incxx = INT16(temp);

	temp = source[3];
	params.top = (temp&0x7000)>>3;
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incxy = INT16(temp);

	temp = source[4];
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incyx = INT16(temp);

	temp = source[5];
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incyy = INT16(temp);

	params.startx = INT16(source[6]);
	params.starty = INT16(source[7]);
	params.startx <<= 4;
	params.starty <<= 4;

	params.startx += xoffset * params.incxx + yoffset * params.incyx;
	params.starty += xoffset * params.incxy + yoffset * params.incyy;

	// normalize
	params.startx <<= 8;
	params.starty <<= 8;
	params.incxx <<= 8;
	params.incxy <<= 8;
	params.incyx <<= 8;
	params.incyy <<= 8;
}

void namcos2_shared_state::c169_roz_draw_helper(screen_device &screen, bitmap_ind16 &bitmap, tilemap_t &tmap, const rectangle &clip, const roz_parameters &params)
{
	if (m_gametype != NAMCOFL_SPEED_RACER && m_gametype != NAMCOFL_FINAL_LAP_R)
	{
		UINT32 size_mask = params.size - 1;
		bitmap_ind16 &srcbitmap = tmap.pixmap();
		bitmap_ind8 &flagsbitmap = tmap.flagsmap();
		UINT32 startx = params.startx + clip.min_x * params.incxx + clip.min_y * params.incyx;
		UINT32 starty = params.starty + clip.min_x * params.incxy + clip.min_y * params.incyy;
		int sx = clip.min_x;
		int sy = clip.min_y;
		while (sy <= clip.max_y)
		{
			int x = sx;
			UINT32 cx = startx;
			UINT32 cy = starty;
			UINT16 *dest = &bitmap.pix(sy, sx);
			while (x <= clip.max_x)
			{
				UINT32 xpos = (((cx >> 16) & size_mask) + params.left) & 0xfff;
				UINT32 ypos = (((cy >> 16) & size_mask) + params.top) & 0xfff;
				if (flagsbitmap.pix(ypos, xpos) & TILEMAP_PIXEL_LAYER0)
					*dest = srcbitmap.pix(ypos, xpos) + params.color;
				cx += params.incxx;
				cy += params.incxy;
				x++;
				dest++;
			}
			startx += params.incyx;
			starty += params.incyy;
			sy++;
		}
	}
	else
	{
		tmap.set_palette_offset(params.color);
		tmap.draw_roz(
			screen,
			bitmap,
			clip,
			params.startx, params.starty,
			params.incxx, params.incxy,
			params.incyx, params.incyy,
			1,0,0); // wrap, flags, pri
	}
}

void namcos2_shared_state::c169_roz_draw_scanline(screen_device &screen, bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect)
{
	if (line >= cliprect.min_y && line <= cliprect.max_y)
	{
		int row = line / 8;
		int offs = row * 0x100 + (line & 7) * 0x10 + 0xe080;
		UINT16 *source = &m_c169_roz_videoram[offs / 2];

		// if enabled
		if ((source[1] & 0x8000) == 0)
		{
			roz_parameters params;
			c169_roz_unpack_params(source, params);

			// check priority
			if (pri == params.priority)
			{
				rectangle clip(0, bitmap.width() - 1, line, line);
				clip &= cliprect;
				c169_roz_draw_helper(screen, bitmap, *m_c169_roz_tilemap[which], clip, params);
			}
		}
	}
}

void namcos2_shared_state::c169_roz_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	int special = (m_gametype == NAMCOFL_SPEED_RACER || m_gametype == NAMCOFL_FINAL_LAP_R) ? 0 : 1;
	int mode = m_c169_roz_control[0]; // 0x8000 or 0x1000

	for (int which = 1; which >= 0; which--)
	{
		const UINT16 *source = &m_c169_roz_control[which * 8];
		UINT16 attrs = source[1];

		// if enabled
		if ((attrs & 0x8000) == 0)
		{
			// second ROZ layer is configured to use per-scanline registers
			if (which == special && mode == 0x8000)
			{
				for (int line = 0; line < 224; line++)
					c169_roz_draw_scanline(screen, bitmap, line, which, pri, cliprect);
			}
			else
			{
				roz_parameters params;
				c169_roz_unpack_params(source, params);
				if (params.priority == pri)
					c169_roz_draw_helper(screen, bitmap, *m_c169_roz_tilemap[which], cliprect, params);
			}
		}
	}
}

READ16_MEMBER( namcos2_shared_state::c169_roz_control_r )
{
	return m_c169_roz_control[offset];
}

WRITE16_MEMBER( namcos2_shared_state::c169_roz_control_w )
{
	COMBINE_DATA(&m_c169_roz_control[offset]);
}

READ16_MEMBER( namcos2_shared_state::c169_roz_bank_r )
{
	return m_c169_roz_bank[offset];
}

WRITE16_MEMBER( namcos2_shared_state::c169_roz_bank_w )
{
	UINT16 old_data = m_c169_roz_bank[offset];
	COMBINE_DATA(&m_c169_roz_bank[offset]);
	if (m_c169_roz_bank[offset] != old_data)
		for (auto & elem : m_c169_roz_tilemap)
			elem->mark_all_dirty();
}

READ16_MEMBER( namcos2_shared_state::c169_roz_videoram_r )
{
	return m_c169_roz_videoram[offset];
}

WRITE16_MEMBER( namcos2_shared_state::c169_roz_videoram_w )
{
	COMBINE_DATA(&m_c169_roz_videoram[offset]);
	for (auto & elem : m_c169_roz_tilemap)
		elem->mark_tile_dirty(offset);
}
