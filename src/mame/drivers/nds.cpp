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

READ32_MEMBER(nds_state::arm7_io_r)
{
	switch(offset)
	{
		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			return m_arm7_postflg;
		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

WRITE32_MEMBER(nds_state::arm7_io_w)
{
	switch(offset)
	{
		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			if (!(m_arm7_postflg & POSTFLG_PBF_MASK) && m_arm7->pc() < 0x4000)
			{
				m_arm7_postflg &= ~POSTFLG_PBF_MASK;
				m_arm7_postflg |= data & POSTFLG_PBF_MASK;
			}
			break;
		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

READ32_MEMBER(nds_state::arm9_io_r)
{
	switch(offset)
	{
		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			return m_arm9_postflg;
		default:
			verboselog(*this, 0, "[ARM9] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

WRITE32_MEMBER(nds_state::arm9_io_w)
{
	switch(offset)
	{
		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			if (!(m_arm9_postflg & POSTFLG_PBF_MASK))
			{
				m_arm9_postflg &= ~POSTFLG_PBF_MASK;
				m_arm9_postflg |= data & POSTFLG_PBF_MASK;
			}
			m_arm9_postflg &= ~POSTFLG_RAM_MASK;
			m_arm9_postflg |= data & POSTFLG_RAM_MASK;
			break;
		default:
			verboselog(*this, 0, "[ARM7] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

static ADDRESS_MAP_START( nds_arm7_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x03800000, 0x0380ffff) AM_RAM
	AM_RANGE(0x04000000, 0x0400ffff) AM_READWRITE(arm7_io_r, arm7_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nds_arm9_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000000, 0x00000fff) AM_ROM
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x04000000, 0x0400ffff) AM_READWRITE(arm9_io_r, arm9_io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( nds )
INPUT_PORTS_END


void nds_state::machine_reset()
{
	m_arm7_postflg = 0;
	m_arm9_postflg = 0;
}

void nds_state::machine_start()
{
}

static MACHINE_CONFIG_START( nds )
	MCFG_CPU_ADD("arm7", ARM7, XTAL_33_333MHz)
	MCFG_CPU_PROGRAM_MAP(nds_arm7_map)

	MCFG_CPU_ADD("arm9", ARM946ES, XTAL_66_6667MHz)
	MCFG_CPU_PROGRAM_MAP(nds_arm9_map)
MACHINE_CONFIG_END

/* Help identifying the region and revisions of the set would be greatly appreciated! */
ROM_START( nds )
	ROM_REGION( 0x1000, "arm9", 0 )
	ROM_LOAD( "biosnds9.rom", 0x0000, 0x1000, CRC(2ab23573) SHA1(bfaac75f101c135e32e2aaf541de6b1be4c8c62d) )

	ROM_REGION( 0x4000, "arm7", 0 )
	ROM_LOAD( "biosnds7.rom", 0x0000, 0x4000, CRC(1280f0d5) SHA1(24f67bdea115a2c847c8813a262502ee1607b7df) )

	ROM_REGION32_LE( 0x40000, "firmware", 0 )
	ROM_LOAD( "firmware.bin", 0x0000, 0x40000, CRC(945f9dc9) SHA1(cfe072921ee3fb93f688743f8beef89043c3e9ad) )
ROM_END

//   YEAR  NAME PARENT COMPAT MACHINE INPUT  STATE      INIT  COMPANY     FULLNAME            FLAGS
CONS(2004, nds, 0,     0,     nds,    nds,   nds_state, 0,    "Nintendo", "DS",               MACHINE_IS_SKELETON)
