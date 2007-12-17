/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "includes/phoenix.h"

UINT8 *naughtyb_videoram2;

static int videoreg;

/* use these to draw charset B */
UINT8 *naughtyb_scrollreg;

/* use this to select palette */
static UINT8 palreg;

/* used in Naughty Boy to select video bank */
static int bankreg;

// true if cabinet == cocktail -AND- handling player 2
int naughtyb_cocktail;

static rectangle scrollvisiblearea =
{
	2*8, 34*8-1,
	0*8, 28*8-1
};

static rectangle leftvisiblearea =
{
	0*8, 2*8-1,
	0*8, 28*8-1
};

static rectangle rightvisiblearea =
{
	34*8, 36*8-1,
	0*8, 28*8-1
};



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Naughty Boy has two 256x4 palette PROMs, one containing the high bits and
  the other the low bits (2x2x2 color space).
  The palette PROMs are connected to the RGB output this way:

  bit 3 --
        -- 270 ohm resistor  -- GREEN
        -- 270 ohm resistor  -- BLUE
  bit 0 -- 270 ohm resistor  -- RED

  bit 3 --
        -- GREEN
        -- BLUE
  bit 0 -- RED

  plus 270 ohm pullup and pulldown resistors on all lines

***************************************************************************/
PALETTE_INIT( naughtyb )
{
	int i;
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	/* note: there is no resistor on second PROM so we define second resistance as 0 */
	static const int resistances[2] = { 270, 0 };
	double weights_r[2], weights_g[2], weights_b[2];


	compute_resistor_weights(0,	255,	-1.0,
			2,	resistances,	weights_r,	270,	270,
			2,	resistances,	weights_g,	270,	270,
			2,	resistances,	weights_b,	270,	270);


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;

		/*r = 0x55 * bit0 + 0xaa * bit1;*/
		r = combine_2_weights(weights_r, bit0, bit1);

		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;

		/*g = 0x55 * bit0 + 0xaa * bit1;*/
		g = combine_2_weights(weights_g, bit0, bit1);

		bit0 = (color_prom[0] >> 1) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;

		/*b = 0x55 * bit0 + 0xaa * bit1;*/
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* first bank of characters use colors 0-31, 64-95, 128-159 and 192-223 */
	for (i = 0;i < 8;i++)
	{
		int j;


		for (j = 0;j < 4;j++)
		{
			COLOR(0,4*i + j*4*8) = i + j*64;
			COLOR(0,4*i + j*4*8 + 1) = 8 + i + j*64;
			COLOR(0,4*i + j*4*8 + 2) = 2*8 + i + j*64;
			COLOR(0,4*i + j*4*8 + 3) = 3*8 + i + j*64;
		}
	}

	/* second bank of characters use colors 32-63, 96-127, 160-191 and 224-255 */
	for (i = 0;i < 8;i++)
	{
		int j;


		for (j = 0;j < 4;j++)
		{
			COLOR(1,4*i + j*4*8) = i + 32 + j*64;
			COLOR(1,4*i + j*4*8 + 1) = 8 + i + 32 + j*64;
			COLOR(1,4*i + j*4*8 + 2) = 2*8 + i + 32 + j*64;
			COLOR(1,4*i + j*4*8 + 3) = 3*8 + i + 32 + j*64;
		}
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( naughtyb )
{
	videoreg = palreg = bankreg = 0;

	/* Naughty Boy has a virtual screen twice as large as the visible screen */
	dirtybuffer = auto_malloc(videoram_size);
	memset(dirtybuffer, 1, videoram_size);

	tmpbitmap = auto_bitmap_alloc(68*8,28*8,machine->screen[0].format);
}




WRITE8_HANDLER( naughtyb_videoram2_w )
{
	if (naughtyb_videoram2[offset] != data)
	{
		dirtybuffer[offset] = 1;

		naughtyb_videoram2[offset] = data;
	}
}



WRITE8_HANDLER( naughtyb_videoreg_w )
{
	// bits 4+5 control the sound circuit

	pleiads_sound_control_c_w(offset,data);

    if ((videoreg & 0x0f) != (data & 0x0f))
	{
		videoreg = data;

		naughtyb_cocktail =
			( ( input_port_2_r(0) & 0x80 ) &&	// cabinet == cocktail
			  ( data & 0x01 ) );				// handling player 2
		palreg  = (data >> 1) & 0x03;			// pallette sel is bit 1 & 2
		bankreg = (data >> 2) & 0x01;			// banksel is just bit 2

		memset (dirtybuffer, 1, videoram_size);
	}
}

WRITE8_HANDLER( popflame_videoreg_w )
{
	// bits 4+5 control the sound circuit

	pleiads_sound_control_c_w(offset,data);

    if ((videoreg & 0x0f) != (data & 0x0f))
	{
		videoreg = data;

		naughtyb_cocktail =
			( ( input_port_2_r(0) & 0x80 ) &&	// cabinet == cocktail
			  ( data & 0x01 ) );				// handling player 2
		palreg  = (data >> 1) & 0x03;			// pallette sel is bit 1 & 2
		bankreg = (data >> 3) & 0x01;			// banksel is just bit 3

		memset (dirtybuffer, 1, videoram_size);
	}
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
VIDEO_UPDATE( naughtyb )
{
	int offs;

	// for every character in the Video RAM, check if it has been modified
	// since last time and update it accordingly.

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		if (dirtybuffer[offs])
		{
			int sx,sy;

			dirtybuffer[offs] = 0;

			if ( naughtyb_cocktail )
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

			drawgfx(tmpbitmap,machine->gfx[0],
					naughtyb_videoram2[offs] + 256 * bankreg,
					(naughtyb_videoram2[offs] >> 5) + 8 * palreg,
					naughtyb_cocktail,naughtyb_cocktail,
					8*sx,8*sy,
					0,TRANSPARENCY_NONE,0);

			drawgfx(tmpbitmap,machine->gfx[1],
					videoram[offs] + 256*bankreg,
					(videoram[offs] >> 5) + 8 * palreg,
					naughtyb_cocktail,naughtyb_cocktail,
					8*sx,8*sy,
					0,TRANSPARENCY_PEN,0);
		}
	}

	// copy the temporary bitmap to the screen
	{
		int scrollx;

		copybitmap(bitmap,tmpbitmap,0,0,-66*8,0,&leftvisiblearea,TRANSPARENCY_NONE,0);
		copybitmap(bitmap,tmpbitmap,0,0,-30*8,0,&rightvisiblearea,TRANSPARENCY_NONE,0);

		scrollx = ( naughtyb_cocktail ) ? *naughtyb_scrollreg - 239 : -*naughtyb_scrollreg + 16;
		copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,0,0,&scrollvisiblearea,TRANSPARENCY_NONE,0);
	}
	return 0;
}
