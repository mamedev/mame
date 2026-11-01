// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_PCI_NCR53C810_H
#define MAME_BUS_PCI_NCR53C810_H

#pragma once

#include "pci_slot.h"
#include "machine/ncr53c810.h"

class ncr53c810_pci_device : public pci_card_device
{
public:
	ncr53c810_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// for machine configuration, before finders resolve
	ncr53c810_device &scripts() const { return *m_scripts.lookup(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
		u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;
	virtual u8 latency_timer_r() override;

private:
	void registers_map(address_map &map) ATTR_COLD;
	void command_w(offs_t offset, u16 data, u16 mem_mask);
	void command_changed();
	void status_w(offs_t offset, u16 data, u16 mem_mask);
	void latency_timer_w(u8 data);
	void bar_w(offs_t offset, u32 data, u32 mem_mask);
	u32 memory_r(offs_t address, u32 mem_mask);
	void memory_w(offs_t address, u32 data, u32 mem_mask);
	u32 io_r(offs_t address, u32 mem_mask);
	void io_w(offs_t address, u32 data, u32 mem_mask);
	void irq_w(int state);

	required_device<ncr53c810_device> m_scripts;
	address_space *m_io_space;
	u64 m_io_offset;
	u8 m_latency_timer;
};

DECLARE_DEVICE_TYPE(NCR53C810_PCI, ncr53c810_pci_device)

#endif // MAME_BUS_PCI_NCR53C810_H
