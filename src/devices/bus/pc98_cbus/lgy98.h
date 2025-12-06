// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_LGY98_H
#define MAME_BUS_PC98_CBUS_LGY98_H

#pragma once

#include "slot.h"
#include "machine/eepromser.h"

class lgy98_device : public device_t
					, public device_pc98_cbus_slot_interface
{
public:
	lgy98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	void io_map(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(LGY98, lgy98_device)

#endif // MAME_BUS_PC98_CBUS_LGY98_H
