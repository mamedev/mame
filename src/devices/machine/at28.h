// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  Atmel 28-series Parallel EEPROMs

***************************************************************************/

#ifndef MAME_MACHINE_AT28_H
#define MAME_MACHINE_AT28_H

#pragma once

#include "eeprom28.h"

// Concrete devices

#define DECLARE_AT28_DEVICES_AND_TYPES(Type, Class, AddrBits, PageBytes, TBLC, TWC, ...)                        \
/* Device Type - includes a forward declaration of the Class */                                                 \
DECLARE_DEVICE_TYPE(Type, Class##_device)                                                                       \
/* The Class, which can now refer to the Device Type when calling the superclass's constructor */               \
class Class##_device                                                                                            \
: public eeprom28_device<AddrBits, PageBytes, TBLC, TWC, false, true, true, true __VA_OPT__(,) __VA_ARGS__>     \
{                                                                                                               \
public:                                                                                                      \
	Class##_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)        \
	: eeprom28_device(mconfig, Type, tag, owner, clock) { }                                                    \
};                                                                                                           \
/* Device Type - includes a forward declaration of the Class */                                              \
DECLARE_DEVICE_TYPE(Type##_NVRAM, Class##_nvram_device)                                                      \
/* The Class, which can now refer to the Device Type when calling the superclass's constructor */            \
class Class##_nvram_device : public eeprom28_nvram_device<AddrBits, PageBytes, TBLC, TWC, false, true, true, true __VA_OPT__(,) __VA_ARGS__>                                       \
{                                                                                                            \
public:                                                                                                      \
	Class##_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)  \
	: eeprom28_nvram_device(mconfig, Type##_NVRAM, tag, owner, clock) { }                                      \
};

// AT28C64B: 64kbit == 8k bytes, 64 bytes per page
DECLARE_AT28_DEVICES_AND_TYPES(AT28C64B, at28c64b, 13, 64, 100, 5000)

// AT28C256: 256kbit == 32k bytes, 64 bytes per page
DECLARE_AT28_DEVICES_AND_TYPES(AT28C256, at28c256, 15, 64, 100, 5000)

// AT28C010: 1Mbit = 128k bytes, 128 bytes per page
DECLARE_AT28_DEVICES_AND_TYPES(AT28C010, at28c010, 17, 128, 100, 10000)

// AT28C040: 4Mbit = 512 kbytes, 256 bytes per page
DECLARE_AT28_DEVICES_AND_TYPES(AT28C040, at28c040, 19, 256, 150, 10000)

#undef DECLARE_AT28_DEVICES_AND_TYPES

#endif // MAME_MACHINE_AT28_H
