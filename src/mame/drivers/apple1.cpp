// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    apple1.cpp - Apple I

    Next generation driver written in February 2016 by R. Belmont.
    Thanks to the original crew.

    Apple I has:
        6502 @ 1.023 MHz (~0.960 MHz with RAM refresh)
        4 or 8 KB RAM on-board
        256 byte Monitor ROM
        No IRQs, no sound, dumb terminal video
        6820 PIA for keyboard / terminal interface

    -------------------------------------------------------------------

    How to use cassettes:
    The system has no error checking or checksums, and the cassette
    has no header.
    Therefore, you must know the details, and pass these to the
    interface yourself.

    BASIC has no cassette handling. You must enter the monitor
    with: CALL -151
    then when finished, re-enter BASIC with: E2B3R

    Examples:

    A machine-language program will typically be like this:
    C100R    (enter the interface)
    0300.0FFFR  (enter the load and end addresses, then load the tape)
    You start the tape.
    When the prompt returns you stop the tape.
    0300R  (run your program)


    To Load Tape Basic:
    C100R
    E000.EFFFR
    You start the tape.
    When the prompt returns you stop the tape.
    E000R  (It must say 4C - if not, your tape is no good).
    The BASIC prompt will appear
    >@


    A BASIC program is split into two areas, one for the scratch pad,
    and one for the program proper.
    In BASIC you may have to adjust the allowed memory area, such as
    LOMEM = 768
    Then, go to the monitor: CALL -151
    C100R    (enter the interface)
    00A4.00FFR 0300.0FFFR   (load the 2 parts)
    You start the tape.
    When the prompt returns you stop the tape.
    E2B3R    (back to BASIC)
    You can LIST or RUN now.


    Saving is almost the same, when you specify the address range, enter
    W instead of R. The difficulty is finding out how long your program is.

    Insert a blank tape
    C100R
    0300.0FFFW
    Quickly press Record.
    When the prompt returns, press Stop.

**********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"

#include "bus/a1bus/a1bus.h"
#include "bus/a1bus/a1cassette.h"
#include "bus/a1bus/a1cffa.h"

#include "softlist.h"

#define A1_CPU_TAG  "maincpu"
#define A1_PIA_TAG  "pia6821"
#define A1_BUS_TAG  "a1bus"
#define A1_BASICRAM_TAG "basicram"

class apple1_state : public driver_device
{
public:
	apple1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, A1_CPU_TAG),
		m_pia(*this, A1_PIA_TAG),
		m_ram(*this, RAM_TAG),
		m_basicram(*this, A1_BASICRAM_TAG),
		m_kb0(*this, "KEY0"),
		m_kb1(*this, "KEY1"),
		m_kb2(*this, "KEY2"),
		m_kb3(*this, "KEY3"),
		m_kbspecial(*this, "KBSPECIAL")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_basicram;
	required_ioport m_kb0, m_kb1, m_kb2, m_kb3, m_kbspecial;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_PALETTE_INIT(apple2);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(pia_keyboard_r);
	DECLARE_WRITE8_MEMBER(pia_display_w);
	DECLARE_WRITE_LINE_MEMBER(pia_display_gate_w);
	DECLARE_SNAPSHOT_LOAD_MEMBER( apple1 );
	TIMER_CALLBACK_MEMBER(ready_start_cb);
	TIMER_CALLBACK_MEMBER(ready_end_cb);
	TIMER_CALLBACK_MEMBER(keyboard_strobe_cb);

private:
	UINT8 *m_ram_ptr, *m_char_ptr;
	int m_ram_size, m_char_size;

	UINT8 m_vram[40*24];
	int m_cursx, m_cursy;

	bool m_reset_down;
	bool m_clear_down;

	UINT8 m_transchar;
	UINT16 m_lastports[4];

	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code, const UINT8 *textgfx_data, UINT32 textgfx_datalen);
	void poll_keyboard();

	emu_timer *m_ready_start_timer, *m_ready_end_timer, *m_kbd_strobe_timer;
};

static const UINT8 apple1_keymap[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '=', '[', ']', ';', '\'',    // KEY0
	',', '.', '/', '\\', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',    // KEY1
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\r', '_',    // KEY2
	' ', '\x1b', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              // KEY3

	')', '!', '@', '#', '$', '%', '^', '&', '*', '(', '_', '+', '[', ']', ':', '"',     // KEY0 + shift
	'<', '>', '?', '\\', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',    // KEY1 + shift
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\r', '_',    // KEY2 + shift
	' ', '\x1b', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              // KEY3 + shift

	'0', '1', '\x00', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f', '8', '9', '\x1f', '=', '\x1b', '\x1d', ';', '\'', // KEY0 + CTRL
	',', '.', '/', '\x1c', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0a', '\x0b', '\x0c',  // KEY1 + CTRL
	'\x0d', '\x0e', '\x0f', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1a', '\r', '_',  // KEY2 + CTRL
	' ', '\x1b', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                              // KEY3 + CTRL

};

// header is "LOAD:abcdDATA:" where abcd is the starting address
SNAPSHOT_LOAD_MEMBER( apple1_state, apple1 )
{
	UINT64 snapsize;
	UINT8 *data;
	UINT16 start, end;
	static const char hd1[6] = "LOAD:";
	static const char hd2[6] = "DATA:";

	// get the snapshot's size
	snapsize = image.length();

	if (snapsize < 12)
	{
		logerror("Snapshot is too short\n");
		return IMAGE_INIT_FAIL;
	}

	if ((snapsize - 12) > 65535)
	{
		logerror("Snapshot is too long\n");
		return IMAGE_INIT_FAIL;
	}

	data = (UINT8 *)image.ptr();
	if (!data)
	{
		logerror("Internal error loading snapshot\n");
		return IMAGE_INIT_FAIL;
	}

	if ((memcmp(hd1, data, 5)) || (memcmp(hd2, &data[7], 5)))
	{
		logerror("Snapshot is invalid\n");
		return IMAGE_INIT_FAIL;
	}

	start = (data[5]<<8) | data[6];
	end = (snapsize - 12) + start;

	// check if this fits in RAM; load below 0xe000 must fit in RAMSIZE,
	// load at 0xe000 must fit in 4K
	if (((start < 0xe000) && (end > (m_ram_size - 1))) || (end > 0xefff))
	{
		logerror("Snapshot can't fit in RAM\n");
		return IMAGE_INIT_FAIL;
	}

	if (start < 0xe000)
	{
		memcpy(m_ram_ptr + start, &data[12], snapsize - 12);
	}
	else if ((start >= 0xe000) && (start <= 0xefff))
	{
		memcpy(m_basicram + (start - 0xe000), &data[12], snapsize - 12);
	}
	else
	{
		logerror("Snapshot has invalid load address %04x\n", start);
		return IMAGE_INIT_FAIL;
	}

	return IMAGE_INIT_PASS;
}

void apple1_state::poll_keyboard()
{
	UINT8 special = m_kbspecial->read();
	UINT16 ports[4];
	int rawkey = 0;
	bool bKeypress = false;

	// handle special keys first:
	if (special & 0x10) // RESET
	{
		m_reset_down = true;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_pia->reset();
	}
	else if (m_reset_down)
	{
		m_reset_down = false;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}

	if (special & 0x20) // CLEAR SCREEN
	{
		m_clear_down = true;
		memset(m_vram, 0, sizeof(m_vram));
		m_cursx = m_cursy = 0;
	}
	else
	{
		m_clear_down = false;
	}

	// lower the keyboard strobe
	m_pia->ca1_w(0);

	// cache all the rows
	ports[0] = m_kb0->read();
	ports[1] = m_kb1->read();
	ports[2] = m_kb2->read();
	ports[3] = m_kb3->read();

	for (int port = 0; port < 4; port++)
	{
		UINT16 ptread = ports[port] ^ m_lastports[port];

		for (int bit = 0; bit < 16; bit++)
		{
			// key changed?
			if (ptread & (1 << bit))
			{
				// key down?
				if (ports[port] & (1 << bit))
				{
					rawkey = (port * 16) + bit;
					m_lastports[port] |= (1 << bit);
					port = 4;   // force outer for loop to quit too
					bKeypress = true;
				}
				else    // key up
				{
					m_lastports[port] &= ~(1 << bit);
				}
				break;
			}
		}
	}

	if (bKeypress)
	{
		if ((special & 0xc) != 0)
		{
			m_transchar = apple1_keymap[rawkey + (8*16)];
		}
		else if ((special & 0x3) != 0)
		{
			m_transchar = apple1_keymap[rawkey + (4*16)];
		}
		else
		{
			m_transchar = apple1_keymap[rawkey];
		}
		// pulse the strobe line
		m_pia->ca1_w(1);
	}
}

void apple1_state::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
	const UINT8 *textgfx_data, UINT32 textgfx_datalen)
{
	int x, y, i;
	const UINT8 *chardata;
	UINT16 color;
	int fg = 1, bg = 0;
	int charcode = (code & 0x1f) | (((code ^ 0x40) & 0x40) >> 1);

	/* look up the character data */
	chardata = &textgfx_data[(charcode * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 7; x++)
		{
			color = (chardata[y] & (1 << (6-x))) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

UINT32 apple1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int vramad;
	int cursor_blink = 0;
	UINT8 curs_save = 0;

	poll_keyboard();

	// the cursor 555 timer counts 0.52 of a second; the cursor is ON for
	// 2 of those counts and OFF for the last one.
	if (((int)(machine().time().as_double() / (0.52 / 3.0)) % 3) < 2)
	{
		curs_save = m_vram[(m_cursy * 40) + m_cursx];
		m_vram[(m_cursy * 40) + m_cursx] = 0x40;
		cursor_blink = 1;
	}

	for (int row = 0; row < cliprect.max_y; row += 8)
	{
		for (int col = 0; col < 40; col++)
		{
			vramad = ((row/8) * 40) + col;

			plot_text_character(bitmap, col * 14, row, 2, m_vram[vramad],
				m_char_ptr, m_char_size);
		}
	}

	if (cursor_blink)
	{
		m_vram[(m_cursy * 40) + m_cursx] = curs_save;
	}

	return 0;
}

void apple1_state::machine_start()
{
	m_ram_ptr = m_ram->pointer();
	m_ram_size = m_ram->size();
	m_char_ptr = memregion("gfx1")->base();
	m_char_size = memregion("gfx1")->bytes();

	m_reset_down = m_clear_down = false;

	m_ready_start_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(apple1_state::ready_start_cb), this));
	m_ready_end_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(apple1_state::ready_end_cb), this));
	m_kbd_strobe_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(apple1_state::keyboard_strobe_cb), this));

	// setup save states
	save_item(NAME(m_vram));
	save_item(NAME(m_cursx));
	save_item(NAME(m_cursy));
	save_item(NAME(m_reset_down));
	save_item(NAME(m_clear_down));
	save_item(NAME(m_transchar));
	save_item(NAME(m_lastports));
}

void apple1_state::machine_reset()
{
	memset(m_vram, 0, sizeof(m_vram));
	m_transchar = 0;
	m_cursx = m_cursy = 0;
	m_lastports[0] = m_lastports[1] = m_lastports[2] = m_lastports[3] = 0;
}

READ8_MEMBER(apple1_state::ram_r)
{
	if (offset < m_ram_size)
	{
		return m_ram_ptr[offset];
	}

	return 0xff;
}

WRITE8_MEMBER(apple1_state::ram_w)
{
	if (offset < m_ram_size)
	{
		m_ram_ptr[offset] = data;
	}
}

static ADDRESS_MAP_START( apple1_map, AS_PROGRAM, 8, apple1_state )
	AM_RANGE(0x0000, 0xbfff) AM_READWRITE(ram_r, ram_w)
	AM_RANGE(0xd010, 0xd013) AM_MIRROR(0x0fec) AM_DEVREADWRITE(A1_PIA_TAG, pia6821_device, read, write)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE(A1_BASICRAM_TAG)
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION(A1_CPU_TAG, 0)
ADDRESS_MAP_END

READ8_MEMBER(apple1_state::pia_keyboard_r)
{
	return m_transchar | 0x80;  // bit 7 is wired high, similar-ish to the Apple II
}

WRITE8_MEMBER(apple1_state::pia_display_w)
{
	data &= 0x7f;   // D7 is ignored by the video h/w

	// ignore characters if CLEAR is down
	if (m_clear_down)
	{
		return;
	}

	// video h/w rejects control characters except CR
	if ((data < 32) && (data != '\r'))
	{
		return;
	}

	if (data == '\r')
	{
		m_cursx = 0;
		m_cursy++;
	}
	else
	{
		m_vram[(m_cursy * 40) + m_cursx] = data;

		m_cursx++;
		if (m_cursx > 39)
		{
			m_cursx = 0;
			m_cursy++;
		}
	}

	// scroll the screen if we're at the bottom
	if (m_cursy > 23)
	{
		for (int sy = 0; sy < 23; sy++)
		{
			memcpy(&m_vram[sy * 40], &m_vram[(sy + 1) * 40], 40);
		}
		memset(&m_vram[23*40], 0, 40);
		m_cursy = 23;
	}
}

// CB2 here is connected two places: Port B bit 7 for CPU readback,
// and to the display hardware
WRITE_LINE_MEMBER(apple1_state::pia_display_gate_w)
{
	m_pia->portb_w((state << 7) ^ 0x80);

	// falling edge means start the display timer
	if (state == CLEAR_LINE)
	{
		m_ready_start_timer->adjust(machine().first_screen()->time_until_pos(m_cursy, m_cursx));
	}
}

TIMER_CALLBACK_MEMBER(apple1_state::ready_start_cb)
{
	// we're ready, pulse CB1 for 3500 nanoseconds
	m_pia->cb1_w(0);
	m_ready_end_timer->adjust(attotime::from_nsec(3500));
}

TIMER_CALLBACK_MEMBER(apple1_state::ready_end_cb)
{
	m_pia->cb1_w(1);
}

TIMER_CALLBACK_MEMBER(apple1_state::keyboard_strobe_cb)
{
	m_pia->ca1_w(0);
}

static INPUT_PORTS_START( apple1 )
	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)        PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)        PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)        PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)        PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)        PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)        PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)    PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)    PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)        PORT_CHAR('A')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)        PORT_CHAR('B')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)        PORT_CHAR('C')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)        PORT_CHAR('D')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)        PORT_CHAR('E')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)        PORT_CHAR('F')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)        PORT_CHAR('G')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)        PORT_CHAR('H')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)        PORT_CHAR('I')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)        PORT_CHAR('J')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)        PORT_CHAR('K')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)        PORT_CHAR('L')

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)        PORT_CHAR('M')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)        PORT_CHAR('N')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)        PORT_CHAR('O')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)        PORT_CHAR('P')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)        PORT_CHAR('Q')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)        PORT_CHAR('R')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)        PORT_CHAR('S')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)        PORT_CHAR('T')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)        PORT_CHAR('U')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)        PORT_CHAR('V')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)        PORT_CHAR('W')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)        PORT_CHAR('X')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)        PORT_CHAR('Y')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)        PORT_CHAR('Z')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('_')

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KBSPECIAL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Control") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
INPUT_PORTS_END

static SLOT_INTERFACE_START(apple1_cards)
	SLOT_INTERFACE("cassette", A1BUS_CASSETTE)
	SLOT_INTERFACE("cffa", A1BUS_CFFA)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apple1, apple1_state )
	MCFG_CPU_ADD(A1_CPU_TAG, M6502, 960000)        // effective CPU speed
	MCFG_CPU_PROGRAM_MAP(apple1_map)

	// video timings are identical to the Apple II, unsurprisingly
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_14_31818MHz, (65*7)*2, 0, (40*7)*2, 262, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(apple1_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD( A1_PIA_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(apple1_state, pia_keyboard_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(apple1_state, pia_display_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(apple1_state, pia_display_gate_w))

	MCFG_DEVICE_ADD(A1_BUS_TAG, A1BUS, 0)
	MCFG_A1BUS_CPU("maincpu")
	MCFG_A1BUS_SLOT_ADD(A1_BUS_TAG, "exp", apple1_cards, "cassette")

	MCFG_SNAPSHOT_ADD("snapshot", apple1_state, apple1, "snp", 0)

	MCFG_SOFTWARE_LIST_ADD("cass_list", "apple1")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,28K,32K,36K,40K,44K")
MACHINE_CONFIG_END

ROM_START(apple1)
	ROM_REGION(0x100, A1_CPU_TAG, 0)
	ROM_LOAD_NIB_HIGH("apple-a2.a2", 0x0000, 0x0100, CRC(254bfb95) SHA1(b6468b72295b7d8ac288d104d252f24de1f1d611) )
	ROM_LOAD_NIB_LOW("apple-a1.a1", 0x0000, 0x0100, CRC(434f8ce6) SHA1(9deee2d39903209b20c3fc6b58e16372f8efece1) )
	ROM_REGION(0x0200, "gfx1",0)
	ROM_LOAD("s2513.d2", 0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // apple1.vid
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   STATE         INIT     COMPANY            FULLNAME */
COMP( 1976, apple1,  0,     0,      apple1,     apple1, driver_device,  0,        "Apple Computer",    "Apple I", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
