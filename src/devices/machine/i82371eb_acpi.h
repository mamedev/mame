// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82371EB_ACPI_H
#define MAME_MACHINE_I82371EB_ACPI_H

#pragma once

#include "pci.h"
#include "lpc-acpi.h"
#include "pci-smbus.h"

class i82371eb_acpi_device : public pci_device
{
public:
	i82371eb_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;

private:
	required_device<lpc_acpi_device> m_acpi;
	required_device<smbus_device> m_smbus;

	u8 pmregmisc_r();
	void pmregmisc_w(u8 data);

	u8 smbhstcfg_r();
	void smbhstcfg_w(u8 data);

	u32 pmba_r();
	void pmba_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 devresa_r();
	void devresa_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 smbba_r();
	void smbba_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	bool m_pmiose = false;
	u32 m_pmba = 0;
	u32 m_smbba = 0;
	u8 m_smbus_host_config = 0;
	u32 m_devresa = 0;

	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(I82371EB_ACPI, i82371eb_acpi_device)

#endif
