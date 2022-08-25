// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef MAME_BUS_GAMEBOY_ROM_H
#define MAME_BUS_GAMEBOY_ROM_H

#include "gb_slot.h"

DECLARE_DEVICE_TYPE(GB_STD_ROM,    device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_TAMA5,  device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_WISDOM, device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_YONG,   device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_ATVRAC, device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_LASAMA, device_gb_cart_interface)

DECLARE_DEVICE_TYPE(MEGADUCK_ROM,  device_gb_cart_interface)

#endif // MAME_BUS_GAMEBOY_ROM_H
