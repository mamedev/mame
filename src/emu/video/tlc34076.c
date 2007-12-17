/***************************************************************************

    tlc34076.c

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#include "driver.h"
#include "tlc34076.h"

static UINT8 local_paletteram[0x300];
static UINT8 regs[0x10];
static UINT8 palettedata[3];
static UINT8 writeindex, readindex;
static UINT8 dacbits;


#define PALETTE_WRITE_ADDR	0x00
#define PALETTE_DATA		0x01
#define PIXEL_READ_MASK		0x02
#define PALETTE_READ_ADDR	0x03
#define GENERAL_CONTROL		0x08
#define INPUT_CLOCK_SEL		0x09
#define OUTPUT_CLOCK_SEL	0x0a
#define MUX_CONTROL			0x0b
#define PALETTE_PAGE		0x0c
#define TEST_REGISTER		0x0e
#define RESET_STATE			0x0f



/*************************************
 *
 *  Palette update
 *
 *************************************/

static void update_palette(int which)
{
	int totalcolors = (Machine->drv->total_colors <= 256) ? Machine->drv->total_colors : 256;
	int i, mask = regs[PIXEL_READ_MASK];

	for (i = 0; i < totalcolors; i++)
		if (which == -1 || (i & mask) == which)
		{
			int r = local_paletteram[3 * i + 0];
			int g = local_paletteram[3 * i + 1];
			int b = local_paletteram[3 * i + 2];
			if (dacbits == 6)
			{
				r = pal6bit(r);
				g = pal6bit(g);
				b = pal6bit(b);
			}
			palette_set_color(Machine, i, MAKE_RGB(r, g, b));
		}
}



/*************************************
 *
 *  State reset
 *
 *************************************/

void tlc34076_reset(int dacwidth)
{
	/* set the DAC width */
	dacbits = dacwidth;
	if (dacbits != 6 && dacbits != 8)
	{
		logerror("tlc34076_reset: dacwidth must be 6 or 8!\n");
		dacbits = 6;
	}

	/* reset the registers */
	regs[PIXEL_READ_MASK]		= 0xff;
	regs[GENERAL_CONTROL]		= 0x03;
	regs[INPUT_CLOCK_SEL]		= 0x00;
	regs[OUTPUT_CLOCK_SEL]		= 0x3f;
	regs[MUX_CONTROL]			= 0x2d;
	regs[PALETTE_PAGE]			= 0x00;
	regs[TEST_REGISTER]			= 0x00;
	regs[RESET_STATE]			= 0x00;
}



/*************************************
 *
 *  Read access
 *
 *************************************/

READ8_HANDLER( tlc34076_r )
{
	UINT8 result;

	/* keep in range */
	offset &= 0x0f;
	result = regs[offset];

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_DATA:
			if (readindex == 0)
			{
				palettedata[0] = local_paletteram[3 * regs[PALETTE_READ_ADDR] + 0];
				palettedata[1] = local_paletteram[3 * regs[PALETTE_READ_ADDR] + 1];
				palettedata[2] = local_paletteram[3 * regs[PALETTE_READ_ADDR] + 2];
			}
			result = palettedata[readindex++];
			if (readindex == 3)
			{
				readindex = 0;
				regs[PALETTE_READ_ADDR]++;
			}
			break;
	}

	return result;
}



/*************************************
 *
 *  Write access
 *
 *************************************/

WRITE8_HANDLER( tlc34076_w )
{
	UINT8 oldval;

	/* keep in range */
	offset &= 0x0f;
	oldval = regs[offset];
	regs[offset] = data;

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_WRITE_ADDR:
			writeindex = 0;
			break;

		case PALETTE_DATA:
			palettedata[writeindex++] = data;
			if (writeindex == 3)
			{
				local_paletteram[3 * regs[PALETTE_WRITE_ADDR] + 0] = palettedata[0];
				local_paletteram[3 * regs[PALETTE_WRITE_ADDR] + 1] = palettedata[1];
				local_paletteram[3 * regs[PALETTE_WRITE_ADDR] + 2] = palettedata[2];
				update_palette(regs[PALETTE_WRITE_ADDR]);
				writeindex = 0;
				regs[PALETTE_WRITE_ADDR]++;
			}
			break;

		case PALETTE_READ_ADDR:
			readindex = 0;
			break;

		case GENERAL_CONTROL:
			/*
                7 6 5 4 3 2 1 0
                X X X X X X X 0 HSYNCOUT is active-low
                X X X X X X X 1 HSYNCOUT is active-high (default)
                X X X X X X 0 X VSYNCOUT is active-low
                X X X X X X 1 X VSYNCOUT is active-high (default)
                X X X X X 0 X X Disable split shift register transfer (default)
                X X X X 0 1 X X Enable split shift register transfer
                X X X X 0 X X X Disable special nibble mode (default)
                X X X X 1 0 X X Enable special nibble mode
                X X X 0 X X X X 0-IRE pedestal (default)
                X X X 1 X X X X 7.5-IRE pedestal
                X X 0 X X X X X Disable sync (default)
                X X 1 X X X X X Enable sync
                X 0 X X X X X X Little-endian mode (default)
                X 1 X X X X X X Big-endian mode
                0 X X X X X X X MUXOUT is low (default)
                1 X X X X X X X MUXOUT is high
            */
			break;

		case INPUT_CLOCK_SEL:
			/*
                3 2 1 0
                0 0 0 0 Select CLK0 as clock source?
                0 0 0 1 Select CLK1 as clock source
                0 0 1 0 Select CLK2 as clock source
                0 0 1 1 Select CLK3 as TTL clock source
                0 1 0 0 Select CLK3 as TTL clock source
                1 0 0 0 Select CLK3 and CLK3 as ECL clock sources
            */
			break;

		case OUTPUT_CLOCK_SEL:
			/*
                0 0 0 X X X VCLK frequency = DOTCLK frequency
                0 0 1 X X X VCLK frequency = DOTCLK frequency/2
                0 1 0 X X X VCLK frequency = DOTCLK frequency/4
                0 1 1 X X X VCLK frequency = DOTCLK frequency/8
                1 0 0 X X X VCLK frequency = DOTCLK frequency/16
                1 0 1 X X X VCLK frequency = DOTCLK frequency/32
                1 1 X X X X VCLK output held at logic high level (default condition)
                X X X 0 0 0 SCLK frequency = DOTCLK frequency
                X X X 0 0 1 SCLK frequency = DOTCLK frequency/2
                X X X 0 1 0 SCLK frequency = DOTCLK frequency/4
                X X X 0 1 1 SCLK frequency = DOTCLK frequency/8
                X X X 1 0 0 SCLK frequency = DOTCLK frequency/16
                X X X 1 0 1 SCLK frequency = DOTCLK frequency/32
                X X X 1 1 X SCLK output held at logic level low (default condition)
            */
			break;

		case PIXEL_READ_MASK:
		case PALETTE_PAGE:
			update_palette(-1);
			break;

		case RESET_STATE:
			tlc34076_reset(dacbits);
			break;
	}
}



/*************************************
 *
 *  16-bit accessors
 *
 *************************************/

READ16_HANDLER( tlc34076_lsb_r )
{
	return tlc34076_r(offset);
}

WRITE16_HANDLER( tlc34076_lsb_w )
{
	if (ACCESSING_LSB)
		tlc34076_w(offset, data);
}

READ16_HANDLER( tlc34076_msb_r )
{
	return tlc34076_r(offset) << 8;
}

WRITE16_HANDLER( tlc34076_msb_w )
{
	if (ACCESSING_MSB)
		tlc34076_w(offset, data >> 8);
}

