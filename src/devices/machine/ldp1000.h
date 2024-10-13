// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Sony LDP-1000 laserdisc emulation.

***************************************************************************/

#ifndef MAME_MACHINE_LDP1000_H
#define MAME_MACHINE_LDP1000_H

#pragma once

#include "laserdsc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(SONY_LDP1000, sony_ldp1000_device)

// ======================> sony_ldp1000_device

class sony_ldp1000_device : public laserdisc_device
{
public:
	// construction/destruction
	sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations TODO: both actually protected
	void command_w(uint8_t data);
	uint8_t status_r();

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	enum ldp1000_status {
		stat_undef =        0x00,
		stat_completion =   0x01,
		stat_error =        0x02,
		stat_pgm_end =      0x04,
		stat_not_target =   0x05,
		stat_no_frame =     0x06,
		stat_ack =          0x0a,
		stat_nak =          0x0b
	};

	enum ldp1000_player_state {
		player_standby = 0,
		player_search
	};

private:
	uint8_t m_command;
	ldp1000_status m_status;
	ldp1000_player_state m_player_state;
	bool m_audio_enable[2];
	// TODO: sub-class into a specific internal player state
	void set_new_player_state(ldp1000_player_state which);
	void set_new_player_bcd(uint8_t data);
	uint32_t bcd_to_raw();
	void exec_enter_cmd();
	uint8_t m_internal_bcd[0x10];
	uint8_t m_index_state;

};

#endif // MAME_MACHINE_LDP1000_H
