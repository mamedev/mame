// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


    This hardware only generates sprites. But they're of various types,
    including some large "floating tilemap" ones.

    Sprites RAM is 0x40000 bytes long. All games write the sprites list
    at offset 0x3000. Each entry in the list holds data for a multi-sprite
    of up to 256 single-sprites. The list looks like this:

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Last sprite
                -ed- ---- ---- ----     ?
                ---c ---- ---- ----     0 = Each sprite specifies its size, 1 = Use the global size (following words)
                ---- b--- ---- ----     Shadow
                ---- -a98 ---- ----     Tile color depth
                ---- ---- 7654 3210     Number of sprites - 1

        2.w     fedc ba-- ---- ----     X global size
                ---- --98 7654 3210     X displacement

        4.w     fedc ba-- ---- ----     Y global size
                ---- --98 7654 3210     Y displacement

        6.w     f--- ---- ---- ----     Single-sprite(s) type: tile (0) or row of tiles (1)
                -edc ba98 7654 3210     Offset of the single-sprite(s) data (8 bytes each)


    A single-sprite can be a tile or some horizontal rows of tiles.

    Tile case:

        0.w     fedc ---- ---- ----
                ---- ba-- ---- ----     Number of tiles along X (1 << n)
                ---- --98 7654 3210     X

        2.w     fedc ---- ---- ----
                ---- ba-- ---- ----     Number of tiles along Y (1 << n)
                ---- --98 7654 3210     Y

        4.w     fedc ba98 765- ----     Color code (16 color steps)
                ---- ---- ---4 ----     Flip X
                ---- ---- ---- 3---     Flip Y
                ---- ---- ---- -210     Code (high bits)

        6.w                             Code (low bits)


    Row case:

        0.w     fedc ba-- ---- ----     Number of columns
                ---- --98 7654 3210     X

        2.w     fedc ba-- ---- ----     Number of rows - 1
                ---- --98 7654 3210     Y

        4.w     f--- ---- ---- ----     Tile size: 8x8 (0) or 16x16 (1)
                -edc ba-- ---- ----     "Tilemap" page (0x2000 bytes each)
                ---- --98 7654 3210     "Tilemap" scroll X

        6.w     fedc ba9- ---- ----
                ---- ---8 7654 3210     "Tilemap" scroll Y


    Shadows (same principle as ssv.c):

    The low bits of the pens from a "shadowing" tile (regardless of color code)
    substitute the top bits of the color index (0-7fff) in the frame buffer.

***************************************************************************/

#include "emu.h"
#include "includes/seta2.h"

/***************************************************************************

                                Video Registers

    Offset:     Bits:                   Value:

    0/2/4/6                             ? Horizontal (same as ssv.c?)
    8/a/c/e                             ? Vertical   (same as ssv.c?)

    10
    12                                  Offset X?
    14                                  Zoom X? low bits
    16                                  Zoom X? high bits *

    18
    1a                                  Offset Y?
    1c                                  Zoom Y? low bits
    1e                                  Zoom Y? high bits *

    26                                  1->0 during INT0, before writing sprites

    30          fedc ba98 7654 321-
                ---- ---- ---- ---0     Disable video

    32..3f                              ?

    * A value of 0x0100 is means no zoom, a value of 0x0200 will halve the size.
      A value less than 0x0100 probably means magnification.

***************************************************************************/

WRITE16_MEMBER(seta2_state::vregs_w)
{
	/* 02/04 = horizontal display start/end
	           mj4simai = 0065/01E5 (0180 visible area)
	           myangel =  005D/01D5 (0178 visible area)
	           pzlbowl =  0058/01D8 (0180 visible area)
	           penbros =  0065/01A5 (0140 visible area)
	           grdians =  0059/0188 (012f visible area)
	   06    = horizontal total?
	           mj4simai = 0204
	           myangel =  0200
	           pzlbowl =  0204
	           penbros =  01c0
	           grdians =  019a
	*/

	UINT16 olddata = m_vregs[offset];

	COMBINE_DATA(&m_vregs[offset]);
	if ( m_vregs[offset] != olddata )
		logerror("CPU #0 PC %06X: Video Reg %02X <- %04X\n",space.device().safe_pc(),offset*2,data);

	switch( offset*2 )
	{
	case 0x1c:  // FLIP SCREEN (myangel)    <- this is actually zoom
		flip_screen_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flip screen unknown bits %04X\n",space.device().safe_pc(),data);
		break;
	case 0x2a:  // FLIP X (pzlbowl)
		flip_screen_x_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flipx unknown bits %04X\n",space.device().safe_pc(),data);
		break;
	case 0x2c:  // FLIP Y (pzlbowl)
		flip_screen_y_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flipy unknown bits %04X\n",space.device().safe_pc(),data);
		break;

	case 0x30:  // BLANK SCREEN (pzlbowl, myangel)
		if (data & ~1)  logerror("CPU #0 PC %06X: blank unknown bits %04X\n",space.device().safe_pc(),data);
		break;
	}
}


/***************************************************************************


                                Sprites Drawing


***************************************************************************/

static void seta_drawgfx(   bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx,
							UINT32 code,UINT32 color,int flipx,int flipy,int x0,int y0,
							int shadow_depth )
{
	const UINT8 *addr, *source;
	UINT8 pen;
	UINT16 *dest;
	int sx, x1, dx;
	int sy, y1, dy;

	addr    =   gfx->get_data(code  % gfx->elements());
	color   =   gfx->granularity() * (color % gfx->colors());

	if ( flipx )    {   x1 = x0-1;              x0 += gfx->width()-1;       dx = -1;    }
	else            {   x1 = x0 + gfx->width();                         dx =  1;    }

	if ( flipy )    {   y1 = y0-1;              y0 += gfx->height()-1;  dy = -1;    }
	else            {   y1 = y0 + gfx->height();                            dy =  1;    }

#define SETA_DRAWGFX(SETPIXELCOLOR)                                             \
	for ( sy = y0; sy != y1; sy += dy )                                         \
	{                                                                           \
		if ( sy >= cliprect.min_y && sy <= cliprect.max_y )                 \
		{                                                                       \
			source  =   addr;                                                   \
			dest    =   &bitmap.pix16(sy);                          \
																				\
			for ( sx = x0; sx != x1; sx += dx )                                 \
			{                                                                   \
				pen = *source++;                                                \
																				\
				if ( pen && sx >= cliprect.min_x && sx <= cliprect.max_x )  \
					SETPIXELCOLOR                                               \
			}                                                                   \
		}                                                                       \
																				\
		addr    +=  gfx->rowbytes();                                            \
	}

	if (shadow_depth)
	{
		int pen_shift = 15 - shadow_depth;
		int pen_mask  = (1 << pen_shift) - 1;
		SETA_DRAWGFX( { dest[sx] = ((dest[sx] & pen_mask) | (pen << pen_shift)) & 0x7fff; } )
	}
	else
	{
		SETA_DRAWGFX( { dest[sx] = (color + pen) & 0x7fff; } )
	}
}

void seta2_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	// Sprites list

	// When debugging, use m_spriteram here, and run mame -update_in_pause
	UINT16 *buffered_spriteram16 = m_buffered_spriteram;
	UINT16 *s1  = buffered_spriteram16 + 0x3000/2;
	UINT16 *end = &buffered_spriteram16[m_spriteram.bytes()/2];

//  for ( ; s1 < end; s1+=4 )
	for ( ; s1 < buffered_spriteram16 + 0x4000/2; s1+=4 )   // more reasonable (and it cures MAME lockup in e.g. funcube3 boot)
	{
		gfx_element *gfx;
		int num     = s1[0];
		int xoffs   = s1[1];
		int yoffs   = s1[2];
		int sprite  = s1[3];

		// Single-sprite address
		UINT16 *s2 = &buffered_spriteram16[(sprite & 0x7fff) * 4];

		// Single-sprite size
		int global_sizex = xoffs & 0xfc00;
		int global_sizey = yoffs & 0xfc00;

		int use_global_size =   num & 0x1000;
		int use_shadow      =   num & 0x0800;

		xoffs &= 0x3ff;
		yoffs &= 0x3ff;

		// Color depth
		int shadow_depth;
		switch (num & 0x0700)
		{
			default:
				popmessage("unknown gfxset %x",(num & 0x0700)>>8);
				shadow_depth = 0;
				gfx = m_gfxdecode->gfx(machine().rand()&3);
				break;
			case 0x0700:            // 8bpp tiles (76543210)
				shadow_depth = 8;   // ?
				gfx = m_gfxdecode->gfx(3);
				break;
			case 0x0600:            // 6bpp tiles (--543210) (myangel sliding blocks test)
				shadow_depth = 6;   // ?
				gfx = m_gfxdecode->gfx(2);
				break;
			case 0x0500:            // 4bpp tiles (3210----)
				shadow_depth = 4;   // ?
				gfx = m_gfxdecode->gfx(1);
				break;
			case 0x0400:            // 4bpp tiles (----3210)
				shadow_depth = 3;   // reelquak
				gfx = m_gfxdecode->gfx(0);
				break;
//          case 0x0300:
//              unknown
			case 0x0200:            // 3bpp tiles?  (-----210) (myangel "Graduate Tests")
				shadow_depth = 3;   // ?
				gfx = m_gfxdecode->gfx(4);
				break;
			case 0x0100:            // 2bpp tiles??? (--10----) (myangel2 question bubble, myangel endgame)
				shadow_depth = 2;   // myangel2
				gfx = m_gfxdecode->gfx(5);
				break;
			case 0x0000:            // no idea!
				shadow_depth = 4;   // ?
				gfx = m_gfxdecode->gfx(0);
				break;
		}
		if (!use_shadow)
			shadow_depth = 0;

		// Number of single-sprites
		num = (num & 0x00ff) + 1;

		for( ; num > 0; num--,s2+=4 )
		{
			if (s2 >= end)  break;

// "tilemap" sprite

			if (sprite & 0x8000)
			{
				rectangle clip;
				int dx, x, y;
				int flipx;
				int flipy;
				int sx       = s2[0];
				int sy       = s2[1];
				int scrollx  = s2[2];
				int scrolly  = s2[3];
				int tilesize = (scrollx & 0x8000) >> 15;
				int page     = (scrollx & 0x7c00) >> 10;

				int width    = use_global_size ? global_sizex : sx;
				int height   = use_global_size ? global_sizey : sy;
				height = ((height & 0xfc00) >> 10) + 1;
				width  = ((width  & 0xfc00) >> 10)/* + 1*/; // reelquak reels
				if (!width)
					continue;

				sx &= 0x3ff;
				sy &= 0x1ff;

				scrollx += m_xoffset;
				scrollx &= 0x3ff;
				scrolly &= 0x1ff;

				// sprite clipping region (x)
				clip.min_x = (sx + xoffs) & 0x3ff;
				clip.min_x = (clip.min_x & 0x1ff) - (clip.min_x & 0x200);
				clip.max_x = clip.min_x + width * 0x10 - 1;

				if (clip.min_x > cliprect.max_x)    continue;
				if (clip.max_x < cliprect.min_x)    continue;
				if (clip.min_x < cliprect.min_x)    clip.min_x = cliprect.min_x;
				if (clip.max_x > cliprect.max_x)    clip.max_x = cliprect.max_x;

				// sprite clipping region (y)
				clip.min_y = ((sy + yoffs) & 0x1ff) - m_yoffset;
				clip.max_y = clip.min_y + height * 0x10 - 1;

				if (clip.min_y > cliprect.max_y)    continue;
				if (clip.max_y < cliprect.min_y)    continue;
				if (clip.min_y < cliprect.min_y)    clip.min_y = cliprect.min_y;
				if (clip.max_y > cliprect.max_y)    clip.max_y = cliprect.max_y;

				dx = sx + (scrollx & 0x3ff) + xoffs + 0x10;

				// Draw the rows
				for (y = 0; y < (0x40 >> tilesize); y++)
				{
					int py = ((scrolly - (y+1) * (8 << tilesize) + 0x10) & 0x1ff) - 0x10 - m_yoffset;

					for (x = 0; x < 0x40; x++)
					{
						int px = ((dx + x * (8 << tilesize) + 0x10) & 0x3ff) - 0x10;
						int tx, ty;
						int attr, code, color;
						UINT16 *s3;

						s3  =   &buffered_spriteram16[2 * ((page * 0x2000/4) + ((y & 0x1f) << 6) + (x & 0x03f))];

						attr  = s3[0];
						code  = s3[1] + ((attr & 0x0007) << 16);
						flipx = (attr & 0x0010);
						flipy = (attr & 0x0008);
						color = (attr & 0xffe0) >> 5;

						if (tilesize) code &= ~3;

						for (ty = 0; ty <= tilesize; ty++)
						{
							for (tx = 0; tx <= tilesize; tx++)
							{
								int dst_x = (px + (flipx ? tilesize-tx : tx) * 8) & 0x3ff;
								int dst_y = (py + (flipy ? tilesize-ty : ty) * 8) & 0x1ff;

								dst_x = (dst_x & 0x1ff) - (dst_x & 0x200);

								seta_drawgfx(bitmap, clip, gfx,
										code ^ tx ^ (ty<<1),
										color,
										flipx, flipy,
										dst_x, dst_y,
										shadow_depth );
							}
						}

					}
				}
			}
			else
// "normal" sprite
			{
				int sx    = s2[0];
				int sy    = s2[1];
				int attr  = s2[2];
				int code  = s2[3] + ((attr & 0x0007) << 16);
				int flipx = (attr & 0x0010);
				int flipy = (attr & 0x0008);
				int color = (attr & 0xffe0) >> 5;

				int sizex = use_global_size ? global_sizex : sx;
				int sizey = use_global_size ? global_sizey : sy;
				int x,y;
				sizex = (1 << ((sizex & 0x0c00)>> 10))-1;
				sizey = (1 << ((sizey & 0x0c00)>> 10))-1;


				sx += xoffs;
				sy += yoffs;

				sx = (sx & 0x1ff) - (sx & 0x200);

				sy &= 0x1ff;
				sy -= m_yoffset;

				code &= ~((sizex+1) * (sizey+1) - 1);   // see myangel, myangel2 and grdians

				for (y = 0; y <= sizey; y++)
				{
					for (x = 0; x <= sizex; x++)
					{
						seta_drawgfx(bitmap, cliprect, gfx,
								code++,
								color,
								flipx, flipy,
								sx + (flipx ? sizex-x : x) * 8, sy + (flipy ? sizey-y : y) * 8,
								shadow_depth );
					}
				}
			}
		}

		if (s1[0] & 0x8000) break;  // end of list marker
	}   // sprite list
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

void seta2_state::video_start()
{
	m_gfxdecode->gfx(2)->set_granularity(16);
	m_gfxdecode->gfx(3)->set_granularity(16);
	m_gfxdecode->gfx(4)->set_granularity(16);
	m_gfxdecode->gfx(5)->set_granularity(16);

	m_buffered_spriteram = auto_alloc_array(machine(), UINT16, m_spriteram.bytes()/2);

	m_xoffset = 0;
	m_yoffset = 0;
}

VIDEO_START_MEMBER(seta2_state,xoffset)
{
	video_start();

	m_xoffset = 0x200;
}

VIDEO_START_MEMBER(seta2_state,yoffset)
{
	video_start();

	m_yoffset = 0x10;
}

UINT32 seta2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Black or pen 0?
	bitmap.fill(m_palette->pen(0), cliprect);

	if ( (m_vregs[0x30/2] & 1) == 0 )   // 1 = BLANK SCREEN
		draw_sprites(bitmap, cliprect);

	return 0;
}

void seta2_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		// Buffer sprites by 1 frame
		memcpy(m_buffered_spriteram, m_spriteram, m_spriteram.bytes());
	}
}
