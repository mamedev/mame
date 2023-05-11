// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Commodore MPS-1200 & MPS-1250 printers (skeleton)

    The MPS-1200's CPU board was originally designed for a standard
    Centronics parallel interface (Y8300). However, an alternate
    "Basic Interface Pack" board (Y8306) instead supported the IEC bus
    using some extra LSTTL glue logic to convert serial data input to
    the parallel format read by the CPU. The later MPS-1250 board
    (Y8307) had hardware to support both serial and parallel
    interfaces, but only used one at a time.

**********************************************************************/

#include "emu.h"
#include "mps1200.h"

#include "cpu/m6502/m50734.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(MPS1200, mps1200_device, "mps1200", "Commodore MPS-1200 Dot Matrix Printer")
DEFINE_DEVICE_TYPE(MPS1250, mps1250_device, "mps1250", "Commodore MPS-1250 Dot Matrix Printer")


//-------------------------------------------------
//  mps1200_device - constructor
//-------------------------------------------------

mps1200_device::mps1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
	, m_mpscpu(*this, "mpscpu")
{
}

mps1200_device::mps1200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mps1200_device(mconfig, MPS1200, tag, owner, clock)
{
}


//-------------------------------------------------
//  mps1250_device - constructor
//-------------------------------------------------

mps1250_device::mps1250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mps1200_device(mconfig, MPS1250, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1200_device::device_start()
{
}


//-------------------------------------------------
//  cbm_iec_atn - ATN line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_atn)
{
	// TODO
}


//-------------------------------------------------
//  cbm_iec_data - serial data line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_data)
{
	// TODO
}


//-------------------------------------------------
//  cbm_iec_reset - reset line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_reset)
{
	// TODO
}


//-------------------------------------------------
//  mem_map - address map for main memory space
//-------------------------------------------------

void mps1200_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram"); // M5M5165P-12
	map(0x8000, 0xffff).rom().region("firmware", 0x8000);
}


//-------------------------------------------------
//  data_map - address map for data memory space
//-------------------------------------------------

void mps1200_device::data_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram");
	map(0x8000, 0xffff).rom().region("firmware", 0);
}


//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void mps1200_device::device_add_mconfig(machine_config &config)
{
	M50734(config, m_mpscpu, 8_MHz_XTAL);
	m_mpscpu->set_addrmap(AS_PROGRAM, &mps1200_device::mem_map);
	m_mpscpu->set_addrmap(AS_DATA, &mps1200_device::data_map);
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(mps1200)
	PORT_START("SW")
	PORT_DIPNAME(0x0080, 0x0080, "Device Number" )                PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x0080, "4" )
	PORT_DIPSETTING(0x0000, "5" )
	PORT_DIPNAME(0x0040, 0x0040, "ASCII Translation" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x0040, "PET ASCII" )
	PORT_DIPSETTING(0x0000, "ASCII" ) // access to full ascii character set
	PORT_DIPNAME(0x0020, 0x0020, "Control Code Mode" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x0020, "Commodore" )
	PORT_DIPSETTING(0x0000, "Epson FX" ) // access to escape control codes when using Epson printer driver (software-specific use)
	PORT_DIPNAME(0x0010, 0x0010, "Print Quality" )                PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x0010, "Draft" )
	PORT_DIPSETTING(0x0000, "NLQ" )
	PORT_DIPNAME(0x0008, 0x0008, "Page Length" )                  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x0008, "11 inch" )
	PORT_DIPSETTING(0x0000, "12 inch" )
	PORT_DIPNAME(0x0004, 0x0000, "Paper End Detect" )             PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x0004, "Enabled" )  // when enabled printer stops printing 2 inches before end of page and beeps to change paper
	PORT_DIPSETTING(0x0000, "Disabled" ) // so generally leave this disabled (ON) to allow printing closer to the end of a page
	PORT_DIPNAME(0x0002, 0x0002, "Automatic Line Feed" )          PORT_DIPLOCATION("SW1:7") // only does something if sw1:3 is on, otherwise ignored
	PORT_DIPSETTING(0x0002, "Disabled" )
	PORT_DIPSETTING(0x0000, "Enabled" )
	PORT_DIPNAME(0x0001, 0x0001, "Character Spacing" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x0001, "Pica / 10 cpi" )      // 10 characters per inch (standard character spacing)
	PORT_DIPSETTING(0x0000, "Compressed / 17 cpi") // 17 characters per inch

	PORT_DIPNAME(0xe000, 0xe000, "International Character Set" )  PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(0xe000, "US/UK/Netherlands" ) // off off off  US/UK/Netherlands (default all off)
	PORT_DIPSETTING(0xc000, "Switzerland" )       // on  on  off  Switzerland
	PORT_DIPSETTING(0xa000, "Italy" )             // on  off on   Italy
	PORT_DIPSETTING(0x8000, DEF_STR(Unused))      // on  off off  Unused
	PORT_DIPSETTING(0x6000, "France/Belgium" )    // off on  on   France/Belgium
	PORT_DIPSETTING(0x4000, "Sweden/Finland" )    // off on  off  Sweden/Finland
	PORT_DIPSETTING(0x2000, "Denmark/Norway" )    // off off on   Denmark/Norway
	PORT_DIPSETTING(0x0000, "Spain" )             // on  on  on   Spain
	PORT_DIPNAME(0x1000, 0x1000, DEF_STR(Unused))                 PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x1000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_BIT(0x0f00, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(mps1250)  // all DIP switches correct as per manual
	PORT_START("SW")               // all off = acts like Commodore VIC-1525 or MPS-803
	PORT_DIPNAME(0x0080, 0x0080, "Interface" )                    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x0080, "Commodore Serial IEC" ) // use with VIC20/C64/C128
	PORT_DIPSETTING(0x0000, "Parallel" )             // use with PC or Amiga

// when sw1:1 is off (IEC mode) these settings apply....
	PORT_DIPNAME(0x0040, 0x0040, "ASCII Translation" )            PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0040, "PET ASCII" )
	PORT_DIPSETTING(0x0000, "ASCII" ) // access to full ascii character set
	PORT_DIPNAME(0x0020, 0x0020, "Control Code Mode" )            PORT_DIPLOCATION("SW1:3") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0020, "Commodore" )
	PORT_DIPSETTING(0x0000, "Epson FX" ) // access to escape control codes when using Epson printer driver (software-specific use)
	PORT_DIPNAME(0x0010, 0x0010, "Print Quality" )                PORT_DIPLOCATION("SW1:4") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0010, "Draft" )
	PORT_DIPSETTING(0x0000, "NLQ" )
	PORT_DIPNAME(0x0008, 0x0008, "Device Number" )                PORT_DIPLOCATION("SW1:5") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0008, "4" )
	PORT_DIPSETTING(0x0000, "5" )
	PORT_DIPNAME(0x0004, 0x0000, "Paper End Detect" )             PORT_DIPLOCATION("SW1:6") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0004, "Enabled" )  // when enabled printer stops printing 2 inches before end of page and beeps to change paper
	PORT_DIPSETTING(0x0000, "Disabled" ) // so generally leave this disabled (ON) to allow printing closer to the end of a page
	PORT_DIPNAME(0x0002, 0x0002, "Automatic Line Feed" )          PORT_DIPLOCATION("SW1:7") PORT_CONDITION("SW", 0x80, EQUALS, 0x80) // only does something if sw1:3 is on, otherwise ignored
	PORT_DIPSETTING(0x0002, "Disabled" )
	PORT_DIPSETTING(0x0000, "Enabled" ) // adds a line feed to each carriage return received
	PORT_DIPNAME(0x0001, 0x0001, "Character Spacing" )            PORT_DIPLOCATION("SW1:8") PORT_CONDITION("SW", 0x80, EQUALS, 0x80)
	PORT_DIPSETTING(0x0001, "Pica / 10 cpi" )      // 10 characters per inch (standard character spacing)
	PORT_DIPSETTING(0x0000, "Compressed / 17 cpi") // 17 characters per inch

// when sw1:1 is on (parallel mode) these settings apply....
	PORT_DIPNAME(0x0040, 0x0040, "Automatic Line Feed" )          PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(0x0040, "Disabled" )
	PORT_DIPSETTING(0x0000, "Enabled" ) // adds a line feed (LF) to each carriage return received
	PORT_DIPNAME(0x0030, 0x0030, "Printer Configuration" )        PORT_DIPLOCATION("SW1:3,4") PORT_CONDITION("SW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(0x0030, "Epson #1" )             // off off (best for use with Amiga or PC and Epson printer driver)
	PORT_DIPSETTING(0x0020, "Epson #2" )             // on  off (gives access to full list of international character sets)
	PORT_DIPSETTING(0x0010, "Epson #3" )             // off on  (gives access to partial list of international character sets and other options)
	PORT_DIPSETTING(0x0000, "IBM Graphics Printer" ) // on  on  (for use with IBM-compatible PC)

// when Epson #1 is selected these settings apply....
	PORT_DIPNAME(0x0008, 0x0008, "ASCII Codes 128-159" )          PORT_DIPLOCATION("SW1:5") PORT_CONDITION("SW", 0xb0, EQUALS, 0x30)
	PORT_DIPSETTING(0x0008, "High-bit Control Codes" )  // standard characters
	PORT_DIPSETTING(0x0000, "Line and Block Graphics" ) // graphics characters
	PORT_DIPNAME(0x0004, 0x0004, "Zero" )                         PORT_DIPLOCATION("SW1:6") PORT_CONDITION("SW", 0xb0, EQUALS, 0x30)
	PORT_DIPSETTING(0x0004, "Not Slashed" )
	PORT_DIPSETTING(0x0000, "Slashed" )
	PORT_DIPNAME(0x0002, 0x0002, "Print Quality" )                PORT_DIPLOCATION("SW1:7")  PORT_CONDITION("SW", 0xb0, EQUALS, 0x30) // only active if sw1:3 is on
	PORT_DIPSETTING(0x0002, "Draft" )
	PORT_DIPSETTING(0x0000, "NLQ" )
	PORT_DIPNAME(0x0001, 0x0001, "Character Spacing" )            PORT_DIPLOCATION("SW1:8") PORT_CONDITION("SW", 0xb0, EQUALS, 0x30)
	PORT_DIPSETTING(0x0001, "Pica / 10 cpi" )      // 10 characters per inch (standard character spacing)
	PORT_DIPSETTING(0x0000, "Compressed / 17 cpi") // 17 characters per inch

// when Epson #2 is selected these settings apply....
	PORT_DIPNAME(0x000e, 0x000e, "International Character Set" )  PORT_DIPLOCATION("SW1:5,6,7") PORT_CONDITION("SW", 0xb0, EQUALS, 0x20)
	PORT_DIPSETTING(0x000e, "USA" )     // off off off  USA
	PORT_DIPSETTING(0x000c, "England" ) // on  on  off  England
	PORT_DIPSETTING(0x000a, "Sweden" )  // on  off on   Sweden
	PORT_DIPSETTING(0x0008, "France" )  // on  off off  Unused
	PORT_DIPSETTING(0x0006, "Italy" )   // off on  on   Italy
	PORT_DIPSETTING(0x0004, "Germany" ) // off on  off  Germany
	PORT_DIPSETTING(0x0002, "Denmark" ) // off off on   Denmark
	PORT_DIPSETTING(0x0000, "Spain" )   // on  on  on   Spain
	PORT_DIPNAME(0x0001, 0x0001, "Page Length" )                  PORT_DIPLOCATION("SW1:8") PORT_CONDITION("SW", 0xb0, EQUALS, 0x20)
	PORT_DIPSETTING(0x0001, "11 inch" )
	PORT_DIPSETTING(0x0000, "12 inch" )

// when Epson #3 is selected these settings apply....
	PORT_DIPNAME(0x0008, 0x0008, "ASCII Codes 128-159" )          PORT_DIPLOCATION("SW1:5") PORT_CONDITION("SW", 0xb0, EQUALS, 0x10)
	PORT_DIPSETTING(0x0008, "High-bit Control Codes" )   // standard characters
	PORT_DIPSETTING(0x0000, "Line and Block Graphics" )  // graphics characters
	PORT_DIPNAME(0x0006, 0x0006, "International Character Set" )  PORT_DIPLOCATION("SW1:6,7") PORT_CONDITION("SW", 0xb0, EQUALS, 0x10)
	PORT_DIPSETTING(0x0006, "USA" )     // off off  USA
	PORT_DIPSETTING(0x0004, "France" )  // on  off  France
	PORT_DIPSETTING(0x0002, "Germany" ) // off on   Germany
	PORT_DIPSETTING(0x0000, "England" ) // on  on   England
	PORT_DIPNAME(0x0001, 0x0001, "Character Spacing" )            PORT_DIPLOCATION("SW1:8") PORT_CONDITION("SW", 0xb0, EQUALS, 0x10)
	PORT_DIPSETTING(0x0001, "Pica / 10 cpi" )      // 10 characters per inch (standard character spacing)
	PORT_DIPSETTING(0x0000, "Compressed / 17 cpi") // 17 characters per inch

// when IBM Graphics Printer #1 is selected these settings apply....
	PORT_DIPNAME(0x0008, 0x0008, "ASCII Codes 128-159" )          PORT_DIPLOCATION("SW1:5") PORT_CONDITION("SW", 0xb0, EQUALS, 0x00)
	PORT_DIPSETTING(0x0008, "High-bit Control Codes" ) // IBM-graphics set 1
	PORT_DIPSETTING(0x0000, "Accented Characters" )    // IBM-graphics set 2
	PORT_DIPNAME(0x0004, 0x0004, "Line Spacing" )                 PORT_DIPLOCATION("SW1:6") PORT_CONDITION("SW", 0xb0, EQUALS, 0x00)
	PORT_DIPSETTING(0x0004, "1/6 inch" )
	PORT_DIPSETTING(0x0000, "1/8 inch" )
	PORT_DIPNAME(0x0002, 0x0002, "Auto Carriage Return" )         PORT_DIPLOCATION("SW1:7") PORT_CONDITION("SW", 0xb0, EQUALS, 0x00)
	PORT_DIPSETTING(0x0002, "Enabled" ) // carriage return (CR) is inserted when printer receives a line feed
	PORT_DIPSETTING(0x0000, "Disabled" )
	PORT_DIPNAME(0x0001, 0x0001, "Buffer-Full Printing" )         PORT_DIPLOCATION("SW1:8") PORT_CONDITION("SW", 0xb0, EQUALS, 0x00)
	PORT_DIPSETTING(0x0001, "Enabled" )  // Determines how the printer acts when receiving more characters than will fit on one line without a
	PORT_DIPSETTING(0x0000, "Disabled" ) // carriage return received. 'Enabled' will insert a line feed at the right hard margin and 'disabled'
										 //  will return the carriage to the left margin and the remaining characters will overwrite the line.
	PORT_BIT(0x0f00, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor mps1200_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps1200);
}

ioport_constructor mps1250_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps1250);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START(mps1200)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("mps1200-k405-0202.bin", 0x00000, 0x10000, CRC(87aa884a) SHA1(0ceb753c17599bc69458cfbb1cb3e81c2b60d107)) // "VER 1.01" "JUL-24-86" "Y8306 COMMODORE B.I.P."
ROM_END

ROM_START(mps1250)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("mps1250_k111_0201.bin", 0x00000, 0x10000, CRC(f2de9b69) SHA1(bb7357e83497b333e3f95548d94970003b2dfa9d)) // "VER 1.34" "MAR-03-87" "Y8307 COMMODORE DUAL  B.I.P."
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1200_device::device_rom_region() const
{
	return ROM_NAME(mps1200);
}

const tiny_rom_entry *mps1250_device::device_rom_region() const
{
	return ROM_NAME(mps1250);
}
