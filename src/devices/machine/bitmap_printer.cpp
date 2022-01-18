// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    bitmap_printer.cpp

    Implementation of Bitmap Printer

**********************************************************************/

#include "emu.h"
#include "video.h"
#include "screen.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"
#include "bitmap_printer.h"
#include "corestr.h"

/***************************************************************************
    DEVICE DECLARATION
***************************************************************************/

DEFINE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device, "bitmap_printer", "Bitmap Printer Device")

//**************************************************************************
//    INPUT PORTS
//**************************************************************************

#define PORT_ADJUSTER_16MASK(_default, _name)                   \
		configurer.field_alloc(IPT_ADJUSTER, (_default), 0xffff, (_name)); \
		configurer.field_set_min_max(0, 100);

INPUT_PORTS_START(bitmap_printer)
	PORT_START("DRAWMARKS")
	PORT_CONFNAME(0x3, 0x02, "Draw Inch Marks")
	PORT_CONFSETTING(0x0, "Off")
	PORT_CONFSETTING(0x1, "with marks")
	PORT_CONFSETTING(0x2, "with numbers")

	PORT_START("TOPMARGIN")
	PORT_ADJUSTER_16MASK(18, "Printer Top Margin")
	PORT_MINMAX(0,500)

	PORT_START("BOTTOMMARGIN")
	PORT_ADJUSTER_16MASK(18, "Printer Bottom Margin")
	PORT_MINMAX(0,500)

INPUT_PORTS_END


ioport_constructor bitmap_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bitmap_printer);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bitmap_printer_device::device_add_mconfig(machine_config &config)
{
	// video hardware (simulates paper)
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(m_paper_width, PAPER_SCREEN_HEIGHT);
	screen.set_visarea(0, m_paper_width - 1, 0, PAPER_SCREEN_HEIGHT - 1);
	screen.set_screen_update(FUNC(bitmap_printer_device::screen_update_bitmap));

	STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	STEPPER(config, m_cr_stepper, (uint8_t) 0xa);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bitmap_printer_device::bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_screen(*this, "screen"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),
	m_top_margin_ioport(*this, "TOPMARGIN"),
	m_bottom_margin_ioport(*this, "BOTTOMMARGIN"),
	m_draw_marks_ioport(*this, "DRAWMARKS"),
	m_cr_direction(1),
	m_pf_stepper_ratio0(1),
	m_pf_stepper_ratio1(1),
	m_cr_stepper_ratio0(1),
	m_cr_stepper_ratio1(1),
	m_xpos(0),
	m_ypos(0),
	m_printhead_color(0x00EE00),
	m_printhead_bordercolor(0xEE0000),
	m_printhead_bordersize(2),
	m_printhead_xsize(10),
	m_printhead_ysize(20),
	m_page_dirty(0),
	m_paper_width(0),
	m_paper_height(0),
	m_hdpi(0),
	m_vdpi(0),
	m_clear_pos(0),
	m_newpage_flag(0),
	m_led_state{0,1,1,1,1},
	m_num_leds(1)
{
}

bitmap_printer_device::bitmap_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		bitmap_printer_device(mconfig, BITMAP_PRINTER, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bitmap_printer_device::device_start()
{
	m_page_bitmap.allocate(m_paper_width, m_paper_height);
	m_page_bitmap.fill(0xffffff);  // Start with a white piece of paper

	save_item(NAME(m_page_bitmap));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_cr_direction));
	save_item(NAME(m_pf_stepper_ratio0));
	save_item(NAME(m_pf_stepper_ratio1));
	save_item(NAME(m_cr_stepper_ratio0));
	save_item(NAME(m_cr_stepper_ratio1));
	save_item(NAME(m_printhead_color));
	save_item(NAME(m_printhead_bordercolor));
	save_item(NAME(m_printhead_bordersize));
	save_item(NAME(m_printhead_xsize));
	save_item(NAME(m_printhead_ysize));
	save_item(NAME(m_page_dirty));
	save_item(NAME(m_paper_width));
	save_item(NAME(m_paper_height));
	save_item(NAME(m_hdpi));
	save_item(NAME(m_vdpi));
	save_item(NAME(m_clear_pos));
	save_item(NAME(m_newpage_flag));
}

void bitmap_printer_device::device_reset_after_children()
{
	m_ypos = get_top_margin();
}

void bitmap_printer_device::device_reset()
{
}

//-------------------------------------------------
//    SCREEN UPDATE FUNCTIONS
//-------------------------------------------------

int bitmap_printer_device::calc_scroll_y(bitmap_rgb32& bitmap)
{
	return bitmap.height() - m_distfrombottom - m_ypos;
}

uint32_t bitmap_printer_device::screen_update_bitmap(screen_device &screen,
							 bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static constexpr u32 top_edge_color = 0xEEE8AA;
	static constexpr u32 bottom_edge_color = 0xEE8844;
	static constexpr u32 coverup_color = 0xDDDDDD;

	int scrolly = calc_scroll_y(bitmap);

	copyscrollbitmap(bitmap, m_page_bitmap, 0, nullptr, 1, &scrolly, cliprect);

	// draw a line on the very top of the top edge of page
	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos, m_paper_width, 2, top_edge_color);
	// draw a line on the bottom edge of page
	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos + m_paper_height, m_paper_width, 2, bottom_edge_color);
	// cover up visible parts of current page at the bottom
	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos + m_paper_height + 2, m_paper_width, m_distfrombottom, coverup_color);

	draw_printhead(bitmap, std::max(m_xpos, 0) , bitmap.height() - m_distfrombottom);

	draw_inch_marks(bitmap);

	return 0;
}

//-------------------------------------------------
//    BITMAP CLEARING FUNCTIONS
//-------------------------------------------------

void bitmap_printer_device::clear_to_pos(int to_line, u32 color)
{
	int from_line = m_clear_pos;
	to_line = std::min(m_page_bitmap.height(), to_line);
	if (to_line >= from_line)
	{
		bitmap_clear_band(m_page_bitmap, from_line, to_line, color);
	}
	m_clear_pos = std::max(m_clear_pos, to_line + 1);
}

void bitmap_printer_device::bitmap_clear_band(int from_line, int to_line, u32 color)
{
	bitmap_clear_band(m_page_bitmap, from_line, to_line, color);
}

void bitmap_printer_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
	bitmap.plot_box(0, from_line, m_paper_width, to_line - from_line + 1, color);
}

//-------------------------------------------------
//    PRINTHEAD FUNCTIONS
//-------------------------------------------------

void bitmap_printer_device::set_printhead_color(int headcolor, int bordcolor)
{
	m_printhead_color = headcolor;
	m_printhead_bordercolor = bordcolor;
}

void bitmap_printer_device::set_printhead_size(int xsize, int ysize, int bordersize)
{
	m_printhead_xsize = xsize;
	m_printhead_ysize = ysize;
	m_printhead_bordersize = bordersize;
}

u32 bitmap_printer_device::dimcolor(u32 incolor, int factor)
{
	return  (((incolor & 0xff0000) >> 16) / factor << 16) |
			(((incolor & 0xff00) >> 8) / factor << 8) |
			(((incolor & 0xff) >> 0) / factor);
}

void bitmap_printer_device::draw_printhead(bitmap_rgb32 &bitmap, int x, int y)
{
	int bordx = m_printhead_bordersize;
	int bordy = m_printhead_bordersize;
	int offy = 9 + bordy;
	int sizex = m_printhead_xsize;
	int sizey = m_printhead_ysize;
	bitmap.plot_box(x - sizex / 2- bordx, y + offy - bordy, sizex + 2 * bordx, sizey + bordy * 2,
		m_led_state[0] ? m_printhead_bordercolor : dimcolor(m_printhead_bordercolor, 4));

	for (int i = 1; i <= m_num_leds; i++)
	bitmap.plot_box(x - sizex / 2, y + offy + ((i -1) * sizey / m_num_leds), sizex,
		((i+1) * sizey / m_num_leds) - (i * sizey / m_num_leds),
		m_led_state[i] ? m_printhead_color : dimcolor(m_printhead_color, 4));
}

//-------------------------------------------------
//    DRAW INCH MARKS AND NUMBERS
//-------------------------------------------------

void bitmap_printer_device::draw7seg(u8 data, bool is_digit, int x0, int y0, int width, int height, int thick, bitmap_rgb32 &bitmap, u32 color, u32 erasecolor)
{
	// pass nonzero erasecolor to erase blank segments
	const u8 pat[] = { 0x3f, 0x06,  0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
	u8 seg = is_digit ? pat[data & 0xf] : data;

	if (BIT(seg,0) || erasecolor) bitmap.plot_box(x0,       y0,                  width, thick,       BIT(seg,0) ? color : erasecolor);
	if (BIT(seg,1) || erasecolor) bitmap.plot_box(x0+width, y0+thick,            thick, height,      BIT(seg,1) ? color : erasecolor);
	if (BIT(seg,2) || erasecolor) bitmap.plot_box(x0+width, y0+2*thick+height,   thick, height,      BIT(seg,2) ? color : erasecolor);
	if (BIT(seg,3) || erasecolor) bitmap.plot_box(x0,       y0+2*thick+2*height, width, thick,       BIT(seg,3) ? color : erasecolor);
	if (BIT(seg,4) || erasecolor) bitmap.plot_box(x0-thick, y0+2*thick+height,   thick, height,      BIT(seg,4) ? color : erasecolor);
	if (BIT(seg,5) || erasecolor) bitmap.plot_box(x0-thick, y0+thick,            thick, height,      BIT(seg,5) ? color : erasecolor);
	if (BIT(seg,6) || erasecolor) bitmap.plot_box(x0,       y0+thick+height,     width, thick,       BIT(seg,6) ? color : erasecolor);
	if (BIT(seg,7) || erasecolor) bitmap.plot_box(x0+width+thick, y0+2*thick+2*height, thick, thick, BIT(seg,7) ? color : erasecolor); // draw dot
}

void bitmap_printer_device::draw_number(int number, int x, int y, bitmap_rgb32& bitmap)
{
	std::string s(std::to_string(number));

	int width = std::max(m_hdpi / 15, 1);  // 1/10 of inch
	int height = std::max(m_vdpi / 30, 1);
	int thick = std::max(m_vdpi / 72, 1);

	for (int i = s.length() - 1; i >= 0; i--)
		draw7seg( s.at(i) - 0x30, true,
					x + ( i - s.length()) * (3 * width) - (width), y + height * 3 / 2,
					width, height, thick, bitmap, 0x000000, 0);
}

void bitmap_printer_device::draw_inch_marks(bitmap_rgb32& bitmap)
{
	static constexpr u32 dark_grey_color = 0x202020;
	static constexpr u32 light_grey_color = 0xc0c0c0;

	int drawmarks = m_draw_marks_ioport->read();
	if (!drawmarks) return;

	for (int i = 0; i < m_vdpi * 11; i += m_vdpi / 4)
	{
		int adj_i = i + calc_scroll_y(bitmap) % m_paper_height;
		int barbase = m_vdpi / 6;
		int barwidth = ((i % m_vdpi) == 0) ? barbase * 2 : barbase;
		int barcolor = ((i % m_vdpi) == 0) ? dark_grey_color : light_grey_color;
		if (adj_i < bitmap.height())
		{
			bitmap.plot_box(bitmap.width() - 1 - barwidth, adj_i, barwidth, 1, barcolor);
			if ((i % m_vdpi) == 0)
			{
				if (drawmarks & 2)
					draw_number(i / m_vdpi, bitmap.width(), adj_i, bitmap);
			}
		}
	}
}

//-------------------------------------------------
//    DRAW PIXEL FUNCTIONS
//-------------------------------------------------

void bitmap_printer_device::draw_pixel(int x, int y, int pixelval)
{
	if (y >= m_page_bitmap.height()) y = m_page_bitmap.height() - 1;
	if (x >= m_page_bitmap.width()) x = m_page_bitmap.width() - 1;

	m_page_bitmap.pix(y, x) = pixelval;

	m_page_dirty = 1;
};

int bitmap_printer_device::get_pixel(int x, int y)
{
	if (y >= m_page_bitmap.height()) y = m_page_bitmap.height() - 1;
	if (x >= m_page_bitmap.width()) x = m_page_bitmap.width() - 1;

	return m_page_bitmap.pix(y, x);
};

unsigned int& bitmap_printer_device::pix(int y, int x)    // reversed y x
{
	if (y >= m_page_bitmap.height()) y = m_page_bitmap.height() - 1;
	if (x >= m_page_bitmap.width()) x = m_page_bitmap.width() - 1;

	return m_page_bitmap.pix(y,x);
};

//-------------------------------------------------
//    WRITE SNAPSHOT TO FILE
//-------------------------------------------------

void bitmap_printer_device::write_snapshot_to_file()
{
	machine().popmessage("writing printer snapshot");

	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = machine().video().open_next(file, "png");

	if (!filerr)
	{
		static const rgb_t png_palette[] = { rgb_t::white(), rgb_t::black() };

		// save the paper into a png
		util::png_write_bitmap(file, nullptr, m_page_bitmap, 2, png_palette);
	}
}

//-------------------------------------------------
//    STEPPER AND MARGIN FUNCTIONS
//-------------------------------------------------

int bitmap_printer_device::get_top_margin()    { return m_top_margin_ioport->read(); }
int bitmap_printer_device::get_bottom_margin() { return m_bottom_margin_ioport->read(); }

bool bitmap_printer_device::check_new_page()
{
	bool retval = false;

	// idea here is that you update the position, then check the page, this will do the saving of the page
	// if this routine returns true, means there's a new page and you should clear the yposition
	if (m_newpage_flag == 1)
	{
		// if you change m_ypos you have to change the stepper abs position too
		m_ypos = get_top_margin();  // lock to the top of page until we seek horizontally
		m_pf_stepper->set_absolute_position(get_top_margin() / m_pf_stepper_ratio0 * m_pf_stepper_ratio1);
	}
	if (m_ypos > m_page_bitmap.height() - 1 - get_bottom_margin())
			// If we are at the bottom of the page we will
			// write the page to a file, then erase the top part of the page
			// so we can still see the last page printed.
		{
			// clear paper to bottom from current position
			clear_to_pos(m_paper_height - 1, rgb_t::white());

			// save a snapshot
			write_snapshot_to_file();

			m_newpage_flag = 1;

			// clear page down to visible area, starting from the top of page
			m_clear_pos = 0;
			clear_to_pos(m_paper_height - 1 - PAPER_SCREEN_HEIGHT),

			m_ypos = get_top_margin();  // lock to the top of page until we seek horizontally
			m_pf_stepper->set_absolute_position(get_top_margin() / m_pf_stepper_ratio0 * m_pf_stepper_ratio1);
			retval = true;
		}
	else { clear_to_pos ( m_ypos + m_distfrombottom); }
	return retval;
}

int bitmap_printer_device::update_stepper_delta(stepper_device * stepper, uint8_t pattern)
{
	int lastpos = stepper->get_absolute_position();
	stepper->update(pattern);
	int delta = stepper->get_absolute_position() - lastpos;
	return delta;
}

// When sending patterns to the update_cr_stepper and update_pf_stepper
// functions, the stepper device uses a "standard drive table"
// so you have to match that drive table by using a bitswap function.
// If the stepper drive is in the opposite direction, just reverse the
// bits in the bitswap.

void bitmap_printer_device::update_cr_stepper(int pattern)
{
	int delta = update_stepper_delta(m_cr_stepper, pattern);

	if (delta != 0)
	{
		m_newpage_flag = 0;

		if      (delta > 0) {m_cr_direction = 1;}
		else if (delta < 0) {m_cr_direction = -1;}
	}
	m_xpos = m_cr_stepper->get_absolute_position() * m_cr_stepper_ratio0 / m_cr_stepper_ratio1;
}

void bitmap_printer_device::update_pf_stepper(int pattern)
{
	update_stepper_delta(m_pf_stepper, pattern);
	m_ypos = m_pf_stepper->get_absolute_position() * m_pf_stepper_ratio0 / m_pf_stepper_ratio1;
	check_new_page();
}

