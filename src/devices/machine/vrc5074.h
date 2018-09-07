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

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void reset_all_mappings() override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;
	virtual void device_post_load() override;

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_sdram_size(int index, int size) { m_sdram_size[index] = size; };

	void set_map(int id, const address_map_constructor &map, device_t *device);

	virtual void config_map(address_map &map) override;
	DECLARE_READ32_MEMBER(sdram_addr_r);
	DECLARE_WRITE32_MEMBER(sdram_addr_w);

	// PCI interrupts
	DECLARE_WRITE_LINE_MEMBER(pci_intr_a);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_b);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_c);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_d);
	DECLARE_WRITE_LINE_MEMBER(pci_intr_e);
	void update_pci_irq(const int index, const int state);

	//cpu bus registers
	DECLARE_READ32_MEMBER (cpu_reg_r);
	DECLARE_WRITE32_MEMBER(cpu_reg_w);
	DECLARE_READ32_MEMBER(serial_r);
	DECLARE_WRITE32_MEMBER(serial_w);
	void update_nile_irqs();

	DECLARE_READ32_MEMBER (pci0_r);
	DECLARE_WRITE32_MEMBER(pci0_w);

	DECLARE_READ32_MEMBER (pci1_r);
	DECLARE_WRITE32_MEMBER(pci1_w);

	virtual void target1_map(address_map &map);
	DECLARE_READ32_MEMBER (target1_r);
	DECLARE_WRITE32_MEMBER(target1_w);

	// Serial port
	DECLARE_WRITE_LINE_MEMBER(uart_irq_callback);

protected:
	address_space *m_cpu_space;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	required_device<mips3_device> m_cpu;
	int m_sdram_size[2];

	address_space_config m_mem_config, m_io_config;

	void cpu_map(address_map &map);
	void serial_map(address_map &map);

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
