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
	template <typename T, typename U, typename V>
	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&pcihost0_tag, V &&pcihost1_tag):
		mv64361_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_pcihost_tag(0, std::forward<U>(pcihost0_tag));
		set_pcihost_tag(1, std::forward<V>(pcihost1_tag));
	}

	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_pcihost_tag(int idx, T &&tag) { m_pcihost[idx].set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void register_map(address_map &map) ATTR_COLD;

	// internal register cpu interface
	uint32_t cpu_config_r(offs_t offset, uint32_t mem_mask);
	void cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_io_base_r(offs_t offset, uint32_t mem_mask);
	void pci0_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_io_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem0_base_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem0_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t internal_base_r(offs_t offset, uint32_t mem_mask);
	void internal_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem1_base_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem1_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_io_base_r(offs_t offset, uint32_t mem_mask);
	void pci1_io_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_io_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_io_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem0_base_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem0_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem1_base_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem1_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_io_remap_r(offs_t offset, uint32_t mem_mask);
	void pci0_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem0_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem1_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_io_remap_r(offs_t offset, uint32_t mem_mask);
	void pci1_io_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem0_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem0_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem1_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem1_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t bootcs_base_r(offs_t offset, uint32_t mem_mask);
	void bootcs_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t bootcs_size_r(offs_t offset, uint32_t mem_mask);
	void bootcs_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem2_base_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem2_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sram_base_r(offs_t offset, uint32_t mem_mask);
	void sram_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t base_address_enable_r(offs_t offset, uint32_t mem_mask);
	void base_address_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem3_base_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem3_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem2_base_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem2_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem3_base_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem3_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem2_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem3_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem2_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem2_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem3_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem3_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem0_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem1_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem2_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem3_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem0_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem0_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem1_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem1_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem2_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem2_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem3_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem3_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void map_windows();

	uint32_t remap_offset(offs_t offset, uint16_t size, uint16_t remap);

	// pci windows installed into cpu memory space
	uint32_t pci0_io_r(offs_t offset, uint32_t mem_mask);
	void pci0_io_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem0_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem1_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem2_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_mem3_r(offs_t offset, uint32_t mem_mask);
	void pci0_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_io_r(offs_t offset, uint32_t mem_mask);
	void pci1_io_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem0_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem1_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem2_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_mem3_r(offs_t offset, uint32_t mem_mask);
	void pci1_mem3_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	required_device<ppc_device> m_cpu;
	required_device_array<mv64361_pci_host_device, 2> m_pcihost;

	address_space *m_cpu_space;

	// internal registers
	enum
	{
		REG_CPU_CONFIG = 0,
		REG_PCI0_IO_BASE,
		REG_PCI0_IO_SIZE,
		REG_PCI0_IO_REMAP,
		REG_PCI0_MEM0_BASE,
		REG_PCI0_MEM0_SIZE,
		REG_PCI0_MEM0_REMAP_LOW,
		REG_PCI0_MEM0_REMAP_HIGH,
		REG_PCI0_MEM1_BASE,
		REG_PCI0_MEM1_SIZE,
		REG_PCI0_MEM1_REMAP_LOW,
		REG_PCI0_MEM1_REMAP_HIGH,
		REG_PCI0_MEM2_BASE,
		REG_PCI0_MEM2_SIZE,
		REG_PCI0_MEM2_REMAP_LOW,
		REG_PCI0_MEM2_REMAP_HIGH,
		REG_PCI0_MEM3_BASE,
		REG_PCI0_MEM3_SIZE,
		REG_PCI0_MEM3_REMAP_LOW,
		REG_PCI0_MEM3_REMAP_HIGH,
		REG_PCI1_IO_BASE,
		REG_PCI1_IO_SIZE,
		REG_PCI1_IO_REMAP,
		REG_PCI1_MEM0_BASE,
		REG_PCI1_MEM0_SIZE,
		REG_PCI1_MEM0_REMAP_LOW,
		REG_PCI1_MEM0_REMAP_HIGH,
		REG_PCI1_MEM1_BASE,
		REG_PCI1_MEM1_SIZE,
		REG_PCI1_MEM1_REMAP_LOW,
		REG_PCI1_MEM1_REMAP_HIGH,
		REG_PCI1_MEM2_BASE,
		REG_PCI1_MEM2_SIZE,
		REG_PCI1_MEM2_REMAP_LOW,
		REG_PCI1_MEM2_REMAP_HIGH,
		REG_PCI1_MEM3_BASE,
		REG_PCI1_MEM3_SIZE,
		REG_PCI1_MEM3_REMAP_LOW,
		REG_PCI1_MEM3_REMAP_HIGH,
		REG_INTERNAL_BASE,
		REG_BOOTCS_BASE,
		REG_BOOTCS_SIZE,
		REG_SRAM_BASE,
		REG_BASE_ADDRESS_ENABLE
	};

	uint32_t m_regs[REG_BASE_ADDRESS_ENABLE + 1];

	std::unique_ptr<uint8_t[]> m_sram;

	// helper to keep track of what we installed into cpu space
	struct decode_range
	{
		uint32_t start;
		uint32_t end;
		bool enabled;
	};

	decode_range m_ranges[21];
};

// ======================> mv64361_pci_host_device

class mv64361_pci_host_device : public pci_host_device
{
public:
	// construction/destruction
	mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t config_address_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t config_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t io_r(offs_t offset, uint32_t mem_mask);
	void io_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t mem_r(offs_t offset, uint32_t mem_mask);
	void mem_w(offs_t offset, uint32_t data, uint32_t mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual space_config_vector memory_space_config() const override;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	address_space_config m_mem_config;
	address_space_config m_io_config;
};

// device type definition
DECLARE_DEVICE_TYPE(MV64361, mv64361_device)
DECLARE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device)

#endif // MAME_MACHINE_MV6436X_H
