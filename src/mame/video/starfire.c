/***************************************************************************

    Star Fire video system

***************************************************************************/

#include "emu.h"
#include "includes/starfire.h"


#define	NUM_PENS	(0x40)


UINT8 *starfire_videoram;
UINT8 *starfire_colorram;

/* local allocated storage */
static UINT8 starfire_vidctrl;
static UINT8 starfire_vidctrl1;
static UINT8 starfire_color;
static UINT16 starfire_colors[NUM_PENS];



/*************************************
 *
 *  Initialize the video system
 *
 *************************************/

VIDEO_START( starfire )
{
	/* register for state saving */
	state_save_register_global(machine, starfire_vidctrl);
	state_save_register_global(machine, starfire_vidctrl1);
	state_save_register_global(machine, starfire_color);
	state_save_register_global_array(machine, starfire_colors);
}



/*************************************
 *
 *  Video control writes
 *
 *************************************/

WRITE8_HANDLER( starfire_vidctrl_w )
{
    starfire_vidctrl = data;
}

WRITE8_HANDLER( starfire_vidctrl1_w )
{
    starfire_vidctrl1 = data;
}



/*************************************
 *
 *  Color RAM read/writes
 *
 *************************************/

WRITE8_HANDLER( starfire_colorram_w )
{
	/* handle writes to the pseudo-color RAM */
	if ((offset & 0xe0) == 0)
	{
		int palette_index = (offset & 0x1f) | ((offset & 0x200) >> 4);

		/* set RAM regardless */
		starfire_colorram[offset & ~0x100] = data;
		starfire_colorram[offset |  0x100] = data;

		starfire_color = data & 0x1f;

		/* don't modify the palette unless the TRANS bit is set */
		if (starfire_vidctrl1 & 0x40)
		{
			space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

			starfire_colors[palette_index] = ((((data << 1) & 0x06) | ((offset >> 8) & 0x01)) << 6) |
											 (((data >> 5) & 0x07) << 3) |
											 ((data >> 2) & 0x07);
		}
	}

	/* handle writes to the rest of color RAM */
	else
	{
		/* set RAM based on CDRM */
		starfire_colorram[offset] = (starfire_vidctrl1 & 0x80) ? starfire_color : (data & 0x1f);
		starfire_color = data & 0x1f;
	}
}



/*************************************
 *
 *  Video RAM read/writes
 *
 *************************************/

WRITE8_HANDLER( starfire_videoram_w )
{
	int sh, lr, dm, ds, mask, d0, dalu;
	int offset1 = offset & 0x1fff;
	int offset2 = (offset + 0x100) & 0x1fff;

	/* PROT */
	if (!(offset & 0xe0) && !(starfire_vidctrl1 & 0x20))
		return;

	/* selector 6A */
	if (offset & 0x2000)
	{
		sh = (starfire_vidctrl >> 1) & 0x07;
		lr = starfire_vidctrl & 0x01;
	}
	else
	{
		sh = (starfire_vidctrl >> 5) & 0x07;
		lr = (starfire_vidctrl >> 4) & 0x01;
	}

	/* mirror bits 5B/5C/5D/5E */
	dm = data;
	if (lr)
		dm = ((dm & 0x01) << 7) | ((dm & 0x02) << 5) | ((dm & 0x04) << 3) | ((dm & 0x08) << 1) |
		     ((dm & 0x10) >> 1) | ((dm & 0x20) >> 3) | ((dm & 0x40) >> 5) | ((dm & 0x80) >> 7);

	/* shifters 6D/6E */
	ds = (dm << 8) >> sh;
	mask = 0xff00 >> sh;

	/* ROLL */
	if ((offset & 0x1f00) == 0x1f00)
	{
		if (starfire_vidctrl1 & 0x10)
			mask &= 0x00ff;
		else
			mask &= 0xff00;
	}

	/* ALU 8B/8D */
	d0 = (starfire_videoram[offset1] << 8) | starfire_videoram[offset2];
	dalu = d0 & ~mask;
	d0 &= mask;
	ds &= mask;
	switch (~starfire_vidctrl1 & 15)
	{
		case 0:		dalu |= ds ^ mask;				break;
		case 1:		dalu |= (ds | d0) ^ mask;		break;
		case 2:		dalu |= (ds ^ mask) & d0;		break;
		case 3:		dalu |= 0;						break;
		case 4:		dalu |= (ds & d0) ^ mask;		break;
		case 5:		dalu |= d0 ^ mask;				break;
		case 6:		dalu |= ds ^ d0;				break;
		case 7:		dalu |= ds & (d0 ^ mask);		break;
		case 8:		dalu |= (ds ^ mask) | d0;		break;
		case 9:		dalu |= (ds ^ d0) ^ mask;		break;
		case 10:	dalu |= d0;						break;
		case 11:	dalu |= ds & d0;				break;
		case 12:	dalu |= mask;					break;
		case 13:	dalu |= ds | (d0 ^ mask);		break;
		case 14:	dalu |= ds | d0;				break;
		case 15:	dalu |= ds;						break;
	}

	/* final output */
	starfire_videoram[offset1] = dalu >> 8;
	starfire_videoram[offset2] = dalu;

	/* color output */
	if (!(offset & 0x2000) && !(starfire_vidctrl1 & 0x80))
	{
		if (mask & 0xff00)
			starfire_colorram[offset1] = starfire_color;
		if (mask & 0x00ff)
			starfire_colorram[offset2] = starfire_color;
	}
}

READ8_HANDLER( starfire_videoram_r )
{
	int sh, mask, d0;
	int offset1 = offset & 0x1fff;
	int offset2 = (offset + 0x100) & 0x1fff;

	/* selector 6A */
	if (offset & 0x2000)
		sh = (starfire_vidctrl >> 1) & 0x07;
	else
		sh = (starfire_vidctrl >> 5) & 0x07;

	/* shifters 6D/6E */
	mask = 0xff00 >> sh;

	/* ROLL */
	if ((offset & 0x1f00) == 0x1f00)
	{
		if (starfire_vidctrl1 & 0x10)
			mask &= 0x00ff;
		else
			mask &= 0xff00;
	}

	/* munge the results */
	d0 = (starfire_videoram[offset1] & (mask >> 8)) | (starfire_videoram[offset2] & mask);
	d0 = (d0 << sh) | (d0 >> (8 - sh));
	return d0 & 0xff;
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

static void get_pens(pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < NUM_PENS; offs++)
	{
		UINT16 color = starfire_colors[offs];

		pens[offs] = MAKE_RGB(pal3bit(color >> 6), pal3bit(color >> 3), pal3bit(color >> 0));
	}
}


VIDEO_UPDATE( starfire )
{
	pen_t pens[NUM_PENS];

	UINT8 *pix = &starfire_videoram[cliprect->min_y - 32];
	UINT8 *col = &starfire_colorram[cliprect->min_y - 32];
	int x, y;

	get_pens(pens);

	for (x = 0; x < 256; x += 8)
	{
		for (y = cliprect->min_y; y <= cliprect->max_y ; y++)
		{
			int data = pix[y];
			int color = col[y];

			*BITMAP_ADDR32(bitmap, y, x + 0) = pens[color | ((data >> 2) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 1) = pens[color | ((data >> 1) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 2) = pens[color | ((data >> 0) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 3) = pens[color | ((data << 1) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 4) = pens[color | ((data << 2) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 5) = pens[color | ((data << 3) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 6) = pens[color | ((data << 4) & 0x20)];
			*BITMAP_ADDR32(bitmap, y, x + 7) = pens[color | ((data << 5) & 0x20)];
		}

		pix += 256;
		col += 256;
	}

	return 0;
}
