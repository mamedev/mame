// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Drive Slot

    32-pin slot

     1A 0 VOLTS     1B WR
     2A 0 VOLTS     2B A0
     3A 0 VOLTS     3B A1
     4A 0 VOLTS     4B D0
     5A 0 VOLTS     5B D1
     6A 0 VOLTS     6B D2
     7A 0 VOLTS     7B D3
     8A 0 VOLTS     8B D4
     9A 0 VOLTS     9B D5
    10A 0 VOLTS    10B D6
    11A 0 VOLTS    11B D7
    12A 0 VOLTS    12B 8 MHz
    13A 0 VOLTS    13B RST
    14A 0 VOLTS    14B N/C
    15A 0 VOLTS    15B A2
    16A 0 VOLTS    16B DISK 1 OR DISK 2

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_DRIVE_DRIVE_H
#define MAME_BUS_SAMCOUPE_DRIVE_DRIVE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_samcoupe_drive_interface;

// ======================> samcoupe_drive_port_device

class samcoupe_drive_port_device : public device_t, public device_single_card_slot_interface<device_samcoupe_drive_interface>
{
public:
	// construction/destruction
	template <typename T>
	samcoupe_drive_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, const char *dflt)
		: samcoupe_drive_port_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	samcoupe_drive_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~samcoupe_drive_port_device();

	// called from host
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void disk1_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	device_samcoupe_drive_interface *m_module;
};

// ======================> device_samcoupe_drive_interface

class device_samcoupe_drive_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_samcoupe_drive_interface();

	virtual uint8_t read(offs_t offset) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data) { }

protected:
	device_samcoupe_drive_interface(const machine_config &mconfig, device_t &device);

	samcoupe_drive_port_device *m_port;
};

// device type definition
DECLARE_DEVICE_TYPE(SAMCOUPE_DRIVE_PORT, samcoupe_drive_port_device)

// include here so drivers don't need to
#include "modules.h"

#endif // MAME_BUS_SAMCOUPE_DRIVE_DRIVE_H
