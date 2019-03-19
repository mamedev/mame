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

class bt545s_device : public device_t, public device_isa16_card_interface
{
public:
	bt545s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;

private:
	u8 local_status_r();

	void local_map(address_map &map);
	void fsc_config(device_t *device);

	required_device<i80188_cpu_device> m_mpu;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

DECLARE_DEVICE_TYPE(BT545S, bt545s_device)

#endif // MAME_BUS_ISA_BT54X_H
