// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for first-generation C. Itoh video terminals.

CIT-101 (released December 1980)
    C. Itoh's first terminal, based on DEC VT100. ANSI X3.64 and V52 compatible.
    12-inch monochrome screen displaying 24 lines of 80 or 132 characters.
    8 x 10 character cell, 10 x 10 (80 columns)/9 x 10 (132 columns) display cell.
    15,600 Hz horizontal frequency; 50 Hz/60 Hz vertical frequency selectable.
    Cursor may be selected as blinking or solid block/underline, or invisible.
    7 or 8 bit ASCII characters.
    RS232-C or 20 mA current loop communications and auxiliary (printer) ports.
    85-key detachable keyboard with 7 LEDs and settable key click.
CIT-80 (released September 1981)
    "Entry-level version" of CIT-101.
    12-inch monochrome screen displaying 24 lines of 80 characters.
    7-bit characters only.
CIT-161 (released 1982)
    Colorized version of the CIT-101.
    12-inch color screen displaying 24 lines of 80 or 132 characters.
    64 combinations of 8 colors are programmable.
CIT-500 (released 1982)
    Word processing terminal with full page display.
    15-inch vertically oriented monochrome screen with tilt/swivel.
    64 lines of 80 characters (interlaced).
    105-key keyboard.
CIT-101e (released 1983)
    Ergonomic redesign of CIT-101.
    Competitive with DEC VT220 (which was released several months later).
    14-inch monochrome screen with tilt/swivel, 24 lines of 80 or 132 characters.
    85-key low-profile keyboard.
CIG-201
    Plug-in graphics card for CIT-101 and CIT-101e.
    Compatible with Tektronix 4010/4014.
CIG-261
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4010/4014.
CIG-267
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4027A.

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/er2055.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "screen.h"


class cit101_state : public driver_device
{
public:
	cit101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_nvr(*this, "nvr")
		, m_chargen(*this, "chargen")
	{ }

	void cit101(machine_config &config);
private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(nvr_address_w);
	DECLARE_READ8_MEMBER(nvr_data_r);
	DECLARE_WRITE8_MEMBER(nvr_data_w);
	DECLARE_WRITE8_MEMBER(nvr_control_w);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<er2055_device> m_nvr;
	required_region_ptr<u8> m_chargen;
};


u32 cit101_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


WRITE8_MEMBER(cit101_state::nvr_address_w)
{
	m_nvr->set_address(data & 0x3f);
	m_nvr->set_clk(BIT(data, 6));
}

READ8_MEMBER(cit101_state::nvr_data_r)
{
	return m_nvr->data();
}

WRITE8_MEMBER(cit101_state::nvr_data_w)
{
	m_nvr->set_data(data);
}

WRITE8_MEMBER(cit101_state::nvr_control_w)
{
	m_nvr->set_control(BIT(data, 5), !BIT(data, 4), BIT(data, 7), BIT(data, 6));
}

void cit101_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0xbfff).ram();
	map(0xc000, 0xcfff).ram().share("videoram");
	map(0xfc00, 0xfc00).rw("usart0", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc01, 0xfc01).rw("usart0", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc20, 0xfc20).rw("usart1", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc21, 0xfc21).rw("usart1", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc40, 0xfc40).rw("usart2", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xfc41, 0xfc41).rw("usart2", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xfc60, 0xfc63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfc80, 0xfc83).w("pit0", FUNC(pit8253_device::write));
	map(0xfcc0, 0xfcc3).w("pit1", FUNC(pit8253_device::write));
}

void cit101_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw("usart0", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x01, 0x01).rw("usart0", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x20, 0x20).rw("usart1", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x21, 0x21).rw("usart1", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x40, 0x40).rw("usart2", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x41, 0x41).rw("usart2", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa0).nopw(); // ?
	map(0xe0, 0xe0).noprw(); // ?
}


static INPUT_PORTS_START( cit101 )
INPUT_PORTS_END


MACHINE_CONFIG_START(cit101_state::cit101)
	MCFG_CPU_ADD("maincpu", I8085A, 6.144_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(14.976_MHz_XTAL, 960, 0, 800, 260, 0, 240)
	//MCFG_SCREEN_RAW_PARAMS(22.464_MHz_XTAL, 1440, 0, 1188, 260, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(cit101_state, screen_update)

	MCFG_DEVICE_ADD("usart0", I8251, 6.144_MHz_XTAL / 2) // M5L8251AP-5
	MCFG_DEVICE_ADD("usart1", I8251, 6.144_MHz_XTAL / 2) // M5L8251AP-5
	MCFG_DEVICE_ADD("usart2", I8251, 6.144_MHz_XTAL / 2) // M5L8251AP-5

	//MCFG_RS232_PORT_ADD("comm", default_rs232_devices, nullptr)
	//MCFG_RS232_PORT_ADD("printer", default_rs232_devices, nullptr)

	MCFG_DEVICE_ADD("pit0", PIT8253, 0) // NEC D8253C-2
	MCFG_DEVICE_ADD("pit1", PIT8253, 0) // NEC D8253C-2

	MCFG_DEVICE_ADD("ppi", I8255A, 0) // NEC D8255AC-2
	MCFG_I8255_OUT_PORTA_CB(WRITE8(cit101_state, nvr_address_w))
	MCFG_I8255_IN_PORTB_CB(READ8(cit101_state, nvr_data_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(cit101_state, nvr_data_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(cit101_state, nvr_control_w))

	MCFG_DEVICE_ADD("nvr", ER2055, 0)
MACHINE_CONFIG_END


ROM_START( cit101 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "IC1_(3G04)_V10A.BIN", 0x0000, 0x1000, CRC(5601fcac) SHA1(cad0d0335d133dd43993bc718ff72d12b8445cd1) )
	ROM_LOAD( "IC2_(3H04)_V10A.BIN", 0x1000, 0x1000, CRC(23d263e0) SHA1(586e8185f9804987e0a4081724c060e74769d41d) )
	ROM_LOAD( "IC3_(3I04)_V10A.BIN", 0x2000, 0x1000, CRC(15994b1d) SHA1(6d125db4ef5e1dd4d5a4d2f4d6f6bdf574e5bad8) )
	ROM_LOAD( "IC4_(3J04)_V10A.BIN", 0x3000, 0x1000, CRC(274b9df0) SHA1(d0006258cbd56f6f6bb53906fb618ebe379f6563) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "1H (5H 1 02) Char ROM.BIN", 0x0000, 0x1000, CRC(ee0ff889) SHA1(a74ada19d19041b29e1b49aaf57ba7d9d54575e1) )
ROM_END

COMP( 1980, cit101, 0, 0, cit101, cit101, cit101_state, 0, "C. Itoh Electronics", "CIT-101", MACHINE_IS_SKELETON )
