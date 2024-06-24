// license:BSD-3-Clause

/*************************************************************************

    ldp1450hle.h


*************************************************************************/

#ifndef MAME_MACHINE_LDP1450_H
#define MAME_MACHINE_LDP1450_H

#pragma once

#include "laserdsc.h"
#include "diserial.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(SONY_LDP1450HLE, sony_ldp1450hle_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sony_ldp1450hle_device

class sony_ldp1450hle_device : public laserdisc_device, public device_serial_interface
{
public:
	// construction/destruction
	sony_ldp1450hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto serial_tx() { return m_serial_tx.bind(); }

	void set_baud(s32 clock) { m_baud = clock; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// laserdisc overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual s32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	// diserial overrides
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;


	TIMER_CALLBACK_MEMBER(process_vbi_data);
	TIMER_CALLBACK_MEMBER(process_queue);

private:
	enum player_command : u16
	{
		CMD_AUDIO_OFF           =0x24,
		CMD_AUDIO_ON            =0x25,
		CMD_VIDEO_OFF           =0x26,
		CMD_VIDEO_ON            =0x27,
		CMD_PSC_ON              =0x28,
		CMD_PSC_OFF             =0x29,
		CMD_EJECT               =0x2a,
		CMD_STEP_STILL          =0x2b,
		CMD_STEP_STILL_REVERSE  =0x2c,
		CMD_PLAY                =0x3a,
		CMD_FAST_FORWARD        =0x3b,
		CMD_SLOW_FORWARD        =0x3c,
		CMD_STEP_FORWARD        =0x3d,
		CMD_SCAN_FORWARD        =0x3e,
		CMD_STOP                =0x3f,
		CMD_ENTER               =0x40,
		CMD_CLEAR               =0x41,
		CMD_MENU                =0x42,
		CMD_SEARCH              =0x43,
		CMD_REPEAT              =0x44,
		CMD_CH1_ON              =0x46,
		CMD_CH1_OFF             =0x47,
		CMD_CH2_ON              =0x48,
		CMD_CH2_OFF             =0x49,
		CMD_PLAY_REVERSE        =0x4a,
		CMD_FAST_REVERSE        =0x4b,
		CMD_SLOW_REVERSE        =0x4c,
		CMD_STEP_REVERSE        =0x4d,
		CMD_SCAN_REVERSE        =0x4e,
		CMD_STILL               =0x4f,
		CMD_INDEX_ON            =0x50,
		CMD_INDEX_OFF           =0x51,
		CMD_FRAME_SET           =0x55,
		CMD_CLEAR_ALL           =0x56,
		CMD_ADDR_INQ            =0x60,
		CMD_STATUS_INQ          =0x67,
		CMD_CHAPTER_SET         =0x69,
		CMD_USER_INDEX_CTRL     =0x80,
		CMD_USER_INDEX_ON       =0x81,
		CMD_USER_INDEX_OFF      =0x82,
	};

	enum player_mode : u8
	{
		MODE_PARK,
		MODE_DOOR_OPEN,
		MODE_PAUSE,
		MODE_PLAY,
		MODE_MS_FORWARD,
		MODE_MS_REVERSE,
		MODE_SEARCH,
		MODE_SEARCH_CMD,
		MODE_SEARCH_REP,
		MODE_SEARCH_CL,
		MODE_REPEAT_CMD_MARK,
		MODE_REPEAT_CMD_REPS,
		MODE_STILL,
		SUBMODE_NORMAL,
		SUBMODE_USER_INDEX,
		SUBMODE_USER_INDEX_MODE_1,
		SUBMODE_USER_INDEX_MODE_2,
		SUBMODE_USER_INDEX_MODE_3,
		SUBMODE_USER_INDEX_STRING_1,
		SUBMODE_USER_INDEX_STRING_2,
		SUBMODE_USER_INDEX_WINDOW,

	};

	enum address_mode : u8
	{
		ADDRESS_FRAME,
		ADDRESS_CHAPTER
	};


	void queue_reply(u8 reply, float delay);
	void queue_reply_buffer (const u8 reply[], float delay);

	static u32 bcd_to_literal(u32 bcd);
	static bool is_number(char value);
	void add_command_byte(u8 command);

	void begin_search(u32 value);

	void update_audio_squelch();
	void update_video_enable();

	// internal state
	devcb_write_line    m_serial_tx;
	emu_timer *         m_vbi_fetch;
	emu_timer *         m_queue_timer;
	bool                m_cmd_running;
	u8             m_reply_buffer[64];
	
	u8             m_reply_write_index;
	u8             m_reply_read_index;
	u8             m_reply;

	u8             m_mode;                 // current player mode
	u8             m_submode;
	u32            m_baud;
	u32            m_chapter;
	u32            m_time;
	u32            m_frame;                // raw frame index (CAV mode)
	u32            m_search_chapter;
	u32            m_search_frame;
	s32            m_cmd_buffer;
	u32            m_mark_chapter;
	u32            m_mark_frame;
	u32            m_repeat_chapter_start;
	u32            m_repeat_chapter_end;
	u32            m_repeat_frame_start;
	u32            m_repeat_frame_end;
	u32            m_repeat_repetitions;

	u8             m_video_switch;
	bool           m_ch1_switch;
	bool           m_ch2_switch;
	u8             m_display_switch;
	u8             m_address_flag;
	u16            m_base_speed;
	u16            m_speed;
	u32            m_speed_accum;
	u32            m_curr_frame;

	u8             m_user_index_x;
	u8             m_user_index_y;
	u8             m_user_index_mode;
	u8             m_user_index_char_idx;
	u8             m_user_index_window_idx;
	char           m_user_index_chars[32];

};

#endif // MAME_MACHINE_LDP1450_H
