// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles
/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/

#include "emu.h"
#include "includes/astrocde.h"

#include "cpu/z80/z80.h"
#include "sound/astrocde.h"
#include "video/resnet.h"

#include <cmath>


/*************************************
 *
 *  Machine setup
 *
 *************************************/

void astrocde_state::machine_start()
{
	save_item(NAME(m_ram_write_enable));
	save_item(NAME(m_input_select));

	m_input_select = 0;
}

void seawolf2_state::machine_start()
{
	astrocde_state::machine_start();

	save_item(NAME(m_port_1_last));
	save_item(NAME(m_port_2_last));

	m_port_1_last = m_port_2_last = 0xff;
}

void tenpindx_state::machine_start()
{
	astrocde_state::machine_start();

	m_lamps.resolve();
}



/*************************************
 *
 *  Constants
 *
 *************************************/

#define RNG_PERIOD      ((1 << 17) - 1)
#define VERT_OFFSET     (22)                /* pixels from top of screen to top of game area */
#define HORZ_OFFSET     (16)                /* pixels from left of screen to left of game area */

/*************************************
 *
 *  Scanline conversion
 *
 *************************************/

inline int astrocde_state::mame_vpos_to_astrocade_vpos(int scanline)
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

void astrocde_state::astrocade_palette(palette_device &palette) const
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

	// loop over color values
	for (int color = 0; color < 32; color++)
	{
		// color 0 maps to ry = by = 0
		double const angle = (color / 32.0) * (2.0 * M_PI);
		float const ry = color ? (0.75 * std::sin(angle)) : 0;
		float const by = color ? (1.15 * std::cos(angle)) : 0;

		// iterate over luminence values
		for (int luma = 0; luma < 16; luma++)
		{
			float const y = luma / 15.0;

			// transform to RGB
			int r = (ry + y) * 255;
			int g = ((y - 0.299f * (ry + y) - 0.114f * (by + y)) / 0.587f) * 255;
			int b = (by + y) * 255;

			// clamp and store
			r = std::clamp(r, 0, 255);
			g = std::clamp(g, 0, 255);
			b = std::clamp(b, 0, 255);
			palette.set_pen_color(color * 16 + luma, rgb_t(r, g, b));
		}
	}
}


void astrocde_state::profpac_palette(palette_device &palette) const
{
	// Professor Pac-Man uses a more standard 12-bit RGB palette layout
	static constexpr int resistances[4] = { 6200, 3000, 1500, 750 };

	// compute the color output resistor weights
	double weights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, weights, 1500, 0,
			4, resistances, weights, 1500, 0,
			4, resistances, weights, 1500, 0);

	// initialize the palette with these colors
	for (int i = 0; i < 4096; i++)
	{
		int bit0, bit1, bit2, bit3;

		// blue component
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 1);
		bit2 = BIT(i, 2);
		bit3 = BIT(i, 3);
		int const b = combine_weights(weights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(i, 4);
		bit1 = BIT(i, 5);
		bit2 = BIT(i, 6);
		bit3 = BIT(i, 7);
		int const g = combine_weights(weights, bit0, bit1, bit2, bit3);

		// red component
		bit0 = BIT(i, 8);
		bit1 = BIT(i, 9);
		bit2 = BIT(i, 10);
		bit3 = BIT(i, 11);
		int const r = combine_weights(weights, bit0, bit1, bit2, bit3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void astrocde_state::video_start()
{
	/* allocate timers */
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
	m_scanline_timer->adjust(m_screen->time_until_pos(1), 1);
	m_intoff_timer = timer_alloc(TIMER_INTERRUPT_OFF);

	/* register for save states */
	init_savestate();

	/* initialize the sparkle and stars */
	if (m_video_config & AC_STARS)
		init_sparklestar();
}


VIDEO_START_MEMBER(astrocde_state,profpac)
{
	/* allocate timers */
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
	m_scanline_timer->adjust(m_screen->time_until_pos(1), 1);
	m_intoff_timer = timer_alloc(TIMER_INTERRUPT_OFF);

	/* allocate videoram */
	m_profpac_videoram = std::make_unique<uint16_t[]>(0x4000 * 4);

	/* register for save states */
	init_savestate();

	/* register our specific save state data */
	save_pointer(NAME(m_profpac_videoram), 0x4000 * 4);
	save_item(NAME(m_profpac_palette));
	save_item(NAME(m_profpac_colormap));
	save_item(NAME(m_profpac_intercept));
	save_item(NAME(m_profpac_vispage));
	save_item(NAME(m_profpac_readpage));
	save_item(NAME(m_profpac_readshift));
	save_item(NAME(m_profpac_writepage));
	save_item(NAME(m_profpac_writemode));
	save_item(NAME(m_profpac_writemask));
	save_item(NAME(m_profpac_vw));

	std::fill(std::begin(m_profpac_palette), std::end(m_profpac_palette), 0);

	m_pattern_height = 0;
	m_pattern_width = 0;
}


void astrocde_state::init_savestate()
{
	save_item(NAME(m_sparkle));

	save_item(NAME(m_interrupt_enabl));
	save_item(NAME(m_interrupt_vector));
	save_item(NAME(m_interrupt_scanline));
	save_item(NAME(m_vertical_feedback));
	save_item(NAME(m_horizontal_feedback));

	save_item(NAME(m_colors));
	save_item(NAME(m_colorsplit));
	save_item(NAME(m_bgdata));
	save_item(NAME(m_vblank));
	save_item(NAME(m_video_mode));

	save_item(NAME(m_funcgen_expand_color));
	save_item(NAME(m_funcgen_control));
	save_item(NAME(m_funcgen_expand_count));
	save_item(NAME(m_funcgen_rotate_count));
	save_item(NAME(m_funcgen_rotate_data));
	save_item(NAME(m_funcgen_shift_prev_data));
	save_item(NAME(m_funcgen_intercept));

	save_item(NAME(m_pattern_source));
	save_item(NAME(m_pattern_mode));
	save_item(NAME(m_pattern_dest));
	save_item(NAME(m_pattern_skip));
	save_item(NAME(m_pattern_width));
	save_item(NAME(m_pattern_height));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t astrocde_state::screen_update_astrocde(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const videoram = m_videoram;
	uint32_t sparklebase = 0;
	const int colormask = (m_video_config & AC_MONITOR_BW) ? 0 : 0x1f0;
	int xystep = 2 - m_video_mode;

	/* compute the starting point of sparkle for the current frame */
	int width = screen.width();
	int height = screen.height();

	if (m_video_config & AC_STARS)
		sparklebase = (screen.frame_number() * (uint64_t)(width * height)) % RNG_PERIOD;

	/* iterate over scanlines */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *dest = &bitmap.pix(y);
		int effy = mame_vpos_to_astrocade_vpos(y);
		uint16_t offset = (effy / xystep) * (80 / xystep);
		uint32_t sparkleoffs = 0, staroffs = 0;

		/* compute the star and sparkle offset at the start of this line */
		if (m_video_config & AC_STARS)
		{
			staroffs = ((effy < 0) ? (effy + 262) : effy) * width;
			sparkleoffs = sparklebase + y * width;
			if (sparkleoffs >= RNG_PERIOD)
				sparkleoffs -= RNG_PERIOD;
		}

		/* iterate over groups of 4 pixels */
		for (int x = 0; x < 456/4; x += xystep)
		{
			int effx = x - HORZ_OFFSET/4;
			const uint8_t *colorbase = &m_colors[(effx < m_colorsplit) ? 4 : 0];

			/* select either video data or background data */
			uint8_t data = (effx >= 0 && effx < 80 && effy >= 0 && effy < m_vblank) ? videoram[offset++] : m_bgdata;

			/* iterate over the 4 pixels */
			for (int xx = 0; xx < 4; xx++)
			{
				uint8_t pixdata = (data >> 6) & 3;
				int colordata = colorbase[pixdata] << 1;
				int luma = colordata & 0x0f;

				/* handle stars/sparkle */
				if (m_video_config & AC_STARS)
				{
					/* if sparkle is enabled for this pixel index and either it is non-zero or a star */
					/* then adjust the intensity */
					if (m_sparkle[pixdata] == 0)
					{
						if (pixdata != 0 || (m_sparklestar[staroffs] & 0x10))
							luma = m_sparklestar[sparkleoffs] & 0x0f;
						else if (pixdata == 0)
							colordata = luma = 0;
					}

					/* update sparkle/star offsets */
					staroffs++;
					if (++sparkleoffs >= RNG_PERIOD)
						sparkleoffs = 0;
				}
				rgb_t const color = (colordata & colormask) | luma;

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


uint32_t astrocde_state::screen_update_profpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* iterate over scanlines */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int effy = mame_vpos_to_astrocade_vpos(y);
		uint16_t *dest = &bitmap.pix(y);
		uint16_t offset = m_profpac_vispage * 0x4000 + effy * 80;

		/* star with black */

		/* iterate over groups of 4 pixels */
		for (int x = 0; x < 456/4; x++)
		{
			int effx = x - HORZ_OFFSET/4;

			/* select either video data or background data */
			uint16_t data = (effx >= 0 && effx < 80 && effy >= 0 && effy < m_vblank) ? m_profpac_videoram[offset++] : 0;

			/* iterate over the 4 pixels */
			*dest++ = m_profpac_palette[(data >> 12) & 0x0f];
			*dest++ = m_profpac_palette[(data >> 8) & 0x0f];
			*dest++ = m_profpac_palette[(data >> 4) & 0x0f];
			*dest++ = m_profpac_palette[(data >> 0) & 0x0f];
		}
	}

	return 0;
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

void astrocde_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_INTERRUPT_OFF:
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;
	case TIMER_SCANLINE:
		scanline_callback(ptr, param);
		break;
	default:
		throw emu_fatalerror("Unknown id in astrocde_state::device_timer");
	}
}

WRITE_LINE_MEMBER(astrocde_state::lightpen_trigger_w)
{
	if (state)
	{
		uint8_t res_shift = 1 - m_video_mode;
		astrocade_trigger_lightpen(mame_vpos_to_astrocade_vpos(m_screen->vpos()) & ~res_shift, (m_screen->hpos() >> res_shift) + 12);
	}
}

void astrocde_state::astrocade_trigger_lightpen(uint8_t vfeedback, uint8_t hfeedback)
{
	/* both bits 1 and 4 enable lightpen interrupts; bit 4 enables them even in horizontal */
	/* blanking regions; we treat them both the same here */
	if ((m_interrupt_enabl & 0x12) != 0)
	{
		/* bit 0 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((m_interrupt_enabl & 0x01) == 0)
		{
			m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_interrupt_vector & 0xf0); // Z80
			m_intoff_timer->adjust(m_screen->time_until_pos(vfeedback));
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, m_interrupt_vector & 0xf0); // Z80
			m_intoff_timer->adjust(m_maincpu->cycles_to_attotime(1));
		}

		/* latch the feedback registers */
		m_vertical_feedback = vfeedback;
		m_horizontal_feedback = hfeedback;
	}
}



/*************************************
 *
 *  Per-scanline callback
 *
 *************************************/

TIMER_CALLBACK_MEMBER(astrocde_state::scanline_callback)
{
	int scanline = param;
	int astrocade_scanline = mame_vpos_to_astrocade_vpos(scanline);

	/* force an update against the current scanline */
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	/* generate a scanline interrupt if it's time */
	if (astrocade_scanline == m_interrupt_scanline && (m_interrupt_enabl & 0x08) != 0)
	{
		/* bit 2 controls the interrupt mode: mode 0 means assert until acknowledged */
		if ((m_interrupt_enabl & 0x04) == 0)
		{
			m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_interrupt_vector); // Z80
			m_intoff_timer->adjust(m_screen->time_until_vblank_end());
		}

		/* mode 1 means assert for 1 instruction */
		else
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, m_interrupt_vector); // Z80
			m_intoff_timer->adjust(m_maincpu->cycles_to_attotime(1));
		}
	}

	/* on some games, the horizontal drive line is conected to the lightpen interrupt */
	else if (m_video_config & AC_LIGHTPEN_INTS)
		astrocade_trigger_lightpen(astrocade_scanline, 8);

	/* advance to the next scanline */
	scanline++;
	if (scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}



/*************************************
 *
 *  Data chip registers
 *
 *************************************/

uint8_t astrocde_state::video_register_r(offs_t offset)
{
	uint8_t result = 0xff;

	/* these are the core registers */
	switch (offset & 0xff)
	{
	case 0x08:  /* intercept feedback */
		result = m_funcgen_intercept;
		m_funcgen_intercept = 0;
		break;

	case 0x0e:  /* vertical feedback (from lightpen interrupt) */
		result = m_vertical_feedback;
		break;

	case 0x0f:  /* horizontal feedback (from lightpen interrupt) */
		result = m_horizontal_feedback;
		break;
	}

	return result;
}


void astrocde_state::video_register_w(offs_t offset, uint8_t data)
{
	/* these are the core registers */
	switch (offset & 0xff)
	{
	case 0x00:  /* color table is in registers 0-7 */
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		m_colors[offset & 7] = data;
		break;

	case 0x08:  /* mode register */
		m_video_mode = data & 1;
		break;

	case 0x09:  /* color split pixel */
		m_colorsplit = 2 * (data & 0x3f);
		m_bgdata = ((data & 0xc0) >> 6) * 0x55;
		break;

	case 0x0a:  /* vertical blank register */
		m_vblank = data;
		break;

	case 0x0b:  /* color block transfer */
		m_colors[(offset >> 8) & 7] = data;
		break;

	case 0x0c:  /* function generator */
		m_funcgen_control = data;
		m_funcgen_expand_count = 0;     /* reset flip-flop for expand mode on write to this register */
		m_funcgen_rotate_count = 0;     /* reset counter for rotate mode on write to this register */
		m_funcgen_shift_prev_data = 0;  /* reset shift buffer on write to this register */
		break;

	case 0x0d:  /* interrupt feedback */
		m_interrupt_vector = data;
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;

	case 0x0e:  /* interrupt enable and mode */
		m_interrupt_enabl = data;
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;

	case 0x0f:  /* interrupt line */
		m_interrupt_scanline = data;
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;

#ifdef UNUSED_OLD_CODE
	case 0x10:  /* master oscillator register */
	case 0x11:  /* tone A frequency register */
	case 0x12:  /* tone B frequency register */
	case 0x13:  /* tone C frequency register */
	case 0x14:  /* vibrato register */
	case 0x15:  /* tone C volume, noise modulation and MUX register */
	case 0x16:  /* tone A volume and tone B volume register */
	case 0x17:  /* noise volume register */
	case 0x18:  /* sound block transfer */
		if (m_video_config & AC_SOUND_PRESENT)
			m_astrocade_sound1->write(space, offset, data);
		break;
#endif
	}
}



/*************************************
 *
 *  Function generator
 *
 *************************************/

void astrocde_state::astrocade_funcgen_w(address_space &space, offs_t offset, uint8_t data)
{
	uint8_t prev_data;

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
		data =  (m_funcgen_expand_color[(data >> 3) & 1] << 6) |
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
			uint8_t shift = 2 * (~m_funcgen_rotate_count++ & 3);
			data =  (((m_funcgen_rotate_data[3] >> shift) & 3) << 6) |
					(((m_funcgen_rotate_data[2] >> shift) & 3) << 4) |
					(((m_funcgen_rotate_data[1] >> shift) & 3) << 2) |
					(((m_funcgen_rotate_data[0] >> shift) & 3) << 0);
		}
	}
	else
	{
		/* shift */
		uint8_t shift = 2 * (m_funcgen_control & 0x03);
		data = (data >> shift) | (prev_data << (8 - shift));
	}

	/* flopping */
	if (m_funcgen_control & 0x40)
		data = (data >> 6) | ((data >> 2) & 0x0c) | ((data << 2) & 0x30) | (data << 6);

	/* OR/XOR */
	if (m_funcgen_control & 0x30)
	{
		uint8_t olddata = space.read_byte(0x4000 + offset);

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


void astrocde_state::expand_register_w(uint8_t data)
{
	m_funcgen_expand_color[0] = data & 0x03;
	m_funcgen_expand_color[1] = (data >> 2) & 0x03;
}



/*************************************
 *
 *  Pattern board
 *
 *************************************/

inline void astrocde_state::increment_source(uint8_t curwidth, uint8_t *u13ff)
{
	/* if the flip-flop at U13 is high and mode.d2 is 1 we can increment */
	/* however, if mode.d3 is set and we're on the last byte of a row, the increment is suppressed */
	if (*u13ff && (m_pattern_mode & 0x04) != 0 && (curwidth != 0 || (m_pattern_mode & 0x08) == 0))
		m_pattern_source++;

	/* if mode.d1 is 1, toggle the flip-flop; otherwise leave it preset */
	if ((m_pattern_mode & 0x02) != 0)
		*u13ff ^= 1;
}


inline void astrocde_state::increment_dest(uint8_t curwidth)
{
	/* increment is suppressed for the last byte in a row */
	if (curwidth != 0)
	{
		/* if mode.d5 is 1, we increment */
		if ((m_pattern_mode & 0x20) != 0)
			m_pattern_dest++;

		/* otherwise, we decrement */
		else
			m_pattern_dest--;
	}
}


void astrocde_state::execute_blit()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/*
	    m_pattern_source = counter set U7/U16/U25/U34
	    m_pattern_dest = counter set U9/U18/U30/U39
	    m_pattern_mode = latch U21
	    m_pattern_skip = latch set U30/U39
	    m_pattern_width = latch set U32/U41
	    m_pattern_height = counter set U31/U40

	    m_pattern_mode bits:
	        d0 = direction (0 = read from src, write to dest, 1 = read from dest, write to src)
	        d1 = expand (0 = increment src each pixel, 1 = increment src every other pixel)
	        d2 = constant (0 = never increment src, 1 = normal src increment)
	        d3 = flush (0 = copy all data, 1 = copy a 0 in place of last byte of each row)
	        d4 = dest increment direction (0 = decrement dest on carry, 1 = increment dest on carry)
	        d5 = dest direction (0 = increment dest, 1 = decrement dest)
	*/

	uint8_t curwidth; /* = counter set U33/U42 */
	uint8_t u13ff;    /* = flip-flop at U13 */
	int cycles = 0;

/*  logerror("Blit: src=%04X mode=%02X dest=%04X skip=%02X width=%02X height=%02X\n",
            m_pattern_source, m_pattern_mode, m_pattern_dest, m_pattern_skip, m_pattern_width, m_pattern_height);*/

	/* flip-flop at U13 is cleared at the beginning */
	u13ff = 0;

	/* it is also forced preset if mode.d1 == 0 */
	if ((m_pattern_mode & 0x02) == 0)
		u13ff = 1;

	/* loop over height */
	do
	{
		uint16_t carry;

		/* loop over width */
		curwidth = m_pattern_width;
		do
		{
			uint16_t busaddr;
			uint8_t busdata;

			/* ----- read phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((m_pattern_mode & 0x01) == 0) ? m_pattern_source : m_pattern_dest;

			/* if mode.d3 is set, then the last byte fetched per row is forced to 0 */
			if (curwidth == 0 && (m_pattern_mode & 0x08) != 0)
				busdata = 0;
			else
				busdata = space.read_byte(busaddr);

			/* increment the appropriate address */
			if ((m_pattern_mode & 0x01) == 0)
				increment_source(curwidth, &u13ff);
			else
				increment_dest(curwidth);

			/* ----- write phase ----- */

			/* address is selected between source/dest based on mode.d0 */
			busaddr = ((m_pattern_mode & 0x01) != 0) ? m_pattern_source : m_pattern_dest;
			space.write_byte(busaddr, busdata);

			/* increment the appropriate address */
			if ((m_pattern_mode & 0x01) == 0)
				increment_dest(curwidth);
			else
				increment_source(curwidth, &u13ff);

			/* count 4 cycles (two read, two write) */
			cycles += 4;

		} while (curwidth-- != 0);

		/* at the end of each row, the skip value is added to the dest value */
		carry = ((m_pattern_dest & 0xff) + m_pattern_skip) & 0x100;
		m_pattern_dest = (m_pattern_dest & 0xff00) | ((m_pattern_dest + m_pattern_skip) & 0xff);

		/* carry behavior into the top byte is controlled by mode.d4 */
		if ((m_pattern_mode & 0x10) == 0)
			m_pattern_dest += carry;
		else
			m_pattern_dest -= carry ^ 0x100;

	} while (m_pattern_height-- != 0);

	/* count cycles we ran the bus */
	m_maincpu->adjust_icount(-cycles);
}


void astrocde_state::astrocade_pattern_board_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:     /* source offset low 8 bits */
			m_pattern_source = (m_pattern_source & 0xff00) | (data << 0);
			break;

		case 1:     /* source offset upper 8 bits */
			m_pattern_source = (m_pattern_source & 0x00ff) | (data << 8);
			break;

		case 2:     /* mode control; also clears low byte of dest */
			m_pattern_mode = data & 0x3f;
			m_pattern_dest &= 0xff00;
			break;

		case 3:     /* skip value */
			m_pattern_skip = data;
			break;

		case 4:     /* dest offset upper 8 bits; also adds skip to low 8 bits */
			m_pattern_dest = ((m_pattern_dest + m_pattern_skip) & 0xff) | (data << 8);
			break;

		case 5:     /* width of blit */
			m_pattern_width = data;
			break;

		case 6:     /* height of blit and initiator */
			m_pattern_height = data;
			execute_blit();
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

void astrocde_state::init_sparklestar()
{
	uint32_t shiftreg;
	int i;

	/* reset global sparkle state */
	m_sparkle[0] = m_sparkle[1] = m_sparkle[2] = m_sparkle[3] = 0;

	/* allocate memory for the sparkle/star array */
	m_sparklestar = std::make_unique<uint8_t[]>(RNG_PERIOD);

	/* generate the data for the sparkle/star array */
	for (shiftreg = i = 0; i < RNG_PERIOD; i++)
	{
		uint8_t newbit;

		/* clock the shift register */
		newbit = ((shiftreg >> 12) ^ ~shiftreg) & 1;
		shiftreg = (shiftreg >> 1) | (newbit << 16);

		/* extract the sparkle/star intensity here */
		/* this is controlled by the shift register at U17/U19/U18 */
		m_sparklestar[i] = (((shiftreg >> 4) & 1) << 3) |
							(((shiftreg >> 12) & 1) << 2) |
							(((shiftreg >> 16) & 1) << 1) |
							(((shiftreg >> 8) & 1) << 0);

		/* determine the star enable here */
		/* this is controlled by the shift register at U17/U12/U11 */
		if ((shiftreg & 0xff) == 0xfe)
			m_sparklestar[i] |= 0x10;
	}
}



/*************************************
 *
 *  16-color video board registers
 *
 *************************************/

void astrocde_state::profpac_page_select_w(uint8_t data)
{
	m_profpac_readpage = data & 3;
	m_profpac_writepage = (data >> 2) & 3;
	m_profpac_vispage = (data >> 4) & 3;
}


uint8_t astrocde_state::profpac_intercept_r()
{
	return m_profpac_intercept;
}


void astrocde_state::profpac_screenram_ctrl_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:     /* port 0xC0 - red component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 1:     /* port 0xC1 - green component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:     /* port 0xC2 - blue component */
			m_profpac_palette[data >> 4] = (m_profpac_palette[data >> 4] & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 3:     /* port 0xC3 - set 2bpp to 4bpp mapping and clear intercepts */
			m_profpac_colormap[(data >> 4) & 3] = data & 0x0f;
			m_profpac_intercept = 0x00;
			break;

		case 4:     /* port 0xC4 - which half to read on a memory access */
			m_profpac_vw = data & 0x0f; /* refresh write enable lines TBD */
			m_profpac_readshift = 2 * ((data >> 4) & 1);
			break;

		case 5:     /* port 0xC5 - write enable and write mode */
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

uint8_t astrocde_state::profpac_videoram_r(offs_t offset)
{
	uint16_t temp = m_profpac_videoram[m_profpac_readpage * 0x4000 + offset] >> m_profpac_readshift;
	return ((temp >> 6) & 0xc0) | ((temp >> 4) & 0x30) | ((temp >> 2) & 0x0c) | ((temp >> 0) & 0x03);
}


/* All this information comes from decoding the PLA at U39 on the screen ram board */
void astrocde_state::profpac_videoram_w(offs_t offset, uint8_t data)
{
	uint16_t oldbits = m_profpac_videoram[m_profpac_writepage * 0x4000 + offset];
	uint16_t newbits, result = 0;

	/* apply the 2->4 bit expansion first */
	newbits = (m_profpac_colormap[(data >> 6) & 3] << 12) |
				(m_profpac_colormap[(data >> 4) & 3] << 8) |
				(m_profpac_colormap[(data >> 2) & 3] << 4) |
				(m_profpac_colormap[(data >> 0) & 3] << 0);

	/* there are 4 write modes: overwrite, xor, overlay, or underlay */
	switch (m_profpac_writemode)
	{
		case 0:     /* normal write */
			result = newbits;
			break;

		case 1:     /* xor write */
			result = newbits ^ oldbits;
			break;

		case 2:     /* overlay write */
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
