// license: BSD-3-Clause
// copyright-holders: Dirk Best, Märt Põder
/***************************************************************************

    Juku E5101/E5104

    Hardware:
    - КР580ВМ80A (called КР580ИК80A until 1986)
    - КР580ИР82
    - КР580ВА86 x3
    - КР580ВА87 x3
    - КР580ВИ53 x3
    - КР580ВК38
    - КР580ВН59
    - КР580ВВ51A x2
    - КР580ВВ55A x2
    - КР1818ВГ93 (on all E5104 production models)

    Note:
    - In the monitor, enter A to start BASIC and T to boot from disk/network

    TODO:
    - Work out how the floppy interface really works?
    - Tape? (split up to E5101 test batch as tape only?)
    - Network?
    - Ramdisk?
    - Mouse!

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/74148.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "formats/juku_dsk.h"
#include "softlist_dev.h"
#include "screen.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "osdcore.h" // osd_printf_*

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace {

class juku_state : public driver_device
{
public:
	juku_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank(*this, "bank"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit%u", 0U),
		m_pio(*this, "pio%u", 0U),
		m_sio(*this, "sio%u", 0U),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_key_encoder(*this, "keyenc"),
		m_keys(*this, "COL.%u", 0U),
		m_key_special(*this, "SPECIAL"),
		m_screen(*this, "screen"),
		m_speaker(*this, "speaker")
	{ }

	void juku(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i8080a_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank;
	required_device<pic8259_device> m_pic;
	required_device_array<pit8253_device, 3> m_pit;
	required_device_array<i8255_device, 2> m_pio;
	required_device_array<i8251_device, 2> m_sio;
	required_device<kr1818vg93_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ttl74148_device> m_key_encoder;
	required_ioport_array<16> m_keys;
	required_ioport m_key_special;
	required_device<screen_device> m_screen;
	required_device<speaker_sound_device> m_speaker;

	void mem_map(address_map &map);
	void bank_map(address_map &map);
	void io_map(address_map &map);

	void pio0_porta_w(uint8_t data);
	uint8_t pio0_portb_r();
	void pio0_portc_w(uint8_t data);
	uint8_t m_prev_porta;
	uint8_t m_prev_portc;

	int m_screen_x;
	int m_screen_y;
	void screen_cmd_a_w(uint8_t data);
	void screen_data_a_w(uint8_t data);
	void screen_cmd_b_w(uint8_t data);
	void screen_data_b_w(uint8_t data);
	void screen_scan_sequence(int pos, uint8_t data);
	void screen_mode(int x, int y);
	int m_screen_cur_seq[5];
	static constexpr int screen_320_240_seq[5] = { 0x24, 0x8, 0x72, 0x0, 0x25 };
	static constexpr int screen_384_200_seq[5] = { 0x16, 0x4, 0x12, 0x1, 0x45 };
	static constexpr int screen_400_192_seq[5] = { 0x14, 0x3, 0x1a, 0x1, 0x45 };
	static constexpr int screen_256_192_seq[5] = { 0x32, 0x12, 0x20, 0x1, 0x49 };
	static constexpr const int *screen_mode_sequences[4] = { screen_320_240_seq, screen_384_200_seq, screen_400_192_seq, screen_256_192_seq };
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	bool m_beep_state;
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	void update_speaker();
	static constexpr double speaker_levels[3] = { 0, 0.67, 1 };

	static void floppy_formats(format_registration &fr);
	void fdc_drq_w(int state);
	void fdc_cmd_w(uint8_t data);
	uint8_t fdc_data_r();
	void fdc_data_w(uint8_t data);
	uint8_t m_fdc_cur_cmd;

	std::unique_ptr<uint8_t[]> m_ram;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void juku_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).m(m_bank, FUNC(address_map_bank_device::amap8));
}

void juku_state::bank_map(address_map &map)
{
	// memory mode 0
	map(0x00000, 0x03fff).rom().region("maincpu", 0);
	map(0x00000, 0x03fff).bankw("ram_0000");
	map(0x04000, 0x0ffff).bankrw("ram_4000");
	// memory mode 1
	map(0x10000, 0x1ffff).bankrw("ram_0000");
	map(0x1d800, 0x1ffff).bankr("rom_d800");
	// memory mode 2
	map(0x20000, 0x23fff).bankrw("ram_0000");
	map(0x24000, 0x2bfff).rom().region("extension", 0);
	map(0x2c000, 0x2ffff).bankrw("ram_c000");
	map(0x2d800, 0x2ffff).bankr("rom_d800");
	// memory mode 3
	map(0x30000, 0x3ffff).bankrw("ram_0000");
}

void juku_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x04, 0x07).rw(m_pio[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_sio[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0c, 0x0f).rw(m_pio[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x14, 0x17).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x18, 0x1b).rw(m_pit[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1c, 0x1f).rw(m_fdc, FUNC(kr1818vg93_device::read), FUNC(kr1818vg93_device::write));
	map(0x1c, 0x1c).w(FUNC(juku_state::fdc_cmd_w));
	map(0x1f, 0x1f).rw(FUNC(juku_state::fdc_data_r), FUNC(juku_state::fdc_data_w));
	map(0x11,0x11).w(FUNC(juku_state::screen_cmd_a_w));
	map(0x12,0x12).w(FUNC(juku_state::screen_data_a_w));
	map(0x15,0x15).w(FUNC(juku_state::screen_cmd_b_w));
	map(0x16,0x16).w(FUNC(juku_state::screen_data_b_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************


static INPUT_PORTS_START( juku )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')// n N
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // y Y
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') // 6 &
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // h H
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // x X
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // w W
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') // 2 "
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // s S
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // v V
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // r R
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 4 $
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // f F
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAPS LOCK")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // b B
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // t T
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // 5 %
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // g G
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // z Z
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // q Q
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // 1 !
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // a A
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3) // c C
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // e E
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // 3 #
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // d D
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // m M
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // u U
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39) // 7 '
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // j J
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("DEL")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') // ] õ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("ERASE") // ERASE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') // [ ö
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Ä  Ü") // Ä Ü
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("õ  Õ") // õ Õ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_NAME("ö  Õ") // ö Õ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(':') PORT_CHAR('*') // : *
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(';') PORT_CHAR('+') // ; +
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	// Picture of machine shows "\ ^" here. You can use Ü to represent ^ in BASIC.
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('\\') PORT_CHAR('^') PORT_NAME("ü  Ü") // ü Ü
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') // - =
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("ä  Ä") // ä Ä
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // / ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // p P
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_') // 0 _
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME("ö  Ö") // ö Ö
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // . >
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT),8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // o O
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') // 9 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // l L
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // , <
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_NAME("LAT RUS") // LAT/RUS
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // i I
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') // 8 (
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // k K
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	// CONTRDAT?
	PORT_START("COL.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SPECIAL")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO
//**************************************************************************

void juku_state::screen_cmd_a_w(uint8_t data) { m_pit[0]->write(0x11, data); screen_scan_sequence(0, data); }
void juku_state::screen_data_a_w(uint8_t data) { m_pit[0]->write(0x12, data); screen_scan_sequence(1, data); }
void juku_state::screen_cmd_b_w(uint8_t data) { m_pit[1]->write(0x15, data); m_screen_cur_seq[2] == -1 ? screen_scan_sequence(2, data) : screen_scan_sequence(3, data); }
void juku_state::screen_data_b_w(uint8_t data) { m_pit[1]->write(0x16, data); screen_scan_sequence(4, data); }

void juku_state::screen_scan_sequence(int pos, uint8_t data)
{
	m_screen_cur_seq[pos] = (int)data;

	for (int i=0; i<3; i++)
		if (memcmp(screen_mode_sequences[i], m_screen_cur_seq, pos*sizeof(int)) == 0) {
			if (pos == 4)
				switch (i) {
				case 0:
					screen_mode(320, 240);
					break;
				case 1:
					screen_mode(384, 200);
					break;
				case 2:
					screen_mode(400, 192);
					break;
				case 3:
					screen_mode(256, 192);
					break;
				}
			else return;
		}

	for (int i=0; i<5; i++)
		m_screen_cur_seq[i]=-1;
}

void juku_state::screen_mode(int x, int y)
{
	if (m_screen_x == x && m_screen_y == y)
		return;

	m_screen_x = x, m_screen_y = y;
	rectangle v = m_screen->visible_area();
	v.max_x = x - 1;
	v.max_y = y - 1;

	m_screen->configure(x, y, v, m_screen->frame_period().attoseconds());
}

uint32_t juku_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < m_screen_y; y++)
	{
		uint32_t *dest = &bitmap.pix(y);
		for (int x = 0; x < m_screen_x; x++)
			*dest++ = BIT(m_ram[0xd800 + (y * (m_screen_x / 8) + x / 8)], 7 - (x % 8)) ? rgb_t::white() : rgb_t::black();
	}

	return 0;
}


//**************************************************************************
//  FLOPPY DISK
//**************************************************************************

void juku_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_JUKU_FORMAT);
}

static void juku_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("525ssqd", FLOPPY_525_SSQD);
}

void juku_state::fdc_drq_w(int state)
{
	// clear HALT state of CPU when data is ready to read
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

void juku_state::fdc_cmd_w(uint8_t data)
{
	if (m_fdc_cur_cmd != data)
		m_fdc_cur_cmd = data;

	m_fdc->cmd_w(data);
}

uint8_t juku_state::fdc_data_r()
{
	// on read commands (100xxxxx, 11000xxx, 11100xxx) and fdc reports busy
	if ( ((m_fdc_cur_cmd >> 5) == 0x4 || (m_fdc_cur_cmd >> 5) == 0x6 || (m_fdc_cur_cmd >> 3) == 0x1c) && m_fdc->drq_r() == 0 && (m_fdc->status_r() & 0x1) == 0x1 )
	{
		// cpu tries to read data without drq active. halt it and reset the
		// pc back to the beginning of the instruction
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_state_int(i8080a_cpu_device::I8085_PC, m_maincpu->pc() - 2);

		return 0;
	}

	return m_fdc->data_r();
}

void juku_state::fdc_data_w(uint8_t data)
{
	// on write commands (101xxxxx, 11110xxx) and fdc reports busy
	if ( ((m_fdc_cur_cmd >> 5) == 0x5 || (m_fdc_cur_cmd >> 3) == 0x1e) && m_fdc->drq_r() == 0 && (m_fdc->status_r() & 0x1) == 0x1 )
	{
		// cpu tries to write data without drq, halt it and reset pc
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_state_int(i8080a_cpu_device::I8085_PC, m_maincpu->pc() - 2);

		return;
	}

	m_fdc->data_w(data);
}

//**************************************************************************
//  SOUND
//**************************************************************************

void juku_state::update_speaker()
{
	m_speaker->level_w(m_beep_state << BIT(m_prev_porta, 4));
}

WRITE_LINE_MEMBER(juku_state::speaker_w)
{
	m_beep_state = state;
	update_speaker();
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void juku_state::pio0_porta_w(uint8_t data)
{
	// 7-------  stb
	// -6------  pren
	// --5-----  not used?
	// ---4----  audc
	// ----3210  keyboard column

	for (int i = 0; i < 6; i++)
		m_key_encoder->input_line_w(i, BIT(m_keys[data & 0x0f]->read(), i));

	m_key_encoder->update();

	if (BIT(data, 4) != BIT(m_prev_porta, 4))
		update_speaker();

	if (m_prev_porta != data)
		m_prev_porta = data;
}

uint8_t juku_state::pio0_portb_r()
{
	// 7-------  ctrl
	// -6------  shift
	// --54----  not used
	// ----321-  keyboard data
	// -------0  key pressed

	uint8_t data = 0;

	data |= m_key_special->read();
	data |= 0x30;
	data |= m_key_encoder->output_r() << 1;
	data |= m_key_encoder->output_valid_r();

	return data;
}

void juku_state::pio0_portc_w(uint8_t data)
{
	// 7-------  (cas?) pof
	// -6------  (cas?) stop / floppy side select
	// --5-----  (cas?) rn / floppy drive select
	// ---4----  (cas?) ff / floppy?
	// ----3---  (cas?) play
	// -----2--  (cas?) rec / floppy?
	// ------10  memory mode

	for (int i = 2; i < 8; i++)
		if (BIT(data, i) !=  BIT(m_prev_portc, i))
			osd_printf_verbose("pio0: c%d = %d\n", i, BIT(data, i));

	floppy_image_device *floppy = m_floppy[BIT(data, 5)]->get_device();
	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		// the motor is always running for now
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 6));
	}

	m_bank->set_bank(data & 0x03);

	if (m_prev_portc != data)
		m_prev_portc = data;
}

void juku_state::machine_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x10000);

	membank("rom_d800")->set_base(memregion("maincpu")->base() + 0x1800);

	membank("ram_0000")->set_base(&m_ram[0x0000]);
	membank("ram_4000")->set_base(&m_ram[0x4000]);
	membank("ram_c000")->set_base(&m_ram[0xc000]);

	// register for save states
	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_prev_portc));
	save_item(NAME(m_prev_porta));
	save_item(NAME(m_beep_state));
	save_item(NAME(m_fdc_cur_cmd));
	save_item(NAME(m_screen_cur_seq));
	save_item(NAME(m_screen_x));
	save_item(NAME(m_screen_y));
}

void juku_state::machine_reset()
{
	m_bank->set_bank(0);
	m_key_encoder->enable_input_w(0);
	m_prev_portc = 0;
	m_prev_porta = 0;
	m_beep_state = 0;
	m_fdc_cur_cmd = 0;
	m_screen_x = 320, m_screen_y = 240;
	for (int i=0; i<5; i++)
		m_screen_cur_seq[i]=-1;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void juku_state::juku(machine_config &config)
{
	// КР580ВМ80A @ 2 MHz
	I8080A(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &juku_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &juku_state::io_map);
	m_maincpu->in_inta_func().set("pic", FUNC(pic8259_device::acknowledge));

	ADDRESS_MAP_BANK(config, m_bank);
	m_bank->set_map(&juku_state::bank_map);
	m_bank->set_data_width(8);
	m_bank->set_addr_width(18);
	m_bank->set_stride(0x10000);

	// КР580ВН59
	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	// КР580ВИ53 (#1)
	PIT8253(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(16_MHz_XTAL/16);
	m_pit[0]->set_clk<1>(16_MHz_XTAL/16);
	m_pit[0]->set_clk<2>(16_MHz_XTAL/16);

	m_pit[0]->out_handler<0>().set(m_pit[1], FUNC(pit8253_device::write_clk0));
	m_pit[0]->out_handler<0>().append(m_pit[0], FUNC(pit8253_device::write_gate1));
	m_pit[0]->out_handler<0>().append(m_pit[0], FUNC(pit8253_device::write_gate2));

	// КР580ВИ53 (#2)
	PIT8253(config, m_pit[1], 0);

	m_pit[0]->out_handler<2>().set(m_pit[1], FUNC(pit8253_device::write_clk1)); // H SYNC DSL
	m_pit[0]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk2));

	m_pit[1]->out_handler<0>().append(m_pit[1], FUNC(pit8253_device::write_gate1));
	m_pit[1]->out_handler<0>().append(m_pit[1], FUNC(pit8253_device::write_gate2));

	m_pit[1]->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir5_w)); // VERT RTR

	//m_pit[1]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk1)); // VERT SYNC BEL
	//m_pit[1]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk2));

	// КР580ВИ53 (#3)
	PIT8253(config, m_pit[2], 0);

	m_pit[2]->set_clk<0>(16_MHz_XTAL/13);
	m_pit[2]->set_clk<1>(16_MHz_XTAL/8);
	m_pit[2]->set_clk<2>(16_MHz_XTAL/13);

	//m_pit[1]->out_handler<0>().append(...); // BAUD RATE
	m_pit[2]->out_handler<1>().append(FUNC(juku_state::speaker_w)); // SOUND
	//m_pit[1]->out_handler<2>().append(...); // SYNC BAUD RATE

	// КР580ВВ55A
	I8255A(config, m_pio[0]);
	m_pio[0]->out_pa_callback().set(FUNC(juku_state::pio0_porta_w));
	m_pio[0]->in_pb_callback().set(FUNC(juku_state::pio0_portb_r));
	m_pio[0]->out_pc_callback().set(FUNC(juku_state::pio0_portc_w));

	// КР580ВВ55A
	I8255A(config, m_pio[1]);

	// КР580ВВ51A
	I8251(config, m_sio[0], 0);
	m_sio[0]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir2_w));
	m_sio[0]->txrdy_handler().set("pic", FUNC(pic8259_device::ir3_w));

	// КР580ВВ51A (instead of FDC?)
	I8251(config, m_sio[1], 0);
	m_sio[1]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir0_w));
	m_sio[1]->txrdy_handler().set("pic", FUNC(pic8259_device::ir1_w));

	// Электроника МС 6105.4 (60 Hz)
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen_x = 320, m_screen_y = 240;
	m_screen->set_size(m_screen_x, m_screen_y);
	m_screen->set_visarea(0, m_screen_x-1, 0, m_screen_y-1);
	m_screen->set_screen_update(FUNC(juku_state::screen_update));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1);
	m_speaker->set_levels(3, speaker_levels);

	TTL74148(config, m_key_encoder, 0);

	// КР1818ВГ93
	KR1818VG93(config, m_fdc, 1_MHz_XTAL);
	m_fdc->drq_wr_callback().set(FUNC(juku_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", juku_floppies, "525qd", juku_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", juku_floppies, "525qd", juku_state::floppy_formats);

	SOFTWARE_LIST(config, "floppy_list").set_original("juku");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( juku )
	ROM_DEFAULT_BIOS("3.43m")
	ROM_SYSTEM_BIOS(0, "3.43m", "RomBios 3.43m")

	ROM_REGION(0x4000, "maincpu", 0)
	// This is RomBios 3.43m from widespread Baltijets E5104 batch with
	// identification: "EktaSoft '88  Serial #0037"
	ROMX_LOAD("jukurom0.bin", 0x0000, 0x2000, CRC(b26f5080) SHA1(db8bab6ff7143be890d6aaa25d10386dfdac3fc7), ROM_BIOS(0))
	ROMX_LOAD("jukurom1.bin", 0x2000, 0x2000, CRC(b184e253) SHA1(d169acde61f643d7d0780cca0eeaf33ebdf75b92), ROM_BIOS(0))

	ROM_REGION(0x8000, "extension", 0)
	// This is EKTA JBASIC cartridge from 1990, probably version 1.1
	// from 14.09.1987, there is also version with HEX$ directive
	// publised for running from EKDOS. E5101 test batch probably had
	// JBASIC onboard with earlier RomBios 2.4/3.4 from 6.12.1987.
	ROMX_LOAD("bas0.bin", 0x0000, 0x0800, CRC(c03996cd) SHA1(3c45537c2a1879998e5315b79eb44dcf7c007d69), ROM_BIOS(0))
	ROMX_LOAD("bas1.bin", 0x0800, 0x0800, CRC(d8016869) SHA1(baef9e9c55171a9192bc13d48e3b45394c7780d9), ROM_BIOS(0))
	ROMX_LOAD("bas2.bin", 0x1000, 0x0800, CRC(9a958621) SHA1(08baca27e1ccdb0a441706df267c1f82b82d56ab), ROM_BIOS(0))
	ROMX_LOAD("bas3.bin", 0x1800, 0x0800, CRC(d4ffbf67) SHA1(bced7ff2420f630dbd4cd1c0c83481ed874869f1), ROM_BIOS(0))
ROM_END

} // Anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME      FLAGS
COMP( 1988, juku, 0,      0,      juku,    juku,  juku_state, empty_init, "EKTA",  "Juku E5101", MACHINE_SUPPORTS_SAVE)
