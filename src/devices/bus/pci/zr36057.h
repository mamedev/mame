// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_ZR36057_H
#define MAME_BUS_PCI_ZR36057_H

#pragma once

#include "pci_slot.h"


class zr36057_device : public pci_card_device
{
public:
	zr36057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ZR36057_PCI, zr36057_device)

#endif // MAME_BUS_PCI_ZR36057_H
