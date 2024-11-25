// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hazeltine 1420 terminal.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/i8243.h"
#include "machine/input_merger.h"
#include "machine/ins8250.h"
#include "sound/beep.h"
#include "video/dp8350.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hazl1420_state : public driver_device
{
public:
	hazl1420_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_ioexp(*this, "ioexp%u", 0U)
		, m_mainint(*this, "mainint")
		, m_crtc(*this, "crtc")
		, m_beeper(*this, "beep")
		, m_videoram(*this, "videoram")
		, m_keys(*this, "KEY%u", 0U)
	{
	}

	void hazl1420(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void p1_w(u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	void videoram_w(offs_t offset, u8 data);
	void crtc_w(offs_t offset, u8 data);
	void p6_w(u8 data);
	void p7_w(u8 data);

	u8 key_r();

	void crtc_lbre_w(int state);
	void crtc_vblank_w(int state);

	void prog_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void bank_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mcs48_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device_array<i8243_device, 2> m_ioexp;
	required_device<input_merger_device> m_mainint;
	required_device<dp8350_device> m_crtc;
	required_device<beep_device> m_beeper;
	required_shared_ptr<u8> m_videoram;
	required_ioport_array<10> m_keys;
};

void hazl1420_state::p1_w(u8 data)
{
	m_ioexp[0]->cs_w((data & 0xc0) == 0x80 ? 0 : 1);
	m_ioexp[1]->cs_w((data & 0xc0) == 0xc0 ? 0 : 1);

	// acknowledge CRTC interrupts
	if (BIT(data, 4))
		m_mainint->in_w<0>(0);
}

u8 hazl1420_state::p2_r()
{
	u8 result = 0xe0 | (!m_crtc->lbre_r() << 4);
	result |= m_ioexp[0]->p2_r() & m_ioexp[1]->p2_r();
	return result;
}

void hazl1420_state::p2_w(u8 data)
{
	m_bankdev->set_bank((data & 0xe0) >> 1 | (data & 0x0f));
}

void hazl1420_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	if (BIT(m_maincpu->p1_r(), 5))
		m_videoram[offset ^ 1] = data;
}

void hazl1420_state::crtc_w(offs_t offset, u8 data)
{
	// CRTC registers are loaded only during vertical blanking period
	m_crtc->register_load(bitswap<2>(offset >> 12, 0, 1), offset & 0xfff);
}

void hazl1420_state::p6_w(u8 data)
{
	m_beeper->set_state(!BIT(data, 1));
}

void hazl1420_state::p7_w(u8 data)
{
}

u8 hazl1420_state::key_r()
{
	u8 row = m_maincpu->p1_r() & 0x0f;
	return (row < 10) ? m_keys[row]->read() : 0xff;
}

void hazl1420_state::prog_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void hazl1420_state::io_map(address_map &map)
{
	map(0x00, 0xff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void hazl1420_state::bank_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("videoram").w(FUNC(hazl1420_state::videoram_w));
	map(0x0800, 0x0807).mirror(0x10).rw("ace", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x0c00, 0x0cff).ram(); // optional input buffer?
	map(0x4000, 0x7fff).w(FUNC(hazl1420_state::crtc_w));
}

void hazl1420_state::machine_start()
{
}

void hazl1420_state::crtc_lbre_w(int state)
{
	if (!state && !m_crtc->vblank_r() && !BIT(m_maincpu->p1_r(), 4))
		m_mainint->in_w<0>(1);
}

void hazl1420_state::crtc_vblank_w(int state)
{
	if (state && !BIT(m_maincpu->p1_r(), 4))
		m_mainint->in_w<0>(1);
}

u32 hazl1420_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START(hazl1420)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_COMMA_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('/') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// DIP switches are on access panel above keyboard
	// "SW1" and "SW2" are not actual names

	PORT_START("INP4")
	PORT_DIPNAME(0x7, 0x6, "Baud Rate") PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(0x0, "110")
	PORT_DIPSETTING(0x1, "300")
	PORT_DIPSETTING(0x2, "600")
	PORT_DIPSETTING(0x3, "1200")
	PORT_DIPSETTING(0x4, "1800")
	PORT_DIPSETTING(0x5, "2400")
	PORT_DIPSETTING(0x6, "4800")
	PORT_DIPSETTING(0x7, "9600")
	PORT_DIPNAME(0x8, 0x0, "Lead-In") PORT_DIPLOCATION("SW1:6") // not verified
	PORT_DIPSETTING(0x0, "ESC")
	PORT_DIPSETTING(0x8, "~")

	PORT_START("INP5")
	PORT_DIPNAME(0x3, 0x3, "Parity") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(0x0, "Odd")
	PORT_DIPSETTING(0x1, "Even")
	PORT_DIPSETTING(0x2, "1")
	PORT_DIPSETTING(0x3, "0")
	PORT_DIPNAME(0x4, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INP6")
	PORT_DIPNAME(0x1, 0x0, "Cursor") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x0, "Wraparound")
	PORT_DIPSETTING(0x1, "No Wrap")
	PORT_DIPNAME(0x2, 0x2, "On Line") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x4, "Automatic LF/CR") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x4, "Auto LF")
	PORT_DIPSETTING(0x0, "Carriage Return")
	PORT_DIPNAME(0x8, 0x8, "Communication Mode") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x0, "Half Duplex")
	PORT_DIPSETTING(0x8, "Full Duplex")

	PORT_START("INP7")
	PORT_DIPNAME(0x1, 0x1, "Font") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x1, "Upper/Lower Case")
	PORT_DIPSETTING(0x0, "Upper Case Only")
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("All Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("UNUSED")
	PORT_DIPNAME(0x1, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x1, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x2, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x2, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x4, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x4, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
	PORT_DIPNAME(0x8, 0x0, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x8, DEF_STR(Off))
	PORT_DIPSETTING(0x0, DEF_STR(On))
INPUT_PORTS_END

void hazl1420_state::hazl1420(machine_config &config)
{
	I8049(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hazl1420_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &hazl1420_state::io_map);
	m_maincpu->p1_out_cb().set(FUNC(hazl1420_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(hazl1420_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(hazl1420_state::p2_w));
	m_maincpu->p2_out_cb().append(m_ioexp[0], FUNC(i8243_device::p2_w));
	m_maincpu->p2_out_cb().append(m_ioexp[1], FUNC(i8243_device::p2_w));
	m_maincpu->prog_out_cb().set(m_ioexp[0], FUNC(i8243_device::prog_w));
	m_maincpu->prog_out_cb().append(m_ioexp[1], FUNC(i8243_device::prog_w));
	m_maincpu->t0_in_cb().set(m_crtc, FUNC(dp8350_device::vblank_r));
	m_maincpu->t1_in_cb().set("ace", FUNC(ins8250_device::intrpt_r));

	INPUT_MERGER_ANY_HIGH(config, m_mainint);
	m_mainint->output_handler().set_inputline(m_maincpu, MCS48_INPUT_IRQ);

	ADDRESS_MAP_BANK(config, m_bankdev);
	m_bankdev->set_addrmap(0, &hazl1420_state::bank_map);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(15);
	m_bankdev->set_stride(0x100);

	I8243(config, m_ioexp[0]);
	m_ioexp[0]->p4_in_cb().set_ioport("INP4");
	m_ioexp[0]->p5_in_cb().set_ioport("INP5");
	m_ioexp[0]->p6_out_cb().set(FUNC(hazl1420_state::p6_w));
	m_ioexp[0]->p7_out_cb().set(FUNC(hazl1420_state::p7_w));

	I8243(config, m_ioexp[1]);
	m_ioexp[1]->p4_in_cb().set(FUNC(hazl1420_state::key_r));
	m_ioexp[1]->p5_in_cb().set(FUNC(hazl1420_state::key_r)).rshift(4);
	m_ioexp[1]->p6_in_cb().set_ioport("INP6");
	m_ioexp[1]->p7_in_cb().set_ioport("INP7");

	ins8250_device &ace(INS8250(config, "ace", 2'764'800));
	ace.out_int_callback().set(m_mainint, FUNC(input_merger_device::in_w<1>));
	ace.out_tx_callback().set("eia", FUNC(rs232_port_device::write_txd));
	ace.out_rts_callback().set("eia", FUNC(rs232_port_device::write_rts));
	ace.out_dtr_callback().set("eia", FUNC(rs232_port_device::write_dtr));
	//ace.baudout_callback().set("eia", FUNC(rs232_port_device::write_etc)); // 16x rate output (unemulated)

	DP8350(config, m_crtc, 10.92_MHz_XTAL).set_screen("screen");
	m_crtc->lbre_callback().set(FUNC(hazl1420_state::crtc_lbre_w));
	m_crtc->vblank_callback().set(FUNC(hazl1420_state::crtc_vblank_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hazl1420_state::screen_update));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1000).add_route(ALL_OUTPUTS, "mono", 1.00);

	rs232_port_device &eia(RS232_PORT(config, "eia", default_rs232_devices, nullptr));
	eia.rxd_handler().set("ace", FUNC(ins8250_device::rx_w));
	eia.cts_handler().set("ace", FUNC(ins8250_device::cts_w));
	eia.dsr_handler().set("ace", FUNC(ins8250_device::dsr_w));
	eia.dcd_handler().set("ace", FUNC(ins8250_device::dcd_w));
}

ROM_START(hazl1420)
	ROM_REGION(0x0800, "maincpu_internal", 0)
	// This internal ROM seems to belong to some earlier program revision
	ROM_LOAD("8049h.u19", 0x0000, 0x0800, CRC(81beb6de) SHA1(f272d1277f100af92384a4c4cec2c9db9424b603))

	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("2716.u10", 0x0000, 0x0800, CRC(7c40ba24) SHA1(7575225adf1a06d66b079efcf0f4f9ee77fbddd4))
	ROM_LOAD("8316.u11", 0x0800, 0x0800, CRC(1c112f09) SHA1(fa4973e99c6d66809cffef009c4869787089a774))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("8316.u23", 0x0000, 0x0800, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(1979, hazl1420, 0, 0, hazl1420, hazl1420, hazl1420_state, empty_init, "Hazeltine", "1420 Video Display Terminal", MACHINE_NOT_WORKING)
