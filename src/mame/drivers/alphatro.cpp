// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Robbbert
/***************************************************************************

    Triumph-Adler (or Royal) Alphatronic PC

    skeleton driver

    z80 + HD46505SP as a CRTC

    Other chips: 8251, 8257, 8259
    Crystals: 16MHz, 17.73447MHz
    Floppy format: 13cm, 2 sides, 40 tracks, 16 sectors, 256 bytes = 320kb.
    FDC (uPD765) is in a plug-in module, with an undumped rom.

    Has a socket for monochrome (to the standard amber monitor),
    and another for RGB. We emulate this with a configuration switch.

    This machine has 64k RAM, the ROMs being copied into RAM when
    needed.

    There are several mystery ports which might be connected to the floppy
    disk system. These are Port 30 (in and out), Port 20 (out) and certain
    bits of Port 10 (in). This unit communicates via a simple parallel
    interface.

    Port 10 bit 6 when high switches A000-DFFF over to roms that are in
    a cart which is plugged into the RomPack socket. If C3 is found at
    A000 or C000, then a jump is made to that address. It is however
    possible that one of these addresses might be for the rom in the
    floppy controller unit.


    ToDo:
    - Add fdc, then connect up software list

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "machine/ram.h"
#include "softlist.h"

#define MAIN_CLOCK XTAL_4MHz


class alphatro_state : public driver_device
{
public:
	enum
	{
		TIMER_SYSTEM
	};

	alphatro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_usart(*this, "usart")
		, m_cass(*this, "cassette")
		, m_beep(*this, "beeper")
		, m_p_ram(*this, "main_ram"),
		m_palette(*this, "palette")
	{ }

	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_INPUT_CHANGED_MEMBER(alphatro_break);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);
	DECLARE_WRITE_LINE_MEMBER(write_usart_clock);
	DECLARE_PALETTE_INIT(alphatro);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_c);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_p);
	MC6845_UPDATE_ROW(crtc_update_row);
	required_shared_ptr<UINT8> m_p_videoram;
	UINT8 *m_p_chargen;
	UINT8 m_flashcnt;

private:
	UINT8 m_timer_bit;
	UINT8 m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;
	emu_timer* m_sys_timer;
	virtual void video_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_usart;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	required_shared_ptr<UINT8> m_p_ram;
public:
	required_device<palette_device> m_palette;
};

READ8_MEMBER( alphatro_state::port10_r )
{
	//return (space.machine().device<screen_device>("screen")->vblank() ? 0x00 : 0x80);
	return m_timer_bit;
}

WRITE8_MEMBER( alphatro_state::port10_w )
{
// Bit 0 -> 0 = 40 cols; 1 = 80 cols
// Bit 1 ? each keystroke, and a lot when it scrolls
// Bit 2 ? after a cload
// Bit 3 -> cassette relay
// Bit 4 -> BEEP
// Bit 6 -> 1 = select ROM pack

	data &= 0xfe;

	m_beep->set_state(BIT(data, 4));

	m_cass->change_state( BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (BIT(data,2))
		m_cass_state = 1;
}

void alphatro_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_SYSTEM:
		m_timer_bit ^= 0x80;
		break;
	default:
		assert_always(FALSE, "Unknown id in alphatro_state::device_timer");
	}
}

WRITE_LINE_MEMBER( alphatro_state::txdata_callback )
{
	m_cass_state = state;
}

WRITE_LINE_MEMBER( alphatro_state::write_usart_clock )
{
	m_usart->write_txc(state);
	m_usart->write_rxc(state);
}

void alphatro_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

MC6845_UPDATE_ROW( alphatro_state::crtc_update_row )
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	bool palette = BIT(ioport("CONFIG")->read(), 5);
	if (y==0) m_flashcnt++;
	bool inv;
	UINT8 chr,gfx,attr,bg,fg;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x);
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		attr = m_p_videoram[mem | 0x800];
		bg = (palette) ? 8 : attr & 7; // amber or RGB
		fg = (palette) ? 0 : (attr & 0x38) >> 3;

		if BIT(attr, 7) // reverse video
		{
			inv ^= 1;
			chr &= 0x7f;
		}

		if (BIT(attr, 6) & BIT(m_flashcnt, 4)) // flashing
		{
			inv ^= 1;
		}

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | ra];

		if (inv)
		{
			UINT8 t = bg;
			bg = fg;
			fg = t;
		}

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7) ? fg : bg];
		*p++ = pens[BIT(gfx, 6) ? fg : bg];
		*p++ = pens[BIT(gfx, 5) ? fg : bg];
		*p++ = pens[BIT(gfx, 4) ? fg : bg];
		*p++ = pens[BIT(gfx, 3) ? fg : bg];
		*p++ = pens[BIT(gfx, 2) ? fg : bg];
		*p++ = pens[BIT(gfx, 1) ? fg : bg];
		*p++ = pens[BIT(gfx, 0) ? fg : bg];
	}
}

INPUT_CHANGED_MEMBER( alphatro_state::alphatro_break )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

static ADDRESS_MAP_START( alphatro_map, AS_PROGRAM, 8, alphatro_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( alphatro_io, AS_IO, 8, alphatro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x10, 0x10) AM_READWRITE(port10_r,port10_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("X0")
	AM_RANGE(0x21, 0x21) AM_READ_PORT("X1")
	AM_RANGE(0x22, 0x22) AM_READ_PORT("X2")
	AM_RANGE(0x23, 0x23) AM_READ_PORT("X3")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("X4")
	AM_RANGE(0x25, 0x25) AM_READ_PORT("X5")
	AM_RANGE(0x26, 0x26) AM_READ_PORT("X6")
	AM_RANGE(0x27, 0x27) AM_READ_PORT("X7")
	AM_RANGE(0x28, 0x28) AM_READ_PORT("X8")
	AM_RANGE(0x29, 0x29) AM_READ_PORT("X9")
	AM_RANGE(0x2a, 0x2a) AM_READ_PORT("XA")
	AM_RANGE(0x2b, 0x2b) AM_READ_PORT("XB")
	//AM_RANGE(0x30, 0x30) unknown
	// USART for cassette reading and writing
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("usart", i8251_device, data_r, data_w)
	AM_RANGE(0x41, 0x41) AM_DEVREADWRITE("usart", i8251_device, status_r, control_w)
	// CRTC - HD46505 / HD6845SP
	AM_RANGE(0x50, 0x50) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x51, 0x51) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( alphatro )
	PORT_START("X0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("X1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad - /") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ASTERISK) // keypad equals
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad + x") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("X2")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("X3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("X4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("X5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("X6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xa3)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_DEL) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("X8")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("X9")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("XA")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Clear Home") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)

	PORT_START("XB")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)

	PORT_START("other")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF,alphatro_state,alphatro_break,0)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x20, 0x00, "Monitor")
	PORT_CONFSETTING(    0x00, "RGB")
	PORT_CONFSETTING(    0x20, "Amber")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( alphatro )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 4 )
GFXDECODE_END

void alphatro_state::machine_start()
{
	m_sys_timer = timer_alloc(TIMER_SYSTEM);
}

void alphatro_state::machine_reset()
{
	// do what the IPL does
	UINT8* ROM = memregion("roms")->base();
	m_maincpu->set_pc(0xe000);
	// If BASIC is missing then it boots into the Monitor
	memcpy(m_p_ram, ROM, 0x6000); // Copy BASIC to RAM
	// Top half of this rom has keyboard tables and appears to be unused
	memcpy(m_p_ram+0xe000, ROM+0xe000, 0x1000); // copy BIOS + Monitor to RAM

	// probably not correct, exact meaning of port is unknown, vblank/vsync is too slow.
	m_sys_timer->adjust(attotime::from_usec(10),0,attotime::from_usec(10));
	m_timer_bit = 0;
	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
	m_cass_state = 0;
	m_cassold = 0;
	m_usart->write_rxd(0);
	m_beep->set_state(0);
}

PALETTE_INIT_MEMBER(alphatro_state, alphatro)
{
	// RGB colours
	palette.set_pen_color(0, 0x00, 0x00, 0x00);
	palette.set_pen_color(1, 0x00, 0x00, 0xff);
	palette.set_pen_color(2, 0xff, 0x00, 0x00);
	palette.set_pen_color(3, 0xff, 0x00, 0xff);
	palette.set_pen_color(4, 0x00, 0xff, 0x00);
	palette.set_pen_color(5, 0x00, 0xff, 0xff);
	palette.set_pen_color(6, 0xff, 0xff, 0x00);
	palette.set_pen_color(7, 0xff, 0xff, 0xff);
	// Amber
	palette.set_pen_color(8, 0xf7, 0xaa, 0x00);
}


TIMER_DEVICE_CALLBACK_MEMBER(alphatro_state::timer_c)
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(alphatro_state::timer_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_usart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

static MACHINE_CONFIG_START( alphatro, alphatro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(alphatro_map)
	MCFG_CPU_IO_MAP(alphatro_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not correct
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", alphatro)
	MCFG_PALETTE_ADD("palette", 9) // 8 colours + amber
	MCFG_PALETTE_INIT_OWNER(alphatro_state, alphatro)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 950) /* piezo-device needs to be measured */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_12_288MHz / 8) // clk unknown
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(alphatro_state, crtc_update_row)

	MCFG_DEVICE_ADD("usart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE(alphatro_state, txdata_callback))

	MCFG_DEVICE_ADD("usart_clock", CLOCK, 19218) // 19218 to load a real tape, 19222 to load a tape made by this driver
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(alphatro_state, write_usart_clock))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("alphatro_cass")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_c", alphatro_state, timer_c, attotime::from_hz(4800))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_p", alphatro_state, timer_p, attotime::from_hz(40000))

	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "alphatro_flop")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( alphatro )
	ROM_REGION( 0x10000, "roms", ROMREGION_ERASE00)
	ROM_LOAD( "613256.ic-1058", 0x0000, 0x6000, CRC(ceea4cb3) SHA1(b332dea0a2d3bb2978b8422eb0723960388bb467) )
	ROM_LOAD( "2764.ic-1038",   0xe000, 0x2000, CRC(e337db3b) SHA1(6010bade6a21975636383179903b58a4ca415e49) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )
	ROM_LOAD( "ipl.bin",        0x8000, 0x2000, NO_DUMP  )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2732.ic-1067",   0x0000, 0x1000, CRC(61f38814) SHA1(35ba31c58a10d5bd1bdb202717792ca021dbe1a8) )
ROM_END

COMP( 1983, alphatro,   0,       0,    alphatro,   alphatro, driver_device,  0,  "Triumph-Adler", "Alphatronic PC", MACHINE_NOT_WORKING )
