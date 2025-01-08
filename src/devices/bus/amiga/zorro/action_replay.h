// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Datel Action Replay

    Freezer cartridge for Amiga 500 and Amiga 2000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_ACTION_REPLAY_H
#define MAME_BUS_AMIGA_ZORRO_ACTION_REPLAY_H

#pragma once

#include "zorro.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> action_replay_device_base

class action_replay_device_base : public device_t, public device_exp_card_interface
{
public:
	DECLARE_INPUT_CHANGED_MEMBER( freeze );

protected:
	// construction/destruction
	action_replay_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_button;
};

class action_replay_mk1_device : public action_replay_device_base
{
public:
	// construction/destruction
	action_replay_mk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class action_replay_mk2_device : public action_replay_device_base
{
public:
	// construction/destruction
	action_replay_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class action_replay_mk3_device : public action_replay_device_base
{
public:
	// construction/destruction
	action_replay_mk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

} // namespace bus::amiga::zorro

// device type definition
DECLARE_DEVICE_TYPE_NS(ZORRO_ACTION_REPLAY_MK1, bus::amiga::zorro, action_replay_mk1_device)
DECLARE_DEVICE_TYPE_NS(ZORRO_ACTION_REPLAY_MK2, bus::amiga::zorro, action_replay_mk2_device)
DECLARE_DEVICE_TYPE_NS(ZORRO_ACTION_REPLAY_MK3, bus::amiga::zorro, action_replay_mk3_device)

#endif // MAME_BUS_AMIGA_ZORRO_ACTION_REPLAY_H
