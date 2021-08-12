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

INPUT_PORTS_START(bitmap_printer)
	PORT_START("CNF")
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
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bitmap_printer_device::bitmap_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_screen(*this, "screen")
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
	time(&m_session_time);  // initialize session time
	initprintername();

	m_bitmap->allocate(m_paperwidth, m_paperheight);  // try 660 pixels for 11 inch long
	m_bitmap->fill(0xffffff);  // Start with a white piece of paper

	save_item(NAME(m_internal_bitmap));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
}

void bitmap_printer_device::device_reset_after_children()
{
	m_ypos=10;
}

void bitmap_printer_device::device_reset()
{
	printf("Bitmap Printer : Tagname=%s\n",tagname().c_str());
	printf("Bitmap Printer : Simplename=%s\n",simplename().c_str());
	printf("Bitmap Printer : name=%s\n",getprintername().c_str());
}


uint32_t bitmap_printer_device::screen_update_bitmap(screen_device &screen,
							 bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrolly = bitmap.height() - m_distfrombottom - m_ypos;

	copyscrollbitmap(bitmap, *m_bitmap, 0, nullptr, 1, &scrolly, cliprect);

	bitmap.plot_box(0, bitmap.height() - m_distfrombottom - m_ypos, m_paperwidth, 2, 0xEEE8AA);  // draw a line on the very top of the bitmap

	drawprinthead(bitmap, m_xpos, bitmap.height() - m_distfrombottom);

	return 0;
}


void bitmap_printer_device::bitmap_clear_band(int from_line, int to_line, u32 color)
{
	bitmap_clear_band(*m_bitmap, from_line, to_line, color);
}

void bitmap_printer_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
//  printf("clear band (%d,%d)\n",from_line,to_line);
	bitmap.plot_box(0, from_line, m_paperwidth, to_line - from_line + 1, color);
}

void bitmap_printer_device::write_snapshot_to_file(std::string directory, std::string name)
{
	printf("write snapshot\n");
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

std::string bitmap_printer_device::fixchar(std::string in, char from, char to)
{
	std::string final;
	for(std::string::const_iterator it = in.begin(); it != in.end(); ++it)
	{
		if((*it) != from)
		{
			final += *it;
		}
		else final += to;
	}
	return final;
}

std::string bitmap_printer_device::fixcolons(std::string in)
{
	return fixchar(in, ':', '-');
}

std::string bitmap_printer_device::sessiontime()
{
	struct tm *info;
	char buffer[80];
	info = localtime( &m_session_time );
	strftime(buffer,120,"%Y-%m-%d %H-%M-%S", info);
	return std::string(buffer);
}

std::string bitmap_printer_device::tagname()
{
   return fixcolons(std::string(getrootdev()->shortname()) + std::string(tag()));
}

std::string bitmap_printer_device::simplename()
{
	device_t * dev;
	dev = this;
	device_t * rootdev = getrootdev();
//  [[maybe_unused]]device_t * rootdev2 = & machine().root_device();
//  printf("rootdevs equal %x\n",rootdev == rootdev2);

	std::string s;
	int skipcount = 2;

	while (dev){
			if (skipcount-- <= 0)
				s = std::string( (dev == rootdev ? dev->shortname() : dev->basetag() ) ) +
						std::string( s.length() ? "-" : "") + s;
//              s=std::string(dev->shortname()) + std::string(" ") + s;
			dev=dev->owner();
	}
	return s;
}

device_t* bitmap_printer_device::getrootdev()
{
	device_t* dev;
	device_t* lastdev = NULL;

	dev = this;
	while (dev){
			lastdev = dev;
			dev=dev->owner();
	}
	return lastdev;
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
	bitmap.plot_box(x-sizex/2-bordx, y+offy-bordy, sizex+2*bordx, sizey+bordy*2, m_printheadbordercolor);
	bitmap.plot_box(x-sizex/2,       y+offy,       sizex,         sizey,         m_printheadcolor);
}

