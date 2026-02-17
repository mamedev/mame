// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_VT82C598MVP_H
#define MAME_MACHINE_VT82C598MVP_H

#pragma once

#include "pci.h"

class vt82c598mvp_host_device : public pci_host_device
{
public:
	template <typename T> vt82c598mvp_host_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&cpu_tag, int ram_size
	) : vt82c598mvp_host_device(mconfig, tag, owner, clock)
	{
		set_ids(0x11060598, 0x00, 0x060000, 0x00);
		//set_multifunction_device(true);
		m_host_cpu.set_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}

	vt82c598mvp_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_ram_size(int ram_size) { m_ram_size = ram_size; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

   	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<cpu_device> m_host_cpu;
	std::vector<uint32_t> m_ram;

	u32 m_ram_size = 0;

	void map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting);

	u8 m_dram_type;
	u8 m_shadow_ram_control[3];
};

// TODO: bridge

DECLARE_DEVICE_TYPE(VT82C598MVP_HOST, vt82c598mvp_host_device)

#endif // MAME_MACHINE_VT82C598MVP_H
