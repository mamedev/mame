/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/astrocde.h"
#include "sound/astrocde.h"
#include "video/resnet.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define RNG_PERIOD		((1 << 17) - 1)
#define VERT_OFFSET		(22)				/* pixels from top of screen to top of game area */
#define HORZ_OFFSET		(16)				/* pixels from left of screen to left of game area */



/*************************************
 *
 *  Global variables
 *
 *************************************/

/* video configuration */
UINT8 astrocade_video_config;
UINT8 astrocade_sparkle[4];

/* palette/sparkle/star */
static UINT8 *sparklestar;

/* Astrocade interrupts */
static UINT8 interrupt_enable;
static UINT8 interrupt_vector;
static UINT8 interrupt_scanline;
static UINT8 vertical_feedback;
static UINT8 horizontal_feedback;

/* Astrocade video parameters */
static emu_timer *scanline_timer;
static UINT8 colors[8];
static UINT8 colorsplit;
static UINT8 bgdata;
static UINT8 vblank;
static UINT8 video_mode;

/* Astrocade function generator */
static UINT8 funcgen_expand_color[2];
static UINT8 funcgen_control;
static UINT8 funcgen_expand_count;
static UINT8 funcgen_rotate_count;
static UINT8 funcgen_rotate_data[4];
static UINT8 funcgen_shift_prev_data;
static UINT8 funcgen_intercept;

/* pattern board registers */
static UINT16 pattern_source;
static UINT8 pattern_mode;
static UINT16 pattern_dest;
static UINT8 pattern_skip;
static UINT8 pattern_width;
static UINT8 pattern_height;

/* 16-color board registers and video RAM */
static UINT16 *profpac_videoram;
static UINT16 profpac_palette[16];
static UINT8 profpac_colormap[4];
static UINT8 profpac_intercept;
static UINT8 profpac_vispage;
static UINT8 profpac_readpage;
static UINT8 profpac_readshift;
static UINT8 profpac_writepage;
static UINT8 profpac_writemode;
static UINT16 profpac_writemask;
static UINT8 profpac_vw;



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void init_savestate(running_machine *machine);
static TIMER_CALLBACK( scanline_callback );
static void init_sparklestar(running_machine *machine);



/*************************************
 *
 *  Scanline conversion
 *
 *************************************/

INLINE int mame_vpos_to_astrocade_vpos(int scanline)
{
	scanline -= VERT_OFFSET;
	if (scanline < 0)
		scanline += 262;
	return scanline;
}



/*************************************
 *
 *  Palette initialization
 *
 *************************************/

PALETTE_INIT( astrocde )
{
	/*
        The Astrocade has a 256 color palette: 32 colors with 8 luminance
        values for each color. The 32 colors circle around the YUV color
        space, with the exception of the first 8 which are grayscale.

        We actually build a 512 entry palette with an extra bit of
        luminance informaton. This is because the sparkle/star circuitry
        on Wizard of War and Gorf replaces the luminance with a value
        that has a 4-bit resolution.
    */

	int color, luma;

	/* loop over color values */
	for (color = 0; color < 32; color++)
	{
		float ry = 0.75 * sin((color / 32.0) * (2.0 * M_PI));
		float by = 1.15 * cos((color / 32.0) * (2.0 * M_PI));

		/* color 0 maps to ry = by = 0 */
		if (color == 0)
			ry = by = 0;

		/* iterate over luminence values */
		for (luma = 0; luma < 16; luma++)
		{
			float y = luma / 15.0;
			int r, g, b;

			/* transform to RGB */
			r = (ry + y) * 255;
			g = ((y - 0.299 * (ry + y) - 0.114 * (by + y)) / 0.587) * 255;
			b = (by + y) * 255;

			/* clamp and store */
			r = MAX(r, 0);
			r = MIN(r, 255);
			g = MAX(g, 0);
			g = MIN(g, 255);
			b = MAX(b, 0);
			b = MIN(b, 255);
			palette_set_color(machine, color * 16 + luma, MAKE_RGB(r, g, b));
		}
	}
}


PALETTE_INIT( profpac )
{
	/* Professor Pac-Man uses a more standard 12-bit RGB palette layout */
	static const int resistances[4] = { 6200, 3000, 1500, 750 };
	double weights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, weights, 1500, 0,
			4, resistances, weights, 1500, 0,
			4, resistances, weights, 1500, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < 4096; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* blue component */
		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		bit2 = (i >> 2) & 0x01;
		bit3 = (i >> 3) & 0x01;
		b = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (i >> 4) & 0x01;
		bit1 = (i >> 5) & 0x01;
		bit2 = (i >> 6) & 0x01;
		bit3 = (i >> 7) & 0x01;
		g = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		/* red component */
		bit0 = (i >> 8) & 0x01;
		bit1 = (i >> 9) & 0x01;
		bit2 = (i >> 10) & 0x01;
		bit3 = (i >> 11) & 0x01;
		r = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( astrocde )
{
	/* allocate a per-scanline timer */
	scanline_timer = timer_alloc(machine, scanline_callback, NULL);
	timer_adjust_oneshot(scanline_timer, machine->primary_screen->time_until_pos(1), 1);

	/* register for save states */
	init_savestate(machine);

	/* initialize the sparkle and stars */
	if (astrocade_video_config & AC_STARS)
		init_sparklestar(machine);
}


VIDEO_START( profpac )
{
	/* allocate a per-scanline timer */
	scanline_timer = timer_alloc(machine, scanline_callback, NULL);
	timer_adjust_oneshot(scanline_timer, machine->primary_screen->time_until_pos(1), 1);

	/* allocate videoram */
	profpac_videoram = auto_alloc_array(machine, UINT16, 0x4000 * 4);

	/* register for save states */
	init_savestate(machine);

	/* register our specific save state data */
	state_save_register_global_pointer(machine, profpac_videoram, 0x4000 * 4);
	state_save_register_global_array(machine, profpac_palette);
	state_save_register_global_array(machine, profpac_colormap);
	state_save_register_global(machine, profpac_intercept);
	state_save_register_global(machine, profpac_vispage);
	state_save_register_global(machine, profpac_readpage);
	state_save_register_global(machine, profpac_readshift);
	state_save_register_global(machine, profpac_writepage);
	state_save_register_global(machine, profpac_writemode);
	state_save_register_global(machine, profpac_writemask);
	state_save_register_global(machine, profpac_vw);
}


static void init_savestate(running_machine *machine)
{
	state_save_register_global_array(machine, astrocade_sparkle);

	state_save_register_global(machine, interrupt_enable);
	state_save_register_global(machine, interrupt_vector);
	state_save_register_global(machine, interrupt_scanline);
	state_save_register_global(machine, vertical_feedback);
	state_save_register_global(machine, horizontal_feedback);

	state_save_register_global_array(machine, colors);
	state_save_register_global(machine, colorsplit);
	state_save_register_global(machine, bgdata);
	state_save_register_global(machine, vblank);
	state_save_register_global(machine, video_mode);

	state_save_register_global_array(machine, funcgen_expand_color);
	state_save_register_global(machine, funcgen_control);
	state_save_register_global(machine, funcgen_expand_count);
	state_save_register_global(machine, funcgen_rotate_count);
	state_save_register_global_array(machine, funcgen_rotate_data);
	state_save_register_global(machine, funcgen_shift_prev_data);
	state_save_register_global(machine, funcgen_intercept);

	state_save_register_global(machine, pattern_source);
	state_save_register_global(machine, pattern_mode);
	state_save_register_global(machine, pattern_dest);
	state_save_register_global(machine, pattern_skip);
	state_save_register_global(machine, pattern_width);
	state_save_register_global(machine, pattern_height);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( astrocde )
{
	int xystep = 2 - video_mode;
	UINT32 sparklebase = 0;
	int y;

	/* compute the starting point of sparkle for the current frame */
	int width = screen->width();
	int height = screen->height();

	if (astrocade_video_config & AC_STARS)
		sparklebase = (screen->frame_number() * (UINT64)(width * height)) % RNG_PERIOD;

	/* iterate over scanlines */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);
		int effy = mame_vpos_to_astrocade_vpos(y);
		UINT16 offset = (effy / xystep) * (80 / xystep);
		UINT32 sparkleoffs = 0, staroffs = 0;
		int x;

		/* compute the star and sparkle offset at the start of this line */
		if (astrocade_video_config & AC_STARS)
		{
			staroffs = ((effy < 0) ? (effy + 262) : effy) * width;
			sparkleoffs = sparklebase + y * width;
			if (sparkleoffs >= RNG_PERIOD)
				sparkleoffs -= RNG_PERIOD;
		}

		/* iterate over groups of 4 pixels */
		for (x = 0; x < 456/4; x += xystep)
		{
			int effx = x - HORZ_OFFSET/4;
			const UINT8 *colorbase = &colors[(effx < colorsplit) ? 4 : 0];
			UINT8 data;
			int xx;

			/* select either video data or background data */
			data = (effx >= 0 && effx < 80 && effy >= 0 && effy < vblank) ? screen->machine->generic.videoram.u8[offset++] : bgdata;

			/* iterate over the 4 pixels */
			for (xx = 0; xx < 4; xx++)
			{
				UINT8 pixdata = (data >> 6) & 3;
				int coldata = colorbase[pixdata] << 1;
				int luma = coldata & 0x0f;
				rgb_t color;

				/* handle stars/sparkle */
				if (astrocade_video_config & AC_STARS)
				{
					/* if sparkle is enabled for this pixel index and either it is non-zero or a star */
					/* then adjust the intensity */
					if (astrocade_sparkle[pixdata] == 0)
					{
						if (pixdata != 0 || (sparklestar[staroffs] & 0x10))
							luma = sparklestar[sparkleoffs] & 0x0f;
						else if (pixdata == 0)
							coldata = luma = 0;
					}

					/* update sparkle/star offsets */
					staroffs++;
					if (++sparkleoffs >= RNG_PERIOD)
						sparkleoffs = 0;
				}
				color = (coldata & 0x1f0) | luma;

				/* store the final color to the destination and shift */
				*dest++ = color;
				if (xystep == 2)
					*dest++ = color;
				data <<= 2;
			}
		}
	}

	return 0;
}


VIDEO_UPDATE( profpac )
{
	int y;

	/* iterate over scanlines */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int effy = mame_vpos_to_astrocade_vpos(y);
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);
		UINT16 offset = profpac_vispage * 0x4000 + effy * 80;
		int x;

		/* star with black */

		/* iterate over groups of 4 pixels */
		for (x = 0; x < 456/4; x++)
		{
			int effx = x - HORZ_OFFSET/4;

			/* select either video data or background data */
			UINT16 data = (effx >= 0 && effx < 80 && effy >= 0 && effy < vblank) ? profpac_videoram[offset++] : 0;

			/* iterate over the 4 pixels */
			*dest++ = profpac_palette[(data >> 12) & 0x0f];
			*dest++ = profpac_palette[(data >> 8) & 0x0f];
			*dest++ = profpac_palette[(data >> 4) & 0x0f];
			*dest++ = profpac_palette[(data >> 0) & 0x0f];
		}
	}

	return 0;
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static TIMER_CALLBACK( interrupt_off )
{
	cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
}


static void astrocade_trigger_lightpen(running_machine *machine, UINT8 vfeedback, UINT8 hfeedback)
{
	/* both bits 1 and 4 enable lightpen interrupts; bit 4 enables them even in horizontal */
	/* blanking regions; we treat them both the same here */
	if ((interrupt_enable & 0x12) != 0)
	{
		/* bit 0 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((interrupt_enable & 0x01) == 0)
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, interrupt_vector & 0xf0);
			timer_set(machine, machine->primary_screen->time_until_vblank_end(), NULL, 0, interrupt_off);
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, ASSERT_LINE, interrupt_vector & 0xf0);
			timer_set(machine, cputag_clocks_to_attotime(machine, "maincpu", 1), NULL, 0, interrupt_off);
		}

		/* latch the feedback registers */
		vertical_feedback = vfeedback;
		horizontal_feedback = hfeedback;
	}
}



/*************************************
 *
 *  Per-scanline callback
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	int scanline = param;
	int astrocade_scanline = mame_vpos_to_astrocade_vpos(scanline);

	/* force an update against the current scanline */
	if (scanline > 0)
		machine->primary_screen->update_partial(scanline - 1);

	/* generate a scanline interrupt if it's time */
	if (astrocade_scanline == interrupt_scanline && (interrupt_enable & 0x08) != 0)
	{
		/* bit 2 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((interrupt_enable & 0x04) == 0)
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, interrupt_vector);
			timer_set(machine, machine->primary_screen->time_until_vblank_end(), NULL, 0, interrupt_off);
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, ASSERT_LINE, interrupt_vector);
			timer_set(machine, cputag_clocks_to_attotime(machine, "maincpu", 1), NULL, 0, interrupt_off);
		}
	}

	/* on some games, the horizontal drive line is conected to the lightpen interrupt */
	else if (astrocade_video_config & AC_LIGHTPEN_INTS)
		astrocade_trigger_lightpen(machine, astrocade_scanline, 8);

	/* advance to the next scanline */
	scanline++;
	if (scanline >= machine->primary_screen->height())
		scanline = 0;
	timer_adjust_oneshot(scanline_timer, machine->primary_screen->time_until_pos(scanline), scanline);
}



/*************************************
 *
 *  Data chip registers
 *
 *************************************/

READ8_HANDLER( astrocade_data_chip_register_r )
{
	UINT8 result = 0xff;

	/* these are the core registers */
	switch (offset & 0xff)
	{
		case 0x08:	/* intercept feedback */
			result = funcgen_intercept;
			funcgen_intercept = 0;
			break;

		case 0x0e:	/* vertical feedback (from lightpen interrupt) */
			result = vertical_feedback;
			break;

		case 0x0f:	/* horizontal feedback (from lightpen interrupt) */
			result = horizontal_feedback;
			break;

		case 0x10:	/* player 1 handle */
			result = input_port_read_safe(space->machine, "P1HANDLE", 0xff);
			break;

		case 0x11:	/* player 2 handle */
			result = input_port_read_safe(space->machine, "P2HANDLE", 0xff);
			break;

		case 0x12:	/* player 3 handle */
			result = input_port_read_safe(space->machine, "P3HANDLE", 0xff);
			break;

		case 0x13:	/* player 4 handle */
			result = input_port_read_safe(space->machine, "P4HANDLE", 0xff);
			break;

		case 0x14:	/* keypad column 0 */
			result = input_port_read_safe(space->machine, "KEYPAD0", 0xff);
			break;

		case 0x15:	/* keypad column 1 */
			result = input_port_read_safe(space->machine, "KEYPAD1", 0xff);
			break;

		case 0x16:	/* keypad column 2 */
			result = input_port_read_safe(space->machine, "KEYPAD2", 0xff);
			break;

		case 0x17:	/* keypad column 3 */
			result = input_port_read_safe(space->machine, "KEYPAD3", 0xff);
			break;

		case 0x1c:	/* player 1 knob */
			result = input_port_read_safe(space->machine, "P1_KNOB", 0xff);
			break;

		case 0x1d:	/* player 2 knob */
			result = input_port_read_safe(space->machine, "P2_KNOB", 0xff);
			break;

		case 0x1e:	/* player 3 knob */
			result = input_port_read_safe(space->machine, "P3_KNOB", 0xff);
			break;

		case 0x1f:	/* player 4 knob */
			result = input_port_read_safe(space->machine, "P4_KNOB", 0xff);
			break;
	}

	return result;
}


WRITE8_HANDLER( astrocade_data_chip_register_w )
{
	/* these are the core registers */
	switch (offset & 0xff)
	{
		case 0x00:	/* color table is in registers 0-7 */
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			colors[offset & 7] = data;
			break;

		case 0x08:	/* mode register */
			video_mode = data & 1;
			break;

		case 0x09:	/* color split pixel */
			colorsplit = 2 * (data & 0x3f);
			bgdata = ((data & 0xc0) >> 6) * 0x55;
			break;

		case 0x0a:	/* vertical blank register */
			vblank = data;
			break;

		case 0x0b:	/* color block transfer */
			colors[(offset >> 8) & 7] = data;
			break;

		case 0x0c:	/* function generator */
			funcgen_control = data;
			funcgen_expand_count = 0;		/* reset flip-flop for expand mode on write to this register */
			funcgen_rotate_count = 0;		/* reset counter for rotate mode on write to this register */
			funcgen_shift_prev_data = 0;	/* reset shift buffer on write to this register */
			break;

		case 0x0d:	/* interrupt feedback */
			interrupt_vector = data;
			break;

		case 0x0e:	/* interrupt enable and mode */
			interrupt_enable = data;
			break;

		case 0x0f:	/* interrupt line */
			interrupt_scanline = data;
			break;

		case 0x10:	/* master oscillator register */
		case 0x11:	/* tone A frequency register */
		case 0x12:	/* tone B frequency register */
		case 0x13:	/* tone C frequency register */
		case 0x14:	/* vibrato register */
		case 0x15:	/* tone C volume, noise modulation and MUX register */
		case 0x16:	/* tone A volume and tone B volume register */
		case 0x17:	/* noise volume register */
		case 0x18:	/* sound block transfer */
			if (astrocade_video_config & AC_SOUND_PRESENT)
				astrocade_sound_w(space->machine->device("astrocade1"), offset, data);
			break;

		case 0x19:	/* expand register */
			funcgen_expand_color[0] = data & 0x03;
			funcgen_expand_color[1] = (data >> 2) & 0x03;
			break;
	}
}



/*************************************
 *
 *  Function generator
 *
 *************************************/

WRITE8_HANDLER( astrocade_funcgen_w )
{
	UINT8 prev_data;

	/* control register:
        bit 0 = shift amount LSB
        bit 1 = shift amount MSB
        bit 2 = rotate
        bit 3 = expand
        bit 4 = OR
        bit 5 = XOR
        bit 6 = flop
    */

	/* expansion */
	if (funcgen_control & 0x08)
	{
		funcgen_expand_count ^= 1;
		data >>= 4 * funcgen_expand_count;
		data =	(funcgen_expand_color[(data >> 3) & 1] << 6) |
				(funcgen_expand_color[(data >> 2) & 1] << 4) |
				(funcgen_expand_color[(data >> 1) & 1] << 2) |
				(funcgen_expand_color[(data >> 0) & 1] << 0);
	}
	prev_data = funcgen_shift_prev_data;
	funcgen_shift_prev_data = data;

	/* rotate or shift */
	if (funcgen_control & 0x04)
	{
		/* rotate */

		/* first 4 writes accumulate data for the rotate */
		if ((funcgen_rotate_count & 4) == 0)
		{
			funcgen_rotate_data[funcgen_rotate_count++ & 3] = data;
			return;
		}

		/* second 4 writes actually write it */
		else
		{
			UINT8 shift = 2 * (~funcgen_rotate_count++ & 3);
			data =	(((funcgen_rotate_data[3] >> shift) & 3) << 6) |
					(((funcgen_rotate_data[2] >> shift) & 3) << 4) |
					(((funcgen_rotate_data[1] >> shift) & 3) << 2) |
					(((funcgen_rotate_data[0] >> shift) & 3) << 0);
		}
	}
	else
	{
		/* shift */
		UINT8 shift = 2 * (funcgen_control & 0x03);
		data = (data >> shift) | (prev_data << (8 - shift));
	}

	/* flopping */
	if (funcgen_control & 0x40)
		data = (data >> 6) | ((data >> 2) & 0x0c) | ((data << 2) & 0x30) | (data << 6);

	/* OR/XOR */
	if (funcgen_control & 0x30)
	{
		UINT8 olddata = memory_read_byte(space, 0x4000 + offset);

		/* compute any intercepts */
		funcgen_intercept &= 0x0f;
		if ((olddata & 0xc0) && (data & 0xc0))
			funcgen_intercept |= 0x11;
		if ((olddata & 0x30) && (data & 0x30))
			funcgen_intercept |= 0x22;
		if ((olddata & 0x0c) && (data & 0x0c))
			funcgen_intercept |= 0x44;
		if ((olddata & 0x03) && (data & 0x03))
			funcgen_intercept |= 0x88;

		/* apply the operation */
		if (funcgen_control & 0x10)
			data |= olddata;
		else if (funcgen_control & 0x20)
			data ^= olddata;
	}

	/* write the result */
	memory_write_byte(space, 0x4000 + offset, data);
}



/*************************************
 *
 *  Pattern board
 *
 *************************************/

INLINE void increment_source(UINT8 curwidth, UINT8 *u13ff)
{
	/* if the flip-flop at U13 is high and mode.d2 is 1 we can increment */
	/* however, if mode.d3 is set and we're on the last byte of a row, the increment is suppressed */
	if (*u13ff && (pattern_mode & 0x04) != 0 && (curwidth != 0 || (pattern_mode & 0x08) == 0))
		pattern_source++;

	/* if mode.d1 is 1, toggle the flip-flop; otherwise leave it preset */
	if ((pattern_mode & 0x02) != 0)
		*u13ff ^= 1;
}


INLINE void increment_dest(UINT8 curwidth)
{
	/* increment is suppressed for the last byte in a row */
	if (curwidth != 0)
	{
		/* if mode.d5 is 1, we increment */
		if ((pattern_mode & 0x20) != 0)
			pattern_dest++;

		/* otherwise, we decrement */
		else
			pattern_dest--;
	}
}


static void execute_blit(const address_space *space)
{
	/*
        pattern_source = counter set U7/U16/U25/U34
        pattern_dest = counter set U9/U18/U30/U39
        pattern_mode = latch U21
        pattern_skip = latch set U30/U39
        pattern_width = latch set U32/U41
        pattern_height = counter set U31/U40

        pattern_mode bits:
            d0 = direction (0 = read from src, write to dest, 1 = read from dest, write to src)
            d1 = expand (0 = increment src each pixel, 1 = increment src every other pixel)
            d2 = constant (0 = never increment src, 1 = normal src increment)
            d3 = flush (0 = copy all data, 1 = copy a 0 in place of last byte of each row)
            d4 = dest increment direction (0 = decrement dest on carry, 1 = increment dest on carry)
            d5 = dest direction (0 = increment dest, 1 = decrement dest)
    */

	UINT8 curwidth;	/* = counter set U33/U42 */
	UINT8 u13ff;	/* = flip-flop at U13 */
	int cycles = 0;

/*  logerror("Blit: src=%04X mode=%02X dest=%04X skip=%02X width=%02X height=%02X\n",
            pattern_source, pattern_mode, pattern_dest, pattern_skip, pattern_width, pattern_height);*/

	/* flip-flop at U13 is cleared at the beginning */
	u13ff = 0;

	/* it is also forced preset if mode.d1 == 0 */
	if ((pattern_mode & 0x02) == 0)
		u13ff = 1;

	/* loop over height */
	do
	{
		UINT16 carry;

		/* loop over width */
		curwidth = pattern_width;
		do
		{
			UINT16 busaddr;
			UINT8 busdata;

			/* ----- read phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((pattern_mode & 0x01) == 0) ? pattern_source : pattern_dest;

			/* if mode.d3 is set, then the last byte fetched per row is forced to 0 */
			if (curwidth == 0 && (pattern_mode & 0x08) != 0)
				busdata = 0;
			else
				busdata = memory_read_byte(space, busaddr);

			/* increment the appropriate address */
			if ((pattern_mode & 0x01) == 0)
				increment_source(curwidth, &u13ff);
			else
				increment_dest(curwidth);

			/* ----- write phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((pattern_mode & 0x01) != 0) ? pattern_source : pattern_dest;
			memory_write_byte(space, busaddr, busdata);

			/* increment the appropriate address */
			if ((pattern_mode & 0x01) == 0)
				increment_dest(curwidth);
			else
				increment_source(curwidth, &u13ff);

			/* count 4 cycles (two read, two write) */
			cycles += 4;

		} while (curwidth-- != 0);

		/* at the end of each row, the skip value is added to the dest value */
		carry = ((pattern_dest & 0xff) + pattern_skip) & 0x100;
		pattern_dest = (pattern_dest & 0xff00) | ((pattern_dest + pattern_skip) & 0xff);

		/* carry behavior into the top byte is controlled by mode.d4 */
		if ((pattern_mode & 0x10) == 0)
			pattern_dest += carry;
		else
			pattern_dest -= carry ^ 0x100;

	} while (pattern_height-- != 0);

	/* count cycles we ran the bus */
	cpu_adjust_icount(space->cpu, -cycles);
}


WRITE8_HANDLER( astrocade_pattern_board_w )
{
	switch (offset)
	{
		case 0:		/* source offset low 8 bits */
			pattern_source = (pattern_source & 0xff00) | (data << 0);
			break;

		case 1:		/* source offset upper 8 bits */
			pattern_source = (pattern_source & 0x00ff) | (data << 8);
			break;

		case 2:		/* mode control; also clears low byte of dest */
			pattern_mode = data & 0x3f;
			pattern_dest &= 0xff00;
			break;

		case 3:		/* skip value */
			pattern_skip = data;
			break;

		case 4:		/* dest offset upper 8 bits; also adds skip to low 8 bits */
			pattern_dest = ((pattern_dest + pattern_skip) & 0xff) | (data << 8);
			break;

		case 5:		/* width of blit */
			pattern_width = data;
			break;

		case 6:		/* height of blit and initiator */
			pattern_height = data;
			execute_blit(cpu_get_address_space(space->cpu, ADDRESS_SPACE_PROGRAM));
			break;
	}
}



/*************************************
 *
 *  Sparkle/star circuit
 *
 *************************************/

/*
    Counters at U15/U16:
        On VERTDR, load 0x33 into counters at U15/U16
        On HORZDR, clock counters, stopping at overflow to 0x00 (prevents sparkle in VBLANK)

    Shift registers at U17/U12/U11:
        cleared on vertdr
        clocked at 7M (pixel clock)
        taps from bit 4, 8, 12, 16 control sparkle intensity

    Shift registers at U17/U19/U18:
        cleared on reset
        clocked at 7M (pixel clock)
        if bits 0-7 == 0xfe, a star is generated

    Both shift registers are the same with identical feedback.
    We use one array to hold both shift registers. Bits 0-3
    bits hold the intensity, and bit 4 holds whether or not
    a star is present.

    We must use independent lookups for each case. For the star
    lookup, we need to compute the pixel index relative to the
    end of VBLANK and use that (which at 455*262 is guaranteed
    to be less than RNG_PERIOD).

    For the sparkle lookup, we need to compute the pixel index
    relative to the beginning of time and use that, mod RNG_PERIOD.
*/

static void init_sparklestar(running_machine *machine)
{
	UINT32 shiftreg;
	int i;

	/* reset global sparkle state */
	astrocade_sparkle[0] = astrocade_sparkle[1] = astrocade_sparkle[2] = astrocade_sparkle[3] = 0;

	/* allocate memory for the sparkle/star array */
	sparklestar = auto_alloc_array(machine, UINT8, RNG_PERIOD);

	/* generate the data for the sparkle/star array */
	for (shiftreg = i = 0; i < RNG_PERIOD; i++)
	{
		UINT8 newbit;

		/* clock the shift register */
		newbit = ((shiftreg >> 12) ^ ~shiftreg) & 1;
		shiftreg = (shiftreg >> 1) | (newbit << 16);

		/* extract the sparkle/star intensity here */
		/* this is controlled by the shift register at U17/U19/U18 */
		sparklestar[i] = (((shiftreg >> 4) & 1) << 3) |
						 (((shiftreg >> 12) & 1) << 2) |
						 (((shiftreg >> 16) & 1) << 1) |
						 (((shiftreg >> 8) & 1) << 0);

		/* determine the star enable here */
		/* this is controlled by the shift register at U17/U12/U11 */
		if ((shiftreg & 0xff) == 0xfe)
			sparklestar[i] |= 0x10;
	}
}



/*************************************
 *
 *  16-color video board registers
 *
 *************************************/

WRITE8_HANDLER( profpac_page_select_w )
{
	profpac_readpage = data & 3;
	profpac_writepage = (data >> 2) & 3;
	profpac_vispage = (data >> 4) & 3;
}


READ8_HANDLER( profpac_intercept_r )
{
	return profpac_intercept;
}


WRITE8_HANDLER( profpac_screenram_ctrl_w )
{
	switch (offset)
	{
		case 0:		/* port 0xC0 - red component */
			profpac_palette[data >> 4] = (profpac_palette[data >> 4] & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 1:		/* port 0xC1 - green component */
			profpac_palette[data >> 4] = (profpac_palette[data >> 4] & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:		/* port 0xC2 - blue component */
			profpac_palette[data >> 4] = (profpac_palette[data >> 4] & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 3:		/* port 0xC3 - set 2bpp to 4bpp mapping and clear intercepts */
			profpac_colormap[(data >> 4) & 3] = data & 0x0f;
			profpac_intercept = 0x00;
			break;

		case 4:		/* port 0xC4 - which half to read on a memory access */
			profpac_vw = data & 0x0f;	/* refresh write enable lines TBD */
			profpac_readshift = 2 * ((data >> 4) & 1);
			break;

		case 5:		/* port 0xC5 - write enable and write mode */
			profpac_writemask = ((data & 0x0f) << 12) | ((data & 0x0f) << 8) | ((data & 0x0f) << 4) | ((data & 0x0f) << 0);
			profpac_writemode = (data >> 4) & 0x03;
			break;
	}
}



/*************************************
 *
 *  16-color video board VRAM access
 *
 *************************************/

READ8_HANDLER( profpac_videoram_r )
{
	UINT16 temp = profpac_videoram[profpac_readpage * 0x4000 + offset] >> profpac_readshift;
	return ((temp >> 6) & 0xc0) | ((temp >> 4) & 0x30) | ((temp >> 2) & 0x0c) | ((temp >> 0) & 0x03);
}


/* All this information comes from decoding the PLA at U39 on the screen ram board */
WRITE8_HANDLER( profpac_videoram_w )
{
	UINT16 oldbits = profpac_videoram[profpac_writepage * 0x4000 + offset];
	UINT16 newbits, result = 0;

	/* apply the 2->4 bit expansion first */
	newbits = (profpac_colormap[(data >> 6) & 3] << 12) |
			  (profpac_colormap[(data >> 4) & 3] << 8) |
			  (profpac_colormap[(data >> 2) & 3] << 4) |
			  (profpac_colormap[(data >> 0) & 3] << 0);

	/* there are 4 write modes: overwrite, xor, overlay, or underlay */
	switch (profpac_writemode)
	{
		case 0:		/* normal write */
			result = newbits;
			break;

		case 1:		/* xor write */
			result = newbits ^ oldbits;
			break;

		case 2:		/* overlay write */
			result  = ((newbits & 0xf000) == 0) ? (oldbits & 0xf000) : (newbits & 0xf000);
			result |= ((newbits & 0x0f00) == 0) ? (oldbits & 0x0f00) : (newbits & 0x0f00);
			result |= ((newbits & 0x00f0) == 0) ? (oldbits & 0x00f0) : (newbits & 0x00f0);
			result |= ((newbits & 0x000f) == 0) ? (oldbits & 0x000f) : (newbits & 0x000f);
			break;

		case 3: /* underlay write */
			result  = ((oldbits & 0xf000) != 0) ? (oldbits & 0xf000) : (newbits & 0xf000);
			result |= ((oldbits & 0x0f00) != 0) ? (oldbits & 0x0f00) : (newbits & 0x0f00);
			result |= ((oldbits & 0x00f0) != 0) ? (oldbits & 0x00f0) : (newbits & 0x00f0);
			result |= ((oldbits & 0x000f) != 0) ? (oldbits & 0x000f) : (newbits & 0x000f);
			break;
	}

	/* apply the write mask and store */
	result = (result & profpac_writemask) | (oldbits & ~profpac_writemask);
	profpac_videoram[profpac_writepage * 0x4000 + offset] = result;

	/* Intercept (collision) stuff */

	/* There are 3 bits on the register, which are set by various combinations of writes */
	if (((oldbits & 0xf000) == 0x2000 && (newbits & 0x8000) == 0x8000) ||
	    ((oldbits & 0xf000) == 0x3000 && (newbits & 0xc000) == 0x4000) ||
	    ((oldbits & 0x0f00) == 0x0200 && (newbits & 0x0800) == 0x0800) ||
	    ((oldbits & 0x0f00) == 0x0300 && (newbits & 0x0c00) == 0x0400) ||
	    ((oldbits & 0x00f0) == 0x0020 && (newbits & 0x0080) == 0x0080) ||
	    ((oldbits & 0x00f0) == 0x0030 && (newbits & 0x00c0) == 0x0040) ||
	    ((oldbits & 0x000f) == 0x0002 && (newbits & 0x0008) == 0x0008) ||
	    ((oldbits & 0x000f) == 0x0003 && (newbits & 0x000c) == 0x0004))
	    profpac_intercept |= 0x01;

	if (((newbits & 0xf000) != 0x0000 && (oldbits & 0xc000) == 0x4000) ||
	    ((newbits & 0x0f00) != 0x0000 && (oldbits & 0x0c00) == 0x0400) ||
	    ((newbits & 0x00f0) != 0x0000 && (oldbits & 0x00c0) == 0x0040) ||
	    ((newbits & 0x000f) != 0x0000 && (oldbits & 0x000c) == 0x0004))
	    profpac_intercept |= 0x02;

	if (((newbits & 0xf000) != 0x0000 && (oldbits & 0x8000) == 0x8000) ||
	    ((newbits & 0x0f00) != 0x0000 && (oldbits & 0x0800) == 0x0800) ||
	    ((newbits & 0x00f0) != 0x0000 && (oldbits & 0x0080) == 0x0080) ||
	    ((newbits & 0x000f) != 0x0000 && (oldbits & 0x0008) == 0x0008))
	    profpac_intercept |= 0x04;
}
