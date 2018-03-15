// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for first-generation C. Itoh video terminals.

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
//#include "machine/er2055.h"
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
		, m_chargen(*this, "chargen")
	{ }

	void cit101(machine_config &config);
private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_chargen;
};


u32 cit101_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
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
	map(0xfc60, 0xfc60).noprw(); // ER2055 data?
	map(0xfc80, 0xfc83).w("pit0", FUNC(pit8253_device::write));
	map(0xfcc0, 0xfcc3).w("pit1", FUNC(pit8253_device::write));
}

void cit101_state::io_map(address_map &map)
{
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
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

	//MCFG_DEVICE_ADD("nvr", ER2055, 0)
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

COMP( 1981, cit101, 0, 0, cit101, cit101, cit101_state, 0, "C. Itoh", "CIT-101", MACHINE_IS_SKELETON )
