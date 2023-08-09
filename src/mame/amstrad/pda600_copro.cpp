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
	, m_fake_ioport(*this, "FAKE%u", 0U)
	, m_state(0)
	, m_resp_type(0)
	, m_resp_data(0)
	, m_buf_size(0)
{
}


static INPUT_PORTS_START(pda600_copro)
	PORT_START("FAKE0")    // 0x20 - 0x3f
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('!')
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'£')
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('#')
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('$')
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('%')
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('&')
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('\'') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('(')
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(')')
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('*') PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('+') PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(',') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('-') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('.') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('/') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('0') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('1') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('2') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('3') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('4') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('5') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('6') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('7') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('8') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('9') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(':')
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(';') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'²')
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('=') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'³')
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('?')

	PORT_START("FAKE1")    // 0x40 - 0x5f
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'°')
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'¿')
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ä')
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Å')
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Æ')
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ç')

	PORT_START("FAKE2")    // 0x60 - 0x7f
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ë')
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('a')
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('b')
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('c')
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('d')
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('e')
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('f')
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('g')
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('h')
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('i')
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('j')
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('k')
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('l')
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('m')
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('n')
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('o')
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('p')
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('q')
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('r')
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('s')
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('t')
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('u')
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('v')
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('w')
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('x')
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('y')
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR('z')
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ñ')
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ö')
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ø')
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ù')
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'Ü')

	PORT_START("FAKE3")    // 0x80 - 0x9a
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'à')
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'á')
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'â')
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ä')
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'å')
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'æ')
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ç')
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'è')
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'é')
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ê')
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ë')
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ì')
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'í')
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'î')
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ï')
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ñ')
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ò')
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ó')
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ô')
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ö')
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ø')
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ù')
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ú')
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'û')
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'ü')
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'β')
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CHAR(U'§')
INPUT_PORTS_END

ioport_constructor pda600_copro_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pda600_copro);
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
		std::memmove(&m_buf[0], &m_buf[2], m_buf_size);
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


void pda600_copro_device::wakeup_w(int state)
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
			std::ostringstream tmp;
			for (auto b : m_buf)
				util::stream_format(tmp, "%02X ", b);

			logerror("%s\n", std::move(tmp).str());
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

		if (m_buf[1] == 0)
		{
			logerror("PDA600: malformed frame\n");
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
	std::memmove(&m_buf[0], &m_buf[3], m_buf_size);
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

	for (int i = 0; i < 4; i++)
	{
		u32 value = m_fake_ioport[i]->read();
		if (value != 0)
			return 0x20 + i * 32 + count_leading_zeros_32(value);
	}

	return 0;
}
