// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS5513_IDE_H
#define MAME_MACHINE_SIS5513_IDE_H

#pragma once

#include "pci.h"
#include "machine/pci-ide.h"

class sis5513_ide_device : public pci_device 
{
public:
	sis5513_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_pri() { return m_irq_pri_callback.bind(); }
	auto irq_sec() { return m_irq_sec_callback.bind(); }

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

//	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	void ide1_command_map(address_map &map);
	void ide1_control_map(address_map &map);
	void ide2_command_map(address_map &map);
	void ide2_control_map(address_map &map);
	void bus_master_ide_control_map(address_map &map);
private:
	required_device<bus_master_ide_controller_device> m_ide1;
	required_device<bus_master_ide_controller_device> m_ide2;
	devcb_write_line m_irq_pri_callback;
	devcb_write_line m_irq_sec_callback;

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(SIS5513_IDE, sis5513_ide_device)

#endif
