// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PCI_OTI_SPITFIRE_PCI_H
#define MAME_BUS_PCI_OTI_SPITFIRE_PCI_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga_oak.h"

class oti64111_pci_device : public pci_card_device
{
public:
	oti64111_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	oti64111_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	required_device<oak_oti111_vga_device> m_svga;
	required_memory_region m_vga_rom;
private:
	void mmio_map(address_map &map) ATTR_COLD;
	void vram_aperture_map(address_map &map) ATTR_COLD;
	void extio_map(address_map &map) ATTR_COLD;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(OTI64111_PCI, oti64111_pci_device)

#endif // MAME_BUS_PCI_OTI_SPITFIRE_PCI_H
