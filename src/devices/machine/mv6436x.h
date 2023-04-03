// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Marvell MV64360/1/2

    System controller for PowerPC processors

***************************************************************************/

#ifndef MAME_MACHINE_MV6436X_H
#define MAME_MACHINE_MV6436X_H

#pragma once

#include "cpu/powerpc/ppc.h"
#include "pci.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class mv64361_pci_host_device;

// ======================> mv64361_device

class mv64361_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag):
		mv64361_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void register_map(address_map &map);

	uint32_t cpu_config_r(offs_t offset, uint32_t mem_mask);
	void cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	required_device<ppc_device> m_cpu;
	address_space *m_cpu_space;
};

// ======================> mv64361_pci_host_device

class mv64361_pci_host_device : public pci_host_device
{
public:
	// construction/destruction
	template <typename T>
	mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, int busnum):
		mv64361_pci_host_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		m_busnum = busnum;
	}

	mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual space_config_vector memory_space_config() const override;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	void pci_map(address_map &map);

	uint32_t be_config_address_r(offs_t offset, uint32_t mem_mask = ~0);
	void be_config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t be_config_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void be_config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t pci_io_r(offs_t offset, uint32_t mem_mask);
	void pci_io_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	required_device<ppc_device> m_cpu;

	address_space_config m_mem_config, m_io_config;
	address_space *m_cpu_space;

	int m_busnum;
};

// device type definition
DECLARE_DEVICE_TYPE(MV64361, mv64361_device)
DECLARE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device)

#endif // MAME_MACHINE_MV6436X_H
