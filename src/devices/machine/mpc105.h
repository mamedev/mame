// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Motorola MPC105 PCI bridge/memory controller (Eagle).

**********************************************************************/

#ifndef MAME_MACHINE_MPC105_H
#define MAME_MACHINE_MPC105_H

#pragma once

#include "mpc10x.h"

class mpc105_host_device : public mpc10x_host_device
{
public:
	template <typename T>
	mpc105_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, map_type map, T &&cpu_tag, const char *rom_tag)
		: mpc105_host_device(mconfig, tag, owner, clock)
	{
		set_map_type(map);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_rom_tag(rom_tag);
	}

	template <typename T>
	mpc105_host_device(const machine_config &mconfig, const char *tag, device_t *owner, map_type map, T &&cpu_tag, const char *rom_tag)
		: mpc105_host_device(mconfig, tag, owner, 0, map, std::forward<T>(cpu_tag), rom_tag)
	{
	}

	mpc105_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual u8 header_type_r() override;
	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
			u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void install_cpu_map(address_space &space) override ATTR_COLD;
	virtual void install_config_access_map(address_space &space) override;
	virtual void install_ram() override;
	virtual void memory_start_w(offs_t offset, u32 data, u32 mem_mask) override;
	virtual void memory_end_w(offs_t offset, u32 data, u32 mem_mask) override;

private:
	static constexpr u32 PICR1_XIO_MODE = 0x0008'0000;

	void access_map_a(address_map &map) ATTR_COLD;
	void access_map_b(address_map &map) ATTR_COLD;
	u32 config_address_a_r(offs_t offset, u32 mem_mask);
	void config_address_a_w(offs_t offset, u32 data, u32 mem_mask);
	u32 config_data_mp_r(address_space &space, offs_t offset, u32 mem_mask);
	u32 be_config_data_mp_r(address_space &space, offs_t offset, u32 mem_mask);
	u32 internal_config_r(address_space &cpu_space, offs_t offset, u32 mem_mask);
	void internal_config_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pci_io_a_r(offs_t offset, u32 mem_mask);
	void pci_io_a_w(offs_t offset, u32 data, u32 mem_mask);
	void picr1_w(offs_t offset, u32 data, u32 mem_mask);
	u32 picr2_r();
	void picr2_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mccr_r(offs_t offset);
	void mccr_w(offs_t offset, u32 data, u32 mem_mask);

	u32 m_picr2;
	u32 m_mccr[4];
};

DECLARE_DEVICE_TYPE(MPC105, mpc105_host_device)

#endif // MAME_MACHINE_MPC105_H
