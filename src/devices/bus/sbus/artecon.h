// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Artecon SB-300P 3-serial 1-parallel SBus card skeleton

***************************************************************************/

#ifndef MAME_BUS_SBUS_ARTECON_H
#define MAME_BUS_SBUS_ARTECON_H

#pragma once

#include "sbus.h"


class sbus_artecon_device : public device_t, public device_sbus_card_interface
{
public:
	// construction/destruction
	sbus_artecon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_sbus_slot_interface overrides
	virtual void install_device() override;

	DECLARE_READ32_MEMBER(unknown_r);
	DECLARE_WRITE32_MEMBER(unknown_w);
	DECLARE_READ32_MEMBER(rom_r);

private:
	void mem_map(address_map &map) override;

	required_memory_region m_rom;
};


DECLARE_DEVICE_TYPE(SBUS_SB300P, sbus_artecon_device)

#endif // MAME_BUS_SBUS_ARTECON_H
