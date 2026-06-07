// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82434LX_PCMC_H
#define MAME_MACHINE_I82434LX_PCMC_H

#pragma once

#include "pci.h"

class i82434nx_pcmc_device : public pci_host_device
{
public:
	template <typename T>
	i82434nx_pcmc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, int ram_size)
		: i82434nx_pcmc_device(mconfig, tag, owner)
	{
		set_ids_host(0x808604a3, 0x11, 0x00000000);

		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ram_size(ram_size);
	}
	i82434nx_pcmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size) { m_ram_size = ram_size; }

protected:
//  i82434nx_pcmc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

private:
	required_device<cpu_device> m_host_cpu;
	std::vector<uint32_t> m_ram;

	u32 m_ram_size = 0;

	void map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting);

	virtual uint8_t latency_timer_r() override;
	void latency_timer_w(uint8_t data);

	u8 m_latency_timer;

	u8 m_pam[7];
};


//DECLARE_DEVICE_TYPE(I82434LX_PCMC, i82434lx_pcmc_device)
DECLARE_DEVICE_TYPE(I82434NX_PCMC, i82434nx_pcmc_device)

#endif // MAME_MACHINE_I82434LX_PCMC_H
