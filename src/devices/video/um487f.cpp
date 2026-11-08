// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote
/***************************************************************************

    UMC UM487F HCGA (Hercules + CGA) single-chip video controller

    Combination of MGA (Hercules monochrome) and CGA display adapters in a
    100-pin QFP, built around an embedded UM6845R CRTC.  The chip needs only
    64 KiB of DRAM, a few TTLs, an optional character generator ROM and
    the dot clock crystals: 14.31818 MHz for CGA (OSC pin) and 16.257 MHz
    for MGA (MOSC pin).

    The CLOCK pin is fed with the CPU clock and only generates the enable
    signal of the CPU interface of the embedded 6845, it doesn't take part
    in the video timing.

    The printer port registers (3BC-3BE) belong to an external UM82C11, the
    UM487F only decodes its chip select.

    Status register bit 7 (vertical sync, active low) is documented only for
    MGA mode, but all the known CGA mode games wait on it for the vertical
    retrace.

    The display mode is selected at power on with the SWS (CGA) / SWR (MGA)
    inputs. When allowed by SW3, software can switch it setting the "enable
    change mode" bit of the mode control register first, and then writing
    the new mode to bit 6 of the configuration register.

    The character generator ROM is addressed as in the datasheet application
    circuit (a 2764 with A11 = CRA3 and A12 = JMPO), which matches the layout
    of the usual PC MDA/CGA font ROMs.

    TODO:
    - The status register video dot (MGA) is faked.
    - Printer port chip select (82C11CSB) and PSW input.
    - IORDY wait states for flicker free CPU access.
    - Border (overscan) color.
    - Composite and monochrome monitor outputs.
    - The datasheet gives CD0 as the first pixel, check the character
      generator bit order.
    - The datasheet only lists 3x4/3x5 for the CRTC, the mirrors at 3x0-3x7
      are a guess.
    - Verify the CGA text, 640x200 and MGA modes with real software.

***************************************************************************/

#include "emu.h"
#include "um487f.h"

#include "screen.h"

#include <algorithm>
#include <iterator>

#define LOG_REGS    (1U << 1)
#define LOG_MODE    (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_MODE)
#include "logmacro.h"

#define LOGREGS(...)    LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGMODE(...)    LOGMASKED(LOG_MODE, __VA_ARGS__)


DEFINE_DEVICE_TYPE(UM487F, um487f_device, "um487f", "UMC UM487F HCGA Controller")


um487f_device::um487f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UM487F, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_crtc(*this, "crtc")
	, m_chargen(*this, finder_base::DUMMY_TAG)
	, m_vram(*this, "vram", 0x10000, ENDIANNESS_LITTLE)
	, m_hsync_cb(*this)
	, m_vsync_cb(*this)
	, m_rgbi_cb(*this)
	, m_mga_clock(0)
	, m_strap_mga(false)
	, m_strap_change_enable(true)
	, m_mga(false)
	, m_mode(0)
	, m_color(0)
	, m_config(0)
	, m_lpen_latched(false)
	, m_video_dot(0)
	, m_framecnt(0)
	, m_vsync_on_pos(0)
{
}


void um487f_device::device_add_mconfig(machine_config &config)
{
	// embedded UM6845R
	MC6845(config, m_crtc, DERIVED_CLOCK(1, 16));
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(um487f_device::crtc_update_row));
	m_crtc->set_reconfigure_callback(FUNC(um487f_device::crtc_reconfigure));
	m_crtc->out_hsync_callback().set(FUNC(um487f_device::crtc_hsync_w));
	m_crtc->out_vsync_callback().set(FUNC(um487f_device::crtc_vsync_w));
}


void um487f_device::device_start()
{
	if (m_strap_mga && !m_mga_clock)
		logerror("MGA mode selected at power on, but no MGA clock\n");

	// default: IBM 5153 compatible monitor (dark yellow shown as brown)
	m_rgbi_cb.resolve();
	for (int i = 0; i < 16; i++)
	{
		if (!m_rgbi_cb.isnull())
		{
			set_pen_color(i, m_rgbi_cb(i));
			continue;
		}

		uint8_t const inten = BIT(i, 3) ? 0x55 : 0x00;
		uint8_t const r = (BIT(i, 2) ? 0xaa : 0x00) + inten;
		uint8_t g = (BIT(i, 1) ? 0xaa : 0x00) + inten;
		uint8_t const b = (BIT(i, 0) ? 0xaa : 0x00) + inten;

		if (i == 6)
			g = 0x55;

		set_pen_color(i, rgb_t(r, g, b));
	}

	// VIDOT/IOUT output (MGA)
	set_pen_color(PEN_MGA_BLACK, rgb_t(0x00, 0x00, 0x00));
	set_pen_color(PEN_MGA_NORMAL, rgb_t(0xaa, 0xaa, 0xaa));
	set_pen_color(PEN_MGA_BRIGHT, rgb_t(0xff, 0xff, 0xff));

	std::fill(std::begin(m_palette_lut_2bpp), std::end(m_palette_lut_2bpp), 0);

	save_item(NAME(m_mga));
	save_item(NAME(m_mode));
	save_item(NAME(m_color));
	save_item(NAME(m_config));
	save_item(NAME(m_lpen_latched));
	save_item(NAME(m_video_dot));
	save_item(NAME(m_framecnt));
}


void um487f_device::device_reset()
{
	m_mga = m_strap_mga;
	m_mode = 0;
	m_color = 0;
	m_config = m_mga ? 0 : CONFIG_CGA;
	m_lpen_latched = false;
	m_video_dot = 0;

	update_crtc_clock();
	update_palette_lut();
}


void um487f_device::device_post_load()
{
	update_crtc_clock();
	update_palette_lut();
}


/***************************************************************************
    CRTC interface
***************************************************************************/

MC6845_RECONFIGURE(um487f_device::crtc_reconfigure)
{
	screen().configure(width, height, visarea, frame_period);
	m_vsync_on_pos = vsync_on;
}


void um487f_device::crtc_hsync_w(int state)
{
	m_hsync_cb(state);
}


void um487f_device::crtc_vsync_w(int state)
{
	if (state)
	{
		// keep the screen beam in step with the CRTC, the vertical sync
		// pulse begins at the start of the line selected with R7
		if (m_vsync_on_pos < screen().height())
			screen().reset_origin(m_vsync_on_pos, 0);

		m_framecnt++;
	}

	m_vsync_cb(state);
}


void um487f_device::update_crtc_clock()
{
	if (m_mga)
	{
		// without the MOSC crystal the CRTC is left running from OSC, but nothing is displayed
		if (m_mga_clock)
		{
			bool const gfx = mga_graphics();

			m_crtc->set_unscaled_clock(m_mga_clock / (gfx ? 16 : 9));
			m_crtc->set_hpixels_per_column(gfx ? 16 : 9);
		}
	}
	else
	{
		// as on the IBM CGA, the high resolution clock is selected by mode bit 0,
		// while 640x200 graphics use the low clock with 16 pixels per character
		bool const hires_gfx = (m_mode & (MODE_GRAPHICS | MODE_640)) == (MODE_GRAPHICS | MODE_640);

		m_crtc->set_unscaled_clock(clock() / ((m_mode & MODE_HIRES) ? 8 : 16));
		m_crtc->set_hpixels_per_column(hires_gfx ? 16 : 8);
	}
}


void um487f_device::update_palette_lut()
{
	uint8_t const inten = BIT(m_color, 4) ? 0x08 : 0x00;

	// background color, not available in 640x200 mode
	m_palette_lut_2bpp[0] = (m_mode & MODE_640) ? 0 : (m_color & 0x0f);

	if ((m_mode & MODE_BW) || BIT(m_color, 5))
	{
		// cyan, magenta (red in black/white mode), white
		m_palette_lut_2bpp[1] = inten | 3;
		m_palette_lut_2bpp[2] = inten | ((m_mode & MODE_BW) ? 4 : 5);
		m_palette_lut_2bpp[3] = inten | 7;
	}
	else
	{
		// green, red, brown/yellow
		m_palette_lut_2bpp[1] = inten | 2;
		m_palette_lut_2bpp[2] = inten | 4;
		m_palette_lut_2bpp[3] = inten | 6;
	}
}


/***************************************************************************
    Rendering
***************************************************************************/

uint32_t um487f_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	if (m_mga && !m_mga_clock)
		return 0;

	m_crtc->screen_update(screen, bitmap, cliprect);

	return 0;
}


uint8_t um487f_device::chargen_r(uint8_t chr, uint8_t ra) const
{
	if (!m_chargen)
		return 0;

	// JMPO selects the font set, CRA3 the upper half of a 16 line font
	offs_t const addr = (m_mga ? 0x0000 : 0x1000) | (BIT(ra, 3) << 11) | (chr << 3) | (ra & 0x07);

	return (addr < m_chargen.length()) ? m_chargen[addr] : 0;
}


MC6845_UPDATE_ROW(um487f_device::crtc_update_row)
{
	if (!(m_mode & MODE_VIDEO) || (y > cliprect.bottom()) || (y < cliprect.top()))
		return;

	if (m_mga)
	{
		if (mga_graphics())
			mga_gfx_row(bitmap, ma, ra, y, x_count);
		else
			mga_text_row(bitmap, ma, ra, y, x_count, cursor_x);
	}
	else
	{
		if (!(m_mode & MODE_GRAPHICS))
			cga_text_row(bitmap, ma, ra, y, x_count, cursor_x);
		else if (m_mode & MODE_640)
			cga_gfx_1bpp_row(bitmap, ma, ra, y, x_count);
		else
			cga_gfx_2bpp_row(bitmap, ma, ra, y, x_count);
	}
}


void um487f_device::cga_text_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x)
{
	uint32_t *p = &bitmap.pix(y);
	int const width = bitmap.width();
	bool const blink = m_mode & MODE_BLINK;

	for (int i = 0; (i < x_count) && (((i + 1) * 8) <= width); i++)
	{
		offs_t const offset = ((ma + i) << 1) & 0x3fff;
		uint8_t const chr = m_vram[offset];
		uint8_t const attr = m_vram[offset + 1];
		uint8_t data = chargen_r(chr, ra);
		uint8_t const fg = attr & 0x0f;
		uint8_t bg = attr >> 4;

		if (blink)
		{
			bg &= 0x07;
			if (BIT(attr, 7) && BIT(m_framecnt, 4))
				data = 0x00;
		}

		if ((i == cursor_x) && BIT(m_framecnt, 3))
			data = 0xff;

		for (int bit = 7; bit >= 0; bit--)
			*p++ = pen_color(BIT(data, bit) ? fg : bg);
	}
}


void um487f_device::cga_gfx_2bpp_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	uint32_t *p = &bitmap.pix(y);
	int const width = bitmap.width();

	for (int i = 0; (i < x_count) && (((i + 1) * 8) <= width); i++)
	{
		offs_t const offset = (((ma + i) << 1) & 0x1fff) | ((ra & 1) << 13);

		for (int byte = 0; byte < 2; byte++)
		{
			uint8_t const data = m_vram[offset + byte];

			*p++ = pen_color(m_palette_lut_2bpp[BIT(data, 6, 2)]);
			*p++ = pen_color(m_palette_lut_2bpp[BIT(data, 4, 2)]);
			*p++ = pen_color(m_palette_lut_2bpp[BIT(data, 2, 2)]);
			*p++ = pen_color(m_palette_lut_2bpp[BIT(data, 0, 2)]);
		}
	}
}


void um487f_device::cga_gfx_1bpp_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	uint32_t *p = &bitmap.pix(y);
	int const width = bitmap.width();
	uint8_t const fg = m_color & 0x0f;

	for (int i = 0; (i < x_count) && (((i + 1) * 16) <= width); i++)
	{
		offs_t const offset = (((ma + i) << 1) & 0x1fff) | ((ra & 1) << 13);

		for (int byte = 0; byte < 2; byte++)
		{
			uint8_t const data = m_vram[offset + byte];

			for (int bit = 7; bit >= 0; bit--)
				*p++ = pen_color(BIT(data, bit) ? fg : 0);
		}
	}
}


void um487f_device::mga_text_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x)
{
	uint32_t *p = &bitmap.pix(y);
	int const width = bitmap.width();
	bool const blink = m_mode & MODE_BLINK;
	offs_t const page = mga_page1() ? 0x8000 : 0x0000;

	for (int i = 0; (i < x_count) && (((i + 1) * 9) <= width); i++)
	{
		offs_t const offset = page | (((ma + i) << 1) & 0x0fff);
		uint8_t const chr = m_vram[offset];
		uint8_t const attr = m_vram[offset + 1];
		uint8_t data = chargen_r(chr, ra);
		bool dup = (chr & 0xe0) == 0xc0; // line drawing characters extend into the 9th column
		uint8_t fg, bg;

		switch (attr)
		{
		case 0x00: case 0x08: case 0x80: case 0x88:
			// non-display
			fg = bg = PEN_MGA_BLACK;
			break;
		case 0x70: case 0x78:
			// reverse video
			fg = PEN_MGA_BLACK;
			bg = PEN_MGA_NORMAL;
			break;
		case 0xf0: case 0xf8:
			// reverse video, bright background unless blinking is enabled
			fg = PEN_MGA_BLACK;
			bg = blink ? PEN_MGA_NORMAL : PEN_MGA_BRIGHT;
			break;
		default:
			fg = BIT(attr, 3) ? PEN_MGA_BRIGHT : PEN_MGA_NORMAL;
			bg = PEN_MGA_BLACK;
			break;
		}

		// underline
		if ((ra == 12) && ((attr & 0x07) == 0x01))
		{
			data = 0xff;
			dup = true;
		}

		if (blink && BIT(attr, 7) && BIT(m_framecnt, 4))
			data = 0x00;

		if ((i == cursor_x) && BIT(m_framecnt, 3))
		{
			data = 0xff;
			dup = true;
		}

		for (int bit = 7; bit >= 0; bit--)
			*p++ = pen_color(BIT(data, bit) ? fg : bg);

		*p++ = pen_color((dup && BIT(data, 0)) ? fg : bg);
	}
}


void um487f_device::mga_gfx_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	uint32_t *p = &bitmap.pix(y);
	int const width = bitmap.width();
	offs_t const page = mga_page1() ? 0x8000 : 0x0000;

	for (int i = 0; (i < x_count) && (((i + 1) * 16) <= width); i++)
	{
		offs_t const offset = page | ((ra & 3) << 13) | (((ma + i) << 1) & 0x1fff);

		for (int byte = 0; byte < 2; byte++)
		{
			uint8_t const data = m_vram[offset + byte];

			for (int bit = 7; bit >= 0; bit--)
				*p++ = pen_color(BIT(data, bit) ? PEN_MGA_NORMAL : PEN_MGA_BLACK);
		}
	}
}


/***************************************************************************
    CPU interface
***************************************************************************/

uint8_t um487f_device::mem_r(offs_t offset)
{
	offset &= 0xffff;

	if (m_mga)
	{
		if (!BIT(offset, 15) || (m_config & CONFIG_MGA_PAGE1))
			return m_vram[offset];
	}
	else if (BIT(offset, 15))
	{
		return m_vram[offset & 0x3fff];
	}

	return 0xff;
}


void um487f_device::mem_w(offs_t offset, uint8_t data)
{
	offset &= 0xffff;

	if (m_mga)
	{
		if (!BIT(offset, 15) || (m_config & CONFIG_MGA_PAGE1))
			m_vram[offset] = data;
	}
	else if (BIT(offset, 15))
	{
		m_vram[offset & 0x3fff] = data;
	}
}


uint8_t um487f_device::status_r()
{
	uint8_t data = 0;

	if (m_mga)
	{
		if (m_crtc->hsync_r())
			data |= STATUS_MGA_HSYNC;

		// fake pixel stream
		uint8_t const dot = m_video_dot ^ STATUS_MGA_VIDEO;
		if (!machine().side_effects_disabled())
			m_video_dot = dot;
		data |= dot;
	}
	else
	{
		if (!m_crtc->de_r())
			data |= STATUS_CGA_NODISP;

		if (m_crtc->vsync_r())
			data |= STATUS_CGA_VSYNC;
	}

	if (m_lpen_latched)
		data |= STATUS_LPEN_TRIG;

	if (!m_crtc->vsync_r())
		data |= STATUS_NOVSYNC;

	return data;
}


void um487f_device::mode_w(uint8_t data)
{
	LOGREGS("%s: mode control %02x\n", machine().describe_context(), data);

	if (data != m_mode)
	{
		screen().update_partial(screen().vpos());

		m_mode = data;

		update_crtc_clock();
		update_palette_lut();
	}
}


void um487f_device::color_w(uint8_t data)
{
	LOGREGS("%s: color select %02x\n", machine().describe_context(), data);

	if (data != m_color)
	{
		screen().update_partial(screen().vpos());

		m_color = data;

		update_palette_lut();
	}
}


void um487f_device::config_w(uint8_t data)
{
	LOGREGS("%s: configuration %02x\n", machine().describe_context(), data);

	screen().update_partial(screen().vpos());

	bool const mga = !(data & CONFIG_CGA);
	if ((mga != m_mga) && m_strap_change_enable && (m_mode & MODE_CHANGE))
	{
		LOGMODE("%s: switching to %s mode\n", machine().describe_context(), mga ? "MGA" : "CGA");
		if (mga && !m_mga_clock)
			logerror("%s: switching to MGA mode without MGA clock\n", machine().describe_context());

		m_mga = mga;
	}

	m_config = data;

	update_crtc_clock();
}


uint8_t um487f_device::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	// 0x00-0x0f: 0x3b0-0x3bf (MGA), 0x20-0x2f: 0x3d0-0x3df (CGA)
	if ((offset & 0x30) == (m_mga ? 0x00 : 0x20))
	{
		switch (offset & 0x0f)
		{
		case 0x1: case 0x3: case 0x5: case 0x7:
			data = m_crtc->register_r();
			break;

		case 0xa:
			data = status_r();
			break;
		}
	}

	return data;
}


void um487f_device::io_w(offs_t offset, uint8_t data)
{
	if (offset == 0x0f)
	{
		config_w(data);
		return;
	}

	if ((offset & 0x30) != (m_mga ? 0x00 : 0x20))
		return;

	switch (offset & 0x0f)
	{
	case 0x0: case 0x2: case 0x4: case 0x6:
		m_crtc->address_w(data);
		break;

	case 0x1: case 0x3: case 0x5: case 0x7:
		m_crtc->register_w(data);
		break;

	case 0x8:
		mode_w(data);
		break;

	case 0x9:
		if (m_mga)
		{
			// light pen set (MGA)
			if (!m_lpen_latched)
				m_crtc->assert_light_pen_input();
			m_lpen_latched = true;
		}
		else
		{
			color_w(data);
		}
		break;

	case 0xb:
		// light pen reset
		m_lpen_latched = false;
		break;

	case 0xc:
		// light pen set (CGA)
		if (!m_mga)
		{
			if (!m_lpen_latched)
				m_crtc->assert_light_pen_input();
			m_lpen_latched = true;
		}
		break;
	}
}
