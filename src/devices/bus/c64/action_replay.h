// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Datel Electronics Action Replay emulation

**********************************************************************/

#ifndef MAME_BUS_C64_ACTION_REPLAY_H
#define MAME_BUS_C64_ACTION_REPLAY_H

#pragma once


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_action_replay_cartridge_device

class c64_action_replay_cartridge_device : public device_t,
										   public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_action_replay_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER( freeze );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	memory_share_creator<uint8_t> m_ram;
	required_memory_region m_pla;
	emu_timer *m_freeze_timer;

	u8 m_bank;
	bool m_pla_a7;
	bool m_freeze;

	bool pla_enabled() { return !BIT(m_bank, 2); }
	u8 read_pla(offs_t offset, int io2);
	void update_freeze(int ba);

	TIMER_CALLBACK_MEMBER(unfreeze);
};


// device type definition
DECLARE_DEVICE_TYPE(C64_ACTION_REPLAY, c64_action_replay_cartridge_device)


#endif // MAME_BUS_C64_ACTION_REPLAY_H
