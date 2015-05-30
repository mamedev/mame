// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nbmj8688.h"


/* the blitter can copy data both in "direct" mode, where every byte of the source
   data is copied verbatim to video RAM *twice* (thus doubling the pixel width),
   and in "lookup" mode, where the source byte is taken 4 bits at a time and indexed
   though a lookup table.
   Video RAM directly maps to a RGB output. In the first version of the hardware
   the palette was 8-bit, then they added more video RAM to have better color
   reproduction in photos. This was done in different ways, which differ for the
   implementation and the control over pixel color in the two drawing modes.
 */
enum
{
	GFXTYPE_8BIT,           // direct mode:  8-bit; lookup table:  8-bit
	GFXTYPE_HYBRID_12BIT,   // direct mode: 12-bit; lookup table:  8-bit
	GFXTYPE_HYBRID_16BIT,   // direct mode: 16-bit; lookup table: 12-bit
	GFXTYPE_PURE_16BIT,     // direct mode: 16-bit; lookup table: 16-bit
	GFXTYPE_PURE_12BIT      // direct mode:    n/a; lookup table: 12-bit
};


/******************************************************************************


******************************************************************************/

PALETTE_INIT_MEMBER(nbmj8688_state,mbmj8688_8bit)
{
	int i;
	int bit0, bit1, bit2, r, g, b;

	/* initialize 332 RGB lookup */
	for (i = 0; i < 0x100; i++)
	{
		// xxxxxxxx_bbgggrrr
		/* red component */
		bit0 = ((i >> 0) & 0x01);
		bit1 = ((i >> 1) & 0x01);
		bit2 = ((i >> 2) & 0x01);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = ((i >> 3) & 0x01);
		bit1 = ((i >> 4) & 0x01);
		bit2 = ((i >> 5) & 0x01);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((i >> 6) & 0x01);
		bit2 = ((i >> 7) & 0x01);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

PALETTE_INIT_MEMBER(nbmj8688_state,mbmj8688_12bit)
{
	int i;
	int r, g, b;

	/* initialize 444 RGB lookup */
	for (i = 0; i < 0x1000; i++)
	{
		// high and low bytes swapped for convenience
		r = ((i & 0x07) << 1) | (((i >> 8) & 0x01) >> 0);
		g = ((i & 0x38) >> 2) | (((i >> 8) & 0x02) >> 1);
		b = ((i & 0xc0) >> 4) | (((i >> 8) & 0x0c) >> 2);

		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

PALETTE_INIT_MEMBER(nbmj8688_state,mbmj8688_16bit)
{
	int i;
	int r, g, b;

	/* initialize 655 RGB lookup */
	for (i = 0; i < 0x10000; i++)
	{
		r = (((i & 0x0700) >>  5) | ((i & 0x0007) >>  0));  // R 6bit
		g = (((i & 0x3800) >>  9) | ((i & 0x0018) >>  3));  // G 5bit
		b = (((i & 0xc000) >> 11) | ((i & 0x00e0) >>  5));  // B 5bit

		palette.set_pen_color(i, pal6bit(r), pal5bit(g), pal5bit(b));
	}
}

PALETTE_INIT_MEMBER(nbmj8688_state,mbmj8688_lcd)
{
	palette.set_pen_color(0, rgb_t(28, 123, 57));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

WRITE8_MEMBER(nbmj8688_state::clut_w)
{
	m_clut[offset] = (data ^ 0xff);
}

/******************************************************************************


******************************************************************************/

WRITE8_MEMBER(nbmj8688_state::blitter_w)
{
	switch (offset)
	{
		case 0x00:  m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 0x01:  m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:  m_blitter_destx = data; break;
		case 0x03:  m_blitter_desty = data; break;
		case 0x04:  m_blitter_sizex = data; break;
		case 0x05:  m_blitter_sizey = data;
					/* writing here also starts the blit */
					gfxdraw(m_gfxmode);
					break;
		case 0x06:  m_blitter_direction_x = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y = (data & 0x02) ? 1 : 0;
					m_flipscreen = (data & 0x04) ? 0 : 1;
					m_dispflag = (data & 0x08) ? 0 : 1;
					vramflip();
					break;
		case 0x07:  break;
	}
}

WRITE8_MEMBER(nbmj8688_state::mjsikaku_gfxflag2_w)
{
	m_gfxflag2 = data;

	if (m_nb1413m3->m_nb1413m3_type == NB1413M3_SEIHAM
			|| m_nb1413m3->m_nb1413m3_type == NB1413M3_KORINAI
			|| m_nb1413m3->m_nb1413m3_type == NB1413M3_KORINAIM
			|| m_nb1413m3->m_nb1413m3_type == NB1413M3_LIVEGAL)
		m_gfxflag2 ^= 0x20;

	if (m_nb1413m3->m_nb1413m3_type == NB1413M3_OJOUSANM
			|| m_nb1413m3->m_nb1413m3_type == NB1413M3_RYUUHA)
		m_gfxflag2 |= 0x20;
}

WRITE8_MEMBER(nbmj8688_state::mjsikaku_gfxflag3_w)
{
	m_gfxflag3 = (data & 0xe0);
}

WRITE8_MEMBER(nbmj8688_state::scrolly_w)
{
	m_scrolly = data;
}

WRITE8_MEMBER(nbmj8688_state::mjsikaku_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = (data & 0x0f);

	if ((m_gfxrom << 17) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

WRITE8_MEMBER(nbmj8688_state::secolove_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = ((data & 0xc0) >> 4) + (data & 0x03);
	mjsikaku_gfxflag2_w(space, 0, data);

	if ((m_gfxrom << 17) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

WRITE8_MEMBER(nbmj8688_state::crystalg_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = (data & 0x03);
	mjsikaku_gfxflag2_w(space, 0, data);

	if ((m_gfxrom << 17) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

WRITE8_MEMBER(nbmj8688_state::seiha_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = (data & 0x1f);
	mjsikaku_gfxflag3_w(space, 0, data);

	if ((m_gfxrom << 17) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
void nbmj8688_state::vramflip()
{
	int x, y;
	UINT16 color1, color2;

	if (m_flipscreen == m_flipscreen_old) return;

	for (y = 0; y < (256 / 2); y++)
	{
		for (x = 0; x < 512; x++)
		{
			color1 = m_videoram[(y * 512) + x];
			color2 = m_videoram[((y ^ 0xff) * 512) + (x ^ 0x1ff)];
			m_videoram[(y * 512) + x] = color2;
			m_videoram[((y ^ 0xff) * 512) + (x ^ 0x1ff)] = color1;
		}
	}

	m_flipscreen_old = m_flipscreen;
	m_screen_refresh = 1;
}


void nbmj8688_state::update_pixel(int x, int y)
{
	int color = m_videoram[(y * 512) + x];
	m_tmpbitmap->pix16(y, x) = color;
}

void nbmj8688_state::writeram_low(int x, int y, int color)
{
	m_videoram[(y * 512) + x] &= 0xff00;
	m_videoram[(y * 512) + x] |= color;
	update_pixel(x, y);
}

void nbmj8688_state::writeram_high(int x, int y, int color)
{
	m_videoram[(y * 512) + x] &= 0x00ff;
	m_videoram[(y * 512) + x] |= color << 8;
	update_pixel(x, y);
}

void nbmj8688_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER:
		m_nb1413m3->m_busyflag = 1;
		break;
	default:
		assert_always(FALSE, "Unknown id in nbmj8688_state::device_timer");
	}
}

void nbmj8688_state::gfxdraw(int gfxtype)
{
	UINT8 *GFX = memregion("gfx1")->base();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	int gfxaddr, gfxlen;
	UINT16 color, color1, color2;

	if (gfxtype == GFXTYPE_PURE_12BIT)
	{
		if (m_gfxflag2 & 0x20) return;
	}

	m_nb1413m3->m_busyctr = 0;

	startx = m_blitter_destx + m_blitter_sizex;
	starty = m_blitter_desty + m_blitter_sizey;

	if (m_blitter_direction_x)
	{
		sizex = m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = m_blitter_sizex;
		skipx = -1;
	}

	if (m_blitter_direction_y)
	{
		sizey = m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		sizey = m_blitter_sizey;
		skipy = -1;
	}

	gfxlen = memregion("gfx1")->bytes();
	gfxaddr = (m_gfxrom << 17) + (m_blitter_src_addr << 1);

//popmessage("ADDR:%08X DX:%03d DY:%03d SX:%03d SY:%03d", gfxaddr, startx, starty, sizex, sizey);
//logerror("ADDR:%08X DX:%03d DY:%03d SX:%03d SY:%03d\n", gfxaddr, startx, starty, sizex, sizey);
//if (m_blitter_direction_x|m_blitter_direction_y) popmessage("ADDR:%08X FX:%01d FY:%01d", gfxaddr, m_blitter_direction_x, m_blitter_direction_y);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
//#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
//#endif
				gfxaddr = 0;
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;
			dy = (y + m_scrolly) & 0xff;

			if (m_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy ^= 0xff;
			}

			if (gfxtype == GFXTYPE_HYBRID_16BIT)
			{
				if (m_gfxflag3 & 0x40)
				{
					// direct mode

					if (m_gfxflag3 & 0x80)
					{
						/* least significant bits */
						if (color != 0xff)
						{
							writeram_low(dx1, dy, color);
							writeram_low(dx2, dy, color);
						}
					}
					else
					{
						/* most significant bits */
						if (color != 0xff)
						{
							writeram_high(dx1, dy, color);
							writeram_high(dx2, dy, color);
						}
					}
				}
				else
				{
					/* 16-bit palette with 4-to-12 bit lookup (!) */
					// lookup table mode

					// unknown flag (seiha, seiham)
				//  if (m_gfxflag3 & 0x80) return;

					// unknown (seiha, seiham, iemoto, ojousan)
					if (!(m_gfxflag2 & 0x20)) return;

					if (m_blitter_direction_x)
					{
						// flip
						color1 = (color & 0x0f) >> 0;
						color2 = (color & 0xf0) >> 4;
					}
					else
					{
						// normal
						color1 = (color & 0xf0) >> 4;
						color2 = (color & 0x0f) >> 0;
					}

					color1 = (m_clut[color1] << 8) | ((m_clut[color1 | 0x10] & 0x0f) << 4);
					color2 = (m_clut[color2] << 8) | ((m_clut[color2 | 0x10] & 0x0f) << 4);

					if (color1 != 0xfff0)
					{
						/* extend color from 12-bit to 16-bit */
						color1 = (color1 & 0xffc0) | ((color1 & 0x20) >> 1) | ((color1 & 0x10) >> 2);
						m_videoram[(dy * 512) + dx1] = color1;
						update_pixel(dx1, dy);
					}

					if (color2 != 0xfff0)
					{
						/* extend color from 12-bit to 16-bit */
						color2 = (color2 & 0xffc0) | ((color2 & 0x20) >> 1) | ((color2 & 0x10) >> 2);
						m_videoram[(dy * 512) + dx2] = color2;
						update_pixel(dx2, dy);
					}
				}
			}
			else if (gfxtype == GFXTYPE_PURE_12BIT)
			{
				/* 12-bit palette with 4-to-12 bit lookup table */

				if (m_blitter_direction_x)
				{
					// flip
					color1 = (color & 0x0f) >> 0;
					color2 = (color & 0xf0) >> 4;
				}
				else
				{
					// normal
					color1 = (color & 0xf0) >> 4;
					color2 = (color & 0x0f) >> 0;
				}

				color1 = m_clut[color1] | ((m_clut[color1 | 0x10] & 0x0f) << 8);
				color2 = m_clut[color2] | ((m_clut[color2 | 0x10] & 0x0f) << 8);

				if (color1 != 0x0fff)
				{
					m_videoram[(dy * 512) + dx1] = color1;
					update_pixel(dx1, dy);
				}
				if (color2 != 0x0fff)
				{
					m_videoram[(dy * 512) + dx2] = color2;
					update_pixel(dx2, dy);
				}
			}
			else
			{
				if (gfxtype == GFXTYPE_HYBRID_12BIT && (m_gfxflag2 & 0x20))
				{
					/* 4096 colors mode, wedged in on top of normal mode
					   Here we affect only the 4 least significant bits, the others are
					   changed as usual.
					 */

					if (m_gfxflag2 & 0x10)
					{
						// 4096 colors low mode (2nd draw upper)
						color = m_clut[((color & 0xf0) >> 4)];
					}
					else
					{
						// 4096 colors low mode (1st draw lower)
						color = m_clut[((color & 0x0f) >> 0)];
					}

					if (color != 0xff)
					{
						color &= 0x0f;
						writeram_high(dx1, dy, color);
						writeram_high(dx2, dy, color);
					}
				}
				else
				{
					if (m_gfxflag2 & 0x04)
					{
						// direct mode

						color1 = color2 = color;
					}
					else
					{
						// lookup table mode

						if (m_blitter_direction_x)
						{
							// flip
							color1 = (color & 0x0f) >> 0;
							color2 = (color & 0xf0) >> 4;
						}
						else
						{
							// normal
							color1 = (color & 0xf0) >> 4;
							color2 = (color & 0x0f) >> 0;
						}

						color1 = m_clut[color1];
						color2 = m_clut[color2];
					}

					if (gfxtype == GFXTYPE_PURE_16BIT && !(m_gfxflag2 & 0x20))
					{
						/* 16-bit palette most significant bits */
						if (color1 != 0xff) writeram_high(dx1, dy, color1);
						if (color2 != 0xff) writeram_high(dx2, dy, color2);
					}
					else
					{
						/* 8-bit palette or 16-bit palette least significant bits */
						if (color1 != 0xff) writeram_low(dx1, dy, color1);
						if (color2 != 0xff) writeram_low(dx2, dy, color2);
					}
				}
			}

			m_nb1413m3->m_busyctr++;
		}
	}

	m_nb1413m3->m_busyflag = 0;

	if (gfxtype == GFXTYPE_8BIT)
		m_blitter_timer->adjust(attotime::from_hz(400000) * m_nb1413m3->m_busyctr);
	else
		m_blitter_timer->adjust(attotime::from_hz(400000) * m_nb1413m3->m_busyctr);
}


/******************************************************************************


******************************************************************************/

void nbmj8688_state::common_video_start()
{
	m_blitter_timer = timer_alloc(TIMER_BLITTER);

	m_tmpbitmap = auto_bitmap_ind16_alloc(machine(), 512, 256);
	m_videoram = auto_alloc_array_clear(machine(), UINT16, 512 * 256);
	m_clut = auto_alloc_array(machine(), UINT8, 0x20);

	m_scrolly = 0;  // reset because crystalg/crystal2 don't write to this register
	m_screen_refresh = 1;

	save_pointer(NAME(m_videoram), 512 * 256);
	save_pointer(NAME(m_clut), 0x20);
	save_item(NAME(m_scrolly));
	save_item(NAME(m_blitter_destx));
	save_item(NAME(m_blitter_desty));
	save_item(NAME(m_blitter_sizex));
	save_item(NAME(m_blitter_sizey));
	save_item(NAME(m_blitter_direction_x));
	save_item(NAME(m_blitter_direction_y));
	save_item(NAME(m_blitter_src_addr));
	save_item(NAME(m_gfxrom));
	save_item(NAME(m_dispflag));
	save_item(NAME(m_gfxflag2));
	save_item(NAME(m_gfxflag3));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_flipscreen_old));

	machine().save().register_postload(save_prepost_delegate(FUNC(nbmj8688_state::postload), this));
}

void nbmj8688_state::postload()
{
	m_screen_refresh = 1;
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_8bit)
{
	m_gfxmode = GFXTYPE_8BIT;
	common_video_start();
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_hybrid_12bit)
{
	m_gfxmode = GFXTYPE_HYBRID_12BIT;
	common_video_start();
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_pure_12bit)
{
	m_gfxmode = GFXTYPE_PURE_12BIT;
	common_video_start();
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_hybrid_16bit)
{
	m_gfxmode = GFXTYPE_HYBRID_16BIT;
	common_video_start();
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_pure_16bit)
{
	m_gfxmode = GFXTYPE_PURE_16BIT;
	common_video_start();
}

VIDEO_START_MEMBER(nbmj8688_state,mbmj8688_pure_16bit_LCD)
{
	m_gfxmode = GFXTYPE_PURE_16BIT;
	common_video_start();
}

WRITE8_MEMBER(nbmj8688_state::HD61830B_both_instr_w)
{
	m_lcdc0->control_w(space, offset, data);
	m_lcdc1->control_w(space, offset, data);
}

WRITE8_MEMBER(nbmj8688_state::HD61830B_both_data_w)
{
	m_lcdc0->data_w(space, offset, data);
	m_lcdc1->data_w(space, offset, data);
}



/******************************************************************************


******************************************************************************/


UINT32 nbmj8688_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

if(machine().input().code_pressed_once(KEYCODE_T))
{
	//
}

	if (m_screen_refresh)
	{
		m_screen_refresh = 0;
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 512; x++)
			{
				update_pixel(x, y);
			}
		}
	}

//  if (m_dispflag)
	{
		int scrolly;
		if (m_flipscreen) scrolly =   m_scrolly;
		else                     scrolly = (-m_scrolly) & 0xff;

		copybitmap(bitmap, *m_tmpbitmap, 0, 0, 0, scrolly,       cliprect);
		copybitmap(bitmap, *m_tmpbitmap, 0, 0, 0, scrolly - 256, cliprect);
	}
//  else
//      bitmap.fill(0);

	return 0;
}
