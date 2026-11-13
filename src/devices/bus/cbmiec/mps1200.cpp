// license:BSD-3-Clause
// copyright-holders:AJR,Curt Coder
/**********************************************************************

    Commodore MPS-1200 & MPS-1250 printers

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

#define LOG_PORTS (1U << 1)
#define LOG_IEC   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)
#define LOGIEC(...)   LOGMASKED(LOG_IEC, __VA_ARGS__)


static constexpr attotime EOI_TIMEOUT = attotime::from_usec(256);



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MPS1200, mps1200_device, "mps1200", "Commodore MPS-1200 Dot Matrix Printer")
DEFINE_DEVICE_TYPE(MPS1250, mps1250_device, "mps1250", "Commodore MPS-1250 Dot Matrix Printer")


//-------------------------------------------------
//  ROM( mps1200 )
//-------------------------------------------------

ROM_START( mps1200 )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_LOAD( "mps1200-k405-0202.bin", 0x00000, 0x10000, CRC(87aa884a) SHA1(0ceb753c17599bc69458cfbb1cb3e81c2b60d107) ) // "VER 1.01" "JUL-24-86" "Y8306 COMMODORE B.I.P."
ROM_END


//-------------------------------------------------
//  ROM( mps1250 )
//-------------------------------------------------

ROM_START( mps1250 )
	ROM_REGION( 0x10000, "firmware", 0 )
	ROM_LOAD( "mps1250_k111_0201.bin", 0x00000, 0x10000, CRC(f2de9b69) SHA1(bb7357e83497b333e3f95548d94970003b2dfa9d) ) // "VER 1.34" "MAR-03-87" "Y8307 COMMODORE DUAL  B.I.P."
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1200_device::device_rom_region() const
{
	return ROM_NAME( mps1200 );
}

const tiny_rom_entry *mps1250_device::device_rom_region() const
{
	return ROM_NAME( mps1250 );
}


//-------------------------------------------------
//  mem_map - program memory space
//-------------------------------------------------

void mps1200_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram");
	map(0x8000, 0xffff).rom().region("firmware", 0x8000);
}


//-------------------------------------------------
//  data_map - DME memory space
//-------------------------------------------------

void mps1200_device::data_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram");
	map(0x8000, 0xffff).rom().region("firmware", 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mps1200_device::device_add_mconfig(machine_config &config)
{
	M50734(config, m_mpscpu, 8_MHz_XTAL);
	m_mpscpu->set_addrmap(AS_PROGRAM, &mps1200_device::mem_map);
	m_mpscpu->set_addrmap(AS_DATA, &mps1200_device::data_map);
	m_mpscpu->p0_in_cb().set(FUNC(mps1200_device::p0_r));
	m_mpscpu->p0_out_cb().set(FUNC(mps1200_device::p0_w));
	m_mpscpu->p1_in_cb().set(FUNC(mps1200_device::p1_r));
	m_mpscpu->p2_in_cb().set(FUNC(mps1200_device::p2_r));
	m_mpscpu->p2_out_cb().set(FUNC(mps1200_device::p2_w));
	m_mpscpu->p3_out_cb().set(FUNC(mps1200_device::p3_w));
	m_mpscpu->p4_in_cb().set(FUNC(mps1200_device::p4_r));
	m_mpscpu->an0_in_cb().set(FUNC(mps1200_device::hsen_r));
	m_mpscpu->an1_in_cb().set(FUNC(mps1200_device::vmon_r));
	m_mpscpu->an2_in_cb().set(FUNC(mps1200_device::hthm_r));
	m_mpscpu->sclk_cb().set(FUNC(mps1200_device::dsck_w));
	m_mpscpu->sio_in_cb().set(FUNC(mps1200_device::dsdt_r));
	m_mpscpu->sio_out_cb().set(FUNC(mps1200_device::sio_out_w));

	CITIZEN_120D(config, m_mech, 0);
}


//-------------------------------------------------
//  INPUT_PORTS( mps1200_panel )
//-------------------------------------------------

static INPUT_PORTS_START( mps1200_panel )
	PORT_START("PANEL")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ON LINE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LINE FEED")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FORM FEED")
	PORT_BIT(0xe3, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

//-------------------------------------------------
//  INPUT_PORTS( mps1200 )
//-------------------------------------------------

static INPUT_PORTS_START( mps1200 )
	PORT_INCLUDE( mps1200_panel )

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

//-------------------------------------------------
//  INPUT_PORTS( mps1250 )
//-------------------------------------------------

static INPUT_PORTS_START( mps1250 )  // all DIP switches correct as per manual
	PORT_INCLUDE( mps1200_panel )

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
	return INPUT_PORTS_NAME( mps1200 );
}

ioport_constructor mps1250_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mps1250 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mps1200_device - constructor
//-------------------------------------------------

mps1200_device::mps1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_mpscpu(*this, "mpscpu"),
	m_sw(*this, "SW"),
	m_panel(*this, "PANEL"),
	m_mech(*this, "mech")
{
}

mps1200_device::mps1200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mps1200_device(mconfig, MPS1200, tag, owner, clock)
{
}


//-------------------------------------------------
//  mps1250_device - constructor
//-------------------------------------------------

mps1250_device::mps1250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mps1200_device(mconfig, MPS1250, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1200_device::device_start()
{
	m_eoi_timer = timer_alloc(FUNC(mps1200_device::eoi_timeout), this);

	save_item(NAME(m_dip_shift));
	save_item(NAME(m_dip_load));
	save_item(NAME(m_shift));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_byte_ready));
	save_item(NAME(m_eoi));
	save_item(NAME(m_data_out));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_mech_shift));
	save_item(NAME(m_p3_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mps1200_device::device_reset()
{
	m_bit_count = 0;
	m_byte_ready = false;
	m_eoi = false;
	m_data_out = false;
	m_clk_state = m_bus != nullptr ? m_bus->clk_r() : 1;
	m_eoi_timer->adjust(attotime::never);

	update_int1();
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void mps1200_device::cbm_iec_atn(int state)
{
	int const atn = m_bus->atn_r();
	LOGIEC("ATN %d\n", atn);

	m_mpscpu->set_input_line(m50734_device::M50734_INT2_LINE, atn ? CLEAR_LINE : ASSERT_LINE);

	if (!atn)
	{
		m_eoi_timer->adjust(attotime::never);
		m_bit_count = 0;
		if (m_eoi)
		{
			m_eoi = false;
			update_int1();
		}
	}
}


//-------------------------------------------------
//  cbm_iec_clk -
//-------------------------------------------------

void mps1200_device::cbm_iec_clk(int state)
{
	int const clk = m_bus->clk_r();
	if (clk == m_clk_state)
		return;
	m_clk_state = clk;

	m_mpscpu->set_input_line(m50734_device::M50734_CNTR_LINE, clk ? ASSERT_LINE : CLEAR_LINE);

	if (!clk)
	{
		m_eoi_timer->adjust(attotime::never);
		if (m_eoi)
		{
			m_eoi = false;
			update_int1();
		}
		return;
	}

	if (m_byte_ready)
	{
		m_byte_ready = false;
		update_int1();
	}

	m_shift = m_shift >> 1 | (m_bus->data_r() ? 0x00 : 0x80);

	if (++m_bit_count == 9)
	{
		m_bit_count = 0;
		m_byte_ready = true;
		LOGIEC("byte received %02X\n", m_shift ^ 0xff);
		update_int1();
	}
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void mps1200_device::cbm_iec_reset(int state)
{
	m_mpscpu->set_input_line(INPUT_LINE_RESET, m_bus->reset_r() ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  update_int1 - IC10's byte strobe and IC12's
//  EOI timeout are wired-OR onto P00
//-------------------------------------------------

void mps1200_device::update_int1()
{
	m_mpscpu->set_input_line(m50734_device::M50734_INT1_LINE, (m_byte_ready || m_eoi) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  eoi_timeout - IC12 reached Q10
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mps1200_device::eoi_timeout)
{
	LOGIEC("EOI timeout\n");

	m_eoi = true;
	update_int1();
}


//-------------------------------------------------
//  p0_r -
//-------------------------------------------------

u8 mps1200_device::p0_r()
{
	/*

		bit		description

		P00		_INT1
		P01		_INT2 (ATN)
		P02		CNTR (CLOCK)
		P03
		P04
		P05
		P06
		P07		_RESET

	*/

	if (m_bus == nullptr)
		return 0xff;

	u8 data = 0x78;

	data |= !(m_byte_ready || m_eoi);
	data |= m_bus->atn_r() << 1;
	data |= m_bus->clk_r() << 2;
	data |= m_bus->reset_r() << 7;

	return data;
}


//-------------------------------------------------
//  p0_w -
//-------------------------------------------------

void mps1200_device::p0_w(u8 data)
{
	/*

		bit		description

		P00
		P01
		P02
		P03		BUSYOUT (DATA)
		P04		_HDEN
		P05		DME
		P06
		P07

	*/

	bool const data_low = BIT(data, 3);
	if (data_low != m_data_out)
	{
		m_data_out = data_low;

		if (data_low)
		{
			m_eoi_timer->adjust(attotime::never);
			if (m_eoi)
			{
				m_eoi = false;
				update_int1();
			}
		}
		else if (m_clk_state && m_bit_count == 1 && m_bus != nullptr && m_bus->atn_r())
			m_eoi_timer->adjust(EOI_TIMEOUT);
	}

	if (m_bus != nullptr)
		m_bus->data_w(this, data_low ? 0 : 1);

	m_mech->head_en_w(BIT(data, 4));
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

u8 mps1200_device::p1_r()
{
	return m_shift;
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

u8 mps1200_device::p2_r()
{
	/*

		bit		description

		P20
		P21
		P22		OLSW
		P23		LFSW
		P24		FFSW
		P25
		P26
		P27		_CONT

	*/

	LOGPORTS("%s: P2 read\n", machine().describe_context());

	return m_panel->read() | 0xe3;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void mps1200_device::p2_w(u8 data)
{
	/*

		bit		description

		P20		_CMVM
		P21
		P22		(OLSW in)
		P23		(LFSW in)
		P24		(FFSW in)
		P25		FLTL
		P26		RDYL
		P27

	*/

	LOGPORTS("%s: P2 = %02X\n", machine().describe_context(), data);

	m_mech->fault_led_w(BIT(data, 5));
	m_mech->ready_led_w(BIT(data, 6));
}


//-------------------------------------------------
//  p3_w -
//-------------------------------------------------

void mps1200_device::p3_w(u8 data)
{
	/*

		bit		description

		P30		_LOAD
		P31		DSCK
		P32		DSDT
		P33		HDLD
		P34		MDLD
		P35		_CVH1
		P36		_CVH2
		P37		_PMVH

	*/

	u8 const risen = data & ~m_p3_data;
	m_p3_data = data;
	if (BIT(risen, 3))
		m_mech->solenoid_w(m_mech_shift & 0x1ff);
	if (BIT(risen, 4))
		m_mech->phase_w(m_mech_shift & 0xff);

	m_dip_load = BIT(data, 0);
	if (!m_dip_load)
		return;

	u16 sw = m_sw->read();
	if (BIT(device_cbm_iec_interface::m_slot->get_address() - 4, 0))
		sw &= ~device_number_mask();
	else
		sw |= device_number_mask();

	m_dip_shift = 0;
	for (int i = 0; i < 8; i++)
		m_dip_shift |= u16(BIT(sw, i)) << (15 - i);
	for (int i = 0; i < 4; i++)
		m_dip_shift |= u16(BIT(sw, 12 + i)) << (3 - i);
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

u8 mps1200_device::p4_r()
{
	/*

		bit		description

		P40		AN0 (VMON)
		P41		AN1
		P42		AN2
		P43		byte received

	*/

	return 0x07 | m_byte_ready << 3;
}


//-------------------------------------------------
//  hsen_r - AN0, the carriage home sensor
//-------------------------------------------------

u8 mps1200_device::hsen_r()
{
	return m_mech->at_home() ? 0x20 : 0x80;
}


//-------------------------------------------------
//  vmon_r - AN1, the +24V rail monitor
//-------------------------------------------------

u8 mps1200_device::vmon_r()
{
	return 0x80;
}


//-------------------------------------------------
//  hthm_r - AN2, the print head thermistor
//-------------------------------------------------

u8 mps1200_device::hthm_r()
{
	return 0xe0;
}


//-------------------------------------------------
//  dsck_w - M50734 clocked serial shift clock
//-------------------------------------------------

void mps1200_device::dsck_w(int state)
{
	if (state && !m_dip_load)
		m_dip_shift <<= 1;
}


//-------------------------------------------------
//  dsdt_r - M50734 clocked serial data in
//-------------------------------------------------

int mps1200_device::dsdt_r()
{
	return BIT(m_dip_shift, 15);
}


//-------------------------------------------------
//  sio_out_w - M50734 clocked serial data out, to
//  the Power/Driver PCB's chained shift registers
//-------------------------------------------------

void mps1200_device::sio_out_w(int state)
{
	m_mech_shift = m_mech_shift << 1 | (state ? 1 : 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1250_device::device_start()
{
	mps1200_device::device_start();

	save_item(NAME(m_cent_data));
	save_item(NAME(m_cent_strobe));
	save_item(NAME(m_cent_byte));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mps1250_device::device_reset()
{
	m_cent_byte = false;
	m_cent_strobe = true;

	mps1200_device::device_reset();

	if (has_centronics())
	{
		output_perror(0);
		output_select(1);
		output_fault(1);
	}
}


//-------------------------------------------------
//  input_strobe - _DSTB from CN12
//-------------------------------------------------

void mps1250_device::input_strobe(int state)
{
	if (bool(state) == m_cent_strobe)
		return;
	m_cent_strobe = state;

	if (!ifsel())
		return;

	if (!state)
	{
		LOGIEC("parallel byte %02X\n", m_cent_data);
		m_cent_byte = true;
	}

	update_int1();
}


//-------------------------------------------------
//  input_init - _INIT from CN12
//-------------------------------------------------

void mps1250_device::input_init(int state)
{
	m_mpscpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

u8 mps1250_device::p1_r()
{
	if (!ifsel())
	{
		return ~mps1200_device::p1_r();
	}

	return m_cent_data;
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

u8 mps1250_device::p4_r()
{
	if (!ifsel())
		return mps1200_device::p4_r();

	return 0x07 | m_cent_byte << 3;
}


//-------------------------------------------------
//  p0_w -
//-------------------------------------------------

void mps1250_device::p0_w(u8 data)
{
	if (!ifsel())
	{
		mps1200_device::p0_w(data);
		return;
	}

	if (has_centronics())
	{
		output_busy(BIT(data, 3));
		output_ack(BIT(data, 6));
	}

	if (BIT(data, 3))
		m_cent_byte = false;

	m_mech->head_en_w(BIT(data, 4));
}


//-------------------------------------------------
//  update_int1 -
//-------------------------------------------------

void mps1250_device::update_int1()
{
	if (!ifsel())
	{
		mps1200_device::update_int1();
		return;
	}

	m_mpscpu->set_input_line(m50734_device::M50734_INT1_LINE, m_cent_strobe ? CLEAR_LINE : ASSERT_LINE);
}
