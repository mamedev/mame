// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    VIA VT82C505 ISA/VL PCI bridge

*/

#ifndef MAME_MACHINE_VT82C505_H
#define MAME_MACHINE_VT82C505_H

#pragma once

#include "pci.h"

class vt82c505_device : public pci_host_device
{
public:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	template <typename T>
	vt82c505_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: vt82c505_device(mconfig, tag, owner)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	vt82c505_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	u32 config_80_r();
	void config_84_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void config_88_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	required_device<cpu_device> m_host_cpu;

	address_space_config m_pci_mem_config;
	address_space_config m_pci_io_config;

	uint16_t m_window_addr[3];
	uint8_t m_window_attr[3];
};

DECLARE_DEVICE_TYPE(VT82C505, vt82c505_device)

#endif // MAME_MACHINE_VT82C505_H
