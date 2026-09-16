// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_VIDEO_GF7600GS_H
#define MAME_VIDEO_GF7600GS_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga_nvidia.h"

// FIXME: PCIe x16
class geforce_7600gs_device : public pci_device {
public:
	geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id)
		: geforce_7600gs_device(mconfig, tag, owner, clock)
	{
		set_ids_agp(0x10de02e1, 0xa1, subdevice_id);
	}
	geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void legacy_memory_map(address_map &map) ATTR_COLD;
	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	void map1(address_map &map) ATTR_COLD;
	void map2(address_map &map) ATTR_COLD;
	void map3(address_map &map) ATTR_COLD;

	required_device<nvidia_nv3_vga_device> m_vga;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device)

#endif // MAME_VIDEO_GF7600GS_H
