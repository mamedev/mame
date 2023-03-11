// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "bus/msx/cart/nomapper.h"

void msx_yamaha_minicart(device_slot_interface &device, bool is_in_subslot)
{
	device.option_add_internal("nomapper", MSX_CART_NOMAPPER);
}
