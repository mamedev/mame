// license:BSD-3-Clause
// copyright-holders: Aaron Giles, Ted Green
// NEC VRC 5074 System Controller

#ifndef MAME_MACHINE_VRC5074_H
#define MAME_MACHINE_VRC5074_H

#pragma once

#include "pci.h"
#include "cpu/mips/mips3.h"
#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"

class vrc5074_device : public pci_host_device {
public:
	template <typename T>
	vrc5074_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: vrc5074_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	vrc5074_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<ns16550_device> m_uart;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void device_post_load() override;

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_sdram_size(int index, int size) { m_sdram_size[index] = size; }

	void set_map(int id, const address_map_constructor &map, device_t *device);

	virtual void config_map(address_map &map) override ATTR_COLD;
	uint32_t sdram_addr_r();
	void sdram_addr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// PCI interrupts
	void pci_intr_a(int state);
	void pci_intr_b(int state);
	void pci_intr_c(int state);
	void pci_intr_d(int state);
	void pci_intr_e(int state);
	void update_pci_irq(const int index, const int state);

	//cpu bus registers
	uint32_t cpu_reg_r(offs_t offset);
	void cpu_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t serial_r(offs_t offset);
	void serial_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void update_nile_irqs();

	uint32_t pci0_r(offs_t offset, uint32_t mem_mask = ~0);
	void pci0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t pci1_r(offs_t offset, uint32_t mem_mask = ~0);
	void pci1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	virtual void target1_map(address_map &map) ATTR_COLD;
	uint32_t target1_r(offs_t offset, uint32_t mem_mask = ~0);
	void target1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// Serial port
	void uart_irq_callback(int state);

protected:
	address_space *m_cpu_space;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	required_device<mips3_device> m_cpu;
	int m_sdram_size[2];

	address_space_config m_mem_config, m_io_config;

	void cpu_map(address_map &map) ATTR_COLD;
	void serial_map(address_map &map) ATTR_COLD;

	void map_cpu_space();

	emu_timer* m_dma_timer;
	TIMER_CALLBACK_MEMBER(dma_transfer);
	emu_timer *m_timer[4];
	double m_timer_period[4];
	TIMER_CALLBACK_MEMBER(nile_timer_callback);

	required_memory_region m_romRegion;
	optional_memory_region m_updateRegion;
	std::vector<uint32_t> m_sdram[2];

	// Chip Select
	device_t *m_cs_devices[7];
	address_map_constructor m_cs_maps[7];

	uint32_t m_cpu_regs[0x1ff / 4];
	uint16_t m_nile_irq_state;
	int m_uart_irq;
	uint8_t m_irq_pins;

	void setup_pci_space();
	uint32_t m_pci_laddr[2], m_pci_mask[2], m_pci_type[2];
	uint32_t m_sdram_addr[2];

};


DECLARE_DEVICE_TYPE(VRC5074, vrc5074_device)

#endif // MAME_MACHINE_VRC5074_H
