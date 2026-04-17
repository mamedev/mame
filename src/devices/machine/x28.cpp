// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
  Xicor X28 EEPROMs
***************************************************************************/

#include "emu.h"
#include "x28.h"

// device type definitions
DEFINE_DEVICE_TYPE(X28C64, x28c64_device, "x28c64", "X28C64 8Kx8 EEPROM")
DEFINE_DEVICE_TYPE(X28C256, x28c256_device, "x28c256", "X28C256 32Kx8 EEPROM")
DEFINE_DEVICE_TYPE(X28HC256, x28hc256_device, "x28hc256", "X28HC256 32Kx8 EEPROM")
DEFINE_DEVICE_TYPE(X28C512, x28c512_device, "x28c512", "X28C512 64Kx8 EEPROM")
DEFINE_DEVICE_TYPE(X28C010, x28c010_device, "x28c010", "X28C010 128Kx8 EEPROM")
DEFINE_DEVICE_TYPE(XM28C020, xm28c020_device, "xm28c020", "XM28C020 256Kx8 EEPROM")
DEFINE_DEVICE_TYPE(XM28C040, xm28c040_device, "xm28c040", "XM28C040 512Kx8 EEPROM")

x28c64_device::x28c64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, X28C64, tag, owner, clock)
{
}

x28c256_device::x28c256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, X28C256, tag, owner, clock)
{
}

x28hc256_device::x28hc256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, X28HC256, tag, owner, clock)
{
}

x28c512_device::x28c512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, X28C512, tag, owner, clock)
{
}

x28c010_device::x28c010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, X28C010, tag, owner, clock)
{
}

xm28c020_device::xm28c020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, XM28C020, tag, owner, clock)
{
}

xm28c040_device::xm28c040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: x28_device(mconfig, XM28C040, tag, owner, clock)
{
}
