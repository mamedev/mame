// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  Enhanced Graphics Adapter (EGA) section

TODO - Write documentation

"Regular" register on an EGA graphics card:

    3C2 - 7 6 5 4 3 2 1 0 - Misc Output Register - Write Only
          | | | | | | | |
          | | | | | | | +-- 3Bx/3Dx I/O port select
          | | | | | | |     0 = 3Bx for CRTC I/O, 3BA for status reg 1
          | | | | | | |     1 = 3Dx for CRTC I/O, 3DA for status reg 1
          | | | | | | +---- enable ram
          | | | | | |       0 = disable ram from the processor
          | | | | | |       1 = enable ram to respond to addresses
          | | | | | |           designated by the Control Data Select
          | | | | | |           value in the Graphics Controllers.
          | | | | | +------ clock select bit 0
          | | | | +-------- clock select bit 1
          | | | |           00 = 14MHz from Processor I/O channel
          | | | |           01 = 16MHz on-bord clock
          | | | |           10 = External clock from feature connector
          | | | |           11 = reserved/unused
          | | | +---------- disable video drivers
          | | |             0 = activate internal video drivers
          | | |             1 = disable internal video drivers
          | | +------------ page bit for odd/even. Selects between 2 pages
          | |               of 64KB of memory when in odd/even mode.
          | |               0 = select low page
          | |               1 = select high page
          | +-------------- horizontal retrace polarity
          |                 0 = select positive
          |                 1 = select negative
          +---------------- vertical retrace polarity
                            0 = select positive
                            1 = select negative


    3C2 - 7 6 5 4 3 2 1 0 - Input Status Reg 0 - Read Only
          | | | | | | | |
          | | | | | | | +-- reserved/unused
          | | | | | | +---- reserved/unused
          | | | | | +------ reserved/unused
          | | | | +-------- reserved/unused
          | | | +---------- switch sense
          | | |             0 = switch is closed
          | | |             1 = allows processor to read the 4 config switches
          | | |                 on the EGA adapter. The setting of CLKSEL determines
          | | |                 switch to read.
          | | +------------ input from FEAT0 on the feature connector
          | +-------------- input from FEAT1 on the feature connector
          +---------------- CRT Interrupt
                            0 = vertical retrace if occurring
                            1 = video is being displayed


    Configuration switches
    SW1 SW2 SW3 SW4
    OFF OFF OFF ON  - EGA, Color 80x25 (5153)
                    - EGA (primary) + MDA, Color 80x25 + Monochrome
    OFF OFF ON  OFF - EGA, Monochrome (5151)
                    - EGA (primary) + CGA, Monochrome + Color 80x25
    OFF OFF ON  ON  - EGA + MDA (primary), 5154 + Enhanced Monochrome
    OFF ON  OFF ON  - EGA + CGA (primary), Monochrome + Color 80x25
    OFF ON  ON  OFF - EGA, Enhanced Color - Enhanced Mode (5154)
                    - EGA (primary) + MDA, 5154 monitor + Enhanced Monochrome
    OFF ON  ON  ON  - EGA + MDA (primary), Color 80x25 + Monochrome
    ON  OFF OFF ON  - EGA, Color 40x25 (5153)
                    - EGA (primary) + MDA, Color 40x25 + Monochrome
    ON  OFF ON  OFF - EGA (primary) + CGA, Monochrome + Color 40x25
    ON  OFF ON  ON  - EGA + MDA (primary), 5154 + Normal Monochrome
    ON  ON  OFF ON  - EGA + CGA (primary), Monochrome + Color 40x25
    ON  ON  ON  OFF - EGA, Enhanced Color - Enhanced Mode (5154)
                    - EGA (primary) + MDA, 5154 monitor + Normal Monochrome
    ON  ON  ON  ON  - EGA + MDA (primary), Color 40x25 + Monochrome


    3XA - 7 6 5 4 3 2 1 0 - Feature Control Register - Write Only
          | | | | | | | |
          | | | | | | | +-- output to FEAT0 of the feature connector
          | | | | | | +---- output to FEAT1 of the feature connector
          | | | | | +------ reserved/unused
          | | | | +-------- reserved/unused
          | | | +---------- reserved/unused
          | | +------------ reserved/unused
          | +-------------- reserved/unused
          +---------------- reserved/unused

    3XA - 7 6 5 4 3 2 1 0 - Input Status Reg 1 - Read Only
          | | | | | | | |
          | | | | | | | +-- display enable
          | | | | | | |     0 = indicates the CRT raster is in a horizontal or vertical retrace
          | | | | | | |     1 = otherwise
          | | | | | | +---- light pen strobe
          | | | | | |       0 = light pen trigger has not been set
          | | | | | |       1 = light pen trigger has been set
          | | | | | +------ light pen switch
          | | | | |         0 = switch is closed
          | | | | |         1 = switch is open
          | | | | +-------- vertical blank
          | | | |           0 = video information is being displayed
          | | | |           1 = CRT is in vertical blank
          | | | +---------- diagnostic usage, output depends on AR12 video status mux bits
          | | |             mux bits - output
          | | |             00       - blue
          | | |             01       - I blue
          | | |             10       - I red
          | | |             11       - unknown
          | | +------------ diagnostic usage, output depends on AR12 video status mux bits
          | |               mux bits - output
          | |               00       - red
          | |               01       - green
          | |               10       - I green
          | |               11       - unknown
          | +-------------- reserved/unused
          +---------------- reserved/unused



The EGA graphics card introduces a lot of new indexed registers to handle the
enhanced graphics. These new indexed registers can be divided into three
groups:
- attribute registers
- sequencer registers
- graphics controllers registers


Attribute Registers AR00 - AR13

The Attribute Registers are all accessed through I/O port 0x3C0. The first
write to I/O port 0x3C0 sets the index register. The next write to I/O port
0x3C0 actually sets the data to the indexed register.

    3C0 - 7 6 5 4 3 2 1 0 - Attribute Access Register
          | | | | | | | |
          | | | | | | | +-- index bit 0
          | | | | | | +---- index bit 1
          | | | | | +------ index bit 2
          | | | | +-------- index bit 3
          | | | +---------- index bit 4
          | | +------------ palette source
          | +-------------- reserved/unused
          +---------------- reserved/unused


    AR00-AR0F - 7 6 5 4 3 2 1 0 - Palette Register #00 - #0F
                | | | | | | | |
                | | | | | | | +-- MSB B
                | | | | | | +---- MSB G
                | | | | | +------ MSB R
                | | | | +-------- LSB B
                | | | +---------- LSB G
                | | +------------ LSB R
                | +-------------- reserved/unused
                +---------------- reserved/unused


    AR10 - 7 6 5 4 3 2 1 0 - Mode Control Register
           | | | | | | | |
           | | | | | | | +-- Text/Graphics select
           | | | | | | +---- Monochrome/Color select
           | | | | | +------ 9th dot setting
           | | | | +-------- Blink Enable
           | | | +---------- reserved/unsued
           | | +------------ 0 = line compare does not affect pixel output
           | |               1 = line compare does affect pixel output
           | +-------------- 0 = pixel changes every dot clock
           |                 1 = pixel changes every other dot clock
           +---------------- reserved/unused


    AR11 - 7 6 5 4 3 2 1 0 - Overscan Color Register
           | | | | | | | |
           | | | | | | | +-- MSB B
           | | | | | | +---- MSB G
           | | | | | +------ MSB R
           | | | | +-------- LSB B
           | | | +---------- LSB G
           | | +------------ LSB R
           | +-------------- reserved/unused
           +---------------- reserved/unused


    AR12 - 7 6 5 4 3 2 1 0 - Color Plane Enable Register
           | | | | | | | |
           | | | | | | | +-- Enable plane 0
           | | | | | | +---- Enable plane 1
           | | | | | +------ Enable plane 2
           | | | | +-------- Enable plane 3
           | | | +---------- Video Status Mux bit 0
           | | +------------ Video Status Mux bit 1
           | +-------------- reserved/unused
           +---------------- reserved/unused


    AR13 - 7 6 5 4 3 2 1 0 - Horizontal Panning Register
           | | | | | | | |
           | | | | | | | +-- Pixel left shift bit 0
           | | | | | | +---- Pixel left shift bit 1
           | | | | | +------ Pixel left shift bit 2
           | | | | +-------- Pixel left shift bit 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


Sequencer Registers SR00 - SR04

The Sequencer Registers are accessed through an index register located at I/O
port 0x3C4, and a data register located at I/O port 0x3C5.

    3C4 - 7 6 5 4 3 2 1 0 - Sequencer Index Register - Write Only
          | | | | | | | |
          | | | | | | | +-- index bit 0
          | | | | | | +---- index bit 1
          | | | | | +------ index bit 2
          | | | | +-------- reserved/unused
          | | | +---------- reserved/unused
          | | +------------ reserved/unused
          | +-------------- reserved/unused
          +---------------- reserved/unused


    3C5 - 7 6 5 4 3 2 1 0 - Sequencer Data Register - Write Only
          | | | | | | | |
          | | | | | | | +-- data bit 0
          | | | | | | +---- data bit 1
          | | | | | +------ data bit 2
          | | | | +-------- data bit 3
          | | | +---------- data bit 4
          | | +------------ data bit 5
          | +-------------- data bit 6
          +---------------- data bit 7


    SR00 - 7 6 5 4 3 2 1 0 - Reset Control Register
           | | | | | | | |
           | | | | | | | +-- Must be 1 for normal operation
           | | | | | | +---- Must be 1 for normal operation
           | | | | | +------ reserved/unused
           | | | | +-------- reserved/unused
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    SR01 - 7 6 5 4 3 2 1 0 - Clocking Mode
           | | | | | | | |
           | | | | | | | +-- 0 = 9 dots per char, 1 = 8 dots per char
           | | | | | | +---- clock frequency, 0 = 4 out of 5 memory cycles, 1 = 2 out of 5 memory cycles
           | | | | | +------ shift load
           | | | | +-------- 0 = normal dot clock, 1 = master dot clock / 2
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    SR02 - 7 6 5 4 3 2 1 0 - Map Mask
           | | | | | | | |
           | | | | | | | +-- 1 = enable map 0 for writing
           | | | | | | +---- 1 = enable map 1 for writing
           | | | | | +------ 1 = enable map 2 for writing
           | | | | +-------- 1 = enable map 3 for writing
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    SR03 - 7 6 5 4 3 2 1 0 - Character Map Select
           | | | | | | | |
           | | | | | | | +-- character map select B bit 0
           | | | | | | +---- character map select B bit 1
           | | | | | |       Selects the map used to generate alpha characters when
           | | | | | |       attribute bit 3 is set to 0
           | | | | | |       00 = map 0 - 1st 8KB of plane 2 bank 0
           | | | | | |       01 = map 1 - 2nd 8KB of plane 2 bank 1
           | | | | | |       10 = map 2 - 3rd 8KB of plane 2 bank 2
           | | | | | |       11 = map 3 - 4th 8KB of plane 2 bank 3
           | | | | | +------ character map select A bit 0
           | | | | +-------- character map select A bit 1
           | | | |           Selects the map used to generate alpha characters when
           | | | |           attribute bit 3 is set to 1
           | | | |           00 = map 0 - 1st 8KB of plane 2 bank 0
           | | | |           01 = map 1 - 2nd 8KB of plane 2 bank 1
           | | | |           10 = map 2 - 3rd 8KB of plane 2 bank 2
           | | | |           11 = map 3 - 4th 8KB of plane 2 bank 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    SR04 - 7 6 5 4 3 2 1 0 - Memory Mode Register
           | | | | | | | |
           | | | | | | | +-- 0 = graphics mode, 1 = text mode
           | | | | | | +---- 0 = no memory extension, 1 = memory extension
           | | | | | +------ 0 = odd/even storage, 1 = sequential storage
           | | | | +-------- reserved/unused
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


Graphics Controller Registers GR00 - GR08

The Graphics Controller Registers are accessed through an index register
located at I/O port 0x3CE, and a data register located at I/O port 0x3CF.

    GR00 - 7 6 5 4 3 2 1 0 - Set/Reset Register
           | | | | | | | |
           | | | | | | | +-- set/reset for plane 0
           | | | | | | +---- set/reset for plane 1
           | | | | | +------ set/reset for plane 2
           | | | | +-------- set/reset for plane 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR01 - 7 6 5 4 3 2 1 0 - Enable Set/Reset Register
           | | | | | | | |
           | | | | | | | +-- enable set/reset for plane 0
           | | | | | | +---- enable set/reset for plane 1
           | | | | | +------ enable set/reset for plane 2
           | | | | +-------- enable set/reset for plane 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR02 - 7 6 5 4 3 2 1 0 - Color Compare Register
           | | | | | | | |
           | | | | | | | +-- color compare 0
           | | | | | | +---- color compare 1
           | | | | | +------ color compare 2
           | | | | +-------- color compare 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR03 - 7 6 5 4 3 2 1 0 - Data Rotate Register
           | | | | | | | |
           | | | | | | | +-- number of positions to rotate bit 0
           | | | | | | +---- number of positions to rotate bit 1
           | | | | | +------ number of positions to rotate bit 2
           | | | | +-------- function select bit 0
           | | | +---------- function select bit 1
           | | |             00 = data overwrites in specified color
           | | |             01 = data ANDed with latched data
           | | |             10 = data ORed with latched data
           | | |             11 = data XORed with latched data
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR04 - 7 6 5 4 3 2 1 0 - Read Map Select Register
           | | | | | | | |
           | | | | | | | +-- plane select bit 0
           | | | | | | +---- plane select bit 1
           | | | | | +------ reserved/unused
           | | | | +-------- reserved/unused
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR05 - 7 6 5 4 3 2 1 0 - Mode Register
           | | | | | | | |
           | | | | | | | +-- write mode bit 0
           | | | | | | +---- write mode bit 1
           | | | | | |       00 = write 8 bits of value in set/reset register if enabled,
           | | | | | |            otherwise write rotated processor data
           | | | | | |       01 = write with contents of processor latches
           | | | | | |       10 = memory plane 0-3 filled with 8 bits of value of data bit 0-3
           | | | | | |       11 = reserved/unused
           | | | | | +------ test condition
           | | | | |         0 = normal operation
           | | | | |         1 = put outputs in high impedance state
           | | | | +-------- read mode
           | | | |           0 = read from plane selected by GR04
           | | | |           1 = do color compare
           | | | +---------- odd/even addressing mode
           | | +------------ shift register mode
           | |               0 = sequential
           | |               1 = even bits from even maps, odd bits from odd maps
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR06 - 7 6 5 4 3 2 1 0 - Miscellaneous Register
           | | | | | | | |
           | | | | | | | +-- 0 = text mode, 1 = graphics mode
           | | | | | | +---- chain odd maps to even
           | | | | | +------ memory map bit 0
           | | | | +-------- memory map bit 1
           | | | |           00 = 0xA0000, 128KB
           | | | |           01 = 0xA0000, 64KB
           | | | |           10 = 0xB0000, 32KB
           | | | |           11 = 0xB8000, 32KB
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR07 - 7 6 5 4 3 2 1 0 - Color Plane Ignore Register
           | | | | | | | |
           | | | | | | | +-- ignore color plane 0
           | | | | | | +---- ignore color plane 1
           | | | | | +------ ignore color plane 2
           | | | | +-------- ignore color plane 3
           | | | +---------- reserved/unused
           | | +------------ reserved/unused
           | +-------------- reserved/unused
           +---------------- reserved/unused


    GR08 - 7 6 5 4 3 2 1 0 - Bit Mask Register
           | | | | | | | |
           | | | | | | | +-- write enable bit 0
           | | | | | | +---- write enable bit 1
           | | | | | +------ write enable bit 2
           | | | | +-------- write enable bit 3
           | | | +---------- write enable bit 4
           | | +------------ write enable bit 5
           | +-------------- write enable bit 6
           +---------------- write enable bit 7


***************************************************************************/

#include "emu.h"
#include "ega.h"

#define LOG_READ    (1U << 1)
#define LOG_SETUP   (1U << 2)
#define LOG_MODE    (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_MODE)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define EGA_SCREEN_NAME "ega_screen"
#define EGA_CRTC_NAME   "crtc_ega_ega"

#define EGA_MODE_GRAPHICS 1
#define EGA_MODE_TEXT     2

ROM_START( ega )
	ROM_REGION(0x4000, "user1", 0)
	ROM_DEFAULT_BIOS("ega")
	ROM_SYSTEM_BIOS(0, "ega", "IBM EGA BIOS")
	ROMX_LOAD("6277356.u44", 0x0000, 0x4000, CRC(dc146448) SHA1(dc0794499b3e499c5777b3aa39554bbf0f2cc19b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "iskr3104", "Iskra-3104 EGA BIOS")
	ROMX_LOAD( "143-03.bin", 0x0001, 0x2000, CRC(d0706345) SHA1(e04bb40d944426a4ae2e3a614d3f4953d7132ede), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "143-02.bin", 0x0000, 0x2000, CRC(c8c18ebb) SHA1(fd6dac76d43ab8b582e70f1d5cc931d679036fb9), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_REGION(0x4000, "user2", ROMREGION_ERASE00)
ROM_END

/*
0000 - MONOC PRIMARY, EGA COLOR, 40x25
0001 - MONOC PRIMARY, EGA COLOR, 80x25
0010 - MONOC PRIMARY, EGA HI RES EMULATE (SAME AS 0001)
0011 - MONOC PRIMARY, EGA HI RES ENHANCED
0100 - COLOR 40 PRIMARY, EGA MONOCHROME
0101 - COLOR 80 PRIMARY, EGA MONOCHROME

0110 - MONOC SECONDARY, EGA COLOR, 40x24
0111 - MONOC SECONDARY, EGA COLOR, 80x25
1000 - MONOC SECONDARY, EGA HI RES EMULATE (SAME AS 0111)
1001 - MONOC SECONDARY, EGA HI RES ENHANCED
1010 - COLOR 40 SECONDARY, EGA
1011 - COLOR 80 SECONDARY, EGA

1100 - RESERVED
1101 - RESERVED
1110 - RESERVED
1111 - RESERVED
*/

INPUT_PORTS_START( ega )
	PORT_START( "config" )
	PORT_DIPNAME( 0x0f, 0x09, "Display Type" )
	PORT_DIPSETTING( 0x00, "0000 - MDA PRIMARY, EGA COLOR, 40x25" )                            /* DIAG: ?? 40 cols, RGBI */
	PORT_DIPSETTING( 0x08, "0001 - MDA PRIMARY, EGA COLOR, 80x25" )                            /* DIAG: ?? 80 cols, RGBI */
	PORT_DIPSETTING( 0x04, "0010 - MDA PRIMARY, EGA HI RES EMULATE (SAME AS 0001)" )           /* DIAG: ?? 80 cols, RGBI */
	PORT_DIPSETTING( 0x0c, "0011 - MDA PRIMARY, EGA HI RES ENHANCED" )                         /* DIAG: Color Display 40 cols, RrGgBb */
	PORT_DIPSETTING( 0x02, "0100 - CGA 40 PRIMARY, EGA MONOCHROME" )                           /* DIAG: ??, Mono RGBI */
	PORT_DIPSETTING( 0x0a, "0101 - CGA 80 PRIMARY, EGA MONOCHROME" )                           /* DIAG: ??, Mono RGBI */
	PORT_DIPSETTING( 0x06, "0110 - MDA SECONDARY, EGA COLOR, 40x25" )                          /* DIAG: Color Display 40 cols, RGBI */
	PORT_DIPSETTING( 0x0e, "0111 - MDA SECONDARY, EGA COLOR, 80x25" )                          /* DIAG: Color Display 80 cols, RGBI */
	PORT_DIPSETTING( 0x01, "1000 - MDA SECONDARY, EGA HI RES EMULATE (SAME AS 0111)" )         /* DIAG: Color Display 80 cols, RGBI */
	PORT_DIPSETTING( 0x09, "1001 - MDA SECONDARY, EGA HI RES ENHANCED" )                       /* DIAG: Color Display 40 cols, RrGgBb */
	PORT_DIPSETTING( 0x05, "1010 - COLOR 40 SECONDARY, EGA" )                                  /* DIAG: Monochrome display, Mono RGBI */
	PORT_DIPSETTING( 0x0d, "1011 - COLOR 80 SECONDARY, EGA" )                                  /* DIAG: Monochrome display, Mono RGBI */
	PORT_DIPSETTING( 0x03, "1100 - RESERVED" )                                                 /* ??, RGBI */
	PORT_DIPSETTING( 0x0b, "1101 - RESERVED" )                                                 /* ??, RGBI */
	PORT_DIPSETTING( 0x07, "1110 - RESERVED" )                                                 /* ??, RGBI */
	PORT_DIPSETTING( 0x0f, "1111 - RESERVED" )                                                 /* ??, RGBI */
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_EGA, isa8_ega_device, "ega", "IBM Enhanced Graphics Adapter")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_ega_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16.257_MHz_XTAL, 912, 0, 640, 262, 0, 200);
	m_screen->set_screen_update(EGA_CRTC_NAME, FUNC(crtc_ega_device::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(64);

	CRTC_EGA(config, m_crtc_ega, 16.257_MHz_XTAL/8);
	m_crtc_ega->set_screen(EGA_SCREEN_NAME);
	m_crtc_ega->config_set_hpixels_per_column(8);
	m_crtc_ega->set_row_update_callback(FUNC(isa8_ega_device::ega_update_row));
	m_crtc_ega->res_out_de_callback().set(FUNC(isa8_ega_device::de_changed));
	m_crtc_ega->res_out_hsync_callback().set(FUNC(isa8_ega_device::hsync_changed));
	m_crtc_ega->res_out_vsync_callback().set(FUNC(isa8_ega_device::vsync_changed));
	m_crtc_ega->res_out_vblank_callback().set(FUNC(isa8_ega_device::vblank_changed));
	m_crtc_ega->res_out_irq_callback().set([this](int state) { m_irq = state; m_isa->irq2_w(state); });
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_ega_device::device_rom_region() const
{
	return ROM_NAME( ega );
}

ioport_constructor isa8_ega_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ega );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_ega_device - constructor
//-------------------------------------------------

isa8_ega_device::isa8_ega_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_ega_device(mconfig, ISA8_EGA, tag, owner, clock)
{
}

isa8_ega_device::isa8_ega_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_crtc_ega(*this, EGA_CRTC_NAME), m_videoram(nullptr), m_charA(nullptr), m_charB(nullptr),
	m_misc_output(0), m_feature_control(0), m_frame_cnt(0), m_hsync(0), m_vsync(0), m_vblank(0), m_display_enable(0), m_irq(0), m_video_mode(0),
	m_palette(*this, "palette"), m_screen(*this, EGA_SCREEN_NAME)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ega_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();

	for (int i = 0; i < 64; i++ )
	{
		uint8_t r = ( ( i & 0x04 ) ? 0xAA : 0x00 ) + ( ( i & 0x20 ) ? 0x55 : 0x00 );
		uint8_t g = ( ( i & 0x02 ) ? 0xAA : 0x00 ) + ( ( i & 0x10 ) ? 0x55 : 0x00 );
		uint8_t b = ( ( i & 0x01 ) ? 0xAA : 0x00 ) + ( ( i & 0x08 ) ? 0x55 : 0x00 );

		m_palette->set_pen_color( i, r, g, b );
	}

	if(m_default_bios_tag != "iskr3104")
	{
		uint8_t   *dst = memregion(subtag("user2").c_str())->base() + 0x0000;
		uint8_t   *src = memregion(subtag("user1").c_str())->base() + 0x3fff;
		int     i;

		/* Perform the EGA bios address line swaps */
		for( i = 0; i < 0x4000; i++ )
		{
			*dst++ = *src--;
		}
	}
	else
		memcpy(memregion(subtag("user2").c_str())->base(), memregion(subtag("user1").c_str())->base(), 0x4000);

	/* Install 256KB Video ram on our EGA card */
	m_vram = make_unique_clear<uint8_t[]>(256 * 1024);

	m_videoram = m_vram.get();
	m_plane[0] = m_videoram + 0x00000;
	m_plane[1] = m_videoram + 0x10000;
	m_plane[2] = m_videoram + 0x20000;
	m_plane[3] = m_videoram + 0x30000;

	save_item(STRUCT_MEMBER(m_graphics_controller, index));
	save_item(STRUCT_MEMBER(m_graphics_controller, data));
	save_item(STRUCT_MEMBER(m_sequencer, index));
	save_item(STRUCT_MEMBER(m_sequencer, data));
	save_item(STRUCT_MEMBER(m_attribute, index));
	save_item(STRUCT_MEMBER(m_attribute, data));
	save_item(STRUCT_MEMBER(m_attribute, index_write));
	save_pointer(NAME(m_vram), 256 * 1024);

	m_isa->install_rom(this, 0xc0000, 0xc3fff, "user2");
	m_isa->install_device(0x3b0, 0x3bf, read8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3b0_r)), write8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3b0_w)));
	m_isa->install_device(0x3c0, 0x3cf, read8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3c0_r)), write8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3c0_w)));
	m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3d0_r)), write8sm_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3d0_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_ega_device::device_reset()
{
	m_feature_control = 0;

	memset(&m_attribute,0,sizeof(m_attribute));
	memset(&m_sequencer,0,sizeof(m_sequencer));
	memset(&m_graphics_controller,0,sizeof(m_graphics_controller));

	m_frame_cnt = 0;
	m_hsync = 0;
	m_vsync = 0;
	m_vblank = 0;
	m_display_enable = 0;
	m_irq = 0;

	install_banks();

	m_misc_output = 0;
	m_attribute.index_write = 1;

	/* Set up default palette */
	m_attribute.data[0] = 0;
	m_attribute.data[1] = 1;
	m_attribute.data[2] = 2;
	m_attribute.data[3] = 3;
	m_attribute.data[4] = 4;
	m_attribute.data[5] = 5;
	m_attribute.data[6] = 0x14;
	m_attribute.data[7] = 7;
	m_attribute.data[8] = 0x38;
	m_attribute.data[9] = 0x39;
	m_attribute.data[10] = 0x3A;
	m_attribute.data[11] = 0x3B;
	m_attribute.data[12] = 0x3C;
	m_attribute.data[13] = 0x3D;
	m_attribute.data[14] = 0x3E;
	m_attribute.data[15] = 0x3F;

	m_video_mode = 0;
}

void isa8_ega_device::install_banks()
{
	switch ( m_graphics_controller.data[6] & 0x0c )
	{
	case 0x00:      /* 0xA0000, 128KB */
		if ( m_misc_output & 0x02 )
		{
			m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(isa8_ega_device::read)), write8sm_delegate(*this, FUNC(isa8_ega_device::write)));
		}
		else
		{
			m_isa->unmap_bank(0xa0000, 0xaffff);
			m_isa->unmap_bank(0xb0000, 0xb7fff);
			m_isa->unmap_bank(0xb8000, 0xbffff);
		}
		break;
	case 0x04:      /* 0xA0000, 64KB */
		if ( m_misc_output & 0x02 )
		{
			m_isa->install_memory(0xa0000, 0xaffff, read8sm_delegate(*this, FUNC(isa8_ega_device::read)), write8sm_delegate(*this, FUNC(isa8_ega_device::write)));
		}
		else
		{
			m_isa->unmap_bank(0xa0000, 0xaffff);
		}
		/* These unmaps may break multi graphics card support */
		m_isa->unmap_bank(0xb0000, 0xb7fff);
		m_isa->unmap_bank(0xb8000, 0xbffff);
		break;
	case 0x08:      /* 0xB0000, 32KB */
		if ( m_misc_output & 0x02 )
		{
			m_isa->install_memory(0xb0000, 0xb7fff, read8sm_delegate(*this, FUNC(isa8_ega_device::read)), write8sm_delegate(*this, FUNC(isa8_ega_device::write)));
		}
		else
		{
			m_isa->unmap_bank(0xb0000, 0xb7fff);
		}
		/* These unmaps may break multi graphics card support */
		m_isa->unmap_bank(0xa0000, 0xaffff);
		m_isa->unmap_bank(0xb8000, 0xbffff);
		break;
	case 0x0c:      /* 0xB8000, 32KB */
		if ( m_misc_output & 0x02 )
		{
			m_isa->install_memory(0xb8000, 0xbffff, read8sm_delegate(*this, FUNC(isa8_ega_device::read)), write8sm_delegate(*this, FUNC(isa8_ega_device::write)));
		}
		else
		{
			m_isa->unmap_bank(0xb8000, 0xbffff);
		}
		/* These unmaps may break multi graphics card support */
		m_isa->unmap_bank(0xa0000, 0xaffff);
		m_isa->unmap_bank(0xb0000, 0xb7fff);
		break;
	}
}

CRTC_EGA_PIXEL_UPDATE( isa8_ega_device::ega_update_row )
{
	if (m_video_mode == EGA_MODE_GRAPHICS)
		pc_ega_graphics(bitmap, cliprect, ma, ra, y, x, cursor_x);
	else if (m_video_mode == EGA_MODE_TEXT)
		pc_ega_text(bitmap, cliprect, ma, ra, y, x, cursor_x);
}


void isa8_ega_device::de_changed(int state)
{
	m_display_enable = state ? 1 : 0;
}


void isa8_ega_device::hsync_changed(int state)
{
	m_hsync = state ? 1 : 0;
}


void isa8_ega_device::vsync_changed(int state)
{
	m_vsync = state ? 1 : 0;
	if ( state )
	{
		m_frame_cnt++;
	}
}


void isa8_ega_device::vblank_changed(int state)
{
	m_vblank = state ? 8 : 0;
}


CRTC_EGA_PIXEL_UPDATE( isa8_ega_device::pc_ega_graphics )
{
	uint16_t  *p = &bitmap.pix(y, x * 8);

	LOG("%s: y = %d, x = %d, ma = %d, ra = %d\n", FUNCNAME, y, x, ma, ra );

	if ( m_graphics_controller.data[5] & 0x10 )
	{
		// Odd/Even mode (CGA compatible)

		uint16_t offset = ( ma & 0x1fff ) | ( ( y & 1 ) << 12 );
		uint8_t data = m_plane[BIT(m_misc_output, 5) ? 2 : 0][offset];

		*p = m_attribute.data[ ( data >> 6 )        ]; p++;
		*p = m_attribute.data[ ( data >> 4 ) & 0x03 ]; p++;
		*p = m_attribute.data[ ( data >> 2 ) & 0x03 ]; p++;
		*p = m_attribute.data[   data        & 0x03 ]; p++;

		data = m_plane[BIT(m_misc_output, 5) ? 3 : 1][offset];

		*p = m_attribute.data[ ( data >> 6 )        ]; p++;
		*p = m_attribute.data[ ( data >> 4 ) & 0x03 ]; p++;
		*p = m_attribute.data[ ( data >> 2 ) & 0x03 ]; p++;
		*p = m_attribute.data[   data        & 0x03 ]; p++;
	}
	else
	{
		// EGA mode

		uint8_t mask = m_attribute.data[0x12] & 0x0f;

		uint16_t offset = ma;
		uint16_t data0 = m_plane[0][offset];
		uint16_t data1 = m_plane[1][offset] << 1;
		uint16_t data2 = m_plane[2][offset] << 2;
		uint16_t data3 = m_plane[3][offset] << 3;

		for ( int j = 7; j >= 0; j-- )
		{
			uint16_t col = ( data0 & 0x01 ) | ( data1 & 0x02 ) | ( data2 & 0x04 ) | ( data3 & 0x08 );

			col &= mask;

			if ( m_misc_output & 0x80 )
				p[j] = m_attribute.data[col];
			else
			{
				p[j] = m_attribute.data[col] | (BIT(m_attribute.data[col], 4) ? 0x38 : 0);
				if ( p[j] == 6 )
					p[j] = 0x14;
			}

			data0 >>= 1;
			data1 >>= 1;
			data2 >>= 1;
			data3 >>= 1;
		}
	}
}


CRTC_EGA_PIXEL_UPDATE( isa8_ega_device::pc_ega_text )
{
	uint16_t  *p = &bitmap.pix(y, x * ( ( m_sequencer.data[0x01] & 0x01 ) ? 8 : 9 ) );

	LOG("%s: y = %d, x = %d, ma = %d, ra = %d\n", FUNCNAME, y, x, ma, ra );

	uint16_t  offset = ma;
	uint8_t   chr = m_plane[0][ offset ];
	uint8_t   attr = m_plane[1][ offset ];
	uint8_t   data;
	uint8_t blink = m_attribute.data[0x10] & 0x08;
	uint16_t  fg = m_attribute.data[ attr & 0x0f ];
	uint16_t  bg = m_attribute.data[ ( attr >> 4 ) & (blink ? 0x07 : 0x0f) ];

	if ( m_charA == m_charB )
		data = m_charB[ chr * 32 + ra ];
	else
	{
		/* character set selector */
		data = ( attr & 0x08 ) ? m_charA[ chr * 32 + ra ] : m_charB[ chr * 32 + ra ];
		fg &= 0x07;
	}

	if(m_screen->visible_area().height() == 200) // the ibm 5154 forces cga compatibility in 200 line modes
	{
		fg = fg | (BIT(fg, 4) ? 0x38 : 0);
		if ( fg == 6 )
			fg = 0x14;
		bg = bg | (BIT(bg, 4) ? 0x38 : 0);
		if ( bg == 6 )
			bg = 0x14;
	}

	if ( x == cursor_x )
	{
		if ( m_frame_cnt & 0x08 )
		{
			data = 0xFF;
		}
	}
	else
	{
		/* Check for blinking */
		if ( ( m_attribute.data[0x10] & 0x08 ) && ( attr & 0x80 ) && ( m_frame_cnt & 0x10 ) )
		{
			data = 0x00;
		}
	}

	*p = ( data & 0x80 ) ? fg : bg; p++;
	*p = ( data & 0x40 ) ? fg : bg; p++;
	*p = ( data & 0x20 ) ? fg : bg; p++;
	*p = ( data & 0x10 ) ? fg : bg; p++;
	*p = ( data & 0x08 ) ? fg : bg; p++;
	*p = ( data & 0x04 ) ? fg : bg; p++;
	*p = ( data & 0x02 ) ? fg : bg; p++;
	*p = ( data & 0x01 ) ? fg : bg; p++;
	if ( !( m_sequencer.data[0x01] & 0x01 ) )
		*p = ( m_attribute.data[0x10] & 0x04 ) ? *(p - 1) : bg;
}


void isa8_ega_device::change_mode()
{
	m_video_mode = 0;

	/* Check for graphics mode */
	if (   ( m_attribute.data[0x10] & 0x01 ) &&
			! ( m_sequencer.data[0x04] & 0x01 ) &&
			( m_graphics_controller.data[0x06] & 0x01 ) )
	{
		LOGMODE("%s: Switch to graphics mode\n", FUNCNAME);

		m_video_mode = EGA_MODE_GRAPHICS;
	}

	/* Check for text mode */
	if ( ! ( m_attribute.data[0x10] & 0x01 ) &&
			( m_sequencer.data[0x04] & 0x01 ) &&
			! ( m_graphics_controller.data[0x06] & 0x01 ) )
	{
		LOGMODE("%s: Switching to text mode\n", FUNCNAME);

		m_video_mode = EGA_MODE_TEXT;

		/* Set character maps */
		if ( m_sequencer.data[0x04] & 0x01 )
		{
			m_charA = m_plane[2] + ( ( m_sequencer.data[0x03] & 0x0c ) >> 2 ) * 0x4000;
			m_charB = m_plane[2] + ( m_sequencer.data[0x03] & 0x03 ) * 0x4000;
		}
		else
		{
			m_charA = m_plane[2];
			m_charB = m_plane[2];
		}
	}

	/* Check for changes to the crtc input clock and number of pixels per clock */
	int clock = ( ( m_misc_output & 0x0c ) ? 16257000 : 14318181 );
	int pixels = ( ( m_sequencer.data[0x01] & 0x01 ) ? 8 : 9 );

	if ( m_sequencer.data[0x01] & 0x08 )
	{
		clock >>= 1;
	}
	m_crtc_ega->set_clock( clock / pixels );
	m_crtc_ega->set_hpixels_per_column( pixels );

	if (!m_video_mode)
		logerror("unknown video mode\n");
}


uint8_t isa8_ega_device::read(offs_t offset)
{
	uint8_t data = 0xFF;

	if ( !machine().side_effects_disabled() && !( m_graphics_controller.data[5] & 0x10 ) )
	{
		/* Fill read latches */
		m_read_latch[0] = m_plane[0][offset & 0xffff];
		m_read_latch[1] = m_plane[1][offset & 0xffff];
		m_read_latch[2] = m_plane[2][offset & 0xffff];
		m_read_latch[3] = m_plane[3][offset & 0xffff];
	}

	if ( m_graphics_controller.data[5] & 0x08 )
	{
		// Read mode #1
		popmessage("ega: Read mode 1 not supported yet!");
		printf("EGA: Read mode 1 not supported yet!\n");
	}
	else
	{
		// Read mode #0
		if ( !( m_graphics_controller.data[5] & 0x10 ) )
		{
			// Normal addressing mode
			if ( m_graphics_controller.data[6] & 2 )
			{
				data = m_plane[ m_graphics_controller.data[4] & 0x03 ][offset & 0xfffe];
			}
			else
			{
				data = m_plane[ m_graphics_controller.data[4] & 0x03 ][offset & 0xffff];
			}
		}
		else
		{
			// Odd/Even addressing mode
			data = m_plane[offset & 1][offset & 0xfffe];
		}
	}

	return data;
}


uint8_t isa8_ega_device::alu_op( uint8_t data, uint8_t latch_data )
{
	uint8_t mask = m_graphics_controller.data[8];

	switch( m_graphics_controller.data[3] & 0x18 )
	{
	case 0x00:      // Unmodified
		return ( data & mask ) | ( latch_data & ~mask );

	case 0x08:      // AND
		return ( data | ~mask ) & latch_data;

	case 0x10:      // OR
		return ( data & mask ) | latch_data;

	case 0x18:      // XOR
		return ( data & mask ) ^ latch_data;
	}
	return 0;
}


void isa8_ega_device::write(offs_t offset, uint8_t data)
{
	uint8_t d[4];
	uint8_t alu[4];

	alu[0] =alu[1] = alu[2] = alu[3] = 0;

	switch( m_graphics_controller.data[5] & 0x03 )
	{
	case 0:     // Write mode 0
		// Pass through barrel shifter
		data = ( ( ( data << 8 ) | data ) >> ( m_graphics_controller.data[3] & 0x07 ) ) & 0xFF;

		d[0] = d[1] = d[2] = d[3] = data;

		/* Apply Set/Reset settings */
		if ( m_graphics_controller.data[1] & 0x01 )
		{
			d[0] = ( m_graphics_controller.data[0] & 0x01 ) ? 0xff : 0x00;
		}
		if ( m_graphics_controller.data[1] & 0x02 )
		{
			d[1] = ( m_graphics_controller.data[0] & 0x02 ) ? 0xff : 0x00;
		}
		if ( m_graphics_controller.data[1] & 0x04 )
		{
			d[2] = ( m_graphics_controller.data[0] & 0x04 ) ? 0xff : 0x00;
		}
		if ( m_graphics_controller.data[1] & 0x08 )
		{
			d[3] = ( m_graphics_controller.data[0] & 0x08 ) ? 0xff : 0x00;
		}

		// Pass through ALUs
		alu[0] = alu_op( d[0], m_read_latch[0] );
		alu[1] = alu_op( d[1], m_read_latch[1] );
		alu[2] = alu_op( d[2], m_read_latch[2] );
		alu[3] = alu_op( d[3], m_read_latch[3] );

		break;

	case 1:     // Write mode 1
		alu[0] = m_read_latch[0];
		alu[1] = m_read_latch[1];
		alu[2] = m_read_latch[2];
		alu[3] = m_read_latch[3];
		break;

	case 2:     // Write mode 2
		d[0] = ( data & 0x01 ) ? 0xff : 0x00;
		d[1] = ( data & 0x02 ) ? 0xff : 0x00;
		d[2] = ( data & 0x04 ) ? 0xff : 0x00;
		d[3] = ( data & 0x08 ) ? 0xff : 0x00;

		alu[0] = alu_op( d[0], m_read_latch[0] );
		alu[1] = alu_op( d[1], m_read_latch[1] );
		alu[2] = alu_op( d[2], m_read_latch[2] );
		alu[3] = alu_op( d[3], m_read_latch[3] );
		break;

	case 3:     // Write mode 3
		popmessage("EGA: Write mode 3 not supported!");
		return;
	}

	offset &= 0xffff;

	//
	// Plane selection
	// TODO: Get this logic clearer. The documentation is unclear on the exact magic combination of bits.
	//
	if ( m_sequencer.data[4] & 0x04 )
	{
		// Sequential addressing mode
		if ( m_sequencer.data[2] & 0x01 )
		{
			// Plane 0
			// Bit selection
			m_plane[0][offset] = alu[0];
		}
		if ( m_sequencer.data[2] & 0x02 )
		{
			// Plane 1
			// Bit selection
			m_plane[1][offset] = alu[1];
		}
		if ( m_sequencer.data[2] & 0x04 )
		{
			// Plane 2
			// Bit selection
			m_plane[2][offset] = alu[2];
		}
		if ( m_sequencer.data[2] & 0x08 )
		{
			// Plane 3
			// Bit selection
			m_plane[3][offset] = alu[3];
		}
	}
	else
	{
		// Odd/Even addressing mode
		if ( offset & 1 )
		{
			// Odd addresses go to planes 1 and 3
			offset &= ~1;

			if ( m_sequencer.data[2] & 0x02 )
			{
				// Plane 1
				// Bit selection
				m_plane[1][offset] = alu[1];
			}
			if ( ( m_sequencer.data[2] & 0x08 ) && ! ( m_sequencer.data[4] & 0x01 ) )
			{
				// Plane 3
				// Bit selection
				m_plane[3][offset] = alu[3];
			}
		}
		else
		{
			// Even addresses go to planes 0 and 2

			if ( m_sequencer.data[2] & 0x01 )
			{
				// Plane 0
				// Bit selection
				m_plane[0][offset] = alu[0];
			}
			if ( ( m_sequencer.data[2] & 0x04 ) && ! ( m_sequencer.data[4] & 0x01 ) )
			{
				// Plane 2
				// Bit selection
				m_plane[2][offset] = alu[2];
			}
		}
	}
}


uint8_t isa8_ega_device::pc_ega8_3X0_r(offs_t offset)
{
	int data = 0xff;

	switch ( offset )
	{
	/* CRT Controller - address register */
	case 0: case 2: case 4: case 6:
		/* return last written mc6845 address value here? */
		break;

	/* CRT Controller - data register */
	case 1: case 3: case 5: case 7:
		data = m_crtc_ega->register_r();
		break;

	/* Input Status Register 1 */
	case 10:
		data = m_vblank | ( m_hsync | m_vsync ); //  m_display_enable;

		if ( m_display_enable )
		{
			/* For the moment i'm putting in some bogus data */
			static int pixel_data;

			pixel_data = ( pixel_data + 1 ) & 0x03;
			data |= ( pixel_data << 4 );
		}

		/* Reset the attirubte writing flip flop to let the next write go to the index reigster */
		m_attribute.index_write = 1;
		break;
	}

	return data;
}

void isa8_ega_device::pc_ega8_3X0_w(offs_t offset, uint8_t data)
{
	LOGSETUP("%s: offset = %02x, data = %02x\n", FUNCNAME, offset, data );

	switch ( offset )
	{
	/* CRT Controller - address register */
	case 0: case 2: case 4: case 6:
		m_crtc_ega->address_w(data);
		break;

	/* CRT Controller - data register */
	case 1: case 3: case 5: case 7:
		m_crtc_ega->register_w(data);
		break;

	/* Set Light Pen Flip Flop */
	case 9:
		break;

	/* Feature Control */
	case 10:
		m_feature_control = data;
		break;

	/* Clear Light Pen Flip Flop */
	case 11:
		break;
	}
}



uint8_t isa8_ega_device::pc_ega8_3b0_r(offs_t offset)
{
	return ( m_misc_output & 0x01 ) ? 0xFF : pc_ega8_3X0_r(offset);
}


uint8_t isa8_ega_device::pc_ega8_3d0_r(offs_t offset)
{
	return ( m_misc_output & 0x01 ) ? pc_ega8_3X0_r(offset) : 0xFF;
}


void isa8_ega_device::pc_ega8_3b0_w(offs_t offset, uint8_t data)
{
	if ( ! ( m_misc_output & 0x01 ) )
	{
		pc_ega8_3X0_w( offset, data );
	}
}


void isa8_ega_device::pc_ega8_3d0_w(offs_t offset, uint8_t data)
{
	if ( m_misc_output & 0x01 )
	{
		pc_ega8_3X0_w( offset, data );
	}
}


uint8_t isa8_ega_device::pc_ega8_3c0_r(offs_t offset)
{
	int data = 0xff;

	LOGR("%s: offset = %02x\n", FUNCNAME, offset );

	switch ( offset )
	{
	/* Attributes Controller */
	case 0:
		break;

	/* Feature Read */
	case 2:
		{
			uint8_t dips = ioport("config")->read();

			data = ( data & 0x0f );
			data |= ( ( m_feature_control & 0x03 ) << 5 );
			data |= ( m_irq ? 0x00 : 0x80 );
			data |= ( ( ( dips >> ( ( ( m_misc_output & 0x0c ) >> 2 ) ) ) & 0x01 ) << 4 );
		}
		break;

	/* Sequencer */
	case 4:
		break;
	case 5:
		break;

	/* Graphics Controller */
	case 14:
		break;
	case 15:
		break;
	}
	return data;
}


void isa8_ega_device::pc_ega8_3c0_w(offs_t offset, uint8_t data)
{
	static const uint8_t ar_reg_mask[0x20] =
		{
			0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
			0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
			0x7F, 0x3F, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
	static const uint8_t sr_reg_mask[0x08] =
		{
			0x03, 0x0F, 0x0F, 0x0F, 0x07, 0x00, 0x00, 0x00
		};
	static const uint8_t gr_reg_mask[0x10] =
		{
			0x0F, 0x0F, 0x0F, 0x1F, 0x07, 0x3F, 0x0F, 0x0F,
			0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
	int index;

	LOGSETUP("%s: offset = %02x, data = %02x\n", FUNCNAME, offset, data );

	switch ( offset )
	{
	/* Attributes Controller */
	case 0:
		if ( m_attribute.index_write )
		{
			m_attribute.index = data;
		}
		else
		{
			index = m_attribute.index & 0x1F;

			LOGSETUP(" - AR%02X = 0x%02x\n", index, data );

			/* Clear unused bits */
			m_attribute.data[ index ] = data & ar_reg_mask[ index ];

			switch ( index )
			{
			case 0x10:      /* AR10 */
				change_mode();
				break;
			}
		}
		m_attribute.index_write ^= 0x01;
		break;

	/* Miscellaneous Output */
	case 2:
		m_misc_output = data;
		install_banks();
		change_mode();
		break;

	/* Sequencer */
	case 4:
		m_sequencer.index = data;
		break;
	case 5:
		index = m_sequencer.index & 0x07;

		LOGSETUP(" - SR%02X = 0x%02x\n", index & 0x07, data );

		/* Clear unused bits */
		m_sequencer.data[ index ] = data & sr_reg_mask[ index ];

		switch ( index )
		{
		case 0x01:      /* SR01 */
		case 0x03:      /* SR03 */
		case 0x04:      /* SR04 */
			change_mode();
			break;
		}
		break;

	/* Graphics Controller */
	case 14:
		m_graphics_controller.index = data;
		break;
	case 15:
		index = m_graphics_controller.index & 0x0F;

		LOGSETUP(" - GR%02X = 0x%02x\n", index, data );

		/* Clear unused bits */
		m_graphics_controller.data[ index ] = data & gr_reg_mask[ index ];

		switch ( index )
		{
		case 0x06:      /* GR06 */
			change_mode();
			install_banks();
			break;
		}
		break;
	}
}
