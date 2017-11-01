// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "ds128x.h"

DEFINE_DEVICE_TYPE(DS12885, ds12885_device, "ds12885", "DS12885 RTC/NVRAM")

//-------------------------------------------------
//  ds12885_device - constructor
//-------------------------------------------------

ds12885_device::ds12885_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc146818_device(mconfig, DS12885, tag, owner, clock)
{
}

int ds12885_device::get_timer_bypass()
{
	if( !( m_data[REG_A] & REG_A_DV0 ) ) //DV0 must be 0 for timekeeping
	{
		return 7; // Fixed at 1 Hz with clock at 32768Hz
	}

	return 22; // No tick
}
