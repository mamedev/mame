// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-27 Skeleton

Wyse terminals.

WY-30+: P8031AH, OKI M76V020, TC5565APL-12, HY6116AP-15, Beeper, 31.2795, 7.3728 (for CPU)

WY-50: SAB8031P, SCN2672A (CRTC), SCN2661B (UART), 2x MSM2128-15RS, SY2158A, 80-435-00 (WYSE proprietory gate array),
       Beeper, 4.9152 (for UART), 11.000 (for CPU), 68.850 (for video).

WY-160: ?

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
	MCFG_CPU_ADD("maincpu", I8031, 11'000'000) // confirmed for WY-50
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
MACHINE_CONFIG_END

ROM_START( wy50 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2301_E.u6", 0x0000, 0x2000, CRC(2a62ea25) SHA1(f69c596aab307ef1872df29d353b5a61ff77bb74) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2201_B.u16", 0x0000, 0x1000, CRC(ee318814) SHA1(0ac64b60ff978e607a087e9e6f4d547811c015c5) )
ROM_END

ROM_START( wy30p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "250971-02.u4", 0x0000, 0x4000, CRC(3666549c) SHA1(23c432da2083df4b355daf566dd6514d1f9a7690) )
ROM_END

ROM_START( wy160 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // cpu not identified
	ROM_LOAD( "251167-06.bin", 0x00000, 0x010000, CRC(36e920df) SHA1(8fb7f51b4f47ef63b21d421227d6fef98001e4e9) )
ROM_END

COMP( 1984, wy50,  0,    0, wyse, wyse, wyse_state, 0, "Wyse", "WY-50",  MACHINE_IS_SKELETON )
COMP( 1990, wy160, wy50, 0, wyse, wyse, wyse_state, 0, "Wyse", "WY-160", MACHINE_IS_SKELETON )
COMP( 1991, wy30p, wy50, 0, wyse, wyse, wyse_state, 0, "Wyse", "WY-30+", MACHINE_IS_SKELETON )
