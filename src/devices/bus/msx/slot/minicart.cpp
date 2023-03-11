// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "minicart.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_MINICART,  msx_slot_yamaha_minicart_device,  "msx_slot_yamaha_minicart",  "MSX Yamaha Minicart slot")


msx_slot_yamaha_minicart_device::msx_slot_yamaha_minicart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_MINICART, tag, owner, clock)
{
}

void msx_slot_yamaha_minicart_device::device_start()
{
}
