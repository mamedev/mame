// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_NSCSI_CDR52_H
#define MAME_BUS_NSCSI_CDR52_H

#pragma once

#include "machine/cxd1185.h"
#include "machine/nscsi_bus.h"
#include "cpu/upd7810/upd7810.h"

class nscsi_cdr52_device : public device_t, public nscsi_slot_card_interface
{
public:
	nscsi_cdr52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<upd78c10_device> m_mcu;
	required_device<cxd1185_device> m_scsic;
	required_region_ptr<u8> m_rom;
};

DECLARE_DEVICE_TYPE(NSCSI_CDR52, nscsi_cdr52_device)

#endif // MAME_BUS_NSCSI_CDU75S_H
