/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nb1413m3.h"
#include "includes/nbmj8991.h"


static void nbmj8991_vramflip(running_machine &machine);
static void nbmj8991_gfxdraw(running_machine &machine);
static void update_pixel(running_machine &machine, int x, int y);


/******************************************************************************


******************************************************************************/

WRITE8_MEMBER(nbmj8991_state::nbmj8991_palette_type1_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 0] & 0x0f) >> 0);
	g = ((m_generic_paletteram_8[offset + 1] & 0xf0) >> 4);
	b = ((m_generic_paletteram_8[offset + 1] & 0x0f) >> 0);

	palette_set_color_rgb(machine(), (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_MEMBER(nbmj8991_state::nbmj8991_palette_type2_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 0] & 0x7c) >> 2);
	g = (((m_generic_paletteram_8[offset + 0] & 0x03) << 3) | ((m_generic_paletteram_8[offset + 1] & 0xe0) >> 5));
	b = ((m_generic_paletteram_8[offset + 1] & 0x1f) >> 0);

	palette_set_color_rgb(machine(), (offset / 2), pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(nbmj8991_state::nbmj8991_palette_type3_w)
{
	int r, g, b;

	m_generic_paletteram_8[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_generic_paletteram_8[offset + 1] & 0x0f) >> 0);
	g = ((m_generic_paletteram_8[offset + 0] & 0xf0) >> 4);
	b = ((m_generic_paletteram_8[offset + 0] & 0x0f) >> 0);

	palette_set_color_rgb(machine(), (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj8991_state::nbmj8991_blitter_w)
{
	int gfxlen = machine().region("gfx1")->bytes();

	switch (offset)
	{
		case 0x00:	m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 0x01:	m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:	break;
		case 0x03:	break;
		case 0x04:	m_blitter_sizex = data; break;
		case 0x05:	m_blitter_sizey = data;
					/* writing here also starts the blit */
					nbmj8991_gfxdraw(machine());
					break;
		case 0x06:	m_blitter_direction_x = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y = (data & 0x02) ? 1 : 0;
					m_flipscreen = (data & 0x04) ? 0 : 1;
					m_dispflag = (data & 0x10) ? 0 : 1;
					nbmj8991_vramflip(machine());
					break;
		case 0x07:	break;
		case 0x10:	m_blitter_destx = (m_blitter_destx & 0xff00) | data; break;
		case 0x20:	m_blitter_desty = (m_blitter_desty & 0xff00) | data; break;
		case 0x30:	m_scrollx = (m_scrollx & 0xff00) | data; break;
		case 0x40:	m_scrolly = (m_scrolly & 0xff00) | data; break;
		case 0x50:	m_blitter_destx = (m_blitter_destx & 0x00ff) | ((data & 0x01) << 8);
					m_blitter_desty = (m_blitter_desty & 0x00ff) | ((data & 0x02) << 7);
					m_scrollx = (m_scrollx & 0x00ff) | ((data & 0x04) << 6);
					m_scrolly = (m_scrolly & 0x00ff) | ((data & 0x08) << 5);
					break;
		case 0x60:	m_gfxrom = data; break;
		case 0x70:	m_clutsel = data; break;
	}

	if ((0x20000 * m_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

READ8_MEMBER(nbmj8991_state::nbmj8991_clut_r)
{
	return m_clut[offset];
}

WRITE8_MEMBER(nbmj8991_state::nbmj8991_clut_w)
{
	m_clut[((m_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
static void nbmj8991_vramflip(running_machine &machine)
{
	nbmj8991_state *state = machine.driver_data<nbmj8991_state>();
	int x, y;
	UINT8 color1, color2;
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	if (state->m_flipscreen == state->m_flipscreen_old) return;

	for (y = 0; y < height / 2; y++)
	{
		for (x = 0; x < width / 2; x++)
		{
			// rotate 180 degrees (   0,   0) - ( 511, 511)
			color1 = state->m_videoram[(y * width) + x];
			color2 = state->m_videoram[(((height - 1) - y) * width) + (((width / 2) - 1) - x)];
			state->m_videoram[(y * width) + x] = color2;
			state->m_videoram[(((height - 1) - y) * width) + (((width / 2) - 1) - x)] = color1;
			// rotate 180 degrees ( 512,   0) - (1023, 511)
			color1 = state->m_videoram[(y * width) + (x + (width / 2))];
			color2 = state->m_videoram[(((height - 1) - y) * width) + ((((width / 2) - 1) - x) + (width / 2))];
			state->m_videoram[(y * width) + (x + (width / 2))] = color2;
			state->m_videoram[(((height - 1) - y) * width) + ((((width / 2) - 1) - x) + (width / 2))] = color1;
		}
	}

	state->m_flipscreen_old = state->m_flipscreen;
	state->m_screen_refresh = 1;
}

static void update_pixel(running_machine &machine, int x, int y)
{
	nbmj8991_state *state = machine.driver_data<nbmj8991_state>();
	UINT8 color = state->m_videoram[(y * machine.primary_screen->width()) + x];
	state->m_tmpbitmap.pix16(y, x) = color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void nbmj8991_gfxdraw(running_machine &machine)
{
	nbmj8991_state *state = machine.driver_data<nbmj8991_state>();
	UINT8 *GFX = machine.region("gfx1")->base();
	int width = machine.primary_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT8 color, color1, color2;
	int gfxaddr, gfxlen;

	nb1413m3_busyctr = 0;

	if (state->m_blitter_direction_x)
	{
		startx = state->m_blitter_destx;
		sizex = state->m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		startx = state->m_blitter_destx + state->m_blitter_sizex;
		sizex = state->m_blitter_sizex;
		skipx = -1;
	}

	if (state->m_blitter_direction_y)
	{
		starty = state->m_blitter_desty;
		sizey = state->m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		starty = state->m_blitter_desty + state->m_blitter_sizey;
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
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!state->m_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy ^= 0x1ff;
			}

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

			color1 = state->m_clut[((state->m_clutsel & 0x7f) * 0x10) + color1];
			color2 = state->m_clut[((state->m_clutsel & 0x7f) * 0x10) + color2];

			if (color1 != 0xff)
			{
				state->m_videoram[(dy * width) + dx1] = color1;
				update_pixel(machine, dx1, dy);
			}
			if (color2 != 0xff)
			{
				state->m_videoram[(dy * width) + dx2] = color2;
				update_pixel(machine, dx2, dy);
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	machine.scheduler().timer_set(attotime::from_nsec(1650) * nb1413m3_busyctr, FUNC(blitter_timer_callback));
}

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj8991 )
{
	nbmj8991_state *state = machine.driver_data<nbmj8991_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap);
	state->m_videoram = auto_alloc_array(machine, UINT8, width * height);
	state->m_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(state->m_videoram, 0x00, (width * height * sizeof(UINT8)));
}

SCREEN_UPDATE_IND16( nbmj8991_type1 )
{
	nbmj8991_state *state = screen.machine().driver_data<nbmj8991_state>();
	int x, y;

	if (state->m_screen_refresh)
	{
		int width = screen.machine().primary_screen->width();
		int height = screen.machine().primary_screen->height();

		state->m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel(screen.machine(), x, y);
	}

	if (state->m_dispflag)
	{
		int scrollx, scrolly;

		if (state->m_flipscreen)
		{
			scrollx = (((-state->m_scrollx) + 0x000) & 0x1ff) * 2;
			scrolly =  ((-state->m_scrolly) - 0x00f) & 0x1ff;
		}
		else
		{
			scrollx = (((-state->m_scrollx) - 0x100) & 0x1ff) * 2;
			scrolly =  (( state->m_scrolly) + 0x0f1) & 0x1ff;
		}

		copyscrollbitmap(bitmap, state->m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}
	else
		bitmap.fill(0);

	return 0;
}

SCREEN_UPDATE_IND16( nbmj8991_type2 )
{
	nbmj8991_state *state = screen.machine().driver_data<nbmj8991_state>();
	int x, y;

	if (state->m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		state->m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel(screen.machine(), x, y);
	}

	if (nb1413m3_inputport & 0x20)
	{
		int scrollx, scrolly;

		if (state->m_flipscreen)
		{
			scrollx = (((-state->m_scrollx) + 0x000) & 0x1ff) * 2;
			scrolly =  ((-state->m_scrolly) - 0x00f) & 0x1ff;
		}
		else
		{
			scrollx = (((-state->m_scrollx) - 0x100) & 0x1ff) * 2;
			scrolly =  (( state->m_scrolly) + 0x0f1) & 0x1ff;
		}

		copyscrollbitmap(bitmap, state->m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}
	else
		bitmap.fill(0);

	return 0;
}
