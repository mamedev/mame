// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_NSCSI_CW7501
#define MAME_BUS_NSCSI_CW7501 1

#pragma once

#include "cpu/m37710/m37710.h"
#include "machine/nscsi_bus.h"

class cw7501_device : public device_t, public nscsi_slot_card_interface
{
public:
	cw7501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	cw7501_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 mystery_data_r();
	void mystery_data_w(u8 data);
	void mystery_address_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<m37710s4_device> m_cdcpu;

	u8 m_mystery_address;
};

class cdr4210_device : public cw7501_device
{
public:
	cdr4210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(CW7501, cw7501_device)
DECLARE_DEVICE_TYPE(CDR4210, cdr4210_device)

#endif // MAME_BUS_NSCSI_CW7501
