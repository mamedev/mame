// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_LGY98_H
#define MAME_BUS_PC98_CBUS_LGY98_H

#pragma once

#include "slot.h"
#include "machine/dp8390.h"
#include "machine/eepromser.h"

class lgy98_device : public device_t
				   , public device_memory_interface
				   , public device_pc98_cbus_slot_interface
{
public:
	lgy98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_device<dp8390d_device> m_nic;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	address_space_config m_space_mem_config;

	void io_map(address_map &map) ATTR_COLD;

	void memory_map(address_map &map) ATTR_COLD;

	uint8_t m_prom[16];
};


DECLARE_DEVICE_TYPE(LGY98, lgy98_device)

#endif // MAME_BUS_PC98_CBUS_LGY98_H
