// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for ADDS Viewpoint 60 terminal.
No significant progress can be made until the 8051 has its internal ROM dumped.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/er2055.h"
//#include "video/i8275.h"
#include "screen.h"

class vp60_state : public driver_device
{
public:
	vp60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vp60(machine_config &config);
	void io_map(address_map &map);
	void kbd_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

u32 vp60_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vp60_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
}

void vp60_state::io_map(address_map &map)
{
	map(0x8000, 0x87ff).ram();
}

void vp60_state::kbd_map(address_map &map)
{
	map(0x000, 0x3ff).rom().region("keyboard", 0);
}

static INPUT_PORTS_START( vp60 )
INPUT_PORTS_END

MACHINE_CONFIG_START(vp60_state::vp60)
	MCFG_DEVICE_ADD("maincpu", I8051, XTAL(10'920'000))
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'920'000), 1350, 0, 1056, 320, 0, 300) // dimensions guessed
	MCFG_SCREEN_UPDATE_DRIVER(vp60_state, screen_update)

	MCFG_DEVICE_ADD("kbdcpu", I8035, XTAL(3'579'545)) // 48-300-010 XTAL
	MCFG_DEVICE_PROGRAM_MAP(kbd_map)
MACHINE_CONFIG_END


/**************************************************************************************************************

ADDS Viewpoint 60.
Chips: P8051, P8275, EAROM ER-2055, HM6116P-4
Crystals: 25.92, 10.920
Keyboard: INS8035N-6, crystal marked 48-300-010.

***************************************************************************************************************/

ROM_START( vp60 )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "p8051.ub1",  0x0000, 0x1000, NO_DUMP ) // internal ROM not dumped
	ROM_LOAD( "pgm.uc1",    0x2000, 0x1000, CRC(714ca569) SHA1(405424369fd5458e02c845c104b2cb386bd857d2) )
	ROM_CONTINUE(           0x1000, 0x1000 )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "font.uc4",   0x0000, 0x1000, CRC(3c4d39c0) SHA1(9503c0d5a76e8073c94c86be57bcb312641f6cc4) )

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD( "195.kbd",    0x0000, 0x0400, CRC(14885da3) SHA1(3b06f658af1a62b28e62d8b3a557b74169917a12) )
ROM_END

COMP( 1982, vp60, 0, 0, vp60, vp60, vp60_state, empty_init, "ADDS", "Viewpoint 60", MACHINE_IS_SKELETON )
