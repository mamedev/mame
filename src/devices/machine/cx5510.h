// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*

    Cyrix Cx5510 MediaGX companion chip

    Config space stub only. The MediaGX host bridge is not emulated, so this
    device also provides the configuration ports for the PCI bus.

*/

#ifndef MAME_MACHINE_CX5510_H
#define MAME_MACHINE_CX5510_H

#pragma once

#include "pci.h"

class cx5510_device : public pci_host_device
{
public:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	template <typename T>
	cx5510_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: cx5510_device(mconfig, tag, owner)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	cx5510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<cpu_device> m_host_cpu;

	address_space_config m_pci_mem_config;
	address_space_config m_pci_io_config;

	uint32_t m_regs[256/4];
};

DECLARE_DEVICE_TYPE(CX5510, cx5510_device)

#endif // MAME_MACHINE_CX5510_H
