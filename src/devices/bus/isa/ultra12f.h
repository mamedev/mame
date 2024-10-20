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
	ultra12f_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void hpc_map(address_map &map) ATTR_COLD;

	required_device<hpc_device> m_hpc;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

class ultra12f32_device : public ultra12f_device
{
public:
	ultra12f32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ULTRA12F, ultra12f_device)
DECLARE_DEVICE_TYPE(ULTRA12F32, ultra12f32_device)

#endif // MAME_BUS_ISA_ULTRA12F_H
