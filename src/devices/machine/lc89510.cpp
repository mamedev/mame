// license:BSD-3-Clause
// copyright-holders:David Haywood
/* LC89510 CD Controller
 completely empty placeholder - should be populated or removed

*/


#include "emu.h"
#include "lc89510.h"

DEFINE_DEVICE_TYPE(LC89510, lc89510_device, "lc89510", "LC89510 CD Controller")

lc89510_device::lc89510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LC89510, tag, owner, clock)
{
}


void lc89510_device::device_start()
{
}

void lc89510_device::device_reset()
{
}
