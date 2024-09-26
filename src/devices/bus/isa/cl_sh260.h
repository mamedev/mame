// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ISA 16-bit disk controllers based on the Cirrus Logic CL-SH260-15PC-D
    * Everex EV-346
    * Joincom Electronic JC-1310

***************************************************************************/

#ifndef MAME_BUS_ISA_CL_SH260_H
#define MAME_BUS_ISA_CL_SH260_H

#pragma once

#include "isa.h"
#include "machine/upd765.h"

class isa16_cl_sh260_device : public device_t, public device_isa16_card_interface
{
protected:
	isa16_cl_sh260_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void i8031_map(address_map &map) ATTR_COLD;

	required_device<upd765_family_device> m_fdc;
};

class isa16_ev346_device : public isa16_cl_sh260_device
{
public:
	isa16_ev346_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void ext_map(address_map &map) ATTR_COLD;
};

class isa16_jc1310_device : public isa16_cl_sh260_device
{
public:
	isa16_jc1310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void ext_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(EV346, isa16_ev346_device)
DECLARE_DEVICE_TYPE(JC1310, isa16_jc1310_device)

#endif // MAME_BUS_ISA_CL_SH260_H
