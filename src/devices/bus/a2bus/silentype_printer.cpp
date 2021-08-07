// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    silentype_printer.cpp

    Implementation of the Apple Silentype Printer

**********************************************************************/

#include "emu.h"
#include "silentype.h"
#include "video.h"
#include "screen.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"
#include <bitset>
#include "silentype_printer.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// write bits @ c091
#define SILENTYPE_DATA                (0)
#define SILENTYPE_SHIFTCLOCKB         (1)  // SHIFT CLOCK (MOMENTARILY CLOCKED OUT WITH PHI-1)
#define SILENTYPE_STORECLOCK          (2)  // STORE CLOCK  (printer receives inverted signal, so falling edge does store)
#define SILENTYPE_DATAWRITEENABLE     (3)  // OUTPUT ENABLE
#define SILENTYPE_SHIFTCLOCKA         (4)  // LATCHED SHIFT CLOCK
#define SILENTYPE_ROMENABLE           (5)  // ENABLE ROM
#define SILENTYPE_SHIFTCLOCKDISABLE   (6)  // SHIFT CLOCK OUTPUT DISABLE

// shift register shifts when shift clock goes low (falling edge)
//
// storage register of 673 gets loaded on rising edge of store clock but this signal is inverted
//    so it will load on falling edge of bit 2 (d2 going from 1 to 0).
//
// (both store clock low (d4=0) and shift clock low (d2=0) will clear the storage register)
//   for example, writing 0 to c091 will reset the 673 parallel storage register
//
// Normal writing pattern to the shift register is 16 bytes of either 0x0e or 0x0f.
//
//    0x0e will shift in a 0
//    0x0f will shift in a 1
//
//       hstepperbits = BITS(m_parallel_reg, 3,  0);  // bits 0-3 are for the horizontal stepper
//       vstepperbits = BITS(m_parallel_reg, 7,  4);  // bits 4-7 are for the vertical stepper
//       headbits     = BITS(m_parallel_reg, 15, 9);  // bits 9-15 are for the print head
//
//    Bit 8 is for R/W* input to the shift register and is normally sent as a 0.
//
//    It was most likely used for diagnostic purposes in order to read the contents of the shift register,
//       since the serial data connection is bidirectional.  Once the parallel register gets loaded with bit 8 = 1,
//       the LS673 will be put into read mode.
//
//  Following the 16 bytes is a sequence of 4 bytes of 0x1c, 0x18, 0x1c, 0x0c.
//    The first 0x1c will raise d4.  d2 is already kept high from the previous 0x0e or previous 0x0f.
//    The 0x18 will drop d2: this will load the parallel storage register on the falling edge of d2.
//    We have to raise d2 by sending 0x1c before we drop d4 by sending 0x0c or the parallel register will be reset.
//    Dropping d4 has the side effect of shifting in another bit.
//  We don't care about this extra shift since we will always load 16 bits, and the extra bit will
//    get completely shifted out.


// read bits @ c094
#define SILENTYPE_STATUS              (7)  // margin switch status

// read bits @ c092
#define SERIAL_DATA_Q15               (7)  // shift register Q15 output
#define SERIAL_CLOCK_STATUS           (6)  // current shift clock output, read on A01


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SILENTYPE_PRINTER, silentype_printer_device, "silentype", "Apple Silentype Printer")


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

INPUT_PORTS_START(silentype_printer)
	PORT_START("CNF")
	PORT_CONFNAME(0x1, 0x01, "Print Darkness")
	PORT_CONFSETTING(0x0, "Normal (grey)")
	PORT_CONFSETTING(0x1, "Dark   (b/w)")
INPUT_PORTS_END


ioport_constructor silentype_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(silentype_printer);
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void silentype_printer_device::device_add_mconfig(machine_config &config)
{
   /* video hardware (simulates paper) */
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(PAPER_WIDTH, PAPER_SCREEN_HEIGHT);
	screen.set_visarea(0, PAPER_WIDTH - 1, 0, PAPER_SCREEN_HEIGHT - 1);
	screen.set_screen_update(FUNC(silentype_printer_device::screen_update_silentype));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

silentype_printer_device::silentype_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_screen(*this, "screen")
{
}

silentype_printer_device::silentype_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		silentype_printer_device(mconfig, SILENTYPE_PRINTER, tag, owner, clock)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void silentype_printer_device::device_start()
{
	m_bitmap.allocate(PAPER_WIDTH,PAPER_HEIGHT);  // try 660 pixels for 11 inch long
	m_bitmap.fill(0xffffff); // Start with a white piece of paper

	save_item(NAME(m_bitmap));
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

}

void silentype_printer_device::device_reset_after_children()
{
	m_ypos=10;
}

void silentype_printer_device::device_reset()
{
	update_pf_stepper(0);
	update_cr_stepper(0);
	update_printhead(0);
}


uint32_t silentype_printer_device::screen_update_silentype(screen_device &screen,
							 bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrolly = bitmap.height() - distfrombottom - (m_ypos * 7 / 4);

	int bottomlinetoclear = std::min(PAPER_HEIGHT-PAPER_SCREEN_HEIGHT,PAPER_HEIGHT);

	m_bitmap.plot_box(0, m_ypos * 7 / 4 + 10, PAPER_WIDTH, bottomlinetoclear, rgb_t::white());

	copyscrollbitmap(bitmap, m_bitmap, 0, nullptr, 1, &scrolly, cliprect);

	m_bitmap.plot_box(0, 0, 559, 2, 0xEEE8AA);  // draw a line on the very top of the bitmap

	bitmap.plot_box(m_xpos - 10, bitmap.height() - distfrombottom + 10,     20, 30, 0xBDB76B);
	bitmap.plot_box(m_xpos - 5,  bitmap.height() - distfrombottom + 10 + 5, 10, 20, 0xEEE8AA);

	return 0;
}

void silentype_printer_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
//  bitmap.plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, rgb_t::white());
	bitmap.plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, color);

}

void silentype_printer_device::write_snapshot_to_file(std::string directory, std::string name)
{
	emu_file file(machine().options().snapshot_directory() + std::string("/") + directory,
		  OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);

	auto const filerr = file.open(name);

	if (filerr == osd_file::error::NONE)
	{
		static const rgb_t png_palette[] = { rgb_t::white(), rgb_t::black() };

		// save the paper into a png
		util::png_write_bitmap(file, nullptr, m_bitmap, 2, png_palette);
	}
}

void silentype_printer_device::darken_pixel(double headtemp, u32& pixel)
{
	if (headtemp > 0.0)
	{
//      u8 intensity = headtemp * 15.0;
		u8 intensity = (
						ioport("CNF")->read() & 0x1 ?
							std::min(headtemp * 4, 1.0) :
							headtemp
						) * 15.0;
		u32 pixelval = pixel;
		u32 darkenval = intensity * 0x111111;

		pixelval &= 0xffffff;

		u32 rp = BITS(pixelval, 23, 16);
		u32 gp = BITS(pixelval, 15,  8);
		u32 bp = BITS(pixelval,  7,  0);

		u32 rd = BITS(darkenval, 23, 16);
		u32 gd = BITS(darkenval, 15,  8);
		u32 bd = BITS(darkenval,  7,  0);

		u32 r = (rp >= rd) ? rp - rd : 0;    // subtract the amount to darken
		u32 g = (gp >= gd) ? gp - gd : 0;
		u32 b = (bp >= bd) ? bp - bd : 0;

		pixelval = (r << 16) | (g << 8) | (b << 0);

		pixel = pixelval;
	}
}

//-------------------------------------------------
//    Adjust Printhead Temperature
//-------------------------------------------------

void silentype_printer_device::adjust_headtemp(u8 pin_status, double time_elapsed,  double& temp)
{
	temp += ( (pin_status) ?
				(time_elapsed / ((double) heattime  / 1.0E6)) :
			  - (time_elapsed / ((double) decaytime / 1.0E6)) );
	if (temp < 0.0) temp = 0;
	if (temp > 1.0) temp = 1.0;
}

//-------------------------------------------------
//    Update Printhead
//-------------------------------------------------

void silentype_printer_device::update_printhead(uint8_t headbits)
{

	double current_time = machine().time().as_double();
	double time_elapsed = current_time - last_update_time;
	last_update_time = current_time;

	LOG("PRINTHEAD %x\n",headbits);
	LOG("PRINTHEAD TIME ELAPSED = %f   %f usec     bitpattern=%s\n",
	  time_elapsed, time_elapsed*1e6, std::bitset<8>(headbits).to_string().c_str());

	for (int i=0;i<7;i++)
	{
		adjust_headtemp( BIT(lastheadbits,i), time_elapsed,  headtemp[i] );

		int xpixel = m_xpos + ((xdirection == 1) ? right_offset : left_offset);
		int ypixel = (m_ypos * 7 / 4) + (6 - i);

		if ((xpixel >= 0) && (xpixel <= (PAPER_WIDTH - 1)))
			darken_pixel( headtemp[i], m_bitmap.pix(ypixel, xpixel) );
	}
	lastheadbits = headbits;
}

//-------------------------------------------------
//    Update Paper Stepper
//-------------------------------------------------

void silentype_printer_device::update_pf_stepper(uint8_t vstepper)
{
	int halfstepflag;
	const int drivetable[4]    = {3, 9, 12, 6};
	const int halfsteptable[4] = {2, 4, 8, 1};

	if (vstepper != 0)
	{
		for(int i = 0; i < 4; i++)
		{
			if (drivetable[i] == vstepperlast) // scan table until we match index
			{
				if (drivetable[wrap(i + 1, 4)] == vstepper) // we are moving down the page
				{
					m_ypos += 1; // move down

					if (newpageflag == 1)
					{
						m_ypos = 10;  // lock to the top of page until we seek horizontally
					}
					if (m_ypos * 7 / 4 > m_bitmap.height() - 50)
						// if we are within 50 pixels of the bottom of the page we will
						// write the page to a file, then erase the top part of the page
						// so we can still see the last page printed.
					{

						// clear paper to bottom from current position
						bitmap_clear_band(m_bitmap, m_ypos * 7 / 4, PAPER_HEIGHT - 1, rgb_t::white());

						// save a snapshot with the slot and page as part of the filename
						write_snapshot_to_file(
									std::string("silentype"),
									std::string("silentype") +
//                                  std::string("_slot") + std::to_string(slotno()) +
								  std::string("_slot") +
//                                std::to_string(((a2bus_silentype_device *) (this->owner()))->get_slotno()) +
								  std::to_string( static_cast<a2bus_silentype_device *> (this->owner()) -> get_slotno() ) +
									"_page" + std::to_string(page_count++) + ".png");

						newpageflag = 1;
						// clear page down to visible area, starting from the top of page
						bitmap_clear_band(m_bitmap, 0, PAPER_HEIGHT - 1 - PAPER_SCREEN_HEIGHT, rgb_t::white());

						m_ypos = 10;
					}
					// clear page down to visible area
					bitmap_clear_band(m_bitmap, m_ypos * 7 / 4 + distfrombottom, PAPER_HEIGHT - 1 - PAPER_SCREEN_HEIGHT, rgb_t::white());

				}
				else if (drivetable[wrap(i - 1, 4)] == vstepper) // we are moving up the page
				{
					m_ypos -= 1;
					if (m_ypos < 0) m_ypos = 0;  // don't go backwards past top of page
				}
			}
		} // end for

		// ignore half steps
		halfstepflag=0;
		for (int i = 0; i < 4; i++) if (halfsteptable[i] == vstepper) halfstepflag = 1;

		if (!halfstepflag) vstepperlast = vstepper; // update the vstepperlast ignoring half steps
	}
}

//-------------------------------------------------
//    Update Carriage Stepper
//-------------------------------------------------

void silentype_printer_device::update_cr_stepper(uint8_t hstepper)
{
	int halfstepflag;
	const int drivetable[4]    = {3, 9, 12, 6};
	const int halfsteptable[4] = {2, 4, 8, 1};

	if (hstepper != 0)
	{
		newpageflag = 0;

		for(int i = 0; i < 4; i++)
		{
			if (drivetable[i] == hstepperlast) // scan table until we match index
			{
				if (drivetable[wrap(i + 1, 4)] == hstepper)
				{
					m_xpos += 1; xdirection = 1;
				}
				else if (drivetable[wrap(i - 1, 4)] == hstepper)
				{
					m_xpos -= 1; xdirection = -1;
					if (m_xpos < 0) m_xpos = 0;
				}
			}
		} // end for

		// ignore half steps
		halfstepflag = 0;
		for (int i = 0; i < 4; i++) if (halfsteptable[i] == hstepper) halfstepflag = 1;

		if (!halfstepflag) hstepperlast = hstepper; // update the hstepperlast ignoring half steps
	}
}


