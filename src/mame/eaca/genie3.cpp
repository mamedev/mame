// license: BSD-3-Clause
// copyright-holders: Dirk Best
// thanks-to: Klaus Kämpf
/***************************************************************************

    EACA Genie III (EG3200)

    CPU board:
    - Z80A
    - 3 ROM sockets (only one populated)
    - 32x HYB 4116 (64k RAM)
    - 16 MHz XTAL

    Interface I board:
    - HD46505SP
    - 1 ROM socket (chargen)
    - 12.875 MHz XTAL
    - INS8250 UART
    - 3.0720 MHz XTAL (next to UART)
    - Centronics interface
    - Connectors for the optional programmable graphics board

    Interface II board:
    - MB8876A FDC
    - INS1771N-1 FDC
    - MSM5832RS RTC
    - 555 timer

    TODO:
    - Sound
    - Make PGA optional
    - Only the second FDC is currently used
    - Convert to use the slot system for the cards?
    - RAM expansion, HRG, HDD

    Notes:
    - Largely compatible with the TRS-80 Model 1

***************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "formats/dmk_dsk.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/ins8250.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class genie3_state : public driver_device
{
public:
	genie3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_uart(*this, "uart"),
		m_serial(*this, "serial"),
		m_centronics(*this, "centronics"),
		m_centronics_latch(*this, "centronics_latch"),
		m_rtc(*this, "rtc"),
		m_fdc1(*this, "fdc1"),
		m_fdc2(*this, "fdc2"),
		m_floppy(*this, "fdc2:%u", 0U),
		m_vram(*this, "vid%u", 0U),
		m_chargen(*this, "chargen"),
		m_kbd(*this, "row%u", 0U),
		m_banks { { *this, "bank1" }, { *this, "bank2" }, { *this, "bank3" }, { *this, "bank4" } }
	{ }

	void genie3(machine_config &config) ATTR_COLD;

	void reset_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum
	{
		IRQ_RTC = 0x80,
		IRQ_WDC = 0x40
	};

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t keyboard_r(offs_t offset);

	uint8_t centronics_status_r();
	void centronics_data_w(uint8_t data);
	void screen_mode_w(uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);

	void rtc_intrq_w(int state);
	void fdc_intrq_w(int state);
	uint8_t irq_r();
	uint8_t rtc_r();
	void rtc_w(uint8_t data);
	void rtc_rw_w(uint8_t data);
	void drive_select_w(uint8_t data);
	uint8_t fdc_status_r();
	void fdc_cmd_w(uint8_t data);
	uint8_t fdc_track_r();
	void fdc_track_w(uint8_t data);
	uint8_t fdc_sector_r();
	void fdc_sector_w(uint8_t data);
	uint8_t fdc_data_r();
	void fdc_data_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

	void bank_select_w(uint8_t data);

	required_device<z80_device> m_maincpu;
	required_device<hd6845s_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ins8250_device> m_uart;
	required_device<rs232_port_device> m_serial;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_centronics_latch;
	required_device<msm5832_device> m_rtc;
	required_device<fd1771_device> m_fdc1;
	required_device<fd1791_device> m_fdc2;
	required_device_array<floppy_connector, 4> m_floppy;
	required_shared_ptr_array<uint8_t, 3> m_vram;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport_array<11> m_kbd;

	memory_view m_banks[4];

	uint8_t m_screen_mode = 0;
	uint8_t m_irq_state = 0;

	uint8_t m_centronics_busy;
	uint8_t m_centronics_out_of_paper;
	uint8_t m_centronics_unit_select;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void genie3_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// bank 0
	map(0x0000, 0xffff).ram();
	// bank 1
	map(0x0000, 0x2fff).view(m_banks[0]);
	m_banks[0][0](0x0000, 0x07ff).rom().region("maincpu", 0);
	// bank 2
	map(0x3c00, 0x3fff).view(m_banks[1]);
	m_banks[1][0](0x3c00, 0x3fff).ram().share(m_vram[0]); // vid0
	// bank 3
	map(0x4000, 0x47ff).view(m_banks[2]);
	m_banks[2][0](0x4000, 0x43ff).ram().share(m_vram[1]); // vid1
	m_banks[2][0](0x4400, 0x47ff).ram().share(m_vram[2]); // vid2 (pga)
	// bank 4
	map(0x37e0, 0x3bff).view(m_banks[3]);
	m_banks[3][0](0x37e0, 0x37e0).mirror(0x3).rw(FUNC(genie3_state::irq_r), FUNC(genie3_state::drive_select_w));
	m_banks[3][0](0x37ec, 0x37ec).rw(FUNC(genie3_state::fdc_status_r), FUNC(genie3_state::fdc_cmd_w));
	m_banks[3][0](0x37ed, 0x37ed).rw(FUNC(genie3_state::fdc_track_r), FUNC(genie3_state::fdc_track_w));
	m_banks[3][0](0x37ee, 0x37ee).rw(FUNC(genie3_state::fdc_sector_r), FUNC(genie3_state::fdc_sector_w));
	m_banks[3][0](0x37ef, 0x37ef).rw(FUNC(genie3_state::fdc_data_r), FUNC(genie3_state::fdc_data_w));
	m_banks[3][0](0x3800, 0x38ff).mirror(0x300).r(FUNC(genie3_state::keyboard_r));
}

void genie3_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x20, 0x20).unmaprw(); // Genieplus Memory Card
	map(0x48, 0x4f).unmaprw(); // HDD
	map(0x80, 0x83).unmaprw(); // HRG Extension
	map(0xe0, 0xe0).rw(FUNC(genie3_state::rtc_r), FUNC(genie3_state::rtc_w));
	map(0xe1, 0xe1).w(FUNC(genie3_state::rtc_rw_w));
	map(0xe8, 0xef).rw(m_uart, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0xf5, 0xf5).w(FUNC(genie3_state::screen_mode_w));
	map(0xf6, 0xf6).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0xf7, 0xf7).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfa, 0xfa).w(FUNC(genie3_state::bank_select_w));
	map(0xfd, 0xfd).rw(FUNC(genie3_state::centronics_status_r), FUNC(genie3_state::centronics_data_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( genie3de )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g')  PORT_CHAR('G')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä') PORT_CHAR(U'Ä')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(':')  PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('@')  PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(U'ß') // with shift another "-"
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)                      PORT_NAME("NEW LINE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)                                PORT_CHAR(UCHAR_MAMEKEY(F10))      PORT_NAME("CLEAR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)                                 PORT_CHAR(UCHAR_MAMEKEY(F9))       PORT_NAME("BREAK")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                                 PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                               PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)  PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                              PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                              PORT_CHAR(' ')

	PORT_START("row7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x7c, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER)    PORT_NAME("LP")

	PORT_START("row8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("row9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("row10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_00_PAD)    PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)                                   PORT_NAME("LOCK")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("reset")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_WRITE_LINE_MEMBER(FUNC(genie3_state::reset_w))
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD
//**************************************************************************

void genie3_state::reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t genie3_state::keyboard_r(offs_t offset)
{
	uint8_t data = 0;

	// low 5 bits are directly connected
	for (unsigned i = 0; i < 5; i++)
		if (BIT(offset, i)) data |= m_kbd[i]->read();

	// top 3 bits are connected to a ls156
	switch (offset & 0xe0)
	{
		case 0x00: break; // not connected
		case 0x20: data |= m_kbd[5]->read(); break;
		case 0x40: data |= m_kbd[6]->read(); break;
		case 0x60: break; // TODO: speaker
		case 0x80: data |= m_kbd[7]->read(); break;
		case 0xa0: data |= m_kbd[8]->read(); break;
		case 0xc0: data |= m_kbd[9]->read(); break;
		case 0xe0: data |= m_kbd[10]->read(); break;
	}

	return data;
}


//**************************************************************************
//  INTERFACE BOARD 1
//**************************************************************************

uint8_t genie3_state::centronics_status_r()
{
	// 7-------  centronics busy
	// -6------  centronics perror
	// --5-----  centronics select

	uint8_t data = 0xff;

	data &= ~(m_centronics_busy << 7);
	data &= ~(m_centronics_out_of_paper << 6);
	data &= ~(m_centronics_unit_select << 5);

	return data;
}

void genie3_state::centronics_data_w(uint8_t data)
{
	m_centronics_latch->write(data);

	// TODO: timing
	m_centronics->write_strobe(1);
	m_centronics->write_strobe(0);
}

void genie3_state::screen_mode_w(uint8_t data)
{
	// 7654321-  not used?
	// -------0  inverse video mode

	m_screen_mode = data;
}

MC6845_UPDATE_ROW( genie3_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (unsigned x = 0; x < x_count; x++)
	{
		offs_t addr = ma + x;
		uint8_t code = m_vram[addr >> 10][addr & 0x3ff];

		if ((BIT(m_screen_mode, 0) == 0) && BIT(code, 7) && (BIT(code, 6) == 0))
		{
			// semi-graphics mode
			unsigned b = (ra >> 2) << 1;

			bitmap.pix(y, x * 8 + 0) = pen[BIT(code, b + 0)];
			bitmap.pix(y, x * 8 + 1) = pen[BIT(code, b + 0)];
			bitmap.pix(y, x * 8 + 2) = pen[BIT(code, b + 0)];
			bitmap.pix(y, x * 8 + 3) = pen[BIT(code, b + 0)];
			bitmap.pix(y, x * 8 + 4) = pen[BIT(code, b + 1)];
			bitmap.pix(y, x * 8 + 5) = pen[BIT(code, b + 1)];
			bitmap.pix(y, x * 8 + 6) = pen[BIT(code, b + 1)];
			bitmap.pix(y, x * 8 + 7) = pen[BIT(code, b + 1)];
		}
		else
		{
			// text mode
			uint8_t data = 0;

			if ((BIT(m_screen_mode, 0) == 0) && BIT(code, 7) && BIT(code, 6))
			{
				// char data from pgm
				data = m_vram[2][(code & 0x3f) << 4 | ra];
			}
			else
			{
				// char data from chargen rom
				data = m_chargen[(code & 0x7f) << 4 | ra];
				data &= 0xfe; // clear last column, always 1?
			}

			// invert
			if (BIT(m_screen_mode, 0) && BIT(code, 7) == 1)
				data ^= 0xff;

			if (x == cursor_x)
				data = 0xff;

			// draw 8 pixels of the character
			bitmap.pix(y, x * 8 + 0) = pen[BIT(data, 7)];
			bitmap.pix(y, x * 8 + 1) = pen[BIT(data, 6)];
			bitmap.pix(y, x * 8 + 2) = pen[BIT(data, 5)];
			bitmap.pix(y, x * 8 + 3) = pen[BIT(data, 4)];
			bitmap.pix(y, x * 8 + 4) = pen[BIT(data, 3)];
			bitmap.pix(y, x * 8 + 5) = pen[BIT(data, 2)];
			bitmap.pix(y, x * 8 + 6) = pen[BIT(data, 1)];
			bitmap.pix(y, x * 8 + 7) = pen[BIT(data, 0)];
		}
	}
}

static const gfx_layout char_layout =
{
	7,9,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  INTERFACE BOARD 2
//**************************************************************************

void genie3_state::rtc_intrq_w(int state)
{
	m_rtc->hold_w(state);

	if (state)
	{
		m_irq_state |= IRQ_RTC;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

void genie3_state::fdc_intrq_w(int state)
{
	if (state)
		m_irq_state |= IRQ_WDC;
	else
		m_irq_state &= ~IRQ_WDC;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t genie3_state::irq_r()
{
	uint8_t data = m_irq_state;

	if (!machine().side_effects_disabled())
	{
		// reading clears the rtc irq
		m_irq_state &= ~IRQ_RTC;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	}

	return data;
}

uint8_t genie3_state::rtc_r()
{
	return m_rtc->data_r();
}

void genie3_state::rtc_w(uint8_t data)
{
	m_rtc->data_w(data);
	m_rtc->address_w(data >> 4);
}

void genie3_state::rtc_rw_w(uint8_t data)
{
	m_rtc->write_w(BIT(data, 7));
	m_rtc->read_w(BIT(data, 6));
}

void genie3_state::drive_select_w(uint8_t data)
{
	// 765-----  not used
	// ---4----  side select
	// ----3---  drive select 3
	// -----2--  drive select 2
	// ------1-  drive select 1
	// -------0  drive select 0

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) floppy = m_floppy[1]->get_device();
	if (BIT(data, 2)) floppy = m_floppy[2]->get_device();
	if (BIT(data, 3)) floppy = m_floppy[3]->get_device();

	m_fdc2->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(BIT(data, 4));
		floppy->mon_w(0); // TODO: timer to turn off
	}
}

uint8_t genie3_state::fdc_status_r()
{
	return m_fdc2->status_r() ^ 0xff;
}

void genie3_state::fdc_cmd_w(uint8_t data)
{
	if ((data & 0xf8) == 0xf8)
		m_fdc2->dden_w(BIT(~data, 0));
	else
		m_fdc2->cmd_w(data ^ 0xff);
}

uint8_t genie3_state::fdc_track_r()
{
	return m_fdc2->track_r() ^ 0xff;
}

void genie3_state::fdc_track_w(uint8_t data)
{
	m_fdc2->track_w(data ^ 0xff);
}

uint8_t genie3_state::fdc_sector_r()
{
	return m_fdc2->sector_r() ^ 0xff;
}

void genie3_state::fdc_sector_w(uint8_t data)
{
	if (BIT(data, 7))
		m_fdc2->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 6) ? 8 : 16));
	else
		m_fdc2->sector_w(data ^ 0xff);
}

uint8_t genie3_state::fdc_data_r()
{
	return m_fdc2->data_r() ^ 0xff;
}

void genie3_state::fdc_data_w(uint8_t data)
{
	m_fdc2->data_w(data ^ 0xff);
}

void genie3_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add_mfm_containers();
	fr.add(FLOPPY_DMK_FORMAT);
}

static void genie3_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_SSSD);
	device.option_add("sd",   FLOPPY_525_SD);
	device.option_add("ssdd", FLOPPY_525_SSDD);
	device.option_add("dd",   FLOPPY_525_DD);
	device.option_add("ssqd", FLOPPY_525_SSQD);
	device.option_add("qd",   FLOPPY_525_QD);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void genie3_state::bank_select_w(uint8_t data)
{
	// 7654----  not used?
	// ----3---  bank 4
	// -----2--  bank 3
	// ------1-  bank 2
	// -------0  bank 1

	for (unsigned i = 0; i < 4; i++)
	{
		if (BIT(data, i) == 0)
			m_banks[i].select(0);
		else
			m_banks[i].disable();
	}
}

void genie3_state::machine_start()
{
	// connected to vcc
	m_rtc->cs_w(1);

	// register for save states
	save_item(NAME(m_screen_mode));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_out_of_paper));
	save_item(NAME(m_centronics_unit_select));
}

void genie3_state::machine_reset()
{
	// reset state: enable bank 1, 2 and 4
	bank_select_w(0xf4);

	m_screen_mode = 0;
	m_irq_state = 0;

	// defaults to 5.25"
	m_fdc2->set_unscaled_clock(16_MHz_XTAL / 16);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void genie3_state::genie3(machine_config &config)
{
	// cpu board

	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &genie3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &genie3_state::io_map);

	// interface board 1

	HD6845S(config, m_crtc, 12.875_MHz_XTAL / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(genie3_state::crtc_update_row));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(12.875_MHz_XTAL, 824, 0, 512, 312, 0, 192);
	m_screen->set_screen_update(m_crtc, FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	INS8250(config, m_uart, 3.072_MHz_XTAL);
	m_uart->out_tx_callback().set(m_serial, FUNC(rs232_port_device::write_txd));
	m_uart->out_rts_callback().set(m_serial, FUNC(rs232_port_device::write_rts));
	m_uart->out_dtr_callback().set(m_serial, FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_uart, FUNC(ins8250_device::rx_w));
	m_serial->cts_handler().set(m_uart, FUNC(ins8250_device::cts_w));
	m_serial->dsr_handler().set(m_uart, FUNC(ins8250_device::dsr_w));
	m_serial->dcd_handler().set(m_uart, FUNC(ins8250_device::dcd_w));

	OUTPUT_LATCH(config, m_centronics_latch);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(*m_centronics_latch);
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; }).invert();
	m_centronics->perror_handler().set([this](int state) { m_centronics_out_of_paper = state; }).invert();
	m_centronics->select_handler().set([this](int state) { m_centronics_unit_select = state; }).invert();

	// interface board 2

	// 555 with R1 250k, R2 250k, C 0.047
	auto &rtc_clock(CLOCK(config, "rtc_clock", ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_MSEC(25))));
	rtc_clock.set_duty_cycle(0.67);
	rtc_clock.signal_handler().set(FUNC(genie3_state::rtc_intrq_w));

	FD1771(config, m_fdc1, 16_MHz_XTAL / 16);

	FD1791(config, m_fdc2, 16_MHz_XTAL / 16);
	m_fdc2->intrq_wr_callback().set(FUNC(genie3_state::fdc_intrq_w));

	FLOPPY_CONNECTOR(config, "fdc2:0", genie3_floppies, "qd", genie3_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc2:1", genie3_floppies, "qd", genie3_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc2:2", genie3_floppies, nullptr, genie3_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc2:3", genie3_floppies, nullptr, genie3_state::floppy_formats);

	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	SOFTWARE_LIST(config, "flop_list").set_original("genie3_flop");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( genie3de )
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("5100-01.bin", 0x000, 0x800, CRC(ef4fbd20) SHA1(5a6ad3e0a80b8c5eee7b235f6ecaba07bfca8267))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("genie3de_cg.bin", 0x000, 0x800, CRC(74aeca3b) SHA1(60071dea1177202fa727dc12c828fe097f0c7952))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT     CLASS         INIT        COMPANY  FULLNAME               FLAGS
COMP( 1982, genie3de, 0,      0,      genie3,  genie3de, genie3_state, empty_init, "EACA",  "Genie III (Germany)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
