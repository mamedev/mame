// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    bandit.h - Apple "Bandit" and "Aspen" 60x bus/PCI bridges

**********************************************************************/

#ifndef MAME_APPLE_BANDIT_H
#define MAME_APPLE_BANDIT_H

#pragma once

#include "machine/pci.h"

class bandit_host_device : public pci_host_device
{
public:
	template <typename T>
	bandit_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: bandit_host_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	bandit_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	void set_dev_offset(int devOffset) { m_dev_offset = devOffset; }

protected:
	bandit_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
						   u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	u32 m_last_config_address;

private:
	void cpu_map(address_map &map) ATTR_COLD;
	virtual u32 be_config_address_r();
	virtual void be_config_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 be_config_data_r(offs_t offset, u32 mem_mask = ~0);
	void be_config_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <u32 Base> u32 pci_memory_r(offs_t offset, u32 mem_mask);
	template <u32 Base> void pci_memory_w(offs_t offset, u32 data, u32 mem_mask);
	template <u32 Base> u32 pci_io_r(offs_t offset, u32 mem_mask);
	template <u32 Base> void pci_io_w(offs_t offset, u32 data, u32 mem_mask);

	address_space_config m_mem_config, m_io_config;
	required_device<device_memory_interface> m_cpu;
	address_space *m_cpu_space;
	int m_dev_offset;
};

class aspen_host_device : public bandit_host_device
{
public:
	template <typename T>
	aspen_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: aspen_host_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	aspen_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	virtual u32 be_config_address_r() override;
	virtual void be_config_address_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;
};

DECLARE_DEVICE_TYPE(BANDIT, bandit_host_device)
DECLARE_DEVICE_TYPE(ASPEN, aspen_host_device)

#endif // MAME_APPLE_BANDIT_H
