// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_VT82C585VP_H
#define MAME_MACHINE_VT82C585VP_H

#pragma once

#include "pci.h"

class vt82c585vp_host_device : public pci_host_device
{
public:
	template <typename T> vt82c585vp_host_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&cpu_tag, int ram_size
	) : vt82c585vp_host_device(mconfig, tag, owner, clock)
	{
		//set_multifunction_device(true);
		m_host_cpu.set_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}

	vt82c585vp_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_ram_size(int ram_size) { m_ram_size = ram_size; }

	void smi_act_w(int state);

protected:
	vt82c585vp_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual bool map_first() const override { return true; }

	required_device<cpu_device> m_host_cpu;
	std::vector<uint32_t> m_ram;

	virtual u8 smi_bank() const { return 3; }

private:
	u32 m_ram_size = 0;

	void map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting);

	u8 m_cache_control_1;
	u8 m_cache_control_2;
	u8 m_noncache_control;
	u8 m_system_perf_control;
	u16 m_noncache_region[2];

	u16 m_dram_ma_map_type;
	u8 m_bank_ending[6];
	u8 m_dram_type;
	u8 m_shadow_ram_control[3];
	u8 m_dram_timing[4];
	u16 m_uma_control;
	u8 m_dram_control;
	u8 m_refresh_counter;
	u8 m_dram_arbitration_control;
	u8 m_sdram_control;
	u8 m_dram_drive_strength;

	u8 m_pci_buffer_control;
	u8 m_pci_flow_control[2];
	u8 m_pci_master_control[2];
	u8 m_pci_arbitration[2];

	int m_smiact;
};


DECLARE_DEVICE_TYPE(VT82C585VP_HOST,   vt82c585vp_host_device)

#endif // MAME_MACHINE_VT82C585VP_H
