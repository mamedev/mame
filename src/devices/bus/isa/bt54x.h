// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    BusTek/BusLogic BT-54x series PC/AT SCSI host adapters

***************************************************************************/

#ifndef MAME_BUS_ISA_BT54X_H
#define MAME_BUS_ISA_BT54X_H

#pragma once

#include "isa.h"
#include "cpu/i86/i186.h"
#include "machine/upd765.h"

class bt54x_device : public device_t, public device_isa16_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	bt54x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	u8 local_status_r();

	void local_map(address_map &map) ATTR_COLD;
	void asc_config(device_t *device);
	void fsc_config(device_t *device);
	void fsc_base(machine_config &config);

	required_device<i80188_cpu_device> m_mpu;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

class bt542b_device : public bt54x_device
{
public:
	bt542b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class bt542bh_device : public bt54x_device
{
public:
	bt542bh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class bt545s_device : public bt54x_device
{
public:
	bt545s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(BT542B, bt542b_device)
DECLARE_DEVICE_TYPE(BT542BH, bt542bh_device)
DECLARE_DEVICE_TYPE(BT545S, bt545s_device)

#endif // MAME_BUS_ISA_BT54X_H
