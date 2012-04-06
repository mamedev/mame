/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  TODO:

  - Get a dump of the PAL16L8 (IC38 - 4M).  This controls transparency.
    It takes as input 4 bits of sprite color, and 6 bits of background tile
    color.  It outputs a sprite enable line, a background enable line, and
    a background select (which layer of background to draw).
  - Add cocktail flipping.

***************************************************************************/

#include "emu.h"
#include "includes/vigilant.h"


static const rectangle bottomvisiblearea(16*8, 48*8-1, 6*8, 32*8-1);


VIDEO_START( vigilant )
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	state->m_bg_bitmap = auto_bitmap_ind16_alloc(machine,512*4,256);
}


VIDEO_RESET( vigilant )
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	state->m_horiz_scroll_low = 0;
	state->m_horiz_scroll_high = 0;
	state->m_rear_horiz_scroll_low = 0;
	state->m_rear_horiz_scroll_high = 0;
	state->m_rear_color = 0;
	state->m_rear_disable = 1;
	state->m_rear_refresh = 1;
}


/***************************************************************************
 update_background

 There are three background ROMs, each one contains a 512x256 picture.
 Redraw them if the palette changes.
 **************************************************************************/
static void update_background(running_machine &machine)
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	int row,col,page;
	int charcode;


	charcode=0;

	/* There are only three background ROMs (4 on bunccaneers!) */
	for (page=0; page<4; page++)
	{
		for( row=0; row<256; row++ )
		{
			for( col=0; col<512; col+=32 )
			{
				drawgfx_opaque(*state->m_bg_bitmap,
						state->m_bg_bitmap->cliprect(),machine.gfx[2],
						charcode,
						row < 128 ? 0 : 1,
						0,0,
						512*page + col,row);
				charcode++;
			}
		}
	}
}

/***************************************************************************
 vigilant_paletteram_w

 There are two palette chips, each one is labelled "KNA91H014".  One is
 used for the sprites, one is used for the two background layers.

 The chip has three enables (!CS, !E0, !E1), R/W pins, A0-A7 as input,
 'L' and 'H' inputs, and D0-D4 as input.  'L' and 'H' are used to bank
 into Red, Green, and Blue memory.  There are only 5 bits of memory for
 each byte, and a total of 256*3 bytes memory per chip.

 There are additionally two sets of D0-D7 inputs per chip labelled 'A'
 and 'B'.  There is also an 'S' pin to select between the two input sets.
 These are used to index a color triplet of RGB.  The triplet is read
 from RAM, and output to R0-R4, G0-G4, and B0-B4.
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_paletteram_w)
{
	int bank,r,g,b;


	m_generic_paletteram_8[offset] = data;

	bank = offset & 0x400;
	offset &= 0xff;

	r = (m_generic_paletteram_8[bank + offset + 0x000] << 3) & 0xFF;
	g = (m_generic_paletteram_8[bank + offset + 0x100] << 3) & 0xFF;
	b = (m_generic_paletteram_8[bank + offset + 0x200] << 3) & 0xFF;

	palette_set_color(machine(), (bank >> 2) + offset,MAKE_RGB(r,g,b));
}



/***************************************************************************
 vigilant_horiz_scroll_w

 horiz_scroll_low  = HSPL, an 8-bit register
 horiz_scroll_high = HSPH, a 1-bit register
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_horiz_scroll_w)
{
	if (offset==0)
		m_horiz_scroll_low = data;
	else
		m_horiz_scroll_high = (data & 0x01) * 256;
}

/***************************************************************************
 vigilant_rear_horiz_scroll_w

 rear_horiz_scroll_low  = RHSPL, an 8-bit register
 rear_horiz_scroll_high = RHSPH, an 8-bit register but only 3 bits are saved
***************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_rear_horiz_scroll_w)
{
	if (offset==0)
		m_rear_horiz_scroll_low = data;
	else
		m_rear_horiz_scroll_high = (data & 0x07) * 256;
}

/***************************************************************************
 vigilant_rear_color_w

 This is an 8-bit register labelled RCOD.
 D6 is hooked to !ROME (rear_disable)
 D3 = RCC2 (rear color bit 2)
 D2 = RCC1 (rear color bit 1)
 D0 = RCC0 (rear color bit 0)

 I know it looks odd, but D1, D4, D5, and D7 are empty.

 What makes this extremely odd is that RCC is supposed to hook up to the
 palette.  However, the top four bits of the palette inputs are labelled:
 "RCC3", "RCC2", "V256E", "RCC0".  Methinks there's a typo.
 **************************************************************************/
WRITE8_MEMBER(vigilant_state::vigilant_rear_color_w)
{
	m_rear_disable = data & 0x40;
	m_rear_color = (data & 0x0d);
}

/***************************************************************************
 draw_foreground

 ???
 **************************************************************************/

static void draw_foreground(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque )
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	UINT8 *videoram = state->m_videoram;
	int offs;
	int scroll = -(state->m_horiz_scroll_low + state->m_horiz_scroll_high);


	for (offs = 0; offs < 0x1000; offs += 2)
	{
		int sy = 8 * ((offs/2) / 64);
		int sx = 8 * ((offs/2) % 64);
		int attributes = videoram[offs+1];
		int color = attributes & 0x0F;
		int tile_number = videoram[offs] | ((attributes & 0xF0) << 4);

		if (priority)	 /* foreground */
		{
			if ((color & 0x0c) == 0x0c)	/* mask sprites */
			{
				if (sy >= 48)
				{
					sx = (sx + scroll) & 0x1ff;

					drawgfx_transmask(bitmap,bottomvisiblearea,machine.gfx[0],
							tile_number,
							color,
							0,0,
							sx,sy,0x00ff);
				}
			}
		}
		else	 /* background */
		{
			if (sy >= 48)
				sx = (sx + scroll) & 0x1ff;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					tile_number,
					color,
					0,0,
					sx,sy,
					(opaque || color >= 4) ? -1 : 0);
		}
	}
}



static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	int scrollx = 0x17a + 16*8 - (state->m_rear_horiz_scroll_low + state->m_rear_horiz_scroll_high);


	if (state->m_rear_refresh)
	{
		update_background(machine);
		state->m_rear_refresh=0;
	}

	copyscrollbitmap(bitmap,*state->m_bg_bitmap,1,&scrollx,0,0,bottomvisiblearea);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	vigilant_state *state = machine.driver_data<vigilant_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram_size;offs += 8)
	{
		int code,color,sx,sy,flipx,flipy,h,y;

		code = spriteram[offs+4] | ((spriteram[offs+5] & 0x0f) << 8);
		color = spriteram[offs+0] & 0x0f;
		sx = (spriteram[offs+6] | ((spriteram[offs+7] & 0x01) << 8));
		sy = 256+128 - (spriteram[offs+2] | ((spriteram[offs+3] & 0x01) << 8));
		flipx = spriteram[offs+5] & 0x40;
		flipy = spriteram[offs+5] & 0x80;
		h = 1 << ((spriteram[offs+5] & 0x30) >> 4);
		sy -= 16 * h;

		code &= ~(h - 1);

		for (y = 0;y < h;y++)
		{
			int c = code;

			if (flipy) c += h-1-y;
			else c += y;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					c,
					color,
					flipx,flipy,
					sx,sy + 16*y,0);
		}
	}
}

SCREEN_UPDATE_IND16( kikcubic )
{
	vigilant_state *state = screen.machine().driver_data<vigilant_state>();
	UINT8 *videoram = state->m_videoram;
	int offs;

	for (offs = 0; offs < 0x1000; offs += 2)
	{
		int sy = 8 * ((offs/2) / 64);
		int sx = 8 * ((offs/2) % 64);
		int attributes = videoram[offs+1];
		int color = (attributes & 0xF0) >> 4;
		int tile_number = videoram[offs] | ((attributes & 0x0F) << 8);

		drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],
				tile_number,
				color,
				0,0,
				sx,sy);
	}

	draw_sprites(screen.machine(),bitmap,cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( vigilant )
{
	vigilant_state *state = screen.machine().driver_data<vigilant_state>();
	int i;

	/* copy the background palette */
	for (i = 0;i < 16;i++)
	{
		int r,g,b;


		r = (state->m_generic_paletteram_8[0x400 + 16 * state->m_rear_color + i] << 3) & 0xFF;
		g = (state->m_generic_paletteram_8[0x500 + 16 * state->m_rear_color + i] << 3) & 0xFF;
		b = (state->m_generic_paletteram_8[0x600 + 16 * state->m_rear_color + i] << 3) & 0xFF;

		palette_set_color(screen.machine(),512 + i,MAKE_RGB(r,g,b));

		r = (state->m_generic_paletteram_8[0x400 + 16 * state->m_rear_color + 32 + i] << 3) & 0xFF;
		g = (state->m_generic_paletteram_8[0x500 + 16 * state->m_rear_color + 32 + i] << 3) & 0xFF;
		b = (state->m_generic_paletteram_8[0x600 + 16 * state->m_rear_color + 32 + i] << 3) & 0xFF;

		palette_set_color(screen.machine(),512 + 16 + i,MAKE_RGB(r,g,b));
	}

	if (state->m_rear_disable)	 /* opaque foreground */
	{
		draw_foreground(screen.machine(),bitmap,cliprect,0,1);
		draw_sprites(screen.machine(),bitmap,bottomvisiblearea);
		draw_foreground(screen.machine(),bitmap,cliprect,1,0);
	}
	else
	{
		draw_background(screen.machine(),bitmap,cliprect);
		draw_foreground(screen.machine(),bitmap,cliprect,0,0);
		draw_sprites(screen.machine(),bitmap,bottomvisiblearea);
		draw_foreground(screen.machine(),bitmap,cliprect,1,0); // priority tiles
	}
	return 0;
}
