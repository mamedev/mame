// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "yamaha.h"

#include "bus/msx/slot/cartridge.h"


// Several yamaha machines had 60 pin expansion slots. Most pinouts of these slots was
// exactly the same as the regular 50 pin cartridge slots. The lowest 10 pins include
// some extra sound and video related pins.
void msx_yamaha_60pin(device_slot_interface &device, bool is_in_subslot)
{
	device.option_add("sfg01", MSX_CART_SFG01);
	device.option_add("sfg05", MSX_CART_SFG05);
}
