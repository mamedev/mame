// license:BSD-3-Clause

/*************************************************************************

    ldp1450hle.cpp

    HLE for the LDP 1000 series, such as LDP-1450 
	NTSC for now, PAL (P suffix) would set base speed to 50, at the least


**************************************************************************

    To do:

        * On-screen display support (we just popmessage the data)
        * Better CLV support
        * Chapter-search support
        * Repeat support (we only store the command)
        * Commands that Time Traveler doesn't use:
          - Scan Forward/Reverse
          - Multitrack-Jump Forward/Reverse
          - Clear
          - Leadout Symbol
          - OSD
          - Status Requests
          - UI text display (Nova games, DL2 need this)
        * Repeat behaviour for reverse play should restart in reverse
        * Delay timing of queue is a guess based on LDP1000A guide
        * Not all features are fully hooked up
		* Still step back and forth in Time Traveler glitches
		  - (level select doesn't stay in place)	
*************************************************************************/

#include "emu.h"
#include "ldp1450hle.h"


#define LOG_COMMAND_BYTES       (1U << 1)
#define LOG_COMMANDS            (1U << 2)
#define LOG_COMMAND_BUFFERS     (1U << 3)
#define LOG_REPLIES             (1U << 4)
#define LOG_REPLY_BYTES         (1U << 5)
#define LOG_SEARCHES            (1U << 6)
#define LOG_STOPS               (1U << 7)
#define LOG_SQUELCHES           (1U << 8)
#define LOG_FRAMES              (1U << 9)
#define LOG_ALL                 (LOG_COMMAND_BYTES | LOG_COMMANDS | LOG_COMMAND_BUFFERS | LOG_REPLY_BYTES | LOG_SEARCHES | LOG_STOPS | LOG_SQUELCHES | LOG_FRAMES)


#define VERBOSE 0

#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(SONY_LDP1450HLE, sony_ldp1450hle_device, "ldp1450hle", "Sony LDP-1450 HLE")


//-------------------------------------------------
//  sony_ldp1450hle_device - constructor
//-------------------------------------------------

sony_ldp1450hle_device::sony_ldp1450hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: laserdisc_device(mconfig, SONY_LDP1450HLE, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_serial_tx(*this)
	, m_vbi_fetch(nullptr)
	, m_cmd_running(false)
	, m_reply_write_index(0)
	, m_reply_read_index(0)
	, m_mode(MODE_PARK)
	, m_chapter(0)
	, m_time(0)
	, m_search_chapter(~u32(0))
	, m_search_frame(~u32(0))
	, m_mark_chapter(~u32(0))
	, m_mark_frame(~u32(0))
	, m_video_switch(1)
	, m_ch1_switch(false)
	, m_ch2_switch(false)
	, m_display_switch(0)
	, m_address_flag(ADDRESS_FRAME)
	, m_base_speed(60)
	, m_speed(60)
	, m_speed_accum(0)
	, m_curr_frame(0)
{
}


void sony_ldp1450hle_device::queue_reply(u8 reply, float delay)
{
	const u8 reply_buffer[5] = {reply, 0, 0, 0, 0};
	queue_reply_buffer(reply_buffer, delay);
}

void sony_ldp1450hle_device::queue_reply_buffer(const u8 reply[], float delay)
{
	u8 max_writable = (u8)std::size(m_reply_buffer);
	for (u8 i = 0; i < 5 && reply[i] != 0; i++)
	{
		m_reply_buffer[m_reply_write_index] = reply[i];
		m_reply_write_index = (m_reply_write_index + 1) % max_writable;
	}

	m_queue_timer->adjust(attotime::from_nsec(delay * 1000000));
}

TIMER_CALLBACK_MEMBER(sony_ldp1450hle_device::process_queue)
{
	LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", (u8)m_reply_buffer[m_reply_read_index]);
	transmit_register_setup(m_reply_buffer[m_reply_read_index]);
}

//-------------------------------------------------
//  bcd_to_literal - converts a BCD value used in
//  commands a direct numeric value
//-------------------------------------------------

u32 sony_ldp1450hle_device::bcd_to_literal(u32 bcd)
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
//  process_command - processes a single command
//  from the command buffer
//-------------------------------------------------

void sony_ldp1450hle_device::add_command_byte(u8 command)
{
	switch (m_submode)
	{
		case SUBMODE_USER_INDEX:
		{
			switch (command)
			{
				case 0x00:
				{
					m_submode = SUBMODE_USER_INDEX_MODE_1;
					queue_reply(0x0a, 4.3);
					break;
				}
				case 0x01:
				{
					m_submode = SUBMODE_USER_INDEX_STRING_1;
					queue_reply(0x0a, 4.3);
					break;
				}
				case 0x02:
				{
					m_submode = SUBMODE_USER_INDEX_WINDOW;
					queue_reply(0x0a, 4.3);
					break;
				}
			}
			break;
		}
		case SUBMODE_USER_INDEX_MODE_1:
		{
			m_user_index_x = (command & 0x3f);
			m_submode = SUBMODE_USER_INDEX_MODE_2;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_MODE_2:
		{
			m_user_index_y = (command & 0x3f);
			m_submode = SUBMODE_USER_INDEX_MODE_3;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_MODE_3:
		{
			m_user_index_mode = command;
			m_submode = SUBMODE_NORMAL;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_STRING_1:
		{
			m_user_index_char_idx = (command & 0x1f);
			m_submode = SUBMODE_USER_INDEX_STRING_2;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_STRING_2:
		{
			m_user_index_chars[m_user_index_char_idx] = (char)(command & 0x5f);
			if (command == 0x1a)
			{
				m_submode = SUBMODE_NORMAL;
			}
			else
			{
				m_user_index_char_idx++;
				if (m_user_index_char_idx > 32)
				{
					m_user_index_char_idx = 0;
				}
			}
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_WINDOW:
		{
			m_user_index_window_idx = (command & 0x1f);
			m_submode = SUBMODE_NORMAL;
			queue_reply(0x0a, 4.3);
			break;
		}

		default:
		{
			if (command >= 0x30 && command <=0x39)
			{
				if (m_mode == MODE_SEARCH_CMD || m_mode == MODE_REPEAT_CMD_MARK || m_mode == MODE_REPEAT_CMD_REPS || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE  )
				{
					//Reset flags
					if (m_cmd_buffer == -2)
					{
						m_cmd_buffer = 0;
					}

					if (m_cmd_buffer != 0)
					{
						m_cmd_buffer *= 10;
					}
					m_cmd_buffer += (command - 0x30);
					queue_reply(0x0a, 4.3);
				}
				else
				{
						queue_reply(0x0b, 4.3);
				}
			}
			else {
			switch (command)
			{
				case CMD_ENTER:
				{
					if (m_mode == MODE_SEARCH_CMD)
					{
						begin_search(m_cmd_buffer);
						m_mode = MODE_SEARCH;
					}
					else if (m_mode == MODE_REPEAT_CMD_MARK)
					{
						if (m_address_flag == ADDRESS_FRAME)
						{
							m_repeat_frame_end = m_cmd_buffer;
						}
						else if (m_address_flag == ADDRESS_CHAPTER)
						{
							m_repeat_chapter_end = m_cmd_buffer;
						}
						m_mode = MODE_REPEAT_CMD_REPS;
						m_cmd_buffer = -2;
					}
					else if (m_mode == MODE_REPEAT_CMD_REPS)
					{
						// if no number, presume 1
						if (m_cmd_buffer == -2)
						{
							m_cmd_buffer = 1;
						}
						else if (m_cmd_buffer == 0)
						{
							m_cmd_buffer = -1; // infinite
						}
						m_repeat_repetitions = m_cmd_buffer;
						m_mode = MODE_PLAY;
						m_cmd_buffer = 0;
						update_video_enable();
						update_audio_squelch();
					}
					else if (m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
					{
						m_speed = m_base_speed / m_cmd_buffer;
						update_video_enable();
						update_audio_squelch();
					}
					queue_reply(0x0a, 1.3);
					break;
				}
				case CMD_STEP_STILL:
				{
					m_mode = MODE_STILL;
					set_audio_squelch(true, true);
					m_mark_frame = ~u32(0);
					advance_slider(1);
					queue_reply(0x0a, 1.4);
					break;
				}
				case CMD_STEP_STILL_REVERSE:
				{
					m_mode = MODE_STILL;
					set_audio_squelch(true, true);
					m_mark_frame = ~u32(0);
					advance_slider(-1);
					queue_reply(0x0a, 1.4);
					break;
				}
				case CMD_STOP:
				{
					m_mode = MODE_PARK;
					set_audio_squelch(true, true);
					set_video_squelch(true);
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_PLAY:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed;
					m_mode = MODE_PLAY;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_FAST_FORWARD:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed * 3;
					m_mode = MODE_MS_FORWARD;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_SLOW_FORWARD:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed / 5;
					m_mode = MODE_MS_FORWARD;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_STEP_FORWARD:
				{
					// Most ROM revisions start at a slow speed, and then update to correct one
					m_speed = m_base_speed / 7;
					m_mode = MODE_MS_FORWARD;
					set_audio_squelch(true, true);
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_SCAN_FORWARD:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed * 100;
					m_mode = MODE_MS_FORWARD;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_PLAY_REVERSE:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed;
					m_mode = MODE_MS_REVERSE;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_FAST_REVERSE:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed * 3;
					m_mode = MODE_MS_REVERSE;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_SLOW_REVERSE:
				{
					m_speed_accum = 0;
					m_speed = m_base_speed / 5;
					m_mode = MODE_MS_REVERSE;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_STEP_REVERSE:
				{
					m_speed = m_base_speed / 7;
					m_mode = MODE_MS_REVERSE;
					set_audio_squelch(true, true);
					m_mark_frame = 0;
					advance_slider(-1);
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_SCAN_REVERSE:
				{
					m_speed = m_base_speed / 7;
					m_mode = MODE_MS_REVERSE;
					update_audio_squelch();
					update_video_enable();
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_STILL:
				{
					m_mode = MODE_STILL;
					set_audio_squelch(true, true);
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_SEARCH:
				{
					m_mode = MODE_SEARCH_CMD;
					m_cmd_buffer = 0;
					m_repeat_frame_start = 0;
					m_repeat_frame_end = 0;
					m_repeat_chapter_start = 0;
					m_repeat_chapter_end = 0;
					m_repeat_repetitions = 0;
					m_search_frame = ~u32(0);
					update_audio_squelch();
					queue_reply(0x0a, 9.0);
					break;
				}
				case CMD_REPEAT:
				{
					m_mode = MODE_REPEAT_CMD_MARK;
					m_cmd_buffer = 0;
					if (m_address_flag == ADDRESS_FRAME)
					{
						m_repeat_frame_start = m_curr_frame;
					}
					else if (m_address_flag == ADDRESS_CHAPTER)
					{
						m_repeat_chapter_start = m_chapter;
					}
					m_repeat_frame_end = 0;
					m_repeat_chapter_end = 0;
					m_repeat_repetitions = 0;
					update_video_enable();
					update_audio_squelch();
					queue_reply(0x0a, 8.0);
					break;
				}
				case CMD_FRAME_SET:
				{
					m_address_flag = ADDRESS_FRAME;
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_CHAPTER_SET:
				{
					m_address_flag = ADDRESS_CHAPTER;
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_CLEAR:
				{
					m_cmd_buffer = 0;
					queue_reply(0x0a, 8.0);
					break;
				}
				case CMD_CLEAR_ALL:
				{
					m_cmd_buffer = 0;
					m_search_frame = 0;
					m_search_chapter = 0;
					m_repeat_chapter_start = 0;
					m_repeat_frame_start = 0;
					m_repeat_chapter_end = 0;
					m_repeat_frame_end = 0;
					m_repeat_repetitions = 0;
					m_speed = m_base_speed;
					m_mode = MODE_STILL;
					queue_reply(0x0a, 5.5);
					break;
				}
				case CMD_CH1_ON:
				{
					m_ch1_switch=true;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_CH1_OFF:
				{
					m_ch1_switch=false;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_CH2_ON:
				{
					m_ch2_switch=true;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_CH2_OFF:
				{
					m_ch2_switch=false;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_AUDIO_ON:
				{
					m_ch1_switch=true;
					m_ch2_switch=true;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_AUDIO_OFF:
				{
					m_ch1_switch=false;
					m_ch2_switch=false;
					update_audio_squelch();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_VIDEO_ON:
				{
					m_video_switch = 1;
					update_video_enable();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_VIDEO_OFF:
				{
					m_video_switch = 0;
					update_video_enable();
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_INDEX_ON:
				{
					m_display_switch = 1;
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_INDEX_OFF:
				{
					m_display_switch =0;
					queue_reply(0x0a, 0.4);
					break;
				}
				case CMD_ADDR_INQ:
				{
					u8 frame_buffer[5];
					u32 frame_val = m_curr_frame;
					for (u8 i = 0; i < 5; i++)
					{
						frame_buffer[4 - i] = frame_val%10 + 0x30;
						frame_val /= 10;
					}
					queue_reply_buffer(frame_buffer, 1.3);
					break;
				}
				case CMD_STATUS_INQ:
				{
					u8 status_buffer[5] = { 0x80, 0x00, 0x10, 0x00, 0xff};

					if (m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
					{
						status_buffer[4] = 0x01;
					}
					else if (m_mode == MODE_PAUSE || m_mode == MODE_STILL)
					{
						status_buffer[4] = 0x20;
					}
					queue_reply_buffer(status_buffer, 1.3);
					break;
				}
				case CMD_USER_INDEX_CTRL:
				{
					m_submode = SUBMODE_USER_INDEX;
					queue_reply(0x0a, 0.4);
					break;
				}

				case CMD_USER_INDEX_ON:
				{
					popmessage("X %x Y %x M%x T%s (Start %x)", m_user_index_x, m_user_index_y, m_user_index_mode, m_user_index_chars,m_user_index_window_idx);
					queue_reply(0x0a, 0.4);
					break;
				}

				case CMD_USER_INDEX_OFF:
				{
					queue_reply(0x0a, 0.4);
					break;
				}

				default:
				{
					popmessage("no implementation cmd %x", command);
					queue_reply(0x0b, 0.4);
					break;
				}
			}
		}
		LOGMASKED(LOG_SEARCHES, "Command %x\n", command);
		}
	}
}

//-------------------------------------------------
//  begin_search - initiates a search operation
//-------------------------------------------------

void sony_ldp1450hle_device::begin_search(u32 value)
{
	if (m_address_flag == ADDRESS_FRAME)
	{
		m_search_frame = value;
	}
	else if (m_address_flag == ADDRESS_CHAPTER)
	{
		m_search_chapter = value;
	}

	set_audio_squelch(true, true);

	if (std::abs((int32_t)m_search_frame - (int32_t)m_curr_frame) > 100)
	{
		video_enable(false);
		set_audio_squelch(true, true);
	}
	else
	{
	}
}


//-------------------------------------------------
//  update_audio_squelch - set audio squelch state
//  on the base device based on our audio switches
//-------------------------------------------------

void sony_ldp1450hle_device::update_audio_squelch()
{
	const bool squelch_both = (m_mode == MODE_STILL || m_mode == MODE_PAUSE || m_mode == MODE_SEARCH || m_mode == MODE_MS_REVERSE || m_mode == MODE_SEARCH_CMD || m_mode == MODE_SEARCH_REP );
	const bool squelch_left = !(m_ch1_switch) || squelch_both;
	const bool squelch_right = !(m_ch2_switch) || squelch_both;
	set_audio_squelch(squelch_left, squelch_right);
}

//-------------------------------------------------
//  update_video_enable - set video enable state
//  on the base device based on our video switch
//-------------------------------------------------

void sony_ldp1450hle_device::update_video_enable()
{
	set_video_squelch(!(m_video_switch == 1 && (m_mode == MODE_STILL || m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)));
	video_enable(m_video_switch == 1 && (m_mode == MODE_STILL || m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE));
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void sony_ldp1450hle_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	// allocate timers
	m_vbi_fetch = timer_alloc(FUNC(sony_ldp1450hle_device::process_vbi_data), this);
	m_queue_timer = timer_alloc(FUNC(sony_ldp1450hle_device::process_queue), this);

	// register state saving
	save_item(NAME(m_baud));
	save_item(NAME(m_cmd_buffer));
	save_item(NAME(m_cmd_running));
	save_item(NAME(m_reply_buffer));
	save_item(NAME(m_reply_write_index));
	save_item(NAME(m_reply_read_index));
	save_item(NAME(m_reply));
	save_item(NAME(m_mode));
	save_item(NAME(m_chapter));
	save_item(NAME(m_time));
	save_item(NAME(m_search_chapter));
	save_item(NAME(m_search_frame));
	save_item(NAME(m_mark_chapter));
	save_item(NAME(m_mark_frame));
	save_item(NAME(m_video_switch));
	save_item(NAME(m_ch1_switch));
	save_item(NAME(m_ch2_switch));
	save_item(NAME(m_display_switch));
	save_item(NAME(m_address_flag));
	save_item(NAME(m_base_speed));
	save_item(NAME(m_speed));
	save_item(NAME(m_speed_accum));
	save_item(NAME(m_curr_frame));
	save_item(NAME(m_frame));
	save_item(NAME(m_repeat_chapter_start));
	save_item(NAME(m_repeat_chapter_end));
	save_item(NAME(m_repeat_frame_start));
	save_item(NAME(m_repeat_frame_end));

}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void sony_ldp1450hle_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();


	// initialize diserial
	set_tra_rate(attotime::from_hz(m_baud));
	set_rcv_rate(attotime::from_hz(m_baud));
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	// reset our state
	m_vbi_fetch->adjust(attotime::never);
	m_queue_timer->adjust(attotime::never);

	m_cmd_running = false;
	std::fill_n(m_reply_buffer, std::size(m_reply_buffer), 0);
	m_reply_write_index = 0;
	m_reply_read_index = 0;

	m_mode = MODE_PARK;
	m_chapter = 0;
	m_time = 0;
	m_cmd_buffer = ~u32(0);
	m_search_chapter = ~u32(0);
	m_search_frame = ~u32(0);
	m_mark_chapter = ~u32(0);
	m_mark_frame = ~u32(0);
	m_repeat_chapter_start = ~u32(0);
	m_repeat_chapter_end = ~u32(0);
	m_repeat_frame_start = ~u32(0);
	m_repeat_frame_end = ~u32(0);
	m_video_switch = 1;
	m_ch1_switch = false;
	m_ch2_switch = false;
	m_display_switch = 0;
	m_address_flag = ADDRESS_FRAME;
	m_base_speed = 60;
	m_speed = m_base_speed;
	m_speed_accum = 0;

	video_enable(true);
	set_audio_squelch(true, true);
}


//-------------------------------------------------
//  process_vbi_data - process VBI data and
//  act on search/play seeking
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sony_ldp1450hle_device::process_vbi_data)
{
	u32 line = get_field_code(LASERDISC_CODE_LINE1718, false);
	if ((line & 0xf80000) == 0xf80000 || line == VBI_CODE_LEADIN || line == VBI_CODE_LEADOUT)
	{
		u32 old_frame = m_curr_frame;
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

			if (m_mark_frame != ~u32(0) && m_search_frame == ~u32(0))
			{
				int32_t old_delta = (int32_t)m_mark_frame - (int32_t)old_frame;
				int32_t curr_delta = (int32_t)m_mark_frame - (int32_t)m_curr_frame;
				LOGMASKED(LOG_STOPS, "Stop Mark is currently %d, old frame is %d, current frame is %d, old delta %d, curr delta %d\n", m_mark_frame, old_frame, m_curr_frame, old_delta, curr_delta);
				if (curr_delta == 0 || (old_delta < 0) != (curr_delta < 0))
				{
					m_mark_frame = ~u32(0);
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

					set_audio_squelch(true, true);
				}
			}

			if (m_search_frame != ~u32(0))
			{
				// TODO: Chapter-search support
				int32_t delta = (int32_t)m_search_frame - (int32_t)m_curr_frame;
				if (delta == 0)
				{
					m_search_frame = ~u32(0);
					if (is_cav_disc())
					{
						if (m_mode == MODE_SEARCH_REP)
						{
							m_mode = MODE_PLAY;
						}
						else
						{
							m_mode = MODE_STILL;
						}
						update_video_enable();
					}
					else
					{
						m_mode = MODE_PLAY;
						update_video_enable();
					}

					queue_reply(0x01, 1.3); //completion
				}
				else if (delta <= 2 && delta > 0)
				{
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


			bool repeat_hit = ((m_repeat_frame_end != ~u32(0) && m_curr_frame >= m_repeat_frame_end) ||  (m_mode == MODE_MS_REVERSE && (m_repeat_frame_start != ~u32(0) && m_curr_frame <= m_repeat_frame_start) ));
			if (m_repeat_repetitions != 0 && repeat_hit)
			{

				if (m_repeat_repetitions > 0)
				{
					m_repeat_repetitions -= 1;
				}

				bool reverse = m_mode == MODE_MS_REVERSE;

				m_mode = MODE_STILL;
				update_video_enable();
				update_audio_squelch();
				if (m_repeat_repetitions != 0)
				{
					m_mode = MODE_SEARCH_REP;
					if (reverse)
					{
						begin_search(m_repeat_frame_end);
					}
					else{
						begin_search(m_repeat_frame_start);
					}
				}
				else
				{
					m_repeat_frame_end = ~u32(0);
					m_repeat_frame_start = ~u32(0);
					queue_reply(0x01, 1.3);
				}
			}

		}
	}
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void sony_ldp1450hle_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
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

int32_t sony_ldp1450hle_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (!fieldnum)
		return 0;

	if (m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
	{
		m_speed_accum += m_speed;
		int elapsed_tracks = m_speed_accum / m_base_speed;
		m_speed_accum -= elapsed_tracks * m_base_speed;
		if (m_mode == MODE_MS_REVERSE)
			elapsed_tracks *= -1;

		if (m_mark_frame != ~u32(0))
		{
			int32_t jump_frame = (int32_t)m_curr_frame + elapsed_tracks;
			int32_t curr_delta = (int32_t)m_mark_frame - (int32_t)m_curr_frame;
			int32_t next_delta = (int32_t)m_mark_frame - (int32_t)jump_frame;
			if ((curr_delta < 0) != (next_delta < 0))
			{
				elapsed_tracks = curr_delta;
			}
		}
		return elapsed_tracks;
	}


	if (m_mode == MODE_PLAY || m_mode == MODE_SEARCH || m_mode == MODE_SEARCH_REP)
	{
		return 1;
	}

	return 0;
}


//-------------------------------------------------
//  rcv_complete - called by diserial when we
//  have received a complete byte
//-------------------------------------------------

void sony_ldp1450hle_device::rcv_complete()
{
	receive_register_extract();
	add_command_byte(get_received_char());
}


//-------------------------------------------------
//  tra_complete - called by diserial when we
//  have transmitted a complete byte
//-------------------------------------------------

void sony_ldp1450hle_device::tra_complete()
{
	m_reply_read_index = (m_reply_read_index + 1) % (u8)std::size(m_reply_buffer);
	if (m_reply_read_index != m_reply_write_index)
	{
		u8 data = (u8)m_reply_buffer[m_reply_read_index];
		LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", data);
		transmit_register_setup(data);
	}
}


//-------------------------------------------------
//  tra_callback - called by diserial when we
//  transmit a single bit
//-------------------------------------------------

void sony_ldp1450hle_device::tra_callback()
{
	m_serial_tx(transmit_register_get_data_bit());
}
