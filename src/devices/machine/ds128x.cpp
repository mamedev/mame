// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "ds128x.h"

/// TODO: Only DV2/DV1/DV0 == 0/1/0 is supported as the chip only has a 15 stage divider and not 22.

DEFINE_DEVICE_TYPE(DS12885, ds12885_device, "ds12885", "DS12885 RTC/NVRAM")

//-------------------------------------------------
//  ds12885_device - constructor
//-------------------------------------------------

ds12885_device::ds12885_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc146818_device(mconfig, DS12885, tag, owner, clock)
{
}
