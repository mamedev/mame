// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_MEDIAGX_CS5530_IDE_H
#define MAME_MACHINE_MEDIAGX_CS5530_IDE_H

#pragma once

#include "pci.h"
#include "idectrl.h"

class mediagx_cs5530_ide_device : public pci_device
{
public:
	template <typename T> mediagx_cs5530_ide_device(
		const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
		T &&host_tag, uint32_t bmspace = AS_PROGRAM
	) : mediagx_cs5530_ide_device(mconfig, tag, owner, clock)
	{
		set_ids(0x10780102, 0x00, 0x010180, 0x00);
		m_bus_master_space.set_tag(host_tag, bmspace);
	}
	mediagx_cs5530_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_pri() { return m_irq_pri_callback.bind(); }
	auto irq_sec() { return m_irq_sec_callback.bind(); }

	void primary_ide_map(address_map &map) ATTR_COLD;
	void secondary_ide_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual void reset_all_mappings() override;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;


private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<bus_master_ide_controller_device> m_ide1;
	required_device<bus_master_ide_controller_device> m_ide2;
	devcb_write_line m_irq_pri_callback;
	devcb_write_line m_irq_sec_callback;
	required_address_space m_bus_master_space;

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);

};

DECLARE_DEVICE_TYPE(MEDIAGX_CS5530_IDE, mediagx_cs5530_ide_device)

#endif
