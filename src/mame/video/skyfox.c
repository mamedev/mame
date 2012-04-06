/***************************************************************************

                        -= Sky Fox / Exerizer =-

                driver by   Luca Elia (l.elia@tin.it)


                            [ 1 Background ]

    The stars in the background are not tile based (I think!) and
    their rendering is entirely guesswork for now..

    I draw a star for each horizontal line using 2 bytes in the
    background rom:

    - the first byte seems a color / shape info
    - the second byte seems a position info

    The rom holds 4 chunks of $2000 bytes. Most of the data does not
    change between chunks, while the remaining part (which is rendered
    to what seems a "milky way") pulsates in color and/or shape
    to simulate the shimmering of stars (?!) if we draw one chunk only
    and cycle through the four. Indeed, there's a register cycling
    through 4 values.

    Since the result kind of matches a screenshot we have, I feel the
    drawn result is not that far from reality. On the other hand we
    have a random arrangement of stars, so it's hard to tell for sure..

                            [ 256 Sprites ]

    Sprites are 8 planes deep and can be 8x8, 16x16 or 32x32 pixels
    in size. They are stored as 32x32x8 tiles in the ROMs.


***************************************************************************/

#include "emu.h"
#include "includes/skyfox.h"


/***************************************************************************

                            Memory Handlers

***************************************************************************/

#ifdef UNUSED_FUNCTION
READ8_MEMBER(skyfox_state::skyfox_vregs_r)// for debug
{
	return m_vreg[offset];
}
#endif

WRITE8_MEMBER(skyfox_state::skyfox_vregs_w)
{

	m_vreg[offset] = data;

	switch (offset)
	{
		case 0:	m_bg_ctrl = data;	break;
		case 1:	soundlatch_w(space, 0, data);	break;
		case 2:	break;
		case 3:	break;
		case 4:	break;
		case 5:	break;
		case 6:	break;
		case 7:	break;
	}
}



/***************************************************************************

  Convert the color PROMs into a more useable format.

  There are three 256x4 palette PROMs (one per gun).
  I don't know the exact values of the resistors between the RAM and the
  RGB output. I assumed these values (the same as Commando)

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( skyfox )
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	/* Grey scale for the background??? */
	for (i = 0; i < 256; i++)
	{
		palette_set_color(machine,i + 256, MAKE_RGB(i, i, i));
	}
}



/***************************************************************************

                                Sprites Drawing

Offset:         Value:

03              Code: selects one of the 32x32 tiles in the ROMs.
                (Tiles $80-ff are bankswitched to cover $180 tiles)

02              Code + Attr

                    7654 ----   Code (low 4 bits)
                                8x8   sprites use bits 7654 (since there are 16 8x8  tiles in the 32x32 one)
                                16x16 sprites use bits --54 (since there are 4 16x16 tiles in the 32x32 one)
                                32x32 sprites use no bits   (since the 32x32 tile is already selected)

                    7--- 3---   Size
                                1--- 1--- : 32x32 sprites
                                0--- 1--- : 16x16 sprites
                                8x8 sprites otherwise

                    ---- -2--   Flip Y
                    ---- --1-   Flip X
                    ---- ---0   X Low Bit

00              Y

01              X (High 8 Bits)

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	skyfox_state *state = machine.driver_data<skyfox_state>();
	int offs;

	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	/* The 32x32 tiles in the 80-ff range are bankswitched */
	int shift =(state->m_bg_ctrl & 0x80) ? (4 - 1) : 4;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int xstart, ystart, xend, yend;
		int xinc, yinc, dx, dy;
		int low_code, high_code, n;

		int y = state->m_spriteram[offs + 0];
		int x = state->m_spriteram[offs + 1];
		int code = state->m_spriteram[offs + 2] + state->m_spriteram[offs + 3] * 256;
		int flipx = code & 0x2;
		int flipy = code & 0x4;

		x = x * 2 + (code & 1);	// add the least significant bit

		high_code = ((code >> 4) & 0x7f0) + ((code & 0x8000) >> shift);

		switch( code & 0x88 )
		{
			case 0x88:	n = 4; low_code = 0;										break;
			case 0x08:	n = 2; low_code = ((code & 0x20) ? 8 : 0) + ((code & 0x10) ? 2 : 0);	break;
			default:	n = 1; low_code = (code >> 4) & 0xf;
		}

#define DRAW_SPRITE(DX,DY,CODE) \
		drawgfx_transpen(bitmap,\
				cliprect,machine.gfx[0], \
				(CODE), \
				0, \
				flipx,flipy, \
				x + (DX),y + (DY), 0xff); \

		if (state->m_bg_ctrl & 1)	// flipscreen
		{
			x = width  - x - (n - 1) * 8;
			y = height - y - (n - 1) * 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)	{ xstart = n - 1;  xend = -1;  xinc = -1; }
		else		{ xstart = 0;      xend = n;   xinc = +1; }

		if (flipy)	{ ystart = n - 1;  yend = -1;  yinc = -1; }
		else		{ ystart = 0;      yend = n;   yinc = +1; }


		code = low_code + high_code;

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc)
				DRAW_SPRITE(dx * 8, dy * 8, code++);

			if (n == 2)	code += 2;
		}
	}
}





/***************************************************************************

                            Background Rendering

***************************************************************************/

static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skyfox_state *state = machine.driver_data<skyfox_state>();
	UINT8 *RAM = machine.region("gfx2")->base();
	int x, y, i;

	/* The foreground stars (sprites) move at twice this speed when
       the bg scroll rate [e.g. (skyfox_bg_reg >> 1) & 7] is 4 */
	int pos = (state->m_bg_pos >> 4) & (512 * 2 - 1);

	for (i = 0 ; i < 0x1000; i++)
	{
		int pen, offs, j;

		offs	= (i * 2 + ((state->m_bg_ctrl >> 4) & 0x3) * 0x2000) % 0x8000;

		pen = RAM[offs];
		x = RAM[offs + 1] * 2 + (i & 1) + pos + ((i & 8) ? 512 : 0);
		y = ((i / 8) / 2) * 8 + (i % 8);

		if (state->m_bg_ctrl & 1)	// flipscreen
		{
			x = 512 * 2 - (x % (512 * 2));
			y = 256     - (y % 256);
		}

		for (j = 0 ; j <= ((pen & 0x80) ? 0 : 3); j++)
			bitmap.pix16(
						   (((j / 2) & 1) + y) % 256,
						   ((j & 1)     + x) % 512) = 256 + (pen & 0x7f);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


SCREEN_UPDATE_IND16( skyfox )
{
	bitmap.fill(255, cliprect);	// the bg is black
	draw_background(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
