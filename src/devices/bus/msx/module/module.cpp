// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "module.h"
#include "sfg.h"
#include "skw01.h"


namespace bus::msx::module::slotoptions {
char const *const SFG01 = "sfg01";
char const *const SFG05 = "sfg05";
char const *const SKW01 = "skw01";
}


void msx_yamaha_60pin(device_slot_interface &device, bool is_in_subslot)
{
	using namespace bus::msx::module;
	device.option_add(slotoptions::SFG01, MSX_CART_SFG01);
	device.option_add(slotoptions::SFG05, MSX_CART_SFG05);
	device.option_add(slotoptions::SKW01, MSX_CART_SKW01);
}


DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device, "msx_slot_yamaha_expansion", "MSX Yamaha Expansion slot")


msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_base_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, tag, owner, clock)
{
}

void msx_slot_yamaha_expansion_device::device_start()
{
}

std::string msx_slot_yamaha_expansion_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	using namespace bus::msx::module;
	return software_get_default_slot(slotoptions::SFG01);
}
