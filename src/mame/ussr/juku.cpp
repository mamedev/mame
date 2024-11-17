// license: BSD-3-Clause
// copyright-holders: Dirk Best, Märt Põder
/***************************************************************************

    Juku E5101/E5104

    Hardware:
    - КР580ВМ80A (=КР580ИК80A)
    - КР580ИР82
    - КР580ВА86 x3
    - КР580ВА87 x3
    - КР580ВИ53 x3
    - КР580ВК38
    - КР580ВН59
    - КР580ВВ51A x2
    - КР580ВВ55A x2 (=КР580ИК55)
    - КР1818ВГ93 (on all E5104 production models)

    Note:
    - In the monitor, enter A or B to start BASIC/Assembler
      and T to boot from tape/disk/network

    TODO:
    - Work out how the floppy interface really works?
    - Tape? (split up to E5101 batch as tape only?)
    - И41 (=Multibus-1) compatibility?
    - Network?
    - Ramdisk?
    - Memory extensions?
    - Mouse!

***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/74148.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/juku_dsk.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace {

static constexpr int DEFAULT_WIDTH = 320;
static constexpr int DEFAULT_HEIGHT = 240;
static constexpr int VERT_FRONT_PORCH = 25;
static constexpr int VERT_BACK_PORCH = 1+72-VERT_FRONT_PORCH;
static constexpr int HORIZ_FRONT_PORCH = 8*8;
static constexpr int HORIZ_BACK_PORCH = 24*8-HORIZ_FRONT_PORCH;

static constexpr int HORIZ_PERIOD = DEFAULT_WIDTH+HORIZ_FRONT_PORCH+HORIZ_BACK_PORCH;
static constexpr int VERT_PERIOD = DEFAULT_HEIGHT+VERT_FRONT_PORCH+VERT_BACK_PORCH;

static constexpr double SPEAKER_LEVELS[3] = {0, 0.5, 1};

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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

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

	int32_t m_width, m_height, m_hbporch, m_vbporch;

	uint8_t m_contrdat;

	int16_t m_height_lsb, m_vblank_period_lsb;
	int16_t m_monitor_bits, m_empty_screen_on_update;

	bool m_beep_state, m_beep_level;

	uint8_t m_fdc_cur_cmd;

	std::unique_ptr<uint8_t[]> m_ram;

	void mem_map(address_map &map) ATTR_COLD;
	void bank_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void pio0_porta_w(uint8_t data);
	uint8_t pio0_portb_r();
	void pio0_portc_w(uint8_t data);

	void screen_width(uint8_t data);
	void screen_hblank_period(uint8_t data);
	void screen_hfporch(uint8_t data);
	void screen_height(uint8_t data);
	void screen_vblank_period(uint8_t data);
	void screen_vfporch(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// helpers to coordinate screen mode switching
	void adjust_monitor_params(uint8_t monitor_bits);

	void speaker_w(int state);

	static void floppy_formats(format_registration &fr);
	void fdc_drq_w(int state);
	void fdc_cmd_w(uint8_t data);
	uint8_t fdc_data_r();
	void fdc_data_w(uint8_t data);
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
	map(0x00, 0x01).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x04, 0x07).rw(m_pio[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_sio[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0c, 0x0f).rw(m_pio[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x14, 0x17).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	// write functions for sceen timer config to adjust emulated display
	map(0x10, 0x10).w(FUNC(juku_state::screen_width));
	map(0x14, 0x14).w(FUNC(juku_state::screen_height));
	map(0x11, 0x11).w(FUNC(juku_state::screen_hblank_period));
	map(0x12, 0x12).w(FUNC(juku_state::screen_hfporch));
	map(0x15, 0x15).w(FUNC(juku_state::screen_vblank_period));
	map(0x16, 0x16).w(FUNC(juku_state::screen_vfporch));
	map(0x18, 0x1b).rw(m_pit[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1c, 0x1f).rw(m_fdc, FUNC(kr1818vg93_device::read), FUNC(kr1818vg93_device::write));
	// functions for floppy drive operation fixes
	map(0x1c, 0x1c).w(FUNC(juku_state::fdc_cmd_w));
	map(0x1f, 0x1f).rw(FUNC(juku_state::fdc_data_r), FUNC(juku_state::fdc_data_w));
	// mapping for cassette version (E5101?)
	// map(0x1c, 0x1d).rw(m_sio[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
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
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHAR('&') // 6 &
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // h H
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // x X
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // w W
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHAR('"') // 2 "
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // s S
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // v V
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // r R
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHAR('$') // 4 $
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
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHAR('%') // 5 %
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // g G
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // z Z
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // q Q
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHAR('!') // 1 !
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // a A
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3) // c C
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // e E
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHAR('#') // 3 #
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // d D
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // m M
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // u U
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHAR(39) // 7 '
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // j J
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("DEL")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(']') // ] õ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("ERASE") // ERASE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) PORT_NAME("RETURN")
	PORT_DIPNAME(0x40, 0x00, "B4_0") PORT_DIPLOCATION("CONTRDAT1:01")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_0") PORT_DIPLOCATION("CONTRDAT2:01")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR('[') // [ ö
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Ä  Ü") // Ä Ü
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x40, 0x00, "B4_1") PORT_DIPLOCATION("CONTRDAT1:02")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_1") PORT_DIPLOCATION("CONTRDAT2:02")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("õ  Õ") // õ Õ
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Ö  Õ") // Ö Õ
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(':') PORT_CHAR('*') // : *
	PORT_DIPNAME(0x40, 0x00, "B4_2") PORT_DIPLOCATION("CONTRDAT1:03")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_2") PORT_DIPLOCATION("CONTRDAT2:03")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(';') PORT_CHAR('+') // ; +
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	// Picture of machine shows "\ ^" here. You can use Ü to represent ^ in BASIC or switch to ASCII font in EKDOS to make ^ show as expected.
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\\') PORT_CHAR('^') PORT_NAME("ü  Ü") // ü Ü
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') PORT_CHAR('=') // - =
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("ä  Ä") // ä Ä
	PORT_DIPNAME(0x40, 0x00, "B4_3") PORT_DIPLOCATION("CONTRDAT1:04")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_3") PORT_DIPLOCATION("CONTRDAT2:04")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // / ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // p P
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHAR('_') // 0 _
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME("ö  Ö") // ö Ö
	PORT_DIPNAME(0x40, 0x00, "B4_4") PORT_DIPLOCATION("CONTRDAT1:05")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_4") PORT_DIPLOCATION("CONTRDAT2:05")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // . >
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT),8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // o O
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(')') // 9 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // l L
	PORT_DIPNAME(0x40, 0x00, "B4_5") PORT_DIPLOCATION("CONTRDAT1:06")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_5") PORT_DIPLOCATION("CONTRDAT2:06")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(F8)) // "7-8-9-F8" on numpad (popular gaming controls)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(',') PORT_CHAR('<') // , <
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME("LAT RUS") // LAT/RUS
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // i I
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_CHAR('(') // 8 (
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // k K
	PORT_DIPNAME(0x40, 0x00, "B4_6") PORT_DIPLOCATION("CONTRDAT1:07")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_6") PORT_DIPLOCATION("CONTRDAT2:07")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("COL.15")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x40, 0x00, "B4_7") PORT_DIPLOCATION("CONTRDAT1:08")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "B5_7") PORT_DIPLOCATION("CONTRDAT2:08")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x80, DEF_STR(On))

	PORT_START("SPECIAL")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO
//**************************************************************************

inline uint16_t bcd_value(uint16_t val) 
{
	return
		((val>>12) & 0xf) *  1000 +
		((val>> 8) & 0xf) *   100 +
		((val>> 4) & 0xf) *    10 +
		( val      & 0xf);
}

void juku_state::screen_width(uint8_t data)
{
	m_pit[0]->write(0x10, data);
	m_screen->set_size(bcd_value(data)*8, m_screen->height());
	adjust_monitor_params(0b0000'0001);
}

void juku_state::screen_height(uint8_t data)
{
	m_pit[1]->write(0x14, data);

	if (m_height_lsb == -1) {
		m_height_lsb = (int)data;
	} else {
		m_screen->set_size(m_screen->width(), ((uint16_t)data << 8) + (uint8_t)m_height_lsb);
		m_height_lsb = -1;
		adjust_monitor_params(0b0000'1000);
	}
}

void juku_state::screen_hblank_period(uint8_t data)
{
	m_pit[0]->write(0x11, data);
	m_width = m_screen->width()-bcd_value(data)*8;
	adjust_monitor_params(0b0000'0010);
}

void juku_state::screen_vblank_period(uint8_t data)
{
	m_pit[1]->write(0x15, data);
	
	if (m_vblank_period_lsb == -1) {
		m_vblank_period_lsb = (int)data;
	} else {
		m_height = m_screen->height()-bcd_value(((uint16_t)data<<8) + (uint8_t)m_vblank_period_lsb) - 1;
		m_vblank_period_lsb = -1;
		adjust_monitor_params(0b0001'0000);
	}
}

void juku_state::screen_hfporch(uint8_t data)
{
	m_pit[0]->write(0x12, data);
	m_hbporch = m_screen->width()-m_width-bcd_value(data)*8;
	adjust_monitor_params(0b0000'0100);
}

void juku_state::screen_vfporch(uint8_t data)
{
	m_pit[1]->write(0x16, data);
	m_vbporch = m_screen->height()-m_height-bcd_value(data);
	adjust_monitor_params(0b0010'0000);
}

/*
 * In changing screen resolution mimic a real monitor and allow tweaking
 * individual parameters without immediately changing the visible area
 */
void juku_state::adjust_monitor_params(uint8_t monitor_bits)
{
	// --5-----  ver front fporch
	// ---4----  ver blank period
	// ----3---  screen height
	// -----2--  hor front porch
	// ------1-  hor blank period
	// -------0  screen width

	m_monitor_bits |= monitor_bits;

	// mostly to make screen positioning tools behave decently
	m_empty_screen_on_update = 2;

	// don't adjust monitor params unless all six screen params in ports 10h-12h and 14h-16h are set
	// horizontal/vertical rates in 10h and 14h are set in BIOS and not changed in normal video mode switching
	// expect changing params in order AND vertical front porch in 16h as the final step
	if (monitor_bits == 0b0010'0000 && m_monitor_bits == 0b0011'1111) {
		m_screen->set_visarea(m_hbporch, m_hbporch + m_width - 1, m_vbporch, m_vbporch + m_height - 1);
		m_monitor_bits = 0b000'01001;
	}
}

uint32_t juku_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{	      
	int y_max = (m_vbporch < 0) ? (m_height + m_vbporch) : (m_vbporch+m_height > m_screen->height()) ? m_screen->height() : m_height;
	int x_max = (m_hbporch < 0) ? (m_width + m_hbporch) : (m_hbporch+m_width > m_screen->width()) ? m_screen->width() : m_width;
	
	if (m_empty_screen_on_update) {
		m_empty_screen_on_update--;
		bitmap.fill(0);
	}

	for (int y = 0; y < y_max; y++) {
		uint32_t *dest = &bitmap.pix(std::max(y+m_vbporch, 0)) + std::max(m_hbporch, 0);
		for (int x = 0; x < x_max; x++)
			*dest++ = BIT(m_ram[0xd800 + (y * (m_width / 8) + x / 8)], 7 - (x % 8)) ? rgb_t::white() : rgb_t::black();
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
	m_fdc_cur_cmd = data;
	m_fdc->cmd_w(data);
}

uint8_t juku_state::fdc_data_r()
{
	// on read commands (100xxxxx, 11000xxx, 11100xxx) and fdc reports busy
	if ( ((m_fdc_cur_cmd >> 5) == 0x4 || (m_fdc_cur_cmd >> 5) == 0x6 || (m_fdc_cur_cmd >> 3) == 0x1c) && m_fdc->drq_r() == 0 && (m_fdc->status_r() & 0x1) == 0x1 ) {
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
	if ( ((m_fdc_cur_cmd >> 5) == 0x5 || (m_fdc_cur_cmd >> 3) == 0x1e) && m_fdc->drq_r() == 0 && (m_fdc->status_r() & 0x1) == 0x1 )	{
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

void juku_state::speaker_w(int state)
{
	m_speaker->level_w((m_beep_state = state) << m_beep_level);
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

	uint8_t col_data = m_keys[data & 0xf]->read();

	for (int i = 0; i < 6; i++)
		m_key_encoder->input_line_w(i, BIT(col_data, i));
	m_contrdat = (col_data & 0b1100'0000) >> 6; // decoded by 2x К555ИД7
	
	if (m_beep_level != BIT(data, 4))
		m_speaker->level_w(m_beep_state << (m_beep_level = BIT(data, 4)));

	m_key_encoder->update();
}

uint8_t juku_state::pio0_portb_r()
{
	// 7-------  ctrl
	// -6------  shift
	// --54----  contrdat?
	// ----321-  keyboard data
	// -------0  key pressed

	uint8_t data = 0;

	data |= m_key_special->read();
	data |= m_contrdat << 4;
	data |= m_key_encoder->output_r() << 1;
	data |= m_key_encoder->output_valid_r();

	return data;
}

void juku_state::pio0_portc_w(uint8_t data)
{
	// 7-------  (cas?) pof
	// -6------  (cas?) stop / floppy side select
	// --5-----  (cas?) rn / floppy drive select
	// ---4----  (cas?) ff / floppy density (sd = 1, dd = 0)
	// ----3---  (cas?) play / floppy size (8" = 1, 5.25" = 0)
	// -----2--  (cas?) rec / floppy motor (on = 1, off = 0)
	// ------10  memory mode

	floppy_image_device *floppy = m_floppy[BIT(data, 5)]->get_device();
	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(BIT(data,4));
	// TODO: 8" floppy select

	if (floppy) {
		floppy->mon_w(!BIT(data,2));
		floppy->ss_w(BIT(data, 6));
	}

	m_bank->set_bank(data & 0x03);
}

void juku_state::machine_start()
{
	m_ram = std::make_unique<uint8_t []>(0x10000);

	membank("rom_d800")->set_base(memregion("maincpu")->base() + 0x1800);

	membank("ram_0000")->set_base(&m_ram[0x0000]);
	membank("ram_4000")->set_base(&m_ram[0x4000]);
	membank("ram_c000")->set_base(&m_ram[0xc000]);

	// register for save states
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_hbporch));
	save_item(NAME(m_vbporch));
	save_item(NAME(m_contrdat));
	save_item(NAME(m_monitor_bits));
	save_item(NAME(m_height_lsb));
	save_item(NAME(m_vblank_period_lsb));
	save_item(NAME(m_empty_screen_on_update));
	save_item(NAME(m_beep_state));
	save_item(NAME(m_beep_level));
	save_item(NAME(m_fdc_cur_cmd));
	save_pointer(NAME(m_ram), 0x10000);
}

void juku_state::machine_reset()
{
	m_bank->set_bank(0);
	m_key_encoder->enable_input_w(0);
	m_beep_state = 0;
	m_beep_level = 0;
	m_contrdat = 0b11;
	m_fdc_cur_cmd = 0;
	m_width = DEFAULT_WIDTH, m_height = DEFAULT_HEIGHT;
	m_hbporch = HORIZ_BACK_PORCH, m_vbporch = VERT_BACK_PORCH;
	m_height_lsb = -1;
	m_vblank_period_lsb = -1;
	m_monitor_bits = 0U;
	m_empty_screen_on_update = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void juku_state::juku(machine_config &config)
{
	// КР580ВМ80A @ 2 MHz (=KP580ИK80А)
	I8080A(config, m_maincpu, 20_MHz_XTAL/10);
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

	// КР580ВИ53 #1
	PIT8253(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(16_MHz_XTAL/16); // РК171 16000kHz variations on board
	m_pit[0]->set_clk<1>(16_MHz_XTAL/16); // РК170ББ-14ГC 16000kHz in specs
	m_pit[0]->set_clk<2>(16_MHz_XTAL/16);
	m_pit[0]->out_handler<0>().set(m_pit[1], FUNC(pit8253_device::write_clk0));
	m_pit[0]->out_handler<0>().append(m_pit[0], FUNC(pit8253_device::write_gate1));
	m_pit[0]->out_handler<0>().append(m_pit[0], FUNC(pit8253_device::write_gate2));
	//m_pit[0]->out_handler<1>().set(, ); // HOR RTR
	//m_pit[0]->out_handler<1>().append(, ); // HOR RTR

	// КР580ВИ53 #2
	PIT8253(config, m_pit[1], 0);
	m_pit[0]->out_handler<2>().set(m_pit[1], FUNC(pit8253_device::write_clk1)); // H SYNC DSL
	m_pit[0]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk2));
	m_pit[1]->out_handler<0>().append(m_pit[1], FUNC(pit8253_device::write_gate1));
	m_pit[1]->out_handler<0>().append(m_pit[1], FUNC(pit8253_device::write_gate2));
	m_pit[1]->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir5_w)); // VER RTR / FRAME INT
	
	//m_pit[1]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk1)); // VERT SYNC DSL
	//m_pit[1]->out_handler<2>().append(m_pit[1], FUNC(pit8253_device::write_clk2));

	// КР580ВИ53 #3
	PIT8253(config, m_pit[2], 0);

	m_pit[2]->set_clk<0>(16_MHz_XTAL/13); // 1.23 MHz
	m_pit[2]->set_clk<1>(16_MHz_XTAL/8); // 2 MHz
	m_pit[2]->set_clk<2>(16_MHz_XTAL/13); // 1.23 MHz

	//m_pit[1]->out_handler<0>().append(...); // BAUD RATE
	m_pit[2]->out_handler<1>().append(FUNC(juku_state::speaker_w)); // SOUND
	//m_pit[1]->out_handler<2>().append(...); // SYNC BAUD RATE

	// КР580ВВ55A #1 (=КР580ИК55)
	I8255A(config, m_pio[0]);
	m_pio[0]->out_pa_callback().set(FUNC(juku_state::pio0_porta_w));
	m_pio[0]->in_pb_callback().set(FUNC(juku_state::pio0_portb_r));
	m_pio[0]->out_pc_callback().set(FUNC(juku_state::pio0_portc_w));

	// КР580ВВ55A #2
	I8255A(config, m_pio[1]);

	// КР580ВВ51A
	I8251(config, m_sio[0], 0);
	m_sio[0]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir2_w));
	m_sio[0]->txrdy_handler().set("pic", FUNC(pic8259_device::ir3_w));

	// КР580ВВ51A (instead of FDC?)
	I8251(config, m_sio[1], 0);
	m_sio[1]->rxrdy_handler().set("pic", FUNC(pic8259_device::ir0_w));
	m_sio[1]->txrdy_handler().set("pic", FUNC(pic8259_device::ir1_w));

	// Электроника МС 6105.1 "Колокольчик" (DEC VR201 analog)
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL/16, HORIZ_PERIOD/8, HORIZ_BACK_PORCH, HORIZ_BACK_PORCH+DEFAULT_WIDTH, VERT_PERIOD, VERT_BACK_PORCH, VERT_BACK_PORCH+DEFAULT_HEIGHT);
	m_screen->set_size(HORIZ_PERIOD, VERT_PERIOD);
	m_screen->set_screen_update(FUNC(juku_state::screen_update));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1);
	m_speaker->set_levels(3, SPEAKER_LEVELS);

	// К155ИВ1
	TTL74148(config, m_key_encoder, 0);

	// КР1818ВГ93 (for E6502 disk drive)
	KR1818VG93(config, m_fdc, 16_MHz_XTAL/16);
	m_fdc->drq_wr_callback().set(FUNC(juku_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", juku_floppies, "525qd", juku_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", juku_floppies, "525qd", juku_state::floppy_formats);

	SOFTWARE_LIST(config, "floppy_list").set_original("juku");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( juku )
	ROM_DEFAULT_BIOS("3.43m_37")
	ROM_REGION(0x4000, "maincpu", 0)

	// Monitor 3.3 with Bootstrap 3.3, FDC 1791 from early 1985 prototype
	// Does not seem to be compatible with JBASIC extension cartridge
	ROM_SYSTEM_BIOS(0, "jmon3.3", "Monitor/Bootstrap 3.3 \\w JBASIC")
	ROMX_LOAD("jmon33.bin", 0x0000, 0x4000, CRC(ed22c287) SHA1(76407d99bf83035ef526d980c9468cb04972608c), ROM_BIOS(0))

	// RomBios 3.42 with Janet 1.2/Bootstrap 4.1, screen 53x24, FDC 1791/2 from Juss prototype (E5103?)
	// Id: "EktaSoft '88  Serial #0024"
	ROM_SYSTEM_BIOS(1, "3.42_24", "Disk/Net \\w JUSS keyb (3.42 #0024)")
	ROMX_LOAD("ekta24.bin", 0x0000, 0x4000, CRC(6ce7ee3b) SHA1(a7185d747c94cd519868692ed3d10fade90dd6d5), ROM_BIOS(1))

	// RomBios 2.43m with TapeBios/Bootstrap 4.1, screen 53x24 (true E5101?)
	// Id: "EktaSoft '88  Serial #0032"
	ROM_SYSTEM_BIOS(2, "2.43m_32", "Tape/Disk (2.43m #0032)")
	ROMX_LOAD("ekta32.bin", 0x0000, 0x4000, CRC(72c0da53) SHA1(57311d53f6fe1e87e0755990f400253caccd4795), ROM_BIOS(2))

	// RomBios 3.43m with Janet 1.2/Bootstrap 4.1, screen 53x24 from Juss prototype (E5103?)
	// Id: "EktaSoft '88  Serial #0035"
	ROM_SYSTEM_BIOS(3, "3.43m_35", "Disk/Net \\w JUSS keyb (3.43m #0035)")
	ROMX_LOAD("ekta35.bin", 0x0000, 0x4000, CRC(85a017bc) SHA1(7aa03497d88cfab9315aa3987765bc06ecb70013), ROM_BIOS(3))

	// RomBios 3.43m with Janet 1.2/Bootstrap 4.1 from widespread Baltijets batch (E5104)
	// Id: "EktaSoft '88  Serial #0037"
	ROM_SYSTEM_BIOS(4, "3.43m_37", "Disk/Net (3.43m #0037)")
	ROMX_LOAD("ekta37.bin", 0x0000, 0x4000, CRC(2c1c9cad) SHA1(29366d74c0e27129f2484a973f7a6de659b90cf4), ROM_BIOS(4))

	// RomBios 2.43m with TapeBios/Bootstrap 4.1, screen 53x24, modified for IBM AT keyboard (homebrew)
	// Id: "EktaSoft '90  Serial #0043"
	ROM_SYSTEM_BIOS(5, "2.43m_43", "Tape/Disk \\w AT keyb (2.43m #0043)")
	ROMX_LOAD("ekta43.bin", 0x0000, 0x4000, CRC(05678f9f) SHA1(a7419bfd8249871cc7dbf5c6ea85022d6963fc9a), ROM_BIOS(5))

	ROM_REGION(0x8000, "extension", 0)

	// EKTA JBASIC cartridge (buggy) seems similar to v1.1 from 14.09.1987.
	// There is also a version with additional HEX$ directive for EKDOS.
	// Initial E5101 had JBASIC onboard with early RomBios/Monitor versions.
	ROMX_LOAD("jbasic11.bin", 0x0000, 0x2000, CRC(bdc471ca) SHA1(3d96ba589aa21d44412efb099a144fbe23a2f52f), ROM_BIOS(1))
	ROMX_LOAD("jbasic11.bin", 0x0000, 0x2000, CRC(bdc471ca) SHA1(3d96ba589aa21d44412efb099a144fbe23a2f52f), ROM_BIOS(2))
	ROMX_LOAD("jbasic11.bin", 0x0000, 0x2000, CRC(bdc471ca) SHA1(3d96ba589aa21d44412efb099a144fbe23a2f52f), ROM_BIOS(3))
	ROMX_LOAD("jbasic11.bin", 0x0000, 0x2000, CRC(bdc471ca) SHA1(3d96ba589aa21d44412efb099a144fbe23a2f52f), ROM_BIOS(4))
	ROMX_LOAD("jbasic11.bin", 0x0000, 0x2000, CRC(bdc471ca) SHA1(3d96ba589aa21d44412efb099a144fbe23a2f52f), ROM_BIOS(5))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME      FLAGS
COMP( 1988, juku, 0,      0,      juku,    juku,  juku_state, empty_init, "EKTA",  "Juku E5104", MACHINE_SUPPORTS_SAVE)
