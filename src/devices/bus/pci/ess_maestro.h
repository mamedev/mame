// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_ESS_MAESTRO_H
#define MAME_BUS_PCI_ESS_MAESTRO_H

#pragma once

#include "pci_slot.h"

class es1946_solo1e_device : public pci_card_device
{
public:
	es1946_solo1e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	es1946_solo1e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

//	virtual const tiny_rom_entry *device_rom_region() const override;

//	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
//						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	virtual u8 capptr_r() override;

private:
	void extended_map(address_map &map);
	void sb_map(address_map &map);
	void vcbase_map(address_map &map);
	void mpu_map(address_map &map);
	void gameport_map(address_map &map);
};

DECLARE_DEVICE_TYPE(ES1946_SOLO1E, es1946_solo1e_device)

#endif // MAME_BUS_PCI_ESS_MAESTRO_H
