// license:BSD-3-Clause
/***************************************************************************

	Sony LDP-1450 laserdisc emulation.

***************************************************************************/

#pragma once

#ifndef __LDP1450DEV_H__
#define __LDP1450DEV_H__

#include "laserdsc.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_LDP1450_ADD(_tag, clock) \
	MCFG_DEVICE_ADD(_tag, SONY_LDP1450, clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
extern const device_type SONY_LDP1450;

// ======================> sony_ldp1450_device

class sony_ldp1450_device : public laserdisc_device
{
public:
	// construction/destruction
	sony_ldp1450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations TODO: both actually protected
	void command_w(UINT8 data);
	UINT8 status_r() const { return m_status; }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	UINT8 m_ld_frame_index;
	UINT8 m_ld_frame[5];
	UINT8 m_ld_command_current_byte;
	UINT8 m_ld_command_to_send[5];
	UINT8 m_ld_command_total_bytes;

	enum LD_INPUT_STATE
	{
		LD_INPUT_GET_COMMAND = 0,
		LD_INPUT_TEXT_COMMAND,
		LD_INPUT_TEXT_GET_X,
		LD_INPUT_TEXT_GET_Y,
		LD_INPUT_TEXT_GET_MODE,
		LD_INPUT_TEXT_GET_STRING,
		LD_INPUT_TEXT_GET_SET_WINDOW
	} m_ld_input_state;

	enum ldp1450_player_state {
		player_standby = 0,
		player_search,
		player_search_clr,
		player_play,
		player_stop,
		player_repeat
	};
	
private:
	UINT8 m_command;
	UINT8 m_status;
	ldp1450_player_state m_player_state;
	bool m_audio_enable[2];
	void set_new_player_state(ldp1450_player_state which);
	void set_new_player_bcd(UINT8 data);
	UINT32 bcd_to_raw();
	void exec_enter_cmd();
	UINT8 m_internal_bcd[0x10];
	UINT8 m_index_state;
	
};






//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
