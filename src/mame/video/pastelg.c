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

WRITE8_HANDLER( pastelg_clut_w )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	state->clut[offset] = data;
}

/******************************************************************************


******************************************************************************/
int pastelg_blitter_src_addr_r(address_space *space)
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	return state->blitter_src_addr;
}

WRITE8_HANDLER( pastelg_blitter_w )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	switch (offset)
	{
		case 0: state->blitter_src_addr = (state->blitter_src_addr & 0xff00) | data; break;
		case 1: state->blitter_src_addr = (state->blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2: state->blitter_destx = data; break;
		case 3: state->blitter_desty = data; break;
		case 4: state->blitter_sizex = data; break;
		case 5: state->blitter_sizey = data;
				/* writing here also starts the blit */
				pastelg_gfxdraw(space->machine());
				break;
		case 6:	state->blitter_direction_x = (data & 0x01) ? 1 : 0;
				state->blitter_direction_y = (data & 0x02) ? 1 : 0;
				state->flipscreen = (data & 0x04) ? 0 : 1;
				state->dispflag = (data & 0x08) ? 0 : 1;
				pastelg_vramflip(space->machine());
				break;
	}
}


WRITE8_HANDLER( threeds_romsel_w )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	if (data&0xfc) printf("%02x\n",data);
	state->gfxrom = (data & 0x3);
}

WRITE8_HANDLER( threeds_output_w )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	state->palbank = ((data & 0x10) >> 4);

}

READ8_HANDLER( threeds_rom_readback_r )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	UINT8 *GFX = space->machine().region("gfx1")->base();

	return GFX[(state->blitter_src_addr | (state->gfxrom << 16)) & 0x3ffff];
}


WRITE8_HANDLER( pastelg_romsel_w )
{
	pastelg_state *state = space->machine().driver_data<pastelg_state>();
	int gfxlen = space->machine().region("gfx1")->bytes();
	state->gfxrom = ((data & 0xc0) >> 6);
	state->palbank = ((data & 0x10) >> 4);
	nb1413m3_sndrombank1_w(space, 0, data);

	if ((state->gfxrom << 16) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		state->gfxrom &= (gfxlen / 0x20000 - 1);
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

	if (state->flipscreen == state->flipscreen_old) return;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = state->videoram[(y * width) + x];
			color2 = state->videoram[((y ^ 0xff) * width) + (x ^ 0xff)];
			state->videoram[(y * width) + x] = color2;
			state->videoram[((y ^ 0xff) * width) + (x ^ 0xff)] = color1;
		}
	}

	state->flipscreen_old = state->flipscreen;
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

	startx = state->blitter_destx + state->blitter_sizex;
	starty = state->blitter_desty + state->blitter_sizey;


	if (state->blitter_direction_x)
	{
		if (state->blitter_sizex&0x80) sizex = 0xff-state->blitter_sizex;
		else sizex=state->blitter_sizex;
		incx = 1;
	}
	else
	{
		sizex = state->blitter_sizex;
		incx = -1;
	}

	if (state->blitter_direction_y)
	{
		if (state->blitter_sizey&0x80) sizey = 0xff-state->blitter_sizey;
		else sizey=state->blitter_sizey;
		incy = 1;
	}
	else
	{
		sizey = state->blitter_sizey;
		incy = -1;
	}

	gfxlen = machine.region("gfx1")->bytes();
	gfxaddr = (state->gfxrom << 16) + state->blitter_src_addr;

	readflag = 0;

	count = 0;
	y = starty;

	for (ctry = sizey; ctry >= 0; ctry--)
	{
		x = startx;

		for (ctrx = sizex; ctrx >= 0; ctrx--)
		{
			gfxaddr = (state->gfxrom << 16) + ((state->blitter_src_addr + count));

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

			if (state->flipscreen)
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

			if (state->clut[color] & 0xf0)
			{
				if (color)
				{
					color = ((state->palbank * 0x10) + color);
					state->videoram[(dy * width) + dx] = color;
				}
			}
			else
			{
				if(state->clut[color] != 0)
				{
					color = ((state->palbank * 0x10) + state->clut[color]);
					state->videoram[(dy * width) + dx] = color;
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

	state->videoram = auto_alloc_array_clear(machine, UINT8, width * height);
	state->clut = auto_alloc_array(machine, UINT8, 0x10);
}

/******************************************************************************


******************************************************************************/
SCREEN_UPDATE( pastelg )
{
	pastelg_state *state = screen->machine().driver_data<pastelg_state>();
	if (state->dispflag)
	{
		int x, y;
		int width = screen->width();
		int height = screen->height();

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				*BITMAP_ADDR16(bitmap, y, x) = state->videoram[(y * width) + x];
	}
	else
		bitmap_fill(bitmap, cliprect, 0);

	return 0;
}
