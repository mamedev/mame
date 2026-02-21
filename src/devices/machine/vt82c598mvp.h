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

	void smi_act_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual bool map_first() const override { return true; }

private:
	required_device<cpu_device> m_host_cpu;
	std::vector<uint32_t> m_ram;

	virtual uint8_t capptr_r() override;

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
	u8 m_dram_timing[3];
	u8 m_dram_control;
	u8 m_refresh_counter;
	u8 m_dram_arbitration_control;
	u8 m_sdram_control;
	u8 m_dram_drive_strength;
	u8 m_ecc_control;
	u8 m_ecc_status;

	u8 m_pci_buffer_control;
	u8 m_pci_flow_control[2];
	u8 m_pci_master_control[2];
	u8 m_pci_arbitration[2];
	u8 m_pmu_control;

	u32 m_gart_control;
	u8 m_aperture_size;
	u32 m_ga_translation_table_base;

	u32 m_agp_command;
	u8 m_agp_control;

	int m_smiact;
};

class vt82c598mvp_bridge_device : public pci_bridge_device
{
public:
	/*template <typename T> vt82c598mvp_bridge_device(
	    const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
	    T &&gui_tag
	) : vt82c598mvp_bridge_device(mconfig, tag, owner, clock)
	{
	    set_ids_bridge(xxxxxxxx, 0x00);
	    //set_multifunction_device(true);
	    //m_vga.set_tag(std::forward<T>(gui_tag));
	}*/

	vt82c598mvp_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	//vt82c598mvp_bridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;
private:
	//required_device<sis630_gui_device> m_vga;

	//virtual void bridge_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	u8 m_pci2_flow_control[2];
	u8 m_pci2_master_control;
};


DECLARE_DEVICE_TYPE(VT82C598MVP_HOST,   vt82c598mvp_host_device)
DECLARE_DEVICE_TYPE(VT82C598MVP_BRIDGE, vt82c598mvp_bridge_device)

#endif // MAME_MACHINE_VT82C598MVP_H
