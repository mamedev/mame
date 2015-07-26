// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*  Namco System NA1/2 Video Hardware */

/*
TODO:
- dynamic screen resolution (changes between emeralda test mode and normal game)
- non-shadow pixels for sprites flagged to enable shadows have bad colors
*/

#include "emu.h"
#include "includes/namcona1.h"


void namcona1_state::tilemap_get_info(
	tile_data &tileinfo,
	int tile_index,
	const UINT16 *tilemap_videoram,
	bool use_4bpp_gfx )
{
	int data = tilemap_videoram[tile_index];
	int tile = data&0xfff;
	int gfx = use_4bpp_gfx ? 1 : 0;
	int color = use_4bpp_gfx ? (data & 0x7000)>>12 : 0;

	if( data & 0x8000 )
	{
		SET_TILE_INFO_MEMBER(gfx,tile,color,TILE_FORCE_LAYER0 );
	}
	else
	{
		SET_TILE_INFO_MEMBER(gfx,tile,color,0 );
		tileinfo.mask_data = &m_shaperam[tile*8];
	}
} /* tilemap_get_info */

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info0)
{
	tilemap_get_info(tileinfo,tile_index,0*0x1000+m_videoram,m_vreg[0xbc/2]&1);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info1)
{
	tilemap_get_info(tileinfo,tile_index,1*0x1000+m_videoram,m_vreg[0xbc/2]&2);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info2)
{
	tilemap_get_info(tileinfo,tile_index,2*0x1000+m_videoram,m_vreg[0xbc/2]&4);
}

TILE_GET_INFO_MEMBER(namcona1_state::tilemap_get_info3)
{
	tilemap_get_info(tileinfo,tile_index,3*0x1000+m_videoram,m_vreg[0xbc/2]&8);
}

TILE_GET_INFO_MEMBER(namcona1_state::roz_get_info)
{
	/* each logical tile is constructed from 4*4 normal tiles */
	int use_4bpp_gfx = m_vreg[0xbc/2]&16; /* ? */
	int c = tile_index%0x40;
	int r = tile_index/0x40;
	int data = m_videoram[0x8000/2+(r/4)*0x40+c/4]&0xfbf; /* mask out bit 0x40 - patch for Emeraldia Japan */
	int tile = (data+(c%4)+(r%4)*0x40)&0xfff;
	int gfx = use_4bpp_gfx ? 1 : 0;
	int color = use_4bpp_gfx ? (data & 0x7000)>>12 : 0;

	if( data & 0x8000 )
	{
		SET_TILE_INFO_MEMBER(gfx,tile,color,TILE_FORCE_LAYER0 );
	}
	else
	{
		SET_TILE_INFO_MEMBER(gfx,tile,color,0 );
		tileinfo.mask_data = &m_shaperam[tile*8];
	}
} /* roz_get_info */

/*************************************************************************/

WRITE16_MEMBER(namcona1_state::videoram_w)
{
	COMBINE_DATA( &m_videoram[offset] );
	if( offset<0x8000/2 )
	{
		m_bg_tilemap[offset/0x1000]->mark_tile_dirty(offset&0xfff);
	}
	else if( offset<0xa000/2 )
	{
		m_bg_tilemap[4]->mark_all_dirty();
	}
} /* videoram_w */

/*************************************************************************/

void namcona1_state::UpdatePalette( int offset )
{
	UINT16 data = m_paletteram[offset]; /* -RRRRRGG GGGBBBBB */
	/**
	 * sprites can be configured to use an alternate interpretation of palette ram
	 * (used in-game in Emeraldia)
	 *
	 * RRRGGGBB RRRGGGBB
	 */
	int r = (((data&0x00e0)>>5)+((data&0xe000)>>13)*2)*0xff/(0x7*3);
	int g = (((data&0x001c)>>2)+((data&0x1c00)>>10)*2)*0xff/(0x7*3);
	int b = (((data&0x0003)>>0)+((data&0x0300)>>8)*2)*0xff/(0x3*3);
	m_palette->set_pen_color(offset+0x1000, r, g, b);

	m_palette->set_pen_color(offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE16_MEMBER(namcona1_state::paletteram_w)
{
	COMBINE_DATA( &m_paletteram[offset] );
	if( m_vreg[0x8e/2] )
	{ /* graphics enabled; update palette immediately */
		UpdatePalette( offset );
	}
	else
	{
		m_palette_is_dirty = 1;
	}
}


READ16_MEMBER(namcona1_state::gfxram_r)
{
	UINT16 type = m_vreg[0x0c/2];
	if( type == 0x03 )
	{
		if( offset<0x4000 )
		{
			offset *= 2;
			return (m_shaperam[offset] << 8) | m_shaperam[offset+1];
		}
	}
	else if( type == 0x02 )
	{
		return m_cgram[offset];
	}
	return 0x0000;
} /* gfxram_r */

WRITE16_MEMBER(namcona1_state::gfxram_w)
{
	UINT16 type = m_vreg[0x0c/2];
	UINT16 old_word;

	if( type == 0x03 )
	{
		if( offset<0x4000 )
		{
			offset *= 2;
			if (ACCESSING_BITS_8_15)
				m_shaperam[offset] = data >> 8;
			if (ACCESSING_BITS_0_7)
				m_shaperam[offset+1] = data;
			m_gfxdecode->gfx(2)->mark_dirty(offset/8);
		}
	}
	else if( type == 0x02 )
	{
		old_word = m_cgram[offset];
		COMBINE_DATA( &m_cgram[offset] );
		if( m_cgram[offset]!=old_word )
		{
			m_gfxdecode->gfx(0)->mark_dirty(offset/0x20);
			m_gfxdecode->gfx(1)->mark_dirty(offset/0x20);
		}
	}
} /* gfxram_w */

void namcona1_state::video_start()
{
	// normal tilemaps
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info0),this), TILEMAP_SCAN_ROWS, 8,8,64,64 );
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info1),this), TILEMAP_SCAN_ROWS, 8,8,64,64 );
	m_bg_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info2),this), TILEMAP_SCAN_ROWS, 8,8,64,64 );
	m_bg_tilemap[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::tilemap_get_info3),this), TILEMAP_SCAN_ROWS, 8,8,64,64 );

	// roz tilemap
	m_bg_tilemap[4] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcona1_state::roz_get_info),this), TILEMAP_SCAN_ROWS, 8,8,64,64 );

	m_shaperam.resize(0x8000);

	m_gfxdecode->gfx(2)->set_source(&m_shaperam[0]);
	
	save_item(NAME(m_shaperam));
	save_item(NAME(m_palette_is_dirty));
	
	machine().save().register_postload(save_prepost_delegate(FUNC(namcona1_state::postload), this));
} /* video_start */

void namcona1_state::postload()
{
	for (int i = 0; i < 3; i++)
		m_gfxdecode->gfx(i)->mark_all_dirty();
}


/*************************************************************************/

void namcona1_state::pdraw_tile(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,
		const rectangle &clip,
		UINT32 code,
		int color,
		int sx, int sy,
		int flipx, int flipy,
		int priority,
		int bShadow,
		int bOpaque,
		int gfx_region )
{
	gfx_element *gfx = m_gfxdecode->gfx(gfx_region);
	gfx_element *mask = m_gfxdecode->gfx(2);

	int pal_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const UINT8 *source_base = gfx->get_data((code % gfx->elements()));
	const UINT8 *mask_base = mask->get_data((code % mask->elements()));

	int sprite_screen_height = ((1<<16)*gfx->height()+0x8000)>>16;
	int sprite_screen_width  = ((1<<16)*gfx->width()+0x8000)>>16;

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
		/* NS 980211 - fixed incorrect clipping */
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

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->rowbytes();
				const UINT8 *mask_addr = mask_base + (y_index>>16) * mask->rowbytes();
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &screen.priority().pix8(y);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					if( bOpaque )
					{
						if (pri[x] <= priority)
						{
							int c = source[x_index>>16];
							dest[x] = pal_base + c;
						}

						pri[x] = 0xff;
					}
					else
					{
						/* sprite pixel is opaque */
						if( mask_addr[x_index>>16] != 0 )
						{
							if (pri[x] <= priority)
							{
								int c = source[x_index>>16];

								/* render a shadow only if the sprites color is $F (8bpp) or $FF (4bpp) */
								if( bShadow )
								{
									if( (gfx_region == 0 && color == 0x0f) ||
										(gfx_region == 1 && color == 0xff) )
									{
										pen_t *palette_shadow_table = m_palette->shadow_table();
										dest[x] = palette_shadow_table[dest[x]];
									}
									else
									{
										dest[x] = pal_base + c + 0x1000;
									}
								}
								else
								{
									dest[x] = pal_base + c;
								}
							}
							pri[x] = 0xff;
						}
					}

					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
} /* pdraw_tile */

void namcona1_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int which;
	const UINT16 *source = m_spriteram;
	UINT16 sprite_control;
	UINT16 ypos,tile,color,xpos;
	int priority;
	int width,height;
	int flipy,flipx;
	int row,col;
	int sx,sy;

	sprite_control = m_vreg[0x22/2];
	if( sprite_control&1 ) source += 0x400; /* alternate spriteram bank */

	for( which=0; which<0x100; which++ )
	{ /* max 256 sprites */
		int bpp4,palbase;
		ypos    = source[0];    /* FHHH---Y YYYYYYYY    flipy, height, ypos */
		tile    = source[1];    /* O???TTTT TTTTTTTT    opaque tile */
		color   = source[2];    /* FSWWOOOO CCCCBPPP    flipx, shadow, width, color offset for 4bpp, color, 4bbp - 8bpp mode, pri*/
		xpos    = source[3];    /* -------X XXXXXXXX    xpos */

		priority = color&0x7;
		width = ((color>>12)&0x3)+1;
		height = ((ypos>>12)&0x7)+1;
		flipy = ypos&0x8000;
		flipx = color&0x8000;

		if( color&8 )
		{
			palbase = ((color>>4)&0xf) * 0x10 + ((color & 0xf00) >> 8);
			bpp4 = 1;
		}
		else
		{
			palbase = (color>>4)&0xf;
			bpp4 = 0;
		}

		for( row=0; row<height; row++ )
		{
			sy = (ypos&0x1ff)-30+32;
			if( flipy )
			{
				sy += (height-1-row)*8;
			}
			else
			{
				sy += row*8;
			}
			sy = ((sy+8)&0x1ff)-8;

			for( col=0; col<width; col++ )
			{
				sx = (xpos&0x1ff)-10;
				if( flipx )
				{
					sx += (width-1-col)*8;
				}
				else
				{
					sx+=col*8;
				}
				sx = ((sx+16)&0x1ff)-8;

				pdraw_tile(screen,
					bitmap,
					cliprect,
					(tile & 0xfff) + row*64+col,
					palbase,
					sx,sy,flipx,flipy,
					priority,
					color & 0x4000, /* shadow */
					tile & 0x8000, /* opaque */
					bpp4 );

			} /* next col */
		} /* next row */
		source += 4;
	}
} /* draw_sprites */

static void draw_pixel_line( UINT16 *pDest, UINT8 *pPri, UINT16 *pSource, const pen_t *paldata )
{
	int x;
	for( x=0; x<38*8; x+=2 )
	{
		UINT16 data = *pSource++;
		pPri[x+0] = 0xff;
		pPri[x+1] = 0xff;
		pDest[x+0] = paldata[data>>8];
		pDest[x+1] = paldata[data&0xff];
	} /* next x */
} /* draw_pixel_line */

void namcona1_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask )
{
	if(which == 4)
	{
		/* draw the roz tilemap all at once */
		int incxx = ((INT16)m_vreg[0xc0/2])<<8;
		int incxy = ((INT16)m_vreg[0xc2/2])<<8;
		int incyx = ((INT16)m_vreg[0xc4/2])<<8;
		int incyy = ((INT16)m_vreg[0xc6/2])<<8;
		INT16 xoffset = m_vreg[0xc8/2];
		INT16 yoffset = m_vreg[0xca/2];
		int dx = 46; /* horizontal adjust */
		int dy = -8; /* vertical adjust */
		UINT32 startx = (xoffset<<12)+incxx*dx+incyx*dy;
		UINT32 starty = (yoffset<<12)+incxy*dx+incyy*dy;
		m_bg_tilemap[4]->draw_roz(screen, bitmap, cliprect,
			startx, starty, incxx, incxy, incyx, incyy, 0, 0, primask, 0);
	}
	else
	{
		/* draw one scanline at a time */
		/*          scrollx lineselect
		*  tmap0   ffe000  ffe200
		*  tmap1   ffe400  ffe600
		*  tmap2   ffe800  ffea00
		*  tmap3   ffec00  ffee00
		*/
		const UINT16 *scroll = &m_scroll[which * 0x400/2];
		const pen_t *paldata = &m_palette->pen(m_bg_tilemap[which]->palette_offset());
		rectangle clip = cliprect;
		int xadjust = 0x3a - which*2;
		int scrollx = 0;
		int scrolly = 0;

		for( int line = 0; line < 256; line++ )
		{
			clip.min_y = line;
			clip.max_y = line;
			int xdata = scroll[line];
			int ydata = scroll[line + 0x200/2];

			if( xdata )
			{
				/* screenwise linescroll */
				scrollx = xadjust+xdata;
			}

			if( ydata&0x4000 )
			{
				/* line select: dword offset from 0xff000 or tilemap source line */
				scrolly = (ydata - line)&0x1ff;
			}

			if (line >= cliprect.min_y && line <= cliprect.max_y)
			{
				if( xdata == 0xc001 )
				{
					/* This is a simplification, but produces the correct behavior for the only game that uses this
					* feature, Numan Athletics.
					*/
					draw_pixel_line(&bitmap.pix16(line),
								&screen.priority().pix8(line),
								m_videoram + ydata + 25,
								paldata );
				}
				else
				{
					m_bg_tilemap[which]->set_scrollx(0, scrollx );
					m_bg_tilemap[which]->set_scrolly(0, scrolly );
					m_bg_tilemap[which]->draw(screen, bitmap, clip, 0, primask, 0 );
				}
			}
		}
	}
} /* draw_background */

UINT32 namcona1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int which;
	int priority;

	/* int flipscreen = m_vreg[0x98/2]; (TBA) */

	if( m_vreg[0x8e/2] )
	{ /* gfx enabled */
		if( m_palette_is_dirty )
		{
			/* palette updates are delayed when graphics are disabled */
			for( which=0; which<0x1000; which++ )
			{
				UpdatePalette( which );
			}
			m_palette_is_dirty = 0;
		}

		for( which=0; which < 4; which++ )
			m_bg_tilemap[which]->set_palette_offset((m_vreg[0xb0/2 + which] & 0xf) * 256);

		m_bg_tilemap[4]->set_palette_offset((m_vreg[0xba/2] & 0xf) * 256);

		screen.priority().fill(0, cliprect );

		bitmap.fill(0xff, cliprect ); /* background color? */

		for( priority = 0; priority<8; priority++ )
		{
			for( which=4; which>=0; which-- )
			{
				int pri;
				if( which==4 )
				{
					pri = m_vreg[0xa0/2+5]&0x7;
				}
				else
				{
					pri = m_vreg[0xa0/2+which]&0x7;
				}
				if( pri == priority )
				{
					draw_background(screen,bitmap,cliprect,which,priority);
				}
			} /* next tilemap */
		} /* next priority level */

		draw_sprites(screen,bitmap,cliprect);
	} /* gfx enabled */
	return 0;
}

/*
roz bad in emeraldaj - blit param?

$efff20: sprite control: 0x3a,0x3e,0x3f
            bit 0x01 selects spriteram bank

               0    2    4    6    8    a    c    e
$efff00:    src0 src1 src2 dst0 dst1 dst2 BANK [src
$efff10:    src] [dst dst] #BYT BLIT eINT 001f 0001
$efff20:    003f 003f IACK ---- ---- ---- ---- ----
...
$efff80:    0050 0170 0020 0100 0170 POS? 0000 GFXE
$efff90:    0000 0001 0002 0003 FLIP ---- ---- ----
$efffa0:    PRI  PRI  PRI  PRI  ???? PRI  00c0 ----     priority (0..7)
$efffb0:    CLR  CLR  CLR  CLR  0001 CLR  BPP  ----     color (0..f), bpp flag per layer
$efffc0:    RZXX RZXY RZYX RZYY RZX0 RZY0 0044 ----     ROZ


Emeralda:   0048 0177 0020 0100 0000 00f0 0000 0001     in-game
            0050 0170 0020 0100 0000 00f0 0000 0001     self test

NumanAth:   0050 0170 0020 0100 0170 00d8 0000 0001     in-game


*/
