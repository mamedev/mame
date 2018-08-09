// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Driver for Micro-Term ACT-5A and other F8-based terminals.

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/f8/f8.h"
#include "machine/ay31015.h"
#include "machine/f3853.h"
#include "screen.h"

class microterm_f8_state : public driver_device
{
public:
	microterm_f8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_io(*this, "io")
		, m_aux(*this, "aux")
		, m_screen(*this, "screen")
		, m_kbdecode(*this, "kbdecode")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%u", 0U)
		, m_modifiers(*this, "MODIFIERS")
	{ }

	void act5a(machine_config &config);

private:
	virtual void machine_start() override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	F3853_INTERRUPT_REQ_CB(f3853_interrupt);

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	bool poll_keyboard();
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_READ8_MEMBER(port01_r);

	void f8_mem(address_map &map);
	void f8_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ay51013_device> m_uart;
	required_device<rs232_port_device> m_io;
	required_device<rs232_port_device> m_aux;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_kbdecode;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<11> m_keys;
	required_ioport m_modifiers;

	u8 m_port00;
	u8 m_keylatch;
	std::unique_ptr<u8[]> m_vram;
};

void microterm_f8_state::machine_start()
{
	m_port00 = 0;
	m_keylatch = 0;
	m_vram = make_unique_clear<u8[]>(0xc00); // 6x MM2114 with weird addressing

	save_item(NAME(m_port00));
	save_item(NAME(m_keylatch));
	save_pointer(NAME(m_vram), 0xc00);
}

u32 microterm_f8_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	unsigned y = cliprect.top();
	offs_t rowbase = (y / 12) * 0x80;
	unsigned line = y % 12;
	if (line >= 6)
		line += 2;

	while (y <= cliprect.bottom())
	{
		for (unsigned x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u8 ch = m_vram[rowbase + (x / 9)];
			u8 dots = m_chargen[(line << 7) | (ch & 0x7f)];
			bitmap.pix32(y, x) = BIT(dots, 8 - (x % 9)) ? rgb_t::white() : rgb_t::black();
		}

		y++;
		line++;
		if ((line & 7) >= 6)
		{
			line = (line + 2) & 15;
			if (line == 0)
				rowbase += 0x80;
		}
	}

	return 0;
}

F3853_INTERRUPT_REQ_CB(microterm_f8_state::f3853_interrupt)
{
	m_maincpu->set_input_line_and_vector(F8_INPUT_LINE_INT_REQ, level ? ASSERT_LINE : CLEAR_LINE, addr);
}

READ8_MEMBER(microterm_f8_state::vram_r)
{
	return m_vram[(offset & 0x1f00) >> 1 | (offset & 0x007f)];
}

WRITE8_MEMBER(microterm_f8_state::vram_w)
{
	m_vram[(offset & 0x1f00) >> 1 | (offset & 0x007f)] = data;
}

bool microterm_f8_state::poll_keyboard()
{
	for (int row = 0; row < 11; row++)
	{
		u8 polled = m_keys[row]->read();
		if (polled == 0xff)
			continue;

		offs_t keyaddr = (m_modifiers->read() << 7) | (row << 3);
		while (BIT(polled, 0))
		{
			polled >>= 1;
			keyaddr++;
		}
		m_keylatch = m_kbdecode[keyaddr];
		return true;
	}

	return false;
}

READ8_MEMBER(microterm_f8_state::key_r)
{
	return m_keylatch;
}

READ8_MEMBER(microterm_f8_state::port00_r)
{
	u8 flags = m_port00;

	// ???
	if (m_uart->tbmt_r())
		flags |= 0x02;

	return flags;
}

WRITE8_MEMBER(microterm_f8_state::port00_w)
{
	m_port00 = data;
}

READ8_MEMBER(microterm_f8_state::port01_r)
{
	u8 flags = 0;

	// Some timing flag (not necessarily HBLANK?)
	if (!m_screen->hblank())
		flags |= 0x80;

	// Keyboard polling
	if (poll_keyboard())
		flags |= 0x40;

	// ???
	if (!m_screen->vblank())
		flags |= 0x20;

	return flags;
}

void microterm_f8_state::f8_mem(address_map &map)
{
	map(0x0000, 0x0bff).rom().region("maincpu", 0);
	map(0x4000, 0x407f).select(0x1f00).rw(FUNC(microterm_f8_state::vram_r), FUNC(microterm_f8_state::vram_w));
	map(0x8000, 0x8000).rw(m_uart, FUNC(ay51013_device::receive), FUNC(ay51013_device::transmit));
	map(0xf000, 0xf000).r(FUNC(microterm_f8_state::key_r));
}

void microterm_f8_state::f8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).rw(FUNC(microterm_f8_state::port00_r), FUNC(microterm_f8_state::port00_w));
	map(0x01, 0x01).r(FUNC(microterm_f8_state::port01_r)).nopw();
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}

static INPUT_PORTS_START(act5a)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key AC")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key AD")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * -") PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". > 9") PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", < 8") PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M 7") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L 6") PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K 5") PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J 4") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O 3") PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I 2") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U 1") PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( 0") PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key 8D")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key AE")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B8")

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key B7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key FD")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key FC")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key FF")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key FE")

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x7")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x0")

	PORT_START("KEY10")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key x9")

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPNAME(0xff, 0xfd, "Printer Data Rate") PORT_DIPLOCATION("S1:8,7,6,5,4,3,2,1")
	PORT_DIPSETTING(0x7f, "110")
	PORT_DIPSETTING(0xbf, "300")
	PORT_DIPSETTING(0xdf, "600")
	PORT_DIPSETTING(0xef, "1,200")
	PORT_DIPSETTING(0xf7, "2,400")
	PORT_DIPSETTING(0xfb, "4,800")
	PORT_DIPSETTING(0xfd, "9,600")
	PORT_DIPSETTING(0xfe, "19,200")

	PORT_START("DSW2")
	PORT_DIPNAME(0xff, 0xfd, "I/O Data Rate") PORT_DIPLOCATION("S2:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(0x7f, "110")
	PORT_DIPSETTING(0xbf, "300")
	PORT_DIPSETTING(0xdf, "600")
	PORT_DIPSETTING(0xef, "1,200")
	PORT_DIPSETTING(0xf7, "2,400")
	PORT_DIPSETTING(0xfb, "4,800")
	PORT_DIPSETTING(0xfd, "9,600")
	PORT_DIPSETTING(0xfe, "19,200")

	PORT_START("DSW3")
	PORT_DIPNAME(0x001, 0x001, "Conversation Mode") PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(0x001, "Half Duplex")
	PORT_DIPSETTING(0x000, "Full Duplex")
	PORT_DIPNAME(0x006, 0x006, "Protected Field Attribute") PORT_DIPLOCATION("S3:2,3")
	PORT_DIPSETTING(0x006, "Reduced Intensity")
	PORT_DIPSETTING(0x002, "Blinking")
	PORT_DIPSETTING(0x004, "Reverse Video")
	PORT_DIPSETTING(0x000, "Underline")
	PORT_DIPNAME(0x008, 0x008, "Display Null Character") PORT_DIPLOCATION("S3:4")
	PORT_DIPSETTING(0x000, DEF_STR(Off))
	PORT_DIPSETTING(0x008, DEF_STR(On))
	PORT_DIPNAME(0x010, 0x010, "8th Bit Transmit") PORT_DIPLOCATION("S3:5")
	PORT_DIPSETTING(0x000, "0 (Space)")
	PORT_DIPSETTING(0x010, "1 (Mark)")
	PORT_DIPNAME(0x060, 0x060, "Word Length") PORT_DIPLOCATION("S3:6,7")
	PORT_DIPSETTING(0x000, "5 Bits")
	PORT_DIPSETTING(0x040, "6 Bits")
	PORT_DIPSETTING(0x020, "7 Bits")
	PORT_DIPSETTING(0x060, "8 Bits")
	PORT_DIPNAME(0x280, 0x280, "Parity") PORT_DIPLOCATION("S3:8,10")
	PORT_DIPSETTING(0x280, "None")
	PORT_DIPSETTING(0x080, "Even")
	PORT_DIPSETTING(0x000, "Odd")
	PORT_DIPNAME(0x100, 0x100, "Number of Stop Bits") PORT_DIPLOCATION("S3:9")
	PORT_DIPSETTING(0x000, "1")
	PORT_DIPSETTING(0x100, "2")

	// TODO: W1-W5 jumper settings
INPUT_PORTS_END

void microterm_f8_state::act5a(machine_config &config)
{
	cpu_device &maincpu(F8(config, "maincpu", 2_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &microterm_f8_state::f8_mem);
	maincpu.set_addrmap(AS_IO, &microterm_f8_state::f8_io);

	f3853_device &smi(F3853(config, "smi", 2_MHz_XTAL));
	smi.set_interrupt_req_callback(f3853_device::interrupt_req_delegate(FUNC(microterm_f8_state::f3853_interrupt), this));

	AY51013(config, m_uart, 0);
	m_uart->read_si_callback().set(m_io, FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set(m_io, FUNC(rs232_port_device::write_txd));
	m_uart->write_dav_callback().set("smi", FUNC(f3853_device::set_external_interrupt_in_line)).invert();
	m_uart->set_auto_rdav(true);

	RS232_PORT(config, m_io, default_rs232_devices, nullptr);
	RS232_PORT(config, m_aux, default_rs232_devices, nullptr);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16.572_MHz_XTAL, 918, 0, 720, 301, 0, 288); // more or less guessed
	screen.set_screen_update(FUNC(microterm_f8_state::screen_update));
}

ROM_START(act5a)
	ROM_REGION(0x0c00, "maincpu", 0) // xtals 16.572MHz, 2.000MHz
	ROM_LOAD("act5a_2708.u1",  0x0000, 0x0400, CRC(ad3bfa5a) SHA1(723c7bfefbf96177171b8e58a8e20ee69daa27f0))
	ROM_LOAD("act5a_2708.u12", 0x0400, 0x0400, CRC(be4a148d) SHA1(69bf838ed9fccdf7f225bc380bcce8e7e0bd88bc))
	ROM_LOAD("act5a_2708.u22", 0x0800, 0x0400, CRC(f0ec3b9f) SHA1(7785eba9993c23a767b84fff2c44d5cb6210ad80))

	ROM_REGION(0x2000, "kbdecode", 0)
	ROM_LOAD("act5a_9316.u39", 0x0000, 0x2000, CRC(6f9eac71) SHA1(0488bdd19a6bff4e869a3480c7e9d8c5ca9938eb))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("act5a_9316.u55", 0x0000, 0x2000, CRC(8f96b7c8) SHA1(652d420ab5be9412cae322cd1799f8a9e3959c44))
ROM_END

//COMP(1976, act4, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-IV", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
//COMP(1978, act5, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-V", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1980, act5a, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-5A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
