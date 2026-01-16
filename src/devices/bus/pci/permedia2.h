// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_PERMEDIA2_H
#define MAME_BUS_PCI_PERMEDIA2_H

#pragma once

#include "pci_slot.h"

#include "video/pc_vga.h"

class permedia2_device : public pci_card_device
{
public:
	permedia2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	permedia2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	virtual u8 capptr_r() override;
private:
	required_device<vga_device> m_vga;
	required_memory_region m_vga_rom;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void control_map(address_map &map) ATTR_COLD;
	void aperture1_map(address_map &map) ATTR_COLD;
	void aperture2_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PERMEDIA2, permedia2_device)

#endif // MAME_BUS_PCI_PERMEDIA2_H
