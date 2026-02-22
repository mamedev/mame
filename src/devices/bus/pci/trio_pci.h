// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_TRIO_PCI_H
#define MAME_BUS_PCI_TRIO_PCI_H

#pragma once

#include "pci_slot.h"
#include "video/pc_vga_s3.h"

class trio64dx_pci_device : public pci_card_device
{
public:
	template <typename T>
	trio64dx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: trio64dx_pci_device(mconfig, tag, owner, clock)
	{
		set_screen_tag(std::forward<T>(screen_tag));
	}

	trio64dx_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	trio64dx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);

	void io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void config_map(address_map &map) override ATTR_COLD;

	void mmio_map(address_map &map);

private:
	required_device<s3trio64_vga_device> m_vga;
	required_device<ibm8514a_device> m_8514;
	required_memory_region m_bios;
	optional_device<screen_device> m_screen;
};

DECLARE_DEVICE_TYPE(TRIO64DX_PCI, trio64dx_pci_device)

#endif
