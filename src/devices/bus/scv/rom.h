// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SCV_ROM_H
#define MAME_BUS_SCV_ROM_H

#pragma once

#include "slot.h"


// device type definition
DECLARE_DEVICE_TYPE(SCV_ROM8K,         device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM16K,        device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM32K,        device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM32K_RAM8K,  device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM64K,        device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM128K,       device_scv_cart_interface)
DECLARE_DEVICE_TYPE(SCV_ROM128K_RAM4K, device_scv_cart_interface)

#endif // MAME_BUS_SCV_ROM_H
