// license:BSD-3-Clause
// copyright-holders:O. Galibert

#ifndef MAME_BUS_NSCSI_CDU415_H
#define MAME_BUS_NSCSI_CDU415_H

#pragma once

#include "machine/nscsi_bus.h"
#include "cpu/h8/h83032.h"
#include "machine/ncr53c90.h"

class cdu415_device : public device_t, public nscsi_slot_card_interface
{
public:
	cdu415_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h83032_device> m_mcu;
	required_device<ncr53c94_device> m_scsi;
	required_region_ptr<u16> m_rom;

	u8 jumpers_r();
};

DECLARE_DEVICE_TYPE(CDU415, cdu415_device)

#endif // MAME_BUS_NSCSI_CDU415_H
