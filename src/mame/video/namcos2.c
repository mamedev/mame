// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/* video hardware for Namco System II */

#include "emu.h"

#include "includes/namcos2.h"
#include "includes/namcoic.h"

static void
TilemapCB( running_machine &machine, UINT16 code, int *tile, int *mask )
//void namcos2_shared_state::tilemap_cb(UINT16 code, int *tile, int *mask)
{
	*mask = code;

	namcos2_shared_state *state = machine.driver_data<namcos2_shared_state>();
	switch( state->m_gametype )
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
}

/**
 * m_gfx_ctrl selects a bank of 128 sprites within spriteram
 *
 * m_gfx_ctrl also supplies palette and priority information that is applied to the output of the
 *            Namco System 2 ROZ chip
 *
 * -xxx ---- ---- ---- roz priority
 * ---- xxxx ---- ---- roz palette
 * ---- ---- xxxx ---- always zero?
 * ---- ---- ---- xxxx sprite bank
 */
READ16_MEMBER( namcos2_state::gfx_ctrl_r )
{
	return m_gfx_ctrl;
}

WRITE16_MEMBER( namcos2_state::gfx_ctrl_w )
{
	COMBINE_DATA(&m_gfx_ctrl);
}

TILE_GET_INFO_MEMBER( namcos2_state::roz_tile_info )
{
	int tile = m_rozram[tile_index];
	SET_TILE_INFO_MEMBER(3,tile,0/*color*/,0);
}

struct roz_param
{
	UINT32 size;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int color;
	int wrap;
};

INLINE void
draw_roz_helper_block(const struct roz_param *rozInfo, int destx, int desty,
	int srcx, int srcy, int width, int height,
	bitmap_ind16 &destbitmap, bitmap_ind8 &flagsbitmap,
	bitmap_ind16 &srcbitmap, UINT32 size_mask)
{
	int desty_end = desty + height;

	int end_incrx = rozInfo->incyx - (width * rozInfo->incxx);
	int end_incry = rozInfo->incyy - (width * rozInfo->incxy);

	UINT16 *dest = &destbitmap.pix16(desty, destx);
	int dest_rowinc = destbitmap.rowpixels() - width;

	while (desty < desty_end)
	{
		UINT16 *dest_end = dest + width;
		while (dest < dest_end)
		{
			UINT32 xpos = (srcx >> 16);
			UINT32 ypos = (srcy >> 16);

			if (rozInfo->wrap)
			{
				xpos &= size_mask;
				ypos &= size_mask;
			}
			else if ((xpos > rozInfo->size) || (ypos >= rozInfo->size))
			{
				goto L_SkipPixel;
			}

			if (flagsbitmap.pix8(ypos, xpos) & TILEMAP_PIXEL_LAYER0)
			{
				*dest = srcbitmap.pix16(ypos, xpos) + rozInfo->color;
			}

		L_SkipPixel:

			srcx += rozInfo->incxx;
			srcy += rozInfo->incxy;
			dest++;
		}
		srcx += end_incrx;
		srcy += end_incry;
		dest += dest_rowinc;
		desty++;
	}
}

static void
draw_roz_helper(
	screen_device &screen,
	bitmap_ind16 &bitmap,
	tilemap_t *tmap,
	const rectangle &clip,
	const struct roz_param *rozInfo )
{
	tmap->set_palette_offset(rozInfo->color );

	if( bitmap.bpp() == 16 )
	{
		/* On many processors, the simple approach of an outer loop over the
		    rows of the destination bitmap with an inner loop over the columns
		    of the destination bitmap has poor performance due to the order
		    that memory in the source bitmap is referenced when rotation
		    approaches 90 or 270 degrees.  The reason is that the inner loop
		    ends up reading pixels not sequentially in the source bitmap, but
		    instead at rozInfo->incxx increments, which is at its maximum at 90
		    degrees of rotation.  This means that only a few (or as few as
		    one) source pixels are in each cache line at a time.

		    Instead of the above, this code iterates in NxN blocks through the
		    destination bitmap.  This has more overhead when there is little or
		    no rotation, but much better performance when there is closer to 90
		    degrees of rotation (as long as the chunk of the source bitmap that
		    corresponds to an NxN destination block fits in cache!).

		    N is defined by ROZ_BLOCK_SIZE below; the best N is one that is as
		    big as possible but at the same time not too big to prevent all of
		    the source bitmap pixels from fitting into cache at the same time.
		    Keep in mind that the block of source pixels used can be somewhat
		    scattered in memory.  8x8 works well on the few processors that
		    were tested; 16x16 seems to work even better for more modern
		    processors with larger caches, but since 8x8 works well enough and
		    is less likely to result in cache misses on processors with smaller
		    caches, it is used.
		*/

#define ROZ_BLOCK_SIZE 8

		UINT32 size_mask = rozInfo->size - 1;
		bitmap_ind16 &srcbitmap = tmap->pixmap();
		bitmap_ind8 &flagsbitmap = tmap->flagsmap();
		UINT32 srcx = (rozInfo->startx + (clip.min_x * rozInfo->incxx) +
			(clip.min_y * rozInfo->incyx));
		UINT32 srcy = (rozInfo->starty + (clip.min_x * rozInfo->incxy) +
			(clip.min_y * rozInfo->incyy));
		int destx = clip.min_x;
		int desty = clip.min_y;

		int row_count = (clip.max_y - desty) + 1;
		int row_block_count = row_count / ROZ_BLOCK_SIZE;
		int row_extra_count = row_count % ROZ_BLOCK_SIZE;

		int column_count = (clip.max_x - destx) + 1;
		int column_block_count = column_count / ROZ_BLOCK_SIZE;
		int column_extra_count = column_count % ROZ_BLOCK_SIZE;

		int row_block_size_incxx = ROZ_BLOCK_SIZE * rozInfo->incxx;
		int row_block_size_incxy = ROZ_BLOCK_SIZE * rozInfo->incxy;
		int row_block_size_incyx = ROZ_BLOCK_SIZE * rozInfo->incyx;
		int row_block_size_incyy = ROZ_BLOCK_SIZE * rozInfo->incyy;

		int i,j;

		// Do the block rows
		for (i = 0; i < row_block_count; i++)
		{
			int sx = srcx;
			int sy = srcy;
			int dx = destx;
			// Do the block columns
			for (j = 0; j < column_block_count; j++)
			{
				draw_roz_helper_block(rozInfo, dx, desty, sx, sy, ROZ_BLOCK_SIZE,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
				// Increment to the next block column
				sx += row_block_size_incxx;
				sy += row_block_size_incxy;
				dx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				draw_roz_helper_block(rozInfo, dx, desty, sx, sy, column_extra_count,
					ROZ_BLOCK_SIZE, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
			// Increment to the next row block
			srcx += row_block_size_incyx;
			srcy += row_block_size_incyy;
			desty += ROZ_BLOCK_SIZE;
		}
		// Do the extra rows
		if (row_extra_count)
		{
			// Do the block columns
			for (i = 0; i < column_block_count; i++)
			{
				draw_roz_helper_block(rozInfo, destx, desty, srcx, srcy, ROZ_BLOCK_SIZE,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
				srcx += row_block_size_incxx;
				srcy += row_block_size_incxy;
				destx += ROZ_BLOCK_SIZE;
			}
			// Do the extra columns
			if (column_extra_count)
			{
				draw_roz_helper_block(rozInfo, destx, desty, srcx, srcy, column_extra_count,
					row_extra_count, bitmap, flagsbitmap, srcbitmap, size_mask);
			}
		}
	}
	else
	{
		tmap->draw_roz(screen,
			bitmap, clip,
			rozInfo->startx, rozInfo->starty,
			rozInfo->incxx, rozInfo->incxy,
			rozInfo->incyx, rozInfo->incyy,
			rozInfo->wrap,0,0); // wrap, flags, pri
	}
}

void namcos2_state::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int xoffset = 38,yoffset = 0;
	struct roz_param rozParam;

	rozParam.color = (m_gfx_ctrl & 0x0f00);
	rozParam.incxx  = (INT16)m_roz_ctrl[0];
	rozParam.incxy  = (INT16)m_roz_ctrl[1];
	rozParam.incyx  = (INT16)m_roz_ctrl[2];
	rozParam.incyy  = (INT16)m_roz_ctrl[3];
	rozParam.startx = (INT16)m_roz_ctrl[4];
	rozParam.starty = (INT16)m_roz_ctrl[5];
	rozParam.size = 2048;
	rozParam.wrap = 1;


	switch( m_roz_ctrl[7] )
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

	draw_roz_helper( screen, bitmap, m_tilemap_roz, cliprect, &rozParam );
}

WRITE16_MEMBER( namcos2_state::rozram_word_w )
{
	COMBINE_DATA(&m_rozram[offset]);
	m_tilemap_roz->mark_tile_dirty(offset);
//      if( space.machine().input().code_pressed(KEYCODE_Q) )
//      {
//          debugger_break(space.machine());
//      }
}

/**************************************************************************/

UINT16 namcos2_state::get_palette_register( int which )
{
	const UINT16 *source = &m_paletteram[0x3000/2];
	return ((source[which*2]&0xff)<<8) | (source[which*2+1]&0xff);
}

READ16_MEMBER( namcos2_state::paletteram_word_r )
{
	if( (offset&0x1800) == 0x1800 )
	{
		/* palette register */
		offset &= 0x180f;

		/* registers 6,7: unmapped? */
		if (offset > 0x180b) return 0xff;
	}
	return m_paletteram[offset];
}

WRITE16_MEMBER( namcos2_state::paletteram_word_w )
{
	if( (offset&0x1800) == 0x1800 )
	{
		/* palette register */
		offset &= 0x180f;

		if( ACCESSING_BITS_0_7 ) data&=0xff;
		else data>>=8;

		switch (offset) {
			/* registers 0-3: clipping */

			/* register 4: ? */
			/* sets using it:
			assault:    $0020
			burnforc:   $0130 after titlescreen
			dirtfoxj:   $0108 at game start
			finalap1/2/3:   $00C0
			finehour:   $0168 after titlescreen
			fourtrax:   $00E8 and $00F0
			luckywld:   $00E8 at titlescreen, $00A0 in game and $0118 if in tunnel
			suzuka8h1/2:    $00E8 and $00A0 */
			case 0x1808: case 0x1809:
				// if (data^m_paletteram[offset]) printf("%04X\n",data<<((~offset&1)<<3)|m_paletteram[offset^1]<<((offset&1)<<3));
				break;

			/* register 5: POSIRQ scanline (only 8 bits used) */
			/*case 0x180a:*/ case 0x180b:
				if (data^m_paletteram[offset]) {
					m_paletteram[offset] = data;
					adjust_posirq_timer(get_pos_irq_scanline());
				}
				break;

			/* registers 6,7: nothing? */
			default: break;
		}

		m_paletteram[offset] = data;
	}
	else
	{
		COMBINE_DATA(&m_paletteram[offset]);
	}
}


inline void
namcos2_state::update_palette()
{
	int bank;
	for( bank=0; bank<0x20; bank++ )
	{
		int pen = bank*256;
		int offset = ((pen & 0x1800) << 2) | (pen & 0x07ff);
		int i;
		for( i=0; i<256; i++ )
		{
			int r = m_paletteram[offset | 0x0000] & 0x00ff;
			int g = m_paletteram[offset | 0x0800] & 0x00ff;
			int b = m_paletteram[offset | 0x1000] & 0x00ff;
			m_palette->set_pen_color(pen++,rgb_t(r,g,b));
			offset++;
		}
	}
}

/**************************************************************************/

void namcos2_state::draw_sprite_init()
{
	/* set table for sprite color == 0x0f */
	for( int i = 0; i<16*256; i++ )
	{
		m_palette->shadow_table()[i] = i+0x2000;
	}
}

/**************************************************************************/

void namcos2_state::video_start()
{
	namco_tilemap_init(2, memregion("gfx4")->base(), TilemapCB);
	m_tilemap_roz = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos2_state::roz_tile_info), this), TILEMAP_SCAN_ROWS, 8,8,256,256);
	m_tilemap_roz->set_transparent_pen(0xff);
	draw_sprite_init();
}

void namcos2_state::apply_clip( rectangle &clip, const rectangle &cliprect )
{
	clip.min_x = get_palette_register(0) - 0x4a;
	clip.max_x = get_palette_register(1) - 0x4a - 1;
	clip.min_y = get_palette_register(2) - 0x21;
	clip.max_y = get_palette_register(3) - 0x21 - 1;
	/* intersect with master clip rectangle */
	clip &= cliprect;
}

UINT32 namcos2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	update_palette();
	bitmap.fill(m_palette->black_pen(), cliprect );
	apply_clip( clip, cliprect );

	/* HACK: enable ROZ layer only if it has priority > 0 */
	m_tilemap_roz->enable((m_gfx_ctrl & 0x7000) ? 1 : 0);

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( screen, bitmap, clip, pri/2 );

			if( ((m_gfx_ctrl & 0x7000) >> 12)==pri/2 )
			{
				draw_roz(screen, bitmap,clip);
			}
			draw_sprites(screen, bitmap, clip, pri/2, m_gfx_ctrl );
		}
	}
	return 0;
}

/**************************************************************************/

void namcos2_state::video_start_finallap()
{
	namco_tilemap_init(2,memregion("gfx4")->base(),TilemapCB);
	draw_sprite_init();
}

UINT32 namcos2_state::screen_update_finallap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	update_palette();
	bitmap.fill(m_palette->black_pen(), cliprect );
	apply_clip( clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( screen, bitmap, clip, pri/2 );
		}
		m_c45_road->draw(bitmap,clip,pri);
		draw_sprites(screen,bitmap,clip,pri,m_gfx_ctrl );
	}
	return 0;
}

/**************************************************************************/

void namcos2_state::video_start_luckywld()
{
	namco_tilemap_init(2,memregion("gfx4")->base(),TilemapCB);
	c355_obj_init( 0, 0x0, namcos2_shared_state::c355_obj_code2tile_delegate() );
	if( m_gametype==NAMCOS2_LUCKY_AND_WILD )
	{
		c169_roz_init(1, "gfx5");
	}
}

UINT32 namcos2_state::screen_update_luckywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	update_palette();
	bitmap.fill(m_palette->black_pen(), cliprect );
	apply_clip( clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( screen, bitmap, clip, pri/2 );
		}
		m_c45_road->draw(bitmap,clip,pri);
		if( m_gametype==NAMCOS2_LUCKY_AND_WILD )
		{
			c169_roz_draw(screen, bitmap, clip, pri);
		}
		c355_obj_draw(screen, bitmap, clip, pri );
	}
	return 0;
}

/**************************************************************************/

void namcos2_state::video_start_sgunner()
{
	namco_tilemap_init(2,memregion("gfx4")->base(),TilemapCB);
	c355_obj_init( 0, 0x0, namcos2_shared_state::c355_obj_code2tile_delegate() );
}

UINT32 namcos2_state::screen_update_sgunner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	update_palette();
	bitmap.fill(m_palette->black_pen(), cliprect );
	apply_clip( clip, cliprect );

	for( pri=0; pri<8; pri++ )
	{
		namco_tilemap_draw( screen, bitmap, clip, pri );
		c355_obj_draw(screen, bitmap, clip, pri );
	}
	return 0;
}


/**************************************************************************/

void namcos2_state::video_start_metlhawk()
{
	namco_tilemap_init(2,memregion("gfx4")->base(),TilemapCB);
	c169_roz_init(1, "gfx5");
}

UINT32 namcos2_state::screen_update_metlhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip;
	int pri;

	update_palette();
	bitmap.fill(m_palette->black_pen(), cliprect );
	apply_clip( clip, cliprect );

	for( pri=0; pri<16; pri++ )
	{
		if( (pri&1)==0 )
		{
			namco_tilemap_draw( screen, bitmap, clip, pri/2 );
		}
		c169_roz_draw(screen, bitmap, clip, pri);
		draw_sprites_metalhawk(screen,bitmap,clip,pri );
	}
	return 0;
}
