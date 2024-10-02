// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_BUS_LPCI_NORTHBRIDGE_H
#define MAME_BUS_LPCI_NORTHBRIDGE_H

#pragma once


#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> northbridge_device

class northbridge_device : public device_t
{
public:
	template <typename T>
	void set_cpu(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

protected:
	// construction/destruction
	northbridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<cpu_device> m_cpu;
	required_device<ram_device> m_ram;
};

#endif // MAME_BUS_LPCI_NORTHBRIDGE_H
