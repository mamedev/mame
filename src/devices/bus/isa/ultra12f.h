// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra 12F ESDI Caching Disk Controller

***************************************************************************/

#ifndef MAME_BUS_ISA_ULTRA12F_H
#define MAME_BUS_ISA_ULTRA12F_H

#pragma once

#include "isa.h"
#include "cpu/hpc/hpc.h"
#include "machine/upd765.h"

class ultra12f_device : public device_t, public device_isa16_card_interface
{
public:
	ultra12f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void hpc_map(address_map &map);

	required_device<hpc_device> m_hpc;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

DECLARE_DEVICE_TYPE(ULTRA12F, ultra12f_device)

#endif // MAME_BUS_ISA_ULTRA12F_H
