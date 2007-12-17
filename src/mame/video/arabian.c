/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "arabian.h"


/* Constants */
#define BITMAP_WIDTH		256
#define BITMAP_HEIGHT		256


/* Local variables */
static UINT8 *main_bitmap;
static UINT8 *converted_gfx;


/* Globals */
UINT8 arabian_video_control;
UINT8 arabian_flip_screen;



/*************************************
 *
 *  Color PROM conversion
 *
 *************************************/

PALETTE_INIT( arabian )
{
	int i;

	/* there are effectively 6 bits of color: 2 red, 2 green, 2 blue */
	for (i = 0; i < 64; i++)
	{
		palette_set_color_rgb(machine, i,
			((i >> 5) & 1) * (153*192/255) + ((i >> 4) & 1) * (102*192/255) + ((i & 0x30) ? 63 : 0),
			((i >> 3) & 1) * (156*192/255) + ((i >> 2) & 1) * (99*192/255) + ((i & 0x0c) ? 63 : 0),
			((i >> 1) & 1) * 192 + ((i >> 0) & 1) * 63);
	}

	/* there are 13 color table bits */
	for (i = 0; i < (1 << 13); i++)
	{
		int ena = (i >> 12) & 1;
		int enb = (i >> 11) & 1;
		int abhf = (~i >> 10) & 1;
		int aghf = (~i >> 9) & 1;
		int arhf = (~i >> 8) & 1;
		int az = (i >> 7) & 1;
		int ar = (i >> 6) & 1;
		int ag = (i >> 5) & 1;
		int ab = (i >> 4) & 1;
		int bz = (i >> 3) & 1;
		int br = (i >> 2) & 1;
		int bg = (i >> 1) & 1;
		int bb = (i >> 0) & 1;

		int planea = (az | ar | ag | ab) & ena;

		/*-------------------------------------------------------------------------
            red derivation:

            ROUT.1200   = !IC192.11
                        = !(!(!IC117.11 | !IC118.12))
                        = !IC117.11 | !IC118.12
                        = !(IC99.8 ^ IC119.6) | !(!(!BLNK & IC119.11 & BR))
                        = !((!ARHF & !BLNK & AR & AZ) ^ !(AR & !BLNK)) | (!BLNK & IC119.11 & BR)
                        = !BLNK & (!((!ARHF & AR & AZ) ^ !AR) | (IC119.11 & BR))
                        = !BLNK & ((!(!ARHF & AR & AZ) ^ AR) | (BR & !(AZ | AR | AG | AB)))

            ROUT.1800   = !IC192.3
                        = !(!(!IC119.6 | !IC118.12))
                        = !IC119.6 | !IC118.12
                        = !(!(AR & !BLNK) | !(!(!BLNK & IC119.11 & BZ)))
                        = (AR & !BLNK) | (!BLNK & IC119.11 & BZ)
                        = !BLNK & (AR | (BZ & !(AZ | AR | AG | AB)))

            RENA        = IC116.6
                        = !IC192.11 | !IC192.3
                        = ROUT.1200 | ROUT.1800

            red.hi   = planea ? ar : bz;
            red.lo   = planea ? ((!arhf & az) ? 0 : ar) : br;
            red.base = (red.hi | red.lo)
        -------------------------------------------------------------------------*/

		int rhi = planea ? ar : enb ? bz : 0;
		int rlo = planea ? ((!arhf & az) ? 0 : ar) : enb ? br : 0;

		/*-------------------------------------------------------------------------
            green derivation:

            GOUT.750    = !IC192.8
                        = !(!(!IC119.8 | !IC120.8))
                        = !IC119.8 | !IC120.8
                        = !(!(AG & !BLNK)) | !(!(!BLNK & IC119.11 & BB))
                        = (AG & !BLNK) | (!BLNK & IC119.11 & BB)
                        = !BLNK & (AG | (IC119.11 & BB))
                        = !BLNK & (AG | (BB & !(AZ | AR | AG | AB)))

            GOUT.1200   = !IC192.6
                        = !(!(!IC117.3 | !IC118.6))
                        = !IC117.3 | !IC118.6
                        = !(IC99.6 ^ IC119.8) | !(!(!BLNK & IC119.11 & BG))
                        = !((!AGHF & !BLNK & AG & AZ) ^ !(AG & !BLNK)) | (!BLNK & IC119.11 & BG)
                        = !BLNK & (!((!AGHF & AG & AZ) ^ !AG) | (IC119.11 & BG))
                        = !BLNK & ((!(!AGHF & AG & AZ) ^ AG) | (BG & !(AZ | AR | AG | AB)))

            GENA        = IC116.8
                        = !IC192.8 | !IC192.6
                        = GOUT.750 | GOUT.1200

            grn.hi   = planea ? ag : bb;
            grn.lo   = planea ? ((!aghf & az) ? 0 : ag) : bg;
            grn.base = (grn.hi | grn.lo)
        -------------------------------------------------------------------------*/

		int ghi = planea ? ag : enb ? bb : 0;
		int glo = planea ? ((!aghf & az) ? 0 : ag) : enb ? bg : 0;

		/*-------------------------------------------------------------------------
            blue derivation:

            BOUT.1200   = !IC117.6
                        = !IC119.3
                        = !(!(AB & !BLNK))
                        = !BLNK & AB

            BENA        = !IC117.8
                        = !(IC189.6 ^ IC119.3)
                        = !((!ABHF & !BLNK & AB & AZ) ^ !(AB & !BLNK))
                        = (!(!ABHF & !BLNK & AB & AZ) ^ (AB & !BLNK))
                        = !BLNK & (!(!ABHF & AB & AZ) ^ AB)

            blu.hi   = ab;
            blu.base = ((!abhf & az) ? 0 : ab);
        -------------------------------------------------------------------------*/

		int bhi = ab;
		int bbase = (!abhf & az) ? 0 : ab;

		*colortable++ = (rhi << 5) | (rlo << 4) |
						(ghi << 3) | (glo << 2) |
						(bhi << 1) | bbase;
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( arabian )
{
	UINT8 *gfxbase = memory_region(REGION_GFX1);
	int offs;

	/* allocate a common bitmap to use for both planes */
	/* plane A (top plane with motion objects) is in the upper 4 bits */
	/* plane B (bottom plane with playfield) is in the lower 4 bits */
	main_bitmap = auto_malloc(BITMAP_WIDTH * BITMAP_HEIGHT);

	/* allocate memory for the converted graphics data */
	converted_gfx = auto_malloc(0x8000 * 2);

	/*--------------------------------------------------
        transform graphics data into more usable format
        which is coded like this:

          byte adr+0x4000  byte adr
          DCBA DCBA        DCBA DCBA

        D-bits of pixel 4
        C-bits of pixel 3
        B-bits of pixel 2
        A-bits of pixel 1

        after conversion :

          byte adr+0x4000  byte adr
          DDDD CCCC        BBBB AAAA
    --------------------------------------------------*/

	for (offs = 0; offs < 0x4000; offs++)
	{
		int v1 = gfxbase[offs + 0x0000];
		int v2 = gfxbase[offs + 0x4000];
		int p1, p2, p3, p4;

		p1 = (v1 & 0x01) | ((v1 & 0x10) >> 3) | ((v2 & 0x01) << 2) | ((v2 & 0x10) >> 1);
		v1 >>= 1;
		v2 >>= 1;
		p2 = (v1 & 0x01) | ((v1 & 0x10) >> 3) | ((v2 & 0x01) << 2) | ((v2 & 0x10) >> 1);
		v1 >>= 1;
		v2 >>= 1;
		p3 = (v1 & 0x01) | ((v1 & 0x10) >> 3) | ((v2 & 0x01) << 2) | ((v2 & 0x10) >> 1);
		v1 >>= 1;
		v2 >>= 1;
		p4 = (v1 & 0x01) | ((v1 & 0x10) >> 3) | ((v2 & 0x01) << 2) | ((v2 & 0x10) >> 1);

		converted_gfx[offs * 4 + 3] = p1;
		converted_gfx[offs * 4 + 2] = p2;
		converted_gfx[offs * 4 + 1] = p3;
		converted_gfx[offs * 4 + 0] = p4;
	}
}



/*************************************
 *
 *  DMA blitter simulation
 *
 *************************************/

static void blit_area(UINT8 plane, UINT16 src, UINT8 x, UINT8 y, UINT8 sx, UINT8 sy)
{
	UINT8 *srcdata = &converted_gfx[src * 4];
	int i,j;

	/* loop over X, then Y */
	for (i = 0; i <= sx; i++, x += 4)
		for (j = 0; j <= sy; j++)
		{
			UINT8 p1 = *srcdata++;
			UINT8 p2 = *srcdata++;
			UINT8 p3 = *srcdata++;
			UINT8 p4 = *srcdata++;
			UINT8 *base;

			/* get a pointer to the bitmap */
			base = &main_bitmap[((y + j) & 0xff) * BITMAP_WIDTH + (x & 0xff)];

			/* bit 0 means write to upper plane (upper 4 bits of our bitmap) */
			if (plane & 0x01)
			{
				if (p4 != 8) base[0] = (base[0] & ~0xf0) | (p4 << 4);
				if (p3 != 8) base[1] = (base[1] & ~0xf0) | (p3 << 4);
				if (p2 != 8) base[2] = (base[2] & ~0xf0) | (p2 << 4);
				if (p1 != 8) base[3] = (base[3] & ~0xf0) | (p1 << 4);
			}

			/* bit 2 means write to lower plane (lower 4 bits of our bitmap) */
			if (plane & 0x04)
			{
				if (p4 != 8) base[0] = (base[0] & ~0x0f) | p4;
				if (p3 != 8) base[1] = (base[1] & ~0x0f) | p3;
				if (p2 != 8) base[2] = (base[2] & ~0x0f) | p2;
				if (p1 != 8) base[3] = (base[3] & ~0x0f) | p1;
			}
		}
}



/*************************************
 *
 *  DMA blitter parameters
 *
 *************************************/

WRITE8_HANDLER( arabian_blitter_w )
{
	/* write the data */
	offset &= 7;
	spriteram[offset] = data;

	/* watch for a write to offset 6 -- that triggers the blit */
	if ((offset & 0x07) == 6)
	{
		/* extract the data */
		int plane = spriteram[offset - 6];
		int src   = spriteram[offset - 5] | (spriteram[offset - 4] << 8);
		int x     = spriteram[offset - 2] << 2;
		int y     = spriteram[offset - 3];
		int sx    = spriteram[offset - 0];
		int sy    = spriteram[offset - 1];

		/* blit it */
		blit_area(plane, src, x, y, sx, sy);
	}
}



/*************************************
 *
 *  VRAM direct I/O
 *
 *************************************/

WRITE8_HANDLER( arabian_videoram_w )
{
	UINT8 *base;
	UINT8 x, y;

	/* determine X/Y and mark the area dirty */
	x = (offset >> 8) << 2;
	y = offset & 0xff;

	/* get a pointer to the pixels */
	base = &main_bitmap[y * BITMAP_WIDTH + x];

	/* the data is written as 4 2-bit values, as follows:

            bit 7 = pixel 3, upper bit
            bit 6 = pixel 2, upper bit
            bit 5 = pixel 1, upper bit
            bit 4 = pixel 0, upper bit
            bit 3 = pixel 3, lower bit
            bit 2 = pixel 2, lower bit
            bit 1 = pixel 1, lower bit
            bit 0 = pixel 0, lower bit
    */

	/* enable writes to AZ/AR */
	if (spriteram[0] & 0x08)
	{
		base[0] = (base[0] & ~0x03) | ((data & 0x10) >> 3) | ((data & 0x01) >> 0);
		base[1] = (base[1] & ~0x03) | ((data & 0x20) >> 4) | ((data & 0x02) >> 1);
		base[2] = (base[2] & ~0x03) | ((data & 0x40) >> 5) | ((data & 0x04) >> 2);
		base[3] = (base[3] & ~0x03) | ((data & 0x80) >> 6) | ((data & 0x08) >> 3);
	}

	/* enable writes to AG/AB */
	if (spriteram[0] & 0x04)
	{
		base[0] = (base[0] & ~0x0c) | ((data & 0x10) >> 1) | ((data & 0x01) << 2);
		base[1] = (base[1] & ~0x0c) | ((data & 0x20) >> 2) | ((data & 0x02) << 1);
		base[2] = (base[2] & ~0x0c) | ((data & 0x40) >> 3) | ((data & 0x04) << 0);
		base[3] = (base[3] & ~0x0c) | ((data & 0x80) >> 4) | ((data & 0x08) >> 1);
	}

	/* enable writes to BZ/BR */
	if (spriteram[0] & 0x02)
	{
		base[0] = (base[0] & ~0x30) | ((data & 0x10) << 1) | ((data & 0x01) << 4);
		base[1] = (base[1] & ~0x30) | ((data & 0x20) << 0) | ((data & 0x02) << 3);
		base[2] = (base[2] & ~0x30) | ((data & 0x40) >> 1) | ((data & 0x04) << 2);
		base[3] = (base[3] & ~0x30) | ((data & 0x80) >> 2) | ((data & 0x08) << 1);
	}

	/* enable writes to BG/BB */
	if (spriteram[0] & 0x01)
	{
		base[0] = (base[0] & ~0xc0) | ((data & 0x10) << 3) | ((data & 0x01) << 6);
		base[1] = (base[1] & ~0xc0) | ((data & 0x20) << 2) | ((data & 0x02) << 5);
		base[2] = (base[2] & ~0xc0) | ((data & 0x40) << 1) | ((data & 0x04) << 4);
		base[3] = (base[3] & ~0xc0) | ((data & 0x80) << 0) | ((data & 0x08) << 3);
	}
}



/*************************************
 *
 *  Core video refresh
 *
 *************************************/

VIDEO_UPDATE( arabian )
{
	const pen_t *colortable = &machine->remapped_colortable[(arabian_video_control >> 3) << 8];
	int y;

	/* render the screen from the bitmap */
	for (y = 0; y < BITMAP_HEIGHT; y++)
	{
		/* non-flipped case */
		if (!arabian_flip_screen)
			draw_scanline8(bitmap, 0, y, BITMAP_WIDTH, &main_bitmap[y * BITMAP_WIDTH], colortable, -1);

		/* flipped case */
		else
		{
			UINT8 scanline[BITMAP_WIDTH];
			int x;
			for (x = 0; x < BITMAP_WIDTH; x++)
				scanline[BITMAP_WIDTH - 1 - x] = main_bitmap[y * BITMAP_WIDTH + x];
			draw_scanline8(bitmap, 0, BITMAP_HEIGHT - 1 - y, BITMAP_WIDTH, scanline, colortable, -1);
		}
	}
	return 0;
}
