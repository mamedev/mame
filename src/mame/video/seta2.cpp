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
                -e-- ---- ---- ----     ?
                --d- ---- ---- ----     Opaque
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

        0.w     fedc ba-- ---- ----     Number of columns (local)
                ---- --98 7654 3210     X

        2.w     fedc ba-- ---- ----     Number of rows - 1 (local)
                ---- --98 7654 3210     Y

        4.w     f--- ---- ---- ----     Tile size: 8x8 (0) or 16x16 (1)
                -edc ba-- ---- ----     "Tilemap" page (0x2000 bytes each)
                ---- --98 7654 3210     "Tilemap" scroll X

        6.w     fedc ba9- ---- ----
                ---- ---8 7654 3210     "Tilemap" scroll Y


    Shadows (same principle as ssv.cpp):

    The low bits of the pens from a "shadowing" tile (regardless of color code)
    substitute the top bits of the color index (0-7fff) in the frame buffer.

***************************************************************************/

#include "emu.h"
#include "includes/seta2.h"

/***************************************************************************

                                Video Registers

    Offset:     Bits:                   Value:

    0/2/4/6                             Horizontal: Sync, Blank, DSPdot, Cycle (same as ssv.c?)
    8/a/c/e                             Vertical  : Sync, Blank, DSPdot, Cycle (same as ssv.c?)

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

	uint16_t olddata = m_vregs[offset];

	COMBINE_DATA(&m_vregs[offset]);
	if ( m_vregs[offset] != olddata )
		logerror("CPU #0 PC %06X: Video Reg %02X <- %04X\n",m_maincpu->pc(),offset*2,data);

	switch( offset*2 )
	{
	case 0x1c:  // FLIP SCREEN (myangel)    <- this is actually zoom
		flip_screen_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flip screen unknown bits %04X\n",m_maincpu->pc(),data);
		break;
	case 0x2a:  // FLIP X (pzlbowl)
		flip_screen_x_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flipx unknown bits %04X\n",m_maincpu->pc(),data);
		break;
	case 0x2c:  // FLIP Y (pzlbowl)
		flip_screen_y_set(data & 1 );
		if (data & ~1)  logerror("CPU #0 PC %06X: flipy unknown bits %04X\n",m_maincpu->pc(),data);
		break;

	case 0x30:  // BLANK SCREEN (pzlbowl, myangel)
		if (data & ~1)  logerror("CPU #0 PC %06X: blank unknown bits %04X\n",m_maincpu->pc(),data);
		break;
	}
}


/***************************************************************************


                                Sprites Drawing


***************************************************************************/


inline void seta2_state::drawgfx_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int which_gfx, const uint8_t* const addr, const uint32_t realcolor, int flipx, int flipy, int base_sx, int use_shadow, int realline, int line, int opaque)
{
	const uint8_t* const source = flipy ? addr + (7 - line) * 8 : addr + line * 8;

	// Color depth
	int shadow = 0;
	int gfx_mask = 0xff;
	int gfx_shift = 0;

	switch (which_gfx & 0x0700)
	{
	case 0x0700:            // 8bpp tiles (76543210)
		shadow = 8;   // ?
		gfx_mask = 0xff;
		gfx_shift = 0;
		break;
	case 0x0600:            // 6bpp tiles (--543210) (myangel sliding blocks test)
		shadow = 5;   // staraudi
		gfx_mask = 0x3f;
		gfx_shift = 0;
		break;
	case 0x0500:            // 4bpp tiles (3210----)
		shadow = 4;   // ?
		gfx_mask = 0xf0;
		gfx_shift = 4;
		break;
	case 0x0400:            // 4bpp tiles (----3210)
		shadow = 3;   // reelquak
		gfx_mask = 0x0f;
		gfx_shift = 0;
		break;
	case 0x0300:            // ??? (staraudi question bubble: pen %00011000 with shadow on!)
		gfx_mask = 0xff; // unknown
		gfx_shift = 0;
		break;
	case 0x0200:            // 3bpp tiles?  (-----210) (myangel "Graduate Tests")
		shadow = 3;   // ?
		gfx_mask = 0x07;
		gfx_shift = 0;
		break;
	case 0x0100:            // 2bpp tiles??? (--10----) (myangel2 question bubble, myangel endgame)
		shadow = 2;   // myangel2
		gfx_mask = 0x30;
		gfx_shift = 4;
		break;
	case 0x0000:            // no idea!
		shadow = 4;   // ?
		gfx_mask = 0xff;
		gfx_shift = 0;
		break;
	}
	if (!use_shadow)
		shadow = 0;


	uint16_t* dest = &bitmap.pix16(realline);

	const int x0 = flipx ? (base_sx + 8 - 1) : (base_sx);
	const int x1 = flipx ? (base_sx - 1) : (x0 + 8);
	const int dx = flipx ? (-1) : (1);

	int column = 0;
	for (int sx = x0; sx != x1; sx += dx)
	{
		uint8_t pen = (source[column++] & gfx_mask) >> gfx_shift;

		if (sx >= cliprect.min_x && sx <= cliprect.max_x)
		{
			if (pen || opaque)
			{
				if (!shadow)
				{
					dest[sx] = (realcolor + pen) & 0x7fff;
				}
				else
				{
					int pen_shift = 15 - shadow;
					int pen_mask = (1 << pen_shift) - 1;
					dest[sx] = ((dest[sx] & pen_mask) | (pen << pen_shift)) & 0x7fff;
				}					
			}
		}
	}
}

inline void seta2_state::get_tile(uint16_t* spriteram, int is_16x16, int x, int y, int page, int& code, int& attr, int& flipx, int& flipy, int& color)
{
	uint16_t *s3 = &spriteram[2 * ((page * 0x2000 / 4) + ((y & 0x1f) << 6) + (x & 0x03f))];
	attr = s3[0];
	code = s3[1] + ((attr & 0x0007) << 16);
	flipx = (attr & 0x0010);
	flipy = (attr & 0x0008);
	color = (attr & 0xffe0) >> 5;
	if (is_16x16)
		code &= ~3;
}

void seta2_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Sprites list

	// When debugging, use m_spriteram here, and run mame -update_in_pause, i.e.:
//  uint16_t *buffered_spriteram16 = m_spriteram;
	uint16_t *buffered_spriteram16 = m_buffered_spriteram.get();
	uint16_t *s1 = buffered_spriteram16 + 0x3000 / 2;
	uint16_t *end = &buffered_spriteram16[m_spriteram.bytes() / 2];

	//  for ( ; s1 < end; s1+=4 )
	for (; s1 < buffered_spriteram16 + 0x4000 / 2; s1 += 4)   // more reasonable (and it cures MAME lockup in e.g. funcube3 boot)
	{
		int num = s1[0];
		int xoffs = s1[1];
		int yoffs = s1[2];
		int sprite = s1[3];

		// Single-sprite address
		uint16_t *s2 = &buffered_spriteram16[(sprite & 0x7fff) * 4];

		// Single-sprite size
		int global_sizex = xoffs & 0xfc00;
		int global_sizey = yoffs & 0xfc00;

		bool opaque = num & 0x2000;
		int use_global_size = num & 0x1000;
		int use_shadow = num & 0x0800;
		int which_gfx = num & 0x0700;
		xoffs &= 0x3ff;
		yoffs &= 0x3ff;

		// Number of single-sprites
		num = (num & 0x00ff) + 1;

		for (; num > 0; num--, s2 += 4)
		{
			if (s2 >= end)  break;

			if (sprite & 0x8000)
			{
				// "floating tilemap" sprite
				// the 'floating tilemap sprites' are just a window into the tilemap, the position of the sprite does not change the scroll values

				int sx = s2[0];
				int sy = s2[1];
				int scrollx = s2[2];
				int scrolly = s2[3];
				int is_16x16 = (scrollx & 0x8000) >> 15;
				int page = (scrollx & 0x7c00) >> 10;
				int local_sizex = sx & 0xfc00;
				int local_sizey = sy & 0xfc00;
				sx &= 0x3ff;
				sy &= 0x1ff;

				int width = use_global_size ? global_sizex : local_sizex;
				int height = use_global_size ? global_sizey : local_sizey;

				height = ((height & 0xfc00) >> 10) + 1;
				width = ((width & 0xfc00) >> 10)/* + 1*/; // reelquak reels
				if (!width)
					continue;

				scrollx += m_xoffset;
				scrollx &= 0x3ff;
				scrolly &= 0x1ff;

				rectangle clip;
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

				if (!is_16x16)
					continue;

				for (int realline = clip.min_y; realline < clip.max_y; realline++)
				{
					int sourceline = (realline - scrolly - m_yoffset) & 0x1ff;

					int y = sourceline >> (is_16x16 ? 4 : 3);

					for (int x = 0; x < 0x40; x++)
					{
						int code, attr, flipx, flipy, color;
						get_tile(buffered_spriteram16, is_16x16, x, y ^ 0x1f, page, code, attr, flipx, flipy, color); // yes the tilemap in RAM is flipped?!

						int line = is_16x16 ? (sourceline & 0x1f) : (sourceline & 0x0f);

						int ty = (line >> 3) & 1;
						line &= 0x7;
						for (int tx = 0; tx <= is_16x16; tx++)
						{
							int dx = sx + (scrollx & 0x3ff) + xoffs + 0x10;
							int px = ((dx + x * (8 << is_16x16) + 0x10) & 0x3ff) - 0x10;
							int dst_x = (px + (flipx ? is_16x16 - tx : tx) * 8) & 0x3ff;
							dst_x = (dst_x & 0x1ff) - (dst_x & 0x200);

							if ((dst_x >= cliprect.min_x - 8) && (dst_x <= cliprect.max_x))
							{
								int realcode = code ^ tx ^ ((flipy ? is_16x16 - ty : ty) << 1);
								drawgfx_line(bitmap, cliprect, which_gfx, m_spritegfx->get_data(m_realtilenumber[realcode]), color << 4, flipx, flipy, dst_x, use_shadow, realline, line, opaque);
							}
						}
					}
				}
			}
			else
			{
				// "normal" sprite
				int sx = s2[0];
				int sy = s2[1];
				int attr = s2[2];
				int code = s2[3] + ((attr & 0x0007) << 16);
				int flipx = (attr & 0x0010);
				int flipy = (attr & 0x0008);
				int color = (attr & 0xffe0) >> 5;

				int sizex = use_global_size ? global_sizex : sx;
				int sizey = use_global_size ? global_sizey : sy;
				sizex = (1 << ((sizex & 0x0c00) >> 10)) - 1;
				sizey = (1 << ((sizey & 0x0c00) >> 10)) - 1;

				sx += xoffs;
				sy += yoffs;

				sx = (sx & 0x1ff) - (sx & 0x200);

				sy &= 0x1ff;
				sy -= m_yoffset;

				int basecode = code &= ~((sizex + 1) * (sizey + 1) - 1);   // see myangel, myangel2 and grdians

				int firstline = sy;
				int endline = sy + (sizey + 1) * 8;

				//for (int realline = cliprect.min_y; realline < cliprect.max_y; realline++)
				for (int realline = firstline; realline < endline; realline++)
				{
					int line = realline - firstline;
					int y = (line >> 3);
					line &= 0x7;

					for (int x = 0; x <= sizex; x++)
					{
						int realcode = (basecode + (flipy ? sizey - y : y)*(sizex + 1)) + (flipx ? sizex - x : x);
						drawgfx_line(bitmap, cliprect, which_gfx, m_spritegfx->get_data(m_realtilenumber[realcode]), color << 4, flipx, flipy, sx + x * 8, use_shadow, realline, line, opaque);
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
	for (int i = 0; m_gfxdecode->gfx(i); ++i)
		m_gfxdecode->gfx(i)->set_granularity(16);

	m_buffered_spriteram = std::make_unique<uint16_t[]>(m_spriteram.bytes()/2);

	m_xoffset = 0;
	m_yoffset = 0;


	m_spritegfx = m_gfxdecode->gfx(0);
	for (int i = 0; i < 0x80000; i++)
	{
		m_realtilenumber[i] = i % m_spritegfx->elements();
	}
}

VIDEO_START_MEMBER(seta2_state,xoffset)
{
	video_start();

	m_xoffset = 0x200;
}

VIDEO_START_MEMBER(seta2_state,xoffset1)
{
	video_start();

	m_xoffset = 0x1;
}

VIDEO_START_MEMBER(seta2_state,yoffset)
{
	video_start();

	m_yoffset = 0x10;
}

uint32_t seta2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Black or pen 0?
	bitmap.fill(m_palette->pen(0), cliprect);

	if (m_vregs.found())
	{
		if ( (m_vregs[0x30/2] & 1) == 0 )   // 1 = BLANK SCREEN
			draw_sprites(bitmap, cliprect);
	}
	else // ablastb doesn't seem to have the same vregs
	{
		draw_sprites(bitmap, cliprect);
	}

	return 0;
}

WRITE_LINE_MEMBER(seta2_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		// Buffer sprites by 1 frame
		memcpy(m_buffered_spriteram.get(), m_spriteram, m_spriteram.bytes());
	}
}

// staraudi
void staraudi_state::draw_rgbram(bitmap_ind16 &bitmap)
{
	if (!(m_cam & 0x0008))
		return;

	for (int y = 0x100; y < 0x200; ++y)
	{
		for (int x = 0; x < 0x200; ++x)
		{
			int offs = x * 2/2 + y * 0x400/2;
			uint32_t data = ((m_rgbram[offs + 0x40000/2] & 0xff) << 16) | m_rgbram[offs];
			bitmap.pix16(y, x) = (data & 0x7fff);
		}
	}
}
uint32_t staraudi_state::staraudi_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update(screen, bitmap, cliprect);
	if (false)
		draw_rgbram(bitmap);

	return 0;
}
