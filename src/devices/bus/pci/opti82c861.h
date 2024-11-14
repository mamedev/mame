// license:BSD-3-Clause
// copyright-holders: R. Belmont

#ifndef MAME_BUS_PCI_OPTI82C861_H
#define MAME_BUS_PCI_OPTI82C861_H

#pragma once

#include "pci_slot.h"

class opti_82c861_device : public pci_card_device
{
public:
	opti_82c861_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	opti_82c861_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mem_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
				   u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;
	void config_map(address_map &map) override;

private:
	u32 m_HcRhDescriptorA = 0;
};

DECLARE_DEVICE_TYPE(OPTI_82C861, opti_82c861_device)

#endif
