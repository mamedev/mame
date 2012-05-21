/*  Namco System NA1/2 Video Hardware */

/*
TODO:
- dynamic screen resolution (changes between emeralda test mode and normal game)
- non-shadow pixels for sprites flagged to enable shadows have bad colors
*/

#include "emu.h"
#include "includes/namcona1.h"


static void tilemap_get_info(
	running_machine &machine,
	tile_data &tileinfo,
	int tile_index,
	const UINT16 *tilemap_videoram,
	int tilemap_color,
	int use_4bpp_gfx )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *source;

	int data = tilemap_videoram[tile_index];
	int tile = data&0xfff;
	int gfx;

	if( use_4bpp_gfx )
	{
		gfx = 1;
		tilemap_color *= 0x10;
		tilemap_color += (data & 0x7000)>>12;
	}
	else
	{
		gfx = 0;
	}

	if( data & 0x8000 )
	{
		SET_TILE_INFO( gfx,tile,tilemap_color,TILE_FORCE_LAYER0 );
	}
	else
	{
		SET_TILE_INFO( gfx,tile,tilemap_color,0 );
		if (ENDIANNESS_NATIVE == ENDIANNESS_BIG)
			tileinfo.mask_data = (UINT8 *)(state->m_shaperam+4*tile);
		else
		{
			UINT8 *mask_data = state->m_mask_data;
			source = state->m_shaperam+4*tile;
			mask_data[0] = source[0]>>8;
			mask_data[1] = source[0]&0xff;
			mask_data[2] = source[1]>>8;
			mask_data[3] = source[1]&0xff;
			mask_data[4] = source[2]>>8;
			mask_data[5] = source[2]&0xff;
			mask_data[6] = source[3]>>8;
			mask_data[7] = source[3]&0xff;
			tileinfo.mask_data = mask_data;
		}
	}
} /* tilemap_get_info */

static TILE_GET_INFO( tilemap_get_info0 )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	tilemap_get_info(machine,tileinfo,tile_index,0*0x1000+videoram,state->m_tilemap_palette_bank[0],state->m_vreg[0xbc/2]&1);
}

static TILE_GET_INFO( tilemap_get_info1 )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	tilemap_get_info(machine,tileinfo,tile_index,1*0x1000+videoram,state->m_tilemap_palette_bank[1],state->m_vreg[0xbc/2]&2);
}

static TILE_GET_INFO( tilemap_get_info2 )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	tilemap_get_info(machine,tileinfo,tile_index,2*0x1000+videoram,state->m_tilemap_palette_bank[2],state->m_vreg[0xbc/2]&4);
}

static TILE_GET_INFO( tilemap_get_info3 )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	tilemap_get_info(machine,tileinfo,tile_index,3*0x1000+videoram,state->m_tilemap_palette_bank[3],state->m_vreg[0xbc/2]&8);
}

static TILE_GET_INFO( roz_get_info )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	/* each logical tile is constructed from 4*4 normal tiles */
	int tilemap_color = state->m_roz_palette;
	int use_4bpp_gfx = state->m_vreg[0xbc/2]&16; /* ? */
	int c = tile_index%0x40;
	int r = tile_index/0x40;
	int data = videoram[0x8000/2+(r/4)*0x40+c/4]&0xfbf; /* mask out bit 0x40 - patch for Emeraldia Japan */
	int tile = (data+(c%4)+(r%4)*0x40)&0xfff;
	int gfx = use_4bpp_gfx;
	if( use_4bpp_gfx )
	{
		tilemap_color *= 0x10;
		tilemap_color += (data & 0x7000)>>12;
	}
	if( data & 0x8000 )
	{
		SET_TILE_INFO( gfx,tile,tilemap_color,TILE_FORCE_LAYER0 );
	}
	else
	{
		UINT8 *mask_data = (UINT8 *)(state->m_shaperam+4*tile);

		if (ENDIANNESS_NATIVE == ENDIANNESS_LITTLE)
		{
			UINT16 *source = (UINT16 *)mask_data;
			UINT8 *conv_data = state->m_conv_data;
			conv_data[0] = source[0]>>8;
			conv_data[1] = source[0]&0xff;
			conv_data[2] = source[1]>>8;
			conv_data[3] = source[1]&0xff;
			conv_data[4] = source[2]>>8;
			conv_data[5] = source[2]&0xff;
			conv_data[6] = source[3]>>8;
			conv_data[7] = source[3]&0xff;
			mask_data = conv_data;
		}
		SET_TILE_INFO( gfx,tile,tilemap_color,0 );
		tileinfo.mask_data = mask_data;
	}
} /* roz_get_info */

/*************************************************************************/

WRITE16_MEMBER(namcona1_state::namcona1_videoram_w)
{
	UINT16 *videoram = m_videoram;
	COMBINE_DATA( &videoram[offset] );
	if( offset<0x8000/2 )
	{
		m_bg_tilemap[offset/0x1000]->mark_tile_dirty(offset&0xfff );
	}
	else if( offset<0xa000/2 )
	{
		m_roz_tilemap ->mark_all_dirty();
	}
} /* namcona1_videoram_w */

READ16_MEMBER(namcona1_state::namcona1_videoram_r)
{
	UINT16 *videoram = m_videoram;
	return videoram[offset];
} /* namcona1_videoram_r */

/*************************************************************************/

static void
UpdatePalette(running_machine &machine, int offset )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 data = state->m_generic_paletteram_16[offset]; /* -RRRRRGG GGGBBBBB */
	/**
     * sprites can be configured to use an alternate interpretation of palette ram
     * (used in-game in Emeraldia)
     *
     * RRRGGGBB RRRGGGBB
     */
	int r = (((data&0x00e0)>>5)+((data&0xe000)>>13)*2)*0xff/(0x7*3);
	int g = (((data&0x001c)>>2)+((data&0x1c00)>>10)*2)*0xff/(0x7*3);
	int b = (((data&0x0003)>>0)+((data&0x0300)>>8)*2)*0xff/(0x3*3);
	palette_set_color_rgb( machine, offset+0x1000, r, g, b);

	palette_set_color_rgb( machine, offset, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
} /* namcona1_paletteram_w */

READ16_MEMBER(namcona1_state::namcona1_paletteram_r)
{
	return m_generic_paletteram_16[offset];
} /* namcona1_paletteram_r */

WRITE16_MEMBER(namcona1_state::namcona1_paletteram_w)
{
	COMBINE_DATA( &m_generic_paletteram_16[offset] );
	if( m_vreg[0x8e/2] )
	{ /* graphics enabled; update palette immediately */
		UpdatePalette(machine(), offset );
	}
	else
	{
		m_palette_is_dirty = 1;
	}
}

/*************************************************************************/

#define XOR(a) BYTE_XOR_BE(a)

static const gfx_layout shape_layout =
{
	8,8,
	0x1000,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 8*XOR(0),8*XOR(1),8*XOR(2),8*XOR(3),8*XOR(4),8*XOR(5),8*XOR(6),8*XOR(7) },
	8*8
}; /* shape_layout */

static const gfx_layout cg_layout_8bpp =
{
	8,8,
	0x1000,
	8, /* 8BPP */
	{ 0,1,2,3,4,5,6,7 },
	{ 8*XOR(0),8*XOR(1),8*XOR(2),8*XOR(3),8*XOR(4),8*XOR(5),8*XOR(6),8*XOR(7) },
	{ 64*0,64*1,64*2,64*3,64*4,64*5,64*6,64*7 },
	64*8
}; /* cg_layout_8bpp */

static const gfx_layout cg_layout_4bpp =
{
	8,8,
	0x1000,
	4, /* 4BPP */
	{ 4,5,6,7 },
	{ 8*XOR(0),8*XOR(1),8*XOR(2),8*XOR(3),8*XOR(4),8*XOR(5),8*XOR(6),8*XOR(7) },
	{ 64*0,64*1,64*2,64*3,64*4,64*5,64*6,64*7 },
	64*8
}; /* cg_layout_4bpp */

READ16_MEMBER(namcona1_state::namcona1_gfxram_r)
{
	UINT16 type = m_vreg[0x0c/2];
	if( type == 0x03 )
	{
		if( offset<0x4000 )
		{
			return m_shaperam[offset];
		}
	}
	else if( type == 0x02 )
	{
		return m_cgram[offset];
	}
	return 0x0000;
} /* namcona1_gfxram_r */

WRITE16_MEMBER(namcona1_state::namcona1_gfxram_w)
{
	UINT16 type = m_vreg[0x0c/2];
	UINT16 old_word;

	if( type == 0x03 )
	{
		if( offset<0x4000 )
		{
			old_word = m_shaperam[offset];
			COMBINE_DATA( &m_shaperam[offset] );
			if( m_shaperam[offset]!=old_word )
				gfx_element_mark_dirty(machine().gfx[2], offset/4);
		}
	}
	else if( type == 0x02 )
	{
		old_word = m_cgram[offset];
		COMBINE_DATA( &m_cgram[offset] );
		if( m_cgram[offset]!=old_word )
		{
			gfx_element_mark_dirty(machine().gfx[0], offset/0x20);
			gfx_element_mark_dirty(machine().gfx[1], offset/0x20);
		}
	}
} /* namcona1_gfxram_w */

static void UpdateGfx(running_machine &machine)
{
} /* UpdateGfx */

VIDEO_START( namcona1 )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	static const tile_get_info_func get_info[4] = { tilemap_get_info0, tilemap_get_info1, tilemap_get_info2, tilemap_get_info3 };
	int i;

	state->m_roz_tilemap = tilemap_create( machine, roz_get_info, tilemap_scan_rows, 8,8,64,64 );
	state->m_roz_palette = -1;

	for( i=0; i<NAMCONA1_NUM_TILEMAPS; i++ )
	{
		state->m_bg_tilemap[i] = tilemap_create( machine, get_info[i], tilemap_scan_rows, 8,8,64,64 );
		state->m_tilemap_palette_bank[i] = -1;
	}

	state->m_shaperam		     = auto_alloc_array(machine, UINT16, 0x2000*4/2 );
	state->m_cgram			     = auto_alloc_array(machine, UINT16, 0x1000*0x40/2 );

	machine.gfx[0] = gfx_element_alloc( machine, &cg_layout_8bpp, (UINT8 *)state->m_cgram, machine.total_colors()/256, 0 );
	machine.gfx[1] = gfx_element_alloc( machine, &cg_layout_4bpp, (UINT8 *)state->m_cgram, machine.total_colors()/16, 0 );
	machine.gfx[2] = gfx_element_alloc( machine, &shape_layout, (UINT8 *)state->m_shaperam, machine.total_colors()/2, 0 );

} /* namcona1_vh_start */

/*************************************************************************/

static void pdraw_tile(running_machine &machine,
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
	const gfx_element *gfx = machine.gfx[gfx_region];
	const gfx_element *mask = machine.gfx[2];

	int pal_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	const UINT8 *source_base = gfx_element_get_data(gfx, (code % gfx->total_elements));
	const UINT8 *mask_base = gfx_element_get_data(mask, (code % mask->total_elements));

	int sprite_screen_height = ((1<<16)*gfx->height+0x8000)>>16;
	int sprite_screen_width  = ((1<<16)*gfx->width+0x8000)>>16;

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
				const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
				const UINT8 *mask_addr = mask_base + (y_index>>16) * mask->line_modulo;
				UINT16 *dest = &dest_bmp.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);

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
										pen_t *palette_shadow_table = machine.shadow_table;
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

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	int which;
	const UINT16 *source = state->m_spriteram;
	UINT16 sprite_control;
	UINT16 ypos,tile,color,xpos;
	int priority;
	int width,height;
	int flipy,flipx;
	int row,col;
	int sx,sy;

	sprite_control = state->m_vreg[0x22/2];
	if( sprite_control&1 ) source += 0x400; /* alternate spriteram bank */

	for( which=0; which<0x100; which++ )
	{ /* max 256 sprites */
		int bpp4,palbase;
		ypos	= source[0];	/* FHHH---Y YYYYYYYY    flipy, height, ypos */
		tile	= source[1];	/* O???TTTT TTTTTTTT    opaque tile */
		color	= source[2];	/* FSWWOOOO CCCCBPPP    flipx, shadow, width, color offset for 4bpp, color, 4bbp - 8bpp mode, pri*/
		xpos	= source[3];	/* -------X XXXXXXXX    xpos */

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

				pdraw_tile(machine,
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

static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int primask )
{
	namcona1_state *state = machine.driver_data<namcona1_state>();
	UINT16 *videoram = state->m_videoram;
	/*          scrollx lineselect
     *  tmap0   ffe000  ffe200
     *  tmap1   ffe400  ffe600
     *  tmap2   ffe800  ffea00
     *  tmap3   ffec00  ffee00
     */
	int xadjust = 0x3a - which*2;
	const UINT16 *scroll = state->m_scroll+0x200*which;
	int line;
	UINT16 xdata, ydata;
	int scrollx, scrolly;
	rectangle clip;
	const pen_t *paldata;
	gfx_element *pGfx;

	pGfx = machine.gfx[0];
	paldata = &machine.pens[pGfx->color_base + pGfx->color_granularity * state->m_tilemap_palette_bank[which]];

	/* draw one scanline at a time */
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;
	scrollx = 0;
	scrolly = 0;
	for( line=0; line<256; line++ )
	{
		clip.min_y = line;
		clip.max_y = line;
		xdata = scroll[line];
		if( xdata )
		{
			/* screenwise linescroll */
			scrollx = xadjust+xdata;
		}
		ydata = scroll[line+0x100];
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
				draw_pixel_line(
					&bitmap.pix16(line),
					&machine.priority_bitmap.pix8(line),
					videoram + ydata + 25,
					paldata );
			}
			else
			{
				if(which == NAMCONA1_NUM_TILEMAPS )
				{
					int incxx = ((INT16)state->m_vreg[0xc0/2])<<8;
					int incxy = ((INT16)state->m_vreg[0xc2/2])<<8;
					int incyx = ((INT16)state->m_vreg[0xc4/2])<<8;
					int incyy = ((INT16)state->m_vreg[0xc6/2])<<8;
					INT16 xoffset = state->m_vreg[0xc8/2];
					INT16 yoffset = state->m_vreg[0xca/2];
					int dx = 46; /* horizontal adjust */
					int dy = -8; /* vertical adjust */
					UINT32 startx = (xoffset<<12)+incxx*dx+incyx*dy;
					UINT32 starty = (yoffset<<12)+incxy*dx+incyy*dy;
					state->m_roz_tilemap->draw_roz(bitmap, clip,
						startx, starty, incxx, incxy, incyx, incyy, 0, 0, primask, 0);
				}
				else
				{
					state->m_bg_tilemap[which]->set_scrollx(0, scrollx );
					state->m_bg_tilemap[which]->set_scrolly(0, scrolly );
					state->m_bg_tilemap[which]->draw(bitmap, clip, 0, primask, 0 );
				}
			}
		}
	}
} /* draw_background */

SCREEN_UPDATE_IND16( namcona1 )
{
	namcona1_state *state = screen.machine().driver_data<namcona1_state>();
	int which;
	int priority;

	/* int flipscreen = state->m_vreg[0x98/2]; (TBA) */

	if( state->m_vreg[0x8e/2] )
	{ /* gfx enabled */
		if( state->m_palette_is_dirty )
		{
			/* palette updates are delayed when graphics are disabled */
			for( which=0; which<0x1000; which++ )
			{
				UpdatePalette(screen.machine(), which );
			}
			state->m_palette_is_dirty = 0;
		}
		UpdateGfx(screen.machine());
		for( which=0; which<NAMCONA1_NUM_TILEMAPS; which++ )
		{
			int tilemap_color = state->m_vreg[0xb0/2+(which&3)]&0xf;
			if( tilemap_color!=state->m_tilemap_palette_bank[which] )
			{
				state->m_bg_tilemap[which] ->mark_all_dirty();
				state->m_tilemap_palette_bank[which] = tilemap_color;
			}
		} /* next tilemap */

		{ /* ROZ tilemap */
			int color = state->m_vreg[0xba/2]&0xf;
			if( color != state->m_roz_palette )
			{
				state->m_roz_tilemap ->mark_all_dirty();
				state->m_roz_palette = color;
			}
		}

		screen.machine().priority_bitmap.fill(0, cliprect );

		bitmap.fill(0xff, cliprect ); /* background color? */

		for( priority = 0; priority<8; priority++ )
		{
			for( which=4; which>=0; which-- )
			{
				int pri;
				if( which==4 )
				{
					pri = state->m_vreg[0xa0/2+5]&0x7;
				}
				else
				{
					pri = state->m_vreg[0xa0/2+which]&0x7;
				}
				if( pri == priority )
				{
					draw_background(screen.machine(),bitmap,cliprect,which,priority);
				}
			} /* next tilemap */
		} /* next priority level */

		draw_sprites(screen.machine(),bitmap,cliprect);
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
