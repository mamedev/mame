/***************************************************************************

    Atari Crystal Castles hardware

***************************************************************************/

#include "driver.h"
#include "ccastles.h"
#include "video/resnet.h"



/*************************************
 *
 *  Globals
 *
 *************************************/

static double rweights[3], gweights[3], bweights[3];
static mame_bitmap *spritebitmap;

static UINT8 video_control[8];
static UINT8 bitmode_addr[2];
static UINT8 hscroll;
static UINT8 vscroll;

static const UINT8 *syncprom;
static const UINT8 *wpprom;
static const UINT8 *priprom;



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( ccastles )
{
	static const int resistances[3] = { 22000, 10000, 4700 };

	/* get pointers to our PROMs */
	syncprom = memory_region(REGION_PROMS) + 0x000;
	wpprom = memory_region(REGION_PROMS) + 0x200;
	priprom = memory_region(REGION_PROMS) + 0x300;

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0,	255, -1.0,
			3,	resistances, rweights, 1000, 0,
			3,	resistances, gweights, 1000, 0,
			3,	resistances, bweights, 1000, 0);

	/* allocate a bitmap for drawing sprites */
	spritebitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);

	/* register for savestates */
	state_save_register_global_array(video_control);
	state_save_register_global_array(bitmode_addr);
	state_save_register_global(hscroll);
	state_save_register_global(vscroll);
}



/*************************************
 *
 *  Video control registers
 *
 *************************************/

WRITE8_HANDLER( ccastles_hscroll_w )
{
	video_screen_update_partial(0, video_screen_get_vpos(0));
	hscroll = data;
}


WRITE8_HANDLER( ccastles_vscroll_w )
{
	vscroll = data;
}


WRITE8_HANDLER( ccastles_video_control_w )
{
	/* only D3 matters */
	video_control[offset] = (data >> 3) & 1;
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

WRITE8_HANDLER( ccastles_paletteram_w )
{
	int bit0, bit1, bit2;
	int r, g, b;

	/* extract the raw RGB bits */
	r = ((data & 0xc0) >> 6) | ((offset & 0x20) >> 3);
	b = (data & 0x38) >> 3;
	g = (data & 0x07);

	/* red component (inverted) */
	bit0 = (~r >> 0) & 0x01;
	bit1 = (~r >> 1) & 0x01;
	bit2 = (~r >> 2) & 0x01;
	r = combine_3_weights(rweights, bit0, bit1, bit2);

	/* green component (inverted) */
	bit0 = (~g >> 0) & 0x01;
	bit1 = (~g >> 1) & 0x01;
	bit2 = (~g >> 2) & 0x01;
	g = combine_3_weights(gweights, bit0, bit1, bit2);

	/* blue component (inverted) */
	bit0 = (~b >> 0) & 0x01;
	bit1 = (~b >> 1) & 0x01;
	bit2 = (~b >> 2) & 0x01;
	b = combine_3_weights(bweights, bit0, bit1, bit2);

	palette_set_color(Machine, offset & 0x1f, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

INLINE void ccastles_write_vram(UINT16 addr, UINT8 data, UINT8 bitmd, UINT8 pixba)
{
	UINT8 *dest = &videoram[addr & 0x7ffe];
	UINT8 promaddr = 0;
	UINT8 wpbits;

	/*
        Inputs to the write-protect PROM:

        Bit 7 = BA1520 = 0 if (BA15-BA12 != 0), or 1 otherwise
        Bit 6 = DRBA11
        Bit 5 = DRBA10
        Bit 4 = /BITMD
        Bit 3 = GND
        Bit 2 = BA0
        Bit 1 = PIXB
        Bit 0 = PIXA
    */
	promaddr |= ((addr & 0xf000) == 0) << 7;
	promaddr |= (addr & 0x0c00) >> 5;
	promaddr |= (!bitmd) << 4;
	promaddr |= (addr & 0x0001) << 2;
	promaddr |= (pixba << 0);

	/* look up the PROM result */
	wpbits = wpprom[promaddr];

	/* write to the appropriate parts of VRAM depending on the result */
	if (!(wpbits & 1))
		dest[0] = (dest[0] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 2))
		dest[0] = (dest[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 4))
		dest[1] = (dest[1] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 8))
		dest[1] = (dest[1] & 0x0f) | (data & 0xf0);
}



/*************************************
 *
 *  Autoincrement control for bit mode
 *
 *************************************/

INLINE void bitmode_autoinc(void)
{
	/* auto increment in the x-direction if it's enabled */
	if (!video_control[0])	/* /AX */
	{
		if (!video_control[2])	/* /XINC */
			bitmode_addr[0]++;
		else
			bitmode_addr[0]--;
	}

	/* auto increment in the y-direction if it's enabled */
	if (!video_control[1])	/* /AY */
	{
		if (!video_control[3])	/* /YINC */
			bitmode_addr[1]++;
		else
			bitmode_addr[1]--;
	}
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

WRITE8_HANDLER( ccastles_videoram_w )
{
	/* direct writes to VRAM go through the write protect PROM as well */
	ccastles_write_vram(offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

READ8_HANDLER( ccastles_bitmode_r )
{
	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (bitmode_addr[1] << 7) | (bitmode_addr[0] >> 1);

	/* the appropriate pixel is selected into the upper 4 bits */
	UINT8 result = videoram[addr] << ((~bitmode_addr[0] & 1) * 4);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();

	/* the low 4 bits of the data lines are not driven so make them all 1's */
	return result | 0x0f;
}


WRITE8_HANDLER( ccastles_bitmode_w )
{
	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (bitmode_addr[1] << 7) | (bitmode_addr[0] >> 1);

	/* the upper 4 bits of data are replicated to the lower 4 bits */
	data = (data & 0xf0) | (data >> 4);

	/* write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA */
	ccastles_write_vram(addr, data, 1, bitmode_addr[0] & 3);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();
}


WRITE8_HANDLER( ccastles_bitmode_addr_w )
{
	/* write through to video RAM and also to the addressing latches */
	ccastles_write_vram(offset, data, 0, 0);
	bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

VIDEO_UPDATE( ccastles )
{
	UINT8 *spriteaddr = &spriteram[video_control[7] * 0x100];	/* BUF1/BUF2 */
	int flip = video_control[4] ? 0xff : 0x00;	/* PLAYER2 */
	pen_t black = get_black_pen(machine);
	int x, y, offs;

	/* draw the sprites */
	fillbitmap(spritebitmap, 0x0f, cliprect);
	for (offs = 0; offs < 320/2; offs += 4)
	{
		int x = spriteaddr[offs+3];
		int y = 256 - 16 - spriteaddr[offs+1];
		int which = spriteaddr[offs];
		int color = spriteaddr[offs+2] >> 7;

		drawgfx(spritebitmap, machine->gfx[0], which, color, flip, flip, x, y, cliprect, TRANSPARENCY_PEN, 7);
	}

	/* draw the bitmap to the screen, looping over Y */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* if we're in the VBLANK region, just fill with black */
		if (syncprom[y] & 1)
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dst[x] = black;
		}

		/* non-VBLANK region: merge the sprites and the bitmap */
		else
		{
			UINT16 *mosrc = (UINT16 *)spritebitmap->base + y * spritebitmap->rowpixels;
			int effy = (((y - ccastles_vblank_end) + (flip ? 0 : vscroll)) ^ flip) & 0xff;
			UINT8 *src;

			/* the "POTATO" chip does some magic here; this is just a guess */
			if (effy < 24)
				effy = 24;
			src = &videoram[effy * 128];

			/* loop over X */
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				/* if we're in the HBLANK region, just store black */
				if (x >= 256)
					dst[x] = black;

				/* otherwise, process normally */
				else
				{
					int effx = (hscroll + (x ^ flip)) & 255;

					/* low 4 bits = left pixel, high 4 bits = right pixel */
					UINT8 pix = (src[effx / 2] >> ((effx & 1) * 4)) & 0x0f;
					UINT8 mopix = mosrc[x];
					UINT8 prindex, prvalue;

					/* Inputs to the priority PROM:

                        Bit 7 = GND
                        Bit 6 = /CRAM
                        Bit 5 = BA4
                        Bit 4 = MV2
                        Bit 3 = MV1
                        Bit 2 = MV0
                        Bit 1 = MPI
                        Bit 0 = BIT3
                    */
					prindex = 0x40;
					prindex |= (mopix & 7) << 2;
					prindex |= (mopix & 8) >> 2;
					prindex |= (pix & 8) >> 3;
					prvalue = priprom[prindex];

					/* Bit 1 of prvalue selects the low 4 bits of the final pixel */
					if (prvalue & 2)
						pix = mopix;

					/* Bit 0 of prvalue selects bit 4 of the final color */
					pix |= (prvalue & 1) << 4;

					/* store the pixel value and also a priority value based on the topmost bit */
					dst[x] = pix;
				}
			}
		}
	}
	return 0;
}
