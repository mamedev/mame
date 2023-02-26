// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_CARTRIDGE_H
#define MAME_BUS_MSX_CART_CARTRIDGE_H

#pragma once


void msx_cart(device_slot_interface &device, bool is_in_subslot);
void msx_yamaha_60pin(device_slot_interface &device, bool is_in_subslot);   // 60 pin expansion slots as found in yamaha machines


#endif // MAME_BUS_MSX_CART_CARTRIDGE_H
