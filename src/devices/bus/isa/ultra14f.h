// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra 14F AT Bus Fast SCSI-2 Bus Master Controller

***************************************************************************/

#ifndef MAME_BUS_ISA_ULTRA14F_H
#define MAME_BUS_ISA_ULTRA14F_H

#pragma once

#include "isa.h"
#include "machine/upd765.h"

class ultra14f_device : public device_t, public device_isa16_card_interface
{
public:
	ultra14f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void uscpu_map(address_map &map) ATTR_COLD;
	void scsic_config(device_t *device);

	required_device<cpu_device> m_uscpu;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

DECLARE_DEVICE_TYPE(ULTRA14F, ultra14f_device)

#endif // MAME_BUS_ISA_ULTRA14F_H
