// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese
#ifndef MAME_VIDEO_MGA2064W_H
#define MAME_VIDEO_MGA2064W_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga_matrox.h"

class mga2064w_device : public pci_device {
public:
	mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	void mgabase1_map(address_map &map);
	void mgabase2_map(address_map &map);

	required_device<matrox_vga_device> m_svga;
	required_memory_region m_vga_rom;
private:
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(MGA2064W, mga2064w_device);

#endif // MAME_VIDEO_MGA2064W_H
