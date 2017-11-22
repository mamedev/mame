// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  nds.cpp

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
	uint8_t temp1, temp2;
	switch(offset)
	{
		case IPCSYNC_OFFSET:
			return m_arm7_ipcsync;

		case GAMECARD_BUS_CTRL_OFFSET:
			return 0xffffffff;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			return m_arm7_postflg;

		case WRAMSTAT_OFFSET:
			temp1 = (((m_vramcntc & 3) == 2) && (m_vramcntc & 0x80)) ? 1 : 0;
			temp2 = (((m_vramcntd & 3) == 2) && (m_vramcntd & 0x80)) ? 2 : 0;
			return (m_wramcnt << 8) | temp1 | temp2;

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
		case IPCSYNC_OFFSET:
			printf("ARM7: %x to IPCSYNC\n", data);
			m_arm9_ipcsync &= ~0xf;
			m_arm9_ipcsync |= ((data >> 8) & 0xf);
			m_arm7_ipcsync &= 0xf;
			m_arm7_ipcsync |= (data & ~0xf);
			break;

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
		case IPCSYNC_OFFSET:
			return m_arm9_ipcsync;

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
		case IPCSYNC_OFFSET:
			printf("ARM9: %x to IPCSYNC\n", data);
			m_arm7_ipcsync &= ~0xf;
			m_arm7_ipcsync |= ((data >> 8) & 0xf);
			m_arm9_ipcsync &= 0xf;
			m_arm9_ipcsync |= (data & ~0xf);
			break;

		case VRAMCNT_A_OFFSET:
			if (ACCESSING_BITS_0_7)	// VRAMCNT_A
			{
				m_vramcnta = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_B
			{
				m_vramcntb = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_C
			{
				m_vramcntc = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // VRAMCNT_D
			{
				m_vramcntd = (data >> 24) & 0xff;
			}
			break;

		case WRAMCNT_OFFSET:
			if (ACCESSING_BITS_0_7)	// VRAMCNT_E
			{
				m_vramcnte = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_F
			{
				m_vramcntf = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_G
			{
				m_vramcntg = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // WRAMCNT
			{
				m_wramcnt = (data>>24) & 0x3;
				m_arm7wrambnk->set_bank(m_wramcnt);
				m_arm9wrambnk->set_bank(m_wramcnt);
			}
			break;

		case VRAMCNT_H_OFFSET:
			if (ACCESSING_BITS_0_7)	// VRAMCNT_H
			{
				m_vramcnth = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_I
			{
				m_vramcnti = (data >> 8) & 0xff;
			}
			break;

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
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM AM_REGION("arm7", 0)
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_MIRROR(0x00400000) AM_SHARE("mainram")
	AM_RANGE(0x03000000, 0x03007fff) AM_DEVICE("nds7wram", address_map_bank_device, amap32) AM_MIRROR(0x007f0000)
	AM_RANGE(0x03800000, 0x0380ffff) AM_RAM AM_MIRROR(0x007f0000) AM_SHARE("arm7ram")
	AM_RANGE(0x04000000, 0x0400ffff) AM_READWRITE(arm7_io_r, arm7_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nds_arm9_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x02000000, 0x023fffff) AM_RAM AM_MIRROR(0x00400000) AM_SHARE("mainram")
	AM_RANGE(0x03000000, 0x03007fff) AM_DEVICE("nds9wram", address_map_bank_device, amap32) AM_MIRROR(0x00ff0000)
	AM_RANGE(0x04000000, 0x0400ffff) AM_READWRITE(arm9_io_r, arm9_io_w)
	AM_RANGE(0xffff0000, 0xffff0fff) AM_ROM AM_MIRROR(0x1000) AM_REGION("arm9", 0)
ADDRESS_MAP_END

// ARM7 views of WRAM
static ADDRESS_MAP_START( nds7_wram_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000, 0x07fff) AM_READWRITE(wram_arm7mirror_r, wram_arm7mirror_w)
	AM_RANGE(0x08000, 0x0bfff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
	AM_RANGE(0x0c000, 0x0ffff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
	AM_RANGE(0x10000, 0x13fff) AM_READWRITE(wram_second_half_r, wram_second_half_w)
	AM_RANGE(0x14000, 0x17fff) AM_READWRITE(wram_second_half_r, wram_second_half_w)
	AM_RANGE(0x18000, 0x1ffff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
ADDRESS_MAP_END

// ARM9 views of WRAM
static ADDRESS_MAP_START( nds9_wram_map, AS_PROGRAM, 32, nds_state )
	AM_RANGE(0x00000, 0x07fff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
	AM_RANGE(0x08000, 0x0bfff) AM_READWRITE(wram_second_half_r, wram_second_half_w)
	AM_RANGE(0x0c000, 0x0ffff) AM_READWRITE(wram_second_half_r, wram_second_half_w)
	AM_RANGE(0x10000, 0x13fff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
	AM_RANGE(0x14000, 0x17fff) AM_READWRITE(wram_first_half_r, wram_first_half_w)
	AM_RANGE(0x18000, 0x1ffff) AM_NOP AM_WRITENOP		// probably actually open bus?  GBATEK describes as "random"
ADDRESS_MAP_END

READ32_MEMBER(nds_state::wram_first_half_r) { return m_WRAM[offset]; }
READ32_MEMBER(nds_state::wram_second_half_r) { return m_WRAM[offset+0x4000]; }
WRITE32_MEMBER(nds_state::wram_first_half_w) { COMBINE_DATA(&m_WRAM[offset]); }
WRITE32_MEMBER(nds_state::wram_second_half_w) { COMBINE_DATA(&m_WRAM[offset+0x4000]); }
READ32_MEMBER(nds_state::wram_arm7mirror_r) { return m_arm7ram[offset]; }
WRITE32_MEMBER(nds_state::wram_arm7mirror_w) { COMBINE_DATA(&m_arm7ram[offset]); }

static INPUT_PORTS_START( nds )
INPUT_PORTS_END


void nds_state::machine_reset()
{
	m_arm7_postflg = 0;
	m_arm9_postflg = 0;
	m_wramcnt = 0;
	m_arm7wrambnk->set_bank(0);
	m_arm9wrambnk->set_bank(0);
}

void nds_state::machine_start()
{
}

static MACHINE_CONFIG_START( nds )
	MCFG_CPU_ADD("arm7", ARM7, XTAL_33_333MHz)
	MCFG_CPU_PROGRAM_MAP(nds_arm7_map)

	MCFG_CPU_ADD("arm9", ARM946ES, XTAL_66_6667MHz)
	MCFG_ARM_HIGH_VECTORS()
	MCFG_CPU_PROGRAM_MAP(nds_arm9_map)

	// WRAM
	MCFG_DEVICE_ADD("nds7wram", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(nds7_wram_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	MCFG_DEVICE_ADD("nds9wram", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(nds9_wram_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)
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
