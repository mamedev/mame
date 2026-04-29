// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
  Xicor X28 EEPROMs
***************************************************************************/

#include "emu.h"
#include "x28.h"

#define DEFINE_X28_DEVICE_TYPES(Type,Class,ShortName,LongName)                                    \
DEFINE_DEVICE_TYPE(Type, Class##_device, ShortName, LongName)                                     \
DEFINE_DEVICE_TYPE(Type##_NVRAM, Class##_nvram_device, ShortName "_nvram", LongName " (nvram)")

// device type definitions
DEFINE_X28_DEVICE_TYPES(X28C64, x28c64, "x28c64", "X28C64 8Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(X28C256, x28c256, "x28c256", "X28C256 32Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(X28HC256, x28hc256, "x28hc256", "X28HC256 32Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(X28C512, x28c512, "x28c512", "X28C512 64Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(X28C010, x28c010, "x28c010", "X28C010 128Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(XM28C020, xm28c020, "xm28c020", "XM28C020 256Kx8 EEPROM")
DEFINE_X28_DEVICE_TYPES(XM28C040, xm28c040, "xm28c040", "XM28C040 512Kx8 EEPROM")

#undef DEFINE_X28_DEVICE_TYPES
