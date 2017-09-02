// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  ds.cpp

  Skeleton driver for first-generation Nintendo DS.

***************************************************************************/

#include "emu.h"
#include "includes/nds.h"

#define VERBOSE_LEVEL   (0)

static inline void ATTR_PRINTF(3,4) verboselog(device_t &device, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%08x: %s", device.machine().describe_context(), buf );
	}
}

static ADDRESS_MAP_START( nds_arm9_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000000, 0x00000fff) AM_ROM
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_SHARE("mainram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( nds_arm7_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x03800000, 0x0380ffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( nds )
INPUT_PORTS_END


void nds_state::machine_reset()
{
}

void nds_state::machine_start()
{
}

static MACHINE_CONFIG_START( nds )

	MCFG_CPU_ADD("arm9", ARM946ES, XTAL_66_6667MHz)
	MCFG_CPU_PROGRAM_MAP(nds_arm9_map)

	MCFG_CPU_ADD("arm7", ARM7, XTAL_33_333MHz)
	MCFG_CPU_PROGRAM_MAP(nds_arm7_map)
MACHINE_CONFIG_END

/* Help identifying the region and revisions of the set would be greatly appreciated! */
ROM_START( nds )
	ROM_REGION( 0x1000, "arm9", 0 )
	ROM_LOAD( "biosnds9.rom", 0x0000, 0x1000, CRC(12345678) SHA1(1234567812345678123456781234567812345678) )

	ROM_REGION( 0x4000, "arm7", 0 )
	ROM_LOAD( "biosnds7.rom", 0x0000, 0x4000, CRC(87654321) SHA1(8765432187654321876543218765432187654321) )

	ROM_REGION32_LE( 0x40000, "firmware", 0 )
	ROM_LOAD( "firmware.bin", 0x0000, 0x40000, CRC(55aa6699) SHA1(55aa669955aa669955aa669955aa669955aa6699) )
ROM_END

//   YEAR  NAME PARENT COMPAT MACHINE INPUT  STATE      INIT  COMPANY     FULLNAME            FLAGS
CONS(2004, nds, 0,     0,     nds,    nds,   nds_state, 0,    "Nintendo", "DS",               MACHINE_IS_SKELETON)
