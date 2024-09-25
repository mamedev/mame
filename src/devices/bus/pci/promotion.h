// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_PROMOTION_H
#define MAME_BUS_PCI_PROMOTION_H

#pragma once

#include "pci_slot.h"

#include "video/pc_vga_alliance.h"


class promotion3210_device : public pci_card_device
{
public:
	promotion3210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

protected:
	promotion3210_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	required_device<promotion_vga_device> m_vga;
	required_memory_region m_vga_rom;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void vram_aperture_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PROMOTION3210, promotion3210_device)

#endif // MAME_BUS_PCI_PROMOTION_H
