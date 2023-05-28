// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_RIVA128_H
#define MAME_VIDEO_RIVA128_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga_nvidia.h"

class riva128_device : public pci_device
{
public:
	riva128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map);
	void legacy_io_map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	virtual void mmio_map(address_map &map);
	virtual void vram_aperture_map(address_map &map);
	virtual void indirect_io_map(address_map &map);
private:
	required_device<nvidia_nv3_vga_device> m_svga;
	required_memory_region m_vga_rom;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	u32 vga_3b0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3c0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u32 vga_3d0_r(offs_t offset, uint32_t mem_mask = ~0);
	void vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	u32 unmap_log_r(offs_t offset, u32 mem_mask = ~0);
	void unmap_log_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	bool m_vga_legacy_enable = false;
	u32 m_main_scratchpad_id = 0;
};

DECLARE_DEVICE_TYPE(RIVA128, riva128_device)

#endif // MAME_VIDEO_RIVA128_H
