// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************

Argo

2011-03-16 Skeleton driver.

Some info obtained from EMU-80.
There are no manuals, diagrams, or anything else available afaik.
The entire driver is guesswork.

The monitor will only allow certain characters to be typed, thus the
modifier keys appear to do nothing. There is no need to use the enter
key; using spacebar and the correct parameters is enough.

Commands: same as UNIOR

ToDo:
- Add devices
- There is no obvious evidence of sound.
- Cassette
-- no info available but seems to be much the same as Unior, so code copied over
-- clock not hooked up, don't know where it comes from
-- unable to test as the i8251 device needs work to support syndet

****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "imagedev/cassette.h"
//#include "sound/spkrdev.h"
#include "speaker.h"
#include "screen.h"


class argo_state : public driver_device
{
public:
	argo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_uart(*this, "uart")
		, m_cass(*this, "cassette")
	{ }

	void argo(machine_config &config);

	void init_argo();

private:
	enum
	{
		TIMER_BOOT
	};

	DECLARE_WRITE8_MEMBER(argo_videoram_w);
	DECLARE_READ8_MEMBER(argo_io_r);
	DECLARE_WRITE8_MEMBER(argo_io_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;
	uint8_t m_framecnt;
	uint8_t m_cursor_pos[3];
	uint8_t m_p_cursor_pos;
	bool m_ram_ctrl;
	uint8_t m_scroll_ctrl;
	bool m_txd, m_txe;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

// write to videoram if following 'out b9,61' otherwise write to the unknown 'extra' ram
WRITE8_MEMBER(argo_state::argo_videoram_w)
{
	uint8_t *RAM;
	if (m_ram_ctrl)
		RAM = memregion("videoram")->base();
	else
		RAM = memregion("extraram")->base();

	RAM[offset] = data;
}

READ8_MEMBER(argo_state::argo_io_r)
{
	uint8_t low_io = offset;

	switch (low_io)
	{
	case 0xC1: // uart data
		return m_uart->data_r();

	case 0xC3: // uart status
		return m_uart->status_r();

	case 0xA1: // keyboard
		char kbdrow[6];
		sprintf(kbdrow,"X%X",uint8_t(offset>>8));
		return ioport(kbdrow)->read();

	case 0xE8: // wants bit 4 low then high
		return m_framecnt << 4;

	default:
		logerror("%s: In %X\n",machine().describe_context(),low_io);
		return 0xff;
	}
}

WRITE8_MEMBER(argo_state::argo_io_w)
{
	uint8_t low_io = offset;

	switch (low_io)
	{
	case 0xC1: // uart data
		m_uart->data_w(data);
		break;

	case 0xC3: // uart control
		m_uart->control_w(data);
		break;

	case 0xA1: // prep scroll step 1
		m_scroll_ctrl = (data == 0x61);
		break;

	case 0xA9: // prep scroll step 2
		if ((m_scroll_ctrl == 1) && (data == 0x61))
			m_scroll_ctrl++;
		break;

	case 0xE8: // hardware scroll - we should use ports E0,E1,E2,E3
		if ((m_scroll_ctrl == 2) & (data == 0xe3))
		{
			uint8_t *RAM = memregion("videoram")->base();
			m_scroll_ctrl = 0;
			memmove(RAM, RAM+80, 24*80);
		}
		break;

	case 0xC4: // prepare to receive cursor position
		m_p_cursor_pos = (data == 0x80);
		break;

	case 0xC0: // store cursor position if it followed 'out c4,80'
		if (m_p_cursor_pos)
		{
			m_cursor_pos[m_p_cursor_pos]=data;
			if (m_p_cursor_pos == 1) m_p_cursor_pos++;
		}
		break;

	case 0xB9: // switch between videoram and extraram
		m_ram_ctrl = (data == 0x61);
		break;

	default:
		logerror("%s: Out %X,%X\n",machine().describe_context(),low_io,data);
	}
}

// Untested. This needs to be driven by a 2400Hz clock.
WRITE_LINE_MEMBER(argo_state::ctc_z1_w)
{
	// write
	if (m_txe)
		m_cass->output(1);
	else
		m_cass->output((m_txd ^ state) ? 1 : 0);

	m_uart->write_txc(state);

	// read - untested
	m_uart->write_rxd((m_cass->input() > 0.04) ? 1 : 0);
	m_uart->write_rxc(state);
}


void argo_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankrw("boot");
	map(0x0800, 0xf7af).ram();
	map(0xf7b0, 0xf7ff).ram().share("videoram");
	map(0xf800, 0xffff).rom().w(FUNC(argo_state::argo_videoram_w));
}

void argo_state::io_map(address_map &map)
{
	map(0x0000, 0xFFFF).rw(FUNC(argo_state::argo_io_r), FUNC(argo_state::argo_io_w));
}

/* Input ports */
static INPUT_PORTS_START( argo ) // Keyboard was worked out by trial & error;'F' keys produce symbols
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) // J
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // nothing
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LShift")      PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR('7')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR('9')
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")         PORT_CODE(KEYCODE_ESC)        PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RShift")      PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")          PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR('0')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+")           PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)          PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")          PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR('1')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR('2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR('3')
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) // A
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)          PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)          PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR('4')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR('5')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR('6')
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) // B
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3)          PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)          PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) // K
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) // H
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) // I
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) // C
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4)          PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS")          PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) // G
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) // F
	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5)          PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)          PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")       PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*")           PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) // D
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6)          PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")          PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\')
	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) // E
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7)          PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("`")           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')
	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8)          PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\'")          PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')
	PORT_START("XA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)          PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)          PORT_CHAR('L') // if L is the first character, computer hangs
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_START("XB")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void argo_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BOOT:
		/* after the first 4 bytes have been read from ROM, switch the ram back in */
		membank("boot")->set_entry(0);
		break;
	default:
		assert_always(false, "Unknown id in argo_state::device_timer");
	}
}

void argo_state::machine_reset()
{
	membank("boot")->set_entry(1);
	timer_set(attotime::from_usec(5), TIMER_BOOT);
}

void argo_state::init_argo()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xf800);
}

uint32_t argo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;
	uint8_t *p_vram = m_p_videoram;

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint32_t *p = &bitmap.pix32(sy++);

			for (x = 1; x < 81; x++) // align x to the cursor position numbers
			{
				gfx = 0;

				if (ra < 9)
				{
					chr = p_vram[x+ma-1];

					/* Take care of flashing characters */
					//if ((chr & 0x80) && (m_framecnt & 0x08))
					//  chr = 0x20;

					chr &= 0x7f;

					gfx = m_p_chargen[(chr<<4) | ra ];
				}
				else
				// display cursor if at cursor position and flash on
				if ((x==m_cursor_pos[1]) && (y==m_cursor_pos[2]) && (m_framecnt & 0x08))
					gfx = 0xff;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 6) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 5) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 4) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 3) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 2) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 1) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 0) ? rgb_t::white() : rgb_t::black();
			}
		}

		if (y)
			ma+=80;
		else
		{
			ma=0;
			p_vram = memregion("videoram")->base();
		}
	}
	return 0;
}

void argo_state::argo(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &argo_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &argo_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(argo_state::screen_update));
	screen.set_size(640, 250);
	screen.set_visarea_full();

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	//SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.15);

	/* Devices */
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_txd = state; });
	m_uart->txempty_handler().set([this] (bool state) { m_txe = state; });
}

/* ROM definition */
ROM_START( argo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "argo.rom", 0xf800, 0x0800, CRC(4c4c045b) SHA1(be2b97728cc190d4a8bd27262ba9423f252d31a3) )

	ROM_REGION( 0x0800, "videoram", ROMREGION_ERASEFF )
	ROM_REGION( 0x0800, "extraram", ROMREGION_ERASEFF ) // no idea what this is for

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY      FULLNAME  FLAGS */
COMP( 1986, argo, 0,      0,      argo,    argo,  argo_state, init_argo, "<unknown>", "Argo",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
