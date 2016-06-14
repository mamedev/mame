// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Sony LDP-1000 laserdisc emulation.

***************************************************************************/

#pragma once

#ifndef __LDP1000DEV_H__
#define __LDP1000DEV_H__

#include "laserdsc.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_LDP1000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SONY_LDP1000, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
extern const device_type SONY_LDP1000;

// ======================> sony_ldp1000_device

class sony_ldp1000_device : public laserdisc_device
{
public:
	// construction/destruction
	sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

<<<<<<< HEAD
	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

=======
	// I/O operations TODO: both actually protected
	DECLARE_WRITE8_MEMBER( command_w );
	DECLARE_READ8_MEMBER( status_r );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	enum ldp1000_status {
		stat_undef = 		0x00,
		stat_completion = 	0x01,
		stat_error = 		0x02,
		stat_pgm_end = 		0x04,
		stat_not_target = 	0x05,
		stat_no_frame = 	0x06,
		stat_ack = 			0x0a,
		stat_nak = 			0x0b
	};

	enum ldp1000_player_state {
		player_standby = 0,
		player_search
	};
	
private:
	UINT8 m_command;
	ldp1000_status m_status;
	ldp1000_player_state m_player_state;
	bool m_audio_enable[2];
	// TODO: sub-class
	void set_new_player_state(ldp1000_player_state which, UINT8 fifo_size);
	void set_new_player_bcd(UINT8 data);
	UINT8 m_internal_bcd[0x10];
	UINT8 m_index_state;
	UINT8 m_index_size;
>>>>>>> refs/remotes/origin/master
};






//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
