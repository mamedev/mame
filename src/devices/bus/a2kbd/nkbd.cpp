// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Apple II new keyboard (605-X105)

    This two-piece keyboard, designed by The Keyboard Company, has
    a layout identical to the older single-piece keyboard. It was
    standard on late revisions of the Apple II and on the Apple II+.
    The encoder board bears a 1979 copyright date.

    The AY-5-3600-931 keyboard encoder (Apple P/N 331-0391, second-
    sourced as KR3600-070, variously silkscreened) is specially mask-
    programmed for this keyboard. There were two documented revisions
    of this IC: "A" assigns NUL characters to invalid "ghost" keys,
    while "B" assigns spaces instead.

    The encoder board provides pads and mounting holes for a few
    obscure modifications. One restores the single-button reset
    technique of the original Apple keyboard. Another generates
    unshifted lowercase letters by disconnecting two of the AY-5-3600
    data output lines and replacing them with two others. However,
    this alternate configuration also eliminates the ability to input
    the characters ], ^ and @. A third modification (not currently
    emulated) adds a 9-pin connector for a numeric keypad which uses
    four additional strobes output by the AY-5-3600 but not connected
    to the keyboard proper.

*********************************************************************/

#include "emu.h"
#include "nkbd.h"

#include "machine/kb3600.h"
#include "machine/timer.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2nkbd_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	a2nkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2kbd_interface implementation
	virtual int shift_r() override;
	virtual int control_r() override;

private:
	// AY-5-3600 handlers
	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	void ay3600_ako_w(int state);

	// timer callback
	TIMER_DEVICE_CALLBACK_MEMBER(ay3600_repeat);

	// object finders
	required_device<ay3600_device> m_ay3600;
	required_ioport m_kbspecial;
	required_ioport m_kbrepeat;
	required_ioport m_resetdip;

	// internal state
	u16 m_lastchar;
	u8 m_transchar;
	bool m_anykeydown;
};

a2nkbd_device::a2nkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_NKBD, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_ay3600(*this, "ay3600")
	, m_kbspecial(*this, "keyb_special")
	, m_kbrepeat(*this, "keyb_repeat")
	, m_resetdip(*this, "reset_dip")
{
}

void a2nkbd_device::device_start()
{
	m_anykeydown = false;
	m_transchar = 0;
	m_lastchar = 0;

	save_item(NAME(m_anykeydown));
	save_item(NAME(m_transchar));
	save_item(NAME(m_lastchar));
}

//**************************************************************************
//  KEYBOARD
//**************************************************************************

int a2nkbd_device::shift_r()
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return 0;
	}

	return 1;
}

int a2nkbd_device::control_r()
{
	if (m_kbspecial->read() & 0x08)
	{
		return 0;
	}

	return 1;
}

static const u8 a2_key_remap[0x32][6] =
{
/*    norm shft ctrl both */
	{ 0x33,0x23,0x33,0x23, 0x33,0x23 },    // 3 #     00
	{ 0x34,0x24,0x34,0x24, 0x34,0x24 },    // 4 $     01
	{ 0x35,0x25,0x35,0x25, 0x35,0x25 },    // 5 %     02
	{ 0x36,'&', 0x35,'&',  0x36,'&'  },    // 6 &     03
	{ 0x37,0x27,0x37,0x27, 0x37,0x27 },    // 7 '     04
	{ 0x38,0x28,0x38,0x28, 0x38,0x28 },    // 8 (     05
	{ 0x39,0x29,0x39,0x29, 0x39,0x29 },    // 9 )     06
	{ 0x30,0x30,0x30,0x30, 0x30,0x30 },    // 0       07
	{ 0x3a,0x2a,0x3a,0x2a, 0x3a,0x2a },    // : *     08
	{ 0x2d,0x3d,0x2d,0x3d, 0x2d,0x3d },    // - =     09
	{ 0x51,0x51,0x11,0x11, 0x71,0x51 },    // q Q     0a
	{ 0x57,0x57,0x17,0x17, 0x77,0x57 },    // w W     0b
	{ 0x45,0x45,0x05,0x05, 0x65,0x45 },    // e E     0c
	{ 0x52,0x52,0x12,0x12, 0x72,0x52 },    // r R     0d
	{ 0x54,0x54,0x14,0x14, 0x74,0x54 },    // t T     0e
	{ 0x59,0x59,0x19,0x19, 0x79,0x59 },    // y Y     0f
	{ 0x55,0x55,0x15,0x15, 0x75,0x55 },    // u U     10
	{ 0x49,0x49,0x09,0x09, 0x69,0x49 },    // i I     11
	{ 0x4f,0x4f,0x0f,0x0f, 0x6f,0x4f },    // o O     12
	{ 0x50,0x40,0x10,0x00, 0x70,0x50 },    // p P     13
	{ 0x44,0x44,0x04,0x04, 0x64,0x44 },    // d D     14
	{ 0x46,0x46,0x06,0x06, 0x66,0x46 },    // f F     15
	{ 0x47,0x47,0x07,0x07, 0x67,0x47 },    // g G     16
	{ 0x48,0x48,0x08,0x08, 0x68,0x48 },    // h H     17
	{ 0x4a,0x4a,0x0a,0x0a, 0x6a,0x4a },    // j J     18
	{ 0x4b,0x4b,0x0b,0x0b, 0x6b,0x4b },    // k K     19
	{ 0x4c,0x4c,0x0c,0x0c, 0x6c,0x4c },    // l L     1a
	{ ';' ,0x2b,';' ,0x2b, ';' ,0x2b },    // ; +     1b
	{ 0x08,0x08,0x08,0x08, 0x08,0x08 },    // Left    1c
	{ 0x15,0x15,0x15,0x15, 0x15,0x15 },    // Right   1d
	{ 0x5a,0x5a,0x1a,0x1a, 0x7a,0x5a },    // z Z     1e
	{ 0x58,0x58,0x18,0x18, 0x78,0x58 },    // x X     1f
	{ 0x43,0x43,0x03,0x03, 0x63,0x43 },    // c C     20
	{ 0x56,0x56,0x16,0x16, 0x76,0x56 },    // v V     21
	{ 0x42,0x42,0x02,0x02, 0x62,0x42 },    // b B     22
	{ 0x4e,0x5e,0x0e,0x1e, 0x6e,0x4e },    // n N     23
	{ 0x4d,0x5d,0x0d,0x1d, 0x6d,0x4d },    // m M     24
	{ 0x2c,0x3c,0x2c,0x3c, 0x2c,0x3c },    // , <     25
	{ 0x2e,0x3e,0x2e,0x3e, 0x2e,0x3e },    // . >     26
	{ 0x2f,0x3f,0x2f,0x3f, 0x2f,0x3f },    // / ?     27
	{ 0x53,0x53,0x13,0x13, 0x73,0x53 },    // s S     28
	{ 0x32,0x22,0x32,0x22, 0x32,0x22 },    // 2 "     29
	{ 0x31,0x21,0x31,0x31, 0x31,0x21 },    // 1 !     2a
	{ 0x1b,0x1b,0x1b,0x1b, 0x1b,0x1b },    // Escape  2b
	{ 0x41,0x41,0x01,0x01, 0x61,0x41 },    // a A     2c
	{ 0x20,0x20,0x20,0x20, 0x20,0x20 },    // Space   2d
	{ 0x20,0x20,0x20,0x20, 0x20,0x20 },    // 0x2e unused
	{ 0x20,0x20,0x20,0x20, 0x20,0x20 },    // 0x2f unused
	{ 0x20,0x20,0x20,0x20, 0x20,0x20 },    // 0x30 unused
	{ 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d },    // Return  31
};

void a2nkbd_device::ay3600_data_ready_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_lastchar = m_ay3600->b_r();

		ioport_value kbspecial = m_kbspecial->read();
		int mod = (kbspecial & 0x06) ? 0x01 : 0x00;
		mod |= (kbspecial & 0x08) ? 0x02 : (kbspecial & 0x10) ? 0x04 : 0x00;

		u8 transchar = a2_key_remap[m_lastchar&0x3f][mod];
//      printf("new char = %04x (%02x)\n", m_lastchar&0x3f, transchar);
		b_w(transchar);

		strobe_w(1);
		strobe_w(0);
	}
}

void a2nkbd_device::ay3600_ako_w(int state)
{
	m_anykeydown = (state == ASSERT_LINE) ? true : false;
}

TIMER_DEVICE_CALLBACK_MEMBER(a2nkbd_device::ay3600_repeat)
{
	// is the key still down?
	if (m_anykeydown)
	{
		if (m_kbrepeat->read() & 1)
		{
			strobe_w(1);
			strobe_w(0);
		}
	}
}

INPUT_CHANGED_MEMBER(a2nkbd_device::reset_changed)
{
	if (m_resetdip->read() & 1)
	{
		// CTRL-RESET
		reset_w((m_kbspecial->read() & 0x88) != 0x88);
	}
	else
	{
		// plain RESET
		reset_w(~m_kbspecial->read() & 0x80);
	}
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

	/*
	  Apple II / II Plus key matrix (from "The Apple II Circuit Description")

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  0  | :*  |  -  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  |  Q  |  W  |  E  |  R  |  T  |  Y  |  U  |  I  |  O  |  P  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  D  |  F  |  G  |  H  |  J  |  K  |  L  | ;+  |LEFT |RIGHT|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  N  |  M  | ,<  | .>  |  /? |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |  S  |  2  |  1  | ESC |  A  |SPACE|     |     |     |ENTER|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

static INPUT_PORTS_START( apple2_nkbd )
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)  PORT_CHAR('0')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q', 'q') PORT_CHAR() PORT_CHAR(0x11)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W', 'w') PORT_CHAR() PORT_CHAR(0x17)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E', 'e') PORT_CHAR() PORT_CHAR(0x05)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R', 'r') PORT_CHAR() PORT_CHAR(0x12)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T', 't') PORT_CHAR() PORT_CHAR(0x14)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y', 'y') PORT_CHAR() PORT_CHAR(0x19)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U', 'u')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I', 'i') PORT_CHAR() PORT_CHAR(0x09)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O', 'o') PORT_CHAR() PORT_CHAR(0x0f)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P  @")     PORT_CODE(KEYCODE_P)  PORT_CHAR('P', 'p') PORT_CHAR('@') PORT_CHAR(0x10)

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D', 'd') PORT_CHAR() PORT_CHAR(0x04)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F', 'f') PORT_CHAR() PORT_CHAR(0x06)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G  BELL")  PORT_CODE(KEYCODE_G)  PORT_CHAR('G', 'g') PORT_CHAR() PORT_CHAR(0x07)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H', 'h')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J', 'j') PORT_CHAR() PORT_CHAR(0x0a)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K', 'k') PORT_CHAR() PORT_CHAR(0x0b)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L', 'l') PORT_CHAR() PORT_CHAR(0x0c)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15)

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z', 'z') PORT_CHAR() PORT_CHAR(0x1a)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X', 'x') PORT_CHAR() PORT_CHAR(0x18)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C', 'c') PORT_CHAR() PORT_CHAR(0x03)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V', 'v') PORT_CHAR() PORT_CHAR(0x16)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B', 'b') PORT_CHAR() PORT_CHAR(0x02)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N  ^")     PORT_CODE(KEYCODE_N)  PORT_CHAR('N', 'n') PORT_CHAR('^') PORT_CHAR(0x0e)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M', 'm') PORT_CHAR(']')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S', 's') PORT_CHAR() PORT_CHAR(0x13)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)  PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(0x1b)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A', 'a') PORT_CHAR() PORT_CHAR(0x01)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(0x0d)

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")         PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
												  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2nkbd_device::reset_changed), 0)
	PORT_DIPNAME( 0x10, 0x00, "Letters" ) PORT_DIPLOCATION("S2:!1")
	PORT_DIPSETTING( 0x00, "Uppercase only" )
	PORT_DIPSETTING( 0x10, "Lowercase and uppercase" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset")        PORT_CODE(KEYCODE_F12)
												  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2nkbd_device::reset_changed), 0)

	PORT_START("keyb_repeat")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Rept")         PORT_CODE(KEYCODE_BACKSLASH)

	PORT_START("reset_dip")
	PORT_DIPNAME( 0x01, 0x01, "Reset" ) PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING( 0x01, "Ctrl+Reset" )
	PORT_DIPSETTING( 0x00, "Reset" )
INPUT_PORTS_END

ioport_constructor a2nkbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apple2_nkbd );
}

void a2nkbd_device::device_add_mconfig(machine_config &config)
{
	// keyboard controller
	AY3600(config, m_ay3600, 0);
	m_ay3600->x0().set_ioport("X0");
	m_ay3600->x1().set_ioport("X1");
	m_ay3600->x2().set_ioport("X2");
	m_ay3600->x3().set_ioport("X3");
	m_ay3600->x4().set_ioport("X4");
	m_ay3600->x5().set_ioport("X5");
	m_ay3600->x6().set_ioport("X6");
	m_ay3600->x7().set_ioport("X7");
	m_ay3600->x8().set_ioport("X8");
	m_ay3600->shift().set(FUNC(a2nkbd_device::shift_r)).invert();
	m_ay3600->control().set(FUNC(a2nkbd_device::control_r)).invert();
	m_ay3600->data_ready().set(FUNC(a2nkbd_device::ay3600_data_ready_w));
	m_ay3600->ako().set(FUNC(a2nkbd_device::ay3600_ako_w));

	// repeat timer.  15 Hz from page 90 of "The Apple II Circuit Description"
	TIMER(config, "repttmr", 0).configure_periodic(FUNC(a2nkbd_device::ay3600_repeat), attotime::from_hz(15));
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_NKBD, device_a2kbd_interface, a2nkbd_device, "a2nkbd", "Apple II new keyboard (AY-5-3600-931 based)")
