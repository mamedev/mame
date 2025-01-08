// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS5513_IDE_H
#define MAME_MACHINE_SIS5513_IDE_H

#pragma once

#include "pci.h"
#include "idectrl.h"

class sis5513_ide_device : public pci_device
{
public:
	template <typename T> sis5513_ide_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&host_tag, uint32_t bmspace = AS_PROGRAM
	) : sis5513_ide_device(mconfig, tag, owner, clock)
	{
		// IDE controller with 0xd0 as programming i/f "ATA Host Adapters standard"
		// pclass bits 1-3 are actually 1 when controller is in native mode
		// pclass bits 0-2 can be r/w from $09
		set_ids(0x10395513, 0xd0, 0x010180, 0x00);
		m_bus_master_space.set_tag(host_tag, bmspace);
	}
	sis5513_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_pri() { return m_irq_pri_callback.bind(); }
	auto irq_sec() { return m_irq_sec_callback.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual void reset_all_mappings() override;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<bus_master_ide_controller_device> m_ide1;
	required_device<bus_master_ide_controller_device> m_ide2;
	devcb_write_line m_irq_pri_callback;
	devcb_write_line m_irq_sec_callback;
	required_address_space m_bus_master_space;

	void ide1_command_map(address_map &map) ATTR_COLD;
	void ide1_control_map(address_map &map) ATTR_COLD;
	void ide2_command_map(address_map &map) ATTR_COLD;
	void ide2_control_map(address_map &map) ATTR_COLD;
	void bus_master_ide_control_map(address_map &map) ATTR_COLD;

	bool ide1_mode();
	bool ide2_mode();

	u32 bar_r(offs_t offset);
	void bar_w(offs_t offset, u32 data);
	u32 m_bar[5]{};

	void prog_if_w(u8 data);
	u8 ide_ctrl_0_r();
	void ide_ctrl_0_w(u8 data);
	u8 ide_misc_ctrl_r();
	void ide_misc_ctrl_w(u8 data);

	u8 m_ide_ctrl0 = 0;
	u8 m_ide_misc = 0;

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

//  void compatible_io_map(address_map &map) ATTR_COLD;
	void flush_ide_mode();

	u8 unmap_log_r(offs_t offset);
	void unmap_log_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(SIS5513_IDE, sis5513_ide_device)

#endif
