// license:BSD-3-Clause
// copyright-holders:
#ifndef MAME_BUS_PCI_NEON250_H
#define MAME_BUS_PCI_NEON250_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga.h"

class neon250_device : public pci_card_device
{
public:
	neon250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	neon250_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	required_device<vga_device> m_svga;
	required_memory_region m_vga_rom;

	virtual u8 capptr_r() override;

//  bool m_vga_legacy_enable = false;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void pvr_fb_map(address_map &map) ATTR_COLD;
	void pvr_mmio_map(address_map &map) ATTR_COLD;
	void pvr_io_map(address_map &map) ATTR_COLD;

	void vga_fb_map(address_map &map) ATTR_COLD;
	void vga_mmio_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NEON250,   neon250_device)

#endif // MAME_BUS_PCI_NEON250_H
