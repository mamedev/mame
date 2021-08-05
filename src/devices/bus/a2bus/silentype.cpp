// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    silentype.cpp

    Implementation of the Apple Silentype Printer

**********************************************************************

Useful Resources:

OpenEmulator's Apple Silentype driver by Marc S. Ressl

Apple II Documentation Project:
Apple II Documentation Project/Interface Cards/Serial/Apple Synch Printer Interface Card (Silentype)/

Apple II Documentation Project/Interface Cards/Serial/Apple Synch Printer Interface Card (Silentype)/Schematics/
Apple Silentype Schematics:
Apple Sync Printer Card - Schematics 050-0024-00.pdf
Apple Sync Printer Card - Schematics 050-0024-01.pdf

Apple II Documentation Project/Peripherals/Printers/Apple Silentype/Photos/
Apple Silentype - Info 1.jpg  (specs: 60 dpi  6 pixel wide chars * 80 chars = 480 dots across = 8 inches)
                              (actually can do 83 chars across for 498 dots across)
Apple Silentype - Info 2.jpg  (Full Technical Specs: horiz resolution = 60 dots/inch  vertical resolution = 60 dots/inch)

Apple II Documentation Project/Peripherals/Printers/Apple Silentype/Manuals/
Apple Silentype Operation and Reference Manual.pdf

Apple II Documentation Project/Peripherals/Printers/Apple Silentype/Schematics/
Apple Deserializer Driver Printer - Schematics 050-0023-01.pdf
(shows the 74LS673 16 bit shift register and how the outputs are connected
to the print head, paper drive motor and the head drive motor.)

Apple-Orchard-v1n3-1980-1-Winter.pdf
Inside the Silentype Firmware by J.D. Eisenberg and A.J. Hertzfeld
(The Apple Orchard Winter 1980, p.43)
(This article shows you how to print bit graphics patterns.  It has warnings about
using the Silentype and the Disk II simultaneously as it will
damage the Apple's power supply.)

1982_02_BYTE_07-02_Winter_Computing.pdf
Double-Width Silentype Graphics for Your Apple by Charles H Putney (Byte Feb 1982, p413)

Nibble 1981_v2n6.pdf
Silentype Double Hi-Res Printing by Jenny Schmidt
(Nibble Magazine, V2N6 1981 p.121)

Apple Service Level I Technical Procedures #072 0062 Vol II
https://archive.org/details/AppleServiceLevelITechnicalProcedures0720062VolIIJan1986Ed/page/n29/mode/2up
covers using the Apple II Product Diagnostics Disk to perform diagnostics on the Silentype Printer and Interface.

Diagnostics disk named "Apple II+ Products diagnostic 652-0334.dsk"
Diagnostics disk named "Apple II Peripherals diskette 077-0217-A.dsk"

https://www.folklore.org/StoryView.py?project=Macintosh&story=What_Hath_Woz_Wrought
Story about Silentype development by Andy Hertzfeld from September 1979


https://www.folklore.org/StoryView.py?project=Macintosh&story=Apple_II_Mouse_Card.txt
Mentions using the 6522 chip in Apple III to interface to the Silentype thermal printer.
======================================================================


    C0nX: C0n1 is write address, for slot 1 = C091, offset=1, bit 5 = rom enable
          C0n4 is read register, for slot 1 = C094, offset=4, bit 7 = left side head limit switch


    Cn00-CnFF: ROM (First 256 bytes of ROM mirrored here)
    C800-CBFF: ROM 2048 byte ROM
    CF00-CFFF: RAM (256 bytes) switched in/out by bit 5 of data writes to C081

    The ROM in the overlapping RAM/ROM area from CF00-CFFF is only accessed for font table data,
    so the code writes $2C to the write register, setting bit 5 and enabling the rom.  Once it reads the
    font data, it immediately writes a $0C to the write register, disabling the rom again.
    This is the only place in the code that the font data is accessed.

00CC37  1  A9 2C                LDA #$2C
00CC39  1  20 A3 CA             JSR sendStateBit

00CAA3  1               sendStateBit:
00CAA3  1  AE 00 CF             LDX varIOBase
00CAA6  1  9D 81 C0             STA ioBase1,X    ; Write 0x2C, enables rom, bit 5 = 1
00CAA9  1  60                   RTS

00CC3C  1  B1 2A                LDA (textBaseL),Y  ; load our font data
00CC3E  1  48                   PHA
00CC3F  1  A9 0C                LDA #$0C
00CC41  1  9D 81 C0             STA ioBase1,X   ; Write 0x0c, disables rom, bit 5 = 0
00CC44  1  68                   PLA


*********************************************************************/

#include "emu.h"
#include "silentype.h"
#include "video.h"
#include "screen.h"
#include "emuopts.h"
#include "fileio.h"
#include "png.h"
//#include <bitset>

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

DEFINE_DEVICE_TYPE(A2BUS_SILENTYPE, a2bus_silentype_device, "a2silentype", "Apple Silentype Printer")

#define SILENTYPE_ROM_REGION  "silentype_rom"

ROM_START( silentype )
	ROM_REGION(0x800, SILENTYPE_ROM_REGION, 0)
	ROM_LOAD( "341-0039-00.bin", 0x000000, 0x000800, CRC(bfdcf54d) SHA1(5a133c11b379c5866bcf7fcef902ed2bad415f57))
ROM_END



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_silentype_device::device_add_mconfig(machine_config &config)
{
   // video hardware (simulates paper)
/*
    screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
    screen.set_refresh_hz(60);
    screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
    screen.set_size(PAPER_WIDTH, PAPER_SCREEN_HEIGHT);
    screen.set_visarea(0, PAPER_WIDTH-1, 0, PAPER_SCREEN_HEIGHT-1);
    screen.set_screen_update(FUNC(a2bus_silentype_device::screen_update_silentype));
*/

[[maybe_unused]]    silentype_printer_device &printer(SILENTYPE_PRINTER(config, m_silentype_printer, 0));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_silentype_device::device_rom_region() const
{
	return ROM_NAME( silentype );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_silentype_device::a2bus_silentype_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_rom(nullptr),
	//  m_screen(*this, "screen"), // aagh missing comma makes compile freakout
		m_silentype_printer(*this, "silentype_printer")
{
}

a2bus_silentype_device::a2bus_silentype_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_silentype_device(mconfig, A2BUS_SILENTYPE, tag, owner, clock)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_silentype_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(SILENTYPE_ROM_REGION).c_str())->base();

	memset(m_ram, 0, 256);

//  m_bitmap.allocate(PAPER_WIDTH,PAPER_HEIGHT);  // try 660 pixels for 11 inch long
//  m_bitmap.fill(0xffffff); // Start with a white piece of paper

//  save_item(NAME(m_bitmap));
	save_item(NAME(m_ram));
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

void a2bus_silentype_device::device_reset_after_children()
{
	m_ypos=10;
}

void a2bus_silentype_device::device_reset()
{
/*
    update_pf_stepper(0);
    update_cr_stepper(0);
    update_printhead(0);
*/
}

/*

uint32_t a2bus_silentype_device::screen_update_silentype(screen_device &screen,
                             bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    const int distfrombottom = 50;
    int scrolly = bitmap.height() - distfrombottom - (m_ypos * 7 / 4);

    int bottomlinetoclear = std::min(PAPER_HEIGHT-PAPER_SCREEN_HEIGHT,PAPER_HEIGHT);

    m_bitmap.plot_box(0, m_ypos * 7 / 4 + 10, PAPER_WIDTH, bottomlinetoclear, rgb_t::white());

    copyscrollbitmap(bitmap, m_bitmap, 0, nullptr, 1, &scrolly, cliprect);

    m_bitmap.plot_box(0, 0, 559, 2, 0xEEE8AA);  // draw a line on the very top of the bitmap

    bitmap.plot_box(m_xpos - 10, bitmap.height() - distfrombottom + 10,     20, 30, 0xBDB76B);
    bitmap.plot_box(m_xpos - 5,  bitmap.height() - distfrombottom + 10 + 5, 10, 20, 0xEEE8AA);

    return 0;
}


*/

/*-------------------------------------------------
  read_c0nx - called for reads from this card's c0nx space
  -------------------------------------------------*/

uint8_t a2bus_silentype_device::read_c0nx(uint8_t offset)
{
	if (offset == 4)
	{
		return (m_xpos <= 0) << SILENTYPE_STATUS;
	}
	else
		return 0x00;
}


/*

void a2bus_silentype_device::bitmap_clear_band(bitmap_rgb32 &bitmap, int from_line, int to_line, u32 color)
{
    bitmap.plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, color);
//  bitmap.plot_box(0, from_line, PAPER_WIDTH, to_line - from_line + 1, rgb_t::white());

}

void a2bus_silentype_device::write_snapshot_to_file(std::string directory, std::string name)
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


*/


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/
/*
void a2bus_silentype_device::darken_pixel(double headtemp, u32& pixel)
{
    if (headtemp > 0.0)
    {
        u8 intensity = headtemp * 15.0;
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

void a2bus_silentype_device::adjust_headtemp(u8 pin_status, double time_elapsed,  double& temp)
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

void a2bus_silentype_device::update_printhead(uint8_t headbits)
{

    double current_time = machine().time().as_double();
    double time_elapsed = current_time - last_update_time;
    last_update_time = current_time;

//      printf("PRINTHEAD %x\n",headbits);
//      printf("PRINTHEAD TIME ELAPSED = %f   %f usec     bitpattern=%s\n",
//      time_elapsed, time_elapsed*1e6, std::bitset<8>(headbits).to_string().c_str());

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

void a2bus_silentype_device::update_pf_stepper(uint8_t vstepper)
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

                        // clear paper to bottom
                        // something doesn't make sense with this formula...revisit this after I get some sleep
                        //m_bitmap.plot_box(m_ypos * 7 / 4, 3, PAPER_WIDTH - 1, PAPER_HEIGHT - 1, rgb_t::white());
                        bitmap_clear_band(m_bitmap, m_ypos * 7 / 4, PAPER_HEIGHT - 1, rgb_t::white());

                        // save a snapshot with the slot and page as part of the filename
                        write_snapshot_to_file(
                                    std::string("silentype"),
                                    std::string("silentype") +
                                    std::string("_slot") + std::to_string(slotno()) +
                                    "_page" + std::to_string(page_count++) + ".png");

                        newpageflag = 1;
                        m_ypos = 10;

                        // clear page down to visible area
                        bitmap_clear_band(m_bitmap, m_ypos * 7 / 4, PAPER_HEIGHT - 1, rgb_t::white());
                    }
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

void a2bus_silentype_device::update_cr_stepper(uint8_t hstepper)
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


*/


//-------------------------------------------------
//    Write c0nx
//-------------------------------------------------

void a2bus_silentype_device::write_c0nx(uint8_t offset, uint8_t data)
{
//  printf("WRITE %x = %x\n",offset+slotno()*0x10+0xc080,data);
	m_romenable = BIT(data,SILENTYPE_ROMENABLE) ? 1 : 0;

	if ((BIT(data,SILENTYPE_SHIFTCLOCKA) == 0) && (BIT(data,SILENTYPE_SHIFTCLOCKB) == 0))
	{
//  printf("CLEAR\n");
		m_shift_reg = 0;
	}
	else if ((BIT(data,SILENTYPE_SHIFTCLOCKA) == 0) && (BIT(data,SILENTYPE_SHIFTCLOCKB) == 1))
	{
//      printf("SHIFT\n");
		m_shift_reg = (m_shift_reg << 1) | BIT(data,SILENTYPE_DATA);
	}
	else if ((BIT(data,SILENTYPE_STORECLOCK) == 0))  // when NOT STORECLOCK, store shift register to parallel register
	{
		m_parallel_reg = m_shift_reg;

		uint8_t hstepperbits = BITS(m_parallel_reg, 3,  0);
		uint8_t vstepperbits = BITS(m_parallel_reg, 7,  4);
		uint8_t headbits     = BITS(m_parallel_reg, 15, 9);

		printf("PARALLEL REGISTER = %4x\n",m_parallel_reg);
//      update_pf_stepper(vstepperbits);
//      update_cr_stepper(hstepperbits);
//      update_printhead(headbits);

		m_silentype_printer->update_pf_stepper(vstepperbits);
		m_silentype_printer->update_cr_stepper(hstepperbits);
		m_silentype_printer->update_printhead(headbits);
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_silentype_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
-------------------------------------------------*/

void a2bus_silentype_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_silentype_device::read_c800(uint16_t offset)
{
	if ((offset>=0x700) && (!m_romenable))
	{
		return m_ram[offset-0x700];
	}
	else
	{
		return m_rom[offset];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/

void a2bus_silentype_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset >= 0x700) m_ram[(offset-0x700)] = data;
}

