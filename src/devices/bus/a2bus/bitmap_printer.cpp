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
#include <bitset>
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
	PORT_CONFNAME(0x1, 0x01, "Print Darkness")
	PORT_CONFSETTING(0x0, "Normal (grey)")
	PORT_CONFSETTING(0x1, "Dark   (b/w)")
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
	screen.set_size(PAPER_WIDTH, PAPER_SCREEN_HEIGHT);
	screen.set_visarea(0, PAPER_WIDTH - 1, 0, PAPER_SCREEN_HEIGHT - 1);
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
	
	m_bitmap->allocate(PAPER_WIDTH,PAPER_HEIGHT);  // try 660 pixels for 11 inch long
	m_bitmap->fill(0xffffff); // Start with a white piece of paper


/*	save_item(NAME(m_bitmap));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(right_offset));
	save_item(NAME(left_offset));
	save_item(NAME(heattime));
	save_item(NAME(decaytime));
	save_item(NAME(lastheadbits));
	save_item(NAME(headtemp));
	save_item(NAME(hstepperlast));
	save_item(NAME(vstepperlast));
	save_item(NAME(xdirection));
	save_item(NAME(newpageflag));
	save_item(NAME(page_count));
	save_item(NAME(last_update_time));
*/
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

	int scrolly = bitmap.height() - distfrombottom - m_ypos;

	copyscrollbitmap(bitmap, *m_bitmap, 0, nullptr, 1, &scrolly, cliprect);

	bitmap.plot_box(0, bitmap.height() - distfrombottom - m_ypos, PAPER_WIDTH, 2, 0xEEE8AA);  // draw a line on the very top of the bitmap

	bitmap.plot_box(m_xpos - 10, bitmap.height() - distfrombottom + 10,     20, 30, 0xBDB76B);
	bitmap.plot_box(m_xpos - 5,  bitmap.height() - distfrombottom + 10 + 5, 10, 20, 0xEEE8AA);

	return 0;
}


void bitmap_printer_device::bitmap_clear_band(int from_line, int to_line, u32 color)
{
//	printf("clear band (%d,%d)\n",from_line,to_line);
	m_bitmap->plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, color);
}

void bitmap_printer_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
//	printf("clear band (%d,%d)\n",from_line,to_line);
	bitmap.plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, color);
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
   return fixcolons(std::string(getrootdev()->shortname())+std::string(tag()));
//   return fixcolons(std::string(getrootdev()->shortname())+std::string(device().tag()));
}

std::string bitmap_printer_device::simplename()
{
        device_t * dev;
//        dev = &device();
		dev = this;
		device_t * rootdev = getrootdev();
 //       std::string s(dev->owner()->shortname());
 		std::string s;
 		int skipcount = 2;
 		
        while (dev){
        		if (skipcount-- <= 0)
                	s = std::string( (dev == rootdev ? dev->shortname() : dev->basetag() ) ) + 
                			std::string( s.length() ? "-" : "") + s;
//                s=std::string(dev->shortname())+std::string(" ")+s;
                dev=dev->owner();
        }
        return s;
}

device_t* bitmap_printer_device::getrootdev()
{
        device_t* dev;
        device_t* lastdev = NULL;
//        dev = &device();
		dev = this;
        while (dev){
                lastdev = dev;
                dev=dev->owner();
        }
        return lastdev;
}


