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
//#include "screen.h"

class vp60_state : public driver_device
{
public:
	vp60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, vp60_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_PROGRAM, 8, vp60_state )
	AM_RANGE(0x8000, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kbd_map, AS_PROGRAM, 8, vp60_state )
	AM_RANGE(0x000, 0x3ff) AM_ROM AM_REGION("keyboard", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( vp60 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( vp60 )
	MCFG_CPU_ADD("maincpu", I8051, XTAL_10_920MHz)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_CPU_ADD("kbdcpu", I8035, 5000000)
	MCFG_CPU_PROGRAM_MAP(kbd_map)
MACHINE_CONFIG_END


/**************************************************************************************************************

ADDS Viewpoint 60.
Chips: P8051, P8275, EAROM ER-2055, HM6116P-4
Crystals: 25.92, 10.920
Keyboard: INS8035N-6, unknown crystal marked 48-300-010.

***************************************************************************************************************/

ROM_START( vp60 )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "p8051.ub1",  0x0000, 0x2000, NO_DUMP ) // internal ROM not dumped
	ROM_LOAD( "pgm.uc1",    0x2000, 0x2000, CRC(714ca569) SHA1(405424369fd5458e02c845c104b2cb386bd857d2) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "font.uc4",   0x0000, 0x1000, CRC(3c4d39c0) SHA1(9503c0d5a76e8073c94c86be57bcb312641f6cc4) )

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD( "195.kbd",    0x0000, 0x0400, CRC(14885da3) SHA1(3b06f658af1a62b28e62d8b3a557b74169917a12) )
ROM_END

COMP( 1982, vp60, 0, 0, vp60, vp60, vp60_state, 0, "ADDS", "Viewpoint 60", MACHINE_IS_SKELETON )
