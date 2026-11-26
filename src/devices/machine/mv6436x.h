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


// forward declaration
class device_mv64361_pci_function_interface;
class mv64361_pci_host_device;


//**************************************************************************
//  MV64361 SYSTEM CONTROLLER
//**************************************************************************

class mv64361_device : public device_t
{
public:
	// construction/destruction
	template <typename T, typename U, typename V>
	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&pcibus0_tag, V &&pcibus1_tag):
		mv64361_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_pci_bus_tag(0, std::forward<U>(pcibus0_tag));
		set_pci_bus_tag(1, std::forward<V>(pcibus1_tag));
	}

	template <typename T, typename U, typename V>
	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&pcibus0_tag, V &&pcibus1_tag):
		mv64361_device(mconfig, tag, owner)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_pci_bus_tag(0, std::forward<U>(pcibus0_tag));
		set_pci_bus_tag(1, std::forward<V>(pcibus1_tag));
	}

	template <typename T>
	mv64361_device &set_cs_tag(int cs, T &&tag)
	{
		m_cs_tag[cs] = std::forward<T>(tag);
		return *this;
	}

	mv64361_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_pci_bus_tag(int idx, T &&tag) { m_pci_bus[idx].set_tag(std::forward<T>(tag)); }

	auto cpuint0_cb() { return m_cpuint0_cb.bind(); }
	auto cpuint1_cb() { return m_cpuint1_cb.bind(); }

	template <unsigned Bus> auto pci_iack_cb() { return m_pci_iack_cb[Bus].bind(); }

	void mpp31_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

private:
	void cpu_register_map(address_map &map) ATTR_COLD;
	void pci_register_map(address_map &map) ATTR_COLD;

	// internal register cpu interface
	uint32_t cpu_config_r(offs_t offset, uint32_t mem_mask);
	void cpu_config_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs0_base_r(offs_t offset, uint32_t mem_mask);
	void cs0_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs0_size_r(offs_t offset, uint32_t mem_mask);
	void cs0_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs1_base_r(offs_t offset, uint32_t mem_mask);
	void cs1_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs1_size_r(offs_t offset, uint32_t mem_mask);
	void cs1_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs2_base_r(offs_t offset, uint32_t mem_mask);
	void cs2_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs2_size_r(offs_t offset, uint32_t mem_mask);
	void cs2_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs3_base_r(offs_t offset, uint32_t mem_mask);
	void cs3_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cs3_size_r(offs_t offset, uint32_t mem_mask);
	void cs3_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
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

	uint32_t pci0_cs0_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs0_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs2_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs2_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs0_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs0_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs2_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs2_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs1_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs1_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs3_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs3_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs1_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs1_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs3_bar_size_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs3_bar_size_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_base_address_registers_enable_r(offs_t offset, uint32_t mem_mask);
	void pci0_base_address_registers_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_base_address_registers_enable_r(offs_t offset, uint32_t mem_mask);
	void pci1_base_address_registers_enable_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs0_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs0_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs0_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs0_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs1_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs1_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs1_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs1_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs2_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs2_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs2_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs2_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_cs3_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci0_cs3_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_cs3_base_address_remap_r(offs_t offset, uint32_t mem_mask);
	void pci1_cs3_base_address_remap_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_pci_cpu_base_address_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci0_pci_cpu_base_address_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_pci_cpu_base_address_remap_low_r(offs_t offset, uint32_t mem_mask);
	void pci1_pci_cpu_base_address_remap_low_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_pci_cpu_base_address_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci0_pci_cpu_base_address_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_pci_cpu_base_address_remap_high_r(offs_t offset, uint32_t mem_mask);
	void pci1_pci_cpu_base_address_remap_high_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_pci_address_decode_control_r(offs_t offset, uint32_t mem_mask);
	void pci0_pci_address_decode_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_pci_address_decode_control_r(offs_t offset, uint32_t mem_mask);
	void pci1_pci_address_decode_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci0_command_r(offs_t offset, uint32_t mem_mask);
	void pci0_command_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pci1_command_r(offs_t offset, uint32_t mem_mask);
	void pci1_command_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t gpp_io_control_r(offs_t offset, uint32_t mem_mask);
	void gpp_io_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_level_control_r(offs_t offset, uint32_t mem_mask);
	void gpp_level_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_value_r(offs_t offset, uint32_t mem_mask);
	void gpp_value_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_interrupt_cause_r(offs_t offset, uint32_t mem_mask);
	void gpp_interrupt_cause_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_interrupt_mask0_r(offs_t offset, uint32_t mem_mask);
	void gpp_interrupt_mask0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_interrupt_mask1_r(offs_t offset, uint32_t mem_mask);
	void gpp_interrupt_mask1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_value_set_r(offs_t offset, uint32_t mem_mask);
	void gpp_value_set_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t gpp_value_clear_r(offs_t offset, uint32_t mem_mask);
	void gpp_value_clear_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cunit_arbiter_control_r(offs_t offset, uint32_t mem_mask);
	void cunit_arbiter_control_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void update_interrupts();

	void map_window(int window);
	void unmap_window(int window);
	void update_window(int window);
	void update_windows();

	offs_t remap_offset(offs_t offset, uint32_t size, uint32_t remap, unsigned shift);
	offs_t cpu_remap_offset(offs_t offset, uint32_t size, uint32_t remap);
	offs_t pci_remap_offset(offs_t offset, uint32_t size, uint32_t remap);

	void pci_io_delay(uint8_t pci_bus, uint32_t pci_cycles);

	// cs sdram access from pci devices
	offs_t pci_cs_offset(uint8_t pci_bus, uint8_t cs, offs_t offset);
	uint32_t pci_cs_r(uint8_t pci_bus, uint8_t cs, offs_t offset, uint32_t mem_mask);
	void pci_cs_w(uint8_t pci_bus, uint8_t cs, offs_t offset, uint32_t data, uint32_t mem_mask);

	// shared config registers
	void function_command_w(uint8_t pci_bus, uint8_t pci_function, offs_t offset, uint16_t data, uint16_t mem_mask);

	bool remap_write_disabled(uint8_t pci_bus) const;
	bool pci_loopback_enabled(uint8_t pci_bus) const;

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

	template <int Bus> uint32_t config_address_r(offs_t offset, uint32_t mem_mask);
	template <int Bus> void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Bus> uint32_t config_data_r(offs_t offset, uint32_t mem_mask);
	template <int Bus> void config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	template <int Bus> uint32_t pci_interrupt_acknowledge_r();

	required_device<ppc_device> m_cpu;
	required_device_array<pci_root_device, 2> m_pci_bus;
	required_device_array<device_mv64361_pci_function_interface, 2> m_pci_functions[2];
	std::string m_pci_function_tag[2][2];

	address_space *m_cpu_space;

	// internal registers
	enum
	{
		// CPU interface
		REG_CS0_BASE = 0,
		REG_CS0_SIZE,
		REG_CS1_BASE,
		REG_CS1_SIZE,
		REG_CS2_BASE,
		REG_CS2_SIZE,
		REG_CS3_BASE,
		REG_CS3_SIZE,
		REG_BOOTCS_BASE,
		REG_BOOTCS_SIZE,
		REG_PCI0_IO_BASE,
		REG_PCI0_IO_SIZE,
		REG_PCI0_MEM0_BASE,
		REG_PCI0_MEM0_SIZE,
		REG_PCI0_MEM1_BASE,
		REG_PCI0_MEM1_SIZE,
		REG_PCI0_MEM2_BASE,
		REG_PCI0_MEM2_SIZE,
		REG_PCI0_MEM3_BASE,
		REG_PCI0_MEM3_SIZE,
		REG_PCI1_IO_BASE,
		REG_PCI1_IO_SIZE,
		REG_PCI1_MEM0_BASE,
		REG_PCI1_MEM0_SIZE,
		REG_PCI1_MEM1_BASE,
		REG_PCI1_MEM1_SIZE,
		REG_PCI1_MEM2_BASE,
		REG_PCI1_MEM2_SIZE,
		REG_PCI1_MEM3_BASE,
		REG_PCI1_MEM3_SIZE,
		REG_SRAM_BASE,
		REG_INTERNAL_BASE,
		REG_BASE_ADDRESS_ENABLE,
		REG_PCI0_IO_REMAP,
		REG_PCI0_MEM0_REMAP_LOW,
		REG_PCI0_MEM0_REMAP_HIGH,
		REG_PCI0_MEM1_REMAP_LOW,
		REG_PCI0_MEM1_REMAP_HIGH,
		REG_PCI0_MEM2_REMAP_LOW,
		REG_PCI0_MEM2_REMAP_HIGH,
		REG_PCI0_MEM3_REMAP_LOW,
		REG_PCI0_MEM3_REMAP_HIGH,
		REG_PCI1_IO_REMAP,
		REG_PCI1_MEM0_REMAP_LOW,
		REG_PCI1_MEM0_REMAP_HIGH,
		REG_PCI1_MEM1_REMAP_LOW,
		REG_PCI1_MEM1_REMAP_HIGH,
		REG_PCI1_MEM2_REMAP_LOW,
		REG_PCI1_MEM2_REMAP_HIGH,
		REG_PCI1_MEM3_REMAP_LOW,
		REG_PCI1_MEM3_REMAP_HIGH,
		REG_CPU_CONFIG,
		// PCI interface
		REG_PCI0_CS0_BAR_SIZE,
		REG_PCI1_CS0_BAR_SIZE,
		REG_PCI0_CS1_BAR_SIZE,
		REG_PCI1_CS1_BAR_SIZE,
		REG_PCI0_CS2_BAR_SIZE,
		REG_PCI1_CS2_BAR_SIZE,
		REG_PCI0_CS3_BAR_SIZE,
		REG_PCI1_CS3_BAR_SIZE,
		REG_PCI0_BASE_ADDRESS_REGISTERS_ENABLE,
		REG_PCI1_BASE_ADDRESS_REGISTERS_ENABLE,
		REG_PCI0_CS0_BASE_ADDRESS_REMAP,
		REG_PCI1_CS0_BASE_ADDRESS_REMAP,
		REG_PCI0_CS1_BASE_ADDRESS_REMAP,
		REG_PCI1_CS1_BASE_ADDRESS_REMAP,
		REG_PCI0_CS2_BASE_ADDRESS_REMAP,
		REG_PCI1_CS2_BASE_ADDRESS_REMAP,
		REG_PCI0_CS3_BASE_ADDRESS_REMAP,
		REG_PCI1_CS3_BASE_ADDRESS_REMAP,
		REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_LOW,
		REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_LOW,
		REG_PCI0_PCI_CPU_BASE_ADDRESS_REMAP_HIGH,
		REG_PCI1_PCI_CPU_BASE_ADDRESS_REMAP_HIGH,
		REG_PCI0_PCI_ADDRESS_DECODE_CONTROL,
		REG_PCI1_PCI_ADDRESS_DECODE_CONTROL,
		REG_PCI0_COMMAND,
		REG_PCI1_COMMAND,
		// Communication Unit interface
		REG_CUNIT_ARBITER_CONTROL,
		// General Purpose Port interface
		REG_GPP_IO_CONTROL,
		REG_GPP_LEVEL_CONTROL,
		REG_GPP_VALUE,
		REG_GPP_INTERRUPT_CAUSE,
		REG_GPP_INTERRUPT_MASK0,
		REG_GPP_INTERRUPT_MASK1
	};

	uint32_t m_regs[REG_GPP_INTERRUPT_MASK1 + 1];

	std::unique_ptr<uint8_t[]> m_sram;
	std::string m_cs_tag[4];
	memory_share *m_cs_share[4];

	// helper to keep track of what we installed into cpu space
	struct decode_range
	{
		uint32_t start;
		uint32_t end;
		bool enabled;
	};

	decode_range m_ranges[21];

	devcb_write_line m_cpuint0_cb;
	devcb_write_line m_cpuint1_cb;
	devcb_read8::array<2> m_pci_iack_cb;

	friend class mv64361_pci_host_device;
	friend class mv64361_pci_function1_device;
};


//**************************************************************************
//  MV64361 PCI DEVICES
//**************************************************************************

class device_mv64361_pci_function_interface : public device_interface
{
public:
	device_mv64361_pci_function_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_mv64361_pci_function_interface();

	void set_syscon(mv64361_device &syscon, uint8_t pci_bus, uint8_t pci_function);

	// write to command from other functions
	virtual void function_command_w(offs_t offset, uint16_t data, uint16_t mem_mask) = 0;

	virtual void set_bar_size(int bar, uint64_t size) = 0;
	virtual void set_bar_enable(uint16_t enable) = 0;

protected:
	mv64361_device *m_syscon = nullptr;

	uint8_t m_pci_bus = 0;
	uint8_t m_pci_function = 0;
};

class mv64361_pci_host_device : public pci_host_device, public device_mv64361_pci_function_interface
{
public:
	enum
	{
		AS_PCI_CPU_MEM = 1,
		AS_PCI_IO = 2,
		AS_PCI_BUSMASTER_MEM = 3
	};

	mv64361_pci_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint32_t config_address_r(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;
	uint32_t config_data_r(offs_t offset, uint32_t mem_mask = ~0);
	void config_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t io_r(offs_t offset, uint32_t mem_mask);
	void io_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t mem_r(offs_t offset, uint32_t mem_mask);
	void mem_w(offs_t offset, uint32_t data, uint32_t mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	virtual void function_command_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;

	virtual void set_bar_size(int bar, uint64_t size) override;
	virtual void set_bar_enable(uint16_t enable) override;

private:
	void command_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	virtual uint8_t cache_line_size_r() override;
	void cache_line_size_w(uint8_t data);
	virtual uint8_t latency_timer_r() override;
	void latency_timer_w(uint8_t data);
	virtual uint8_t header_type_r() override;
	void header_type_w(uint8_t data);
	virtual uint8_t bist_r() override;
	void bist_w(uint8_t data);
	uint32_t address_base_r(offs_t offset, uint32_t mem_mask);
	void address_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void cs0_map(address_map &map) ATTR_COLD;
	void cs1_map(address_map &map) ATTR_COLD;

	template <int CS> uint32_t cs_r(offs_t offset, uint32_t mem_mask);
	template <int CS> void cs_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	address_space_config m_mem_config;
	address_space_config m_io_config;
	address_space_config m_busmaster_config;

	uint8_t m_cache_line_size;
	uint8_t m_latency_timer;
	uint8_t m_header_type;
	uint8_t m_bist;
};


class mv64361_pci_function1_device : public pci_device, public device_mv64361_pci_function_interface
{
public:
	mv64361_pci_function1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void function_command_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;

	virtual void set_bar_size(int bar, uint64_t size) override;
	virtual void set_bar_enable(uint16_t enable) override;

private:
	void command_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t address_base_r(offs_t offset, uint32_t mem_mask);
	void address_base_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void cs2_map(address_map &map) ATTR_COLD;
	void cs3_map(address_map &map) ATTR_COLD;
	void sram_map(address_map &map) ATTR_COLD;

	template <int CS> uint32_t cs_r(offs_t offset, uint32_t mem_mask);
	template <int CS> void cs_w(offs_t offset, uint32_t data, uint32_t mem_mask);
};


// device type definition
DECLARE_DEVICE_TYPE(MV64361, mv64361_device)
DECLARE_DEVICE_TYPE(MV64361_PCI_HOST, mv64361_pci_host_device)
DECLARE_DEVICE_TYPE(MV64361_PCI_FUNCTION1, mv64361_pci_function1_device)

#endif // MAME_MACHINE_MV6436X_H
