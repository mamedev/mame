// license:BSD-3-Clause
// copyright-holders:O. Galibert

#ifndef MAME_BUS_NSCSI_CRD254SH_H
#define MAME_BUS_NSCSI_CRD254SH_H

#pragma once

#include "machine/nscsi_bus.h"
#include "cpu/h8/h83042.h"
#include "machine/ncr53c90.h"

class crd254sh_device : public device_t, public nscsi_slot_card_interface
{
public:
	crd254sh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h83040_device> m_mcu;
	required_device<ncr53c94_device> m_scsi;
	required_region_ptr<u16> m_rom;
};

DECLARE_DEVICE_TYPE(CRD254SH, crd254sh_device)

#endif // MAME_BUS_NSCSI_CRD254SH_H
