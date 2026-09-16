// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Ivasim Ivel Ultra 63-key keyboard

    This Serbo-Croatian keyboard has somewhat similar properties to
    the TK-10 keyboard, though its program is undoubtedly original.

    The TTY Lock key functions as both a caps lock key and a modifier
    key that can be used in conjunction with the characters of the
    top row or with the letter M (to enter the monitor). Two of these
    functions, YU and ASCII, produce no keycodes but switch the
    system between the JUS I.B1.002 ("YUSCII") code and normal ASCII.
    This key also has an unemulated LED of unknown significance.

*********************************************************************/

#include "emu.h"
#include "ivelultrkb.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"

namespace {

class ivelultrkb_device : public device_t, public device_a2kbd_interface
{
public:
	// device type constructor
	ivelultrkb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// port handlers
	u8 pia_pa_r();

	// access handlers
	u8 pia_r(offs_t offset);
	void pia_w(offs_t offset, u8 data);

	// memory map
	void mem_map(address_map &map);

	// object finders
	required_device<cpu_device> m_kbmcu;
	required_device<pia6821_device> m_pia;
	required_ioport_array<10> m_keys;
	required_ioport m_ctrl;

	// internal state
	u16 m_scan_offset;
};

ivelultrkb_device::ivelultrkb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IVEL_ULTRA_KEYBOARD, tag, owner, clock)
	, device_a2kbd_interface(mconfig, *this)
	, m_kbmcu(*this, "kbmcu")
	, m_pia(*this, "pia")
	, m_keys(*this, "KEY%u", 0U)
	, m_ctrl(*this, "CTRL")
	, m_scan_offset(0)
{
}

void ivelultrkb_device::device_start()
{
	save_item(NAME(m_scan_offset));
}

u8 ivelultrkb_device::pia_pa_r()
{
	u8 ret = 0xff;

	for (int i = 0; i < 10; i++)
		if (BIT(m_scan_offset, i))
			ret &= m_keys[i]->read();

	return ret;
}

u8 ivelultrkb_device::pia_r(offs_t offset)
{
	m_scan_offset = ~offset & 0x3ff;
	return m_pia->read(offset >> 12);
}

void ivelultrkb_device::pia_w(offs_t offset, u8 data)
{
	m_pia->write(offset >> 12, data);
}

void ivelultrkb_device::mem_map(address_map &map)
{
	map(0x4000, 0x7fff).rw(FUNC(ivelultrkb_device::pia_r), FUNC(ivelultrkb_device::pia_w));
	map(0xf800, 0xffff).rom().region("kbmcu", 0);
}

INPUT_CHANGED_MEMBER(ivelultrkb_device::reset_changed)
{
	const u8 ctrl = m_ctrl->read();

	m_kbmcu->set_input_line(INPUT_LINE_RESET, (ctrl & 0x03) == 0 ? ASSERT_LINE : CLEAR_LINE);
	reset_w((ctrl & 0x03) != 0);
}

static INPUT_PORTS_START(ivelultrkb)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(0x11)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"  HGR2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  HGR1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(0x01)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(0x1a)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Đ \\  |") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\', 0x0110) PORT_CHAR('|', 0x0111) PORT_CHAR(0x1c)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home  EOF") PORT_CODE(KEYCODE_HOME) // Esc @
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+  *  EOL")  PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(0x09)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ž @  `") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('@', 0x017d) PORT_CHAR('`', 0x017e) PORT_CHAR(0x00)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 0x15)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 0x08)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Š [  {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[', 0x0160) PORT_CHAR('{', 0x0161)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(0x10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  ASCII")  PORT_CODE(KEYCODE_MINUS) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  =  YU") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ć ]  }") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(']', 0x0106) PORT_CHAR('}', 0x0107) PORT_CHAR(0x1d)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Č ^  ~") PORT_CODE(KEYCODE_COLON) PORT_CHAR('^', 0x010c) PORT_CHAR('~', 0x010d) PORT_CHAR(0x1e)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR('<') PORT_CHAR('>') // to right of -/_
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(0x0f)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )  LIST") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (  RUN") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(0x0c)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(0x19)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '  TEXT") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &  BOOT") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(0x0e)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  CAT2") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(0x07)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(0x06)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(0x02)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(0x16)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(0x05)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(0x17)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #  EDIT") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  CAT1") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(0x04)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(0x18)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Brk") PORT_CODE(KEYCODE_F1) PORT_CHAR(0x03) // to left of Esc
	PORT_BIT(0x1c, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop List") PORT_CODE(KEYCODE_F2) PORT_CHAR(0x13) // to left of Ctrl
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), 0x0b)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 0x0a)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TTY Lock") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // to left of Left Shift

	PORT_START("KEY9")
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("CTRL")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ivelultrkb_device::reset_changed), 0)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ivelultrkb_device::reset_changed), 0)
INPUT_PORTS_END

ioport_constructor ivelultrkb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ivelultrkb);
}

void ivelultrkb_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_kbmcu, 4'000'000); // unknown clock
	m_kbmcu->set_addrmap(AS_PROGRAM, &ivelultrkb_device::mem_map);

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(ivelultrkb_device::pia_pa_r));
	m_pia->readpb_handler().set_ioport("CTRL").mask(1).lshift(7);
	m_pia->writepb_handler().set(FUNC(ivelultrkb_device::b_w)).mask(0x7f);
	m_pia->ca2_handler().set(FUNC(ivelultrkb_device::mode_w));
	m_pia->cb2_handler().set(FUNC(ivelultrkb_device::strobe_w));
}

ROM_START(ivelultrkb)
	ROM_REGION(0x800, "kbmcu", 0)
	// "KBDULTRA 6802/8507 TIBOR & CO DESIGN"
	ROM_LOAD("ultra4.bin", 0x000, 0x800, CRC(3dce51ac) SHA1(676b6e775d5159049cae5b6143398ec7b2bf437a))
ROM_END

const tiny_rom_entry *ivelultrkb_device::device_rom_region() const
{
	return ROM_NAME(ivelultrkb);
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(IVEL_ULTRA_KEYBOARD, device_a2kbd_interface, ivelultrkb_device, "ivelultrkb", "Ivel Ultra keyboard")
