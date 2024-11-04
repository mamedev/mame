// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_PDC20262_H
#define MAME_BUS_PCI_PDC20262_H

#pragma once

#include "pci_slot.h"
#include "machine/idectrl.h"
#include "machine/input_merger.h"

class pdc20262_device : public pci_card_device
{
public:
	pdc20262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::DISK; }

protected:
	pdc20262_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

//  virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//                         uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	void ide1_command_map(address_map &map) ATTR_COLD;
	void ide1_control_map(address_map &map) ATTR_COLD;
	void ide2_command_map(address_map &map) ATTR_COLD;
	void ide2_control_map(address_map &map) ATTR_COLD;
	void bus_master_ide_control_map(address_map &map) ATTR_COLD;
	void extra_map(address_map &map) ATTR_COLD;

//  virtual void device_config_complete() override;

	required_device<bus_master_ide_controller_device> m_ide1;
	required_device<bus_master_ide_controller_device> m_ide2;
	required_device<input_merger_device> m_irqs;
	required_address_space m_bus_master_space;
	required_memory_region m_bios_rom;

	u8 m_clock = 0;
	u8 m_irq_state = 0;

	uint32_t ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask = ~0);
	void ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t ide1_read_cs1_r();
	void ide1_write_cs1_w(uint8_t data);
	uint8_t ide2_read_cs1_r();
	void ide2_write_cs1_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(PDC20262, pdc20262_device)

#endif // MAME_BUS_PCI_PDC20262_H
