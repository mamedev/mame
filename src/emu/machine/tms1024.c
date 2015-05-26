// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Texas Instruments TMS1024, TMS1025 I/O expander emulation

**********************************************************************/

#include "machine/tms1024.h"


const device_type TMS1024 = &device_creator<tms1024_device>;

//-------------------------------------------------
//  tms1024_device - constructor
//-------------------------------------------------

tms1024_device::tms1024_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS1024, "TMS1024", tag, owner, clock, "tms1024", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms1024_device::device_start()
{
	// resolve callbacks

	// register for savestates
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms1024_device::device_reset()
{
}


// handlers
