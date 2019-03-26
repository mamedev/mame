// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1540/42A and AHA-1540/42B SCSI controllers

***************************************************************************/

#ifndef MAME_BUS_ISA_AHA1542B_H
#define MAME_BUS_ISA_AHA1542B_H

#pragma once

#include "isa.h"
#include "machine/upd765.h"

class aha154x_device : public device_t, public device_isa16_card_interface
{
protected:
	aha154x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void i8085_map(address_map &map);
	void scsic_config(device_t *device);

	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
};

class aha1542a_device : public aha154x_device
{
public:
	aha1542a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class aha1542b_device : public aha154x_device
{
public:
	aha1542b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};

DECLARE_DEVICE_TYPE(AHA1542A, aha1542a_device)
DECLARE_DEVICE_TYPE(AHA1542B, aha1542b_device)

#endif // MAME_BUS_ISA_AHA1542B_H
