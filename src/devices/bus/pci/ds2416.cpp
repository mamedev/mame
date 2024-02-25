// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "ds2416.h"

DEFINE_DEVICE_TYPE(DS2416, ds2416_device, "ds2416", "Yamaha DS2416")

ds2416_device::ds2416_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymp21_device(mconfig, DS2416, tag, owner, clock)
{
	set_ids(0x10732000, 0x00, 0x040100, 0x10732000);
}

void ds2416_device::device_start()
{
	ymp21_device::device_start();
}

void ds2416_device::device_reset()
{
	ymp21_device::device_reset();
}
