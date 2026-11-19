// license:BSD-3-Clause
// copyright-holders:bugjosh
/***************************************************************************

    Sony LDP-1000A laserdisc player HLE

***************************************************************************/

#ifndef MAME_MACHINE_LDP1000_H
#define MAME_MACHINE_LDP1000_H

#pragma once

#include "laserdsc.h"

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(SONY_LDP1000, sony_ldp1000_device)

// ======================> sony_ldp1000_device

class sony_ldp1000_device : public laserdisc_device, public device_serial_interface
{
public:
	// construction/destruction
	sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto serial_tx() { return m_serial_tx.bind(); }

	// baud rate selected by jumpers at S1 on the MP-11 board (factory setting is 1200)
	void set_baud(u32 baud) { m_baud = baud; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// laserdisc_device implementation
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override { }
	virtual s32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	// device_serial_interface implementation
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

private:
	// command codes (0x30-0x39 are the numeric keys)
	enum : u8
	{
		CMD_F_PLAY = 0x3a, CMD_F_FAST, CMD_F_SLOW, CMD_F_STEP, CMD_F_SCAN, CMD_STOP,
		CMD_ENTER, CMD_CE, CMD_MENU, CMD_SEARCH, CMD_REPEAT, CMD_SEGMENT,
		CMD_CH1_ON, CMD_CH1_OFF, CMD_CH2_ON, CMD_CH2_OFF,
		CMD_R_PLAY, CMD_R_FAST, CMD_R_SLOW, CMD_R_STEP, CMD_R_SCAN, CMD_STILL,
		CMD_INDEX_ON, CMD_INDEX_OFF, CMD_DUMP_IN, CMD_DUMP_OUT, CMD_SEG_MODE, CMD_FRAME_MODE, CMD_CL,
		CMD_PGM, CMD_RUN, CMD_END, CMD_MEMORY, CMD_M_SEARCH, CMD_SKIP, CMD_INT, CMD_REVIEW, CMD_MODE,
		CMD_ADDR_INQ, CMD_CONTINUE, CMD_MOTOR_ON, CMD_MOTOR_OFF, CMD_CH1, CMD_CH2, CMD_INDEX,
		CMD_STATUS_INQ, CMD_DISC_ID_INQ, CMD_CHAPTER_MODE
	};

	// return codes
	enum : u8
	{
		RET_COMPLETION  = 0x01,
		RET_ERROR       = 0x02,
		RET_NOT_TARGET  = 0x05,
		RET_NO_FRAME    = 0x06,
		RET_ACK         = 0x0a,
		RET_NAK         = 0x0b
	};

	// what the numeric keys are being entered for
	enum : u8
	{
		INPUT_NONE,
		INPUT_SEARCH,           // SEARCH frame number
		INPUT_REPEAT_FRAME,     // REPEAT end frame number
		INPUT_REPEAT_COUNT,     // REPEAT number of repetitions
		INPUT_REPEAT_STEP,      // REPEAT step rate
		INPUT_STEP              // F/R-STEP step rate
	};

	// operation that ends with a return code, or during which the player doesn't communicate
	enum : u8
	{
		OP_NONE,
		OP_INIT,                // spin-up after power on or MOTOR ON
		OP_MOTOR_OFF,           // returning the sled and stopping the spindle after MOTOR OFF
		OP_SEARCH,
		OP_REPEAT,              // playing towards the end frame
		OP_REPEAT_RETURN        // searching back to the start frame for the next repetition
	};

	// playback speeds, as selected by the F/R keys
	enum : u8
	{
		SPEED_PLAY,
		SPEED_FAST,
		SPEED_SLOW,
		SPEED_STEP,
		SPEED_SCAN
	};

	void process_command(u8 data);
	u8 enter_command();
	void queue_reply(const u8 *data, unsigned length, const attotime &delay);
	void queue_reply(u8 data, const attotime &delay) { queue_reply(&data, 1, delay); }
	TIMER_CALLBACK_MEMBER(send_reply);
	TIMER_CALLBACK_MEMBER(motor_stopped);

	void begin_init();
	void begin_search(s32 frame);
	void begin_repeat();
	void play(u8 speed, bool reverse, u32 step_rate);
	void still(bool stop);
	void finish_operation(u8 code);
	void update_audio_squelch();
	void state_changed(player_state oldstate);

	devcb_write_line m_serial_tx;
	emu_timer *m_reply_timer;
	emu_timer *m_motor_timer;
	u32 m_baud;

	// reply bytes waiting to be sent
	u8 m_reply[16];
	u8 m_reply_read;
	u8 m_reply_count;

	// command input state
	u8 m_input;
	u32 m_number;
	u8 m_digits;
	bool m_error;

	// current operation
	u8 m_operation;
	u32 m_search_fields;
	bool m_search_outside;      // the search went into the lead-in or lead-out
	s32 m_repeat_start;
	s32 m_repeat_end;
	s32 m_repeat_count;         // 0 means repeat indefinitely
	u8 m_repeat_speed;
	u32 m_step_rate;

	// playback state
	u8 m_speed;
	bool m_reverse;
	bool m_stopped;
	bool m_motor_off;
	bool m_ch1;
	bool m_ch2;
	s32 m_last_frame;           // last frame number read from the disc
	player_state m_still_saved_state;   // mode before STILL, for CONTINUE
	s32 m_still_saved_param;
};

#endif // MAME_MACHINE_LDP1000_H
