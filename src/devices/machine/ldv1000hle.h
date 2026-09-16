// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    ldv1000hle.h

    Pioneer LDV-1000 laserdisc player simulation.

*************************************************************************/

#ifndef MAME_MACHINE_LDV1000HLE_H
#define MAME_MACHINE_LDV1000HLE_H

#pragma once

#include "laserdsc.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(PIONEER_LDV1000HLE, pioneer_ldv1000hle_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pioneer_ldv1000hle_device : public parallel_laserdisc_device
{
public:
	// construction/destruction
	pioneer_ldv1000hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void data_w(u8 data) override;
	virtual u8 data_r() override;
	virtual void enter_w(int state) override;
	virtual int status_strobe_r() override { return m_status_strobe; }
	virtual int ready_r() override { return m_command_strobe; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// laserdisc overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual s32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;

	TIMER_CALLBACK_MEMBER(process_vbi_data);
	TIMER_CALLBACK_MEMBER(resume_from_stop);
	TIMER_CALLBACK_MEMBER(park_strobe_tick);
	TIMER_CALLBACK_MEMBER(assert_status_strobe);
	TIMER_CALLBACK_MEMBER(deassert_status_strobe);
	TIMER_CALLBACK_MEMBER(assert_command_strobe);
	TIMER_CALLBACK_MEMBER(deassert_command_strobe);

private:
	enum player_command : u8
	{
		CMD_CLEAR                = 0xbf,
		CMD_0                    = 0x3f,
		CMD_1                    = 0x0f,
		CMD_2                    = 0x8f,
		CMD_3                    = 0x4f,
		CMD_4                    = 0x2f,
		CMD_5                    = 0xaf,
		CMD_6                    = 0x6f,
		CMD_7                    = 0x1f,
		CMD_8                    = 0x9f,
		CMD_9                    = 0x5f,
		CMD_STORE                = 0xf5,
		CMD_RECALL               = 0x7f,
		CMD_DISPLAY              = 0xf1,
		CMD_AUDIO1               = 0xf4,
		CMD_AUDIO2               = 0xfc,
		CMD_PLAY                 = 0xfd,
		CMD_STOP                 = 0xfb,
		CMD_AUTOSTOP             = 0xf3,
		CMD_SEARCH               = 0xf7,
		CMD_SCAN_FWD             = 0xf0,
		CMD_SCAN_REV             = 0xf8,
		CMD_STEP_FWD             = 0xf6,
		CMD_STEP_REV             = 0xfe,
		CMD_REJECT               = 0xf9,
		CMD_NO_ENTRY             = 0xff,
		CMD_LOAD                 = 0xcc,
		CMD_DISPLAY_DISABLE      = 0xcd,
		CMD_DISPLAY_ENABLE       = 0xce,
		CMD_GET_FRAME_NUM        = 0xc2,
		CMD_GET_2ND_DISPLAY      = 0xc3,
		CMD_GET_1ST_DISPLAY      = 0xc4,
		CMD_TRANSFER_MEMORY      = 0xc8,
		CMD_FWD_X0               = 0xa0,
		CMD_FWD_X1_4             = 0xa1,
		CMD_FWD_X1_2             = 0xa2,
		CMD_FWD_X1               = 0xa3,
		CMD_FWD_X2               = 0xa4,
		CMD_FWD_X3               = 0xa5,
		CMD_FWD_X4               = 0xa6,
		CMD_FWD_X5               = 0xa7,
		CMD_SKIP_FWD_10          = 0xb1,
		CMD_SKIP_FWD_20          = 0xb2,
		CMD_SKIP_FWD_30          = 0xb3,
		CMD_SKIP_FWD_40          = 0xb4,
		CMD_SKIP_FWD_50          = 0xb5,
		CMD_SKIP_FWD_60          = 0xb6,
		CMD_SKIP_FWD_70          = 0xb7,
		CMD_SKIP_FWD_80          = 0xb8,
		CMD_SKIP_FWD_90          = 0xb9,
		CMD_SKIP_FWD_100         = 0xba
	};

	enum player_status : u8
	{
		STATUS_PARK              = 0x7c,
		STATUS_PLAY              = 0x64,
		STATUS_STOP              = 0x65,
		STATUS_SEARCH            = 0x50,
		STATUS_SEARCH_FINISH     = 0xd0,
		STATUS_SEARCH_ERROR      = 0x90,
		STATUS_AUTOSTOP          = 0x54,
		STATUS_SCAN              = 0x4c,
		STATUS_FORWARD           = 0x2e,
		STATUS_LOAD              = 0x48,
		STATUS_LOAD_END          = 0xc8,
		STATUS_LOAD_ERROR        = 0xc4,
		STATUS_FOCUS_UNLOCK      = 0xbc,
		STATUS_LEADIN            = 0x58,
		STATUS_LEADOUT           = 0x5c,
		STATUS_REJECT            = 0x60,
		STATUS_READY             = 0x80
	};

	enum player_mode : u8
	{
		MODE_PARK,
		MODE_PLAY,
		MODE_SCAN_FORWARD,
		MODE_SCAN_REVERSE,
		MODE_SEARCH,
		MODE_STOP
	};

	void process_command_buffer();
	bool is_command_byte(const u8 data);
	bool is_command_number(const u8 data);
	void process_command(size_t cmd_index);

	static u32 bcd_to_literal(u32 bcd);
	u32 literal_to_bcd(u32 value);
	u32 cmd_number_to_bcd();
	u8 cmd_to_number(const u8 cmd);

	void set_mode(const u8 mode);
	void set_playing(const u8 new_status, const double fields_per_vsync);
	void set_stopped(const u8 new_status);
	void cmd_play();
	void cmd_stop();
	void cmd_step(const int direction);
	void cmd_play_forward(const double fields_per_vsync);
	void cmd_search(const u32 frame);
	void cmd_skip_forward(const s32 amount);

	void update_video_enable();
	void update_audio_enable();

	// internal state
	emu_timer *         m_vbi_fetch;
	emu_timer *         m_stop_timer;
	emu_timer *         m_park_strobe_timer;
	emu_timer *         m_assert_status_strobe_timer;
	emu_timer *         m_deassert_status_strobe_timer;
	emu_timer *         m_assert_command_strobe_timer;
	emu_timer *         m_deassert_command_strobe_timer;
	u8                  m_cmd_buffer[21];
	u32                 m_cmd_length;
	u8                  m_status;
	u8                  m_pre_stop_status;

	u8                  m_mode;                 // current player mode
	u32                 m_curr_frame;           // frame number
	u32                 m_search_frame;
	u32                 m_stop_frame;
	u8                  m_cmd_number[5];
	u32                 m_cmd_number_length;

	u16                 m_user_ram[1024];
	u32                 m_curr_register;
	u32                 m_scan_speed;
	u32                 m_scan_speed_accum;
	double              m_play_speed;
	bool                m_audio_enable[2];
	bool                m_status_strobe;
	bool                m_command_strobe;
};

#endif // MAME_MACHINE_LDV1000HLE_H
