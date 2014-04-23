// license:BSD
// copyright-holders:Robbbert
/***************************************************************************

Goldstar FC-100 (FC stands for Famicom)

2014/04/20 Skeleton driver.

Known chips: M5C6847P, AY-3-8910, 8251. XTALS 7.15909, 4.9152

No manuals or schematics available.
Shift-Run to BREAK out of CLOAD.
Cassette uses the uart.


Test of semigraphic 6
10 SCREEN 2:CLS
20 FOR I=0 TO 360
30 PSET(128+SIN(I)*90,91-COS(I)*90), 1
40 NEXT
RUN

TODO:
- Cassette can be 600 or 1200 baud, how is 600 baud selected?
- Hookup Graphics modes and colours
- Unknown i/o ports
- Need software


****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6847.h"
#include "machine/i8251.h"
#include "machine/clock.h"
#include "sound/ay8910.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "formats/fc100_cas.h"


class fc100_state : public driver_device
{
public:
	fc100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "vdg")
		, m_p_videoram(*this, "videoram")
		, m_cass(*this, "cassette")
		, m_uart(*this, "uart")
	{ }

	DECLARE_READ8_MEMBER(mc6847_videoram_r);
	DECLARE_WRITE8_MEMBER(port31_w);
	DECLARE_WRITE8_MEMBER(port33_w);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);
	DECLARE_WRITE_LINE_MEMBER(uart_clock_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_c);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_p);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);

	UINT8 *m_p_chargen;
	static UINT8 get_char_rom(running_machine &machine, UINT8 ch, int line)
	{
		fc100_state *state = machine.driver_data<fc100_state>();
		return state->m_p_chargen[ch*16+line];
	}
private:
	virtual void machine_start();
	virtual void machine_reset();

	// graphics signals
	UINT8 m_ag;
	UINT8 m_gm2;
	UINT8 m_gm1;
	UINT8 m_gm0;
	UINT8 m_as;
	UINT8 m_css;
	UINT8 m_intext;
	UINT8 m_inv;
	UINT8 m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;
	bool m_key_pressed;

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<cassette_image_device> m_cass;
	required_device<i8251_device> m_uart;
};


static ADDRESS_MAP_START( fc100_mem, AS_PROGRAM, 8, fc100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x6000, 0x7fff ) AM_ROM AM_REGION("cart", 0)
	AM_RANGE( 0x8000, 0xBFFF ) AM_RAM // expansion ram pack
	AM_RANGE( 0xc000, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fc100_io, AS_IO, 8, fc100_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("00")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("01")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("02")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("03")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("04")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("05")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("06")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("07")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("08")
	AM_RANGE(0x09, 0x09) AM_READ_PORT("09")
	AM_RANGE(0x0A, 0x0A) AM_READ_PORT("0A")
	AM_RANGE(0x0B, 0x0B) AM_READ_PORT("0B")
	AM_RANGE(0x0C, 0x0C) AM_READ_PORT("0C")
	AM_RANGE(0x0D, 0x0D) AM_READ_PORT("0D")
	AM_RANGE(0x0E, 0x0E) AM_READ_PORT("0E")
	AM_RANGE(0x0F, 0x0F) AM_READ_PORT("0F")
	AM_RANGE(0x21, 0x21) AM_DEVWRITE("psg", ay8910_device, data_w)
	AM_RANGE(0x22, 0x22) AM_DEVREAD("psg", ay8910_device, data_r)
	AM_RANGE(0x23, 0x23) AM_DEVWRITE("psg", ay8910_device, address_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(port31_w)
	AM_RANGE(0x33, 0x33) AM_WRITE(port33_w)
	// AM_RANGE(0x60, 0x61)   writes 0 to both ports at boot
	AM_RANGE(0x70, 0x70) AM_WRITENOP //  each screen character also gets written here
	// AM_RANGE(0x71, 0x71)   writes 0 at boot
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0xb8, 0xb8) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( fc100 )
	PORT_START("00")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Graph") // does nothing
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Ctrl")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Caps")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("[") PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("]") PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_NAME("P") PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_NAME("O") PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("01")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("\\") PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("=") PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter") PORT_CHAR(13)

	PORT_START("02")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")

	PORT_START("03")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9") PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME(";") PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("'") PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_NAME("Tab") PORT_CHAR(9)

	PORT_START("04")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5") PORT_CHAR('5') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6") PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7") PORT_CHAR('7') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8") PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("05")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1") PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2") PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3") PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4") PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("06")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space") PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Run")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0") PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("07")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4")

	PORT_START("08")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-") PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/") PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".") PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(",") PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("09")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Home")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Ins")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Del")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_NAME("Esc") PORT_CHAR(27)

	PORT_START("0A")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("B") PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_NAME("N") PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("M") PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_NAME("L") PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("0B")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Z") PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("X") PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("C") PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_NAME("V") PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("0C")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_NAME("G") PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_NAME("H") PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_NAME("J") PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("K") PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("0D")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("A") PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("S") PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("D") PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("F") PORT_CHAR('F') PORT_CHAR('f')

	PORT_START("0E")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_NAME("T") PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Y") PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("U") PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("I") PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("0F")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q") PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("W") PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("E") PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("R") PORT_CHAR('R') PORT_CHAR('r')

	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// The timer frequency controls the auto-repeat delay and speed
TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::timer_k)
{
	/* scan the keyboard */
	UINT8 i;
	char kbdrow[6];

	for (i = 0; i < 16; i++)
	{
		sprintf(kbdrow,"0%X", i);
		if ((ioport(kbdrow)->read()) < 255)
		{
			// IRQ if key pressed
			m_key_pressed = 1;
			m_maincpu->set_input_line(0, HOLD_LINE);
			return;
		}
	}

	if (m_key_pressed) // IRQ for key released
	{
		m_key_pressed = 0;
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}


//********************* AUDIO **********************************
#if 0
WRITE8_MEMBER( fc100_state::ay_port_a_w )
{
	//logerror("ay_port_a_w: %02X\n", data);

	// Lacking schematics, these are all wild guesses
	// Having bit 1 set makes black display as blue??
	m_ag = BIT(data, 4);
	m_gm2 = BIT(data, 6);
	m_gm1 = BIT(data, 3);
	m_gm0 = BIT(data, 3);
	m_css = m_ag;

	m_vdg->ag_w( m_ag ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm2_w( m_gm2 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm1_w( m_gm1 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm0_w( m_gm0 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->css_w( m_css ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->hack_black_becomes_blue( BIT(data, 1) );
}


WRITE8_MEMBER( fc100_state::ay_port_b_w )
{
	//logerror("ay_port_b_w: %02X\n", data);
}
#endif

static const ay8910_interface ay8910_intf =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("JOY0"),
	DEVCB_INPUT_PORT("JOY1"),
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(fc100_state, ay_port_a_w),
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(fc100_state, ay_port_b_w)
};

//******************** VIDEO **********************************

READ8_MEMBER( fc100_state::mc6847_videoram_r )
{
	if (offset == ~0) return 0xff;

	if ( m_ag )
	{
		if ( m_gm2 )
		{
			// 256 x 192 / 6KB
			offset = ( ( offset & 0x1fc0 ) >> 1 ) | ( offset & 0x1f );
			return m_p_videoram[offset % 0xc00];
		}
		else
		{
			// 256 x 96 / 3KB
			return m_p_videoram[offset % 0xc00];
		}
	}

	// Standard text
	UINT8 data = m_p_videoram[offset];
	UINT8 attr = m_p_videoram[offset+0x200];

	// unknown bits 1,2,4,7
	m_vdg->inv_w( BIT( attr, 0 ));
	m_vdg->css_w( BIT( attr, 1)); // guess
	m_vdg->as_w( BIT( attr, 6 ));

	return data;
}

static const mc6847_interface fc100_mc6847_interface =
{
	"screen",
	DEVCB_DRIVER_MEMBER(fc100_state,mc6847_videoram_r),   // data fetch

	DEVCB_NULL,                 /* AG */
	DEVCB_NULL,                 /* GM2 */
	DEVCB_NULL,                 /* GM1 */
	DEVCB_NULL,                 /* GM0 */
	DEVCB_NULL,                 /* CSS */
	DEVCB_NULL,                 /* AS */
	DEVCB_LINE_VCC,             /* INTEXT */
	DEVCB_NULL,                 /* INV */

	&fc100_state::get_char_rom
};

/* F4 Character Displayer */
static const gfx_layout u53_charlayout =
{
	7, 15,                   /* 7 x 15 characters */
	256,                  /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( fc100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, u53_charlayout, 0, 1 )
GFXDECODE_END

//********************** UART/CASSETTE ***********************************

WRITE8_MEMBER( fc100_state::port31_w )
{
	if (data == 8)
		m_cass->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

WRITE8_MEMBER( fc100_state::port33_w )
{
	if (data == 0)
		m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

WRITE_LINE_MEMBER( fc100_state::txdata_callback )
{
	m_cass_state = state;
}

WRITE_LINE_MEMBER( fc100_state::uart_clock_w )
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::timer_c )
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

TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::timer_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

static const cassette_interface fc100_cassette_interface =
{
	fc100_cassette_formats,
	NULL,
	(cassette_state) (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};


//******************** MACHINE ******************************

void fc100_state::machine_start()
{
	m_ag = 0;
	m_gm2 = 0;
	m_gm1 = 0;
	m_gm0 = 0;
	m_as = 0;
	m_css = 0;
	m_intext = 0;
	m_inv = 0;

	save_item(NAME(m_ag));
	save_item(NAME(m_gm2));
	save_item(NAME(m_gm1));
	save_item(NAME(m_gm0));
	save_item(NAME(m_as));
	save_item(NAME(m_css));
	save_item(NAME(m_intext));
	save_item(NAME(m_inv));
}

void fc100_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
	m_cass_state = 0;
	m_cassold = 0;
	m_key_pressed = 1; // force irq to be cleared
}

static MACHINE_CONFIG_START( fc100, fc100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_7_15909MHz/2)
	MCFG_CPU_PROGRAM_MAP(fc100_mem)
	MCFG_CPU_IO_MAP(fc100_io)

	/* video hardware */
	MCFG_MC6847_ADD("vdg", MC6847_NTSC, XTAL_7_15909MHz/3, fc100_mc6847_interface )  // Clock not verified
	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "vdg")
	MCFG_GFXDECODE_ADD("gfxdecode", "f4palette", fc100)
	MCFG_PALETTE_ADD_MONOCHROME_AMBER("f4palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("psg", AY8910, XTAL_7_15909MHz/3/2)  /* AY-3-8910 - clock not verified */
	MCFG_SOUND_CONFIG(ay8910_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.50)

	/* Devices */
	MCFG_CASSETTE_ADD("cassette", fc100_cassette_interface)
	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE(fc100_state, txdata_callback))
	MCFG_DEVICE_ADD("uart_clock", CLOCK, XTAL_4_9152MHz/16/16) // gives 19200
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(fc100_state, uart_clock_w))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_c", fc100_state, timer_c, attotime::from_hz(4800)) // cass write
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_p", fc100_state, timer_p, attotime::from_hz(40000)) // cass read
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_k", fc100_state, timer_k, attotime::from_hz(300)) // keyb scan
	MCFG_CARTSLOT_ADD("cart")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fc100 )
	ROM_REGION( 0x6000, "roms", 0 )
	ROM_LOAD( "08-01.u48",     0x0000, 0x2000, CRC(24e78e75) SHA1(13121706544256a702635448ed2950a75c13f491) )
	ROM_LOAD( "08-02.u49",     0x2000, 0x2000, CRC(e14fc7e9) SHA1(9c5821e65c1efe698e25668d24c36929ea4c3ad7) )
	ROM_LOAD( "06-03.u50",     0x4000, 0x2000, CRC(d783c84e) SHA1(6d1bf53995e08724d5ecc24198cdda4442eb2eb9) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "cg-04-01.u53",  0x0000, 0x1000, CRC(2de75b7f) SHA1(464369d98cbae92ffa322ebaa4404cf5b26825f1) )

	ROM_REGION(0x2000,"cart", ROMREGION_ERASEFF)
	ROM_CART_LOAD("cart", 0x0000, 0x2000, ROM_OPTIONAL)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS          INIT    COMPANY    FULLNAME  FLAGS */
CONS( 1982, fc100,  0,      0,       fc100,   fc100,  driver_device,   0,   "Goldstar", "FC-100", GAME_NOT_WORKING )
