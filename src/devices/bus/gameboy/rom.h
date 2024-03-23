// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Wilbert Pol
#ifndef MAME_BUS_GAMEBOY_ROM_H
#define MAME_BUS_GAMEBOY_ROM_H

#include "slot.h"

DECLARE_DEVICE_TYPE(MEGADUCK_ROM_STD,    device_gb_cart_interface)
DECLARE_DEVICE_TYPE(MEGADUCK_ROM_BANKED, device_gb_cart_interface)

DECLARE_DEVICE_TYPE(GB_ROM_STD,          device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_M161,         device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_WISDOM,       device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_YONG,         device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_ROCKMAN8,     device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_SM3SP,        device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_SACHEN1,      device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_SACHEN2,      device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_ROCKET,       device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_LASAMA,       device_gb_cart_interface)

#endif // MAME_BUS_GAMEBOY_ROM_H
