// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Pravetz 8C keyboard

    This 63-key keyboard communicates with its host using a modified
    Apple II/II+ keyboard connector carrying several unusual signals.
    (A 64th key, `/~, does not exist on the actual keyboard layout
    but is fully supported by the MCU program and is emulated here.)

    To switch between QWERTY and ЙЦУКЕН input modes, press the C/L
    (Cyrillic/Latin) Lock key.

    The Caps Lock setting is overridable by a soft switch in the
    main unit (whose state can be set by writing to bit 0 of $C060).

    The MCU program does not automatically repeat keys, since the
    IOU ASIC on IIe and clones normally implements this feature.

                             ┌─────────┐
                      +5V  1 │•        │ 16  SW1
                   STROBE  2 │         │ 15  KBD7
                   *RESET  3 │         │ 14  SOFTSW
                      AKD  4 │         │ 13  KBD1
                     KBD5  5 │         │ 12  KBD0
                     KBD4  6 │         │ 11  KBD3
                     KBD6  7 │         │ 10  KBD2
                      GND  8 │         │ 9   SW0
                             └─────────┘

*********************************************************************/

#include "emu.h"
#include "prav8ckb.h"

#include "cpu/m6805/m68705.h"

prav8ckb_device::prav8ckb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PRAV8C_KEYBOARD, tag, owner, clock)
	, m_b_callback(*this)
	, m_akd_callback(*this)
	, m_strobe_callback(*this)
	, m_reset_callback(*this)
	, m_keys(*this, "KEY%u", 0U)
	, m_fn(*this, "FN")
	, m_power_led(*this, "power")
	, m_caps_led(*this, "caps")
	, m_cl_led(*this, "cl")
	, m_key_select(0)
	, m_softsw(false)
{
}

void prav8ckb_device::device_start()
{
	m_power_led.resolve();
	m_caps_led.resolve();
	m_cl_led.resolve();

	m_power_led = 1;

	save_item(NAME(m_key_select));
	save_item(NAME(m_softsw));
}

u8 prav8ckb_device::key_r()
{
	// Matrix scanned using 74LS145 decoder
	return m_key_select < 10 ? m_keys[m_key_select]->read() : 0xff;
}

void prav8ckb_device::key_w(u8 data)
{
	// Keycodes are scrambled
	m_b_callback(bitswap<8>(data, 7, 0, 2, 1, 4, 3, 6, 5));
}

u8 prav8ckb_device::misc_r()
{
	// Reset line should normally be pulled up
	u8 result = 0x40 | m_softsw;

	// Bits 4, 3 define operating mode; only this one seems valid
	result |= 0x08;

	return result;
}

void prav8ckb_device::control_w(offs_t offset, u8 data, u8 mem_mask)
{
	// Mode LEDs
	m_caps_led = !BIT(data, 1);
	m_cl_led = !BIT(data, 2);

	// Line outputs
	m_strobe_callback(BIT(data, 5));
	m_reset_callback(BIT(data | ~mem_mask, 6)); // configured as input when inactive
	m_akd_callback(BIT(data, 7));
}

void prav8ckb_device::key_select_w(u8 data)
{
	m_key_select = data;
}

static INPUT_PORTS_START(prav8ckb)
	PORT_START("KEY0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  *  _") PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  (  ?") PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"K  Λ") PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K) // triangular form of Л shown
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"L  Д") PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"I  Ш") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"O  Щ") PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8",  <  Б") PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8".  >  Ю") PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)

	PORT_START("KEY1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  ^  ,") PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  &  .") PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"J  О") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"H  Р") PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Y  Н") PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"U  Г") PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"M  Ь") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"N  Т") PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)

	PORT_START("KEY2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  \"") PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  :") PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"F  А") PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"G  П") PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"R  К") PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"T  Е") PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"V  М") PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"B  И") PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)

	PORT_START("KEY3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  @  -") PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #  /") PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"S  Ы") PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"D  В") PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"W  Ц") PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"E  У") PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"C  С") PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  N") PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"A  Ф") PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Q  Й") PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB) // ↹
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"X  Ч") PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Z  Я") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)

	PORT_START("KEY6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C/L Lock") PORT_CODE(KEYCODE_RCONTROL) // to right of Caps Lock
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) // lower left corner

	PORT_START("KEY7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8";  :  Ж") PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"'  \"  Э") PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"[  {  Х") PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"P  З") PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08) PORT_CODE(KEYCODE_LEFT) // ←
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"/  ?  Е") PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)

	PORT_START("KEY8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  _  !  =") PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  )  %") PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("`  ~") PORT_CHAR('`') PORT_CHAR('~') PORT_CHAR(0x00) PORT_CODE(KEYCODE_TILDE) // not on keyboard, but fully implemented?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"]  }  Ъ") PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15) PORT_CODE(KEYCODE_RIGHT) // →
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CHAR(0x20) PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEY9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |  (  )") PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP), 0x0b) PORT_CODE(KEYCODE_UP) // ↑
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 0x0a) PORT_CODE(KEYCODE_DOWN) // ↓

	PORT_START("FN")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_LALT) // to left of space bar
	PORT_BIT(2, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CHAR(UCHAR_MAMEKEY(RALT)) PORT_CODE(KEYCODE_RALT) // to right of space bar
INPUT_PORTS_END

ioport_constructor prav8ckb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(prav8ckb);
}

void prav8ckb_device::device_add_mconfig(machine_config &config)
{
	m6805p2_device &mcu(M6805P2(config, "mcu", 4'000'000)); // unknown clock
	mcu.porta_r().set(FUNC(prav8ckb_device::key_r));
	mcu.porta_w().set(FUNC(prav8ckb_device::key_w));
	mcu.portb_r().set(FUNC(prav8ckb_device::misc_r));
	mcu.portb_w().set(FUNC(prav8ckb_device::control_w));
	mcu.portc_w().set(FUNC(prav8ckb_device::key_select_w));
}

ROM_START(prav8ckb)
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("cm650-05.bin", 0x000, 0x800, CRC(a97206e3) SHA1(f4bb44507889fd06d9e952aeb6c44be66ec6f8e9))
ROM_END

const tiny_rom_entry *prav8ckb_device::device_rom_region() const
{
	return ROM_NAME(prav8ckb);
}

// device type definition
DEFINE_DEVICE_TYPE(PRAV8C_KEYBOARD, prav8ckb_device, "prav8ckb", "Pravetz 8C keyboard")
