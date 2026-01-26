// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    TK-10 keyboard for Apple II

    Versions of this Taiwanese-made keyboard can be found on many
    Apple II clones. Some variants feature a built-in numeric keypad,
    though the key matrix appears to lack unique positions for the
    digits there.

    Unlike standard II/II+ keyboards, it can generate all 63 ASCII
    codes displayable by a standard Apple II in both lowercase and
    uppercase modes, and it pairs no letters with punctuation
    symbols in either mode.

    The Func key, when used to modify letters, generates common
    Applesoft BASIC keywords. The conspicuously blank areas on the
    flanks of a few keycaps cover up for a few cases of unfortunate
    misspellings (e.g. "LOMEN", "HIMON"). Combinations of Ctrl, Func
    and digits generate strings such as "DF1:=".

    Another version of this keyboard has a MCU labeled TK-11.

*********************************************************************/

#include "emu.h"
#include "tk10.h"

#include "cpu/mcs48/mcs48.h"

namespace {

class a2tk10_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	a2tk10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);
	void int_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2kbd_interface implementation
	virtual int shift_r() override;
	virtual int control_r() override;

private:
	// I/O port handlers
	void key_data_w(u8 data);
	u8 key_matrix_r();

	required_device<mcs48_cpu_device> m_kbdmcu;
	required_ioport_array<8> m_keys;
	required_ioport m_modifiers;
};

a2tk10_device::a2tk10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2KBD_TK10, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_kbdmcu(*this, "kbdmcu")
	, m_keys(*this, "KEY%u", 0U)
	, m_modifiers(*this, "MODIFIERS")
{
}

void a2tk10_device::device_start()
{
}

void a2tk10_device::key_data_w(u8 data)
{
	strobe_w(BIT(data, 7));
	b_w(data & 0x7f);
}

u8 a2tk10_device::key_matrix_r()
{
	const u8 scan = m_kbdmcu->p2_r();

	u8 ret = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(scan, i))
			ret &= m_keys[i]->read();

	return ret;
}

int a2tk10_device::shift_r()
{
	return (m_modifiers->read() & 0x03) == 0x03;
}

int a2tk10_device::control_r()
{
	return BIT(m_modifiers->read(), 2);
}

INPUT_CHANGED_MEMBER(a2tk10_device::reset_changed)
{
	reset_w((m_modifiers->read() & 0x0c) != 0);
}

void a2tk10_device::int_w(int state)
{
	m_kbdmcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START(a2tk10)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // does nothing
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // generates CAN "DF0:=" CAN "DF1:=" (etc.) CAN "DF9:=" CAN CR (like Ctrl-Esc)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // does nothing
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // generates "CALL-151" CR (like Func-M)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // generates DC3 (not populated on most models)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // generates DEL (not populated on most models)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // generates HT (not populated on most models)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // generates _, ^, US (not populated on most models)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // generates ], }, GS (not populated on most models)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // generates [, {, ESC (not populated on most models)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // generates `, ~ (not populated on most models)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad ÷") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // backslash, |, 0x1c (not populated on most models)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W  GET  HTAB") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  NOTRACE CR  SPEED") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A  STEP") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q  HGR CR") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  \\") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^  ]  [") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('^') PORT_CHAR(']') PORT_CHAR(0x1e)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  _") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  VTAB  BSAVE") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F  FLASH  DIM") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  RUN CR  RETURN") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  CATALOG CR  COLOR") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D  DELETE  DATA") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E  RESTORE  READ") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  CHR$  LOCK") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S  SAVE  STR$(") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J    HPLOT") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U  UNLOCK  INIT") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  NEW CR  NEXT") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H  HOME CR  HCOLOR") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y    THEN") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B  BRUN  BLOAD") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  GOSUB  GOTO") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T  TRACE CR  TEXT") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  PRINT  PR#") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  LIST CR  LOAD") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  ONERR  POKE") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K    LEFT$(") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  INPUT  INVERSE") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  MONITOR  NORMAL") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR(0x00)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2tk10_device::reset_changed), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2tk10_device::reset_changed), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock/Func") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_WRITE_LINE_MEMBER(FUNC(a2tk10_device::int_w))
INPUT_PORTS_END

ioport_constructor a2tk10_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2tk10);
}

void a2tk10_device::device_add_mconfig(machine_config &config)
{
	// XTAL is 3.579545 MHz according to one PCB photo, but 6 MHz according to unofficial schematics
	// Since the protocol is not very timing-sensitive, both values may have been used on different systems
	I8049(config, m_kbdmcu, 3.579545_MHz_XTAL);
	m_kbdmcu->bus_out_cb().set(FUNC(a2tk10_device::key_data_w));
	m_kbdmcu->p1_in_cb().set(FUNC(a2tk10_device::key_matrix_r));
	m_kbdmcu->t0_in_cb().set(FUNC(a2tk10_device::control_r));
	m_kbdmcu->t1_in_cb().set(FUNC(a2tk10_device::shift_r));
}

ROM_START(a2tk10)
	ROM_REGION(0x800, "kbdmcu", 0)
	ROM_LOAD("tk10.bin", 0x000, 0x800, CRC(a06c5b78) SHA1(27c5160b913e0f62120f384026d24b9f1acb6970))
ROM_END

const tiny_rom_entry *a2tk10_device::device_rom_region() const
{
	return ROM_NAME(a2tk10);
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2KBD_TK10, device_a2kbd_interface, a2tk10_device, "a2tk10", "TK-10 Apple II keyboard")
