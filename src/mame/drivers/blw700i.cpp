// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Brother LW-700i and friends word processor/typewriters

    Preliminary driver by R. Belmont

    Main CPU: Hitachi H8/3003
    FDC: HD63266F (uPD765 derivative)
    256KiB RAM
    Dot-matrix LCD (480x128)

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h83003.h"
#include "machine/nvram.h"
#include "machine/at28c16.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"

class lw700i_state : public driver_device
{
public:
	lw700i_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mainram(*this, "mainram"),
			m_screen(*this, "screen"),
			m_keyboard(*this, "X%u", 0)
	{ }

	void lw700i(machine_config &config);

	DECLARE_READ16_MEMBER(status_r) { return 0x8080; }  // "ready"
	DECLARE_WRITE16_MEMBER(data_w) { }

	DECLARE_READ8_MEMBER(p7_r);
	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pb_w);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void io_map(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(vbl_interrupt);

	// devices
	required_device<h83003_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_device<screen_device> m_screen;
	required_ioport_array<9> m_keyboard;

	// driver_device overrides
	virtual void video_start() override;

	uint8_t m_keyrow;
};

READ8_MEMBER(lw700i_state::p7_r)
{
	//("Read P7 (PC=%x)\n", m_maincpu->pc());
	// must be non-zero; f0 = French, fe = German, ff = English
	if (m_keyrow == 0xf)
	{
		return 0xff;
	}

	if (m_keyrow < 9)
	{
		return m_keyboard[m_keyrow]->read();
	}

	return 0xff;
}

READ8_MEMBER(lw700i_state::pb_r)
{
	return 0;
}

WRITE8_MEMBER(lw700i_state::pb_w)
{
	//printf("%x to keyboard row\n", data);
	m_keyrow = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(lw700i_state::vbl_interrupt)
{
	int scanline = m_screen->vpos();

	if (scanline == 1)
	{
		m_maincpu->set_input_line(5, ASSERT_LINE);
		// not sure where this is coming from, fix hang
		m_maincpu->space(0).write_byte(0xffffff05, 0);
	}
	else if (scanline == 2)
	{
		m_maincpu->set_input_line(5, CLEAR_LINE);
	}
}

void lw700i_state::machine_reset()
{
}

void lw700i_state::machine_start()
{
}

void lw700i_state::video_start()
{
}

uint32_t lw700i_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	static const uint32_t palette[2] = { 0xffffff, 0 };
	uint8_t *pVRAM = (uint8_t *)m_mainram.target();

	pVRAM += 0x3e200;

	for (y = 0; y < 128; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 480/8; x++)
		{
			pixels = pVRAM[(y * (480/8)) + (BYTE_XOR_BE(x))];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
	}

	return 0;
}

void lw700i_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0x0000);
	map(0x600000, 0x63ffff).ram().share("mainram"); // 256K of main RAM
	map(0xe00000, 0xe00001).rw(FUNC(lw700i_state::status_r), FUNC(lw700i_state::data_w));
	map(0xf00048, 0xf00049).ram();
}

void lw700i_state::io_map(address_map &map)
{
	map(h83003_device::PORT_7, h83003_device::PORT_7).r(FUNC(lw700i_state::p7_r));
	map(h83003_device::PORT_B, h83003_device::PORT_B).rw(FUNC(lw700i_state::pb_r), FUNC(lw700i_state::pb_w));
}

// row 0:   | 4 | 3 | W | E | D | X | ? | Enter? |
// row 1:   | 5 | 6 | R | T | C | F | ? | DArr |
// row 2:   | 8 | 7 | Y | H | G | V | ? | ? |
// row 3:   | 1 | 2 | Q | Z | A | S | ? | Shift Lock? |
// row 4:   | 9 | J | I | U | B | N | ? | RArr |
// row 5:   | - | 0 | P | O | M | , | ? | Menu |
// row 6:   | ? | ; |2/3| | |LAr|UAr| ? | ? |
// row 7:   | ? | ? |ENT|BkS| ? | ? | ? | Shift? |
// row 8:   |3/4| L | = | K | . |1/2| * | ? |

static INPUT_PORTS_START( lw700i )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('@')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('/')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR(0xa3)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('\'')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR(0x2021)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)  PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('-') PORT_CHAR('?')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Menu")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x2154) PORT_CHAR(0x2153)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('|') PORT_CHAR('$')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)     PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xbe) PORT_CHAR(0xbc)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(0xbd) PORT_CHAR('%')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNKNOWN)

INPUT_PORTS_END

MACHINE_CONFIG_START(lw700i_state::lw700i)
	MCFG_DEVICE_ADD("maincpu", H83003, XTAL(16'000'000))
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(io_map)
	TIMER(config, "scantimer").configure_scanline(FUNC(lw700i_state::vbl_interrupt), "screen", 0, 1);

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(lw700i_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 480, 0, 128)
MACHINE_CONFIG_END

ROM_START(blw700i)
	ROM_REGION(0x200000, "maincpu", 0)      /* H8/3003 program ROM */
	ROM_LOAD16_WORD_SWAP( "mx24969b.bin", 0x000000, 0x200000, CRC(78d88d04) SHA1(3cda632c7190257abd20e121575767e8e9a18b1c) )
ROM_END

SYST( 1995, blw700i,    0, 0, lw700i, lw700i, lw700i_state, empty_init, "Brother", "LW-700i", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
