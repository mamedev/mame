// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KONAMI_H
#define MAME_BUS_MSX_CART_KONAMI_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_KONAMI,           msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_KONAMI_SCC,       msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_SUNRISE_SCC,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_GAMEMASTER2,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_SYNTHESIZER,      msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_SOUND_SNATCHER,   msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_SOUND_SDSNATCHER, msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_KEYBOARD_MASTER,  msx_cart_interface)
DECLARE_DEVICE_TYPE(MSX_CART_EC701,            msx_cart_interface)

#endif // MAME_BUS_MSX_CART_KONAMI_H
