/***************************************************************************

    Triumph-Adler's Alphatronic PC

    skeleton driver

    z80 + HD46505 as a CRTC

        Has a socket for monochrome (to the standard amber monitor),
        and another for RGB. We emulate this with a configuration switch.

        The Z80 must start at E000, but unlike other designs (Super80 for
        example) which causes the ROMs to appear everywhere during boot,
        this one (it seems) holds the data bus low until E000 is reached.
        This kind of boot still needs to be emulated.

        This machine has 64k RAM, the ROMs being copied into RAM when
        needed.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "sound/wave.h"
#include "machine/ram.h"

#define MAIN_CLOCK XTAL_4MHz

#define VIDEO_START_MEMBER(name) void name::video_start()
#define SCREEN_UPDATE16_MEMBER(name) UINT32 name::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)

class alphatro_state : public driver_device
{
public:
	alphatro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_crtc(*this, "crtc"),
	m_usart(*this, "usart"),
	m_cass(*this, CASSETTE_TAG),
	m_beep(*this, BEEPER_TAG),
	m_p_ram(*this, "p_ram"),
	m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_usart;
	required_device<cassette_image_device> m_cass;
	required_device<device_t> m_beep;

	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_INPUT_CHANGED_MEMBER(alphatro_break);
	DECLARE_READ_LINE_MEMBER(rxdata_callback);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);

	required_shared_ptr<UINT8> m_p_ram;
	required_shared_ptr<UINT8> m_p_videoram;
	emu_timer* m_sys_timer;
	emu_timer* m_serial_timer;  // for the usart
	UINT8 *m_p_chargen;
	virtual void video_start();
	virtual void machine_start();
	virtual void machine_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id SYSTEM_TIMER = 0;
	static const device_timer_id SERIAL_TIMER = 1;
	UINT8 m_timer_bit;
};

static TIMER_CALLBACK( alphatro_beepoff )
{
	alphatro_state *state = machine.driver_data<alphatro_state>();
	beep_set_state(state->m_beep, 0);
}

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

	UINT16 length = 0;
	data &= 0xfe;

	if (data == 0x10)
		length = 400; // BEEP command

	if (length)
	{
		machine().scheduler().timer_set(attotime::from_msec(length), FUNC(alphatro_beepoff));
		beep_set_state(m_beep, 1);
	}

	m_cass->change_state( BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void alphatro_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case SYSTEM_TIMER:
		m_timer_bit ^= 0x80;
		break;
	case SERIAL_TIMER:
		m_usart->transmit_clock();
		m_usart->receive_clock();
		break;
	}
}

READ_LINE_MEMBER( alphatro_state::rxdata_callback )
{
	return (m_cass->input() > -0.1) ? 1 : 0;
}

WRITE_LINE_MEMBER( alphatro_state::txdata_callback )
{
	m_cass->output( (state) ? 0.8 : -0.8);
}

VIDEO_START_MEMBER( alphatro_state )
{
	m_p_chargen = memregion("chargen")->base();
}

static MC6845_UPDATE_ROW( alphatro_update_row )
{
	alphatro_state *state = device->machine().driver_data<alphatro_state>();
	const rgb_t *pens = palette_entry_list_raw(bitmap.palette());
	bool palette = BIT(state->ioport("CONFIG")->read(), 5);
	UINT8 chr,gfx,attr,fg,inv;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x) ? 0xff : 0;
		mem = (ma + x) & 0x7ff;
		chr = state->m_p_videoram[mem];
		attr = state->m_p_videoram[mem | 0x800];
		fg = (palette) ? 8 : attr & 7; // amber or RGB

		if (BIT(attr, 7)) // reverse video for fkey labels
		{
			inv ^= 0xff;
			chr &= 0x7f;
		}

		/* get pattern of pixels for that character scanline */
		gfx = state->m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7) ? fg : 0];
		*p++ = pens[BIT(gfx, 6) ? fg : 0];
		*p++ = pens[BIT(gfx, 5) ? fg : 0];
		*p++ = pens[BIT(gfx, 4) ? fg : 0];
		*p++ = pens[BIT(gfx, 3) ? fg : 0];
		*p++ = pens[BIT(gfx, 2) ? fg : 0];
		*p++ = pens[BIT(gfx, 1) ? fg : 0];
		*p++ = pens[BIT(gfx, 0) ? fg : 0];
	}
}

INPUT_CHANGED_MEMBER( alphatro_state::alphatro_break )
{
	device_set_input_line(m_maincpu, INPUT_LINE_IRQ0, HOLD_LINE);
}

static ADDRESS_MAP_START( alphatro_map, AS_PROGRAM, 8, alphatro_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM AM_SHARE("p_ram")
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("p_videoram")
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
	m_sys_timer = timer_alloc(SYSTEM_TIMER);
	m_serial_timer = timer_alloc(SERIAL_TIMER);
}

void alphatro_state::machine_reset()
{
	// do what the IPL does
	//  UINT8* RAM = machine().device<ram_device>("ram")->pointer();
	UINT8* ROM = memregion("maincpu")->base();
	m_maincpu->set_pc(0xe000);
	memcpy(m_p_ram, ROM, 0xf000); // copy BASIC to RAM, which the undumped IPL is supposed to do.
	memcpy(m_p_videoram, ROM+0x1000, 0x1000);
	//  membank("bank1")->set_base(RAM);

	// probably not correct, exact meaning of port is unknown, vblank/vsync is too slow.
	m_sys_timer->adjust(attotime::from_usec(10),0,attotime::from_usec(10));
	m_serial_timer->adjust(attotime::from_hz(500),0,attotime::from_hz(500));  // USART clock - this is a guesstimate
	m_timer_bit = 0;
	beep_set_state(m_beep, 0);
	beep_set_frequency(m_beep, 950);	/* piezo-device needs to be measured */
}

static PALETTE_INIT( alphatro )
{
	// RGB colours
	palette_set_color_rgb(machine, 0, 0x00, 0x00, 0x00);
	palette_set_color_rgb(machine, 1, 0x00, 0x00, 0xff);
	palette_set_color_rgb(machine, 2, 0xff, 0x00, 0x00);
	palette_set_color_rgb(machine, 3, 0xff, 0x00, 0xff);
	palette_set_color_rgb(machine, 4, 0x00, 0xff, 0x00);
	palette_set_color_rgb(machine, 5, 0x00, 0xff, 0xff);
	palette_set_color_rgb(machine, 6, 0xff, 0xff, 0x00);
	palette_set_color_rgb(machine, 7, 0xff, 0xff, 0xff);
	// Amber
	palette_set_color_rgb(machine, 8, 0xf7, 0xaa, 0x00);
}

static const mc6845_interface alphatro_crtc6845_interface =
{
	"screen",
	8,
	NULL,
	alphatro_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const i8251_interface alphatro_usart_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(alphatro_state,rxdata_callback), //rxd_cb
	DEVCB_DRIVER_LINE_MEMBER(alphatro_state,txdata_callback), //txd_cb
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const cassette_interface alphatro_cassette_interface =
{
	cassette_default_formats,
	NULL,
	//(cassette_state) (CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	(cassette_state) (CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	"alphatro_cass",
	NULL
};

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
	MCFG_GFXDECODE(alphatro)
	MCFG_PALETTE_INIT(alphatro)
	MCFG_PALETTE_LENGTH(9) // 8 colours + amber

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)


	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, XTAL_12_288MHz / 8, alphatro_crtc6845_interface) // clk unknown

	MCFG_I8251_ADD("usart", alphatro_usart_interface)

	MCFG_CASSETTE_ADD(CASSETTE_TAG, alphatro_cassette_interface)

	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( alphatro )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "613256.ic-1058", 0x0000, 0x6000, CRC(ceea4cb3) SHA1(b332dea0a2d3bb2978b8422eb0723960388bb467) )
	ROM_LOAD( "2764.ic-1038",   0xe000, 0x2000, CRC(e337db3b) SHA1(6010bade6a21975636383179903b58a4ca415e49) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )
	ROM_LOAD( "ipl.bin",        0x8000, 0x2000, NO_DUMP  )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "2732.ic-1067",   0x0000, 0x1000, CRC(61f38814) SHA1(35ba31c58a10d5bd1bdb202717792ca021dbe1a8) )
ROM_END

COMP( 1983, alphatro,   0,       0,    alphatro,   alphatro, driver_device,  0,  "Triumph-Adler", "Alphatronic PC", GAME_NOT_WORKING )
