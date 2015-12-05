// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Datel Action Replay

    Freezer cartridge for Amiga 500 and Amiga 2000

***************************************************************************/

#pragma once

#ifndef __ACTION_REPLAY_H__
#define __ACTION_REPLAY_H__

#include "emu.h"
#include "zorro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> action_replay_device

class action_replay_device : public device_t, public device_exp_card_interface
{
public:
	// construction/destruction
	action_replay_device(const machine_config &mconfig, device_type type, const char *tag,
		device_t *owner, UINT32 clock, const char *name, const char *shortname);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_INPUT_CHANGED_MEMBER( freeze );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_button;
};

class action_replay_mk1_device : public action_replay_device
{
public:
	// construction/destruction
	action_replay_mk1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};

class action_replay_mk2_device : public action_replay_device
{
public:
	// construction/destruction
	action_replay_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};

class action_replay_mk3_device : public action_replay_device
{
public:
	// construction/destruction
	action_replay_mk3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};

// device type definition
extern const device_type ACTION_REPLAY_MK1;
extern const device_type ACTION_REPLAY_MK2;
extern const device_type ACTION_REPLAY_MK3;

#endif
