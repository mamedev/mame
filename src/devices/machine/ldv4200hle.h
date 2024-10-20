// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    ldv4200hle.h

    Pioneer LD-V4200 laserdisc player simulation.

*************************************************************************/

#ifndef MAME_MACHINE_LDV4200_H
#define MAME_MACHINE_LDV4200_H

#pragma once

#include "laserdsc.h"
#include "diserial.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(PIONEER_LDV4200HLE, pioneer_ldv4200hle_device)



//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

// Note: This should be included within the class rather than the global namespace.
// However, doing so results in a "called in a constant expression before its definition is complete" error for the enum values.
static constexpr uint16_t make_ldv4000_command(const char *str) { return ((uint8_t)str[0] << 8) | (uint8_t)str[1]; }



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pioneer_ldv4200hle_device

class pioneer_ldv4200hle_device : public laserdisc_device, public device_serial_interface
{
public:
	// construction/destruction
	pioneer_ldv4200hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto serial_tx() { return m_serial_tx.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// laserdisc overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	// diserial overrides
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

	TIMER_CALLBACK_MEMBER(process_vbi_data);

private:
	enum player_command : uint16_t
	{
		CMD_DOOR_OPEN            = make_ldv4000_command("OP"),
		CMD_DOOR_CLOSE           = make_ldv4000_command("CO"),
		CMD_REJECT               = make_ldv4000_command("RJ"),
		CMD_START                = make_ldv4000_command("SA"),
		CMD_PLAY                 = make_ldv4000_command("PL"),
		CMD_PAUSE                = make_ldv4000_command("PA"),
		CMD_STILL                = make_ldv4000_command("ST"),
		CMD_STEP_FORWARD         = make_ldv4000_command("SF"),
		CMD_STEP_REVERSE         = make_ldv4000_command("SR"),
		CMD_SCAN_FORWARD         = make_ldv4000_command("NF"),
		CMD_SCAN_REVERSE         = make_ldv4000_command("NR"),
		CMD_MULTISPEED_FORWARD   = make_ldv4000_command("MF"),
		CMD_MULTISPEED_REVERSE   = make_ldv4000_command("MR"),
		CMD_SPEED_SET            = make_ldv4000_command("SP"),
		CMD_SEARCH               = make_ldv4000_command("SE"),
		CMD_MULTITRACK_FORWARD   = make_ldv4000_command("JF"),
		CMD_MULTITRACK_REVERSE   = make_ldv4000_command("JR"),
		CMD_STOP_MARKER          = make_ldv4000_command("SM"),
		CMD_FRAME_SET            = make_ldv4000_command("FR"),
		CMD_TIME_SET             = make_ldv4000_command("TM"),
		CMD_CHAPTER_SET          = make_ldv4000_command("CH"),
		CMD_CLEAR                = make_ldv4000_command("CL"),
		CMD_LEADOUT_SYMBOL       = make_ldv4000_command("LO"),
		CMD_AUDIO_CTRL           = make_ldv4000_command("AD"),
		CMD_VIDEO_CTRL           = make_ldv4000_command("VD"),
		CMD_KEY_LOCK             = make_ldv4000_command("KL"),
		CMD_DISPLAY_CONTROL      = make_ldv4000_command("DS"),
		CMD_CLEAR_SCREEN         = make_ldv4000_command("CS"),
		CMD_PRINT_CHAR           = make_ldv4000_command("PR"),
		CMD_REQ_FRAME_NUMBER     = make_ldv4000_command("?F"),
		CMD_REQ_TIME_CODE        = make_ldv4000_command("?T"),
		CMD_REQ_CHAPTER_NUMBER   = make_ldv4000_command("?C"),
		CMD_REQ_PLAYER_MODE      = make_ldv4000_command("?P"),
		CMD_REQ_DISC_STATUS      = make_ldv4000_command("?D"),
		CMD_REQ_LDP_MODEL        = make_ldv4000_command("?X"),
		CMD_REQ_PIONEER_DISC_ID  = make_ldv4000_command("?U"),
		CMD_REQ_STANDARD_DISC_ID = make_ldv4000_command("$Y"),
		CMD_REQ_TV_SYSTEM        = make_ldv4000_command("?S"),
		CMD_COMMUNICATION_CTRL   = make_ldv4000_command("CM"),
		CMD_REQ_CCR_MODE         = make_ldv4000_command("?M"),
		CMD_REGISTER_A_SET       = make_ldv4000_command("RA"),
		CMD_REGISTER_B_SET       = make_ldv4000_command("RB"),
		CMD_REGISTER_C_SET       = make_ldv4000_command("RC"),
		CMD_REGISTER_D_SET       = make_ldv4000_command("RD"),
		CMD_REQ_REGISTER_A       = make_ldv4000_command("$A"),
		CMD_REQ_REGISTER_B       = make_ldv4000_command("$B"),
		CMD_REQ_REGISTER_C       = make_ldv4000_command("$C"),
		CMD_REQ_REGISTER_D       = make_ldv4000_command("$D"),
		CMD_REQ_INPUT_UNIT       = make_ldv4000_command("#I"),
		CMD_INPUT_NUMBER_WAIT    = make_ldv4000_command("?N")
	};

	enum player_mode : uint8_t
	{
		MODE_PARK,
		MODE_DOOR_OPEN,
		MODE_PAUSE,
		MODE_PLAY,
		MODE_MS_FORWARD,
		MODE_MS_REVERSE,
		MODE_SEARCH,
		MODE_STILL
	};

	enum address_mode : uint8_t
	{
		ADDRESS_FRAME,
		ADDRESS_TIME,
		ADDRESS_CHAPTER
	};

	enum error_code : uint8_t
	{
		ERR_NONE                = 0xff,
		ERR_COMMUNICATION       = 0,
		ERR_NOT_AVAILABLE       = 4,
		ERR_MISSING_ARGUMENT    = 6,
		ERR_DISC_NOT_LOADED     = 11,
		ERR_SEARCH              = 12,
		ERR_DEFOCUS             = 13,
		ERR_PICTURE_STOP        = 15,
		ERR_OTHER_INPUT         = 16,
		ERR_PANIC               = 99
	};

	void add_command_byte(uint8_t data);

	void queue_reply(const char *reply);
	void queue_error(error_code err);

	void normalize_command_buffer();
	void process_command_buffer();

	static uint32_t bcd_to_literal(uint32_t bcd);
	static bool is_number(char value);
	uint8_t parse_numeric_value(uint8_t cmd_index, uint32_t &value, error_code &err);
	uint8_t process_command(uint8_t cmd_index, uint32_t value, error_code &err);

	void begin_search(uint32_t value);

	void update_audio_squelch();
	void update_video_enable();

	// internal state
	devcb_write_line    m_serial_tx;
	emu_timer *         m_vbi_fetch;
	char                m_cmd_buffer[21];
	uint8_t             m_cmd_length;
	bool                m_cmd_running;
	char                m_reply_buffer[64];
	uint8_t             m_reply_write_index;
	uint8_t             m_reply_read_index;
	bool                m_replying;

	uint8_t             m_mode;                 // current player mode
	uint32_t            m_chapter;
	uint32_t            m_time;
	uint32_t            m_frame;                // raw frame index (CAV mode)
	uint32_t            m_search_chapter;
	uint32_t            m_search_frame;
	uint32_t            m_mark_chapter;
	uint32_t            m_mark_frame;

	uint8_t             m_key_lock;
	uint8_t             m_video_switch;
	uint8_t             m_audio_switch;
	uint8_t             m_display_switch;
	uint8_t             m_address_flag;
	uint16_t            m_speed;
	uint32_t            m_speed_accum;
	uint8_t             m_comm_ctrl;
	uint8_t             m_reg_a;
	uint8_t             m_reg_b;
	uint8_t             m_reg_c;
	uint8_t             m_reg_d;
	uint8_t             m_aux_port;
	uint32_t            m_curr_frame;
};

#endif // MAME_MACHINE_LDV4200_H
