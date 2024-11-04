// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Takahiro Nogi
/***************************************************************************

    Nichibutsu sound HW

***************************************************************************/

#ifndef MAME_NICHIBUTSU_NICHISND_H
#define MAME_NICHIBUTSU_NICHISND_H

#pragma once

#include "machine/gen_latch.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nichisnd_device

class nichisnd_device : public device_t
{
public:
	// construction/destruction
	nichisnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void sound_host_command_w(uint8_t data);

	void nichisnd_io_map(address_map &map) ATTR_COLD;
	void nichisnd_map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<generic_latch_8_device> m_soundlatch;
	required_region_ptr<uint8_t> m_sound_rom;
	required_memory_bank m_soundbank;

	void soundlatch_clear_w(uint8_t data);
	void soundbank_w(uint8_t data);
};

// device type declaration
DECLARE_DEVICE_TYPE(NICHISND, nichisnd_device)

#endif // MAME_NICHIBUTSU_NICHISND_H
