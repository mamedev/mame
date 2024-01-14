// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "sw1000xg.h"

DEFINE_DEVICE_TYPE(SW1000XG, sw1000xg_device, "sw1000xg", "Yamaha SW1000XG")

sw1000xg_device::sw1000xg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ymp21_device(mconfig, SW1000XG, tag, owner, clock)
{
	set_ids(0x10731000, 0x00, 0x040100, 0x10731000);
}

void sw1000xg_device::device_start()
{
	ymp21_device::device_start();
}

void sw1000xg_device::device_reset()
{
	ymp21_device::device_reset();
}
