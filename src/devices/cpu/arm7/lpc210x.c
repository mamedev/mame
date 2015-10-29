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
	AM_RANGE(0x40000000, 0x40001fff) AM_RAM // 8kb internal SROM (writes should actually latch - see docs)

	AM_RANGE(0xE01FC088, 0xE01FC08b) AM_READ(arm_E01FC088_r)
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
	default:                   return NULL;
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
}


static MACHINE_CONFIG_FRAGMENT( lpc210x )
MACHINE_CONFIG_END

machine_config_constructor lpc210x_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lpc210x );
}
