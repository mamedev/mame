// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    ldv4200hle.cpp

    Pioneer LD-V4200 laserdisc player simulation.

**************************************************************************

    To do:

        * On-screen display support
        * Better CLV support
        * Chapter-search support
        * Commands that Time Traveler doesn't use:
          - Door Open/Close
          - Reject
          - Pause/Still
          - Scan Forward/Reverse
          - Multitrack-Jump Forward/Reverse
          - Clear
          - Leadout Symbol
          - Key Lock and on-screen functions
          - Status Requests
          - Registers A-D

*************************************************************************/


#include "emu.h"
#include "ldv4200hle.h"


#define LOG_COMMAND_BYTES       (1 << 1U)
#define LOG_COMMANDS            (1 << 2U)
#define LOG_COMMAND_BUFFERS     (1 << 3U)
#define LOG_REPLIES             (1 << 4U)
#define LOG_REPLY_BYTES         (1 << 5U)
#define LOG_SEARCHES            (1 << 6U)
#define LOG_STOPS               (1 << 7U)
#define LOG_SQUELCHES           (1 << 8U)
#define LOG_FRAMES              (1 << 9U)
#define LOG_ALL                 (LOG_COMMAND_BYTES | LOG_COMMANDS | LOG_COMMAND_BUFFERS | LOG_REPLY_BYTES | LOG_SEARCHES | LOG_STOPS | LOG_SQUELCHES | LOG_FRAMES)

#define VERBOSE (0)
#include "logmacro.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(PIONEER_LDV4200HLE, pioneer_ldv4200hle_device, "ldv4200hle", "Pioneer LD-V4200 HLE")



//**************************************************************************
//  PIONEER LD-V4200 HLE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  pioneer_ldv4200hle_device - constructor
//-------------------------------------------------

pioneer_ldv4200hle_device::pioneer_ldv4200hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, PIONEER_LDV4200HLE, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_serial_tx(*this)
	, m_vbi_fetch(nullptr)
	, m_cmd_length(0)
	, m_cmd_running(false)
	, m_reply_write_index(0)
	, m_reply_read_index(0)
	, m_mode(MODE_PARK)
	, m_chapter(0)
	, m_time(0)
	, m_frame(0)
	, m_search_chapter(~uint32_t(0))
	, m_search_frame(~uint32_t(0))
	, m_mark_chapter(~uint32_t(0))
	, m_mark_frame(~uint32_t(0))
	, m_key_lock(0)
	, m_video_switch(1)
	, m_audio_switch(0)
	, m_display_switch(0)
	, m_address_flag(ADDRESS_FRAME)
	, m_speed(60)
	, m_speed_accum(0)
	, m_comm_ctrl(3)
	, m_reg_a(3)
	, m_reg_b(0)
	, m_reg_c(0)
	, m_reg_d(0)
	, m_aux_port(3)
	, m_curr_frame(0)
{
}


//-------------------------------------------------
//  add_command_byte - handle a new data byte
//  received over the serial link
//-------------------------------------------------

void pioneer_ldv4200hle_device::add_command_byte(uint8_t data)
{
	// Space and L/F codes are ignored in command sequences, per LD-V4400 Level I & III User's Manual, pg. 4-8
	if (data == 0x20 || data == 0x0a)
		return;

	LOGMASKED(LOG_COMMAND_BYTES, "Command byte added: %02x\n", data);
	if (m_cmd_length < std::size(m_cmd_buffer))
	{
		m_cmd_buffer[m_cmd_length] = data;
		m_cmd_length++;
	}
	if (data == 0x0d)
	{
		normalize_command_buffer();
		process_command_buffer();
		m_cmd_length = 0;
	}
}


//-------------------------------------------------
//  queue_reply - queues a reply string to send
//  back to the host
//-------------------------------------------------

void pioneer_ldv4200hle_device::queue_reply(const char *reply)
{
	char print_buf[128];

	uint8_t max_writable = (uint8_t)std::size(m_reply_buffer);
	for (uint8_t i = 0; i < max_writable && reply[i] != 0; i++)
	{
		m_reply_buffer[m_reply_write_index] = reply[i];
		m_reply_write_index = (m_reply_write_index + 1) % max_writable;
		print_buf[i] = (reply[i] == '\x0d' ? 0 : reply[i]);
	}
	LOGMASKED(LOG_REPLIES, "Sending reply: %s\n", print_buf);

	if (!m_replying)
	{
		m_replying = true;
		LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", (uint8_t)m_reply_buffer[m_reply_read_index]);
		transmit_register_setup(m_reply_buffer[m_reply_read_index]);
	}
}


//-------------------------------------------------
//  queue_error - queues an error-code string to
//  send back to the host
//-------------------------------------------------

void pioneer_ldv4200hle_device::queue_error(error_code err)
{
	char buf[5] = { 'E', '0', '0', '\x0d', '\0' };
	buf[1] += err / 10;
	buf[2] += err % 10;
	queue_reply(buf);
}


//-------------------------------------------------
//  normalize_command_buffer - ensure any alphabet
//  characters in the command buffer are
//  upper-cased for matching purposes.
//-------------------------------------------------

void pioneer_ldv4200hle_device::normalize_command_buffer()
{
	char print_buf[64];
	for (uint8_t i = 0; i < m_cmd_length; i++)
	{
		if (m_cmd_buffer[i] >= 'a' && m_cmd_buffer[i] <= 'z')
		{
			m_cmd_buffer[i] &= ~0x20;
		}
		print_buf[i] = (char)m_cmd_buffer[i];
	}
	print_buf[m_cmd_length - 1] = '\0';
	LOGMASKED(LOG_COMMAND_BUFFERS, "Command Buffer: %02d:%02d:%02d: %s\n", (int)(machine().time().seconds() / 60), (int)(machine().time().seconds()) % 60, (machine().time() * 100).seconds() % 100, print_buf);
}


//-------------------------------------------------
//  process_command_buffer - process a command
//  line sent from the host
//-------------------------------------------------

void pioneer_ldv4200hle_device::process_command_buffer()
{
	if (m_cmd_length <= 1)
		return;

	error_code err = ERR_NONE;
	uint8_t cmd_index = 0;
	bool send_reply = true;
	while (cmd_index < m_cmd_length && err == ERR_NONE)
	{
		if (cmd_index == (m_cmd_length - 1) && m_cmd_buffer[cmd_index] == 0x0d)
		{
			break;
		}

		uint32_t value = ~uint32_t(0);
		if (is_number(m_cmd_buffer[cmd_index]))
		{
			cmd_index += parse_numeric_value(cmd_index, value, err);
		}
		if (err == ERR_NONE)
		{
			cmd_index += process_command(cmd_index, value, err);
			if (m_cmd_running)
			{
				send_reply = false;
			}
		}
	}

	if (send_reply)
	{
		if (err == ERR_NONE)
		{
			queue_reply("R\x0d");
		}
		else
		{
			queue_error(err);
		}
	}
}


//-------------------------------------------------
//  bcd_to_literal - converts a BCD value used in
//  commands a direct numeric value
//-------------------------------------------------

uint32_t pioneer_ldv4200hle_device::bcd_to_literal(uint32_t bcd)
{
	uint32_t value = 0;
	uint32_t shift = 28;
	uint32_t multiplier = 10000000;
	for (uint32_t i = 0; i < 8; i++)
	{
		uint32_t digit = (bcd >> shift) & 0xf;
		bcd &= ~(0xf << shift);

		value += digit * multiplier;

		multiplier /= 10;
		shift -= 4;
	}
	return value;
}


//-------------------------------------------------
//  is_number - indicates if a given character is
//  a numeric value
//-------------------------------------------------

bool pioneer_ldv4200hle_device::is_number(char value)
{
	return value >= '0' && value <= '9';
}


//-------------------------------------------------
//  parse_numeric_value - parses a numeric value
//  from the command buffer
//-------------------------------------------------

uint8_t pioneer_ldv4200hle_device::parse_numeric_value(uint8_t cmd_index, uint32_t &value, error_code &err)
{
	static const uint8_t MAX_NUMBER_LENGTH = 7;
	uint8_t number_length = 0;
	value = 0;
	while (number_length < MAX_NUMBER_LENGTH && is_number(m_cmd_buffer[cmd_index]) && cmd_index < m_cmd_length)
	{
		value *= 10;
		value += m_cmd_buffer[cmd_index] - '0';
		cmd_index++;
		number_length++;
	}

	if (cmd_index == m_cmd_length)
	{
		err = ERR_COMMUNICATION;
		return number_length;
	}

	if (number_length == MAX_NUMBER_LENGTH && is_number(m_cmd_buffer[cmd_index]))
	{
		err = ERR_MISSING_ARGUMENT;
		return number_length;
	}

	return number_length;
}


//-------------------------------------------------
//  process_command - processes a single command
//  from the command buffer
//-------------------------------------------------

uint8_t pioneer_ldv4200hle_device::process_command(uint8_t cmd_index, uint32_t value, error_code &err)
{
	const uint8_t remaining_bytes = m_cmd_length - cmd_index;
	if (remaining_bytes == 1 && m_cmd_buffer[cmd_index] == 0x0d)
	{
		// Done processing
		return remaining_bytes;
	}
	else if (remaining_bytes < 3)
	{
		// Not enough data in the buffer to form a valid command
		err = ERR_COMMUNICATION;
		return remaining_bytes;
	}

	uint16_t command = (m_cmd_buffer[cmd_index] << 8) | m_cmd_buffer[cmd_index + 1];
	switch (command)
	{
		case CMD_DOOR_OPEN:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Door Open\n", machine().describe_context());
			break;
		case CMD_DOOR_CLOSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Door Close\n", machine().describe_context());
			break;
		case CMD_REJECT:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Reject\n", machine().describe_context());
			break;
		case CMD_START:
			LOGMASKED(LOG_COMMANDS | LOG_SQUELCHES, "%s: Command: Start (squelching audio, unsquelching + disabling video)\n", machine().describe_context());
			m_mode = MODE_PAUSE;
			video_enable(false);
			set_video_squelch(false);
			set_audio_squelch(true, true);
			break;
		case CMD_PLAY:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Play [%d] (cancelling search)\n", machine().describe_context(), value == ~uint32_t(0) ? 0 : value);
			m_speed_accum = 0;
			m_mode = MODE_PLAY;
			update_audio_squelch();
			update_video_enable();
			if (value != ~uint32_t(0))
			{
				LOGMASKED(LOG_COMMANDS, "%s:          Setting stop frame\n", machine().describe_context());
				m_mark_frame = value + 1;
				m_cmd_running = true;
			}
			m_search_frame = ~uint32_t(0);
			m_search_chapter = ~uint32_t(0);
			break;
		case CMD_PAUSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Pause\n", machine().describe_context());
			m_mode = MODE_PAUSE;
			video_enable(false);
			set_audio_squelch(true, true);
			break;
		case CMD_STILL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Still\n", machine().describe_context());
			m_mode = MODE_STILL;
			set_audio_squelch(true, true);
			break;
		case CMD_STEP_FORWARD:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Step Forward\n", machine().describe_context());
			m_mode = MODE_STILL;
			set_audio_squelch(true, true);
			m_mark_frame = ~uint32_t(0);
			advance_slider(1);
			break;
		case CMD_STEP_REVERSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Step Reverse\n", machine().describe_context());
			m_mode = MODE_STILL;
			set_audio_squelch(true, true);
			m_mark_frame = ~uint32_t(0);
			advance_slider(-1);
			break;
		case CMD_SCAN_FORWARD:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Scan Forward\n", machine().describe_context());
			break;
		case CMD_SCAN_REVERSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Scan Reverse\n", machine().describe_context());
			break;
		case CMD_MULTISPEED_FORWARD:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Multi-Speed Forward (%d) (cancelling search)\n", machine().describe_context(), value == ~uint32_t(0) ? 0 : value);
			m_search_frame = ~uint32_t(0);
			m_search_chapter = ~uint32_t(0);
			if (value + 1 == m_curr_frame)
			{
				LOGMASKED(LOG_COMMANDS, "%s:          Already at desired frame, entering still/pause\n", machine().describe_context(), value == ~uint32_t(0) ? 0 : value);
				if (is_cav_disc())
				{
					m_mode = MODE_STILL;
					update_video_enable();
				}
				else
				{
					m_mode = MODE_PAUSE;
					video_enable(false);
				}
			}
			else
			{
				m_mode = MODE_MS_FORWARD;
				if (value != ~uint32_t(0))
				{
					LOGMASKED(LOG_COMMANDS, "%s:          Setting stop frame\n", machine().describe_context());
					m_mark_frame = value + 1;
					m_cmd_running = true;
				}
			}
			break;
		case CMD_MULTISPEED_REVERSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Multi-Speed Reverse (%d) (cancelling search)\n", machine().describe_context(), value == ~uint32_t(0) ? 0 : value);
			m_search_frame = ~uint32_t(0);
			m_search_chapter = ~uint32_t(0);
			if (value + 1 == m_curr_frame)
			{
				LOGMASKED(LOG_COMMANDS, "%s:          Already at desired frame, entering still/pause\n", machine().describe_context(), value == ~uint32_t(0) ? 0 : value);
				if (is_cav_disc())
				{
					m_mode = MODE_STILL;
					update_video_enable();
				}
				else
				{
					m_mode = MODE_PAUSE;
					video_enable(false);
				}
			}
			else
			{
				m_mode = MODE_MS_REVERSE;
				if (value != ~uint32_t(0))
				{
					LOGMASKED(LOG_COMMANDS, "%s:          Setting stop frame\n", machine().describe_context());
					m_mark_frame = value + 1;
					m_cmd_running = true;
				}
			}
			break;
		case CMD_SPEED_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Speed Set %05d\n", machine().describe_context(), value);
			if (is_cav_disc())
			{
				m_speed = value;
				if (m_speed == 0)
				{
					m_speed = 1;
				}
				else if (m_speed > 255)
				{
					m_speed = 255;
				}
			}
			else
			{
				err = ERR_NOT_AVAILABLE;
			}
			break;
		case CMD_SEARCH:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Search %d\n", machine().describe_context(), value);
			begin_search(value);
			m_cmd_running = true;
			m_mode = MODE_SEARCH;
			break;
		case CMD_MULTITRACK_FORWARD:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Multi-Track Jump Forward %05d\n", machine().describe_context(), value);
			break;
		case CMD_MULTITRACK_REVERSE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Multi-Track Jump Reverse %05d\n", machine().describe_context(), value);
			break;
		case CMD_STOP_MARKER:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Stop Marker %d\n", machine().describe_context(), value);
			if (m_address_flag == ADDRESS_FRAME)
			{
				m_mark_frame = value + 1;
			}
			else if (m_address_flag == ADDRESS_CHAPTER)
			{
				m_mark_chapter = value;
			}
			break;
		case CMD_FRAME_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Frame Set\n", machine().describe_context());
			m_address_flag = ADDRESS_FRAME;
			break;
		case CMD_TIME_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Time Set\n", machine().describe_context());
			if (!is_cav_disc())
			{
				m_address_flag = ADDRESS_TIME;
			}
			else
			{
				err = ERR_NOT_AVAILABLE;
			}
			break;
		case CMD_CHAPTER_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Chapter Set\n", machine().describe_context());
			m_address_flag = ADDRESS_CHAPTER;
			break;
		case CMD_CLEAR:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Clear\n", machine().describe_context());
			break;
		case CMD_LEADOUT_SYMBOL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Lead-Out Symbol\n", machine().describe_context());
			break;
		case CMD_AUDIO_CTRL:
			m_audio_switch = value;
			LOGMASKED(LOG_COMMANDS, "%s: Command: Audio Control %05d\n", machine().describe_context(), value);
			update_audio_squelch();
			break;
		case CMD_VIDEO_CTRL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Video Control %05d\n", machine().describe_context(), value);
			m_video_switch = value;
			update_video_enable();
			break;
		case CMD_KEY_LOCK:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Key Lock %05d\n", machine().describe_context(), value);
			break;
		case CMD_DISPLAY_CONTROL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Display Control %05d\n", machine().describe_context(), value);
			break;
		case CMD_CLEAR_SCREEN:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Clear Screen\n", machine().describe_context());
			break;
		case CMD_PRINT_CHAR:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Print Character %05d\n", machine().describe_context(), value);
			break;
		case CMD_REQ_FRAME_NUMBER:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Frame Number Request\n", machine().describe_context());
			break;
		case CMD_REQ_TIME_CODE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Time Code Request\n", machine().describe_context());
			break;
		case CMD_REQ_CHAPTER_NUMBER:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Chapter Number Request\n", machine().describe_context());
			break;
		case CMD_REQ_PLAYER_MODE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Player Active Mode Request\n", machine().describe_context());
			break;
		case CMD_REQ_DISC_STATUS:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Disc Status Request\n", machine().describe_context());
			break;
		case CMD_REQ_LDP_MODEL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: LDP Model Name Request\n", machine().describe_context());
			break;
		case CMD_REQ_PIONEER_DISC_ID:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Pioneer User's Code Request (Disc ID)\n", machine().describe_context());
			break;
		case CMD_REQ_STANDARD_DISC_ID:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Standard User's Code Request (Disc ID)\n", machine().describe_context());
			break;
		case CMD_REQ_TV_SYSTEM:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Television System Request\n", machine().describe_context());
			break;
		case CMD_COMMUNICATION_CTRL:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Communication Control %05d\n", machine().describe_context(), value);
			break;
		case CMD_REQ_CCR_MODE:
			LOGMASKED(LOG_COMMANDS, "%s: Command: CCR Mode Request\n", machine().describe_context());
			break;
		case CMD_REGISTER_A_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register A Set (Display) %d\n", machine().describe_context(), value);
			break;
		case CMD_REGISTER_B_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register B Set (Squelch Control) %d\n", machine().describe_context(), value);
			break;
		case CMD_REGISTER_C_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register C Set (Miscellaneous) %d\n", machine().describe_context(), value);
			break;
		case CMD_REGISTER_D_SET:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register D Set (RS-232) %d\n", machine().describe_context(), value);
			break;
		case CMD_REQ_REGISTER_A:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register A Request (Display)\n", machine().describe_context());
			break;
		case CMD_REQ_REGISTER_B:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register B Request (Squelch Control)\n", machine().describe_context());
			break;
		case CMD_REQ_REGISTER_C:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register C Request (Miscellaneous)\n", machine().describe_context());
			break;
		case CMD_REQ_REGISTER_D:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Register D Request (RS-232)\n", machine().describe_context());
			break;
		case CMD_REQ_INPUT_UNIT:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Input Unit Request\n", machine().describe_context());
			break;
		case CMD_INPUT_NUMBER_WAIT:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Input Number Wait\n", machine().describe_context());
			break;
		default:
			LOGMASKED(LOG_COMMANDS, "%s: Command: Unknown (%c%c)\n", machine().describe_context(), m_cmd_buffer[cmd_index], m_cmd_buffer[cmd_index + 1]);
			err = ERR_NOT_AVAILABLE;
			break;
	}

	return 2;
}


//-------------------------------------------------
//  begin_search - initiates a search operation
//-------------------------------------------------

void pioneer_ldv4200hle_device::begin_search(uint32_t value)
{
	if (m_address_flag == ADDRESS_FRAME)
	{
		m_search_frame = value + 1;
		LOGMASKED(LOG_SEARCHES, "%s: Beginning search from frame address %d\n", machine().describe_context(), value);
	}
	else if (m_address_flag == ADDRESS_CHAPTER)
	{
		m_search_chapter = value;
		LOGMASKED(LOG_SEARCHES, "%s: Beginning search from chapter address %d\n", machine().describe_context(), value);
	}

	set_audio_squelch(true, true);

	if (std::abs((int32_t)m_search_frame - (int32_t)m_curr_frame) > 100)
	{
		LOGMASKED(LOG_SEARCHES | LOG_SQUELCHES, "%s: Search distance is outside +/- 100 frames, squelching audio+video\n", machine().describe_context());
		video_enable(false);
		set_audio_squelch(true, true);
	}
	else
	{
		LOGMASKED(LOG_SEARCHES | LOG_SQUELCHES, "%s: Search distance is within +/- 100 frames, squelching audio and doing live search\n", machine().describe_context());
	}
}


//-------------------------------------------------
//  update_audio_squelch - set audio squelch state
//  on the base device based on our audio switch
//-------------------------------------------------

void pioneer_ldv4200hle_device::update_audio_squelch()
{
	const bool squelch_both = (m_mode == MODE_STILL || m_mode == MODE_PAUSE || m_mode == MODE_SEARCH);
	const bool squelch_left = !(m_audio_switch == 1 || m_audio_switch == 3) || squelch_both;
	const bool squelch_right = !(m_audio_switch == 2 || m_audio_switch == 3) || squelch_both;
	set_audio_squelch(squelch_left, squelch_right);
	LOGMASKED(LOG_SQUELCHES, "%s: Updating audio squelch (L:%d, R:%d)\n", machine().describe_context(), squelch_left, squelch_right);
}


//-------------------------------------------------
//  update_video_enable - set video enable state
//  on the base device based on our video switch
//-------------------------------------------------

void pioneer_ldv4200hle_device::update_video_enable()
{
	video_enable(m_video_switch == 1 && (m_mode == MODE_STILL || m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE));
	LOGMASKED(LOG_SQUELCHES, "%s: Updating video enable (Switch:%d, Mode:%d)\n", machine().describe_context(), m_video_switch, m_mode);
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void pioneer_ldv4200hle_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	m_serial_tx.resolve_safe();

	// allocate timers
	m_vbi_fetch = timer_alloc(FUNC(pioneer_ldv4200hle_device::process_vbi_data), this);

	// register state saving
	save_item(NAME(m_cmd_buffer));
	save_item(NAME(m_cmd_length));
	save_item(NAME(m_cmd_running));
	save_item(NAME(m_reply_buffer));
	save_item(NAME(m_reply_write_index));
	save_item(NAME(m_reply_read_index));
	save_item(NAME(m_replying));
	save_item(NAME(m_mode));
	save_item(NAME(m_chapter));
	save_item(NAME(m_time));
	save_item(NAME(m_frame));
	save_item(NAME(m_search_chapter));
	save_item(NAME(m_search_frame));
	save_item(NAME(m_mark_chapter));
	save_item(NAME(m_mark_frame));
	save_item(NAME(m_key_lock));
	save_item(NAME(m_video_switch));
	save_item(NAME(m_audio_switch));
	save_item(NAME(m_display_switch));
	save_item(NAME(m_address_flag));
	save_item(NAME(m_speed));
	save_item(NAME(m_speed_accum));
	save_item(NAME(m_comm_ctrl));
	save_item(NAME(m_reg_a));
	save_item(NAME(m_reg_b));
	save_item(NAME(m_reg_c));
	save_item(NAME(m_reg_d));
	save_item(NAME(m_aux_port));
	save_item(NAME(m_curr_frame));
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void pioneer_ldv4200hle_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();

	// initialize diserial
	set_tra_rate(attotime::from_hz(4800));
	set_rcv_rate(attotime::from_hz(4800));
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	// reset our state
	m_vbi_fetch->adjust(attotime::never);

	std::fill_n(m_cmd_buffer, 0, std::size(m_cmd_buffer));
	m_cmd_length = 0;
	m_cmd_running = false;
	std::fill_n(m_reply_buffer, 0, std::size(m_reply_buffer));
	m_reply_write_index = 0;
	m_reply_read_index = 0;
	m_replying = false;
	m_mode = MODE_PARK;
	m_chapter = 0;
	m_time = 0;
	m_frame = 0;
	m_search_chapter = ~uint32_t(0);
	m_search_frame = ~uint32_t(0);
	m_mark_chapter = ~uint32_t(0);
	m_mark_frame = ~uint32_t(0);
	m_key_lock = 0;
	m_video_switch = 1;
	m_audio_switch = 0;
	m_display_switch = 0;
	m_address_flag = ADDRESS_FRAME;
	m_speed = 60;
	m_speed_accum = 0;
	m_comm_ctrl = 3;
	m_reg_a = 3;
	m_reg_b = 0;
	m_reg_c = 0;
	m_reg_d = 0;
	m_aux_port = 3;
	m_curr_frame = 0;

	video_enable(false);
	set_audio_squelch(true, true);
}


//-------------------------------------------------
//  process_vbi_data - process VBI data and
//  act on search/play seeking
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv4200hle_device::process_vbi_data)
{
	uint32_t line = get_field_code(LASERDISC_CODE_LINE1718, false);
	if ((line & 0xf80000) == 0xf80000 || line == VBI_CODE_LEADIN || line == VBI_CODE_LEADOUT)
	{
		uint32_t old_frame = m_curr_frame;
		if (line == VBI_CODE_LEADIN)
			m_curr_frame = 0;
		else if (line == VBI_CODE_LEADOUT)
			m_curr_frame = 54000;
		else
			m_curr_frame = bcd_to_literal(line & 0x7ffff);

		LOGMASKED(LOG_FRAMES, "Current frame is %d (VBI 16: %06x, VBI 17: %06x, VBI 18: %06x, VBI 1718: %06x\n", m_curr_frame,
			get_field_code(LASERDISC_CODE_LINE16, false),
			get_field_code(LASERDISC_CODE_LINE17, false),
			get_field_code(LASERDISC_CODE_LINE18, false),
			line);

		if (m_mode != MODE_STILL && m_mode != MODE_PAUSE)
		{
			if (m_mark_frame != ~uint32_t(0) && m_search_frame == ~uint32_t(0))
			{
				int32_t old_delta = (int32_t)m_mark_frame - (int32_t)old_frame;
				int32_t curr_delta = (int32_t)m_mark_frame - (int32_t)m_curr_frame;
				LOGMASKED(LOG_STOPS, "%s: Stop Mark is currently %d, old frame is %d, current frame is %d, old delta %d, curr delta %d\n", machine().describe_context(), m_mark_frame, old_frame, m_curr_frame, old_delta, curr_delta);
				if (curr_delta == 0 || std::signbit(old_delta) != std::signbit(curr_delta))
				{
					m_mark_frame = ~uint32_t(0);
					if (is_cav_disc())
					{
						LOGMASKED(LOG_STOPS | LOG_SQUELCHES, "%s: Stop Mark: Zero delta w/ CAV disc, entering still mode and squelching audio\n", machine().describe_context());
						m_mode = MODE_STILL;
						update_video_enable();
					}
					else
					{
						LOGMASKED(LOG_STOPS | LOG_SQUELCHES, "%s: Stop Mark: Zero delta w/ CLV disc, entering still mode and squelching video+audio\n", machine().describe_context());
						m_mode = MODE_PAUSE;
						video_enable(false);
					}

					set_audio_squelch(true, true);

					if (m_cmd_running)
					{
						LOGMASKED(LOG_SEARCHES | LOG_COMMANDS, "%s: Stop Mark: Command running, sending reply\n", machine().describe_context());
						m_cmd_running = false;
						queue_reply("R\x0d");
					}
				}
			}

			if (m_search_frame != ~uint32_t(0))
			{
				// TODO: Chapter-search support
				int32_t delta = (int32_t)m_search_frame - (int32_t)m_curr_frame;
				LOGMASKED(LOG_SEARCHES, "%s: Searching from current frame %d with delta %d\n", machine().describe_context(), m_curr_frame, delta);
				if (delta == 0)
				{
					// We've found our frame, enter play, pause or still mode.
					m_search_frame = ~uint32_t(0);
					if (is_cav_disc())
					{
						LOGMASKED(LOG_SEARCHES | LOG_SQUELCHES, "%s: Search Mark: Zero delta w/ CAV disc, entering still mode and squelching audio\n", machine().describe_context());
						m_mode = MODE_STILL;
						update_video_enable();
					}
					else
					{
						LOGMASKED(LOG_SEARCHES | LOG_SQUELCHES, "%s: Search Mark: Zero delta w/ CLV disc, entering still mode and squelching video+audio\n", machine().describe_context());
						m_mode = MODE_PAUSE;
						video_enable(false);
					}

					set_audio_squelch(true, true);

					if (m_cmd_running)
					{
						LOGMASKED(LOG_SEARCHES | LOG_COMMANDS, "%s: Search Mark: Command running, sending reply\n", machine().describe_context());
						m_cmd_running = false;
						queue_reply("R\x0d");
					}
				}
				else if (delta <= 2 && delta > 0)
				{
					LOGMASKED(LOG_SEARCHES, "%s: Positive near delta, letting disc run to current\n", machine().describe_context());
					// We're approaching our frame, let it run up.
				}
				else
				{
					if (delta < 0)
					{
						advance_slider(std::min(-2, delta / 2));
					}
					else
					{
						advance_slider(std::max(1, delta / 2));
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void pioneer_ldv4200hle_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// set a timer to fetch the VBI data when it is ready
	if (m_mode > MODE_DOOR_OPEN)
	{
		m_vbi_fetch->adjust(screen().time_until_pos(19*2));
	}
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

int32_t pioneer_ldv4200hle_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (!fieldnum)
		return 0;

	if (m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
	{
		m_speed_accum += m_speed;
		int elapsed_tracks = m_speed_accum / 60;
		m_speed_accum -= elapsed_tracks * 60;
		if (m_mode == MODE_MS_REVERSE)
			elapsed_tracks *= -1;

		if (m_mark_frame != ~uint32_t(0))
		{
			int32_t jump_frame = (int32_t)m_curr_frame + elapsed_tracks;
			int32_t curr_delta = (int32_t)m_mark_frame - (int32_t)m_curr_frame;
			int32_t next_delta = (int32_t)m_mark_frame - (int32_t)jump_frame;
			if (std::signbit(curr_delta) != std::signbit(next_delta))
			{
				elapsed_tracks = curr_delta;
			}
		}
		return elapsed_tracks;
	}


	if (m_mode == MODE_PLAY || m_mode == MODE_SEARCH)
	{
		return 1;
	}

	return 0;
}


//-------------------------------------------------
//  rcv_complete - called by diserial when we
//  have received a complete byte
//-------------------------------------------------

void pioneer_ldv4200hle_device::rcv_complete()
{
	receive_register_extract();
	add_command_byte(get_received_char());
}


//-------------------------------------------------
//  tra_complete - called by diserial when we
//  have transmitted a complete byte
//-------------------------------------------------

void pioneer_ldv4200hle_device::tra_complete()
{
	m_reply_read_index = (m_reply_read_index + 1) % (uint8_t)std::size(m_reply_buffer);
	if (m_reply_read_index != m_reply_write_index)
	{
		uint8_t data = (uint8_t)m_reply_buffer[m_reply_read_index];
		LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", data);
		transmit_register_setup(data);
	}
	else
	{
		m_replying = false;
	}
}


//-------------------------------------------------
//  tra_callback - called by diserial when we
//  transmit a single bit
//-------------------------------------------------

void pioneer_ldv4200hle_device::tra_callback()
{
	m_serial_tx(transmit_register_get_data_bit());
}
