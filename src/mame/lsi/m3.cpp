// license: BSD-3-Clause
// copyright-holders: Robbbert, Dirk Best
/***************************************************************************

    LSI M-THREE

    Models:
    - M-THREE/100 (SA400, 5.25" single sided)
    - M-THREE/110 (SA410, 5.25" single sided)
    - M-THREE/150 (SA450, 5.25" double sided)
    - M-THREE/160 (SA460, 5.25" double sided)
    - M-THREE/200 (SA800, 8" single sided)
    - M-THREE/250 (SA850, 8" double sided) [emulated by default]
    - M-THREE/320 (SA800, 8" single sided, Winchester 5 MB)
    - M-THREE/325 (SA850, 8" double sided, Winchester 5 MB)
    - M-THREE/340 (SA800, 8" single sided, Winchester 10 MB)
    - M-THREE/345 (SA850, 8" double sided, Winchester 10 MB)

    Hardware:
    - MK3880N-IRL (Z80)
    - 64 KB RAM
    - Z80 CTC
    - 2x D8255AC PPI
    - D8251A
    - MC6845P CRTC
    - FD1793 FDC
    - XTAL X1 21.??? Mhz (unreadable)
    - XTAL X2 16 MHz
    - XTAL X3 4.9152 MHz
    - Keyboard XTAL 6.1?? MHz (assumed to be 6.144 MHz)

    TODO:
    - Initial PC is currently hacked to f000
    - Verify/fix floppy hookup (CPU needs to be overclocked?)
    - Printer interface
    - Buzzer
    - Map the rest of the keys, verify existing keys
    - Switch FDC to 1 MHz for 5.25" drives

    Notes:
    - No offical software available, but a custom version of CP/M
    - Y to boot from floppy, ESC to enter monitor, any other key to
      boot from IDE

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "imagedev/floppy.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m3_state : public driver_device
{
public:
	m3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctc(*this, "ctc"),
		m_ppi(*this, "ppi%u", 0U),
		m_chargen(*this, "chargen"),
		m_vram(*this, "vram"),
		m_palette(*this, "palette"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_kbdmcu(*this, "kbdmcu"),
		m_special(*this, "SPECIAL"),
		m_keys(*this, "K%02u", 0U)
	{ }

	void m3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device_array<i8255_device, 2> m_ppi;
	required_region_ptr<uint8_t> m_chargen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<palette_device> m_palette;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<i8035_device> m_kbdmcu;
	required_ioport m_special;
	required_ioport_array<16> m_keys;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void kbd_mem_map(address_map &map) ATTR_COLD;
	void kbd_io_map(address_map &map) ATTR_COLD;

	int kbd_t0_r();
	int kbd_t1_r();
	uint8_t kbd_p1_r();
	void kbd_p1_w(uint8_t data);
	void kbd_p2_w(uint8_t data);
	void kbd_data_w(uint8_t data);
	int m_kbd_col = 0;
	int m_kbd_row = 0;
	uint8_t m_kbd_data = 0U;

	void ppi2_pa_w(uint8_t data);
	uint8_t ppi2_pb_r();

	MC6845_UPDATE_ROW(crtc_update_row);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t fdc_data_r(offs_t offset);
	void fdc_data_w(offs_t offset, uint8_t data);
	bool m_nmi_taken = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void m3_state::mem_map(address_map &map)
{
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("vram");
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void m3_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x84, 0x84).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x85, 0x85).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x88, 0x88).rw(m_fdc, FUNC(fd1793_device::status_r), FUNC(fd1793_device::cmd_w));
	map(0x89, 0x89).rw(m_fdc, FUNC(fd1793_device::track_r), FUNC(fd1793_device::track_w));
	map(0x8a, 0x8a).rw(m_fdc, FUNC(fd1793_device::sector_r), FUNC(fd1793_device::sector_w));
	map(0x8b, 0x8b).rw(FUNC(m3_state::fdc_data_r), FUNC(m3_state::fdc_data_w));
	map(0x8c, 0x8d).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x90, 0x93).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x94, 0x97).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void m3_state::kbd_mem_map(address_map &map)
{
	map(0x000, 0x7ff).rom().mirror(0x800).region("keyboard", 0);
}

void m3_state::kbd_io_map(address_map &map)
{
	map(0x30, 0x30).mirror(0xf).w(FUNC(m3_state::kbd_data_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

// unmapped keys:
//
// INIT PROG F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14
// INS_CHAR DEL_CHAR INS_LINE DEL_LINE CLEAR_FORE CLEAR_PAGE PROT_MODE BLOCK_MODE
// SEND_LINE SEND_FORE SEND_PAGE PAGE_FWD UP PAGE_BACK LEFT HOME RIGHT
// BREAK SET_TAB DOWN CLEAR_TAB

static INPUT_PORTS_START( m3 )
	PORT_START("SPECIAL")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // or shift-lock?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Repeat")

	PORT_START("K00")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": (?)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("85")

	PORT_START("K01")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0f")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("86")

	PORT_START("K02")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("05")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("96")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("89")

	PORT_START("K03")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 (?)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("95")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8a")

	PORT_START("K04")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("11")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("94")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8b")

	PORT_START("K05")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("cd")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("93")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') // actually ¦
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8c")

	PORT_START("K06")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("92")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)   PORT_CHAR(9)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8d")

	PORT_START("K07")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("91")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8e")

	PORT_START("K08")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)   PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("90")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("bc")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF")          PORT_CHAR('\n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8f")

	PORT_START("K09")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("cd")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("87")

	PORT_START("K10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("97")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("88")

	PORT_START("K11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("03")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("84")

	PORT_START("K12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("cd")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b6")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("83")

	PORT_START("K13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR(0xa3) // £
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ea")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("82")

	PORT_START("K14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("02")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ba")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b9")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("81")

	PORT_START("K15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("bb")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)   PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("80")
INPUT_PORTS_END

int m3_state::kbd_t0_r()
{
	// outputs 26 alternating 0 and 1 to i/o port 0x00 when active
	return 1;
}

int m3_state::kbd_t1_r()
{
	// checked right before outputing a new keycode
	return 0;
}

uint8_t m3_state::kbd_p1_r()
{
	uint8_t data = 0;

	// ---4----  key down
	// ----3---  repeat
	// -----2--  control
	// ------1-  shift
	// -------0  caps-lock, shift-lock?

	data |= BIT(m_keys[m_kbd_col]->read(), m_kbd_row) ? 0x00 : 0x10;
	data |= m_special->read();

	return data;
}

void m3_state::kbd_p1_w(uint8_t data)
{
	// 765-----  row select

	m_kbd_row = data >> 5;
}

void m3_state::kbd_p2_w(uint8_t data)
{
	// 7654----  column select
	// ----3210  unused

	m_kbd_col = data >> 4;
}

void m3_state::kbd_data_w(uint8_t data)
{
	m_kbd_data = data;

	m_ppi[1]->pc2_w(0);
	m_ppi[1]->pc2_w(1);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_UPDATE_ROW( m3_state::crtc_update_row )
{
	rgb_t const *const pen = m_palette->palette()->entry_list_raw();

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_vram[mem];
		uint8_t gfx = m_chargen[((chr << 4) | ra) & 0x7ff];

		// invert?
		if (BIT(chr, 7))
			gfx ^= 0xff;

		// cursor?
		if (x == cursor_x)
			gfx ^= 0xff;

		// draw 7 pixels of the character
		for (int i = 0; i < 7; i++)
			bitmap.pix(y, x * 7 + i) = pen[BIT(gfx, 6 - i)];
	}
}

static const gfx_layout charlayout =
{
	7, 10,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, charlayout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void m3_state::fdc_intrq_w(int state)
{
	if (state)
	{
		m_nmi_taken = false;
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}

	m_ctc->trg1(state);
}

void m3_state::fdc_drq_w(int state)
{
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (state && !m_nmi_taken)
	{
		m_nmi_taken = true;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

uint8_t m3_state::fdc_data_r(offs_t offset)
{
	if ((m_fdc->drq_r() == 0) && m_nmi_taken)
	{
		// cpu tries to read data without drq, halt it and reset pc
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_state_int(Z80_PC, m_maincpu->state_int(Z80_PC) - 2);

		return 0;
	}

	return m_fdc->data_r();
}

void m3_state::fdc_data_w(offs_t offset, uint8_t data)
{
	if ((m_fdc->drq_r() == 0) && m_nmi_taken)
	{
		// cpu tries to write data without drq, halt it and reset pc
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_state_int(Z80_PC, m_maincpu->state_int(Z80_PC) - 2);

		return;
	}

	m_fdc->data_w(data);
}

void m3_state::ppi2_pa_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// 7-------  not used?
	// -6------  buzzer
	// --5-----  not used?
	// ---4----  unknown (motor?)
	// ----3---  unknown
	// -----2--  floppy side
	// ------10  drive select

	logerror("ppi2_pa_w: %02x\n", data);

	if (!BIT(data, 0) && m_floppy[0])
		floppy = m_floppy[0]->get_device();
	else if (!BIT(data, 1) && m_floppy[1])
		floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(data, 2));
}

uint8_t m3_state::ppi2_pb_r()
{
	return m_kbd_data;
}

void m3_state::machine_start()
{
	// register for save states
	save_item(NAME(m_kbd_col));
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_kbd_data));
	save_item(NAME(m_nmi_taken));
}

void m3_state::machine_reset()
{
	m_maincpu->set_pc(0xf000);

	m_nmi_taken = false;

	// floppy motor is always on
	if (m_floppy[0])
		m_floppy[0]->get_device()->mon_w(0);
	if (m_floppy[1])
		m_floppy[1]->get_device()->mon_w(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

static DEVICE_INPUT_DEFAULTS_START( rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static void m3_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T);
	device.option_add("sa410", FLOPPY_525_SSQD);
	device.option_add("sa450", FLOPPY_525_DD);
	device.option_add("sa460", FLOPPY_525_QD);
	device.option_add("sa800", FLOPPY_8_SSDD);
	device.option_add("sa850", FLOPPY_8_DSDD);
}

void m3_state::m3(machine_config &config)
{
	Z80(config, m_maincpu, 4.9152_MHz_XTAL / 2);
	m_maincpu->set_clock_scale(1.2f); // needs to be overclocked or its too slow for the floppy
	m_maincpu->set_addrmap(AS_PROGRAM, &m3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m3_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80CTC(config, m_ctc, 0); // unknown clock
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(4.9152_MHz_XTAL / 4);
	m_ctc->zc_callback<0>().set("usart", FUNC(i8251_device::write_txc));
	m_ctc->zc_callback<0>().append("usart", FUNC(i8251_device::write_rxc));

	I8255(config, m_ppi[0]);

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set(FUNC(m3_state::ppi2_pa_w));
	m_ppi[1]->in_pb_callback().set(FUNC(m3_state::ppi2_pb_r));

	i8251_device &usart(I8251(config, "usart", 0)); // unknown clock
	usart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	usart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	usart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(rs232_defaults));
	rs232.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("usart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("usart", FUNC(i8251_device::write_cts));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(21'840'000 / 2, 707, 0, 560, 309, 0, 240); // unknown clock, hand-tuned to ~50 fps
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	mc6845_device &crtc(MC6845(config, "crtc", 21'840'000 / 2 / 7)); // unknown clock
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(7);
	crtc.set_update_row_callback(FUNC(m3_state::crtc_update_row));
	crtc.out_vsync_callback().set(m_ctc, FUNC(z80ctc_device::trg2));

	// floppy
	FD1793(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(m3_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(m3_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", m3_floppies, "sa850", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", m3_floppies, "sa850", floppy_image_device::default_mfm_floppy_formats);

	// keyboard
	I8035(config, m_kbdmcu, 6.144_MHz_XTAL);
	m_kbdmcu->set_addrmap(AS_PROGRAM, &m3_state::kbd_mem_map);
	m_kbdmcu->set_addrmap(AS_IO, &m3_state::kbd_io_map);
	m_kbdmcu->p1_in_cb().set(FUNC(m3_state::kbd_p1_r));
	m_kbdmcu->p1_out_cb().set(FUNC(m3_state::kbd_p1_w));
	m_kbdmcu->p2_out_cb().set(FUNC(m3_state::kbd_p2_w));
	m_kbdmcu->t0_in_cb().set(FUNC(m3_state::kbd_t0_r));
	m_kbdmcu->t1_in_cb().set(FUNC(m3_state::kbd_t1_r));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( m3 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("bootstrap_prom_034.bin", 0x0000, 0x0800, CRC(7fdb051e) SHA1(7aa24d4f44b6a0c8f7f647667f4997432c186cac))

	// Homebrew Monitor ROM, written by Steve Hunt. Uses the socket of the HDD ROM.
	ROM_LOAD("monitor_prom_v19_2017-07-05.bin", 0x0800, 0x0800, CRC(0608848f) SHA1(9a82cb49056ff1a1d53ce2bd026537a6914a4847))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("6845crt_font_prom_033.bin", 0x000, 0x800, CRC(cc29f664) SHA1(4197530d9455d665fd4773f95bb6394f6b056dec))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("keyboard_prom_032.bin", 0x000, 0x800, CRC(21548355) SHA1(ee4ce4af9c78474263dd58e0f19e79e5b00926fa))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS     INIT        COMPANY  FULLNAME   FLAGS
COMP( 1982, m3,   0,      0,      m3,      m3,    m3_state, empty_init, "LSI",   "M-THREE", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
