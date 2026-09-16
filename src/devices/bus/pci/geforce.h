// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_GEFORCE_H
#define MAME_BUS_PCI_GEFORCE_H

#pragma once

#include "rivatnt.h"

#include "machine/pci.h"
#include "video/pc_vga_nvidia.h"


class geforce256_device : public rivatnt2_device
{
public:
	geforce256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// ELSA protection
	static constexpr feature_type unemulated_features() { return feature::PROTECTION; }

protected:
	geforce256_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

class geforce256_ddr_device : public geforce256_device
{
public:
	geforce256_ddr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class quadro_device : public geforce256_device
{
public:
	quadro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// ELSA protection
	static constexpr feature_type unemulated_features() { return feature::PROTECTION; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(GEFORCE256,       geforce256_device)
DECLARE_DEVICE_TYPE(GEFORCE256_DDR,   geforce256_ddr_device)
//DECLARE_DEVICE_TYPE(GEFORCE256_ULTRA, geforce256_ultra_device)
DECLARE_DEVICE_TYPE(QUADRO,           quadro_device)


#endif // MAME_BUS_PCI_GEFORCE_H
