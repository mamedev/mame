// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_RIVA128_H
#define MAME_BUS_PCI_RIVA128_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga_nvidia.h"


class riva128_device : public pci_card_device
{
public:
	riva128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	riva128_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void mmio_map(address_map &map) ATTR_COLD;
	virtual void vram_aperture_map(address_map &map) ATTR_COLD;
	virtual void indirect_io_map(address_map &map) ATTR_COLD;

	required_device<nvidia_nv3_vga_device> m_svga;
	required_memory_region m_vga_rom;

	bool m_vga_legacy_enable = false;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	u32 unmap_log_r(offs_t offset, u32 mem_mask = ~0);
	void unmap_log_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 m_main_scratchpad_id = 0;
};

class riva128zx_device : public riva128_device
{
public:
	riva128zx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(RIVA128,   riva128_device)
DECLARE_DEVICE_TYPE(RIVA128ZX, riva128zx_device)

#endif // MAME_BUS_PCI_RIVA128_H
