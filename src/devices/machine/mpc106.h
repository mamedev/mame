// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc106.h - Motorola MPC-106 PCI host bridge, aka "Grackle".

**********************************************************************/

#ifndef MAME_MACHINE_MPC106_H
#define MAME_MACHINE_MPC106_H

#pragma once

#include "pci.h"

class mpc106_host_device : public pci_host_device {
public:
	typedef enum
	{
		MAP_TYPE_A,     // Type A is PowerPC Reference Platform (PReP)
		MAP_TYPE_B      // Type B is Common Hardware Reference Platform (CHRP)
	} map_type;

	template <typename T>
	mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, map_type map, T &&cpu_tag, const char *rom_tag)
		: mpc106_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10570002, 0x00, 0x00000000);
		set_map_type(map);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_rom_tag(rom_tag);
	}
	mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_info(u8 *ram_ptr, int ram_size);
	void set_rom_tag(const char *tag);
	void set_map_type(map_type maptype);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
						   u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	void access_map_le(address_map &map) ATTR_COLD;
	void access_map_be(address_map &map) ATTR_COLD;
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

	u16 pwrconfig1_r();
	void pwrconfig1_w(offs_t offset, u16 data, u16 mem_mask);
	u8 pwrconfig2_r();
	void pwrconfig2_w(offs_t offset, u8 data);
	u32 memory_start_r(offs_t offset);
	void memory_start_w(offs_t offset, u32 data, u32 mem_mask);
	u32 memory_end_r(offs_t offset);
	void memory_end_w(offs_t offset, u32 data, u32 mem_mask);
	u8 memory_enable_r();
	void memory_enable_w(offs_t offset, u8 data);
	u32 picr1_r();
	void picr1_w(u32 data);

	address_space_config m_mem_config, m_io_config;
	const char *m_rom_tag;
	u8 *m_ram;
	int m_ram_size;
	map_type m_map_type;
	required_device<device_memory_interface> m_cpu;
	u8 *m_rom;
	u32 m_rom_size;
	address_space *m_cpu_space;
	u16 m_pwrconfig1;
	u8 m_pwrconfig2;
	u32 m_memory_starts[4];
	u32 m_memory_ends[4];
	u32 m_picr1;
	u8 m_memory_bank_enable;
};

DECLARE_DEVICE_TYPE(MPC106, mpc106_host_device)

#endif // MAME_MACHINE_MPC106_H
