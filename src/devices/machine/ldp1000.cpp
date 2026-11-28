// license:BSD-3-Clause
// copyright-holders:bugjosh
/***************************************************************************

    Sony LDP-1000A laserdisc player HLE

    Emulates the player's RS-232C interface as described in Sony's
    "LDP-1000 Interface Manual" and "LDP-1000A External Computer Interface
    Manual" (1984), following the LDP-1000A where they differ: the LDP-1000
    sends NOT TARGET where the LDP-1000A sends NO FRAME, and its first PROM
    revision (1.5) sends no ACK or NAK.

    The later LDP-1450 and LDP-1550 (see ldp1450hle.cpp) use the same command
    codes, but their status replies, error handling and power-on behaviour
    differ.

    Every command byte is answered with ACK (0x0a), or with ERROR (0x02) if
    the player can't accept it in its current mode; bytes outside 0x30-0x69
    are answered with NAK (0x0b).  After an error only CE and CL (and the
    commands accepted in any mode) are taken until one of them clears it.
    SEARCH later ends with COMPLETION (0x01), NOT TARGET (0x05) or NO FRAME
    (0x06), and REPEAT with COMPLETION or NOT TARGET.  ADDR INQ and STATUS
    INQ answer with five bytes and no ACK.  With the motor off, everything
    but MOTOR ON, STATUS INQ and DISC ID INQ is answered with NAK, and DISC
    ID INQ always is (the disc ID isn't emulated).  The player doesn't
    communicate while it spins up after power on or MOTOR ON, or while it
    stops after MOTOR OFF (about 5 s), and sends an ACK when it's done.

    TODO:
    - Programs (PGM, RUN, MEMORY, M-SEARCH, ...), segments, chapter mode
      and audio code dumps
    - Index display (play mode, frame and chapter number): INDEX ON/OFF are
      accepted and ignored; the player's character generator isn't dumped
      and the position is set on the rear panel
    - Real search times (the manuals only give the time to ACK) and
      spin-up time (given as "12 to 15 sec.")
    - The host's reset doesn't reach the player on real hardware, but a
      MAME soft reset restarts it

***************************************************************************/

#include "emu.h"
#include "ldp1000.h"

#define LOG_COMMAND (1U << 1)
#define LOG_REPLY   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


ALLOW_SAVE_TYPE(laserdisc_device::player_state);


namespace {

// time from power on or MOTOR ON until the player is ready ("12 to 15 sec.")
constexpr attotime SPINUP_TIME = attotime::from_seconds(12);

// time to return the sled and stop the spindle after MOTOR OFF ("about 5 sec.")
constexpr attotime MOTOR_OFF_TIME = attotime::from_seconds(5);

// a search that hasn't found its frame after this many fields (5 s) gives up; the manuals give no
// limit, so this is an assumption
constexpr u32 SEARCH_TIMEOUT_FIELDS = 300;

// SCAN speed: "about 100 times of normal speed" (LDP-1000A Operating Instructions)
constexpr s32 SCAN_SPEED = 100;

// tracks to jump per field to cross the lead-in after spinning up (more than
// the 900 tracks of the virtual lead-in)
constexpr s32 LEAD_IN_JUMP = 1000;

// time to return ACK in microseconds, from the LDP-1000A execution time table
// (2000 for the commands the table doesn't list), and whether the command is
// accepted in any mode, for commands 0x30-0x69
struct command_info
{
	u16 ack_time;
	bool any_mode;
};

constexpr command_info COMMANDS[] =
{
	// 0-9
	{ 1200, false }, { 1200, false }, { 1200, false }, { 1200, false }, { 1200, false },
	{ 1200, false }, { 1200, false }, { 1200, false }, { 1200, false }, { 1200, false },
	// F-PLAY, F-FAST, F-SLOW, F-STEP, F-SCAN, STOP
	{ 2000, false }, { 2000, false }, { 2000, false }, { 2000, false }, { 2000, false }, { 2000, false },
	// ENTER, C.E., MENU, SEARCH, REPEAT, SEGMENT
	{ 2600, false }, { 2000, false }, { 20'000, false }, { 2600, false }, { 2600, false }, { 2500, false },
	// CH-1 ON, CH-1 OFF, CH-2 ON, CH-2 OFF
	{ 430, true }, { 430, true }, { 430, true }, { 430, true },
	// R-PLAY, R-FAST, R-SLOW, R-STEP, R-SCAN, STILL
	{ 2000, false }, { 2000, false }, { 2000, false }, { 2000, false }, { 2000, false }, { 430, true },
	// INDEX ON, INDEX OFF, DUMP IN, DUMP OUT, SEG # MODE, FRAME # MODE, C.L.
	{ 430, true }, { 430, true }, { 2000, true }, { 2000, true }, { 430, true }, { 430, true }, { 4300, false },
	// PGM, RUN, END, MEMORY, M-SEARCH, SKIP, INT, REVIEW, MODE
	{ 2200, false }, { 7900, false }, { 5600, false }, { 760, false }, { 1000, false },
	{ 2500, false }, { 2500, false }, { 1500, false }, { 1700, false },
	// ADDR INQ, CONTINUE, MOTOR ON, MOTOR OFF, CH-1, CH-2, INDEX, STATUS INQ, DISC ID INQ, CHAPTER # MODE
	{ 2000, true }, { 2000, true }, { 2000, true }, { 2000, true }, { 470, true },
	{ 470, true }, { 580, true }, { 2000, true }, { 2000, true }, { 2000, true }
};

static_assert(std::size(COMMANDS) == 0x6a - 0x30);

// STATUS INQ byte 5 bits for PLAY, FAST, SLOW, STEP and SCAN ("D2 and D3 become 1 when in the SLOW mode")
constexpr u8 SPEED_STATUS[] = { 0x01, 0x02, 0x0c, 0x08, 0x10 };

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SONY_LDP1000, sony_ldp1000_device, "ldp1000", "Sony LDP-1000A")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sony_ldp1000_device - constructor
//-------------------------------------------------

sony_ldp1000_device::sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: laserdisc_device(mconfig, SONY_LDP1000, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_serial_tx(*this)
	, m_reply_timer(nullptr)
	, m_motor_timer(nullptr)
	, m_baud(1200)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sony_ldp1000_device::device_start()
{
	laserdisc_device::device_start();

	m_reply_timer = timer_alloc(FUNC(sony_ldp1000_device::send_reply), this);
	m_motor_timer = timer_alloc(FUNC(sony_ldp1000_device::motor_stopped), this);

	save_item(NAME(m_reply));
	save_item(NAME(m_reply_read));
	save_item(NAME(m_reply_count));
	save_item(NAME(m_input));
	save_item(NAME(m_number));
	save_item(NAME(m_digits));
	save_item(NAME(m_error));
	save_item(NAME(m_operation));
	save_item(NAME(m_search_fields));
	save_item(NAME(m_search_outside));
	save_item(NAME(m_repeat_start));
	save_item(NAME(m_repeat_end));
	save_item(NAME(m_repeat_count));
	save_item(NAME(m_repeat_speed));
	save_item(NAME(m_step_rate));
	save_item(NAME(m_speed));
	save_item(NAME(m_reverse));
	save_item(NAME(m_stopped));
	save_item(NAME(m_motor_off));
	save_item(NAME(m_ch1));
	save_item(NAME(m_ch2));
	save_item(NAME(m_last_frame));
	save_item(NAME(m_still_saved_state));
	save_item(NAME(m_still_saved_param));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sony_ldp1000_device::device_reset()
{
	laserdisc_device::device_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(int(m_baud));
	m_serial_tx(1);

	m_reply_timer->adjust(attotime::never);
	m_motor_timer->adjust(attotime::never);
	std::fill(std::begin(m_reply), std::end(m_reply), 0);
	m_reply_read = 0;
	m_reply_count = 0;

	m_input = INPUT_NONE;
	m_number = 0;
	m_digits = 0;
	m_error = false;

	m_operation = OP_NONE;
	m_search_fields = 0;
	m_search_outside = false;
	m_repeat_start = 0;
	m_repeat_end = 0;
	m_repeat_count = 1;
	m_repeat_speed = SPEED_PLAY;
	m_step_rate = 7;

	m_speed = SPEED_PLAY;
	m_reverse = false;
	m_stopped = false;
	m_motor_off = false;
	m_ch1 = true;
	m_ch2 = true;
	m_last_frame = 0;
	m_still_saved_state = LDSTATE_NONE;
	m_still_saved_param = 0;

	begin_init();
}


//**************************************************************************
//  PLAYER STATE
//**************************************************************************

//-------------------------------------------------
//  begin_init - spin up and stop on frame 1, as
//  after power on or MOTOR ON
//-------------------------------------------------

void sony_ldp1000_device::begin_init()
{
	m_operation = OP_INIT;
	m_search_fields = 0;
	m_stopped = false;
	m_player_state.m_state = LDSTATE_SPINUP;
	m_player_state.m_substate = 0;
	m_player_state.m_param = 0;
	m_player_state.m_endtime = machine().time() + SPINUP_TIME;
	set_video_squelch(true);
	update_audio_squelch();
}


//-------------------------------------------------
//  begin_search - search for a frame and stop
//  there in STILL mode
//-------------------------------------------------

void sony_ldp1000_device::begin_search(s32 frame)
{
	m_search_fields = 0;
	m_search_outside = false;
	m_stopped = false;
	m_still_saved_state = LDSTATE_NONE;
	m_player_state.m_state = LDSTATE_SEEKING;
	m_player_state.m_substate = 0;
	m_player_state.m_param = frame;
	// the manuals don't say what the player shows while it searches, but the access motor moves the sled at x100
	// or x360, so mute the video until the frame is found
	set_video_squelch(true);
	update_audio_squelch();
}


//-------------------------------------------------
//  begin_repeat - play from the start frame to
//  the end frame, forwards or in reverse
//-------------------------------------------------

void sony_ldp1000_device::begin_repeat()
{
	m_operation = OP_REPEAT;
	play(m_repeat_speed, m_repeat_end < m_repeat_start, m_step_rate);
}


//-------------------------------------------------
//  play - start playback at one of the F/R speeds
//-------------------------------------------------

void sony_ldp1000_device::play(u8 speed, bool reverse, u32 step_rate)
{
	m_speed = speed;
	m_reverse = reverse;
	m_stopped = false;
	m_still_saved_state = LDSTATE_NONE;
	m_player_state.m_substate = 0;
	switch (speed)
	{
		case SPEED_PLAY:
			// PLAY is the x1 mode
			m_player_state.m_state = reverse ? LDSTATE_PLAYING_SLOW_REVERSE : LDSTATE_PLAYING;
			m_player_state.m_param = 0;
			break;

		case SPEED_FAST:
			// FAST is the x3 mode
			m_player_state.m_state = reverse ? LDSTATE_PLAYING_FAST_REVERSE : LDSTATE_PLAYING_FAST_FORWARD;
			m_player_state.m_param = GENERIC_FAST_SPEED;
			break;

		case SPEED_SLOW:
			// SLOW is the x1/5 mode
			m_player_state.m_state = reverse ? LDSTATE_PLAYING_SLOW_REVERSE : LDSTATE_PLAYING_SLOW_FORWARD;
			m_player_state.m_param = GENERIC_SLOW_SPEED - 1;
			break;

		case SPEED_STEP:
			// STEP is x1/7 unless another step rate is entered
			m_player_state.m_state = reverse ? LDSTATE_PLAYING_SLOW_REVERSE : LDSTATE_PLAYING_SLOW_FORWARD;
			m_player_state.m_param = step_rate - 1;
			break;

		case SPEED_SCAN:
			m_player_state.m_state = reverse ? LDSTATE_PLAYING_FAST_REVERSE : LDSTATE_PLAYING_FAST_FORWARD;
			m_player_state.m_param = SCAN_SPEED;
			break;
	}
	set_video_squelch(false);
	update_audio_squelch();
}


//-------------------------------------------------
//  still - stop on the current frame; STOP also
//  mutes the video
//-------------------------------------------------

void sony_ldp1000_device::still(bool stop)
{
	m_stopped = stop;
	if (m_player_state.m_state != LDSTATE_PAUSED)
	{
		m_player_state.m_state = LDSTATE_PAUSING;
		m_player_state.m_substate = 0;
		m_player_state.m_param = 0;
	}
	set_video_squelch(stop);
	update_audio_squelch();
}


//-------------------------------------------------
//  finish_operation - end a SEARCH or REPEAT with
//  a return code
//-------------------------------------------------

void sony_ldp1000_device::finish_operation(u8 code)
{
	LOGMASKED(LOG_COMMAND, "operation finished with %02x at frame %d\n", code, m_last_frame);
	m_operation = OP_NONE;
	queue_reply(code, attotime::zero);
}


//-------------------------------------------------
//  update_audio_squelch - audio only plays in the
//  x1 forward mode, on the enabled channels
//-------------------------------------------------

void sony_ldp1000_device::update_audio_squelch()
{
	const bool searching = m_operation == OP_INIT || m_operation == OP_SEARCH || m_operation == OP_REPEAT_RETURN;
	const bool audible = !m_motor_off && !m_stopped && !searching && m_player_state.m_state == LDSTATE_PLAYING;
	set_audio_squelch(!audible || !m_ch1, !audible || !m_ch2);
}


//-------------------------------------------------
//  state_changed - act on the generic player
//  reaching a new state
//-------------------------------------------------

void sony_ldp1000_device::state_changed(player_state oldstate)
{
	if (oldstate == LDSTATE_SPINUP)
	{
		// up to speed: stop on frame 1 (no frame number has been read yet; see player_update)
		m_last_frame = -1;
		begin_search(1);
		return;
	}

	if (oldstate == LDSTATE_SEEKING && m_player_state.m_state == LDSTATE_PAUSED)
	{
		set_video_squelch(false);
		switch (m_operation)
		{
			case OP_INIT:
				// ready to communicate
				m_operation = OP_NONE;
				queue_reply(RET_ACK, attotime::zero);
				break;

			case OP_SEARCH:
				finish_operation(RET_COMPLETION);
				break;

			case OP_REPEAT_RETURN:
				// back at the start frame: play the next repetition
				begin_repeat();
				return;
		}
	}
	update_audio_squelch();
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

s32 sony_ldp1000_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// ADDR INQ returns the last frame number that could be read
	const int frame = frame_from_metadata(vbi);
	if (frame != FRAME_NOT_PRESENT && frame != FRAME_LEAD_IN && frame != FRAME_LEAD_OUT)
		m_last_frame = frame;

	// a repetition ends on its end frame, or as soon as it has gone past it
	if (m_operation == OP_REPEAT && frame != FRAME_NOT_PRESENT && is_start_of_frame(vbi))
	{
		const bool reached = frame == m_repeat_end;
		const bool passed = m_reverse ? (frame < m_repeat_end) : (frame > m_repeat_end);
		if (reached || passed)
		{
			if (m_repeat_count != 1)
			{
				if (m_repeat_count > 1)
					m_repeat_count--;
				m_operation = OP_REPEAT_RETURN;
				begin_search(m_repeat_start);
			}
			else
			{
				m_player_state.m_state = LDSTATE_PAUSED;
				m_player_state.m_substate = 0;
				m_player_state.m_param = fieldnum;
				finish_operation(reached ? RET_COMPLETION : RET_NOT_TARGET);
				update_audio_squelch();
			}
			return 0;
		}
	}

	// a search that can't find its frame gives up; the LDP-1000A tells a
	// frame outside the disc (it ended up in the lead-in or lead-out) apart,
	// except for REPEAT
	if (m_player_state.m_state == LDSTATE_SEEKING)
	{
		if (frame == FRAME_LEAD_IN || frame == FRAME_LEAD_OUT)
			m_search_outside = true;
		if (++m_search_fields > SEARCH_TIMEOUT_FIELDS)
		{
			m_player_state.m_state = LDSTATE_PAUSING;
			m_player_state.m_substate = 0;
			m_player_state.m_param = 0;
			set_video_squelch(false);
			if (m_operation == OP_INIT)
			{
				m_operation = OP_NONE;
				queue_reply(RET_ACK, attotime::zero);
			}
			else
			{
				finish_operation((m_search_outside && m_operation == OP_SEARCH) ? RET_NO_FRAME : RET_NOT_TARGET);
			}
			update_audio_squelch();
		}
	}

	// spinning up leaves the pickup at the start of the lead-in, where the generic
	// player would play all the way through it to get to frame 1, so jump through
	// it until a frame number turns up
	if (m_operation == OP_INIT && m_player_state.m_state == LDSTATE_SEEKING && m_last_frame < 0 && frame == FRAME_LEAD_IN)
	{
		m_player_state.m_substate = 0;
		return LEAD_IN_JUMP;
	}

	// let the generic player do the rest; it updates the substate in place,
	// so only take its new state when the state actually changes
	player_state_info newstate;
	const s32 advanceby = generic_update(vbi, fieldnum, curtime, newstate);
	if (newstate.m_state != m_player_state.m_state)
	{
		const player_state oldstate = m_player_state.m_state;
		m_player_state = newstate;
		m_player_state.m_substate = 0;
		state_changed(oldstate);
	}
	return advanceby;
}


//**************************************************************************
//  COMMANDS
//**************************************************************************

//-------------------------------------------------
//  process_command - handle a byte received from
//  the external computer
//-------------------------------------------------

void sony_ldp1000_device::process_command(u8 data)
{
	LOGMASKED(LOG_COMMAND, "command %02x\n", data);

	// no communication is possible while the motor spins up or stops
	if (m_operation == OP_INIT || m_operation == OP_MOTOR_OFF)
	{
		LOGMASKED(LOG_COMMAND, "ignored, player not ready\n");
		return;
	}

	if (data < 0x30 || data > CMD_CHAPTER_MODE)
	{
		queue_reply(RET_NAK, attotime::from_usec(2000));
		return;
	}

	const command_info &info = COMMANDS[data - 0x30];
	const attotime ack_time = attotime::from_usec(info.ack_time);

	// with the motor off, only MOTOR ON, STATUS INQ and DISC ID INQ are accepted
	if (m_motor_off && data != CMD_STATUS_INQ && data != CMD_DISC_ID_INQ && data != CMD_MOTOR_ON)
	{
		queue_reply(RET_NAK, ack_time);
		return;
	}

	if (m_error && data != CMD_CE && data != CMD_CL && !info.any_mode)
	{
		queue_reply(RET_ERROR, ack_time);
		return;
	}

	// the step rate is only waited for until another command arrives (STATUS INQ reports the wait)
	if (m_input == INPUT_STEP && data > 0x39 && data != CMD_ENTER && data != CMD_ADDR_INQ && data != CMD_STATUS_INQ && data != CMD_DISC_ID_INQ)
		m_input = INPUT_NONE;

	const bool native = m_input == INPUT_NONE && m_operation == OP_NONE;
	const bool repeat_input = m_input == INPUT_REPEAT_FRAME || m_input == INPUT_REPEAT_COUNT || m_input == INPUT_REPEAT_STEP;
	u8 reply = RET_ACK;

	switch (data)
	{
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
		case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
			if (m_input == INPUT_NONE)
			{
				reply = RET_ERROR;
			}
			else
			{
				m_number = (m_number * 10 + (data - 0x30)) % 100'000;
				m_digits++;
			}
			break;

		case CMD_ENTER:
			reply = enter_command();
			break;

		case CMD_CE:
			// clears an error, or abandons the command being entered (the LDP-1000A manual's example
			// sends SEARCH again after CE)
			if (m_error)
				m_error = false;
			else
				m_input = INPUT_NONE;
			m_number = 0;
			m_digits = 0;
			break;

		case CMD_CL:
			// cancels everything and returns to the initial state
			m_operation = OP_NONE;
			m_input = INPUT_NONE;
			m_number = 0;
			m_digits = 0;
			m_error = false;
			m_still_saved_state = LDSTATE_NONE;
			still(false);
			break;

		case CMD_MENU:
			if (native || m_input != INPUT_NONE)
				m_input = INPUT_NONE;
			else
				reply = RET_ERROR;
			break;

		case CMD_SEARCH:
			if (!native)
			{
				reply = RET_ERROR;
				break;
			}
			m_input = INPUT_SEARCH;
			m_number = 0;
			m_digits = 0;
			break;

		case CMD_REPEAT:
			// the current frame is the start frame; wait in STILL for the end frame
			if (!native)
			{
				reply = RET_ERROR;
				break;
			}
			m_repeat_start = m_last_frame;
			m_repeat_speed = SPEED_PLAY;
			m_input = INPUT_REPEAT_FRAME;
			m_number = 0;
			m_digits = 0;
			still(false);
			break;

		case CMD_F_PLAY: case CMD_F_FAST: case CMD_F_SLOW: case CMD_F_STEP: case CMD_F_SCAN:
		case CMD_R_PLAY: case CMD_R_FAST: case CMD_R_SLOW: case CMD_R_STEP: case CMD_R_SCAN:
		{
			const bool reverse = data >= CMD_R_PLAY;
			const u8 speed = data - (reverse ? CMD_R_PLAY : CMD_F_PLAY) + SPEED_PLAY;
			if (repeat_input)
			{
				// the speed for REPEAT is chosen with the forward keys; the
				// start and end frames decide the direction
				if (reverse || speed == SPEED_SCAN)
					reply = RET_ERROR;
				else
					m_repeat_speed = speed;
			}
			else if (native)
			{
				m_step_rate = 7;
				play(speed, reverse, m_step_rate);
				if (speed == SPEED_STEP)
				{
					m_input = INPUT_STEP;
					m_number = 0;
					m_digits = 0;
				}
			}
			else
			{
				reply = RET_ERROR;
			}
			break;
		}

		case CMD_STOP:
			if (native)
			{
				m_still_saved_state = LDSTATE_NONE;
				still(true);
			}
			else
			{
				reply = RET_ERROR;
			}
			break;

		case CMD_STILL:
			// the manuals don't say what STILL does to a SEARCH or REPEAT in progress; this abandons it
			// CONTINUE returns to the playback mode before STILL
			m_operation = OP_NONE;
			m_still_saved_state = (m_player_state.m_state == LDSTATE_SEEKING) ? LDSTATE_NONE : m_player_state.m_state;
			m_still_saved_param = m_player_state.m_param;
			still(false);
			break;

		case CMD_CONTINUE:
			if (m_still_saved_state != LDSTATE_NONE)
			{
				m_player_state.m_state = m_still_saved_state;
				m_player_state.m_substate = 0;
				m_player_state.m_param = m_still_saved_param;
				m_still_saved_state = LDSTATE_NONE;
				m_stopped = false;
				set_video_squelch(false);
				update_audio_squelch();
			}
			break;

		case CMD_CH1_ON:
		case CMD_CH1_OFF:
			m_ch1 = data == CMD_CH1_ON;
			update_audio_squelch();
			break;

		case CMD_CH2_ON:
		case CMD_CH2_OFF:
			m_ch2 = data == CMD_CH2_ON;
			update_audio_squelch();
			break;

		case CMD_CH1:
			m_ch1 = !m_ch1;
			update_audio_squelch();
			break;

		case CMD_CH2:
			m_ch2 = !m_ch2;
			update_audio_squelch();
			break;

		case CMD_INDEX_ON:
		case CMD_INDEX_OFF:
		case CMD_INDEX:
		case CMD_FRAME_MODE:
			break;

		case CMD_ADDR_INQ:
		{
			// five ASCII digits, no ACK
			u8 digits[5];
			u32 value = std::clamp(m_last_frame, 0, 99'999);
			for (int i = 4; i >= 0; i--, value /= 10)
				digits[i] = '0' + (value % 10);
			queue_reply(digits, 5, ack_time);
			return;
		}

		case CMD_STATUS_INQ:
		{
			// five status bytes, no ACK
			const bool operation = m_operation == OP_SEARCH || m_operation == OP_REPEAT || m_operation == OP_REPEAT_RETURN;
			const bool searching = m_input == INPUT_SEARCH || m_operation == OP_SEARCH;
			const bool repeating = repeat_input || m_operation == OP_REPEAT || m_operation == OP_REPEAT_RETURN;
			const bool playing = m_player_state.m_state >= LDSTATE_PLAYING && m_player_state.m_state <= LDSTATE_PLAYING_FAST_FORWARD;
			const u8 status[5] = {
					u8((operation ? 0x40 : 0x00) | (m_motor_off ? 0x20 : 0x00) | (m_error ? 0x01 : 0x00)),
					0x00,
					u8(native ? 0x40 : 0x00),
					u8((m_input == INPUT_STEP ? 0x80 : 0x00) | (repeating ? 0x04 : 0x00) | (searching ? 0x02 : 0x00) | (m_input != INPUT_NONE ? 0x01 : 0x00)),
					u8((m_reverse ? 0x80 : 0x00) | (m_stopped ? 0x40 : 0x00) | (playing ? SPEED_STATUS[m_speed] : 0x00)) };
			queue_reply(status, 5, ack_time);
			return;
		}

		case CMD_DISC_ID_INQ:
			// the disc ID is part of the audio code, which isn't emulated
			reply = RET_NAK;
			break;

		case CMD_MOTOR_OFF:
			// return the sled to the home position and stop the spindle; the ACK follows once that's done
			m_operation = OP_MOTOR_OFF;
			m_input = INPUT_NONE;
			m_motor_off = true;
			m_player_state.m_state = LDSTATE_PARKED;
			m_player_state.m_substate = 0;
			m_player_state.m_param = 0;
			set_video_squelch(true);
			update_audio_squelch();
			m_motor_timer->adjust(MOTOR_OFF_TIME);
			return;

		case CMD_MOTOR_ON:
			// ACK, then spin up; another ACK follows when the player is ready
			if (m_motor_off)
			{
				m_motor_off = false;
				queue_reply(RET_ACK, ack_time);
				begin_init();
				return;
			}
			break;

		default:
			logerror("Unimplemented command %02x\n", data);
			break;
	}

	if (reply == RET_ERROR)
		m_error = true;
	queue_reply(reply, ack_time);
}


//-------------------------------------------------
//  enter_command - handle ENTER, which ends a
//  number being entered
//-------------------------------------------------

u8 sony_ldp1000_device::enter_command()
{
	switch (m_input)
	{
		case INPUT_SEARCH:
			if (!m_digits)
				return RET_ERROR;
			m_input = INPUT_NONE;
			m_operation = OP_SEARCH;
			begin_search(m_number);
			return RET_ACK;

		case INPUT_REPEAT_FRAME:
			if (!m_digits)
				return RET_ERROR;
			m_repeat_end = m_number;
			m_input = INPUT_REPEAT_COUNT;
			m_number = 0;
			m_digits = 0;
			return RET_ACK;

		case INPUT_REPEAT_COUNT:
			// 1 when no number is entered, up to 15, and 0 for no limit
			if (m_digits && m_number > 15)
				return RET_ERROR;
			m_repeat_count = m_digits ? m_number : 1;
			m_number = 0;
			m_digits = 0;
			if (m_repeat_speed == SPEED_STEP)
			{
				m_input = INPUT_REPEAT_STEP;
			}
			else
			{
				m_input = INPUT_NONE;
				begin_repeat();
			}
			return RET_ACK;

		case INPUT_REPEAT_STEP:
		case INPUT_STEP:
			// step rate 1 to 255
			if (!m_digits || m_number < 1 || m_number > 255)
				return RET_ERROR;
			m_step_rate = m_number;
			if (m_input == INPUT_REPEAT_STEP)
			{
				m_input = INPUT_NONE;
				begin_repeat();
			}
			else
			{
				m_input = INPUT_NONE;
				play(SPEED_STEP, m_reverse, m_step_rate);
			}
			return RET_ACK;

		default:
			return RET_ERROR;
	}
}


//-------------------------------------------------
//  motor_stopped - the motor has stopped after
//  MOTOR OFF
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sony_ldp1000_device::motor_stopped)
{
	m_operation = OP_NONE;
	queue_reply(RET_ACK, attotime::zero);
}


//**************************************************************************
//  SERIAL INTERFACE
//**************************************************************************

//-------------------------------------------------
//  queue_reply - queue bytes to send to the
//  external computer after a delay
//-------------------------------------------------

void sony_ldp1000_device::queue_reply(const u8 *data, unsigned length, const attotime &delay)
{
	const bool idle = !m_reply_count;
	for (unsigned i = 0; i < length; i++)
	{
		if (m_reply_count >= std::size(m_reply))
		{
			logerror("Reply buffer overflow\n");
			break;
		}
		LOGMASKED(LOG_REPLY, "reply %02x\n", data[i]);
		m_reply[(m_reply_read + m_reply_count) % std::size(m_reply)] = data[i];
		m_reply_count++;
	}

	// a reply that's already queued or being sent carries the new bytes along
	if (idle && m_reply_count)
		m_reply_timer->adjust(delay);
}


//-------------------------------------------------
//  send_reply - start sending the queued reply
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sony_ldp1000_device::send_reply)
{
	transmit_register_setup(m_reply[m_reply_read]);
}


//-------------------------------------------------
//  rcv_complete - called by diserial when a byte
//  has been received
//-------------------------------------------------

void sony_ldp1000_device::rcv_complete()
{
	receive_register_extract();
	process_command(get_received_char());
}


//-------------------------------------------------
//  tra_complete - called by diserial when a byte
//  has been sent
//-------------------------------------------------

void sony_ldp1000_device::tra_complete()
{
	m_reply_read = (m_reply_read + 1) % std::size(m_reply);
	m_reply_count--;
	if (m_reply_count)
		transmit_register_setup(m_reply[m_reply_read]);
}


//-------------------------------------------------
//  tra_callback - called by diserial to send a
//  single bit
//-------------------------------------------------

void sony_ldp1000_device::tra_callback()
{
	m_serial_tx(transmit_register_get_data_bit());
}
