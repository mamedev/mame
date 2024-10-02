// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Intel i82875p northbridge

#ifndef MAME_MACHINE_I82875P_H
#define MAME_MACHINE_I82875P_H

#pragma once

#include "pci.h"

class i82875p_host_device : public pci_host_device {
public:
	template <typename T>
	i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id, T &&cpu_tag, int ram_size)
		: i82875p_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x80862578, 0x02, subdevice_id);
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}

	i82875p_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size);

	virtual uint8_t capptr_r() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	void agp_translation_map(address_map &map) ATTR_COLD;

	int ram_size;
	required_device<device_memory_interface> cpu;
	std::vector<uint32_t> ram;

	uint8_t agpm, fpllcont, pam[8], smram, esmramc;
	uint8_t apsize, amtt, lptt;
	uint16_t toud, mchcfg, errcmd, smicmd, scicmd, skpd;
	uint32_t agpctrl, attbase;

	uint8_t agpm_r();
	void agpm_w(uint8_t data);
	uint8_t gc_r();
	uint8_t csabcont_r();
	uint32_t eap_r();
	uint8_t derrsyn_r();
	uint8_t des_r();
	uint8_t fpllcont_r();
	void fpllcont_w(uint8_t data);
	uint8_t pam_r(offs_t offset);
	void pam_w(offs_t offset, uint8_t data);
	uint8_t smram_r();
	void smram_w(uint8_t data);
	uint8_t esmramc_r();
	void esmramc_w(uint8_t data);
	uint32_t acapid_r();
	uint32_t agpstat_r();
	uint32_t agpcmd_r();
	uint32_t agpctrl_r();
	void agpctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t apsize_r();
	void apsize_w(uint8_t data);
	uint32_t attbase_r();
	void attbase_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t amtt_r();
	void amtt_w(uint8_t data);
	uint8_t lptt_r();
	void lptt_w(uint8_t data);
	uint16_t toud_r();
	void toud_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mchcfg_r();
	void mchcfg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t errsts_r();
	uint16_t errcmd_r();
	void errcmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t smicmd_r();
	void smicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scicmd_r();
	void scicmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t skpd_r();
	void skpd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t capreg1_r();
	uint8_t capreg2_r();
};

class i82875p_agp_device : public agp_bridge_device {
public:
	i82875p_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class i82875p_overflow_device : public pci_device {
public:
	i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subdevice_id)
		: i82875p_overflow_device(mconfig, tag, owner, clock)
	{
		set_ids(0x8086257e, 0x02, 0x088000, subdevice_id);
	}
	i82875p_overflow_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	uint8_t dram_row_boundary_r(offs_t offset);
	void dram_row_boundary_w(offs_t offset, uint8_t data);
	uint8_t dram_row_attribute_r(offs_t offset);
	void dram_row_attribute_w(offs_t offset, uint8_t data);
	uint32_t dram_timing_r();
	void dram_timing_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dram_controller_mode_r();
	void dram_controller_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void overflow_map(address_map &map) ATTR_COLD;

	uint8_t dram_row_boundary[8], dram_row_attribute[4];
	uint32_t dram_timing, dram_controller_mode;
};

DECLARE_DEVICE_TYPE(I82875P_HOST,     i82875p_host_device)
DECLARE_DEVICE_TYPE(I82875P_AGP,      i82875p_agp_device)
DECLARE_DEVICE_TYPE(I82875P_OVERFLOW, i82875p_overflow_device)

#endif // MAME_MACHINE_I82875P_H
