// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    SunPlus SPG110-series SoC peripheral emulation

    It is possible this shares some video features with spg110 and
	can be made a derived device

**********************************************************************/

#include "emu.h"
#include "spg110.h"

DEFINE_DEVICE_TYPE(SPG110, spg110_device, "spg110", "SPG110 System-on-a-Chip")

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
{
}

spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock)
{
}

void spg110_device::map(address_map &map)
{
	map(0x000000, 0x000fff).ram();
	// vregs are at 2000?
}

void spg110_device::device_start()
{
}

void spg110_device::device_reset()
{
}


uint32_t spg110_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE_LINE_MEMBER(spg110_device::vblank)
{
	if (!state)
		return;
}
