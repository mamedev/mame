// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*************************************************************************

    ldv1000hle.cpp

    Pioneer LDV-1000 laserdisc player simulation.

**************************************************************************

    To do:

        * On-screen display support
        * Commands that Dragon's Lair doesn't use

*************************************************************************/


#include "emu.h"
#include "ldv1000hle.h"


#define LOG_COMMAND_BYTES       (1U << 1)
#define LOG_COMMANDS            (1U << 2)
#define LOG_REPLIES             (1U << 3)
#define LOG_SEARCHES            (1U << 4)
#define LOG_STOPS               (1U << 5)
#define LOG_SQUELCHES           (1U << 6)
#define LOG_FRAMES              (1U << 7)
#define LOG_ALL                 (LOG_COMMAND_BYTES | LOG_COMMANDS | LOG_REPLIES | LOG_SEARCHES | LOG_STOPS | LOG_SQUELCHES | LOG_FRAMES)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PIONEER_LDV1000HLE, pioneer_ldv1000hle_device, "ldv1000hle", "Pioneer LDV-1000 HLE")


//-------------------------------------------------
//  pioneer_ldv1000hle_device - constructor
//-------------------------------------------------

pioneer_ldv1000hle_device::pioneer_ldv1000hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: parallel_laserdisc_device(mconfig, PIONEER_LDV1000HLE, tag, owner, clock)
	, m_vbi_fetch(nullptr)
	, m_stop_timer(nullptr)
	, m_park_strobe_timer(nullptr)
	, m_assert_status_strobe_timer(nullptr)
	, m_deassert_status_strobe_timer(nullptr)
	, m_assert_command_strobe_timer(nullptr)
	, m_deassert_command_strobe_timer(nullptr)
	, m_cmd_length(0)
	, m_status(STATUS_PARK | STATUS_READY)
	, m_pre_stop_status(0)
	, m_mode(MODE_PARK)
	, m_curr_frame(0)
	, m_search_frame(~0U)
	, m_stop_frame(~0U)
	, m_cmd_number_length(0)
	, m_curr_register(0)
	, m_scan_speed(60)
	, m_scan_speed_accum(0)
	, m_play_speed(0.0)
	, m_status_strobe(true)
	, m_command_strobe(true)
{
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void pioneer_ldv1000hle_device::device_start()
{
	// pass through to the parent
	parallel_laserdisc_device::device_start();

	// allocate timers
	m_vbi_fetch = timer_alloc(FUNC(pioneer_ldv1000hle_device::process_vbi_data), this);
	m_stop_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::resume_from_stop), this);
	m_park_strobe_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::park_strobe_tick), this);;
	m_assert_status_strobe_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::assert_status_strobe), this);
	m_deassert_status_strobe_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::deassert_status_strobe), this);
	m_assert_command_strobe_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::assert_command_strobe), this);
	m_deassert_command_strobe_timer = timer_alloc(FUNC(pioneer_ldv1000hle_device::deassert_command_strobe), this);

	// register state saving
	save_item(NAME(m_cmd_buffer));
	save_item(NAME(m_cmd_length));
	save_item(NAME(m_status));
	save_item(NAME(m_pre_stop_status));
	save_item(NAME(m_mode));
	save_item(NAME(m_curr_frame));
	save_item(NAME(m_search_frame));
	save_item(NAME(m_stop_frame));
	save_item(NAME(m_cmd_number));
	save_item(NAME(m_cmd_number_length));
	save_item(NAME(m_user_ram));
	save_item(NAME(m_curr_register));
	save_item(NAME(m_scan_speed));
	save_item(NAME(m_scan_speed_accum));
	save_item(NAME(m_play_speed));
	save_item(NAME(m_audio_enable));
	save_item(NAME(m_status_strobe));
	save_item(NAME(m_command_strobe));
}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void pioneer_ldv1000hle_device::device_reset()
{
	// pass through to the parent
	parallel_laserdisc_device::device_reset();

	// reset our state
	m_vbi_fetch->adjust(attotime::never);

	std::fill_n(m_cmd_buffer, std::size(m_cmd_buffer), 0);
	m_cmd_length = 0;
	m_status = STATUS_PARK | STATUS_READY;
	m_pre_stop_status = 0;
	m_mode = MODE_PARK;
	m_curr_frame = 0;
	m_search_frame = ~0U;
	m_stop_frame = ~0U;
	m_cmd_number_length = 0;
	m_curr_register = 0;
	m_scan_speed = 0;
	m_scan_speed_accum = 0;
	m_audio_enable[0] = true;
	m_audio_enable[1] = true;
	m_status_strobe = true;
	m_command_strobe = true;

	set_video_squelch(false);

	m_park_strobe_timer->adjust(attotime::from_msec(21), 0, attotime::from_msec(21));
}


//-------------------------------------------------
//  data_w - handle a new data byte
//  received over the parallel link
//-------------------------------------------------

void pioneer_ldv1000hle_device::data_w(u8 data)
{
	LOGMASKED(LOG_COMMAND_BYTES, "data_w: Command byte added: %02x\n", data);
	if (m_cmd_length < std::size(m_cmd_buffer))
	{
		m_cmd_buffer[m_cmd_length] = data;
		m_cmd_length++;
	}

	if (is_command_byte(data))
	{
		LOGMASKED(LOG_COMMAND_BYTES, "data_w: %02x is a command byte, processing command buffer\n", data);
		process_command_buffer();
	}
	else
	{
		LOGMASKED(LOG_COMMAND_BYTES, "data_w: %02x is not a command byte, leaving in the queue for now\n", data);
	}
}


//-------------------------------------------------
//  data_r - returns the current status byte or
//  reply data when necessary
//-------------------------------------------------

u8 pioneer_ldv1000hle_device::data_r()
{
	u8 data = m_status;
	LOGMASKED(LOG_REPLIES, "Sending reply %02x\n", data);
	return data;
}


//-------------------------------------------------
//  enter_w - used to request service by a host
//  application, not currently implemented
//-------------------------------------------------

void pioneer_ldv1000hle_device::enter_w(int state)
{
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void pioneer_ldv1000hle_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// set a timer to fetch the VBI data when it is ready
	if (m_mode != MODE_PARK)
	{
		m_vbi_fetch->adjust(screen().time_until_pos(19*2), fieldnum);
	}
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

s32 pioneer_ldv1000hle_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (m_mode != MODE_PARK)
	{
		m_assert_status_strobe_timer->adjust(attotime::from_usec(500));
	}

	if (!fieldnum)
		return 0;

	if (m_mode == MODE_SCAN_FORWARD || m_mode == MODE_SCAN_REVERSE)
	{
		m_scan_speed_accum += m_scan_speed;
		int elapsed_tracks = m_scan_speed_accum / 60;
		m_scan_speed_accum -= elapsed_tracks * 60;
		if (m_mode == MODE_SCAN_REVERSE)
			elapsed_tracks *= -1;

		if (m_stop_frame != ~0U)
		{
			const int next_frame = (int)m_curr_frame + elapsed_tracks;
			if (next_frame >= m_stop_frame)
			{
				// If we've landed on (or exceeded) our desired frame and are in a SKIP FORWARD command, resume from the desired frame.
				elapsed_tracks = (int)m_stop_frame - (int)m_curr_frame;
				m_stop_frame = ~0U;
				set_playing(STATUS_FORWARD, m_play_speed);
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
//  process_vbi_data - process VBI data and
//  act on search/play seeking
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::process_vbi_data)
{
	if (param == 0)
	{
		update_video_enable();
		update_audio_enable();
	}

	uint32_t line = get_field_code(LASERDISC_CODE_LINE1718, false);
	if ((line & 0xf00000) == 0xf00000 || line == VBI_CODE_LEADIN || line == VBI_CODE_LEADOUT)
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

		if (m_mode != MODE_STOP)
		{
			if (m_stop_frame != ~0U && m_search_frame == ~0U)
			{
				s32 old_delta = (s32)m_stop_frame - (s32)old_frame;
				s32 curr_delta = (s32)m_stop_frame - (s32)m_curr_frame;
				LOGMASKED(LOG_STOPS, "%s: Stop frame is currently %d, old frame is %d, current frame is %d, old delta %d, curr delta %d\n", machine().describe_context(), m_stop_frame, old_frame, m_curr_frame, old_delta, curr_delta);
				if (curr_delta == 0 || (old_delta < 0) != (curr_delta < 0))
				{
					m_stop_frame = ~0U;
					LOGMASKED(LOG_STOPS, "%s: Stop frame: Zero delta, entering stop mode\n", machine().describe_context());
					set_stopped((m_status & STATUS_READY) | STATUS_STOP);
				}
			}

			if (m_search_frame != ~0U)
			{
				s32 delta = (s32)m_search_frame - (s32)m_curr_frame;
				LOGMASKED(LOG_SEARCHES, "%s: Searching from current frame %d with delta %d\n", machine().describe_context(), m_curr_frame, delta);
				if (delta == 0)
				{
					// We've found our frame, enter play, pause or still mode.
					m_search_frame = ~0U;
					LOGMASKED(LOG_SEARCHES, "%s: Search Mark: Zero delta, entering still mode\n", machine().describe_context());
					set_stopped(STATUS_SEARCH_FINISH);
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
//  resume_from_stop - resume from a previous
//  STOP command
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::resume_from_stop)
{
	set_playing(m_pre_stop_status, m_play_speed);
}


//-------------------------------------------------
//  park_strobe_tick - used to trigger periodic
//  status strobes when the player is parked
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::park_strobe_tick)
{
	m_status_strobe = false;
	m_deassert_status_strobe_timer->adjust(attotime::from_usec(26));
	m_assert_command_strobe_timer->adjust(attotime::from_usec(54));
}


//-------------------------------------------------
//  assert_status_strobe - assert the status
//  strobe signal to a host application at the
//  appropriate time, and prepare for command
//  strobe
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::assert_status_strobe)
{
	m_status_strobe = false;
	m_deassert_status_strobe_timer->adjust(attotime::from_usec(26));
	m_assert_command_strobe_timer->adjust(attotime::from_usec(54));
}


//-------------------------------------------------
//  deassert_status_strobe
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::deassert_status_strobe)
{
	m_status_strobe = true;
}


//-------------------------------------------------
//  assert_command_strobe
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::assert_command_strobe)
{
	m_command_strobe = false;
	m_deassert_command_strobe_timer->adjust(attotime::from_usec(25));
}


//-------------------------------------------------
//  deassert_command_strobe
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pioneer_ldv1000hle_device::deassert_command_strobe)
{
	m_command_strobe = true;
}


//-------------------------------------------------
//  process_command_buffer - process a command
//  line sent from the host
//-------------------------------------------------

void pioneer_ldv1000hle_device::process_command_buffer()
{
	size_t cmd_index = 0;
	while (cmd_index < m_cmd_length)
	{
		process_command(cmd_index);
		cmd_index++;
	}

	m_cmd_length = 0;
}


//-------------------------------------------------
//  is_command_byte - returns whether or not a
//  given byte is a recognized command
//-------------------------------------------------

bool pioneer_ldv1000hle_device::is_command_byte(const u8 data)
{
	switch (data)
	{
		case CMD_CLEAR:
		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
		case CMD_STORE:
		case CMD_RECALL:
		case CMD_DISPLAY:
		case CMD_AUDIO1:
		case CMD_AUDIO2:
		case CMD_PLAY:
		case CMD_STOP:
		case CMD_AUTOSTOP:
		case CMD_SEARCH:
		case CMD_SCAN_FWD:
		case CMD_SCAN_REV:
		case CMD_STEP_FWD:
		case CMD_STEP_REV:
		case CMD_REJECT:
		case CMD_NO_ENTRY:
		case CMD_LOAD:
		case CMD_DISPLAY_DISABLE:
		case CMD_DISPLAY_ENABLE:
		case CMD_GET_FRAME_NUM:
		case CMD_GET_2ND_DISPLAY:
		case CMD_GET_1ST_DISPLAY:
		case CMD_TRANSFER_MEMORY:
		case CMD_FWD_X0:
		case CMD_FWD_X1_4:
		case CMD_FWD_X1_2:
		case CMD_FWD_X1:
		case CMD_FWD_X2:
		case CMD_FWD_X3:
		case CMD_FWD_X4:
		case CMD_FWD_X5:
		case CMD_SKIP_FWD_10:
		case CMD_SKIP_FWD_20:
		case CMD_SKIP_FWD_30:
		case CMD_SKIP_FWD_40:
		case CMD_SKIP_FWD_50:
		case CMD_SKIP_FWD_60:
		case CMD_SKIP_FWD_70:
		case CMD_SKIP_FWD_80:
		case CMD_SKIP_FWD_90:
		case CMD_SKIP_FWD_100:
			return true;
	}
	return false;
}


//-------------------------------------------------
//  is_command_number - returns whether or not a
//  given byte is a number-command
//-------------------------------------------------

bool pioneer_ldv1000hle_device::is_command_number(const u8 data)
{
	switch (data)
	{
		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			return true;
	}
	return false;
}


//-------------------------------------------------
//  process_command - processes a single command
//  from the command buffer
//-------------------------------------------------

void pioneer_ldv1000hle_device::process_command(size_t cmd_index)
{
	const u8 command = m_cmd_buffer[cmd_index];
	LOGMASKED(LOG_COMMAND_BYTES, "process_command: Command byte %02x with status %02x\n", command, m_status);
	if (BIT(m_status, 7))
	{
		m_status &= ~STATUS_READY;

		switch (command)
		{
			case CMD_CLEAR:
				LOGMASKED(LOG_COMMANDS, "process_command: Clear\n");
				break;

			case CMD_0:
			case CMD_1:
			case CMD_2:
			case CMD_3:
			case CMD_4:
			case CMD_5:
			case CMD_6:
			case CMD_7:
			case CMD_8:
			case CMD_9:
				m_cmd_number[m_cmd_number_length] = cmd_to_number(command);
				LOGMASKED(LOG_COMMANDS, "process_command: %d (number length now %d)\n", m_cmd_number[m_cmd_number_length], m_cmd_number_length);
				m_cmd_number_length++;
				break;

			case CMD_STORE:
				if (m_cmd_number_length == 0)
				{
					LOGMASKED(LOG_COMMANDS, "process_command: Store (no value specified, storing current frame in current register)\n");
					m_user_ram[m_curr_register] = literal_to_bcd(m_curr_frame);
				}
				else
				{
					m_user_ram[m_curr_register] = cmd_number_to_bcd();
					LOGMASKED(LOG_COMMANDS, "process_command: Store %05d\n", m_user_ram[m_curr_register]);
				}
				break;

			case CMD_RECALL:
				LOGMASKED(LOG_COMMANDS, "process_command: Recall (not yet implemented)\n");
				break;

			case CMD_DISPLAY:
				LOGMASKED(LOG_COMMANDS, "process_command: Display %d (not yet implemented)\n", cmd_number_to_bcd());
				break;

			case CMD_AUDIO1:
				if (m_cmd_number_length == 0)
				{
					m_audio_enable[0] = !m_audio_enable[0];
					LOGMASKED(LOG_COMMANDS, "process_command: Audio1, toggling audio enable, channels now %d/%d\n", m_audio_enable[0], m_audio_enable[1]);
				}
				else
				{
					const u32 cmd_number = cmd_number_to_bcd();
					m_audio_enable[0] = BIT(cmd_number, 0);
					LOGMASKED(LOG_COMMANDS, "process_command: Audio1, setting audio enable, channels now %d/%d\n", m_audio_enable[0], m_audio_enable[1]);
				}
				break;

			case CMD_AUDIO2:
				if (m_cmd_number_length == 0)
				{
					m_audio_enable[1] = !m_audio_enable[1];
					LOGMASKED(LOG_COMMANDS, "process_command: Audio2, toggling audio enable, channels now %d/%d\n", m_audio_enable[0], m_audio_enable[1]);
				}
				else
				{
					const u32 cmd_number = cmd_number_to_bcd();
					m_audio_enable[1] = BIT(cmd_number, 0);
					LOGMASKED(LOG_COMMANDS, "process_command: Audio2, setting audio enable, channels now %d/%d\n", m_audio_enable[0], m_audio_enable[1]);
				}
				break;

			case CMD_PLAY:
				LOGMASKED(LOG_COMMANDS, "process_command: Play\n");
				cmd_play();
				break;

			case CMD_STOP:
				cmd_stop();
				break;

			case CMD_AUTOSTOP:
			{
				const u32 value = bcd_to_literal(cmd_number_to_bcd());
				LOGMASKED(LOG_COMMANDS, "process_command: Autostop (at frame %d)\n", m_stop_frame);
				if (value < m_curr_frame)
				{
					// Autostop with a frame number less than the current one will cause a search.
					LOGMASKED(LOG_COMMANDS, "process_command: Requested autostop frame is less than current frame (%d), performing a search instead.\n", m_curr_frame);
					cmd_search(value);
					break;
				}
				m_status = STATUS_AUTOSTOP;
				break;
			}

			case CMD_SEARCH:
			{
				const u32 value = bcd_to_literal(cmd_number_to_bcd());
				LOGMASKED(LOG_COMMANDS, "process_command: Search (to frame %d)\n", value);
				cmd_search(value);
				break;
			}

			case CMD_SCAN_FWD:
			case CMD_SCAN_REV:
				LOGMASKED(LOG_COMMANDS, "process_command: Scan %s\n", command == CMD_SCAN_FWD ? "Forward" : "Reverse");
				set_mode(command == CMD_SCAN_FWD ? MODE_SCAN_FORWARD : MODE_SCAN_REVERSE);
				m_status = STATUS_SCAN;
				m_scan_speed = 4000; // "The two SCAN commands move the player's optical head at the rate of approximately 2000 frames per second in the direction specified"
				m_scan_speed_accum = 0;
				m_stop_frame = ~0U;
				m_search_frame = ~0U;
				break;

			case CMD_STEP_FWD:
				LOGMASKED(LOG_COMMANDS, "process_command: Step Forward\n");
				cmd_step(1);
				break;

			case CMD_STEP_REV:
				LOGMASKED(LOG_COMMANDS, "process_command: Step Reverse\n");
				cmd_step(-1);
				break;

			case CMD_REJECT:
				LOGMASKED(LOG_COMMANDS, "process_command: Reject\n");
				set_mode(MODE_PARK);
				m_status = STATUS_PARK;
				m_stop_frame = ~0U;
				m_search_frame = ~0U;
				break;

			case CMD_NO_ENTRY:
				LOGMASKED(LOG_COMMANDS, "process_command: No Entry\n");
				m_status |= STATUS_READY;
				break;

			case CMD_LOAD:
				LOGMASKED(LOG_COMMANDS, "process_command: Load (not yet implemented)\n");
				break;

			case CMD_DISPLAY_DISABLE:
				LOGMASKED(LOG_COMMANDS, "process_command: Display Disable (not yet implemented)\n");
				break;

			case CMD_DISPLAY_ENABLE:
				LOGMASKED(LOG_COMMANDS, "process_command: Display Enable (not yet implemented)\n");
				break;

			case CMD_GET_FRAME_NUM:
				LOGMASKED(LOG_COMMANDS, "process_command: Get Frame Number (not yet implemented)\n");
				break;

			case CMD_GET_2ND_DISPLAY:
				LOGMASKED(LOG_COMMANDS, "process_command: Get 2nd Display (not yet implemented)\n");
				break;

			case CMD_GET_1ST_DISPLAY:
				LOGMASKED(LOG_COMMANDS, "process_command: Get 1st Display (not yet implemented)\n");
				break;

			case CMD_TRANSFER_MEMORY:
				LOGMASKED(LOG_COMMANDS, "process_command: Transfer Memory (not yet implemented)\n");
				break;

			case CMD_FWD_X0:
				cmd_play_forward(0.0);
				break;

			case CMD_FWD_X1_4:
				cmd_play_forward(0.25);
				break;

			case CMD_FWD_X1_2:
				cmd_play_forward(0.5);
				break;

			case CMD_FWD_X1:
				cmd_play_forward(1.0);
				break;

			case CMD_FWD_X2:
				cmd_play_forward(2.0);
				break;

			case CMD_FWD_X3:
				cmd_play_forward(3.0);
				break;

			case CMD_FWD_X4:
				cmd_play_forward(4.0);
				break;

			case CMD_FWD_X5:
				cmd_play_forward(5.0);
				break;

			case CMD_SKIP_FWD_10:
				cmd_skip_forward(10);
				break;

			case CMD_SKIP_FWD_20:
				cmd_skip_forward(20);
				break;

			case CMD_SKIP_FWD_30:
				cmd_skip_forward(30);
				break;

			case CMD_SKIP_FWD_40:
				cmd_skip_forward(40);
				break;

			case CMD_SKIP_FWD_50:
				cmd_skip_forward(50);
				break;

			case CMD_SKIP_FWD_60:
				cmd_skip_forward(60);
				break;

			case CMD_SKIP_FWD_70:
				cmd_skip_forward(70);
				break;

			case CMD_SKIP_FWD_80:
				cmd_skip_forward(80);
				break;

			case CMD_SKIP_FWD_90:
				cmd_skip_forward(90);
				break;

			case CMD_SKIP_FWD_100:
				cmd_skip_forward(100);
				break;
		}

		if (!is_command_number(command))
		{
			m_cmd_number_length = 0;
		}

		LOGMASKED(LOG_COMMAND_BYTES, "process_command: Status is now %02x\n", m_status);
	}
	else if (command == CMD_NO_ENTRY)
	{
		m_status |= STATUS_READY;
		LOGMASKED(LOG_COMMAND_BYTES, "process_command: Received no-entry command while busy, setting ready, status is now %02x\n", m_status);
	}
	else
	{
		m_status &= ~STATUS_READY;
		LOGMASKED(LOG_COMMAND_BYTES, "process_command: Received command %02x while busy, clearing ready, status is now %02x\n", command, m_status);
	}
}


//-------------------------------------------------
//  bcd_to_literal - converts a BCD value used in
//  commands a direct numeric value
//-------------------------------------------------

u32 pioneer_ldv1000hle_device::bcd_to_literal(u32 bcd)
{
	u32 value = 0;
	u32 shift = 28;
	u32 multiplier = 10000000;
	for (u32 i = 0; i < 8; i++)
	{
		u32 digit = (bcd >> shift) & 0xf;
		bcd &= ~(0xf << shift);

		value += digit * multiplier;

		multiplier /= 10;
		shift -= 4;
	}
	return value;
}


//-------------------------------------------------
//  literal_to_bcd - converts a literal value
//  into a BCD representation for commands
//-------------------------------------------------

u32 pioneer_ldv1000hle_device::literal_to_bcd(u32 value)
{
	LOGMASKED(LOG_COMMANDS, "literal_to_bcd: Converting %08x to BCD\n", value);
	u32 bcd_value = 0;
	u32 shift = 28;
	u32 multiplier = 10000000;
	for (u32 i = 0; i < 8; i++)
	{
		u32 digit = value / multiplier;
		bcd_value |= digit << shift;

		value -= digit * multiplier;
		multiplier /= 10;
		shift -= 4;
	}
	LOGMASKED(LOG_COMMANDS, "literal_to_bcd: Result: %08x\n", bcd_value);
	return bcd_value;
}


//-------------------------------------------------
//  literal_to_bcd - converts a literal value
//  into a BCD representation for commands
//-------------------------------------------------

u32 pioneer_ldv1000hle_device::cmd_number_to_bcd()
{
	if (m_cmd_number_length == 0)
	{
		LOGMASKED(LOG_COMMANDS, "cmd_number_to_bcd: No command number length, returning 0\n");
		return 0;
	}
	u32 bcd_value = 0;
	size_t shift = 0;
	for (int i = (int)m_cmd_number_length - 1; i >= 0; i--)
	{
		bcd_value |= m_cmd_number[i] << shift;
		shift += 4;
	}
	LOGMASKED(LOG_COMMANDS, "cmd_number_to_bcd: %05x\n", bcd_value);
	return bcd_value;
}


//-------------------------------------------------
//  set_mode - set the current high-level player
//  mode, updating any enables as necessary
//-------------------------------------------------

u8 pioneer_ldv1000hle_device::cmd_to_number(const u8 cmd)
{
	switch (cmd)
	{
		case CMD_0: return 0;
		case CMD_1: return 1;
		case CMD_2: return 2;
		case CMD_3: return 3;
		case CMD_4: return 4;
		case CMD_5: return 5;
		case CMD_6: return 6;
		case CMD_7: return 7;
		case CMD_8: return 8;
		case CMD_9: return 9;
	}
	return 0;
}


//-------------------------------------------------
//  set_mode - set the current high-level player
//  mode, updating any enables as necessary
//-------------------------------------------------

void pioneer_ldv1000hle_device::set_mode(const u8 mode)
{
	if (m_mode == mode)
	{
		return;
	}

	m_mode = mode;
	if (mode != MODE_PARK)
	{
		m_park_strobe_timer->adjust(attotime::never);
	}
	else
	{
		m_park_strobe_timer->adjust(attotime::from_msec(21), 0, attotime::from_msec(21));
	}
}


//-------------------------------------------------
//  set_playing - general-purpose function for
//  setting a forward play mode
//-------------------------------------------------

void pioneer_ldv1000hle_device::set_playing(const u8 new_status, const double fields_per_vsync)
{
	set_mode(MODE_PLAY);
	m_status = new_status;
	m_play_speed = fields_per_vsync;
	m_search_frame = ~0U;
	m_stop_frame = ~0U;
}


//-------------------------------------------------
//  set_playing - general-purpose function for
//  setting a forward play mode
//-------------------------------------------------

void pioneer_ldv1000hle_device::set_stopped(const u8 new_status)
{
	set_mode(MODE_STOP);
	m_status = new_status;
}


//-------------------------------------------------
//  cmd_play - play forward at 1x speed
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_play()
{
	LOGMASKED(LOG_COMMANDS, "process_command: Play\n");
	set_slider_speed(0);
	set_playing(STATUS_PLAY, 1.0);
}


//-------------------------------------------------
//  cmd_stop - stop/pause/freeze-frame
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_stop()
{
	if (m_cmd_number_length > 0)
	{
		const u32 command_value = cmd_number_to_bcd();
		m_stop_timer->adjust(attotime::from_msec(100 * command_value));
		m_pre_stop_status = m_status;
		LOGMASKED(LOG_COMMANDS, "process_command: Stop (for %d.%d seconds)\n", command_value / 10, command_value % 10);
	}
	else
	{
		LOGMASKED(LOG_COMMANDS, "process_command: Stop\n");
	}
	set_stopped(STATUS_STOP);
}


//-------------------------------------------------
//  cmd_step - step one frame forward or backward
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_step(const int direction)
{
	advance_slider(direction);
	set_stopped(STATUS_STOP);
	m_stop_frame = ~0U;
	m_search_frame = ~0U;
}


//-------------------------------------------------
//  cmd_play_forward - play forward at a specific
//  multiplier
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_play_forward(const double fields_per_vsync)
{
	LOGMASKED(LOG_COMMANDS, "process_command: Forward x%f\n", fields_per_vsync);
	set_slider_speed(fields_per_vsync);
	set_playing(STATUS_FORWARD, fields_per_vsync);
}


//-------------------------------------------------
//  cmd_search - begin a search operation to a
//  specific frame
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_search(const u32 frame)
{
	set_mode(MODE_SEARCH);
	m_status = STATUS_SEARCH;
	m_search_frame = frame;
	m_stop_frame = ~0U;
}


//-------------------------------------------------
//  cmd_skip_forward - skip forward a specific
//  number of frames while in forward-play mode
//-------------------------------------------------

void pioneer_ldv1000hle_device::cmd_skip_forward(const s32 amount)
{
	LOGMASKED(LOG_COMMANDS, "Command: Skip Forward %d\n", amount);
	set_mode(MODE_SEARCH);
	m_search_frame = ~0U;
	m_stop_frame = m_curr_frame + amount;
	m_scan_speed = amount;
}


//-------------------------------------------------
//  update_video_enable - set video enable state
//  on the base device based on our video switch
//-------------------------------------------------

void pioneer_ldv1000hle_device::update_video_enable()
{
	LOGMASKED(LOG_SQUELCHES, "%s: Updating video enable (mode %d), video %s enabled\n", machine().describe_context(), m_mode, m_mode != MODE_PARK ? "is" : "is not");
	video_enable(m_mode == MODE_PLAY || m_mode == MODE_STOP);
}


//-------------------------------------------------
//  update_audio_enable - set audio enable state
//  depending on our current mode or channel mute
//-------------------------------------------------

void pioneer_ldv1000hle_device::update_audio_enable()
{
	if (m_mode == MODE_PLAY && m_play_speed == 1.0)
	{
		LOGMASKED(LOG_SQUELCHES, "%s: Updating audio enable (playing at 1x, channels %d/%d)\n", machine().describe_context(), m_audio_enable[0], m_audio_enable[1]);
		set_audio_squelch(!m_audio_enable[0], !m_audio_enable[1]);
	}
	else
	{
		LOGMASKED(LOG_SQUELCHES, "%s: Updating audio enable (muted, mode %d, speed %f)\n", machine().describe_context(), m_mode, m_play_speed);
		set_audio_squelch(true, true);
	}
}
