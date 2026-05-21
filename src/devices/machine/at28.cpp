// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
  Atmel AT28 EEPROMs
***************************************************************************/

#include "emu.h"
#include "at28.h"

#define DEFINE_AT28_DEVICE_TYPES(Type,Class,ShortName,LongName)                                   \
DEFINE_DEVICE_TYPE(Type, Class##_device, ShortName, LongName)                                     \
DEFINE_DEVICE_TYPE(Type##_NVRAM, Class##_nvram_device, ShortName "_nvram", LongName " (nvram)")

// device type definitions

DEFINE_AT28_DEVICE_TYPES(AT28C64B, at28c64b, "at28c64b", "AT28C64B 8Kx8 EEPROM")
DEFINE_AT28_DEVICE_TYPES(AT28C256, at28c256, "at28c256", "AT28C256 32Kx8 EEPROM")
DEFINE_AT28_DEVICE_TYPES(AT28C010, at28c010, "at28c010", "AT28C010 128Kx8 EEPROM")
DEFINE_AT28_DEVICE_TYPES(AT28C040, at28c040, "at28c040", "AT28C040 512Kx8 EEPROM")

#undef DEFINE_AT28_DEVICE_TYPES
