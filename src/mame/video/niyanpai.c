/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/

#include "emu.h"
#include "includes/niyanpai.h"


static void niyanpai_vramflip(running_machine *machine, int vram);
static void niyanpai_gfxdraw(running_machine *machine, int vram);


/******************************************************************************


******************************************************************************/
READ16_HANDLER( niyanpai_palette_r )
{
	niyanpai_state *state = space->machine->driver_data<niyanpai_state>();
	return state->palette[offset];
}

WRITE16_HANDLER( niyanpai_palette_w )
{
	niyanpai_state *state = space->machine->driver_data<niyanpai_state>();
	int r, g, b;
	int offs_h, offs_l;
	UINT16 oldword = state->palette[offset];
	UINT16 newword;

	COMBINE_DATA(&state->palette[offset]);
	newword = state->palette[offset];

	if (oldword != newword)
	{
		offs_h = (offset / 0x180);
		offs_l = (offset & 0x7f);

		if (ACCESSING_BITS_8_15)
		{
			r  = ((state->palette[(0x000 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			g  = ((state->palette[(0x080 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);
			b  = ((state->palette[(0x100 + (offs_h * 0x180) + offs_l)] & 0xff00) >> 8);

			palette_set_color(space->machine, ((offs_h << 8) + (offs_l << 1) + 0), MAKE_RGB(r, g, b));
		}

		if (ACCESSING_BITS_0_7)
		{
			r  = ((state->palette[(0x000 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			g  = ((state->palette[(0x080 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);
			b  = ((state->palette[(0x100 + (offs_h * 0x180) + offs_l)] & 0x00ff) >> 0);

			palette_set_color(space->machine, ((offs_h << 8) + (offs_l << 1) + 1), MAKE_RGB(r, g, b));
		}
	}
}

/******************************************************************************


******************************************************************************/
static int niyanpai_blitter_r(running_machine *machine, int vram, int offset)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	int ret;
	UINT8 *GFXROM = machine->region("gfx1")->base();

	switch (offset)
	{
		case 0x00:	ret = 0xfe | ((state->nb19010_busyflag & 0x01) ^ 0x01); break;	// NB19010 Busy Flag
		case 0x01:	ret = GFXROM[state->blitter_src_addr[vram]]; break;			// NB19010 GFX-ROM Read
		default:	ret = 0xff; break;
	}

	return ret;
}

static void niyanpai_blitter_w(running_machine *machine, int vram, int offset, int data)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	switch (offset)
	{
		case 0x00:	state->blitter_direction_x[vram] = (data & 0x01) ? 1 : 0;
					state->blitter_direction_y[vram] = (data & 0x02) ? 1 : 0;
					state->clutmode[vram] = (data & 0x04) ? 1 : 0;
				//  if (data & 0x08) popmessage("Unknown GFX Flag!! (0x08)");
					state->transparency[vram] = (data & 0x10) ? 1 : 0;
				//  if (data & 0x20) popmessage("Unknown GFX Flag!! (0x20)");
					state->flipscreen[vram] = (data & 0x40) ? 0 : 1;
					state->dispflag[vram] = (data & 0x80) ? 1 : 0;
					niyanpai_vramflip(machine, vram);
					break;
		case 0x01:	state->scrollx[vram] = (state->scrollx[vram] & 0x0100) | data; break;
		case 0x02:	state->scrollx[vram] = (state->scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x03:	state->scrolly[vram] = (state->scrolly[vram] & 0x0100) | data; break;
		case 0x04:	state->scrolly[vram] = (state->scrolly[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x05:	state->blitter_src_addr[vram] = (state->blitter_src_addr[vram] & 0xffff00) | data; break;
		case 0x06:	state->blitter_src_addr[vram] = (state->blitter_src_addr[vram] & 0xff00ff) | (data << 8); break;
		case 0x07:	state->blitter_src_addr[vram] = (state->blitter_src_addr[vram] & 0x00ffff) | (data << 16); break;
		case 0x08:	state->blitter_sizex[vram] = data; break;
		case 0x09:	state->blitter_sizey[vram] = data; break;
		case 0x0a:	state->blitter_destx[vram] = (state->blitter_destx[vram]  & 0xff00) | data; break;
		case 0x0b:	state->blitter_destx[vram] = (state->blitter_destx[vram]  & 0x00ff) | (data << 8); break;
		case 0x0c:	state->blitter_desty[vram] = (state->blitter_desty[vram]  & 0xff00) | data; break;
		case 0x0d:	state->blitter_desty[vram] = (state->blitter_desty[vram]  & 0x00ff) | (data << 8);
					niyanpai_gfxdraw(machine, vram);
					break;
		default:	break;
	}
}

static void niyanpai_clutsel_w(running_machine *machine, int vram, int data)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	state->clutsel[vram] = data;
}

static void niyanpai_clut_w(running_machine *machine, int vram, int offset, int data)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	state->clut[vram][((state->clutsel[vram] & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
static void niyanpai_vramflip(running_machine *machine, int vram)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	int x, y;
	UINT16 color1, color2;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	if (state->flipscreen[vram] == state->flipscreen_old[vram]) return;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = state->videoram[vram][(y * width) + x];
			color2 = state->videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			state->videoram[vram][(y * width) + x] = color2;
			state->videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = state->videoworkram[vram][(y * width) + x];
			color2 = state->videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			state->videoworkram[vram][(y * width) + x] = color2;
			state->videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	state->flipscreen_old[vram] = state->flipscreen[vram];
	state->screen_refresh = 1;
}

static void update_pixel(running_machine *machine, int vram, int x, int y)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	UINT16 color = state->videoram[vram][(y * machine->primary_screen->width()) + x];
	*BITMAP_ADDR16(state->tmpbitmap[vram], y, x) = color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	state->nb19010_busyflag = 1;
}

static void niyanpai_gfxdraw(running_machine *machine, int vram)
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	UINT8 *GFX = machine->region("gfx1")->base();
	int width = machine->primary_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT16 color, color1, color2;
	int gfxaddr, gfxlen;

	state->nb19010_busyctr = 0;

	if (state->clutmode[vram])
	{
		// NB22090 clut256 mode
		state->blitter_sizex[vram] = GFX[((state->blitter_src_addr[vram] + 0) & 0x00ffffff)];
		state->blitter_sizey[vram] = GFX[((state->blitter_src_addr[vram] + 1) & 0x00ffffff)];
	}

	if (state->blitter_direction_x[vram])
	{
		startx = state->blitter_destx[vram];
		sizex = state->blitter_sizex[vram];
		skipx = 1;
	}
	else
	{
		startx = state->blitter_destx[vram] + state->blitter_sizex[vram];
		sizex = state->blitter_sizex[vram];
		skipx = -1;
	}

	if (state->blitter_direction_y[vram])
	{
		starty = state->blitter_desty[vram];
		sizey = state->blitter_sizey[vram];
		skipy = 1;
	}
	else
	{
		starty = state->blitter_desty[vram] + state->blitter_sizey[vram];
		sizey = state->blitter_sizey[vram];
		skipy = -1;
	}

	gfxlen = machine->region("gfx1")->bytes();
	gfxaddr = ((state->blitter_src_addr[vram] + 2) & 0x00ffffff);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d", gfxaddr, startx, starty, sizex,sizey);
				logerror("GFXROM ADDR OVER:%08X DX,%d,DY:%d,SX:%d,SY:%d\n", gfxaddr, startx, starty, sizex,sizey);
#endif
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x3ff;
			dx2 = (2 * x + 1) & 0x3ff;
			dy = y & 0x1ff;

			if (!state->flipscreen[vram])
			{
				dx1 ^= 0x3ff;
				dx2 ^= 0x3ff;
				dy ^= 0x1ff;
			}

			if (state->blitter_direction_x[vram])
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

			if (state->clutmode[vram])
			{
				// clut256 mode

				if (state->clutsel[vram] & 0x80)
				{
					// clut256 mode 1st(low)
					state->videoworkram[vram][(dy * width) + dx1] &= 0x00f0;
					state->videoworkram[vram][(dy * width) + dx1] |= color1 & 0x0f;
					state->videoworkram[vram][(dy * width) + dx2] &= 0x00f0;
					state->videoworkram[vram][(dy * width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					state->videoworkram[vram][(dy * width) + dx1] &= 0x000f;
					state->videoworkram[vram][(dy * width) + dx1] |= (color1 & 0x0f) << 4;
					state->videoworkram[vram][(dy * width) + dx2] &= 0x000f;
					state->videoworkram[vram][(dy * width) + dx2] |= (color2 & 0x0f) << 4;

		//          state->videoworkram[vram][(dy * width) + dx1] += state->clut[vram][(state->clutsel[vram] * 0x10)];
		//          state->videoworkram[vram][(dy * width) + dx2] += state->clut[vram][(state->clutsel[vram] * 0x10)];
				}

				color1 = state->videoworkram[vram][(dy * width) + dx1];
				color2 = state->videoworkram[vram][(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = state->clut[vram][(state->clutsel[vram] * 0x10) + color1];
				color2 = state->clut[vram][(state->clutsel[vram] * 0x10) + color2];
			}

			color1 |= (0x0100 * vram);
			color2 |= (0x0100 * vram);

			if (((color1 & 0x00ff) != 0x00ff) || (!state->transparency[vram]))
			{
				state->videoram[vram][(dy * width) + dx1] = color1;
				update_pixel(machine, vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!state->transparency[vram]))
			{
				state->videoram[vram][(dy * width) + dx2] = color2;
				update_pixel(machine, vram, dx2, dy);
			}

			state->nb19010_busyctr++;
		}
	}

	if (state->clutmode[vram])
	{
		// NB22090 clut256 mode
		state->blitter_src_addr[vram] = gfxaddr;
	}

	state->nb19010_busyflag = 0;
	machine->scheduler().timer_set(attotime::from_nsec(1650 * state->nb19010_busyctr), FUNC(blitter_timer_callback));
}

/******************************************************************************


******************************************************************************/
WRITE16_HANDLER( niyanpai_blitter_0_w )	{ niyanpai_blitter_w(space->machine, 0, offset, data); }
WRITE16_HANDLER( niyanpai_blitter_1_w )	{ niyanpai_blitter_w(space->machine, 1, offset, data); }
WRITE16_HANDLER( niyanpai_blitter_2_w )	{ niyanpai_blitter_w(space->machine, 2, offset, data); }

READ16_HANDLER( niyanpai_blitter_0_r )	{ return niyanpai_blitter_r(space->machine, 0, offset); }
READ16_HANDLER( niyanpai_blitter_1_r )	{ return niyanpai_blitter_r(space->machine, 1, offset); }
READ16_HANDLER( niyanpai_blitter_2_r )	{ return niyanpai_blitter_r(space->machine, 2, offset); }

WRITE16_HANDLER( niyanpai_clut_0_w )	{ niyanpai_clut_w(space->machine, 0, offset, data); }
WRITE16_HANDLER( niyanpai_clut_1_w )	{ niyanpai_clut_w(space->machine, 1, offset, data); }
WRITE16_HANDLER( niyanpai_clut_2_w )	{ niyanpai_clut_w(space->machine, 2, offset, data); }

WRITE16_HANDLER( niyanpai_clutsel_0_w )	{ niyanpai_clutsel_w(space->machine, 0, data); }
WRITE16_HANDLER( niyanpai_clutsel_1_w )	{ niyanpai_clutsel_w(space->machine, 1, data); }
WRITE16_HANDLER( niyanpai_clutsel_2_w )	{ niyanpai_clutsel_w(space->machine, 2, data); }

/******************************************************************************


******************************************************************************/
VIDEO_START( niyanpai )
{
	niyanpai_state *state = machine->driver_data<niyanpai_state>();
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	state->tmpbitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	state->tmpbitmap[1] = machine->primary_screen->alloc_compatible_bitmap();
	state->tmpbitmap[2] = machine->primary_screen->alloc_compatible_bitmap();
	state->videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->videoram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->videoram[2] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->videoworkram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->videoworkram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->videoworkram[2] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->palette = auto_alloc_array(machine, UINT16, 0x480);
	state->clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	state->clut[1] = auto_alloc_array(machine, UINT8, 0x1000);
	state->clut[2] = auto_alloc_array(machine, UINT8, 0x1000);
	state->nb19010_busyflag = 1;
}

/******************************************************************************


******************************************************************************/
SCREEN_UPDATE( niyanpai )
{
	niyanpai_state *state = screen->machine->driver_data<niyanpai_state>();
	int i;
	int x, y;
	int scrollx[3], scrolly[3];

	if (state->screen_refresh)
	{
		int width = screen->width();
		int height = screen->height();

		state->screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				update_pixel(screen->machine, 0, x, y);
				update_pixel(screen->machine, 1, x, y);
				update_pixel(screen->machine, 2, x, y);
			}
	}

	for (i = 0; i < 3; i++)
	{
		if (state->flipscreen[i])
		{
			scrollx[i] = (((-state->scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = (-state->scrolly[i]) & 0x1ff;
		}
		else
		{
			scrollx[i] = (((-state->scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			scrolly[i] = state->scrolly[i] & 0x1ff;
		}
	}

	if (state->dispflag[0])
		copyscrollbitmap(bitmap, state->tmpbitmap[0], 1, &scrollx[0], 1, &scrolly[0], cliprect);
	else
		bitmap_fill(bitmap, 0, 0x00ff);

	if (state->dispflag[1])
		copyscrollbitmap_trans(bitmap, state->tmpbitmap[1], 1, &scrollx[1], 1, &scrolly[1], cliprect, 0x01ff);

	if (state->dispflag[2])
		copyscrollbitmap_trans(bitmap, state->tmpbitmap[2], 1, &scrollx[2], 1, &scrolly[2], cliprect, 0x02ff);

	return 0;
}
