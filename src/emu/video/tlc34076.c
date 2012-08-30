/***************************************************************************

    tlc34076.c

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#include "emu.h"
#include "tlc34076.h"

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

typedef struct _tlc34076_state  tlc34076_state;
struct _tlc34076_state
{
	UINT8 local_paletteram[0x300];
	UINT8 regs[0x10];
	UINT8 palettedata[3];
	UINT8 writeindex;
	UINT8 readindex;
	UINT8 dacbits;

	rgb_t pens[0x100];
};


/*************************************
 *
 *  Inline functions
 *
 *************************************/

INLINE tlc34076_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TLC34076);

	return (tlc34076_state *)downcast<legacy_device_base *>(device)->token();
}

/*************************************
 *
 *  Retrieve Current Palette
 *
 *************************************/

const pen_t *tlc34076_get_pens(device_t *device)
{
	tlc34076_state *state = get_safe_token(device);
	offs_t i;

	for (i = 0; i < 0x100; i++)
	{
		int r, g, b;

		if ((i & state->regs[PIXEL_READ_MASK]) == i)
		{
			r = state->local_paletteram[3 * i + 0];
			g = state->local_paletteram[3 * i + 1];
			b = state->local_paletteram[3 * i + 2];

			if (state->dacbits == 6)
			{
				r = pal6bit(r);
				g = pal6bit(g);
				b = pal6bit(b);
			}
		}
		else
		{
			r = 0;
			g = 0;
			b = 0;
		}

		state->pens[i] = MAKE_RGB(r, g, b);
	}

	return state->pens;
}



/*************************************
 *
 *  State reset
 *
 *************************************/

static DEVICE_RESET( tlc34076 )
{
	tlc34076_state *state = get_safe_token(device);

	/* reset the registers */
	state->regs[PIXEL_READ_MASK]	= 0xff;
	state->regs[GENERAL_CONTROL]	= 0x03;
	state->regs[INPUT_CLOCK_SEL]	= 0x00;
	state->regs[OUTPUT_CLOCK_SEL]	= 0x3f;
	state->regs[MUX_CONTROL]		= 0x2d;
	state->regs[PALETTE_PAGE]		= 0x00;
	state->regs[TEST_REGISTER]		= 0x00;
	state->regs[RESET_STATE]		= 0x00;
}


/*************************************
 *
 *  Read access
 *
 *************************************/

READ8_DEVICE_HANDLER( tlc34076_r )
{
	tlc34076_state *state = get_safe_token(device);
	UINT8 result;

	/* keep in range */
	offset &= 0x0f;
	result = state->regs[offset];

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_DATA:
			if (state->readindex == 0)
			{
				state->palettedata[0] = state->local_paletteram[3 * state->regs[PALETTE_READ_ADDR] + 0];
				state->palettedata[1] = state->local_paletteram[3 * state->regs[PALETTE_READ_ADDR] + 1];
				state->palettedata[2] = state->local_paletteram[3 * state->regs[PALETTE_READ_ADDR] + 2];
			}
			result = state->palettedata[state->readindex++];
			if (state->readindex == 3)
			{
				state->readindex = 0;
				state->regs[PALETTE_READ_ADDR]++;
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

WRITE8_DEVICE_HANDLER( tlc34076_w )
{
	tlc34076_state *state = get_safe_token(device);
//  UINT8 oldval;

	/* keep in range */
	offset &= 0x0f;
//  oldval = state->regs[offset];
	state->regs[offset] = data;

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_WRITE_ADDR:
			state->writeindex = 0;
			break;

		case PALETTE_DATA:
			state->palettedata[state->writeindex++] = data;
			if (state->writeindex == 3)
			{
				state->local_paletteram[3 * state->regs[PALETTE_WRITE_ADDR] + 0] = state->palettedata[0];
				state->local_paletteram[3 * state->regs[PALETTE_WRITE_ADDR] + 1] = state->palettedata[1];
				state->local_paletteram[3 * state->regs[PALETTE_WRITE_ADDR] + 2] = state->palettedata[2];
				state->writeindex = 0;
				state->regs[PALETTE_WRITE_ADDR]++;
			}
			break;

		case PALETTE_READ_ADDR:
			state->readindex = 0;
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

		case RESET_STATE:
			DEVICE_RESET_CALL(tlc34076);
			break;
	}
}



/*************************************
 *
 *  Device interface
 *
 *************************************/

static DEVICE_START( tlc34076 )
{
	tlc34076_config *config = (tlc34076_config *)downcast<const legacy_device_base *>(device)->inline_config();
	tlc34076_state *state = get_safe_token(device);

	state->dacbits = config->res_sel ? 8 : 6;

	state_save_register_global_array(device->machine(), state->local_paletteram);
	state_save_register_global_array(device->machine(), state->regs);
	state_save_register_global_array(device->machine(), state->pens);

	state_save_register_global(device->machine(), state->writeindex);
	state_save_register_global(device->machine(), state->readindex);
	state_save_register_global(device->machine(), state->dacbits);
}

DEVICE_GET_INFO(tlc34076)
{
 switch (state)
 {
  case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(tlc34076_state); break;

  case DEVINFO_INT_INLINE_CONFIG_BYTES: info->i = sizeof(tlc34076_config); break;

  case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(tlc34076); break;

  case DEVINFO_FCT_RESET: info->reset = DEVICE_RESET_NAME(tlc34076); break;

  case DEVINFO_STR_NAME: strcpy(info->s, "TLC34076"); break;
 }
}

DEFINE_LEGACY_DEVICE(TLC34076, tlc34076);
