// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc106.h - Motorola MPC-106 PCI host bridge, aka "Grackle".

**********************************************************************/

#ifndef MAME_MACHINE_MPC106_H
#define MAME_MACHINE_MPC106_H

#pragma once

#include "mpc10x.h"

class mpc106_host_device : public mpc10x_host_device
{
public:
	template <typename T>
	mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, map_type map, T &&cpu_tag, const char *rom_tag)
		: mpc106_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10570002, 0x00, 0x00000000);
		set_map_type(map);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_rom_tag(rom_tag);
	}
	template <typename T>
	mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, map_type map, T &&cpu_tag, const char *rom_tag)
		: mpc106_host_device(mconfig, tag, owner, 0, map, std::forward<T>(cpu_tag), rom_tag)
	{
		set_ids_host(0x10570002, 0x00, 0x00000000);
		set_map_type(map);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_rom_tag(rom_tag);
	}
	mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
						   u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void install_cpu_map(address_space &space) override ATTR_COLD;
	virtual void install_config_access_map(address_space &space) override;

private:
	void access_map_le(address_map &map) ATTR_COLD;
	void access_map_be(address_map &map) ATTR_COLD;
	u8 pwrconfig2_r();
	void pwrconfig2_w(offs_t offset, u8 data);
	void picr1_w(u32 data);

	u8 m_pwrconfig2;
};

DECLARE_DEVICE_TYPE(MPC106, mpc106_host_device)

#endif // MAME_MACHINE_MPC106_H
