// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_SIS6326_H
#define MAME_BUS_PCI_SIS6326_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga_sis.h"

class sis6326_agp_device : public pci_card_device
{
public:
	sis6326_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

protected:
	sis6326_agp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual u8 capptr_r() override;

	void vram_aperture_map(address_map &map) ATTR_COLD;
	void mmio_map(address_map &map) ATTR_COLD;
	void vmi_map(address_map &map) ATTR_COLD;
private:
	required_device<sis6236_vga_device> m_vga;
	required_memory_region m_vga_rom;

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	struct {
		bool enable = false;
		u8 data_rate = 0;
	} m_agp;

	u32 agp_command_r(offs_t offset, uint32_t mem_mask);
	void agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(SIS6326_AGP, sis6326_agp_device)

#endif // MAME_BUS_PCI_SIS6326_H
