/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -
    Special thanks to Tatsuyuki Satoh

******************************************************************************/

#include "emu.h"
#include "includes/nbmj9195.h"


static void nbmj9195_vramflip(running_machine &machine, int vram);
static void nbmj9195_gfxdraw(running_machine &machine, int vram);


/******************************************************************************


******************************************************************************/
READ8_MEMBER(nbmj9195_state::nbmj9195_palette_r)
{
	return m_palette[offset];
}

WRITE8_MEMBER(nbmj9195_state::nbmj9195_palette_w)
{
	int r, g, b;

	m_palette[offset] = data;

	if (offset & 1)
	{
		offset &= 0x1fe;

		r = ((m_palette[offset + 0] & 0x0f) >> 0);
		g = ((m_palette[offset + 0] & 0xf0) >> 4);
		b = ((m_palette[offset + 1] & 0x0f) >> 0);

		palette_set_color_rgb(machine(), (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

READ8_MEMBER(nbmj9195_state::nbmj9195_nb22090_palette_r)
{
	return m_nb22090_palette[offset];
}

WRITE8_MEMBER(nbmj9195_state::nbmj9195_nb22090_palette_w)
{
	int r, g, b;
	int offs_h, offs_l;

	m_nb22090_palette[offset] = data;

	offs_h = (offset / 0x0300);
	offs_l = (offset & 0x00ff);

	r = m_nb22090_palette[(0x000 + (offs_h * 0x300) + offs_l)];
	g = m_nb22090_palette[(0x100 + (offs_h * 0x300) + offs_l)];
	b = m_nb22090_palette[(0x200 + (offs_h * 0x300) + offs_l)];

	palette_set_color(machine(), ((offs_h * 0x100) + offs_l), MAKE_RGB(r, g, b));
}

/******************************************************************************


******************************************************************************/
static int nbmj9195_blitter_r(address_space *space, int offset, int vram)
{
	nbmj9195_state *state = space->machine().driver_data<nbmj9195_state>();
	int ret;
	UINT8 *GFXROM = space->machine().region("gfx1")->base();

	switch (offset)
	{
		case 0x00:	ret = 0xfe | ((state->m_nb19010_busyflag & 0x01) ^ 0x01); break;	// NB19010 Busy Flag
		case 0x01:	ret = GFXROM[state->m_blitter_src_addr[vram]]; break;			// NB19010 GFX-ROM Read
		default:	ret = 0xff; break;
	}

	return ret;
}

static void nbmj9195_blitter_w(address_space *space, int offset, int data, int vram)
{
	nbmj9195_state *state = space->machine().driver_data<nbmj9195_state>();
	int new_line;

	switch (offset)
	{
		case 0x00:	state->m_blitter_direction_x[vram] = (data & 0x01) ? 1 : 0;
					state->m_blitter_direction_y[vram] = (data & 0x02) ? 1 : 0;
					state->m_clutmode[vram] = (data & 0x04) ? 1 : 0;
				//  if (data & 0x08) popmessage("Unknown GFX Flag!! (0x08)");
					state->m_transparency[vram] = (data & 0x10) ? 1 : 0;
				//  if (data & 0x20) popmessage("Unknown GFX Flag!! (0x20)");
					state->m_flipscreen[vram] = (data & 0x40) ? 0 : 1;
					state->m_dispflag[vram] = (data & 0x80) ? 1 : 0;
					nbmj9195_vramflip(space->machine(), vram);
					break;
		case 0x01:	state->m_scrollx[vram] = (state->m_scrollx[vram] & 0x0100) | data; break;
		case 0x02:	state->m_scrollx[vram] = (state->m_scrollx[vram] & 0x00ff) | ((data << 8) & 0x0100);
					new_line = space->machine().primary_screen->vpos();
					if (state->m_flipscreen[vram])
					{
						for ( ; state->m_scanline[vram] < new_line; state->m_scanline[vram]++)
							state->m_scrollx_raster[vram][state->m_scanline[vram]] = (((-state->m_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					else
					{
						for ( ; state->m_scanline[vram] < new_line; state->m_scanline[vram]++)
							state->m_scrollx_raster[vram][(state->m_scanline[vram] ^ 0x1ff)] = (((-state->m_scrollx[vram]) - 0x4e)  & 0x1ff) << 1;
					}
					break;
		case 0x03:	state->m_scrolly[vram] = (state->m_scrolly[vram] & 0x0100) | data; break;
		case 0x04:	state->m_scrolly[vram] = (state->m_scrolly[vram] & 0x00ff) | ((data << 8) & 0x0100); break;
		case 0x05:	state->m_blitter_src_addr[vram] = (state->m_blitter_src_addr[vram] & 0xffff00) | data; break;
		case 0x06:	state->m_blitter_src_addr[vram] = (state->m_blitter_src_addr[vram] & 0xff00ff) | (data << 8); break;
		case 0x07:	state->m_blitter_src_addr[vram] = (state->m_blitter_src_addr[vram] & 0x00ffff) | (data << 16); break;
		case 0x08:	state->m_blitter_sizex[vram] = data; break;
		case 0x09:	state->m_blitter_sizey[vram] = data; break;
		case 0x0a:	state->m_blitter_destx[vram] = (state->m_blitter_destx[vram]  & 0xff00) | data; break;
		case 0x0b:	state->m_blitter_destx[vram] = (state->m_blitter_destx[vram]  & 0x00ff) | (data << 8); break;
		case 0x0c:	state->m_blitter_desty[vram] = (state->m_blitter_desty[vram]  & 0xff00) | data; break;
		case 0x0d:	state->m_blitter_desty[vram] = (state->m_blitter_desty[vram]  & 0x00ff) | (data << 8);
					nbmj9195_gfxdraw(space->machine(), vram);
					break;
		default:	break;
	}
}

void nbmj9195_clutsel_w(address_space *space, int data)
{
	nbmj9195_state *state = space->machine().driver_data<nbmj9195_state>();
	state->m_clutsel = data;
}

static void nbmj9195_clut_w(address_space *space, int offset, int data, int vram)
{
	nbmj9195_state *state = space->machine().driver_data<nbmj9195_state>();
	state->m_clut[vram][((state->m_clutsel & 0xff) * 0x10) + (offset & 0x0f)] = data;
}

void nbmj9195_gfxflag2_w(address_space *space, int data)
{
	nbmj9195_state *state = space->machine().driver_data<nbmj9195_state>();
	state->m_gfxflag2 = data;
}

/******************************************************************************


******************************************************************************/
static void nbmj9195_vramflip(running_machine &machine, int vram)
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	int x, y;
	UINT16 color1, color2;
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	if (state->m_flipscreen[vram] == state->m_flipscreen_old[vram]) return;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = state->m_videoram[vram][(y * width) + x];
			color2 = state->m_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
			state->m_videoram[vram][(y * width) + x] = color2;
			state->m_videoram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
		}
	}

	if (state->m_gfxdraw_mode == 2)
	{
		for (y = 0; y < (height / 2); y++)
		{
			for (x = 0; x < width; x++)
			{
				color1 = state->m_videoworkram[vram][(y * width) + x];
				color2 = state->m_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)];
				state->m_videoworkram[vram][(y * width) + x] = color2;
				state->m_videoworkram[vram][((y ^ 0x1ff) * width) + (x ^ 0x3ff)] = color1;
			}
		}
	}

	state->m_flipscreen_old[vram] = state->m_flipscreen[vram];
	state->m_screen_refresh = 1;
}

static void update_pixel(running_machine &machine, int vram, int x, int y)
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	UINT16 color = state->m_videoram[vram][(y * machine.primary_screen->width()) + x];
	state->m_tmpbitmap[vram].pix16(y, x) = color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	state->m_nb19010_busyflag = 1;
}

static void nbmj9195_gfxdraw(running_machine &machine, int vram)
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	UINT8 *GFX = machine.region("gfx1")->base();
	int width = machine.primary_screen->width();

	int x, y;
	int dx1, dx2, dy;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT16 color, color1, color2;
	int gfxaddr, gfxlen;

	state->m_nb19010_busyctr = 0;

	if ((state->m_gfxdraw_mode == 2) && (state->m_clutmode[vram]))
	{
		// NB22090 clut256 mode
		state->m_blitter_sizex[vram] = GFX[((state->m_blitter_src_addr[vram] + 0) & 0x00ffffff)];
		state->m_blitter_sizey[vram] = GFX[((state->m_blitter_src_addr[vram] + 1) & 0x00ffffff)];
	}

	if (state->m_blitter_direction_x[vram])
	{
		startx = state->m_blitter_destx[vram];
		sizex = state->m_blitter_sizex[vram];
		skipx = 1;
	}
	else
	{
		startx = state->m_blitter_destx[vram] + state->m_blitter_sizex[vram];
		sizex = state->m_blitter_sizex[vram];
		skipx = -1;
	}

	if (state->m_blitter_direction_y[vram])
	{
		starty = state->m_blitter_desty[vram];
		sizey = state->m_blitter_sizey[vram];
		skipy = 1;
	}
	else
	{
		starty = state->m_blitter_desty[vram] + state->m_blitter_sizey[vram];
		sizey = state->m_blitter_sizey[vram];
		skipy = -1;
	}

	gfxlen = machine.region("gfx1")->bytes();
	gfxaddr = ((state->m_blitter_src_addr[vram] + 2) & 0x00ffffff);

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

			if (!state->m_flipscreen[vram])
			{
				dx1 ^= 0x3ff;
				dx2 ^= 0x3ff;
				dy ^= 0x1ff;
			}

			if (state->m_blitter_direction_x[vram])
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

			if ((state->m_gfxdraw_mode == 2) && (state->m_clutmode[vram]))
			{
				// clut256 mode

				if (state->m_gfxflag2 & 0xc0)
				{
					// clut256 mode 1st(low)
					state->m_videoworkram[vram][(dy * width) + dx1] &= 0x00f0;
					state->m_videoworkram[vram][(dy * width) + dx1] |= color1 & 0x0f;
					state->m_videoworkram[vram][(dy * width) + dx2] &= 0x00f0;
					state->m_videoworkram[vram][(dy * width) + dx2] |= color2 & 0x0f;

					continue;
				}
				else
				{
					// clut256 mode 2nd(high)
					state->m_videoworkram[vram][(dy * width) + dx1] &= 0x000f;
					state->m_videoworkram[vram][(dy * width) + dx1] |= (color1 & 0x0f) << 4;
					state->m_videoworkram[vram][(dy * width) + dx2] &= 0x000f;
					state->m_videoworkram[vram][(dy * width) + dx2] |= (color2 & 0x0f) << 4;

					state->m_videoworkram[vram][(dy * width) + dx1] += state->m_clut[vram][(state->m_clutsel * 0x10)];
					state->m_videoworkram[vram][(dy * width) + dx2] += state->m_clut[vram][(state->m_clutsel * 0x10)];
				}

				color1 = state->m_videoworkram[vram][(dy * width) + dx1];
				color2 = state->m_videoworkram[vram][(dy * width) + dx2];
			}
			else
			{
				// clut16 mode
				color1 = state->m_clut[vram][(state->m_clutsel * 0x10) + color1];
				color2 = state->m_clut[vram][(state->m_clutsel * 0x10) + color2];
			}

			if (state->m_gfxdraw_mode == 2)
			{
				color1 |= (0x0100 * vram);
				color2 |= (0x0100 * vram);
			}

			if (((color1 & 0x00ff) != 0x00ff) || (!state->m_transparency[vram]))
			{
				state->m_videoram[vram][(dy * width) + dx1] = color1;
				update_pixel(machine, vram, dx1, dy);
			}
			if (((color2 & 0x00ff) != 0x00ff) || (!state->m_transparency[vram]))
			{
				state->m_videoram[vram][(dy * width) + dx2] = color2;
				update_pixel(machine, vram, dx2, dy);
			}

			state->m_nb19010_busyctr++;
		}
	}

	if ((state->m_gfxdraw_mode == 2) && (state->m_clutmode[vram]))
	{
		// NB22090 clut256 mode
		state->m_blitter_src_addr[vram] = gfxaddr;
	}

	state->m_nb19010_busyflag = 0;

	/* 1650ns per count */
	machine.scheduler().timer_set(attotime::from_nsec(state->m_nb19010_busyctr * 1650), FUNC(blitter_timer_callback));
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj9195_state::nbmj9195_blitter_0_w){ nbmj9195_blitter_w(&space, offset, data, 0); }
WRITE8_MEMBER(nbmj9195_state::nbmj9195_blitter_1_w){ nbmj9195_blitter_w(&space, offset, data, 1); }

READ8_MEMBER(nbmj9195_state::nbmj9195_blitter_0_r){ return nbmj9195_blitter_r(&space, offset, 0); }
READ8_MEMBER(nbmj9195_state::nbmj9195_blitter_1_r){ return nbmj9195_blitter_r(&space, offset, 1); }

WRITE8_MEMBER(nbmj9195_state::nbmj9195_clut_0_w){ nbmj9195_clut_w(&space, offset, data, 0); }
WRITE8_MEMBER(nbmj9195_state::nbmj9195_clut_1_w){ nbmj9195_clut_w(&space, offset, data, 1); }

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj9195_1layer )
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap[0]);
	state->m_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_palette = auto_alloc_array(machine, UINT8, 0x200);
	state->m_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_scanline[0] = state->m_scanline[1] = SCANLINE_MIN;
	state->m_nb19010_busyflag = 1;
	state->m_gfxdraw_mode = 0;
}

VIDEO_START( nbmj9195_2layer )
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap[0]);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap[1]);
	state->m_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_videoram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_palette = auto_alloc_array(machine, UINT8, 0x200);
	state->m_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_clut[1] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_scanline[0] = state->m_scanline[1] = SCANLINE_MIN;
	state->m_nb19010_busyflag = 1;
	state->m_gfxdraw_mode = 1;
}

VIDEO_START( nbmj9195_nb22090 )
{
	nbmj9195_state *state = machine.driver_data<nbmj9195_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap[0]);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap[1]);
	state->m_videoram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_videoram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_videoworkram[0] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_videoworkram[1] = auto_alloc_array_clear(machine, UINT16, width * height);
	state->m_nb22090_palette = auto_alloc_array(machine, UINT8, 0xc00);
	state->m_clut[0] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_clut[1] = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_scanline[0] = state->m_scanline[1] = SCANLINE_MIN;
	state->m_nb19010_busyflag = 1;
	state->m_gfxdraw_mode = 2;
}

/******************************************************************************


******************************************************************************/
SCREEN_UPDATE_IND16( nbmj9195 )
{
	nbmj9195_state *state = screen.machine().driver_data<nbmj9195_state>();
	int i;
	int x, y;
	int scrolly[2];

	if (state->m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		state->m_screen_refresh = 0;

		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				update_pixel(screen.machine(), 0, x, y);

				if (state->m_gfxdraw_mode)
					update_pixel(screen.machine(), 1, x, y);
			}
	}

	for (i = 0; i < 2; i++)
	{
		if (state->m_flipscreen[i])
		{
			for ( ; state->m_scanline[i] < SCANLINE_MAX; state->m_scanline[i]++)
			{
				state->m_scrollx_raster[i][state->m_scanline[i]] = (((-state->m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = (-state->m_scrolly[i]) & 0x1ff;
		}
		else
		{
			for ( ; state->m_scanline[i] < SCANLINE_MAX; state->m_scanline[i]++)
			{
				state->m_scrollx_raster[i][(state->m_scanline[i] ^ 0x1ff)] = (((-state->m_scrollx[i]) - 0x4e)  & 0x1ff) << 1;
			}
			scrolly[i] = state->m_scrolly[i] & 0x1ff;
		}
		state->m_scanline[i] = SCANLINE_MIN;
	}

	if (state->m_dispflag[0])
		// nbmj9195 1layer
		copyscrollbitmap(bitmap, state->m_tmpbitmap[0], SCANLINE_MAX, state->m_scrollx_raster[0], 1, &scrolly[0], cliprect);
	else
		bitmap.fill(0x0ff);

	if (state->m_dispflag[1])
	{
		if (state->m_gfxdraw_mode == 1)
			// nbmj9195 2layer
			copyscrollbitmap_trans(bitmap, state->m_tmpbitmap[1], SCANLINE_MAX, state->m_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x0ff);

		if (state->m_gfxdraw_mode == 2)
			// nbmj9195 nb22090 2layer
			copyscrollbitmap_trans(bitmap, state->m_tmpbitmap[1], SCANLINE_MAX, state->m_scrollx_raster[1], 1, &scrolly[1], cliprect, 0x1ff);
	}
	return 0;
}
