// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "module.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device, "msx_slot_yamaha_expansion", "MSX Yamaha Expansion slot")


msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, tag, owner, clock)
{
}

void msx_slot_yamaha_expansion_device::device_start()
{
}
