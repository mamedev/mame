// license:BSD-3-Clause
// copyright-holders: R. Belmont

#ifndef MAME_MACHINE_OPTI82C861_H
#define MAME_MACHINE_OPTI82C861_H

#pragma once

#include "machine/pci.h"

class opti_82c861_device : public pci_device
{
public:
	opti_82c861_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	opti_82c861_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mem_map(address_map &map);

protected:
	virtual void device_start() override;

	void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
				   u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;
	void config_map(address_map &map) override;

private:
};

DECLARE_DEVICE_TYPE(OPTI_82C861, opti_82c861_device)

#endif
