// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    This emulates the standard serial keyboard used with Tektronix's
    4105, 4106, 4107 and 4109 terminals, including the trademark
    octagonal "joy disk". The 4404 keyboard is also compatible.

    The hardware was designed by the Keytronic Corporation, and uses
    their 20-pin key strobe driver (22-0950-003) and input receiver
    (22-0908-003) ASICs.

    The 8048/8748's DB pins are connected to drivers for up to 5 LEDs,
    but only one of these LEDs (CR9, on the Caps Lock key) is actually
    populated.

    A different but apparently backward-compatible keyboard was used
    with the CX4107 and CX4109, based on a 8051 CPU with space for an
    external program ROM and adding an onboard speaker as well as many
    extra keys and an IBM 3279-style layout.

    Hexadecimal table of press codes (release codes are the same + 80):

        00      Caps Lock
        01      Left Shift
        02      Right Shift
        03      Ctrl
        04      DEras/SEras [4404 keyboard: ←/↑]
        05      Break
        06      Back Space
        07      Tab
        08      Line Feed
        09      Return
        0A      Esc
        0B      Space Bar
        0C      " '
        0D      < ,
        0E      _ -
        0F      > .
        10      ? /
        11      ) 0
        12      ! 1
        13      @ 2
        14      # 3
        15      $ 4
        16      % 5
        17      ^ 6
        18      & 7
        19      * 8
        1A      ( 9
        1B      : ;
        1C      =
        1D      A
        1E      B
        1F      C
        20      D
        21      E
        22      F
        23      G
        24      H
        25      I
        26      J
        27      K
        28      L
        29      M
        2A      N
        2B      O
        2C      P
        2D      Q
        2E      R
        2F      S
        30      T
        31      U
        32      V
        33      W
        34      X
        35      Y
        36      Z
        37      { [
        38      ` \
        39      } ]
        3A      ~ |
        3B      Rub Out
        3C      Enter (keypad)
        3D      ,     (keypad)
        3E      -     (keypad)
        3F      .     (keypad)
        40      0     (keypad)
        41      1     (keypad)
        42      2     (keypad)
        43      3     (keypad)
        44      4     (keypad)
        45      5     (keypad)
        46      6     (keypad)
        47      7     (keypad)
        48      8     (keypad)
        49      9     (keypad)
        4A      F1    [4404 keyboard: F5]
        4B      F2    [4404 keyboard: F6]
        4C      F3    [4404 keyboard: F7]
        4D      F4    [4404 keyboard: F8]
        4E      F5    [4404 keyboard: F9]
        4F      F6    [4404 keyboard: F10]
        50      F7    [4404 keyboard: F11]
        51      F8    [4404 keyboard: F12]
        52      GEras/Dialog [4404 keyboard: F1]
        53      Cancel/Setup [4404 keyboard: F2]
        54      DCopy/SCopy  [4404 keyboard: F3]
        55      Menu         [4404 keyboard: F4]
        56      Cursor Right (joystick)
        57      Cursor Up    (joystick)
        58      Cursor Left  (joystick)
        59      Cursor Down  (joystick)

    Additional press codes for CX keyboard only:

        5A      Erase EOF
        5B      Dup/PA1
        5C      Reset/Dev Cncl
        5D      Field Mark/PA2/_
        5E      Attn Sys Req
        5F      Cursr Sel/Clear
        60      < >
        61      Cursr Blink/Alt Cursr
        62      Ident
        63      Down Arrow
        64      Right Arrow/Right Double Arrow
        65      Left Arrow/Left Double Arrow
        66      Up Arrow
        67      Insert/Alpha
        68      Right Alt/Ctrl

    The baud rate is 4800, with 8 data bits and no parity.

**********************************************************************/

#include "emu.h"
#include "tek410x_kbd.h"

// device type definition
DEFINE_DEVICE_TYPE(TEK410X_KEYBOARD, tek410x_keyboard_device, "tek410x_kbd", "Tektronix 410X Standard Keyboard (119-1592-01)")

tek410x_keyboard_device::tek410x_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TEK410X_KEYBOARD, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_key_matrix(*this, "X%u", 0U)
	, m_config(*this, "CONFIG")
	, m_tdata_callback(*this)
	, m_rdata_callback(*this)
	, m_select(0)
	, m_p2_out(0)
	, m_kdi(true)
	, m_kdo(false)
{
}

void tek410x_keyboard_device::device_start()
{
	save_item(NAME(m_select));
	save_item(NAME(m_p2_out));
	save_item(NAME(m_kdi));
	save_item(NAME(m_kdo));
}

void tek410x_keyboard_device::kdi_w(int state)
{
	m_kdi = state;
	if (BIT(m_p2_out, 7))
	{
		m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
		m_tdata_callback(state);
	}
}

void tek410x_keyboard_device::kdo_w(int state)
{
	m_kdo = state;
	if (BIT(m_p2_out, 5))
		m_rdata_callback(state);
}

void tek410x_keyboard_device::reset_w(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
		p2_w(0xff);
}

u8 tek410x_keyboard_device::p1_r()
{
	if (m_select < 12)
		return m_key_matrix[m_select]->read();
	else
		return 0xff;
}

u8 tek410x_keyboard_device::p2_r()
{
	return 0xb0 | (m_kdo ? (m_p2_out & 0x20) << 1 : 0) | m_config->read();
}

void tek410x_keyboard_device::p2_w(u8 data)
{
	if (!BIT(data, 4))
		m_select = m_mcu->p1_r() & 0x0f;

	if (m_kdi && BIT(data, 7) != BIT(m_p2_out, 7))
	{
		m_tdata_callback(BIT(data, 7));
		m_mcu->set_input_line(MCS48_INPUT_IRQ, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
	}

	if (m_kdo && BIT(data, 5) != BIT(m_p2_out, 5))
		m_rdata_callback(BIT(data, 5));

	m_p2_out = data;
}

void tek410x_keyboard_device::ext_map(address_map &map)
{
	map(0x00, 0xff).nopr();
}

static INPUT_PORTS_START(tek410x_keyboard)
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RCONTROL) // to right of Back Space
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rub Out") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_BACKSLASH) // to right of =/+
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('`') PORT_CODE(KEYCODE_OPENBRACE) // to right of P
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_CLOSEBRACE) // to right of backslash
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space Bar") PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Setup  Cancel") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SCopy  DCopy") PORT_CODE(KEYCODE_F3)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dialog  GEras") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Menu") PORT_CODE(KEYCODE_F4)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC) // to left of |/~
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB) // to left of Ctrl
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('|') PORT_CHAR('~') PORT_CODE(KEYCODE_BACKSLASH2) // to left of Q
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SEras  DEras") PORT_CODE(KEYCODE_LALT) // to left of [/{
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_TILDE) // to left of 1/!
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)

	PORT_START("X10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('>') PORT_CODE(KEYCODE_COMMA)

	PORT_START("X11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR('(') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)

	PORT_START("CONFIG")
	PORT_DIPNAME(0x0f, 0x00, "Keyboard Type") PORT_DIPLOCATION("E:1,2,3,4")
	PORT_DIPSETTING(0x00, "North America")
	PORT_DIPSETTING(0x08, "United Kingdom (4A)")
	PORT_DIPSETTING(0x04, "French (4B)")
	PORT_DIPSETTING(0x02, "Swedish (4C)")
	PORT_DIPSETTING(0x06, "Danish/Norwegian (4F)")
	PORT_DIPSETTING(0x07, "German (4G)")
	PORT_DIPSETTING(0x0a, "Katakana (4K)")
INPUT_PORTS_END

ioport_constructor tek410x_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tek410x_keyboard);
}

void tek410x_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8748(config, m_mcu, 4.608_MHz_XTAL);
	m_mcu->set_addrmap(AS_IO, &tek410x_keyboard_device::ext_map);
	m_mcu->bus_out_cb().set_output("led0").bit(0);
	m_mcu->p1_in_cb().set(FUNC(tek410x_keyboard_device::p1_r));
	m_mcu->p2_in_cb().set(FUNC(tek410x_keyboard_device::p2_r));
	m_mcu->p2_out_cb().set(FUNC(tek410x_keyboard_device::p2_w));
}

ROM_START(tek410x_kbd)
	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("473_8748.bin", 0x000, 0x400, CRC(371553a8) SHA1(165ffc2c0775c1a3c2cc3ec86fb05adc8e8bb3eb))
ROM_END

const tiny_rom_entry *tek410x_keyboard_device::device_rom_region() const
{
	return ROM_NAME(tek410x_kbd);
}
