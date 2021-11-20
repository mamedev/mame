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

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BITMAP_PRINTER, bitmap_printer_device, "bitmap_printer", "Bitmap Printer Device")


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

#define PORT_ADJUSTER_16MASK(_default, _name)					\
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

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bitmap_printer_device::device_add_mconfig(machine_config &config)
{
	// video hardware (simulates paper)
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(m_paperwidth, PAPER_SCREEN_HEIGHT);
	screen.set_visarea(0, m_paperwidth - 1, 0, PAPER_SCREEN_HEIGHT - 1);
	screen.set_screen_update(FUNC(bitmap_printer_device::screen_update_bitmap));

	SESSION_TIME(config, m_session_time, 3);  // skip 3 levels so we don't include the printer tag, bitmap_printer and session_time

	STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	STEPPER(config, m_cr_stepper, (uint8_t) 0xa);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bitmap_printer_device::bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_screen(*this, "screen"),
		m_session_time(*this, "session_time"),
		m_pf_stepper(*this, "pf_stepper"),
		m_cr_stepper(*this, "cr_stepper")
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
	m_bitmap->allocate(m_paperwidth, m_paperheight);
	m_bitmap->fill(0xffffff);  // Start with a white piece of paper

	save_item(NAME(m_internal_bitmap));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
}

void bitmap_printer_device::device_reset_after_children()
{
	m_ypos = 10;
}

void bitmap_printer_device::device_reset()
{
//  printf("Bitmap Printer : Tagname=%s\n",tagname().c_str());
//  printf("Bitmap Printer : Simplename=%s\n",simplename().c_str());
//  printf("Bitmap Printer : name=%s\n",getprintername().c_str());
}


uint32_t bitmap_printer_device::screen_update_bitmap(screen_device &screen,
							 bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrolly = calc_scroll_y(bitmap);

	copyscrollbitmap(bitmap, *m_bitmap, 0, nullptr, 1, &scrolly, cliprect);

	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos, m_paperwidth, 2, 0xEEE8AA);  // draw a line on the very top of the top edge of page
	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos + m_paperheight, m_paperwidth, 2, 0xEE8844);  // draw a line on the bottom edge of page
	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos + m_paperheight + 2, m_paperwidth, m_distfrombottom, 0xDDDDDD);  // cover up visible parts of current page at the bottom

	drawprinthead(bitmap, std::max(m_xpos, 0) , bitmap.height() - m_distfrombottom);

	draw_inch_marks(bitmap);

	return 0;
}



void bitmap_printer_device::clear_to_pos(int to_line, u32 color)
{
//	printf("clear to pos: %d   current:%d\n",to_line,clear_pos);
	int from_line = clear_pos;
	to_line = std::min(m_bitmap->height(), to_line);
	if (to_line >= from_line)
	{
		bitmap_clear_band(*m_bitmap, from_line, to_line, color);
	}
	clear_pos = std::max(clear_pos, to_line + 1);
//	printf("new clear pos: %d \n",clear_pos);
}




void bitmap_printer_device::bitmap_clear_band(int from_line, int to_line, u32 color)
{
	bitmap_clear_band(*m_bitmap, from_line, to_line, color);
}

void bitmap_printer_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
//	printf("clear band (%d,%d)\n",from_line,to_line);
	// plot_box( x, y, width, height, color)
	bitmap.plot_box(0, from_line, m_paperwidth, to_line - from_line + 1, color);
}

void bitmap_printer_device::write_snapshot_to_file(std::string directory, std::string name)
{
	machine().popmessage("snapshot written to " + directory + "/" + name);
	emu_file file(machine().options().snapshot_directory() + std::string("/") + directory,
		  OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	auto const filerr = file.open(name);

	if (filerr == osd_file::error::NONE)
	{
		static const rgb_t png_palette[] = { rgb_t::white(), rgb_t::black() };

		// save the paper into a png
		util::png_write_bitmap(file, nullptr, *m_bitmap, 2, png_palette);
	}
}

void bitmap_printer_device::setprintheadcolor(int headcolor, int bordcolor)
{
	m_printheadcolor = headcolor;
	m_printheadbordercolor = bordcolor;
}

void bitmap_printer_device::setprintheadsize(int xsize, int ysize, int bordersize)
{
	m_printheadxsize = xsize;
	m_printheadysize = ysize;
	m_printheadbordersize = bordersize;
}

void bitmap_printer_device::drawprinthead(bitmap_rgb32 &bitmap, int x, int y)
{
	int bordx = m_printheadbordersize;
	int bordy = m_printheadbordersize;
	int offy = 9 + bordy;
	int sizex = m_printheadxsize;
	int sizey = m_printheadysize;
	bitmap.plot_box(x - sizex / 2- bordx, y + offy - bordy, sizex + 2 * bordx, sizey + bordy * 2, m_printheadbordercolor);
	bitmap.plot_box(x - sizex / 2,        y + offy,         sizex,             sizey,             m_printheadcolor);
}

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

	int width = m_hdpi / 15;  // 1/10 of inch
	int height = m_vdpi / 30;
	int thick = std::min(m_vdpi / 72, 1);

	for (int i = s.length()-1; i>=0; i--)
		draw7seg( s.at(i) - 0x30, true,
					x + ( i - s.length()) * (3 * width) - (width), y + height * 3 / 2,
					width, height, thick, bitmap, 0x000000, 0);
}

void bitmap_printer_device::draw_inch_marks(bitmap_rgb32& bitmap)
{
	int drawmarks = ioport("DRAWMARKS")->read();
	if (!drawmarks) return;

	for (int i = 0; i < m_vdpi * 11; i += m_vdpi / 4)
	{
		int adj_i = i + calc_scroll_y(bitmap) % m_paperheight;  // modding the bitmap height, not the paper height
		int barbase = m_vdpi / 6;
		int barwidth = ((i % m_vdpi) == 0) ? barbase * 2 : barbase;
		int barcolor = ((i % m_vdpi) == 0) ? 0x202020 : 0xc0c0c0;
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

void bitmap_printer_device::drawpixel(int x, int y, int pixelval)
{
	if (y >= m_bitmap->height()) y = m_bitmap->height() - 1;
	if (x >= m_bitmap->width()) x = m_bitmap->width() - 1;

	m_bitmap->pix(y, x) = pixelval;

	m_pagedirty = 1;
};

int bitmap_printer_device::getpixel(int x, int y)
{
	if (y >= m_bitmap->height()) y = m_bitmap->height() - 1;
	if (x >= m_bitmap->width()) x = m_bitmap->width() - 1;

	return m_bitmap->pix(y, x);
};

unsigned int& bitmap_printer_device::pix(int y, int x)    // reversed y x
{
	if (y >= m_bitmap->height()) y = m_bitmap->height() - 1;
	if (x >= m_bitmap->width()) x = m_bitmap->width() - 1;

	return m_bitmap->pix(y,x);
};

int bitmap_printer_device::calc_scroll_y(bitmap_rgb32& bitmap)
{
	return bitmap.height() - m_distfrombottom - m_ypos;
}






//-------------------------------------------------
//    STEPPER RELATED FUNCTIONS
//-------------------------------------------------

int bitmap_printer_device::get_top_margin()    { return ioport("TOPMARGIN")->read(); }
int bitmap_printer_device::get_bottom_margin() { return ioport("BOTTOMMARGIN")->read(); }

bool bitmap_printer_device::check_new_page()
{
	bool retval = false;
	
	// idea here is that you update the position, then check the page, this will do the saving of the page
	// if this routine returns true, means there's a new page and you should clear the yposition
	if (newpageflag == 1)
	{
		// if you change m_ypos you have to change the stepper abs position too
		m_ypos = get_top_margin();  // lock to the top of page until we seek horizontally
		m_pf_stepper->set_absolute_position(get_top_margin() / m_pf_stepper_ratio0 * m_pf_stepper_ratio1);
	}	
	if (m_ypos > get_bitmap().height() - 1 - get_bottom_margin())  
			// If we are at the bottom of the page we will
			// write the page to a file, then erase the top part of the page
			// so we can still see the last page printed.
		{
			// clear paper to bottom from current position
			clear_to_pos(m_paperheight - 1, rgb_t::white());
			
			// save a snapshot with the slot and page as part of the filename
			write_snapshot_to_file(
						owner()->basetag(),
						owner()->basetag() + std::string("_") +
						get_session_time_device()->getprintername() +
						"_page_" +
						padzeroes(std::to_string(get_session_time_device()->page_num++),3) +
						".png");

			newpageflag = 1;

			// clear page down to visible area, starting from the top of page
			clear_pos = 0;
			clear_to_pos(m_paperheight - 1 - PAPER_SCREEN_HEIGHT), 

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

void bitmap_printer_device::update_cr_stepper(uint8_t pattern) 
{

	int delta = update_stepper_delta(m_cr_stepper, pattern);

	if (delta != 0)
	{
		newpageflag = 0;

		if (delta > 0)
		{
			m_cr_direction = 1;
		}
		else if (delta < 0)
		{
			m_cr_direction = -1;
		}
	}
	m_xpos = m_cr_stepper->get_absolute_position() * m_cr_stepper_ratio0 / m_cr_stepper_ratio1;
}

void bitmap_printer_device::update_pf_stepper(int pattern) 
{	[[maybe_unused]] int delta = update_stepper_delta(m_pf_stepper, pattern);

	m_ypos = m_pf_stepper->get_absolute_position() * m_pf_stepper_ratio0 / m_pf_stepper_ratio1;
	check_new_page();
}

