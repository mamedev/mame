// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II clone keyboard with AUTO

    This keyboard was dumped from a Taiwanese Apple II clone. It
    does not feature a lowercase mode or automatic key repeat, though
    it does include a numeric keypad and macros for BASIC, DOS and
    CP/M commands.

    Ctrl-Shift-A toggles automatic line numbering mode on or off.
    The keyboard is provisionally identified by this feature, which is
    not evident on several similar-looking Apple II clone keyboards.

    Ctrl-Shift-U locks the keyboard until Return is pressed.

*********************************************************************/

#include "emu.h"
#include "autokbd.h"

#include "cpu/mcs48/mcs48.h"

namespace {

class a2autokbd_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	a2autokbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

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

	// address maps
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<mcs48_cpu_device> m_mcu;
	required_ioport_array<8> m_keys;
	required_ioport m_modifiers;
};

a2autokbd_device::a2autokbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2KBD_AUTOKBD, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_keys(*this, "KEY%u", 0U)
	, m_modifiers(*this, "MODIFIERS")
{
}

void a2autokbd_device::device_start()
{
}

void a2autokbd_device::key_data_w(u8 data)
{
	strobe_w(BIT(data, 7));
	b_w(data & 0x7f);
}

u8 a2autokbd_device::key_matrix_r()
{
	const u8 scan = m_mcu->p1_r();

	u8 ret = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(scan, i))
			ret &= m_keys[i]->read();

	return ret;
}

int a2autokbd_device::shift_r()
{
	return (m_modifiers->read() & 0x03) == 0x03;
}

int a2autokbd_device::control_r()
{
	return BIT(m_modifiers->read(), 2);
}

void a2autokbd_device::prog_map(address_map &map)
{
	map(0x000, 0x7ff).mirror(0x800).rom().region("eprom", 0);
}

void a2autokbd_device::ext_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff).rw(FUNC(a2autokbd_device::key_matrix_r), FUNC(a2autokbd_device::key_data_w));
}

INPUT_CHANGED_MEMBER(a2autokbd_device::reset_changed)
{
	reset_w((m_modifiers->read() & 0x0c) != 0);
}

static INPUT_PORTS_START(a2autokbd)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  NEW") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '  PR#7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E  EXEC") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR() PORT_CHAR(0x05)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  SPEED=") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR() PORT_CHAR(0x1a)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  PR#1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  ^  NORMAL") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('^') PORT_CHAR(0x0e)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  CATALOG") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR() PORT_CHAR(0x03)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =  HGR") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )  NOMON") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  PR#5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space  BRUN") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #  PR#3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  CLEAR") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <  CLOSE") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B  BSAVE") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR() PORT_CHAR(0x02)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  MAXFILES") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR() PORT_CHAR(0x0f)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &  PR#6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  RUN") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR() PORT_CHAR(0x12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D  NOTRACE") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR() PORT_CHAR(0x04)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192  LOCK") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15) // →
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  LIST") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR() PORT_CHAR(0x0c)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F  FLASH") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR() PORT_CHAR(0x06)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  @  POSITION") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('@') PORT_CHAR(0x10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  INVERSE") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T  TEXT") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR() PORT_CHAR(0x14)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S  SAVE") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR() PORT_CHAR(0x13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q  CHAIN") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR() PORT_CHAR(0x11)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190  UNLOCK") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08) // ←
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  DELETE") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR() PORT_CHAR(0x0c)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  GR") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR() PORT_CHAR(0x06)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y  HIMEM") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR() PORT_CHAR(0x14)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A  AUTO") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR() PORT_CHAR(0x01)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W  STOP") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR() PORT_CHAR(0x11)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +  LOAD") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J  BLOAD") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR() PORT_CHAR(0x0a)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H  HOME") PORT_CODE(KEYCODE_H) PORT_CHAR('H')

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *  CALL-151") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (  MON") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  PR#4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  TRACE") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR() PORT_CHAR(0x18)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"  PR#2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  OPEN") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  RENAME") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  VERIFY") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR() PORT_CHAR(0x16)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 0  MBASIC") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1  STAT") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2  ASM") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3  PIP") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 4  REN") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5  USER") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 6  DUMP") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 7  DIR") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 8  TYPE") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 9  ERA") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad .  GBASIC") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad +  SUBMIT") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad -  FORMAT") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad ×  COPY") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad ÷  APDOS") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2autokbd_device::reset_changed), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(a2autokbd_device::reset_changed), 0)

	PORT_START("REPT")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rept") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // when low, changes all uppercase letters to lowercase (but shifted letters remain the same)
INPUT_PORTS_END

ioport_constructor a2autokbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2autokbd);
}

void a2autokbd_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_mcu, 3'579'545); // NEC D8035LC001 (XTAL unreadable on photo)
	m_mcu->set_addrmap(AS_PROGRAM, &a2autokbd_device::prog_map);
	m_mcu->set_addrmap(AS_IO, &a2autokbd_device::ext_map);
	m_mcu->p2_in_cb().set_ioport("REPT");
	m_mcu->t0_in_cb().set(FUNC(a2autokbd_device::control_r));
	m_mcu->t1_in_cb().set(FUNC(a2autokbd_device::shift_r));

	config.set_maximum_quantum(attotime::from_usec(200));
}

ROM_START(a2autokbd)
	ROM_REGION(0x800, "eprom", 0)
	ROM_LOAD("keyboard.bin", 0x000, 0x800, CRC(8a9e77a8) SHA1(a0cb73e43f30b6a05bd766136dcd3cee1384bf5f)) // NEC D2716D
ROM_END

const tiny_rom_entry *a2autokbd_device::device_rom_region() const
{
	return ROM_NAME(a2autokbd);
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2KBD_AUTOKBD, device_a2kbd_interface, a2autokbd_device, "a2autokbd", "unknown Apple II clone keyboard with AUTO")
