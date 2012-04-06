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
 *  Function prototypes
 *
 *************************************/

static void init_savestate(running_machine &machine);
static TIMER_CALLBACK( scanline_callback );
static void init_sparklestar(running_machine &machine);



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
	astrocde_state *state = machine.driver_data<astrocde_state>();
	/* allocate a per-scanline timer */
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_callback));
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(1), 1);

	/* register for save states */
	init_savestate(machine);

	/* initialize the sparkle and stars */
	if (state->m_video_config & AC_STARS)
		init_sparklestar(machine);
}


VIDEO_START( profpac )
{
	astrocde_state *state = machine.driver_data<astrocde_state>();
	/* allocate a per-scanline timer */
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(scanline_callback));
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(1), 1);

	/* allocate videoram */
	state->m_profpac_videoram = auto_alloc_array(machine, UINT16, 0x4000 * 4);

	/* register for save states */
	init_savestate(machine);

	/* register our specific save state data */
	state_save_register_global_pointer(machine, state->m_profpac_videoram, 0x4000 * 4);
	state_save_register_global_array(machine, state->m_profpac_palette);
	state_save_register_global_array(machine, state->m_profpac_colormap);
	state_save_register_global(machine, state->m_profpac_intercept);
	state_save_register_global(machine, state->m_profpac_vispage);
	state_save_register_global(machine, state->m_profpac_readpage);
	state_save_register_global(machine, state->m_profpac_readshift);
	state_save_register_global(machine, state->m_profpac_writepage);
	state_save_register_global(machine, state->m_profpac_writemode);
	state_save_register_global(machine, state->m_profpac_writemask);
	state_save_register_global(machine, state->m_profpac_vw);
}


static void init_savestate(running_machine &machine)
{
	astrocde_state *state = machine.driver_data<astrocde_state>();
	state_save_register_global_array(machine, state->m_sparkle);

	state_save_register_global(machine, state->m_interrupt_enabl);
	state_save_register_global(machine, state->m_interrupt_vector);
	state_save_register_global(machine, state->m_interrupt_scanline);
	state_save_register_global(machine, state->m_vertical_feedback);
	state_save_register_global(machine, state->m_horizontal_feedback);

	state_save_register_global_array(machine, state->m_colors);
	state_save_register_global(machine, state->m_colorsplit);
	state_save_register_global(machine, state->m_bgdata);
	state_save_register_global(machine, state->m_vblank);
	state_save_register_global(machine, state->m_video_mode);

	state_save_register_global_array(machine, state->m_funcgen_expand_color);
	state_save_register_global(machine, state->m_funcgen_control);
	state_save_register_global(machine, state->m_funcgen_expand_count);
	state_save_register_global(machine, state->m_funcgen_rotate_count);
	state_save_register_global_array(machine, state->m_funcgen_rotate_data);
	state_save_register_global(machine, state->m_funcgen_shift_prev_data);
	state_save_register_global(machine, state->m_funcgen_intercept);

	state_save_register_global(machine, state->m_pattern_source);
	state_save_register_global(machine, state->m_pattern_mode);
	state_save_register_global(machine, state->m_pattern_dest);
	state_save_register_global(machine, state->m_pattern_skip);
	state_save_register_global(machine, state->m_pattern_width);
	state_save_register_global(machine, state->m_pattern_height);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( astrocde )
{
	astrocde_state *state = screen.machine().driver_data<astrocde_state>();
	UINT8 *videoram = state->m_videoram;
	UINT32 sparklebase = 0;
	const int colormask = (state->m_video_config & AC_MONITOR_BW) ? 0 : 0x1f0;
	int xystep = 2 - state->m_video_mode;
	int y;

	/* compute the starting point of sparkle for the current frame */
	int width = screen.width();
	int height = screen.height();

	if (state->m_video_config & AC_STARS)
		sparklebase = (screen.frame_number() * (UINT64)(width * height)) % RNG_PERIOD;

	/* iterate over scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		int effy = mame_vpos_to_astrocade_vpos(y);
		UINT16 offset = (effy / xystep) * (80 / xystep);
		UINT32 sparkleoffs = 0, staroffs = 0;
		int x;

		/* compute the star and sparkle offset at the start of this line */
		if (state->m_video_config & AC_STARS)
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
			const UINT8 *colorbase = &state->m_colors[(effx < state->m_colorsplit) ? 4 : 0];
			UINT8 data;
			int xx;

			/* select either video data or background data */
			data = (effx >= 0 && effx < 80 && effy >= 0 && effy < state->m_vblank) ? videoram[offset++] : state->m_bgdata;

			/* iterate over the 4 pixels */
			for (xx = 0; xx < 4; xx++)
			{
				UINT8 pixdata = (data >> 6) & 3;
				int colordata = colorbase[pixdata] << 1;
				int luma = colordata & 0x0f;
				rgb_t color;

				/* handle stars/sparkle */
				if (state->m_video_config & AC_STARS)
				{
					/* if sparkle is enabled for this pixel index and either it is non-zero or a star */
					/* then adjust the intensity */
					if (state->m_sparkle[pixdata] == 0)
					{
						if (pixdata != 0 || (state->m_sparklestar[staroffs] & 0x10))
							luma = state->m_sparklestar[sparkleoffs] & 0x0f;
						else if (pixdata == 0)
							colordata = luma = 0;
					}

					/* update sparkle/star offsets */
					staroffs++;
					if (++sparkleoffs >= RNG_PERIOD)
						sparkleoffs = 0;
				}
				color = (colordata & colormask) | luma;

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


SCREEN_UPDATE_IND16( profpac )
{
	astrocde_state *state = screen.machine().driver_data<astrocde_state>();
	int y;

	/* iterate over scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int effy = mame_vpos_to_astrocade_vpos(y);
		UINT16 *dest = &bitmap.pix16(y);
		UINT16 offset = state->m_profpac_vispage * 0x4000 + effy * 80;
		int x;

		/* star with black */

		/* iterate over groups of 4 pixels */
		for (x = 0; x < 456/4; x++)
		{
			int effx = x - HORZ_OFFSET/4;

			/* select either video data or background data */
			UINT16 data = (effx >= 0 && effx < 80 && effy >= 0 && effy < state->m_vblank) ? state->m_profpac_videoram[offset++] : 0;

			/* iterate over the 4 pixels */
			*dest++ = state->m_profpac_palette[(data >> 12) & 0x0f];
			*dest++ = state->m_profpac_palette[(data >> 8) & 0x0f];
			*dest++ = state->m_profpac_palette[(data >> 4) & 0x0f];
			*dest++ = state->m_profpac_palette[(data >> 0) & 0x0f];
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


static void astrocade_trigger_lightpen(running_machine &machine, UINT8 vfeedback, UINT8 hfeedback)
{
	astrocde_state *state = machine.driver_data<astrocde_state>();
	/* both bits 1 and 4 enable lightpen interrupts; bit 4 enables them even in horizontal */
	/* blanking regions; we treat them both the same here */
	if ((state->m_interrupt_enabl & 0x12) != 0)
	{
		/* bit 0 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((state->m_interrupt_enabl & 0x01) == 0)
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, state->m_interrupt_vector & 0xf0);
			machine.scheduler().timer_set(machine.primary_screen->time_until_pos(vfeedback), FUNC(interrupt_off));
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, ASSERT_LINE, state->m_interrupt_vector & 0xf0);
			machine.scheduler().timer_set(machine.device<cpu_device>("maincpu")->cycles_to_attotime(1), FUNC(interrupt_off));
		}

		/* latch the feedback registers */
		state->m_vertical_feedback = vfeedback;
		state->m_horizontal_feedback = hfeedback;
	}
}



/*************************************
 *
 *  Per-scanline callback
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	astrocde_state *state = machine.driver_data<astrocde_state>();
	int scanline = param;
	int astrocade_scanline = mame_vpos_to_astrocade_vpos(scanline);

	/* force an update against the current scanline */
	if (scanline > 0)
		machine.primary_screen->update_partial(scanline - 1);

	/* generate a scanline interrupt if it's time */
	if (astrocade_scanline == state->m_interrupt_scanline && (state->m_interrupt_enabl & 0x08) != 0)
	{
		/* bit 2 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((state->m_interrupt_enabl & 0x04) == 0)
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, state->m_interrupt_vector);
			machine.scheduler().timer_set(machine.primary_screen->time_until_vblank_end(), FUNC(interrupt_off));
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			cputag_set_input_line_and_vector(machine, "maincpu", 0, ASSERT_LINE, state->m_interrupt_vector);
			machine.scheduler().timer_set(machine.device<cpu_device>("maincpu")->cycles_to_attotime(1), FUNC(interrupt_off));
		}
	}

	/* on some games, the horizontal drive line is conected to the lightpen interrupt */
	else if (state->m_video_config & AC_LIGHTPEN_INTS)
		astrocade_trigger_lightpen(machine, astrocade_scanline, 8);

	/* advance to the next scanline */
	scanline++;
	if (scanline >= machine.primary_screen->height())
		scanline = 0;
	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(scanline), scanline);
}



/*************************************
 *
 *  Data chip registers
 *
 *************************************/

READ8_MEMBER(astrocde_state::astrocade_data_chip_register_r)
{
	UINT8 result = 0xff;

	/* these are the core registers */
	switch (offset & 0xff)
	{
		case 0x08:	/* intercept feedback */
			result = m_funcgen_intercept;
			m_funcgen_intercept = 0;
			break;

		case 0x0e:	/* vertical feedback (from lightpen interrupt) */
			result = m_vertical_feedback;
			break;

		case 0x0f:	/* horizontal feedback (from lightpen interrupt) */
			result = m_horizontal_feedback;
			break;

		case 0x10:	/* player 1 handle */
			result = input_port_read_safe(machine(), "P1HANDLE", 0xff);
			break;

		case 0x11:	/* player 2 handle */
			result = input_port_read_safe(machine(), "P2HANDLE", 0xff);
			break;

		case 0x12:	/* player 3 handle */
			result = input_port_read_safe(machine(), "P3HANDLE", 0xff);
			break;

		case 0x13:	/* player 4 handle */
			result = input_port_read_safe(machine(), "P4HANDLE", 0xff);
			break;

		case 0x14:	/* keypad column 0 */
			result = input_port_read_safe(machine(), "KEYPAD0", 0xff);
			break;

		case 0x15:	/* keypad column 1 */
			result = input_port_read_safe(machine(), "KEYPAD1", 0xff);
			break;

		case 0x16:	/* keypad column 2 */
			result = input_port_read_safe(machine(), "KEYPAD2", 0xff);
			break;

		case 0x17:	/* keypad column 3 */
			result = input_port_read_safe(machine(), "KEYPAD3", 0xff);
			break;

		case 0x1c:	/* player 1 knob */
			result = input_port_read_safe(machine(), "P1_KNOB", 0xff);
			break;

		case 0x1d:	/* player 2 knob */
			result = input_port_read_safe(machine(), "P2_KNOB", 0xff);
			break;

		case 0x1e:	/* player 3 knob */
			result = input_port_read_safe(machine(), "P3_KNOB", 0xff);
			break;

		case 0x1f:	/* player 4 knob */
			result = input_port_read_safe(machine(), "P4_KNOB", 0xff);
			break;
	}

	return result;
}


WRITE8_MEMBER(astrocde_state::astrocade_data_chip_register_w)
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
			m_colors[offset & 7] = data;
			break;

		case 0x08:	/* mode register */
			m_video_mode = data & 1;
			break;

		case 0x09:	/* color split pixel */
			m_colorsplit = 2 * (data & 0x3f);
			m_bgdata = ((data & 0xc0) >> 6) * 0x55;
			break;

		case 0x0a:	/* vertical blank register */
			m_vblank = data;
			break;

		case 0x0b:	/* color block transfer */
			m_colors[(offset >> 8) & 7] = data;
			break;

		case 0x0c:	/* function generator */
			m_funcgen_control = data;
			m_funcgen_expand_count = 0;		/* reset flip-flop for expand mode on write to this register */
			m_funcgen_rotate_count = 0;		/* reset counter for rotate mode on write to this register */
			m_funcgen_shift_prev_data = 0;	/* reset shift buffer on write to this register */
			break;

		case 0x0d:	/* interrupt feedback */
			m_interrupt_vector = data;
			break;

		case 0x0e:	/* interrupt enable and mode */
			m_interrupt_enabl = data;
			break;

		case 0x0f:	/* interrupt line */
			m_interrupt_scanline = data;
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
			if (m_video_config & AC_SOUND_PRESENT)
				astrocade_sound_w(machine().device("astrocade1"), offset, data);
			break;

		case 0x19:	/* expand register */
			m_funcgen_expand_color[0] = data & 0x03;
			m_funcgen_expand_color[1] = (data >> 2) & 0x03;
			break;
	}
}



/*************************************
 *
 *  Function generator
 *
 *************************************/

WRITE8_MEMBER(astrocde_state::astrocade_funcgen_w)
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
	if (m_funcgen_control & 0x08)
	{
		m_funcgen_expand_count ^= 1;
		data >>= 4 * m_funcgen_expand_count;
		data =	(m_funcgen_expand_color[(data >> 3) & 1] << 6) |
				(m_funcgen_expand_color[(data >> 2) & 1] << 4) |
				(m_funcgen_expand_color[(data >> 1) & 1] << 2) |
				(m_funcgen_expand_color[(data >> 0) & 1] << 0);
	}
	prev_data = m_funcgen_shift_prev_data;
	m_funcgen_shift_prev_data = data;

	/* rotate or shift */
	if (m_funcgen_control & 0x04)
	{
		/* rotate */

		/* first 4 writes accumulate data for the rotate */
		if ((m_funcgen_rotate_count & 4) == 0)
		{
			m_funcgen_rotate_data[m_funcgen_rotate_count++ & 3] = data;
			return;
		}

		/* second 4 writes actually write it */
		else
		{
			UINT8 shift = 2 * (~m_funcgen_rotate_count++ & 3);
			data =	(((m_funcgen_rotate_data[3] >> shift) & 3) << 6) |
					(((m_funcgen_rotate_data[2] >> shift) & 3) << 4) |
					(((m_funcgen_rotate_data[1] >> shift) & 3) << 2) |
					(((m_funcgen_rotate_data[0] >> shift) & 3) << 0);
		}
	}
	else
	{
		/* shift */
		UINT8 shift = 2 * (m_funcgen_control & 0x03);
		data = (data >> shift) | (prev_data << (8 - shift));
	}

	/* flopping */
	if (m_funcgen_control & 0x40)
		data = (data >> 6) | ((data >> 2) & 0x0c) | ((data << 2) & 0x30) | (data << 6);

	/* OR/XOR */
	if (m_funcgen_control & 0x30)
	{
		UINT8 olddata = space.read_byte(0x4000 + offset);

		/* compute any intercepts */
		m_funcgen_intercept &= 0x0f;
		if ((olddata & 0xc0) && (data & 0xc0))
			m_funcgen_intercept |= 0x11;
		if ((olddata & 0x30) && (data & 0x30))
			m_funcgen_intercept |= 0x22;
		if ((olddata & 0x0c) && (data & 0x0c))
			m_funcgen_intercept |= 0x44;
		if ((olddata & 0x03) && (data & 0x03))
			m_funcgen_intercept |= 0x88;

		/* apply the operation */
		if (m_funcgen_control & 0x10)
			data |= olddata;
		else if (m_funcgen_control & 0x20)
			data ^= olddata;
	}

	/* write the result */
	space.write_byte(0x4000 + offset, data);
}



/*************************************
 *
 *  Pattern board
 *
 *************************************/

INLINE void increment_source(astrocde_state *state, UINT8 curwidth, UINT8 *u13ff)
{
	/* if the flip-flop at U13 is high and mode.d2 is 1 we can increment */
	/* however, if mode.d3 is set and we're on the last byte of a row, the increment is suppressed */
	if (*u13ff && (state->m_pattern_mode & 0x04) != 0 && (curwidth != 0 || (state->m_pattern_mode & 0x08) == 0))
		state->m_pattern_source++;

	/* if mode.d1 is 1, toggle the flip-flop; otherwise leave it preset */
	if ((state->m_pattern_mode & 0x02) != 0)
		*u13ff ^= 1;
}


INLINE void increment_dest(astrocde_state *state, UINT8 curwidth)
{
	/* increment is suppressed for the last byte in a row */
	if (curwidth != 0)
	{
		/* if mode.d5 is 1, we increment */
		if ((state->m_pattern_mode & 0x20) != 0)
			state->m_pattern_dest++;

		/* otherwise, we decrement */
		else
			state->m_pattern_dest--;
	}
}


static void execute_blit(address_space *space)
{
	astrocde_state *state = space->machine().driver_data<astrocde_state>();
	/*
        state->m_pattern_source = counter set U7/U16/U25/U34
        state->m_pattern_dest = counter set U9/U18/U30/U39
        state->m_pattern_mode = latch U21
        state->m_pattern_skip = latch set U30/U39
        state->m_pattern_width = latch set U32/U41
        state->m_pattern_height = counter set U31/U40

        state->m_pattern_mode bits:
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
            state->m_pattern_source, state->m_pattern_mode, state->m_pattern_dest, state->m_pattern_skip, state->m_pattern_width, state->m_pattern_height);*/

	/* flip-flop at U13 is cleared at the beginning */
	u13ff = 0;

	/* it is also forced preset if mode.d1 == 0 */
	if ((state->m_pattern_mode & 0x02) == 0)
		u13ff = 1;

	/* loop over height */
	do
	{
		UINT16 carry;

		/* loop over width */
		curwidth = state->m_pattern_width;
		do
		{
			UINT16 busaddr;
			UINT8 busdata;

			/* ----- read phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((state->m_pattern_mode & 0x01) == 0) ? state->m_pattern_source : state->m_pattern_dest;

			/* if mode.d3 is set, then the last byte fetched per row is forced to 0 */
			if (curwidth == 0 && (state->m_pattern_mode & 0x08) != 0)
				busdata = 0;
			else
				busdata = space->read_byte(busaddr);

			/* increment the appropriate address */
			if ((state->m_pattern_mode & 0x01) == 0)
				increment_source(state, curwidth, &u13ff);
			else
				increment_dest(state, curwidth);

			/* ----- write phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((state->m_pattern_mode & 0x01) != 0) ? state->m_pattern_source : state->m_pattern_dest;
			space->write_byte(busaddr, busdata);

			/* increment the appropriate address */
			if ((state->m_pattern_mode & 0x01) == 0)
				increment_dest(state, curwidth);
			else
				increment_source(state, curwidth, &u13ff);

			/* count 4 cycles (two read, two write) */
			cycles += 4;

		} while (curwidth-- != 0);

		/* at the end of each row, the skip value is added to the dest value */
		carry = ((state->m_pattern_dest & 0xff) + state->m_pattern_skip) & 0x100;
		state->m_pattern_dest = (state->m_pattern_dest & 0xff00) | ((state->m_pattern_dest + state->m_pattern_skip) & 0xff);

		/* carry behavior into the top byte is controlled by mode.d4 */
		if ((state->m_pattern_mode & 0x10) == 0)
			state->m_pattern_dest += carry;
		else
			state->m_pattern_dest -= carry ^ 0x100;

	} while (state->m_pattern_height-- != 0);

	/* count cycles we ran the bus */
	device_adjust_icount(&space->device(), -cycles);
}


WRITE8_MEMBER(astrocde_state::astrocade_pattern_board_w)
{
	switch (offset)
	{
		case 0:		/* source offset low 8 bits */
			m_pattern_source = (m_pattern_source & 0xff00) | (data << 0);
			break;

		case 1:		/* source offset upper 8 bits */
			m_pattern_source = (m_pattern_source & 0x00ff) | (data << 8);
			break;

		case 2:		/* mode control; also clears low byte of dest */
			m_pattern_mode = data & 0x3f;
			m_pattern_dest &= 0xff00;
			break;

		case 3:		/* skip value */
			m_pattern_skip = data;
			break;

		case 4:		/* dest offset upper 8 bits; also adds skip to low 8 bits */
			m_pattern_dest = ((m_pattern_dest + m_pattern_skip) & 0xff) | (data << 8);
			break;

		case 5:		/* width of blit */
			m_pattern_width = data;
			break;

		case 6:		/* height of blit and initiator */
			m_pattern_height = data;
			execute_blit(space.device().memory().space(AS_PROGRAM));
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

static void init_sparklestar(running_machine &machine)
{
	astrocde_state *state = machine.driver_data<astrocde_state>();
	UINT32 shiftreg;
	int i;

	/* reset global sparkle state */
	state->m_sparkle[0] = state->m_sparkle[1] = state->m_sparkle[2] = state->m_sparkle[3] = 0;

	/* allocate memory for the sparkle/star array */
	state->m_sparklestar = auto_alloc_array(machine, UINT8, RNG_PERIOD);

	/* generate the data for the sparkle/star array */
	for (shiftreg = i = 0; i < RNG_PERIOD; i++)
	{
		UINT8 newbit;

		/* clock the shift register */
		newbit = ((shiftreg >> 12) ^ ~shiftreg) & 1;
		shiftreg = (shiftreg >> 1) | (newbit << 16);

		/* extract the sparkle/star intensity here */
		/* this is controlled by the shift register at U17/U19/U18 */
		state->m_sparklestar[i] = (((shiftreg >> 4) & 1) << 3) |
						 (((shiftreg >> 12) & 1) << 2) |
						 (((shiftreg >> 16) & 1) << 1) |
						 (((shiftreg >> 8) & 1) << 0);

		/* determine the star enable here */
		/* this is controlled by the shift register at U17/U12/U11 */
		if ((shiftreg & 0xff) == 0xfe)
			state->m_sparklestar[i] |= 0x10;
	}
}



/*************************************
 *
 *  16-color video board registers
 *
 *************************************/

WRITE8_MEMBER(astrocde_state::profpac_page_select_w)
{
	m_profpac_readpage = data & 3;
	m_profpac_writepage = (data >> 2) & 3;
	m_profpac_vispage = (data >> 4) & 3;
}


READ8_MEMBER(astrocde_state::profpac_intercept_r)
{
	return m_profpac_intercept;
}


WRITE8_MEMBER(astrocde_state::profpac_screenram_ctrl_w)
{
	switch (offset)
	{
		case 0:		/* port 0xC0 - red component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 1:		/* port 0xC1 - green component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:		/* port 0xC2 - blue component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 3:		/* port 0xC3 - set 2bpp to 4bpp mapping and clear intercepts */
			m_profpac_colormap[(data >> 4) & 3] = data & 0x0f;
			m_profpac_intercept = 0x00;
			break;

		case 4:		/* port 0xC4 - which half to read on a memory access */
			m_profpac_vw = data & 0x0f;	/* refresh write enable lines TBD */
			m_profpac_readshift = 2 * ((data >> 4) & 1);
			break;

		case 5:		/* port 0xC5 - write enable and write mode */
			m_profpac_writemask = ((data & 0x0f) << 12) | ((data & 0x0f) << 8) | ((data & 0x0f) << 4) | ((data & 0x0f) << 0);
			m_profpac_writemode = (data >> 4) & 0x03;
			break;
	}
}



/*************************************
 *
 *  16-color video board VRAM access
 *
 *************************************/

READ8_MEMBER(astrocde_state::profpac_videoram_r)
{
	UINT16 temp = m_profpac_videoram[m_profpac_readpage * 0x4000 + offset] >> m_profpac_readshift;
	return ((temp >> 6) & 0xc0) | ((temp >> 4) & 0x30) | ((temp >> 2) & 0x0c) | ((temp >> 0) & 0x03);
}


/* All this information comes from decoding the PLA at U39 on the screen ram board */
WRITE8_MEMBER(astrocde_state::profpac_videoram_w)
{
	UINT16 oldbits = m_profpac_videoram[m_profpac_writepage * 0x4000 + offset];
	UINT16 newbits, result = 0;

	/* apply the 2->4 bit expansion first */
	newbits = (m_profpac_colormap[(data >> 6) & 3] << 12) |
			  (m_profpac_colormap[(data >> 4) & 3] << 8) |
			  (m_profpac_colormap[(data >> 2) & 3] << 4) |
			  (m_profpac_colormap[(data >> 0) & 3] << 0);

	/* there are 4 write modes: overwrite, xor, overlay, or underlay */
	switch (m_profpac_writemode)
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
	result = (result & m_profpac_writemask) | (oldbits & ~m_profpac_writemask);
	m_profpac_videoram[m_profpac_writepage * 0x4000 + offset] = result;

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
	    m_profpac_intercept |= 0x01;

	if (((newbits & 0xf000) != 0x0000 && (oldbits & 0xc000) == 0x4000) ||
	    ((newbits & 0x0f00) != 0x0000 && (oldbits & 0x0c00) == 0x0400) ||
	    ((newbits & 0x00f0) != 0x0000 && (oldbits & 0x00c0) == 0x0040) ||
	    ((newbits & 0x000f) != 0x0000 && (oldbits & 0x000c) == 0x0004))
	    m_profpac_intercept |= 0x02;

	if (((newbits & 0xf000) != 0x0000 && (oldbits & 0x8000) == 0x8000) ||
	    ((newbits & 0x0f00) != 0x0000 && (oldbits & 0x0800) == 0x0800) ||
	    ((newbits & 0x00f0) != 0x0000 && (oldbits & 0x0080) == 0x0080) ||
	    ((newbits & 0x000f) != 0x0000 && (oldbits & 0x0008) == 0x0008))
	    m_profpac_intercept |= 0x04;
}
