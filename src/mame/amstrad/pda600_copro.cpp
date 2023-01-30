// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Amstrad PenPad PDA 600 character recognition Coprocessor HLE


    The protocol flow control:
      - 0x01   SOH   Start a new frame
      - 0x06   ACK   Ack a request
      - 0x15   NAK   Negative ack


    Frame format:
      +--------+--------+--------+---//---+----------+
      |  0x01  | Length |  Type  |  Data  | Checksum |
      +--------+--------+--------+---//---+----------+

      Length:
        Length of data + 1.

      Type:
        Frame type:
          - 0x54 (T) Train a character
          - 0x52 (R) Recognize a character
          - 0x42 (B) Beep
          - 0x49 (I) Init
          - 0x53 (S) Sleep
          - 0x46 (F) Used before training a character
          - 0x59 (Y) Affirmative response
          - 0x4e (N) Negative response

      Checksum:
        The two's complement of the 8-bit sum of Length, Type and Data.

**********************************************************************/

#include "emu.h"
#include "pda600_copro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static constexpr u8 PDA600_SOH = 0x01;
static constexpr u8 PDA600_ACK = 0x06;
static constexpr u8 PDA600_NAK = 0x15;

enum : u8
{
	STATE_READY,
	STATE_WAIT_ACK,
	STATE_SLEEP,
	STATE_BUSY,
	STATE_PLAY_TONE,
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PDA600_COPRO_HLE, pda600_copro_device, "pda600_copro", "PDA600 Coprocessor (HLE)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pda600_copro_device - constructor
//-------------------------------------------------

pda600_copro_device::pda600_copro_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PDA600_COPRO_HLE, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_tx_cb(*this)
	, m_tone_cb(*this)
	, m_state(0)
	, m_resp_type(0)
	, m_resp_data(0)
	, m_buf_size(0)
{
}


void pda600_copro_device::device_resolve_objects()
{
	// resolve callbacks
	m_tx_cb.resolve_safe();
	m_tone_cb.resolve_safe();
}


void pda600_copro_device::device_start()
{
	// parameters used by the MAINCPU to configure the Z180 ASCI1
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(clock() / 1280);

	// state saving
	save_item(NAME(m_state));
	save_item(NAME(m_resp_type));
	save_item(NAME(m_resp_data));
	save_item(NAME(m_buf));
	save_item(NAME(m_buf_size));

	m_update_timer = timer_alloc(FUNC(pda600_copro_device::update_timer), this);
}


void pda600_copro_device::device_reset()
{
	m_tx_cb(1);
	m_tone_cb(0);

	m_state = STATE_READY;
	m_buf_size = 0;
	m_resp_type = m_resp_data = 0;

	m_update_timer->adjust(attotime::never);
}


TIMER_CALLBACK_MEMBER(pda600_copro_device::update_timer)
{
	if (m_state == STATE_PLAY_TONE && m_buf_size >= 2)
	{
		m_tone_cb(m_buf[0]);
		m_update_timer->adjust(attotime::from_msec(m_buf[1] * 10));
		m_buf_size -= 2;
		std::memmove(m_buf.begin(), m_buf.begin() + 2, m_buf_size);
	}
	else
	{
		if (m_state == STATE_PLAY_TONE)
			m_tone_cb(0);   // Mute the DTMF tone generator

		// We are ready to send the response
		send_byte(PDA600_SOH);
		m_state = STATE_WAIT_ACK;
	}
}


WRITE_LINE_MEMBER( pda600_copro_device::wakeup_w )
{
	if (m_state != STATE_SLEEP && m_state != STATE_READY)
		logerror("PDA600: wakeup_w in %d state\n", m_state);

	if (state && m_state == STATE_SLEEP)
		send_byte(PDA600_ACK);

	m_state = state ? STATE_READY : STATE_SLEEP;
}


void pda600_copro_device::send_byte(u8 byte)
{
	transmit_byte(byte);
}


void pda600_copro_device::received_byte(u8 byte)
{
	if (m_state != STATE_READY && m_state != STATE_WAIT_ACK)
	{
		logerror("PDA600: write %02x in %d state\n", byte, m_state);
		return;
	}

	if (m_state == STATE_WAIT_ACK)
	{
		if (byte == PDA600_ACK)
		{
			send_resp();
		}
		else
		{
			logerror("PDA600: Invalid ack %02x\n", byte);
			send_byte(PDA600_NAK);
		}

		if (m_buf_size > 2 && m_buf[2] == 'S')
			m_state = STATE_SLEEP;
		else
			m_state = STATE_READY;

		m_buf_size = 0;
		return;
	}

	m_buf[m_buf_size++] = byte;

	if (m_buf[0] == PDA600_ACK)
	{
		m_buf_size = 0;
	}
	else if (m_buf[0] != PDA600_SOH)
	{
		logerror("PDA600: unknown start %02x\n", byte);
		send_byte(PDA600_NAK);
		m_buf_size = 0;
	}
	else if (m_buf_size == 1)
	{
		send_byte(PDA600_ACK);
	}
	else if (m_buf_size == m_buf[1] + 3)
	{
		if (0)
		{
			std::string tmp = "";
			for (auto b : m_buf)
				tmp += util::string_format("%02X ", b);

			logerror("%s\n", tmp);
		}

		u8 checksum = 0;
		for (int i = 1; i < m_buf_size; i++)
			checksum += m_buf[i];

		m_resp_type = m_resp_data = 0;

		if (checksum != 0)
		{
			logerror("PDA600: invalid frame checksum\n");
			send_byte(PDA600_NAK);
			return;
		}

		switch (m_buf[2])
		{
		case 'T':   // Train a character
			exec_train();
			break;
		case 'R':   // Recognize a character
			exec_recognize();
			break;
		case 'B':   // Beep
			exec_beep();
			break;
		case 'F':   // Used before training a character
			m_resp_type = 'Y';
			[[fallthrough]];
		case 'S':   // Sleep
		case 'I':   // Init
			m_update_timer->adjust(attotime::from_msec(1)); // Time required to complete the command
			break;
		default:
			logerror("PDA600: unknown frame type %02x\n", m_buf[2]);
			send_byte(PDA600_NAK);
		}
	}
}


void pda600_copro_device::send_resp()
{
//  Frame format:
//    +--------+--------+--------+--------+--------+--------+----------+
//    |  0x01  | Length |  Type  | Param0 | Param1 | Param2 | Checksum |
//    +--------+--------+--------+--------+--------+--------+----------+
//
//    Type:
//      'Y'  for affirmative response
//      'N'  for negative response
//
//    Param0:
//      Recognized character.
//
//    Param1 / Param2:
//      Appears to be unused.

	u8 checksum = 0;
	if (m_resp_type != 0)
	{
		send_byte(0x04);        // Data length
		send_byte(m_resp_type); // Frame type
		send_byte(m_resp_data); // Param0
		send_byte(0x00);        // Param1
		send_byte(0x00);        // Param2
		checksum += 0x04 + m_resp_type + m_resp_data;
	}
	else
		send_byte(0x00);        // Data length for an empty response

	send_byte(0x100 - checksum); // Checksum

	m_resp_type = m_resp_data = 0;
}


void pda600_copro_device::exec_beep()
{
//  Frame format:
//    +--------+--------+--------+--------+----------+---//--+----------+
//    |  0x01  | Length |  Type  |  Tone  | Duration |  ...  | Checksum |
//    +--------+--------+--------+--------+----------+---//--+----------+
//
//    Tone:
//      xxxx ---- TMC5089 Column 1-4
//      ---- xxxx TMC5089 Row 1-4
//
//    Duration:
//      Duration in centiseconds.
//
//    More than one Tone/Duration pair can be used.

	m_resp_type = m_resp_data = 0;  // Empty response
	m_state = STATE_PLAY_TONE;
	m_buf_size -= 4;
	std::memmove(m_buf.begin(), m_buf.begin() + 3, m_buf_size);
	m_update_timer->adjust(attotime::zero);
}


void pda600_copro_device::exec_recognize()
{
//  Frame format:
//    +--------+--------+--------+-----//----+---------+-------+----------+
//    |  0x01  | Length |  Type  | Image[32] | Strokes | Flags | Checksum |
//    +--------+--------+--------+-----//----+---------+-------+----------+
//
//    Image:
//     16x16 monochrome image of the character to be recognized.
//
//    Strokes:
//     Number of pen strokes used to draw the character.
//
//    Flags:
//     ---- ---x Set if the upper third of the vertical space is used.
//     ---- --x- Set if the middle third of the vertical space is used.
//     ---- -x-- Set if the lower third of the vertical space is used.

	m_resp_data = recognize_char();
	m_resp_type = m_resp_data != 0 ? 'Y' : 'N';
	m_state = STATE_BUSY;
	m_update_timer->adjust(attotime::from_msec(50));    // Time required to complete the recognition
}


void pda600_copro_device::exec_train()
{
//  Frame format:
//    +--------+--------+--------+-----//----+---------+-------+--------+----------+
//    |  0x01  | Length |  Type  | Image[32] | Strokes | Flags |  Char  | Checksum |
//    +--------+--------+--------+-----//----+---------+-------+--------+----------+
//
//    Image, Strokes, Flags:
//     Same as recognition frame.
//
//    Char:
//     Character to be trained with the given image.

	m_resp_type = 'Y';
	m_state = STATE_BUSY;
	m_update_timer->adjust(attotime::from_msec(50));    // Time required to complete the training
}


u8 pda600_copro_device::recognize_char()
{
	// We don't implement any character recognition functionality, but simply return
	// the character corresponding to the key pressed.

	if (machine().input().code_pressed(KEYCODE_0)) return '0';
	if (machine().input().code_pressed(KEYCODE_1)) return '1';
	if (machine().input().code_pressed(KEYCODE_2)) return '2';
	if (machine().input().code_pressed(KEYCODE_3)) return '3';
	if (machine().input().code_pressed(KEYCODE_4)) return '4';
	if (machine().input().code_pressed(KEYCODE_5)) return '5';
	if (machine().input().code_pressed(KEYCODE_6)) return '6';
	if (machine().input().code_pressed(KEYCODE_7)) return '7';
	if (machine().input().code_pressed(KEYCODE_8)) return '8';
	if (machine().input().code_pressed(KEYCODE_9)) return '9';
	if (machine().input().code_pressed(KEYCODE_A)) return 'A';
	if (machine().input().code_pressed(KEYCODE_B)) return 'B';
	if (machine().input().code_pressed(KEYCODE_C)) return 'C';
	if (machine().input().code_pressed(KEYCODE_D)) return 'D';
	if (machine().input().code_pressed(KEYCODE_E)) return 'E';
	if (machine().input().code_pressed(KEYCODE_F)) return 'F';
	if (machine().input().code_pressed(KEYCODE_G)) return 'G';
	if (machine().input().code_pressed(KEYCODE_H)) return 'H';
	if (machine().input().code_pressed(KEYCODE_I)) return 'I';
	if (machine().input().code_pressed(KEYCODE_J)) return 'J';
	if (machine().input().code_pressed(KEYCODE_K)) return 'K';
	if (machine().input().code_pressed(KEYCODE_L)) return 'L';
	if (machine().input().code_pressed(KEYCODE_M)) return 'M';
	if (machine().input().code_pressed(KEYCODE_N)) return 'N';
	if (machine().input().code_pressed(KEYCODE_O)) return 'O';
	if (machine().input().code_pressed(KEYCODE_P)) return 'P';
	if (machine().input().code_pressed(KEYCODE_Q)) return 'Q';
	if (machine().input().code_pressed(KEYCODE_R)) return 'R';
	if (machine().input().code_pressed(KEYCODE_S)) return 'S';
	if (machine().input().code_pressed(KEYCODE_T)) return 'T';
	if (machine().input().code_pressed(KEYCODE_U)) return 'U';
	if (machine().input().code_pressed(KEYCODE_V)) return 'V';
	if (machine().input().code_pressed(KEYCODE_W)) return 'W';
	if (machine().input().code_pressed(KEYCODE_X)) return 'X';
	if (machine().input().code_pressed(KEYCODE_Y)) return 'Y';
	if (machine().input().code_pressed(KEYCODE_Z)) return 'Z';
	if (machine().input().code_pressed(KEYCODE_MINUS)) return '-';
	if (machine().input().code_pressed(KEYCODE_STOP)) return '.';

	return 0;
}
