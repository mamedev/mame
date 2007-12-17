/***************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

***************************************************************************/

#include "driver.h"
#include "video/tms34061.h"
#include "cpu/m6809/m6809.h"
#include "capbowl.h"

UINT8 *capbowl_rowaddress;

static offs_t blitter_addr;



/*************************************
 *
 *  TMS34061 interfacing
 *
 *************************************/

static void generate_interrupt(int state)
{
	cpunum_set_input_line(0, M6809_FIRQ_LINE, state);
}

static struct tms34061_interface tms34061intf =
{
	0,						/* the screen we are acting on */
	8,						/* VRAM address is (row << rowshift) | col */
	0x10000,				/* size of video RAM */
	generate_interrupt		/* interrupt gen callback */
};



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( capbowl )
{
	/* initialize TMS34061 emulation */
    tms34061_start(&tms34061intf);
}



/*************************************
 *
 *  TMS34061 I/O
 *
 *************************************/

WRITE8_HANDLER( capbowl_tms34061_w )
{
	int func = (offset >> 8) & 3;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	tms34061_w(col, *capbowl_rowaddress, func, data);
}


READ8_HANDLER( capbowl_tms34061_r )
{
	int func = (offset >> 8) & 3;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	return tms34061_r(col, *capbowl_rowaddress, func);
}



/*************************************
 *
 *  Bowl-o-rama blitter
 *
 *************************************/

WRITE8_HANDLER( bowlrama_blitter_w )
{
	switch (offset)
	{
		case 0x08:	  /* Write address high byte (only 2 bits used) */
			blitter_addr = (blitter_addr & ~0xff0000) | (data << 16);
			break;

		case 0x17:    /* Write address mid byte (8 bits)   */
			blitter_addr = (blitter_addr & ~0x00ff00) | (data << 8);
			break;

		case 0x18:	  /* Write Address low byte (8 bits)   */
			blitter_addr = (blitter_addr & ~0x0000ff) | (data << 0);
			break;

		default:
			logerror("PC=%04X Write to unsupported blitter address %02X Data=%02X\n", activecpu_get_pc(), offset, data);
			break;
	}
}


READ8_HANDLER( bowlrama_blitter_r )
{
	UINT8 data = memory_region(REGION_GFX1)[blitter_addr];
	UINT8 result = 0;

	switch (offset)
	{
		/* Read Mask: Graphics data are 4bpp (2 pixels per byte).
            This function returns 0's for new pixel data.
            This allows data to be read as a mask, AND the mask with
            the screen data, then OR new data read by read data command. */
		case 0:
			if (!(data & 0xf0))
				result |= 0xf0;		/* High nibble is transparent */
			if (!(data & 0x0f))
				result |= 0x0f;		/* Low nibble is transparent */
			break;

		/* Read data and increment address */
		case 4:
			result = data;
			blitter_addr = (blitter_addr + 1) & 0x3ffff;
			break;

		default:
			logerror("PC=%04X Read from unsupported blitter address %02X\n", activecpu_get_pc(), offset);
			break;
	}

	return result;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( capbowl )
{
	struct tms34061_display state;
	int x, y;

	/* first get the current display state */
	tms34061_get_display_state(&state);

	/* if we're blanked, just fill with black */
	if (state.blanked)
	{
		fillbitmap(bitmap, get_black_pen(machine), cliprect);
		return 0;
	}

	/* now regenerate the bitmap */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);
		UINT8 *src = &state.vram[256 * y];
		int ybase = 16 * y;

		/* first update the palette for this scanline */
		for (x = 0; x < 16; x++)
		{
			palette_set_color_rgb(machine, ybase + x, pal4bit(src[0]), pal4bit(src[1] >> 4), pal4bit(src[1]));
			src += 2;
		}

		/* expand row to 8bpp */
		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			int pix = src[x/2];
			*dest++ = ybase + (pix >> 4);
			*dest++ = ybase + (pix & 0x0f);
		}
	}
	return 0;
}
