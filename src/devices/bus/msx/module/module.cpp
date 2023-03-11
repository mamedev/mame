// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "module.h"
#include "yamaha.h"


// Several yamaha machines had 60 pin expansion slots. Most pinouts of these slots was
// exactly the same as the regular 50 pin cartridge slots. The lowest 10 pins include
// some extra sound and video related pins.
void msx_yamaha_60pin(device_slot_interface &device, bool is_in_subslot)
{
	device.option_add("sfg01", MSX_CART_SFG01);
	device.option_add("sfg05", MSX_CART_SFG05);
}


DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device, "msx_slot_yamaha_expansion", "MSX Yamaha Expansion slot")


msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, tag, owner, clock)
{
}

void msx_slot_yamaha_expansion_device::device_start()
{
}

std::string msx_slot_yamaha_expansion_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}
