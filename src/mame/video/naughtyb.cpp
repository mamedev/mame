// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Sal and John Bugliarisi,Paul Priest
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/naughtyb.h"


#if 0
static const res_net_decode_info naughtyb_decode_info =
{
	2,      /*  two proms          */
	0,      /*  start at 0         */
	255,    /*  end at 255         */
	/*  R,   G,   B,   R,   G,   B */
	{   0,   0,   0, 256, 256, 256},        /*  offsets */
	{   0,   2,   1,  -1,   1,   0},        /*  shifts */
	{0x01,0x01,0x01,0x02,0x02,0x02}         /*  masks */
};

static const res_net_info naughtyb_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 130, 270, 2, { 270, 1 } }, /* no resistor for bit1 */
		{ RES_NET_AMP_NONE, 130, 270, 2, { 270, 1 } }, /* no resistor for bit1 */
		{ RES_NET_AMP_NONE, 130, 270, 2, { 270, 1 } }  /* no resistor for bit1 */
	}
};
#endif

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Naughty Boy has two 256x4 palette PROMs, one containing the high bits and
  the other the low bits (2x2x2 color space).
  The palette PROMs are connected to a 7407 (hex open collector buffer),
  which in turn is connected to the RGB output this way:

  bit 3 --
        -- 270 ohm resistor  -- GREEN
        -- 270 ohm resistor  -- BLUE
  bit 0 -- 270 ohm resistor  -- RED

  bit 3 --
        -- GREEN
        -- BLUE
  bit 0 -- RED

  plus 130 ohm pullup and 270 ohm pulldown resistors on all lines

***************************************************************************/

PALETTE_INIT_MEMBER(naughtyb_state, naughtyb)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[2] = { 270, 130 };
	double weights[2];

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			2, resistances, weights, 0, 0,
			2, resistances, weights, 0, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0;i < palette.entries(); i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 0) & 0x01;
		r = combine_2_weights(weights, bit0, bit1);

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i+0x100] >> 2) & 0x01;
		g = combine_2_weights(weights, bit0, bit1);

		/* blue component */
		bit0 = (color_prom[i] >> 1) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		b = combine_2_weights(weights, bit0, bit1);

		palette.set_pen_color(BITSWAP8(i,5,7,6,2,1,0,4,3), rgb_t(r, g, b));
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void naughtyb_state::video_start()
{
	m_palreg = m_bankreg = 0;

	/* Naughty Boy has a virtual screen twice as large as the visible screen */
	m_tmpbitmap.allocate(68*8,28*8,m_screen->format());

	save_item(NAME(m_cocktail));
	save_item(NAME(m_palreg));
	save_item(NAME(m_bankreg));
}




WRITE8_MEMBER(naughtyb_state::naughtyb_videoreg_w)
{
	// bits 4+5 control the sound circuit
	m_naughtyb_custom->control_c_w(space,offset,data);

	m_cocktail =
		( ( ioport("DSW0")->read() & 0x80 ) &&  // cabinet == cocktail
			( data & 0x01 ) );              // handling player 2
	m_palreg  = (data >> 1) & 0x03;         // palette sel is bit 1 & 2
	m_bankreg = (data >> 2) & 0x01;         // banksel is just bit 2
}

WRITE8_MEMBER(naughtyb_state::popflame_videoreg_w)
{
	// bits 4+5 control the sound circuit
	m_popflame_custom->control_c_w(space,offset,data);

	m_cocktail =
		( ( ioport("DSW0")->read() & 0x80 ) &&  // cabinet == cocktail
			( data & 0x01 ) );              // handling player 2
	m_palreg  = (data >> 1) & 0x03;         // palette sel is bit 1 & 2
	m_bankreg = (data >> 3) & 0x01;         // banksel is just bit 3
}



/***************************************************************************

  The Naughty Boy screen is split into two sections by the hardware

  NonScrolled = 28x4 - (rows 0,1,34,35, as shown below)
  this area is split between the top and bottom of the screen,
  and the address mapping is really funky.

  Scrolled = 28x64, with a 28x32 viewport, as shown below
  Each column in the virtual screen is 64 (40h) characters high.
  Thus, column 27 is stored in VRAm at address 0-3fh,
  column 26 is stored at 40-7f, and so on.
  This illustration shows the horizonal scroll register set to zero,
  so the topmost 32 rows of the virtual screen are shown.

  The following screen-to-memory mapping. This is shown from player's viewpoint,
  which with the CRT rotated 90 degrees CCW. This example shows the horizonal
  scroll register set to zero.


                          COLUMN
                0   1   2    -    25  26  27
              -------------------------------
            0| 76E 76A 762   -   70A 706 702 |
             |                               |  Nonscrolled display
            1| 76F 76B 762   -   70B 707 703 |
             |-------------------------------| -----
            2| 6C0 680 640   -    80  40  00 |
             |                               |
        R   3| 6C1 681 641   -    81  41  01 |
        O    |                               |  28 x 32 viewport
        W   ||      |                 |      |  into 28x64 virtual,
             |                               |  scrollable screen
           32| 6DE 69E 65E        9E  5E  1E |
             |                               |
           33| 6DF 69F 65F   -    9F  5F  1F |
             |-------------------------------| -----
           34| 76C 768 764       708 704 700 |
             |                               |  Nonscrolled display
           35| 76D 769 765       709 705 701 |
              -------------------------------


***************************************************************************/
UINT32 naughtyb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle scrollvisiblearea(2*8, 34*8-1, 0*8, 28*8-1);
	const rectangle leftvisiblearea(0*8, 2*8-1, 0*8, 28*8-1);
	const rectangle rightvisiblearea(34*8, 36*8-1, 0*8, 28*8-1);

	// for every character in the Video RAM

	for (int offs = 0x800 - 1; offs >= 0; offs--)
	{
		int sx,sy;

		if ( m_cocktail )
		{
			if (offs < 0x700)
			{
				sx = 63 - offs % 64;
				sy = 27 - offs / 64;
			}
			else
			{
				sx = 64 + ( 3 - (offs - 0x700) % 4 );
				sy = 27 - (offs - 0x700) / 4;
			}
		}
		else
		{
			if (offs < 0x700)
			{
				sx = offs % 64;
				sy = offs / 64;
			}
			else
			{
				sx = 64 + (offs - 0x700) % 4;
				sy = (offs - 0x700) / 4;
			}
		}

		m_gfxdecode->gfx(0)->opaque(m_tmpbitmap,m_tmpbitmap.cliprect(),
				m_videoram2[offs] + 256 * m_bankreg,
				(m_videoram2[offs] >> 5) + 8 * m_palreg,
				m_cocktail,m_cocktail,
				8*sx,8*sy);

		m_gfxdecode->gfx(1)->transpen(m_tmpbitmap,m_tmpbitmap.cliprect(),
				m_videoram[offs] + 256*m_bankreg,
				(m_videoram[offs] >> 5) + 8 * m_palreg,
				m_cocktail,m_cocktail,
				8*sx,8*sy,0);
	}

	// copy the temporary bitmap to the screen
	{
		int scrollx;

		copybitmap(bitmap,m_tmpbitmap,0,0,-66*8,0,leftvisiblearea);
		copybitmap(bitmap,m_tmpbitmap,0,0,-30*8,0,rightvisiblearea);

		scrollx = ( m_cocktail ) ? *m_scrollreg - 239 : -*m_scrollreg + 16;
		copyscrollbitmap(bitmap,m_tmpbitmap,1,&scrollx,0,nullptr,scrollvisiblearea);
	}
	return 0;
}
