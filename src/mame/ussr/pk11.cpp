// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Soyuz-Neon PK-11/16K (experimental desktop)

    References:
    - https://github.com/troosh/pk11-16/wiki
    - https://github.com/nzeemin/neonbtl

    todo
    - init and save_item
    - video mode quirks (vn 0, vn 0/1 + vd 0, 1bpp/2bpp palette layout), borders
    - console window palette
    - odd address vs. io_rw
    - bit 21 in 22-bit address fix

    debug
    - kbd (stop button)
    - hdd

    add
    - mouse
    - printer
    - ata
    - slot (uses UR7 and HR7?)

    hw revisions
    - A: IRQ 5 rate = 64 Hz, driven by RTC (SQW output)
      two expansion slots, use IRQs 6,7 and registers UR7 and HR7
      schematic: pk11-16-sch-20180617.pdf
    - B: IRQ 5 rate = 50 Hz, driven by vblank
      one expansion slot, uses IRQ 7
      IRQ 6 -- printer port
      schematic: tbd

    irqs

    0	RESET insn
    1	fdc, hdc
    2	rxrdy
    3	txrdy
    4	kbdc
    5	vblank || rtc
    6	printer
    7	slot

****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "formats/bk0010_dsk.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "machine/wd1010.h"
#include "emupal.h"
#include "screen.h"


#define LOG_MMU       (1U << 1)
#define LOG_DEBUG     (1U << 2)
#define LOG_VIDEO     (1U << 3)
#define LOG_DISK      (1U << 4)

// #define VERBOSE (LOG_DEBUG | LOG_DISK)
#include "logmacro.h"

#define LOGMMU(...) LOGMASKED(LOG_MMU, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)
#define LOGVIDEO(...) LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGDISK(...) LOGMASKED(LOG_DISK, __VA_ARGS__)

#define BUS_ERROR do { m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); } while (0)


namespace {

// these are unverified
static constexpr int PK11_TOTAL_HORZ = 1024;
static constexpr int PK11_DISP_HORZ = 832;
static constexpr int PK11_HORZ_START = 0;

static constexpr int PK11_TOTAL_VERT = 312;
static constexpr int PK11_DISP_VERT = 300;
static constexpr int PK11_VERT_START = 0;


typedef struct
{
	uint32_t addr[26];
	uint16_t mode[26];
	int border[26];
} pk11_scanline;


class pk11_state : public driver_device
{
public:
	pk11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic8259")
		, m_upd8251(*this, "upd8251")
		, m_pit8253_1(*this, "pit8253_1")
		, m_pit8253_2(*this, "pit8253_2")
		, m_ppi8255(*this, "ppi8255")
		, m_kdc(*this, "kdc")
		, m_fdc(*this, "fdc")
		, m_hdc(*this, "hdc")
		, m_hdd(*this, "hdc:1")
		, m_rtc(*this, "rtc")
		, m_halt_merger(*this, "merge_halt")
		, m_irq1_merger(*this, "merge_irq1")
		, m_screen(*this, "screen")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_buttons(*this, "mouse_buttons")
		, m_ram(*this, "ram")
		, m_view_main(*this, "view_main")
	{ }

	void pk11(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(mouse_x_changed);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y_changed);
	DECLARE_INPUT_CHANGED_MEMBER(buttons_changed);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t vdpword(uint32_t offset);
	void update_displaylist();

	uint16_t mmu_user_r(offs_t offset);
	uint16_t mmu_halt_r(offs_t offset);
	void mmu_user_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void mmu_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t mmap_r(uint16_t reg, uint32_t addr);
	void mmap_w(uint16_t reg, uint32_t addr, uint16_t data, uint16_t mem_mask);

	uint16_t mmap_user_r(offs_t offset);
	uint16_t mmap_halt_r(offs_t offset);
	void mmap_user_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void mmap_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t hdc_r(offs_t offset);
	void hdc_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t buf_r();
	void buf_w(uint16_t data);

	uint16_t fdc_buf_r();
	void fdc_buf_w(uint16_t data);

	void fdc_drq_w(int state);

	void pk11_mem(address_map &map) ATTR_COLD;

	int m_odt_map, m_halt, m_ioint, m_ef[2];
	uint16_t m_ur[8], m_hr[8], *m_p_ram;
	uint32_t m_vdpmask;

	std::unique_ptr<pk11_scanline[]> m_scanlines;
	uint32_t m_palette[1024]{};

	static void floppy_formats(format_registration &fr);
	floppy_image_device *floppy[2];

	int8_t m_x, m_y, m_sr;
	uint8_t kbd_r(), m_digit;

	std::map<int, std::string> syscall_map;

	bool m_buf_dir, drq;
	uint16_t m_buf_ptr;
	std::unique_ptr<uint8_t[]> m_buf;

	TIMER_CALLBACK_MEMBER(timer_tick);
	emu_timer *m_timer;

protected:
	required_device<k1801vm2_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<i8251_device> m_upd8251;
	required_device<pit8253_device> m_pit8253_1;
	required_device<pit8253_device> m_pit8253_2;
	required_device<i8255_device> m_ppi8255;
	required_device<i8279_device> m_kdc;
	required_device<upd765_family_device> m_fdc;
	required_device<wd1010_device> m_hdc;
	optional_device<harddisk_image_device> m_hdd;
	required_device<mc146818_device> m_rtc;
	required_device<input_merger_any_high_device> m_halt_merger;
	required_device<input_merger_any_high_device> m_irq1_merger;
	required_device<screen_device> m_screen;
	required_ioport_array<8> m_io_keyboard;
	required_ioport m_buttons;

	required_device<ram_device> m_ram;
	memory_view m_view_main;
};

//

INPUT_CHANGED_MEMBER(pk11_state::mouse_x_changed)
{
	m_x += newval - oldval;
	LOGVIDEO("X %3d->%3d d %3d m_x %3d\n", oldval, newval, newval-oldval, m_x);
}

INPUT_CHANGED_MEMBER(pk11_state::mouse_y_changed)
{
	m_y += newval - oldval;
	LOGVIDEO("Y %3d->%3d d %3d m_y %3d\n", oldval, newval, newval-oldval, m_y);
}

INPUT_CHANGED_MEMBER(pk11_state::buttons_changed)
{
	m_sr = m_buttons->read();
}

/*
 * row 8 (signal KX9 in XS1 connector) is dedicated to STOP key; wired together with row 2 (KX3)
 * columns 0..11 (signals KY1..KY12 in XS2 connector) are mapped to RL bits:
 *
 * 0	RL001	KY3 KY6 KY9
 * 1	RL002	KY4 KY6 KY11
 * 2	RL004	KY5 KY9 KY11
 * 3	RL010	KY7
 * 4	RL020	KY8
 * 5	RL040	KY10
 * 6	RL100	KY1 KY12
 * 7	RL200	KY2
 */
uint8_t pk11_state::kbd_r()
{
	if (m_digit > 7) return 0xff;

	uint16_t raw = bitswap<16>(m_io_keyboard[m_digit]->read(),
		0, 0, 0, 0, 4, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5);

	uint8_t data = bitswap<8>(raw, 1, 0, 9, 7, 6, 4, 3, 2) &
		bitswap<8>(raw, 15, 11, 15, 15, 15, 8, 5, 5) &
		bitswap<8>(raw, 15, 15, 15, 15, 15, 10, 10, 8);

	return data;
}

// keyboard is ms7007

INPUT_PORTS_START(pk11)

	// bit 15 = col 11, bit 5 = col 1, bit 4 = col 12
        PORT_START("X0")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AR2 (Esc)") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

        PORT_START("X1")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a) PORT_CHAR(0x0a)
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CHAR(0x05)
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

        PORT_START("X2")
        PORT_BIT( 0x002F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("<STOP>") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CHAR(0x06)
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CHAR(0x03)
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CHAR(0x15)
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CHAR(0x0b)
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CHAR(0x10)
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CHAR(0x0e)
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CHAR(0x07)
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CHAR(0x0c)
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CHAR(0x04)

        PORT_START("X3")
        PORT_BIT( 0x003F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Graf") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CHAR(0x11)
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CHAR(0x19)
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CHAR(0x17)
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CHAR(0x01)
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09) PORT_CHAR(0x09)
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CHAR(0x12)
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CHAR(0x0f)
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CHAR(0x02)
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(": @") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('`')

        PORT_START("X4")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Alf") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ch")
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CHAR(0x13)
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d) PORT_CHAR(0x0d)
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CHAR(0x14)
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CHAR(0x18)
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Cursor left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

        PORT_START("X5")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UTF8_LEFT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Cursor right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Cursor down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CHAR(0x16)

        PORT_START("X6")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("<ISP> (Go)") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F6))
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("<UST> (Setup)") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F8))
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Cursor up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(": hardsign") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08) PORT_CHAR(0x08)
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CHAR(0x1a)

        PORT_START("X7")
        PORT_BIT( 0x001F, IP_ACTIVE_LOW, IPT_UNUSED )
        PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
        PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad Enter") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
        PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
        PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
        PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("<SBROS> (Reset)") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F7))
        PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("<POM> (Help)") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F9))
        PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*') // FIXME
        PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // FIXME
        PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') // FIXME
        PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
        PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("mouse_x")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pk11_state::mouse_x_changed), 0)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pk11_state::mouse_y_changed), 0)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pk11_state::buttons_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(pk11_state::buttons_changed), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void pk11_state::pk11_mem(address_map &map)
{
	map(0000000, 0177777).view(m_view_main);
	map(0160000, 0167777).lrw16(
		NAME([this](offs_t offset) { if (!machine().side_effects_disabled()) BUS_ERROR; return 0; }),
		NAME([this](offs_t offset, uint16_t data) { BUS_ERROR; }));

	// USER mode
	m_view_main[0](0000000, 0157777).rw(FUNC(pk11_state::mmap_user_r), FUNC(pk11_state::mmap_user_w));
	m_view_main[0](0170000, 0177677).rw(FUNC(pk11_state::io_r), FUNC(pk11_state::io_w)); // 174000 and up trigger HALT irq
	m_view_main[0](0177700, 0177777).lrw16(
		NAME([this](offs_t offset) { if (!machine().side_effects_disabled()) BUS_ERROR; return 0; }),
		NAME([this](offs_t offset, uint16_t data) { BUS_ERROR; }));

	// HALT mode
	m_view_main[1](0000000, 0037777).rom().region("maincpu", 0);
	m_view_main[1](0040000, 0157777).rw(FUNC(pk11_state::mmap_halt_r), FUNC(pk11_state::mmap_halt_w));
	m_view_main[1](0170000, 0177677).lrw16(
		NAME([this](offs_t offset) { return m_p_ram[offset]; }),
		NAME([this](offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_p_ram[offset]); }));

	map(0161000, 0161003).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xff);
	map(0161010, 0161017).rw(m_pit8253_1, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff);
	map(0161020, 0161027).rw(m_pit8253_2, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff);
	map(0161030, 0161037).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff);
	map(0161040, 0161057).rw(FUNC(pk11_state::hdc_r), FUNC(pk11_state::hdc_w));
	map(0161060, 0161063).rw(m_upd8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff);
	map(0161064, 0161067).rw(m_kdc, FUNC(i8279_device::read), FUNC(i8279_device::write)).umask16(0xff);
	map(0161070, 0161073).m(m_fdc, FUNC(upd765a_device::map)).umask16(0xff);
	map(0161076, 0161077).rw(FUNC(pk11_state::fdc_r), FUNC(pk11_state::fdc_w));
	map(0161200, 0161217).rw(FUNC(pk11_state::mmu_halt_r), FUNC(pk11_state::mmu_halt_w));
	map(0161220, 0161237).rw(FUNC(pk11_state::mmu_user_r), FUNC(pk11_state::mmu_user_w));
	map(0161400, 0161477).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).flags(t11_device::UNALIGNED_BYTE);
}

static DEVICE_INPUT_DEFAULTS_START( host_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


void pk11_state::machine_reset()
{
	m_odt_map = 1; m_view_main.select(1);
	m_halt = m_ioint = 0;
	m_ef[0] = m_ef[1] = 0;

	memset(m_palette, 0, sizeof(m_palette));

	m_x = m_y = m_digit = 0;

	m_buf_ptr = 0;
	m_buf_dir = false; // READ
	drq = false;
}

void pk11_state::machine_start()
{
	m_p_ram = (uint16_t *) m_ram->pointer();
	m_vdpmask = (m_ram->size() >> 1) - 1;

	m_scanlines = make_unique_clear<pk11_scanline[]>(PK11_DISP_VERT);
	m_buf = make_unique_clear<uint8_t[]>(2048);
	m_hdc->drdy_w(m_hdd->exists());
	m_hdc->head_w(0);

	for (int i = 0; i < 2; i++)
	{
		char name[2] = { static_cast<char>('0' + i), 0 };
		floppy_connector *conn = m_fdc->subdevice<floppy_connector>(name);
		floppy[i] = conn ? conn->get_device() : nullptr;
	}

	m_timer = timer_alloc(FUNC(pk11_state::timer_tick), this);
	m_timer->adjust(attotime::never, 0, attotime::never);

	for (int i = 0; i < 8; i++)
	{
		m_ur[i] = m_hr[i] = 0;
	}

	// FIXME
	save_item(NAME(m_buf_ptr));
	save_pointer(NAME(m_buf), 2048);

	syscall_map = {
		// syscalls
		{0174176, "GET4K"},
		{0174200, "GET8K"},
		{0174202, "FREMEM"},
		{0174174, "FRE4K"},
		{0174204, "PUTMAP"},
		{0174206, "GETBMAP"},
		{0174210, "GETSMAP"},
		{0174166, "NEWROMP"},
		{0174214, "NEWPROC"},
		{0174222, "KILPROC"},
		{0174152, "SETPRI"},
		{0174212, "HIMPRI"},
		{0174154, "PROVEC"},
		{0174156, "UNPVEC"},
		{0174160, "PROREG"},
		{0174162, "UNPREG"},
		{0174164, "WAITINT"},
		{0174170, "SETINT"},
		{0174172, "RESINT"},
		{0174216, "MTHLT"},
		{0174220, "MFHLT"},
		{0174142, "INITSEM"},
		{0174144, "RELSEM"},
		{0174146, "WAITSEM"},
		{0174150, "SIGSEM"},
		{0174224, "GETPDS"},
		{0174226, "PUTPDS"},
		{0174230, "GETRDS"},
		{0174232, "PUTRDS"},
		{0174234, "GETPAR"},
		{0174236, "PUTPAR"},
		{0174000, "reboot"},
		// devices
		{0177130, "HFBUF"},	// floppy and hard disk, vectors 230 (hdvec), 234 (fdvec)
		{0177144, "MDCSR"},	// ram disk
		{0177146, "MDBUF"},
		{0177150, "MDSIZ"},
		{0176500, "RCSR"},	// serial port, vector 300
		{0176502, "RBUF"},
		{0176504, "TCSR"},
		{0176506, "TBUF"},
		{0177530, "WCSR"},	// window system and manager
		{0177524, "WMCSR"},
		{0177564, "DCSR"},	// console, vector 64
		{0177566, "DBUF"},
		{0177174, "HOLER"},	// calculator
		{0174060, "FILCSR"},	// filer
		{0174062, "FMCSR"},
		{0174064, "FCCSR"},
		{0177600, "GCSR"},	// graphics display
		{0176200, "MXX"},	// mouse
		{0176202, "MYY"},
		{0176204, "MCSR"},
		{0176206, "MCALL"},
		{0176240, "SNCSR"},	// sound
		{0176242, "SNBUF"},
		{0177110, "CLKREG"},	// aux display
		{0174100, "DCHAR"},
		{0174102, "DSTRING"},
		{0174104, "DVAL"},
		{0174106, "KSTRING"},
		{0174110, "RESPON"},
		{0177560, "KBCSR"},	// keyboard, vector 60
		{0177562, "KBBUF"},
		{0174052, "CNSBUF"},	// debugger
		{0174054, "CNSKIL"},
		// undocumented
		{0177546, "CLKCSR"},
		{0176220, "COCO"},
		{0176216, "COCS"},
	};
}

void pk11_state::mmu_user_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMMU("UR%d W <- %06o (A21:12 = %4o, access %d)\n", offset, data, BIT(data, 4, 10), BIT(data, 3));
	COMBINE_DATA(&m_ur[offset]);
}

uint16_t pk11_state::mmu_user_r(offs_t offset)
{
	uint16_t data = m_ur[offset];
	LOGMMU("UR%d R == %06o\n", offset, data);
	return data;
}

void pk11_state::mmu_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!m_odt_map)
	{
		BUS_ERROR;
		return;
	}
	LOGMMU("HR%d W <- %06o (A21:12 = %4o, access %d)\n", offset, data, BIT(data, 4, 10), BIT(data, 3));
	COMBINE_DATA(&m_hr[offset]);
	if (offset == 0 || offset == 1)
	{
		m_ef[0] = m_ef[1] = 0;
		m_halt_merger->in_w<0>(CLEAR_LINE);
	}
}

uint16_t pk11_state::mmu_halt_r(offs_t offset)
{
	uint16_t data = m_hr[offset];
	LOGMMU("HR%d R == %06o\n", offset, data);
	return data;
}


void pk11_state::fdc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGDISK("disk fdc_w <- %04x (buf %d)\n", data, data & 3);
	m_buf_ptr = (data & 3) << 9;
	if (data & 020) m_fdc->reset();
}

uint16_t pk11_state::fdc_r(offs_t offset)
{
	return 0;
}

void pk11_state::fdc_drq_w(int state)
{
	if (drq == state) return;
	drq = state;
	if (!drq)
	{
		m_timer->adjust(attotime::never, 0, attotime::never);
		return;
	}

	LOGDISK("disk drq (%d, %d)\n", m_buf_dir, m_buf_ptr);
	m_timer->adjust(attotime::from_usec(16)); // FIXME timing
}

TIMER_CALLBACK_MEMBER(pk11_state::timer_tick)
{
	int tc = m_buf_ptr == 2047;
	m_fdc->tc_w(tc);
	if (m_buf_dir)
		fdc_buf_w(m_fdc->dma_r());
	else
	{
		m_fdc->dma_w(fdc_buf_r());
		if (!tc && drq) m_timer->adjust(attotime::from_usec(16)); // FIXME timing
	}
}

void pk11_state::hdc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0:
		if (!m_buf_dir) return;
		LOGDISK("disk hdc_w[%4d] <- %02x\n", m_buf_ptr, data);
		m_buf[m_buf_ptr++] = data;
		m_buf_ptr &= 2047;
		return;

	case 6 >> 1: // SNUM
		// workaround for non-standard sector numbering
		if (data == 255) data = 0;
		data &= 31;
		break;

	case 014 >> 1: // SDH
		m_buf_dir = true;
		if (!BIT(data, 3))
		{
			LOGDISK("disk dor %02x (side %d dens %d drive %d motor %d)\n", data, BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 4));
			// 0	side select
			// 1	density
			// 2	drive
			// 3	0=floppy 1=hdd
			// 4	motor
			for (int i=0; i<2; i++)
				if (floppy[i])
					floppy[i]->mon_w(!BIT(data, 4));
			m_fdc->set_floppy(floppy[BIT(data, 2)]);

			return;
		}
		else
		{
			data &= ~0x10; // FIXME
			m_hdc->head_w(data & 7);
		}
		break;

	case 016 >> 1:
		m_buf_dir = false;
		break;
	}

	m_hdc->write(offset, data);
}

uint16_t pk11_state::hdc_r(offs_t offset)
{
	uint16_t data;

	switch (offset)
	{
	case 0:
		if (m_buf_dir) return 0;
		data = m_buf[m_buf_ptr];
		LOGDISK("disk hdc_r[%4d] == %02x\n", m_buf_ptr, data);
		if (!machine().side_effects_disabled())
			m_buf_ptr = (m_buf_ptr + 1) & 2047;
		return data;

	case 014 >> 1:
		if (!machine().side_effects_disabled())
			m_buf_dir = true;
		break;

	case 016 >> 1:
		if (!machine().side_effects_disabled())
			m_buf_dir = false;
		break;
	}

	return m_hdc->read(offset);
}

void pk11_state::fdc_buf_w(uint16_t data)
{
	LOGDISK("disk fdc_buf_w[%4d] <- %02x\n", m_buf_ptr, data);
	m_buf[m_buf_ptr++] = data;
	m_buf_ptr &= 2047;
}

uint16_t pk11_state::fdc_buf_r()
{
	uint16_t data = m_buf[m_buf_ptr];
	LOGDISK("disk fdc_buf_r[%4d] == %02x\n", m_buf_ptr, data);
	if (!machine().side_effects_disabled())
		m_buf_ptr = (m_buf_ptr + 1) & 2047;
	return data;
}

void pk11_state::buf_w(uint16_t data)
{
	LOGDISK("disk buf_w[%4d] <- %02x\n", m_buf_ptr, data);
	m_buf[m_buf_ptr] = data;
	m_buf_ptr = ((m_buf_ptr + 1) & 511) | (m_buf_ptr & (3 << 9));
}

uint16_t pk11_state::buf_r()
{
	uint16_t data = m_buf[m_buf_ptr];
	LOGDISK("disk buf_r[%4d] == %02x\n", m_buf_ptr, data);
	if (!machine().side_effects_disabled())
		m_buf_ptr = ((m_buf_ptr + 1) & 511) | (m_buf_ptr & (3 << 9));
	return data;
}


void pk11_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t a = 0170000 + (offset << 1); // + (mem_mask == 0xff00);
	LOGDBG("%s IO W %06o <- %06o ; %s\n", machine().describe_context(), a, data, syscall_map[a].c_str());
	COMBINE_DATA(&m_p_ram[(offset & 03777)]);
	if (offset >= 02000)
	{
		if (!m_ef[0] || m_hr[0] == a) // XXX rmw
			m_hr[0] = a;
		else
			m_hr[1] = a;
		m_ef[0] = m_ef[1] = 1;
		m_halt_merger->in_w<0>(ASSERT_LINE);
	}
}

uint16_t pk11_state::io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t a = 0170000 + (offset << 1); // + (mem_mask == 0xff00);
	uint16_t data = m_p_ram[(offset & 03777)];
	LOGDBG("%s IO R %06o == %06o ; %s\n", machine().describe_context(), a, data, syscall_map[a].c_str());
	if (offset >= 02000 && !machine().side_effects_disabled())
	{
		if (!m_ef[0])
			m_hr[0] = a;
		else
			m_hr[1] = a;
		m_ef[0] = 1;
		m_halt_merger->in_w<0>(ASSERT_LINE);
	}
	return data;
}

/*
 * write modes
 *
 * 0 -- mask unchanged
 * 1 -- mask for 2bpp modes
 * 2 -- mask for 4bpp modes
 */
void pk11_state::mmap_w(uint16_t reg, uint32_t addr, uint16_t data, uint16_t mem_mask)
{
	if ((addr << 1) >= m_ram->size() || BIT(reg, 3))
	{
		BUS_ERROR;
		return;
	}

	uint16_t mask = 0;
	data &= mem_mask;

	if (BIT(reg, 0))
	{
		for (int i = 0; i < 16; i+=2)
		{
			if (BIT(data, i, 2)) mask |= (3 << i);
		}
		mem_mask &= mask;
	}
	else if (BIT(reg, 1))
	{
		for (int i = 0; i < 16; i+=4)
		{
			if (BIT(data, i, 4)) mask |= (15 << i);
		}
		mem_mask &= mask;
	}

	COMBINE_DATA(&m_p_ram[addr]);
}

uint16_t pk11_state::mmap_r(uint16_t reg, uint32_t addr)
{
	uint16_t data = 0;
	if ((addr << 1) >= m_ram->size() || BIT(reg, 3))
		BUS_ERROR;
	else
		data = m_p_ram[addr];
	return data;
}

void pk11_state::mmap_user_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t ur = BIT(offset, 12, 3);
	uint32_t addr = (offset & 07777) + (BIT(m_ur[ur], 4, 10) << 11);
	mmap_w(m_ur[ur], addr, data, mem_mask);
}

uint16_t pk11_state::mmap_user_r(offs_t offset)
{
	uint16_t ur = BIT(offset, 12, 3);
	uint32_t addr = (offset & 07777) + (BIT(m_ur[ur], 4, 10) << 11);
	return mmap_r(m_ur[ur], addr);
}

void pk11_state::mmap_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset += 020000;
	uint16_t hr = BIT(offset, 12, 3);
	uint32_t addr = (offset & 07777) + (BIT(m_hr[hr], 4, 10) << 11);
	mmap_w(m_hr[hr], addr, data, mem_mask);
}

uint16_t pk11_state::mmap_halt_r(offs_t offset)
{
	offset += 020000;
	uint16_t hr = BIT(offset, 12, 3);
	uint32_t addr = (offset & 07777) + (BIT(m_hr[hr], 4, 10) << 11);
	return mmap_r(m_hr[hr], addr);
}

void pk11_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BK0010_FORMAT);
}

static void pk11_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

uint32_t pk11_state::vdpword(uint32_t offset)
{
	return ((m_p_ram[offset] + (m_p_ram[offset + 1] << 16)) << 1) & m_vdpmask; // offset into m_p_ram
}

/*
 * VDPTAP points to palette list
 *   palette entries use RGB565 format
 *   512 entries -- 1,2,4bpp palettes (two sets, 4 palettes per mode): vm1, vm2, vm40, vm41 modes in each set (vm41 unused in set 1)
 *   512 entries -- 8bpp palettes (two palettes)
 *
 * VDPTAS points to 2-level display list (300 dwords)
 *   list entry format: 19:0 = top 20 bits of physical address, 25:20 and 31 = mode bits, 30:26 = slice length
 *   1st level entries use only address field
 *   max 26 slices per scanline (32 pixel clocks each)
 */
void pk11_state::update_displaylist()
{
	uint32_t paladdr = vdpword(004 >> 1);
	uint32_t tasaddr = vdpword(010 >> 1) + 4; // skip first 2 invisible entries
	uint32_t lineaddr, otraddr;
	uint16_t slicebits;
	int slice;

	/*
	 * 4 sets of 256 entries, layout is: 256 bytes of msb, then 256 bytes of lsb
	 */
	LOGVIDEO("paladdr %09o\n", paladdr);
	for (int c = 0; c < 1024; c++)
	{
		uint16_t hi = m_p_ram[paladdr + ((c >> 1) & 127) + (c & ~255)];
		uint16_t lo = m_p_ram[paladdr + ((c >> 1) & 127) + (c & ~255) + 128];
		uint16_t rgb;
		if (c & 1)
			rgb = (hi & 0xff00) | (lo >> 8);
		else
			rgb = (hi << 8) | (lo & 0xff);

		if (c < 4) LOGVIDEO("pal %d %04x\n", c, rgb);
		m_palette[c] = rgbexpand<5,6,5>(bitswap<16>(rgb, 12, 11, 10, 4, 3, 15, 14, 13, 7, 6, 5, 9, 8, 2, 1, 0), 11, 5, 0);
	}

	for (int i = 0; i < PK11_DISP_VERT; i++)
	{
		lineaddr = vdpword(tasaddr);
		tasaddr += 2;
		pk11_scanline *scanline = &m_scanlines[i];

		if (!i) LOGVIDEO("u_d\n");
		if (i<20) LOGVIDEO("%3d (%09o)=%09o\n", i, (tasaddr - 2) << 1, lineaddr << 1);
		for (int j = 0, bar = 26; bar > 0;)
		{
			otraddr = vdpword(lineaddr);
			slicebits = m_p_ram[lineaddr+1];
			lineaddr += 2;

			slice = 32 - BIT(slicebits, 10, 5);
			if (!slice) slice = 32;
			if (i < 20 && slice) LOGVIDEO("otr (%09o)=%09o %06o %2d  mode pn %d vn %d vd %d\n",
				(lineaddr - 2) << 1, otraddr << 1, slicebits, slice,
				BIT(slicebits, 4, 2), BIT(slicebits, 6, 2) + (BIT(slicebits, 15) << 2), BIT(slicebits, 8, 2));

			for (int first = 0 /* FIXME */; slice > 0 && bar > 0; slice--, bar--, j++)
			{
				const int vd = BIT(slicebits, 8, 2);
				scanline->addr[j] = otraddr;
				scanline->mode[j] = slicebits;
				scanline->border[j] = first;
				first = 0;
				otraddr += 1 << (vd ? vd - 1 : 0);
			}
		}
	}
}

uint32_t pk11_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_displaylist();

	for (int row = 0; row < PK11_DISP_VERT; row++)
	{
		uint32_t *p = &bitmap.pix(row);
		pk11_scanline *scanline = &m_scanlines[row];
		for (int g = 0; g < 26; g++)
		{
			// 0: 1bpp, 1: 2bpp, 2: 4bpp, 3: 4/8bpp
			int vn = BIT(scanline->mode[g], 6, 2);
			// vd 0,1: fetch 52 bytes per scanline, 2: 104, 3: 208 bytes XXX vd 0
			int vd = BIT(scanline->mode[g], 8, 2);
			int pb = (BIT(scanline->mode[g], 15) << 4) | (vn << 2) | BIT(scanline->mode[g], 4, 2);
			int wide = vd ? (1 << (vd - 1)) : 1;
			int zoom = vd ? (1 << (3 - vd)) : 4;

			if (scanline->border[g])
			{
				// prev
				for (int b1 = 0; b1 < 9; b1++) *p++ = m_palette[pb << 4];
				// border
				for (int b2 = 0; b2 < 2; b2++) *p++ = m_palette[0];
				// this
				for (int b3 = 0; b3 < 5; b3++) *p++ = m_palette[pb << 4];
			}
//			else
			for (int w = 0; w < wide; w++)
			{
				uint16_t gfx = m_p_ram[scanline->addr[g] + w];
				uint32_t color;

				switch (vn)
				{
				case 0:
					for (int px = 0; px < 16; px++)
					{
						color = m_palette[BIT(gfx, px) + 14 + (pb << 4)];
						*p++ = color;
						if (vd < 2) { *p++ = color; }
					}
					break;

				case 1:
					for (int px = 0; px < 16; px+=2)
					{
						color = m_palette[BIT(gfx, px, 2) + 12 + (pb << 4)];
						for (int j = 0; j < zoom; j++) *p++ = color;
					}
					break;

				case 2:
				case 3:
					for (int px = 0; px < 16; px+=4)
					{
						if (vn == 3 && BIT(scanline->mode[g], 15))
						{
							color = m_palette[BIT(gfx, px, 8) + (512 << BIT(scanline->mode[g], 4))];
							for (int j = 0; j < (zoom * 4); j++) *p++ = color;
							px += 4;
						}
						else
						{
							color = m_palette[BIT(gfx, px, 4) + (pb << 4)];
							for (int j = 0; j < (zoom * 2); j++) *p++ = color;
						}
					}
					break;

				}
			}
		}
	}

	return 0;
}

void pk11_state::pk11(machine_config &config)
{
	K1801VM2(config, m_maincpu, XTAL(30'800'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pk11_state::pk11_mem);
	m_maincpu->set_initial_mode(0);
	m_maincpu->in_iack().set([this] () -> uint8_t { return 0274; }); // vector timeout handler, adds 64T
	m_maincpu->out_reset().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_maincpu->out_bankswitch().set([this] (int state) {
		if (m_odt_map != state) {
			m_view_main.select(state); m_odt_map = state;
			// m_halt_merger->in_w<0>(CLEAR_LINE);
			LOGDBG("%s HALT mode changed: %d (ef %d,%d io %d halt %d)\n", machine().describe_context(), state, m_ef[0], m_ef[1], m_ioint, m_halt);
		}
	});

	RAM(config, m_ram).set_default_size("512K").set_extra_options("1M,2M,4M");

	INPUT_MERGER_ANY_HIGH(config, m_halt_merger).output_handler().set_inputline(m_maincpu, t11_device::HLT_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_irq1_merger).output_handler().set(m_pic8259, FUNC(pic8259_device::ir1_w));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set(m_halt_merger, FUNC(input_merger_any_high_device::in_w<1>));
	m_pic8259->out_int_callback().append([this] (int state) {
		if (m_ioint != state) LOGDBG("ioint <- %d\n", state);
		m_ioint = state;
	});

	I8251(config, m_upd8251, 0);
	m_upd8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_upd8251->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_upd8251->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_upd8251->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_upd8251->txrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir3_w));

	// debug console, active when power-on memory test fails
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(host_rs232_defaults));
	rs232.rxd_handler().set(m_upd8251, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_upd8251, FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set(m_upd8251, FUNC(i8251_device::write_cts));

	PIT8253(config, m_pit8253_1);
	m_pit8253_1->set_clk<0>(2'000'000);
	m_pit8253_1->set_clk<1>(1'996'800); /* serial port */
	m_pit8253_1->out_handler<1>().set([this] (int state) { m_upd8251->write_txc(state); m_upd8251->write_rxc(state); });
	m_pit8253_1->set_clk<2>(2'000'000);

	PIT8253(config, m_pit8253_2);
	m_pit8253_2->set_clk<0>(2'000'000);
	m_pit8253_2->set_clk<1>(2'000'000);
	m_pit8253_2->set_clk<2>(2'000'000);

	I8255(config, m_ppi8255);
	// PA -- printer, mouse
	// PB -- printer, io, irqs
	m_ppi8255->in_pb_callback().set([this] () -> uint8_t {
		uint8_t data = (!m_ef[0]) | (!m_ef[1] << 1) | (m_ioint << 2) | (m_halt << 3);
		LOGDBG("%s %d in_pb 0x%x (ef %d,%d io %d halt %d)\n", machine().describe_context(), machine().side_effects_disabled(),
			data, m_ef[0], m_ef[1], m_ioint, m_halt);
		return data;
	});
	// PC -- rtc, irqs, mouse
	m_ppi8255->out_pc_callback().set([this] (uint8_t data) {
		m_halt = !BIT(data, 2);
		m_halt_merger->in_w<2>(m_halt);
		m_maincpu->set_input_line(t11_device::VEC_LINE, !BIT(data, 3));
		LOGDBG("%s out_pc halt %d vec %d\n", machine().describe_context(), m_halt, !BIT(data, 3));
	});

	I8279(config, m_kdc, 2'000'000);
	m_kdc->out_irq_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_kdc->out_sl_callback().set([this](uint8_t data) { m_digit = data; });
	m_kdc->in_rl_callback().set(FUNC(pk11_state::kbd_r));         // kbd RL lines
	m_kdc->in_shift_callback().set_constant(0);                   // Shift key
	m_kdc->in_ctrl_callback().set_constant(0);

	UPD765A(config, m_fdc, 8'000'000, false, false);
	m_fdc->intrq_wr_callback().set(m_irq1_merger, FUNC(input_merger_any_high_device::in_w<0>));
	m_fdc->drq_wr_callback().set(*this, FUNC(pk11_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pk11_floppies, "525qd", pk11_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pk11_floppies, "525qd", pk11_state::floppy_formats);

	WD1010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(m_irq1_merger, FUNC(input_merger_any_high_device::in_w<1>));
	m_hdc->out_bdrq_callback().set(m_hdc, FUNC(wd1010_device::brdy_w));
	m_hdc->out_bdrq_callback().append([this](int state) {
		if (state) { m_buf_ptr = ((BIT(m_buf_ptr, 9, 2) + 1) & 3) << 9; }
	});
	m_hdc->in_data_callback().set(FUNC(pk11_state::buf_r));
	m_hdc->out_data_callback().set(FUNC(pk11_state::buf_w));
	m_hdc->out_bcs_callback().set([this](int state) { if (state) m_buf_dir = false; });

	// Seagate ST-225 formatted with 18 sectors per track
	HARDDISK(config, m_hdd, 0);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_24hrs(true);
	m_rtc->set_binary(true);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(pk11_state::screen_update));
	m_screen->set_raw(XTAL(16'000'000), PK11_TOTAL_HORZ, PK11_HORZ_START,
		PK11_HORZ_START+PK11_DISP_HORZ, PK11_TOTAL_VERT, PK11_VERT_START,
		PK11_VERT_START+PK11_DISP_VERT);
	m_screen->screen_vblank().set(m_pic8259, FUNC(pic8259_device::ir5_w));
}

ROM_START(pk11)
	ROM_REGION16_BE(040000, "maincpu", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("mfm")
	ROM_SYSTEM_BIOS(0, "mfm", "mfm hdd")
	ROMX_LOAD("pk11.rom", 0, 040000, CRC(9c877036) SHA1(aba3afcb9a3ffd4093ef5e937b57db0b2686bb31), ROM_BIOS(0))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME    FLAGS */
COMP( 1991, pk11, 0,      0,      pk11,    pk11,  pk11_state, empty_init, "USSR",  "PK-11/16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
