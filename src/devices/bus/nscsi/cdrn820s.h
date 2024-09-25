// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_NSCSI_CDRN820S_H
#define MAME_BUS_NSCSI_CDRN820S_H

#pragma once

#include "cpu/h8/h83048.h"
#include "machine/nscsi_bus.h"

class cdrn820s_device : public device_t, public nscsi_slot_card_interface
{
public:
	cdrn820s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	cdrn820s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h83048_device> m_h8;
};

DECLARE_DEVICE_TYPE(CDRN820S, cdrn820s_device)

#endif // MAME_BUS_NSCSI_CDRN820S_H
