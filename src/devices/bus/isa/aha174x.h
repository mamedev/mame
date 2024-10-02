// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1540/42A and AHA-1540/42B SCSI controllers

***************************************************************************/

#ifndef MAME_BUS_ISA_AHA174X_H
#define MAME_BUS_ISA_AHA174X_H

#pragma once

#include "isa.h"
#include "cpu/hpc/hpc.h"
#include "machine/aic565.h"
#include "machine/7200fifo.h"
#include "machine/upd765.h"

class aha174x_device : public device_t, public device_isa16_card_interface
{
protected:
	aha174x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	void hpc_map(address_map &map) ATTR_COLD;
	void scsic_config(device_t *device);

	required_device<hpc46003_device> m_hpc;
	required_device<aic565_device> m_busaic;
	required_device_array<fifo7200_device, 2> m_fifo;
	required_region_ptr<u8> m_bios;
};

class aha1740_device : public aha174x_device
{
public:
	aha1740_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class aha1742a_device : public aha174x_device
{
public:
	aha1742a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<upd765_family_device> m_fdc;
};

DECLARE_DEVICE_TYPE(AHA1740, aha1740_device)
DECLARE_DEVICE_TYPE(AHA1742A, aha1742a_device)

#endif // MAME_BUS_ISA_AHA174X_H
