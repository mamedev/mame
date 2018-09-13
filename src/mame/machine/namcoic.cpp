// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "machine/namcoic.h"

#include "includes/namcos2.h" /* for game-specific hacks */


/**************************************************************************************/

void namcos2_shared_state::c123_tilemap_invalidate(void)
{
	for( int i = 0; i < 6; i++ )
	{
		m_c123_TilemapInfo.tmap[i]->mark_all_dirty();
	}
} /* namco_tilemap_invalidate */

template<int Offset>
TILE_GET_INFO_MEMBER( namcos2_shared_state::get_tile_info )
{
	const uint16_t *vram = &m_c123_TilemapInfo.videoram[Offset];
	int tile, mask;
	m_c123_TilemapInfo.cb( vram[tile_index], &tile, &mask );
	tileinfo.mask_data = m_c123_TilemapInfo.maskBaseAddr+mask*8;
	SET_TILE_INFO_MEMBER(m_c123_TilemapInfo.gfxbank,tile,0,0);
} /* get_tile_info */

void namcos2_shared_state::c123_tilemap_init( int gfxbank, void *maskBaseAddr, c123_tilemap_delegate tilemap_cb )
{
	m_c123_TilemapInfo.gfxbank = gfxbank;
	m_c123_TilemapInfo.maskBaseAddr = (uint8_t *)maskBaseAddr;
	m_c123_TilemapInfo.cb = tilemap_cb;
	m_c123_TilemapInfo.videoram = std::make_unique<uint16_t[]>( 0x10000 );

		/* four scrolling tilemaps */
		m_c123_TilemapInfo.tmap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x0000>),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		m_c123_TilemapInfo.tmap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x1000>),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		m_c123_TilemapInfo.tmap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x2000>),this),TILEMAP_SCAN_ROWS,8,8,64,64);
		m_c123_TilemapInfo.tmap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x3000>),this),TILEMAP_SCAN_ROWS,8,8,64,64);

		/* two non-scrolling tilemaps */
		m_c123_TilemapInfo.tmap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x4008>),this),TILEMAP_SCAN_ROWS,8,8,36,28);
		m_c123_TilemapInfo.tmap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_shared_state::get_tile_info<0x4408>),this),TILEMAP_SCAN_ROWS,8,8,36,28);

		/* define offsets for scrolling */
		for( int i = 0; i < 4; i++ )
		{
			static const int adj[4] = { 4,2,1,0 };
			int dx = 44+adj[i];
			m_c123_TilemapInfo.tmap[i]->set_scrolldx( -dx, 288+dx );
			m_c123_TilemapInfo.tmap[i]->set_scrolldy( -24, 224+24 );
		}

	save_item(NAME(m_c123_TilemapInfo.control));
	save_pointer(NAME(m_c123_TilemapInfo.videoram), 0x10000);
}

void namcos2_shared_state::c123_tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	for( int i = 0; i < 6; i++ )
	{
		/* note: priority is only in range 0..7, but Point Blank uses 0xf to hide a layer */
		if( (m_c123_TilemapInfo.control[0x20/2+i]&0xf) == pri )
		{
			int color = m_c123_TilemapInfo.control[0x30/2+i] & 0x07;
			m_c123_TilemapInfo.tmap[i]->set_palette_offset( color*256 );
			m_c123_TilemapInfo.tmap[i]->draw(screen, bitmap,cliprect,0,0);
		}
	}
} /* c123_tilemap_draw */

void namcos2_shared_state::c123_SetTilemapVideoram(int offset, uint16_t newword)
{
	m_c123_TilemapInfo.videoram[offset] = newword;
	if( offset<0x4000 )
	{
		m_c123_TilemapInfo.tmap[offset>>12]->mark_tile_dirty(offset&0xfff);
	}
	else if( offset>=0x8010/2 && offset<0x87f0/2 )
	{ /* fixed plane#1 */
		offset-=0x8010/2;
		m_c123_TilemapInfo.tmap[4]->mark_tile_dirty( offset );
	}
	else if( offset>=0x8810/2 && offset<0x8ff0/2 )
	{ /* fixed plane#2 */
		offset-=0x8810/2;
		m_c123_TilemapInfo.tmap[5]->mark_tile_dirty( offset );
	}
} /* SetTilemapVideoram */

WRITE16_MEMBER( namcos2_shared_state::c123_tilemap_videoram_w )
{
	uint16_t newword = m_c123_TilemapInfo.videoram[offset];
	COMBINE_DATA( &newword );
	c123_SetTilemapVideoram( offset, newword );
}

READ16_MEMBER( namcos2_shared_state::c123_tilemap_videoram_r )
{
	return m_c123_TilemapInfo.videoram[offset];
}

void namcos2_shared_state::c123_SetTilemapControl(int offset, uint16_t newword)
{
	m_c123_TilemapInfo.control[offset] = newword;
	if( offset<0x20/2 )
	{
		if( offset == 0x02/2 )
		{
			/* all planes are flipped X+Y from D15 of this word */
			int attrs = (newword & 0x8000)?(TILEMAP_FLIPX|TILEMAP_FLIPY):0;
			int i;
			for( i=0; i<=5; i++ )
			{
				m_c123_TilemapInfo.tmap[i]->set_flip(attrs);
			}
		}
	}
	newword &= 0x1ff;
	if( m_c123_TilemapInfo.control[0x02/2]&0x8000 )
	{
		newword = -newword;
	}
	switch( offset )
	{
	case 0x02/2:
		m_c123_TilemapInfo.tmap[0]->set_scrollx( 0, newword );
		break;
	case 0x06/2:
		m_c123_TilemapInfo.tmap[0]->set_scrolly( 0, newword );
		break;
	case 0x0a/2:
		m_c123_TilemapInfo.tmap[1]->set_scrollx( 0, newword );
		break;
	case 0x0e/2:
		m_c123_TilemapInfo.tmap[1]->set_scrolly( 0, newword );
		break;
	case 0x12/2:
		m_c123_TilemapInfo.tmap[2]->set_scrollx( 0, newword );
		break;
	case 0x16/2:
		m_c123_TilemapInfo.tmap[2]->set_scrolly( 0, newword );
		break;
	case 0x1a/2:
		m_c123_TilemapInfo.tmap[3]->set_scrollx( 0, newword );
		break;
	case 0x1e/2:
		m_c123_TilemapInfo.tmap[3]->set_scrolly( 0, newword );
		break;
	}
} /* SetTilemapControl */

WRITE16_MEMBER( namcos2_shared_state::c123_tilemap_control_w )
{
	uint16_t newword = m_c123_TilemapInfo.control[offset];
	COMBINE_DATA( &newword );
	c123_SetTilemapControl( offset, newword );
}

READ16_MEMBER( namcos2_shared_state::c123_tilemap_control_r )
{
	return m_c123_TilemapInfo.control[offset];
}


/**************************************************************************************/

void namcos2_shared_state::zdrawgfxzoom(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	if (!scalex || !scaley) return;
	if (dest_bmp.bpp() == 16)
	{
		if( gfx )
		{
			int shadow_offset = (m_palette->shadows_enabled())?m_palette->entries():0;
			const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const uint8_t *source_base = gfx->get_data(code % gfx->elements());
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
							const uint8_t *source = source_base + (y_index>>16) * gfx->rowbytes();
							uint16_t *dest = &dest_bmp.pix16(y);
							uint8_t *pri = &priority_bitmap.pix8(y);
							int x, x_index = x_index_base;
							/* this code was previously shared with the c355 where this was needed
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
							*/
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
		uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	/* nop */
}

void namcos2_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int control )
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
	const uint16_t *pSource = m_spriteram;
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
			int scalex = (sizex<<16)/(0x20);//(sizex<<16)/(bBigSprite?0x20:0x10); correct formula?
			int scaley = (sizey<<16)/(bBigSprite?0x20:0x10);

			/* swap xy */
			int rgn = (flags&0x01) ? 3 : 0;

			gfx_element *gfx = m_gfxdecode->gfx(rgn);

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
				gfx->set_source_clip(0, 32, 0, 32);
			}
			else
				gfx->set_source_clip((tile&0x0001) ? 16 : 0, 16, (tile&0x0002) ? 16 : 0, 16);

			zdrawgfxzoom(
				screen,
				bitmap,
				cliprect,
				gfx,
				sprn, color,
				flipx,flipy,
				sx,sy,
				scalex, scaley,
				loop );
		}
		pSource += 8;
	}
} /* namcos2_draw_sprites_metalhawk */
