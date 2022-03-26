// license:BSD-3-Clause
// copyright-holders:Ted Green
// NEC VRC 4373 System Controller

#ifndef MAME_MACHINE_VRC4373_H
#define MAME_MACHINE_VRC4373_H

#pragma once

#include "pci.h"
#include "cpu/mips/mips3.h"

class vrc4373_device : public pci_host_device {
public:
	template <typename T>
	vrc4373_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: vrc4373_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	vrc4373_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void device_post_load() override;

	auto irq_cb() { return m_irq_cb.bind(); }
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int size) { m_ram_size = size; }
	void set_simm0_size(int size) { m_simm0_size = size; }

	virtual void config_map(address_map &map) override;

	uint32_t pcictrl_r(offs_t offset, uint32_t mem_mask = ~0);
	void pcictrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	//cpu bus registers
	uint32_t cpu_if_r(offs_t offset, uint32_t mem_mask = ~0);
	void cpu_if_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master1_r(offs_t offset, uint32_t mem_mask = ~0);
	void master1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master2_r(offs_t offset, uint32_t mem_mask = ~0);
	void master2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t master_io_r(offs_t offset, uint32_t mem_mask = ~0);
	void master_io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void target1_map(address_map &map);
	uint32_t target1_r(offs_t offset, uint32_t mem_mask = ~0);
	void target1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void target2_map(address_map &map);
	uint32_t target2_r(offs_t offset, uint32_t mem_mask = ~0);
	void target2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(dma_transfer);

	address_space *m_cpu_space;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	void cpu_map(address_map &map);

	void map_cpu_space();

	devcb_write_line m_irq_cb;

	required_device<mips3_device> m_cpu;
	int m_ram_size;
	int m_simm0_size;

	address_space_config m_mem_config, m_io_config;

	std::vector<uint32_t> m_ram;

	std::vector<uint32_t> m_simm[4];

	uint32_t m_cpu_regs[0x7c];

	uint32_t m_pci1_laddr, m_pci2_laddr, m_pci_io_laddr;
	uint32_t m_target1_laddr, m_target2_laddr;

	required_memory_region m_romRegion;

	emu_timer* m_dma_timer;
};


DECLARE_DEVICE_TYPE(VRC4373, vrc4373_device)

#endif // MAME_MACHINE_VRC4373_H
