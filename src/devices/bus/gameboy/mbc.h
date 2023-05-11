// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Wilbert Pol
#ifndef MAME_BUS_GAMEBOY_MBC_H
#define MAME_BUS_GAMEBOY_MBC_H

#pragma once

#include "slot.h"


DECLARE_DEVICE_TYPE(GB_ROM_MBC1,     device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_MBC5,     device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_BBD,      device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_DSHGGB81, device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_SINTAX,   device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_CHONGWU,  device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_LICHENG,  device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_NEWGBCHK, device_gb_cart_interface)
DECLARE_DEVICE_TYPE(GB_ROM_VF001,    device_gb_cart_interface)

#endif // MAME_BUS_GAMEBOY_MBC_H
