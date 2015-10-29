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


lpc210x_device::lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: arm7_cpu_device(mconfig, LPC2103, "LPC2103", tag, owner, clock, "lpc2103", __FILE__, 4, eARM_ARCHFLAGS_T, ENDIANNESS_LITTLE)
{
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
