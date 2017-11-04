// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton

Wyse is a well-known maker of terminals. It is presumed this is one of them.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

class wyse_state : public driver_device
{
public:
	wyse_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

private:
//	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, wyse_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, wyse_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( wyse )
INPUT_PORTS_END

static MACHINE_CONFIG_START( wyse )
	MCFG_CPU_ADD("maincpu", I80C32, XTAL_16MHz) // no idea of clock
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
MACHINE_CONFIG_END

ROM_START( wyse )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wyse_2301e.bin", 0x0000, 0x2000, CRC(2a62ea25) SHA1(f69c596aab307ef1872df29d353b5a61ff77bb74) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "wyse_2201b.bin", 0x0000, 0x1000, CRC(ee318814) SHA1(0ac64b60ff978e607a087e9e6f4d547811c015c5) )
ROM_END

ROM_START( wyse160 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // cpu not identified
	ROM_LOAD( "wyse_251167-06.bin", 0x00000, 0x010000, CRC(36e920df) SHA1(8fb7f51b4f47ef63b21d421227d6fef98001e4e9) )
ROM_END

COMP( 1984, wyse,    0,    0, wyse, wyse, wyse_state, 0, "Wyse", "unknown Wyse terminal", MACHINE_IS_SKELETON )
COMP( 1990, wyse160, wyse, 0, wyse, wyse, wyse_state, 0, "Wyse", "WY-160", MACHINE_IS_SKELETON )
