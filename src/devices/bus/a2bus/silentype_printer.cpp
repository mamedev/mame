// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    silentype_printer.cpp

    Implementation of the Apple Silentype Printer

    (Implements physical printhead and stepper motors)

**********************************************************************/

#include "emu.h"
#include "silentype.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"
#include <bitset>

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
//       hstepperbits = BITS(m_parallel_reg, 0, 4);  // bits 0-3 are for the horizontal stepper
//       vstepperbits = BITS(m_parallel_reg, 4, 4);  // bits 4-7 are for the vertical stepper
//       headbits     = BITS(m_parallel_reg, 9, 7);  // bits 9-15 are for the print head
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
	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 60, 60);
	m_bitmap_printer->set_pf_stepper_ratio(7,8);  // 7 / 4 / 2 = 7 / 8
	m_bitmap_printer->set_cr_stepper_ratio(1,2);  // half steps = 1 / 2
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

silentype_printer_device::silentype_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_bitmap_printer(*this, "bitmap_printer")
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
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(right_offset));
	save_item(NAME(left_offset));
	save_item(NAME(heattime));
	save_item(NAME(decaytime));
	save_item(NAME(lastheadbits));
	save_item(NAME(headtemp));
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


void silentype_printer_device::darken_pixel(double headtemp, unsigned int& pixel)
{
	if (headtemp > 0.0)
	{
		u8 intensity = (
						ioport("CNF")->read() & 0x1 ?
							std::min(headtemp * 4, 1.0) :
							headtemp
						) * 15.0;
		u32 pixelval = pixel;
		u32 darkenval = intensity * 0x111111;

		pixelval &= 0xffffff;

		u32 rp = BIT(pixelval, 16, 8);
		u32 gp = BIT(pixelval, 8, 8);
		u32 bp = BIT(pixelval, 0, 8);

		u32 rd = BIT(darkenval, 16, 8);
		u32 gd = BIT(darkenval, 8, 8);
		u32 bd = BIT(darkenval, 0, 8);

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
	int xdirection = m_bitmap_printer->m_cr_direction;

	LOG("PRINTHEAD %x\n",headbits);
	LOG("PRINTHEAD TIME ELAPSED = %f   %f usec     bitpattern=%s\n",
	  time_elapsed, time_elapsed*1e6, std::bitset<8>(headbits).to_string().c_str());

	for (int i=0;i<7;i++)
	{
		adjust_headtemp( BIT(lastheadbits,i), time_elapsed,  headtemp[i] );

		int xpixel = (m_bitmap_printer->m_xpos) + ((xdirection == 1) ? right_offset : left_offset);  // offset to correct alignment when changing direction
		int ypixel = (m_bitmap_printer->m_ypos) + (6 - i);


		if ((xpixel >= 0) && (xpixel <= (PAPER_WIDTH - 1)))
		{
			darken_pixel( headtemp[i], m_bitmap_printer->pix(ypixel, xpixel) );
		}
	}
	lastheadbits = headbits;
}

//-------------------------------------------------
//    Update Paper Feed Stepper
//-------------------------------------------------

void silentype_printer_device::update_pf_stepper(uint8_t pattern)
{
	m_bitmap_printer->update_pf_stepper(bitswap<4>(pattern, 3, 1, 2, 0));
}

//-------------------------------------------------
//    Update Carriage Stepper
//-------------------------------------------------

void silentype_printer_device::update_cr_stepper(uint8_t pattern)
{
	m_bitmap_printer->update_cr_stepper(bitswap<4>(pattern, 3, 1, 2, 0));
}

