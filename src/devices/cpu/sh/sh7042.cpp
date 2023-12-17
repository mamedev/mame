// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#include "emu.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH7042,  sh7042_device,  "sh7042",  "Hitachi SH-2 (SH7042)")

sh7042_device::sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sh2_device(mconfig, SH7042, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7042_device::map), this), 32, 0xffffffff)
{
}

void sh7042_device::device_start()
{
	sh2_device::device_start();
}

void sh7042_device::device_reset()
{
	sh2_device::device_reset();
}

void sh7042_device::map(address_map &map)
{
	map(0xfffff000, 0xffffffff).ram();
}
