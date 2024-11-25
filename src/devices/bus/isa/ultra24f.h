// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra-24F EISA SCSI Host Adapter

***************************************************************************/

#ifndef MAME_BUS_ISA_ULTRA24F_H
#define MAME_BUS_ISA_ULTRA24F_H

#pragma once

#include "isa.h"
#include "machine/i82355.h"
#include "machine/upd765.h"

class ultra24f_device : public device_t, public device_isa16_card_interface
{
public:
	ultra24f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 bmic_r(offs_t offset);
	void bmic_w(offs_t offset, u8 data);

	void uscpu_map(address_map &map) ATTR_COLD;
	void scsic_config(device_t *device);

	required_device<cpu_device> m_uscpu;
	required_device<i82355_device> m_bmic;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u16> m_bios16;
};

DECLARE_DEVICE_TYPE(ULTRA24F, ultra24f_device)

#endif // MAME_BUS_ISA_ULTRA24F_H
