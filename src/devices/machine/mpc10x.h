// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Shared Motorola MPC105/MPC106 PCI host bridge implementation.

**********************************************************************/

#ifndef MAME_MACHINE_MPC10X_H
#define MAME_MACHINE_MPC10X_H

#pragma once

#include "pci.h"

#include <vector>

class mpc10x_host_device : public pci_host_device
{
public:
	enum map_type
	{
		MAP_TYPE_A,     // PowerPC Reference Platform (PReP)
		MAP_TYPE_B      // Chip-specific: only the MPC106 layout is CHRP
	};

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_secondary_cpu_tag(T &&tag) { m_secondary_cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_info(u8 *ram_ptr, int ram_size);
	void set_rom_tag(const char *tag);
	void set_map_type(map_type maptype);

	virtual void config_map(address_map &map) override ATTR_COLD;

protected:
	enum
	{
		AS_PCI_MEM = 1,
		AS_PCI_IO = 2
	};

	static constexpr u32 PICR1_LE_MODE = 0x0000'0020;

	mpc10x_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void install_cpu_map(address_space &space) ATTR_COLD = 0;
	virtual void install_config_access_map(address_space &space) = 0;
	void update_config_access();
	virtual void install_ram();
	void install_memory_banks(bool skip_ff, bool wrap_banks);
	virtual void memory_start_w(offs_t offset, u32 data, u32 mem_mask);
	virtual void memory_end_w(offs_t offset, u32 data, u32 mem_mask);

	u32 be_config_address_r();
	void be_config_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 be_config_data_r(offs_t offset, u32 mem_mask = ~0);
	void be_config_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <u32 Base> u32 cpu_memory_r(offs_t offset, u32 mem_mask);
	template <u32 Base> void cpu_memory_w(offs_t offset, u32 data, u32 mem_mask);
	template <u32 Base> u32 pci_memory_r(offs_t offset, u32 mem_mask);
	template <u32 Base> void pci_memory_w(offs_t offset, u32 data, u32 mem_mask);
	template <u32 Base> u32 pci_io_r(offs_t offset, u32 mem_mask);
	template <u32 Base> void pci_io_w(offs_t offset, u32 data, u32 mem_mask);

	u32 picr1_r();

	int m_ram_size;
	map_type m_map_type;
	address_space *m_cpu_space;
	std::vector<address_space *> m_cpu_spaces;
	u16 m_pwrconfig1;
	u32 m_memory_starts[4];
	u32 m_memory_ends[4];
	u32 m_picr1;
	u8 m_memory_bank_enable;

private:
	u16 pwrconfig1_r();
	void pwrconfig1_w(offs_t offset, u16 data, u16 mem_mask);
	u32 memory_start_r(offs_t offset);
	u32 memory_end_r(offs_t offset);
	u8 memory_enable_r();
	void memory_enable_w(u8 data);

	address_space_config m_mem_config, m_io_config;
	required_device<device_memory_interface> m_cpu;
	optional_device<device_memory_interface> m_secondary_cpu;
	optional_memory_region m_rom;
	u8 *m_ram;
};

#endif // MAME_MACHINE_MPC10X_H
