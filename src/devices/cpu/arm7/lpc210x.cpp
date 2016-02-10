// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

 NXP (Phillips) LPC2103 series
 covering LPC2101, LPC2102, LPC2103*

 *currently only LPC2103

 these are based on an ARM7TDMI-S CPU
 internal flash and integrated peripherals

***************************************************************************/

#include "lpc210x.h"

const device_type LPC2103 = &device_creator<lpc210x_device>;

static ADDRESS_MAP_START( lpc2103_map, AS_PROGRAM, 32, lpc210x_device )
	AM_RANGE(0x00000000, 0x00007fff) AM_READWRITE(flash_r, flash_w) // 32kb internal FLASH rom

	AM_RANGE(0x3FFFC000, 0x3FFFC01f) AM_READWRITE( fio_r, fio_w ) // GPIO


	AM_RANGE(0x40000000, 0x40001fff) AM_RAM // 8kb internal SROM (writes should actually latch - see docs)

	AM_RANGE(0xE0004000, 0xE000407f) AM_READWRITE( timer0_r, timer0_w)

	AM_RANGE(0xE0008000, 0xE000807f) AM_READWRITE( timer1_r, timer1_w)

	AM_RANGE(0xE002C000, 0xE002C007) AM_READWRITE( pin_r, pin_w )

	AM_RANGE(0xE01FC000, 0xE01FC007) AM_READWRITE( mam_r, mam_w )
	AM_RANGE(0xE01FC080, 0xE01FC08f) AM_READWRITE( pll_r, pll_w ) // phase locked loop
	AM_RANGE(0xE01FC100, 0xE01FC103) AM_READWRITE( apbdiv_r, apbdiv_w )
	AM_RANGE(0xE01FC1a0, 0xE01FC1a3) AM_READWRITE( scs_r, scs_w )

	AM_RANGE(0xFFFFF000, 0xFFFFF2ff) AM_READWRITE( vic_r, vic_w ) // interrupt controller
ADDRESS_MAP_END


lpc210x_device::lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: arm7_cpu_device(mconfig, LPC2103, "LPC2103", tag, owner, clock, "lpc2103", __FILE__, 4, eARM_ARCHFLAGS_T, ENDIANNESS_LITTLE),
		m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, ADDRESS_MAP_NAME(lpc2103_map))
{
}

READ32_MEMBER(lpc210x_device::arm_E01FC088_r)
{
	return 0xffffffff;
}

READ32_MEMBER(lpc210x_device::flash_r)
{
	UINT32 ret = (m_flash[offset * 4 + 3] << 24) |
					(m_flash[offset * 4 + 2] << 16) |
					(m_flash[offset * 4 + 1] << 8) |
					(m_flash[offset * 4 + 0] << 0);
	return ret;
}

WRITE32_MEMBER(lpc210x_device::flash_w)
{
	//
}


const address_space_config *lpc210x_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	default:                   return nullptr;
	}
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lpc210x_device::device_start()
{
	arm7_cpu_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lpc210x_device::device_reset()
{
	arm7_cpu_device::device_reset();

	m_TxPR[0] = 0;
	m_TxPR[1] = 0;
}

/* VIC (Vectored Interrupt Controller) */

READ32_MEMBER( lpc210x_device::vic_r )
{
	switch (offset*4)
	{
	default:
		logerror("%08x unhandled read from VIC offset %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::vic_w )
{
	switch (offset * 4)
	{
	default:
		logerror("%08x unhandled write VIC offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, data, mem_mask);
	}
}

/* PIN Select block */

READ32_MEMBER( lpc210x_device::pin_r )
{
	switch (offset*4)
	{
	default:
		logerror("%08x unhandled read from PINSEL offset %08x mem_mask %08x\n",space.device().safe_pc(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::pin_w )
{
	switch (offset * 4)
	{
	default:
		logerror("%08x unhandled write PINSEL offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, data, mem_mask);
	}
}

/* MAM block (memory conttroller) */

READ32_MEMBER( lpc210x_device::mam_r )
{
	switch (offset*4)
	{
	default:
		logerror("%08x unhandled read from MAM offset %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::mam_w )
{
	switch (offset * 4)
	{
	default:
		logerror("%08x unhandled write MAM offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, data, mem_mask);
	}
}

/* FIO block */

READ32_MEMBER( lpc210x_device::fio_r )
{
	switch (offset*4)
	{
	default:
		logerror("%08x unhandled read from FIO offset %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, mem_mask);
	}

	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::fio_w )
{
	switch (offset * 4)
	{
	default:
		logerror("%08x unhandled write FIO offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, data, mem_mask);
	}
}


/* APB Divider */

READ32_MEMBER( lpc210x_device::apbdiv_r )
{
	logerror("%08x unhandled read from APBDIV offset %08x mem_mask %08x\n", space.device().safe_pc(), offset * 4, mem_mask);
	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::apbdiv_w )
{
	logerror("%08x unhandled write APBDIV offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(),offset * 4, data, mem_mask);
}

/* Syscon misc registers */

READ32_MEMBER( lpc210x_device::scs_r )
{
	logerror("%08x unhandled read from SCS offset %08x mem_mask %08x\n", space.device().safe_pc(),offset * 4, mem_mask);
	return 0x00000000;
}


WRITE32_MEMBER( lpc210x_device::scs_w )
{
	logerror("%08x unhandled write SCS offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(),offset * 4, data, mem_mask);
}

/* PLL Phase Locked Loop */

READ32_MEMBER( lpc210x_device::pll_r )
{
	switch (offset*4)
	{
	default:
		logerror("%08x unhandled read from PLL offset %08x mem_mask %08x\n", space.device().safe_pc(),offset * 4, mem_mask);
	}

	return 0xffffffff;
}


WRITE32_MEMBER( lpc210x_device::pll_w )
{
	switch (offset * 4)
	{
	default:
		logerror("%08x unhandled write PLL offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(),offset * 4, data, mem_mask);
	}
}


/* Timers */

UINT32 lpc210x_device::read_timer(address_space &space, int timer, int offset, UINT32 mem_mask)
{
	switch (offset*4)
	{
	case 0x0c:
		return m_TxPR[timer];

	default:
		logerror("%08x unhandled read from timer %d offset %02x mem_mask %08x\n", space.device().safe_pc(),timer, offset * 4, mem_mask);
	}

	return 0x00000000;
}


void lpc210x_device::write_timer(address_space &space, int timer, int offset, UINT32 data, UINT32 mem_mask)
{
	switch (offset * 4)
	{
	case 0x0c:
		COMBINE_DATA(&m_TxPR[timer]);
		logerror("%08x Timer %d Prescale Register set to %08x\n", space.device().safe_pc(),timer, m_TxPR[timer]);
		break;

	default:
		logerror("%08x unhandled write timer %d offset %02x data %08x mem_mask %08x\n", space.device().safe_pc(),timer, offset * 4, data, mem_mask);
	}
}



static MACHINE_CONFIG_FRAGMENT( lpc210x )
MACHINE_CONFIG_END

machine_config_constructor lpc210x_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lpc210x );
}
