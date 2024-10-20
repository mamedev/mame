// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_H
#define MAME_BUS_MSX_SLOT_RAM_H

#pragma once

#include "slot.h"

class msx_slot_ram_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// Set to 0xe000 for 8KB RAM
	void force_start_address(u16 start) { m_start_address = start; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<u8[]> m_ram;
};


DECLARE_DEVICE_TYPE(MSX_SLOT_RAM, msx_slot_ram_device)


#endif // MAME_BUS_MSX_SLOT_RAM_H
