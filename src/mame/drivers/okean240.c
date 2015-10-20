// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Okeah-240 (Ocean-240)

        28/12/2011 Skeleton driver.

        Info from EMU80:

intctl : K580wn59
  irq[0]=kbd.irq2
  irq[1]=kbd.irq
  irq[4]=tim.out[0]

ppa40 : K580ww55
  portA=kbd.pressed.mask
  portB[2]=cas.playback
  portB[5]=kbd.key[58]
  portB[6]=kbd.key[59]
  portC[0-3]=kbd.pressed.row
  portC[4]=kbd.ack

ppaC0 : K580ww55
  portA=vid.scroll.y
  portB[0-3]=mem.frame[0].page
  portB[1-3]=mem.frame[1].page
  portB[4-5]=mm.page
  portC=vid.scroll.x


NOTE ABOUT THE TEST ROM (okean240t):
- You need to press a key every so often.


ToDo:
- Add devices
- Find out if any unconnected keyboard entries are real keys
- Colours?
- Sound? (perhaps port E4 bit 3)
- Add disks
- Cassette?
- Add memory banking (perhaps port C1)

Usage of terminal:
- okean240 - the keyboard
- okean240a - not used
- okean240t - the keyboard & screen

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/keyboard.h"
#include "machine/terminal.h"

#define KEYBOARD_TAG "keyboard"
#define TERMINAL_TAG "terminal"

class okean240_state : public driver_device
{
public:
	enum
	{
		TIMER_OKEAN_BOOT
	};

	okean240_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_term_data(0),
		m_j(0),
		m_scroll(0),
		m_p_videoram(*this, "p_videoram"),
		m_io_modifiers(*this, "MODIFIERS"),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER(okean240_kbd_status_r);
	DECLARE_READ8_MEMBER(okean240a_kbd_status_r);
	DECLARE_READ8_MEMBER(term_status_r);
	DECLARE_READ8_MEMBER(term_r);
	DECLARE_READ8_MEMBER(okean240_keyboard_r);
	DECLARE_WRITE8_MEMBER(okean240_keyboard_w);
	DECLARE_READ8_MEMBER(okean240a_keyboard_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE8_MEMBER(scroll_w);
	UINT8 m_term_data;
	UINT8 m_j;
	UINT8 m_scroll;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_DRIVER_INIT(okean240);
	UINT32 screen_update_okean240(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	optional_ioport m_io_modifiers;
	ioport_port *m_io_port[11];
	required_device<cpu_device> m_maincpu;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

// okean240 requires bit 4 to change
READ8_MEMBER( okean240_state::okean240_kbd_status_r )
{
	if (m_term_data)
		return (machine().rand() & 0x10) | 2;
	else
		return machine().rand() & 0x10;
}

// see if a key is pressed and indicate status
READ8_MEMBER( okean240_state::okean240a_kbd_status_r )
{
	UINT8 i,j;

	for (i = 0; i < 11; i++)
	{
		j = m_io_port[i]->read();
		if (j)
			return (machine().rand() & 0x10) | 2;
	}
	m_j = 0;
	return machine().rand() & 0x10;
}

// for test rom
READ8_MEMBER( okean240_state::term_status_r )
{
	return (m_term_data) ? 3 : 1;
}

READ8_MEMBER( okean240_state::okean240_keyboard_r )
{
	if (offset == 0) // port 40 (get ascii key value)
		return term_r(space, offset);
	else
	if (offset == 1) // port 41 bit 1 (test rom status bit)
	{
		return (machine().rand() & 2);
	}
	else // port 42 (not used)
		return 0;
}

READ8_MEMBER( okean240_state::okean240a_keyboard_r )
{
	UINT8 i,j;

	if (offset == 0) // port 40 (get a column)
	{
		for (i = 0; i < 11; i++)
		{
			j = m_io_port[i]->read();
			if (j)
			{
				if (j==m_j) return 0;
				m_j=j;
				return j;
			}
		}
		m_j=0;
		return 0;
	}
	else
	if (offset == 1) // port 41 bits 6&7 (modifier keys), and bit 1 (test rom status bit)
	{
		return (machine().rand() & 2) | m_io_modifiers->read();
	}
	else // port 42 (get a row)
	{
		for (i = 0; i < 11; i++)
		{
			if (m_io_port[i]->read() )
				return i;
		}
	}
	return 0;
}

// This is a keyboard acknowledge pulse, it goes high then
// straightaway low, if reading port 40 indicates a key is pressed.
WRITE8_MEMBER( okean240_state::okean240_keyboard_w )
{
// okean240: port 42 bit 7
// okean240a: port 42 bit 4
}

// for test rom
READ8_MEMBER( okean240_state::term_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( okean240_state::scroll_w )
{
	m_scroll = data;
}

static ADDRESS_MAP_START(okean240_mem, AS_PROGRAM, 8, okean240_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("boot")
	AM_RANGE(0x0800, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(okean240_io, AS_IO, 8, okean240_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x42) AM_READWRITE(okean240_keyboard_r,okean240_keyboard_w)
	AM_RANGE(0x80, 0x80) AM_READ(okean240_kbd_status_r)
	AM_RANGE(0xa0, 0xa0) AM_READ(term_r)
	AM_RANGE(0xa1, 0xa1) AM_READ(term_status_r)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(scroll_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(okean240a_io, AS_IO, 8, okean240_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x42) AM_READWRITE(okean240a_keyboard_r,okean240_keyboard_w)
	AM_RANGE(0x80, 0x80) AM_READ(okean240a_kbd_status_r)
	AM_RANGE(0xa0, 0xa0) AM_READ(term_r)
	AM_RANGE(0xa1, 0xa1) AM_READ(term_status_r)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(scroll_w)
	// AM_RANGE(0x00, 0x1f)=ppa00.data
	// AM_RANGE(0x20, 0x23)=dsk.data
	// AM_RANGE(0x24, 0x24)=dsk.wait
	// AM_RANGE(0x25, 0x25)=dskctl.data
	// AM_RANGE(0x40, 0x5f)=ppa40.data
	// AM_RANGE(0x60, 0x7f)=tim.data
	// AM_RANGE(0x80, 0x81)=intctl.data
	// AM_RANGE(0xa0, 0xa1)=comport.data
	// AM_RANGE(0xc0, 0xdf)=ppaC0.data
	// AM_RANGE(0xe0, 0xff)=ppaE0.data
ADDRESS_MAP_END

static ADDRESS_MAP_START(okean240t_io, AS_IO, 8, okean240_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x42) AM_READWRITE(okean240_keyboard_r,okean240_keyboard_w)
	AM_RANGE(0x80, 0x80) AM_READ(okean240_kbd_status_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xa0, 0xa0) AM_READ(term_r)
	AM_RANGE(0xa1, 0xa1) AM_READ(term_status_r)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(scroll_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( okean240 )
INPUT_PORTS_END


static INPUT_PORTS_START( okean240a )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) // comma
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) // minus
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("numdel") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("numenter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)        PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)        PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)        PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // null
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num3") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //9E
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)        PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)        PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("num6") PORT_CODE(KEYCODE_6_PAD)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //81
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)        PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)        PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)        PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) // plus
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //7F
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) //03

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //86
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)        PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)        PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)        PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //99
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) //8B

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)        PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)        PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)        PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) //84
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //92 prints # and line feed
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)        PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)        PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)        PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) //98
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) //85 cr and line feed
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_END)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) //83 cancel input
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)        PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)        PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)        PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)        PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)        PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) //93
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)        PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)        PORT_CHAR('0')

	PORT_START("XA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)        PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)        PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)        PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9)      PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("MODIFIERS")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


void okean240_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_OKEAN_BOOT:
		/* after the first 6 bytes have been read from ROM, switch the ram back in */
		membank("boot")->set_entry(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in okean240_state::device_timer");
	}
}


void okean240_state::machine_start()
{
	char kbdrow[6];

	for (int i = 0; i < 11; i++)
	{
		sprintf(kbdrow,"X%X",i);
		m_io_port[i] = ioport(kbdrow);
	}
}


void okean240_state::machine_reset()
{
	timer_set(attotime::from_usec(10), TIMER_OKEAN_BOOT);
	membank("boot")->set_entry(1);
	m_term_data = 0;
	m_j = 0;
	m_scroll = 0;
}

WRITE8_MEMBER( okean240_state::kbd_put )
{
	m_term_data = data;
}

DRIVER_INIT_MEMBER(okean240_state,okean240)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xe000);
}

void okean240_state::video_start()
{
}

UINT32 okean240_state::screen_update_okean240(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 gfx,ma; // ma must be 8bit
	UINT16 x,y;

	for (y = 0; y < 256; y++)
	{
		ma = y + m_scroll;
		UINT16 *p = &bitmap.pix16(y);

		for (x = 0; x < 0x4000; x+=0x200)
		{
			gfx = m_p_videoram[x|ma] | m_p_videoram[x|ma|0x100];

			/* Display a scanline of a character */
			*p++ = BIT(gfx, 0);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 7);
		}
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout okean240_charlayout =
{
	8, 7,                   /* 8 x 7 characters */
	160,                    /* 160 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8 },
	8*7                 /* every char takes 7 bytes */
};

static GFXDECODE_START( okean240 )
	GFXDECODE_ENTRY( "maincpu", 0xec08, okean240_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( okean240a )
	GFXDECODE_ENTRY( "maincpu", 0xef63, okean240_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( okean240t, okean240_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_12MHz / 6)
	MCFG_CPU_PROGRAM_MAP(okean240_mem)
	MCFG_CPU_IO_MAP(okean240t_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 255)
	MCFG_SCREEN_UPDATE_DRIVER(okean240_state, screen_update_okean240)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(okean240_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( okean240a, okean240t )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(okean240a_io)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", okean240a)
	MCFG_DEVICE_REMOVE(TERMINAL_TAG)
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(okean240_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( okean240, okean240t )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(okean240_io)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", okean240)
	MCFG_DEVICE_REMOVE(TERMINAL_TAG)
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(okean240_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( okean240 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monitor.bin", 0xe000, 0x2000, CRC(587799bc) SHA1(1f677afa96722ca4ed2643eaca243548845fc854) )
	ROM_LOAD( "cpm80.bin",   0xc000, 0x2000, CRC(7378e4f9) SHA1(c3c06c6f2e953546452ca6f82140a79d0e4953b4) )
ROM_END

ROM_START( okean240a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fddmonitor.bin", 0xe000, 0x2000, CRC(bcac5ca0) SHA1(602ab824704d3d5d07b3787f6227ff903c33c9d5) )
	ROM_LOAD( "fddcpm80.bin",   0xc000, 0x2000, CRC(b89a7e16) SHA1(b8f937c04f430be18e48f296ed3ef37194171204) )
ROM_END

ROM_START( okean240t )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "test.bin",    0xe000, 0x0800, CRC(e9e2b7b9) SHA1(e4e0b6984a2514b6ba3e97500d487ea1a68b7577) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT      INIT         COMPANY     FULLNAME       FLAGS */
COMP( 1986, okean240,  0,       0,   okean240,  okean240, okean240_state,  okean240,  "<unknown>", "Okeah-240", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1986, okean240a, okean240,0,   okean240a, okean240a, okean240_state, okean240,  "<unknown>", "Ocean-240 with fdd", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1986, okean240t, okean240,0,   okean240t, okean240, okean240_state,  okean240,  "<unknown>", "Ocean-240 Test Rom", MACHINE_NO_SOUND)
