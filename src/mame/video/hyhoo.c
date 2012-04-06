/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/01/28 -

******************************************************************************/

#include "emu.h"
#include "includes/nb1413m3.h"
#include "includes/hyhoo.h"


static void hyhoo_gfxdraw(running_machine &machine);


WRITE8_MEMBER(hyhoo_state::hyhoo_blitter_w)
{
	switch (offset)
	{
		case 0x00:	m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data;
					nb1413m3_gfxradr_l_w(&space, 0, data); break;
		case 0x01:	m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8);
					nb1413m3_gfxradr_h_w(&space, 0, data); break;
		case 0x02:	m_blitter_destx = data; break;
		case 0x03:	m_blitter_desty = data; break;
		case 0x04:	m_blitter_sizex = data; break;
		case 0x05:	m_blitter_sizey = data;
					/* writing here also starts the blit */
					hyhoo_gfxdraw(machine());
					break;
		case 0x06:	m_blitter_direction_x = (data >> 0) & 0x01;
					m_blitter_direction_y = (data >> 1) & 0x01;
					m_flipscreen = (~data >> 2) & 0x01;
					m_dispflag = (~data >> 3) & 0x01;
					break;
		case 0x07:	break;
	}
}


WRITE8_MEMBER(hyhoo_state::hyhoo_romsel_w)
{
	int gfxlen = machine().region("gfx1")->bytes();
	m_gfxrom = (((data & 0xc0) >> 4) + (data & 0x03));
	m_highcolorflag = data;
	nb1413m3_gfxrombank_w(&space, 0, data);

	if ((0x20000 * m_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}


static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void hyhoo_gfxdraw(running_machine &machine)
{
	hyhoo_state *state = machine.driver_data<hyhoo_state>();
	UINT8 *GFX = machine.region("gfx1")->base();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	int gfxaddr, gfxlen;
	UINT8 color, color1, color2;
	int r, g, b;
	pen_t pen;

	nb1413m3_busyctr = 0;

	state->m_gfxrom |= ((nb1413m3_sndrombank1 & 0x02) << 3);

	startx = state->m_blitter_destx + state->m_blitter_sizex;
	starty = state->m_blitter_desty + state->m_blitter_sizey;

	if (state->m_blitter_direction_x)
	{
		sizex = state->m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = state->m_blitter_sizex;
		skipx = -1;
	}

	if (state->m_blitter_direction_y)
	{
		sizey = state->m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		sizey = state->m_blitter_sizey;
		skipy = -1;
	}

	gfxlen = machine.region("gfx1")->bytes();
	gfxaddr = (state->m_gfxrom << 17) + (state->m_blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr = 0;
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;
			dy = y & 0xff;

			if (state->m_highcolorflag & 0x04)
			{
				// direct mode

				if (color != 0xff)
				{
					if (state->m_highcolorflag & 0x20)
					{
						/* least significant bits */

						// src xxxxxxxx_bbbggrrr
						// dst xxbbbxxx_ggxxxrrr

						r = ((color & 0x07) >> 0) & 0x07;
						g = ((color & 0x18) >> 3) & 0x03;
						b = ((color & 0xe0) >> 5) & 0x07;

						pen = MAKE_RGB(pal6bit(r), pal5bit(g), pal5bit(b));

						state->m_tmpbitmap.pix32(dy, dx1) = state->m_tmpbitmap.pix32(dy, dx1) | pen;
						state->m_tmpbitmap.pix32(dy, dx2) = state->m_tmpbitmap.pix32(dy, dx2) | pen;
					}
					else
					{
						/* most significant bits */

						// src xxxxxxxx_bbgggrrr
						// dst bbxxxggg_xxrrrxxx

						r = ((color & 0x07) >> 0) & 0x07;
						g = ((color & 0x38) >> 3) & 0x07;
						b = ((color & 0xc0) >> 6) & 0x03;

						pen = MAKE_RGB(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

						state->m_tmpbitmap.pix32(dy, dx1) = pen;
						state->m_tmpbitmap.pix32(dy, dx2) = pen;
					}
				}
			}
			else
			{
				// lookup table mode

				if (state->m_blitter_direction_x)
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

				if (state->m_clut[color1])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					r = ((~state->m_clut[color1] & 0x07) >> 0) & 0x07;
					g = ((~state->m_clut[color1] & 0x38) >> 3) & 0x07;
					b = ((~state->m_clut[color1] & 0xc0) >> 6) & 0x03;

					pen = MAKE_RGB(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					state->m_tmpbitmap.pix32(dy, dx1) = pen;
				}

				if (state->m_clut[color2])
				{
					// src xxxxxxxx_bbgggrrr
					// dst bbxxxggg_xxrrrxxx

					r = ((~state->m_clut[color2] & 0x07) >> 0) & 0x07;
					g = ((~state->m_clut[color2] & 0x38) >> 3) & 0x07;
					b = ((~state->m_clut[color2] & 0xc0) >> 6) & 0x03;

					pen = MAKE_RGB(pal6bit(r << 3), pal5bit(g << 2), pal5bit(b << 3));

					state->m_tmpbitmap.pix32(dy, dx2) = pen;
				}
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	machine.scheduler().timer_set(attotime::from_hz(400000) * nb1413m3_busyctr, FUNC(blitter_timer_callback));
}


VIDEO_START( hyhoo )
{
	hyhoo_state *state = machine.driver_data<hyhoo_state>();
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap);
}


SCREEN_UPDATE_RGB32( hyhoo )
{
	hyhoo_state *state = screen.machine().driver_data<hyhoo_state>();
	if (state->m_dispflag)
		copybitmap(bitmap, state->m_tmpbitmap, state->m_flipscreen, state->m_flipscreen, 0, 0, cliprect);
	else
		bitmap.fill(RGB_BLACK, cliprect);

	return 0;
}
