/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi 2000/06/07 -

******************************************************************************/

#include "emu.h"
#include "includes/nb1413m3.h"
#include "includes/pastelg.h"


static void pastelg_vramflip(running_machine &machine);
static void pastelg_gfxdraw(running_machine &machine);


/******************************************************************************


******************************************************************************/
PALETTE_INIT( pastelg )
{
	int i;
	int bit0, bit1, bit2, bit3, r, g, b;

	for (i = 0; i < machine.total_colors(); i++)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_MEMBER(pastelg_state::pastelg_clut_w)
{
	m_clut[offset] = data;
}

/******************************************************************************


******************************************************************************/
int pastelg_blitter_src_addr_r(address_space *space)
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	return state->m_blitter_src_addr;
}

WRITE8_MEMBER(pastelg_state::pastelg_blitter_w)
{
	switch (offset)
	{
		case 0: m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 1: m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2: m_blitter_destx = data; break;
		case 3: m_blitter_desty = data; break;
		case 4: m_blitter_sizex = data; break;
		case 5: m_blitter_sizey = data;
				/* writing here also starts the blit */
				pastelg_gfxdraw(machine());
				break;
		case 6:	m_blitter_direction_x = (data & 0x01) ? 1 : 0;
				m_blitter_direction_y = (data & 0x02) ? 1 : 0;
				m_flipscreen = (data & 0x04) ? 0 : 1;
				m_dispflag = (data & 0x08) ? 0 : 1;
				pastelg_vramflip(machine());
				break;
	}
}


WRITE8_MEMBER(pastelg_state::threeds_romsel_w)
{
	if (data&0xfc) printf("%02x\n",data);
	m_gfxrom = (data & 0x3);
}

WRITE8_MEMBER(pastelg_state::threeds_output_w)
{
	m_palbank = ((data & 0x10) >> 4);

}

READ8_MEMBER(pastelg_state::threeds_rom_readback_r)
{
	UINT8 *GFX = machine().region("gfx1")->base();

	return GFX[(m_blitter_src_addr | (m_gfxrom << 16)) & 0x3ffff];
}


WRITE8_MEMBER(pastelg_state::pastelg_romsel_w)
{
	int gfxlen = machine().region("gfx1")->bytes();
	m_gfxrom = ((data & 0xc0) >> 6);
	m_palbank = ((data & 0x10) >> 4);
	nb1413m3_sndrombank1_w(&space, 0, data);

	if ((m_gfxrom << 16) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
static void pastelg_vramflip(running_machine &machine)
{
	pastelg_state *state = machine.driver_data<pastelg_state>();
	int x, y;
	UINT8 color1, color2;
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	if (state->m_flipscreen == state->m_flipscreen_old) return;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = state->m_videoram[(y * width) + x];
			color2 = state->m_videoram[((y ^ 0xff) * width) + (x ^ 0xff)];
			state->m_videoram[(y * width) + x] = color2;
			state->m_videoram[((y ^ 0xff) * width) + (x ^ 0xff)] = color1;
		}
	}

	state->m_flipscreen_old = state->m_flipscreen;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void pastelg_gfxdraw(running_machine &machine)
{
	pastelg_state *state = machine.driver_data<pastelg_state>();
	UINT8 *GFX = machine.region("gfx1")->base();
	int width = machine.primary_screen->width();

	int x, y;
	int dx, dy;
	int startx, starty;
	int sizex, sizey;
	int incx, incy;
	int ctrx, ctry;
	int readflag;
	int gfxaddr, gfxlen;
	int count;
	UINT8 color;

	nb1413m3_busyctr = 0;

	startx = state->m_blitter_destx + state->m_blitter_sizex;
	starty = state->m_blitter_desty + state->m_blitter_sizey;


	if (state->m_blitter_direction_x)
	{
		if (state->m_blitter_sizex&0x80) sizex = 0xff-state->m_blitter_sizex;
		else sizex=state->m_blitter_sizex;
		incx = 1;
	}
	else
	{
		sizex = state->m_blitter_sizex;
		incx = -1;
	}

	if (state->m_blitter_direction_y)
	{
		if (state->m_blitter_sizey&0x80) sizey = 0xff-state->m_blitter_sizey;
		else sizey=state->m_blitter_sizey;
		incy = 1;
	}
	else
	{
		sizey = state->m_blitter_sizey;
		incy = -1;
	}

	gfxlen = machine.region("gfx1")->bytes();
	gfxaddr = (state->m_gfxrom << 16) + state->m_blitter_src_addr;

	readflag = 0;

	count = 0;
	y = starty;

	for (ctry = sizey; ctry >= 0; ctry--)
	{
		x = startx;

		for (ctrx = sizex; ctrx >= 0; ctrx--)
		{
			gfxaddr = (state->m_gfxrom << 16) + ((state->m_blitter_src_addr + count));

			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr = 0;
			}

			color = GFX[gfxaddr];

			dx = x & 0xff;
			dy = y & 0xff;

			if (state->m_flipscreen)
			{
				dx ^= 0xff;
				dy ^= 0xff;
			}

			if (!readflag)
			{
				// 1st, 3rd, 5th, ... read
				color = (color & 0x0f);
			}
			else
			{
				// 2nd, 4th, 6th, ... read
				color = (color & 0xf0) >> 4;
				count++;
			}

			readflag ^= 1;

			if (state->m_clut[color] & 0xf0)
			{
				if (color)
				{
					color = ((state->m_palbank * 0x10) + color);
					state->m_videoram[(dy * width) + dx] = color;
				}
			}
			else
			{
				if(state->m_clut[color] != 0)
				{
					color = ((state->m_palbank * 0x10) + state->m_clut[color]);
					state->m_videoram[(dy * width) + dx] = color;
				}
			}

			nb1413m3_busyctr++;
			x += incx;
		}

		y += incy;
	}

	nb1413m3_busyflag = 0;
	machine.scheduler().timer_set(attotime::from_hz(400000) * nb1413m3_busyctr, FUNC(blitter_timer_callback));
}

/******************************************************************************


******************************************************************************/
VIDEO_START( pastelg )
{
	pastelg_state *state = machine.driver_data<pastelg_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	state->m_videoram = auto_alloc_array_clear(machine, UINT8, width * height);
	state->m_clut = auto_alloc_array(machine, UINT8, 0x10);
}

/******************************************************************************


******************************************************************************/
SCREEN_UPDATE_IND16( pastelg )
{
	pastelg_state *state = screen.machine().driver_data<pastelg_state>();
	if (state->m_dispflag)
	{
		int x, y;
		int width = screen.width();
		int height = screen.height();

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				bitmap.pix16(y, x) = state->m_videoram[(y * width) + x];
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}
