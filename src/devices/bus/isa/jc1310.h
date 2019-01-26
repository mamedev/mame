// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Joincom Electronic JC-1310 MFM floppy disk controller

***************************************************************************/

#ifndef MAME_BUS_ISA_JC1310_H
#define MAME_BUS_ISA_JC1310_H

#pragma once

#include "isa.h"
#include "machine/upd765.h"

class isa16_jc1310_device : public device_t, public device_isa16_card_interface
{
public:
	isa16_jc1310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;

private:
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<wd37c65c_device> m_fdc;
};

DECLARE_DEVICE_TYPE(JC1310, isa16_jc1310_device)

#endif // MAME_BUS_ISA_JC1310_FDC_H
