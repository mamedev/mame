// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_VT6306_H
#define MAME_BUS_PCI_VT6306_H

#pragma once

#include "pci_slot.h"

class vt6306_device : public pci_card_device
{
public:
	vt6306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vt6306_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	void ohci_mmio_map(address_map &map) ATTR_COLD;
	void vio_map(address_map &map) ATTR_COLD;

	virtual u8 capptr_r() override;

};

DECLARE_DEVICE_TYPE(VT6306_PCI, vt6306_device)

#endif // MAME_BUS_PCI_VT6306_H
