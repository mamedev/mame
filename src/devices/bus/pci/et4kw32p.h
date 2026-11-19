// license:BSD-3-Clause
// copyright-holders: Miodrag Milanovic

#ifndef MAME_BUS_PCI_ET4KW32P_H
#define MAME_BUS_PCI_ET4KW32P_H

#pragma once

#include "pci_slot.h"

#include "video/pc_vga_tseng.h"


class et4kw32p_pci_device : public pci_card_device
{
public:
	et4kw32p_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	void legacy_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

private:
	required_device<et4kw32i_vga_device> m_vga;

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(ET4KW32P_PCI, et4kw32p_pci_device)

#endif // MAME_BUS_PCI_ET4KW32P_H
