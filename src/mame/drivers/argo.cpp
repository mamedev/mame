// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Argo

    16/03/2011 Skeleton driver.

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
    - Cassette UART on ports C1 and C3.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class argo_state : public driver_device
{
public:
	enum
	{
		TIMER_BOOT
	};

	argo_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(argo_videoram_w);
	DECLARE_READ8_MEMBER(argo_io_r);
	DECLARE_WRITE8_MEMBER(argo_io_w);
	required_shared_ptr<UINT8> m_p_videoram;
	const UINT8 *m_p_chargen;
	UINT8 m_framecnt;
	UINT8 m_cursor_pos[3];
	UINT8 m_p_cursor_pos;
	bool m_ram_ctrl;
	UINT8 m_scroll_ctrl;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DRIVER_INIT(argo);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

// write to videoram if following 'out b9,61' otherwise write to the unknown 'extra' ram
WRITE8_MEMBER(argo_state::argo_videoram_w)
{
	UINT8 *RAM;
	if (m_ram_ctrl)
		RAM = memregion("videoram")->base();
	else
		RAM = memregion("extraram")->base();

	RAM[offset] = data;
}

READ8_MEMBER(argo_state::argo_io_r)
{
	UINT8 low_io = offset;

	switch (low_io)
	{
	case 0xA1: // keyboard
		char kbdrow[6];
		sprintf(kbdrow,"X%X",offset>>8);
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
	UINT8 low_io = offset;

	switch (low_io)
	{
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
			UINT8 *RAM = memregion("videoram")->base();
			m_scroll_ctrl = 0;
			memcpy(RAM, RAM+80, 24*80);
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



static ADDRESS_MAP_START(argo_mem, AS_PROGRAM, 8, argo_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("boot")
	AM_RANGE(0x0800, 0xf7af) AM_RAM
	AM_RANGE(0xf7b0, 0xf7ff) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_WRITE(argo_videoram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(argo_io, AS_IO, 8, argo_state)
	AM_RANGE(0x0000, 0xFFFF) AM_READWRITE(argo_io_r,argo_io_w)
ADDRESS_MAP_END

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
		assert_always(FALSE, "Unknown id in argo_state::device_timer");
	}
}

void argo_state::machine_reset()
{
	membank("boot")->set_entry(1);
	timer_set(attotime::from_usec(5), TIMER_BOOT);
}

DRIVER_INIT_MEMBER(argo_state,argo)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xf800);
}

void argo_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 argo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;
	UINT8 *p_vram = m_p_videoram;

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

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
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
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

static MACHINE_CONFIG_START( argo, argo_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3500000)
	MCFG_CPU_PROGRAM_MAP(argo_mem)
	MCFG_CPU_IO_MAP(argo_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(argo_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
MACHINE_CONFIG_END

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

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1986, argo,  0,      0,       argo,     argo, argo_state,    argo,     "<unknown>",   "Argo", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
