// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  Xicor 28-series Parallel EEPROMs

***************************************************************************/

#ifndef MAME_MACHINE_X28_H
#define MAME_MACHINE_X28_H

#pragma once

#include "eeprom28.h"

// Concrete devices

#define DECLARE_X28_DEVICES_AND_TYPES(Type, Class, AddrBits, PageBytes, TBLC, TWC, ...)                      \
/* Device Type - includes a forward declaration of the Class */                                              \
DECLARE_DEVICE_TYPE(Type, Class##_device)                                                                    \
/* The Class, which can now refer to the Device Type when calling the superclass's constructor */            \
class Class##_device                                                                                         \
: public eeprom28_device<AddrBits, PageBytes, TBLC, TWC __VA_OPT__(,) __VA_ARGS__>                          \
{                                                                                                            \
public:                                                                                                      \
	Class##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)        \
	: eeprom28_device(mconfig, Type, tag, owner, clock) { }                                                    \
};                                                                                                           \
/* Device Type - includes a forward declaration of the Class */                                              \
DECLARE_DEVICE_TYPE(Type##_NVRAM, Class##_nvram_device)                                                      \
/* The Class, which can now refer to the Device Type when calling the superclass's constructor */            \
class Class##_nvram_device                                                                                   \
: public eeprom28_nvram_device<AddrBits, PageBytes, TBLC, TWC __VA_OPT__(,) __VA_ARGS__>                     \
{                                                                                                            \
public:                                                                                                      \
	Class##_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)  \
	: eeprom28_nvram_device(mconfig, Type##_NVRAM, tag, owner, clock) { }                                      \
};

// X28C64: 64kbit == 8k bytes, 64 bytes per page
DECLARE_X28_DEVICES_AND_TYPES(X28C64, x28c64, 13, 64, 100, 5000)

// X28C256: 256kbit == 32k bytes, 64 bytes per page
DECLARE_X28_DEVICES_AND_TYPES(X28C256, x28c256, 15, 64, 100, 5000)

// X28HC256: 256kbit == 32k bytes, 64 bytes per page, T_WC = 3 ms.
DECLARE_X28_DEVICES_AND_TYPES(X28HC256, x28hc256, 15, 64, 100, 3000)

// X28C512: 512kbit == 64k bytes, 128 bytes per page
DECLARE_X28_DEVICES_AND_TYPES(X28C512, x28c512, 16, 128, 100, 5000)

// X28C010: 1Mbit = 128k bytes, 256 bytes per page
DECLARE_X28_DEVICES_AND_TYPES(X28C010, x28c010, 17, 256, 100, 5000)

// XM28C020: 2Mbit = 256 kbytes, 128 bytes per page;
// comprised of 4 X28C513LCC:s on a single substrate
DECLARE_X28_DEVICES_AND_TYPES(XM28C020, xm28c020, 18, 128, 100, 5000)

// XM28C040: 4Mbit = 512 kbytes, 256 bytes per page;
// comprised of 4 X28C010:s on a single substrate
DECLARE_X28_DEVICES_AND_TYPES(XM28C040, xm28c040, 19, 256, 100, 5000)

#undef DECLARE_X28_DEVICES_AND_TYPES

#endif // MAME_MACHINE_X28_H
