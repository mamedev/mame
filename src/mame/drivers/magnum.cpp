// license:BSD-3-Clause
// copyright-holders:Carl

// Dulmont Magnum
// Additional info https://www.youtube.com/watch?v=st7H_vqSaQc and
// http://www.eevblog.com/forum/blog/eevblog-949-vintage-australian-made-laptop-teardown/msg1080508/#msg1080508
// TODO: cartridge dumps

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/cdp1879.h"
#include "machine/wd_fdc.h"
#include "video/hd61830.h"
#include "video/i8275.h"
#include "sound/beep.h"
#include "emupal.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

class magnum_state : public driver_device
{
public:
	magnum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_beep(*this, "beep")
		, m_shift(*this, "SHIFT")
	{
	}
	void magnum(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(keypress);
protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	void beep_w(u8 data);
	u8 sysctl_r(offs_t offset);
	void sysctl_w(offs_t offset, u8 data);
	u16 irqstat_r();
	void port50_w(u16 data);
	DECLARE_WRITE_LINE_MEMBER(rtcirq_w);

	void check_irq();
	void magnum_io(address_map &map);
	void magnum_map(address_map &map);
	void magnum_lcdc(address_map &map);

	required_device<i80186_cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	required_ioport m_shift;

	u8 m_key;
	bool m_wake, m_rtcirq, m_keybirq;
};

void magnum_state::check_irq()
{
	if(m_rtcirq || m_keybirq)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_maincpu->int0_w(ASSERT_LINE);
		m_wake = true;
	}
	else
		m_maincpu->int0_w(CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(magnum_state::keypress)
{
	if(newval != oldval)
	{
		m_key = (uint8_t)(param & 0xff) | (m_shift->read() & 0xc ? 0 : 0x80);
		m_keybirq = true;
		check_irq();
	}
}

static INPUT_PORTS_START(magnum)
	PORT_START("ROW.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 0)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 1)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 4)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 5)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 8)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 9)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 12)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 13)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 2) // all 0x4 bits in every nibble also repeated in row 4+
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 3) // all 0x8 bits in every nibble also repeated in row 4+
	PORT_START("ROW.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 16)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 17)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 20)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 21)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 24)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 25)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 28)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 29)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 18)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 19)
	PORT_START("ROW.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 32)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 33)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 36)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 37)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 40)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 41)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 44)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 45)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 34)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 35)
	PORT_START("ROW.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 48)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 49)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 52)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 53)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 56)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 57)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 60)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 61)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 50)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 51)
	PORT_START("ROW.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 64)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 65)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 68)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 69)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 72)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 73)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 76)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 77)
	PORT_START("ROW.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 80)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 81)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 84)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 85)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 88)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 89)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 92)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 93)
	PORT_START("ROW.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 96)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 97)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 100)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 101)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 104)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 105)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 108)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 109)
	PORT_START("ROW.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 112)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 113)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 116)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 117)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 120)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 121)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 124)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, magnum_state, keypress, 125)
	PORT_START("SHIFT")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
INPUT_PORTS_END

void magnum_state::machine_start()
{
}

void magnum_state::machine_reset()
{
	m_wake = false;
	m_rtcirq = false;
	m_keybirq = false;
}

void magnum_state::beep_w(u8 data)
{
	if (data & ~1) logerror("beep_w unmapped bits %02x\n", data);
	m_beep->set_state(BIT(data, 0));
}

void magnum_state::magnum_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // fixed 256k for now
	map(0xe0000, 0xfffff).rom().region("bios", 0);
}

u8 magnum_state::sysctl_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return m_key;
		case 1:
		{
			u8 ret = m_keybirq ? 1 : 0;
			m_keybirq = false;
			check_irq();
			return ret | (m_shift->read() & 3 ? 0 : 4);
		}
	}
	return 0;
}

void magnum_state::sysctl_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			break;
		case 2:
			m_maincpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 4:
			break;
		case 6:
			break;
		case 12:
			break;
	}
}

/* IRQs
 * bit  0 -
 *      1 -
 *      2 -
 *      3 -
 *      4 -
 *      5 - Keyboard?
 *      6 - RTC?
 *      7 -
 *      8 - UART1?
 *      9 - UART2?
 *      10- Reset?
 *      11-14 ...
 *      15- CRTC?
 */

u16 magnum_state::irqstat_r()
{
	u16 ret = m_wake ? 0 : 0x400;
	ret |= m_rtcirq ? 0x40 : 0;
	ret |= m_keybirq ? 0x20 : 0;
	m_wake = false;
	return ret;
}

void magnum_state::port50_w(u16 data)
{
}

WRITE_LINE_MEMBER(magnum_state::rtcirq_w)
{
	m_rtcirq = state == ASSERT_LINE;
	check_irq();
}

void magnum_state::magnum_io(address_map &map)
{
	map.unmap_value_high();
	//map(0x000a, 0x000b) cdp1854 1
	//map(0x000e, 0x000f) cpd1854 2
	//map(0x0014, 0x0017).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write)).umask16(0x00ff);
	map(0x0018, 0x0019).rw("lcdc2", FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x001a, 0x001b).rw("lcdc2", FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x001c, 0x001d).rw("lcdc1", FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x001e, 0x001f).rw("lcdc1", FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x0040, 0x004f).rw(FUNC(magnum_state::sysctl_r), FUNC(magnum_state::sysctl_w));
	map(0x0050, 0x0051).rw(FUNC(magnum_state::irqstat_r), FUNC(magnum_state::port50_w));
	map(0x0056, 0x0056).w(FUNC(magnum_state::beep_w));
	map(0x0080, 0x008f).rw("rtc", FUNC(cdp1879_device::read), FUNC(cdp1879_device::write)).umask16(0x00ff);
	//map(0x0100, 0x0107).rw("fdc", FUNC(wd1793_device::read), FUNC(wd1793_device::write)).umask16(0x00ff);
}

void magnum_state::magnum_lcdc(address_map &map)
{
	map(0x0000, 0x027f).ram();
}

void magnum_state::magnum(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &magnum_state::magnum_map);
	m_maincpu->set_addrmap(AS_IO, &magnum_state::magnum_io);

	CDP1879(config, "rtc", XTAL(32'768)).irq_callback().set(FUNC(magnum_state::rtcirq_w));

	screen_device &screen1(SCREEN(config, "screen1", SCREEN_TYPE_LCD));
	screen1.set_refresh_hz(50);
	screen1.set_screen_update("lcdc1", FUNC(hd61830_device::screen_update));
	screen1.set_size(6*40, 9*16);
	screen1.set_visarea(0, 6*40-1, 0, 8*16-1);
	screen1.set_palette("palette");

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_LCD));
	screen2.set_refresh_hz(50);
	screen2.set_screen_update("lcdc2", FUNC(hd61830_device::screen_update));
	screen2.set_size(6*40, 9*16);
	screen2.set_visarea(0, 6*40-1, 0, 8*16-1);
	screen2.set_palette("palette");

	hd61830_device &lcdc1(HD61830(config, "lcdc1", 1000000)); // unknown clock
	lcdc1.set_addrmap(0, &magnum_state::magnum_lcdc);
	lcdc1.set_screen("screen1");

	hd61830_device &lcdc2(HD61830(config, "lcdc2", 1000000)); // unknown clock
	lcdc2.set_addrmap(0, &magnum_state::magnum_lcdc);
	lcdc2.set_screen("screen2");

	//I8275(config, "crtc", 3000000); // unknown clock

	//WD1793(config, "fdc", 1000000); // nothing known, type or if any disks even exist, port 0x44 is possibly motor control

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beep, 500).add_route(ALL_OUTPUTS, "speaker", 0.50); // frequency is guessed
}

ROM_START( magnum )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE("a1.7.88.bin", 0x00000, 0x4000, CRC(57882427) SHA1(97637b65ca43eb9d3bba546fb8ca701ba25ade8d))
	ROM_LOAD16_BYTE("a1.7.81.bin", 0x00001, 0x4000, CRC(949f53a8) SHA1(b339f1495d9af7dfff0c3a2c24789631f9d1265b))
	ROM_LOAD16_BYTE("a1.7.87.bin", 0x08000, 0x4000, CRC(25036dda) SHA1(20bc3782a66855b20cb0abe1051fa2eb50c7a860))
	ROM_LOAD16_BYTE("a1.7.82.bin", 0x08001, 0x4000, CRC(ecf387d8) SHA1(8b42f6ab030afb51f21f4a56c62e5acf7d074066))
	ROM_LOAD16_BYTE("a1.7.86.bin", 0x10000, 0x4000, CRC(c80b3a6b) SHA1(0f0d2cb653bbeff8f3bab6d20dc30c220a67a315))
	ROM_LOAD16_BYTE("a1.7.83.bin", 0x10001, 0x4000, CRC(51f56d78) SHA1(df717eada5e6439b1c01d91bd0ea009cd0f8ddfa))
	ROM_LOAD16_BYTE("a1.7.85.bin", 0x18000, 0x4000, CRC(f5dd5407) SHA1(af2edf7a658bcf648acb8be9f13849f838d96214))
	ROM_LOAD16_BYTE("a1.7.84.bin", 0x18001, 0x4000, CRC(b3434bb0) SHA1(8000a7aca8fc505b136a618d9eb210c50393eff1))

	ROM_REGION(0x1000, "char", 0)
	ROM_LOAD("dulmontcharrom.bin", 0x0000, 0x1000, CRC(9dff89bf) SHA1(d359aeba7f0b0c81accf3bca25e7da636c033721))
ROM_END

COMP( 1983, magnum, 0, 0, magnum, magnum, magnum_state, empty_init, "Dulmont", "Magnum", MACHINE_IMPERFECT_SOUND)
