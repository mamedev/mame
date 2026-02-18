// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MACHINE_VT82C586B_IDE_H
#define MAME_MACHINE_VT82C586B_IDE_H

#pragma once

#include "pci.h"
#include "machine/idectrl.h"

class vt82c586b_ide_device : public pci_device
{
public:
	template <typename T>
	vt82c586b_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: vt82c586b_ide_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	vt82c586b_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_pri() { return m_irq_pri_callback.bind(); }
	auto irq_sec() { return m_irq_sec_callback.bind(); }

	template <typename T>
	void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<bus_master_ide_controller_device, 2> m_ide;

	devcb_write_line m_irq_pri_callback;
	devcb_write_line m_irq_sec_callback;

	void ide1_command_map(address_map &map) ATTR_COLD;
	void ide1_control_map(address_map &map) ATTR_COLD;
	void ide2_command_map(address_map &map) ATTR_COLD;
	void ide2_control_map(address_map &map) ATTR_COLD;
	void bus_master_ide_control_map(address_map &map) ATTR_COLD;

	bool ide1_mode();
	bool ide2_mode();
	void flush_ide_mode();

	u32 bar_r(offs_t offset);
	void bar_w(offs_t offset, u32 data);
	u32 m_bar[5]{};

	void prog_if_w(u8 data);

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

	u8 m_chip_enable;
	u8 m_ide_config;
	u8 m_fifo_config;
	u8 m_misc_control[3];
	u32 m_drive_timing_control;
	u8 m_address_setup_time;
	u8 m_control_port_access_time[2];
	u8 m_udma_timing_control[4];
	u16 m_sector_size[2];
};

DECLARE_DEVICE_TYPE(VT82C586B_IDE, vt82c586b_ide_device)

#endif // MAME_MACHINE_VT82C586B_IDE_H
