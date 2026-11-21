// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Itautec I-7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

    The I-7000, released in 1982, was the first computer manufactured by
    Itautec. It was an 8-bit CP/M machine; later hardware revisions turned
    it into an IBM PC-XT clone, the "I-7000 PC-XT".
    (overview: http://pt.wikipedia.org/wiki/Itautec_I-7000)

    * Operating System: SIM/M / BASIC
    * CPU: National NSC800 D-4 at 4.00 MHz
    * Memory: 64KB to 128KB
    * keyboard: 80 keys (with a reduced numerical keypad and function keys)
    * display:
     - 40 X 25 text
     - 80 X 25 text
     - 160 X 100 (8 colors)
     - 640 X 200 (monochrome, with an expansion board)
     - 320 X 200 (16 colors, with an expansion board)
    * Expansion slots:
     - 1 frontal cart slot
     - 4 internal expansion slots
    * Ports:
     - 1 composite video output for a color monitor
     - 2 cassette interfaces
     - 1 RS-232C serial port
     - 1 parallel interface
    * Storage:
     - Cassette recorder
     - Up to 4 external floppy drives: 8" (FD/DD, 1.1MB) or 5" 1/4
     - Up to 1 external 10 MB hard-drive

    At the boot menu, press E to boot SIM/M from the first 5.25" drive,
    or Z to run the cartridge.

    TODO:
    - the port 0x0c/0x10 paging is only partly understood
    - PIT counter 0 drives RSTB, which needs the NSC800 interrupt control
      register (port 0xbb)
    - hard disk interface, printer, cassette, joystick ADC, Telex board
    - verify the clocks

****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/nsc800.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/imd_dsk.h"
#include "formats/td0_dsk.h"

#include <algorithm>


namespace {

class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic")
		, m_card(*this, "cardslot")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_usart(*this, "usart")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_page_view(*this, "page_view")
		, m_drive_view(*this, "drive_view")
		, m_ram(*this, "ram")
		, m_vram(*this, "vram", 0x1000, ENDIANNESS_LITTLE)
		, m_chargen(*this, "chargen")
		, m_kbd(*this, "X%u", 0U)
		, m_fn(*this, "FN")
		, m_modkeys(*this, "MOD")
		, m_lock_led(*this, "lock_led")
	{ }

	void i7000(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	static constexpr uint32_t DOT_CLOCK = 14'318'181;

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<generic_slot_device> m_card;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<i8251_device> m_usart;
	required_device<wd2793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	memory_view m_page_view;
	memory_view m_drive_view;
	required_shared_ptr<uint8_t> m_ram;
	memory_share_creator<uint8_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;
	required_ioport_array<8> m_kbd;
	required_ioport m_fn;
	required_ioport m_modkeys;
	output_finder<> m_lock_led;

	uint8_t m_row = 0;
	uint8_t m_page_0c = 0;
	uint8_t m_page_10 = 0;
	bool m_80col = false;
	uint8_t m_fdc_ctrl = 0;

	void i7000_palette(palette_device &palette) const ATTR_COLD;
	MC6845_UPDATE_ROW(crtc_update_row);
	IRQ_CALLBACK_MEMBER(inta_cb);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(card_load);

	uint8_t kbd_r();
	void scanlines_w(uint8_t data);
	int shift_r();
	int ctrl_r();

	void col80_w(uint8_t data);
	void page_0c_w(uint8_t data);
	void page_10_w(uint8_t data);
	void update_paging();

	static void floppy_formats(format_registration &fr);
	void fdc_ctrl_w(uint8_t data);
	uint8_t fdc_status_r();

	void i7000_io(address_map &map) ATTR_COLD;
	void i7000_mem(address_map &map) ATTR_COLD;
};

void i7000_state::scanlines_w(uint8_t data)
{
	m_row = data;
}

uint8_t i7000_state::kbd_r()
{
	// the function keys close the digit keys' crosspoints
	uint8_t const row = m_row & 7;
	return m_kbd[row]->read() & (BIT(m_fn->read(), row * 2, 2) | 0xfc);
}

int i7000_state::shift_r()
{
	// the function keys also assert SHIFT and CNTL
	return BIT(m_modkeys->read(), 0) & (m_fn->read() == 0xffff);
}

int i7000_state::ctrl_r()
{
	return BIT(m_modkeys->read(), 1) & (m_fn->read() == 0xffff);
}

// characters and unknown legends follow SIM/M's keyboard tables
INPUT_PORTS_START( i7000 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("!  4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'´') PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"º  8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(U'º')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Down  2") PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('/')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('$') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'ç') PORT_CHAR(U'Ç')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(U'§')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('%') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~  ^") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]  9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('@')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('|') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[  7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Up  1") PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#  5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{  3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('{')

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENQ") PORT_CODE(KEYCODE_TILDE) // legend unknown
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("}  0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('}')

	PORT_START("FN") // bits 2n and 2n+1 share X<n>.0 and X<n>.1
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0xaa00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	PORT_START("DSW") // DP01, read through PPI port B
	PORT_DIPNAME( 0x80, 0x00, "Diagnostic Mode" ) PORT_DIPLOCATION("DP01:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DP01:2" )
	PORT_DIPNAME( 0x20, 0x20, "Columns" ) PORT_DIPLOCATION("DP01:3")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x20, "80" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DP01:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DP01:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DP01:6" )
	PORT_DIPNAME( 0x02, 0x02, "Characters per Line" ) PORT_DIPLOCATION("DP01:7") // CRTC horizontal total with 80 columns selected
	PORT_DIPSETTING(    0x00, "52/103" )
	PORT_DIPSETTING(    0x02, "56/112" )
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DP01:8" )
INPUT_PORTS_END


void i7000_state::machine_start()
{
	// one page view entry per combination of the port 0x0c bits (see page_0c_w)
	uint32_t const cart_size = m_card->exists() ? m_card->get_rom_size() : 0;
	for (int i = 0; i < 16; i++)
	{
		memory_view::memory_view_entry &entry = m_page_view[i];
		if (BIT(i, 0))
			entry.install_ram(0x6000, 0x6fff, &m_vram[0]);
		if (BIT(i, 1))
			entry.install_ram(0x0000, 0x0fff, &m_ram[0]);
		else
			entry.install_ram(0x2000, 0x2fff, &m_vram[0]);
		if (BIT(i, 2) && cart_size)
			entry.install_rom(0x4000, 0x4000 + std::min<uint32_t>(cart_size, 0x4000) - 1, m_card->get_rom_base());
		if (BIT(i, 3) && cart_size)
			entry.install_rom(0x0000, cart_size - 1, m_card->get_rom_base());
	}

	m_drive_view[2].install_ram(0xa000, 0xafff, &m_vram[0]);

	m_usart->write_cts(0); // no hardware flow control

	save_item(NAME(m_row));
	save_item(NAME(m_page_0c));
	save_item(NAME(m_page_10));
	save_item(NAME(m_80col));
	save_item(NAME(m_fdc_ctrl));
}

void i7000_state::machine_reset()
{
	m_page_0c = 0;
	m_page_10 = 0;
	update_paging();

	m_80col = false;
	m_crtc->set_unscaled_clock(DOT_CLOCK / 16);

	fdc_ctrl_w(0);
}

void i7000_state::device_post_load()
{
	fdc_ctrl_w(m_fdc_ctrl);
}

DEVICE_IMAGE_LOAD_MEMBER(i7000_state::card_load)
{
	uint32_t const size = m_card->common_get_size("rom");
	if (!size || (size > 0x8000) || (size & 0x1fff))
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 8K, 16K, 24K or 32K)");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");
	return std::make_pair(std::error_condition(), std::string());
}

// a toggle flip-flop, cleared at reset: 40 or 80 columns
void i7000_state::col80_w(uint8_t data)
{
	m_80col = !m_80col;
	m_crtc->set_unscaled_clock(DOT_CLOCK / (m_80col ? 8 : 16));
}

/* Port 0x0c: bit 1 maps the cartridge at 0x0000, bit 5 maps it at 0x4000
   (the boot ROM looks for its "SIM/M-" header at 0x4040), bit 3 maps RAM
   at 0x0000-0x0fff instead of the video RAM at 0x2000, and bit 7 clear
   maps the video RAM at 0x6000. */
void i7000_state::page_0c_w(uint8_t data)
{
	m_page_0c = data;
	update_paging();
}

/* Port 0x10 selects what appears at 0x8000-0xafff: the video RAM at
   0xa000 (0xb0), the drive board ROM at 0x8000 (0xb1, also used by the
   boot ROM's peripheral scan of 0xb1-0xb7), or that ROM plus the board's
   RAM at 0xa000 (0xb4, used by the cartridges to call the ROM's API). */
void i7000_state::page_10_w(uint8_t data)
{
	m_page_10 = data;
	update_paging();
}

void i7000_state::update_paging()
{
	m_page_view.select(bitswap<4>(m_page_0c ^ 0x80, 1, 5, 3, 7));

	if (m_page_10 == 0xb0)
		m_drive_view.select(2);
	else if (m_page_10 == 0xb1)
		m_drive_view.select(0);
	else if (m_page_10 == 0xb4)
		m_drive_view.select(1);
	else
		m_drive_view.disable();
}

/* Drive board ("Placa Drive"), ports 0xe0-0xe7:
     0xe0-0xe3  WD2793
     0xe4       drive select latch
     0xe5       hard disk data
     0xe6       status
     0xe7       hard disk select */

void i7000_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_IMD_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
}

void i7000_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void i7000_state::fdc_ctrl_w(uint8_t data)
{
	// TODO: the other bits
	m_fdc_ctrl = data;
	floppy_image_device *const floppy = m_floppy[data & 3]->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->mon_w(0);

	// the drive ROM checks Not Ready; an empty drive must still report it
	m_fdc->set_force_ready(floppy && floppy->exists());
}

uint8_t i7000_state::fdc_status_r()
{
	// bit 7: no DRQ, bit 6: INTRQ, bits 0-2: hard disk handshake
	return (m_fdc->drq_r() ? 0x00 : 0x80) | (m_fdc->intrq_r() ? 0x40 : 0x00);
}

IRQ_CALLBACK_MEMBER(i7000_state::inta_cb)
{
	// the PIC is in 8080 mode and answers with a CALL
	uint32_t vector = m_pic->acknowledge() << 16;
	vector |= m_pic->acknowledge();
	vector |= m_pic->acknowledge() << 8;
	return vector;
}

void i7000_state::i7000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x33, 0x44, 0x5d));
	palette.set_pen_color(1, rgb_t(0x67, 0xcc, 0xca));
}

MC6845_UPDATE_ROW(i7000_state::crtc_update_row)
{
	uint32_t *p = &bitmap.pix(y);
	for (int col = 0; col < x_count; col++)
	{
		uint8_t const ch = m_vram[(ma + col) & 0x07ff];
		uint8_t bits = (ra > 7) ? 0 : m_chargen[((ch << 3) | (ra & 0x07)) & 0x07ff];
		if (col == cursor_x)
			bits ^= 0xff;
		for (int cx = 0; cx < 8; cx++)
			*p++ = m_palette->pen(BIT(bits, 7 - cx));
	}
}

void i7000_state::i7000_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_ram);
	map(0x0000, 0x0fff).rom().region("boot", 0);
	map(0x0000, 0x7fff).view(m_page_view);
	map(0x8000, 0xafff).view(m_drive_view);
	m_drive_view[0](0x8000, 0x8fff).rom().region("drive", 0);
	m_drive_view[1](0x8000, 0x8fff).rom().region("drive", 0);
	m_drive_view[1](0xa000, 0xa7ff).mirror(0x0800).ram();
}

void i7000_state::i7000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x03).w(FUNC(i7000_state::col80_w));
	map(0x08, 0x08).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x09, 0x09).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0c, 0x0c).w(FUNC(i7000_state::page_0c_w));
	map(0x10, 0x10).w(FUNC(i7000_state::page_10_w));
	map(0x14, 0x15).mirror(0x02).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x18, 0x1b).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x1c, 0x1f).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x21).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x24, 0x24).mirror(0x02).rw(m_usart, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x25, 0x25).mirror(0x02).rw(m_usart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xe0, 0xe3).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0xe4, 0xe4).w(FUNC(i7000_state::fdc_ctrl_w));
	map(0xe6, 0xe6).r(FUNC(i7000_state::fdc_status_r));
}

// SET VT52 starts up at 9600 baud, 7 data bits, odd parity, 2 stop bits
DEVICE_INPUT_DEFAULTS_START( vt52_serial_defaults )
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_ODD)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END


void i7000_state::i7000(machine_config &config)
{
	/* basic machine hardware */
	NSC800(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &i7000_state::i7000_mem);
	m_maincpu->set_addrmap(AS_IO, &i7000_state::i7000_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(i7000_state::inta_cb));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_size(640, 200); /* the CRTC reconfigures it */
	screen.set_visarea(0, 640-1, 0, 200-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(i7000_state::i7000_palette), 2);

	MC6845(config, m_crtc, DOT_CLOCK / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(i7000_state::crtc_update_row));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	pit8253_device &pit8253(PIT8253(config, "pit8253"));
	pit8253.set_clk<0>(2'000'000);
	pit8253.set_clk<1>(2'000'000);
	pit8253.out_handler<1>().set(m_usart, FUNC(i8251_device::write_txc));
	pit8253.out_handler<1>().append(m_usart, FUNC(i8251_device::write_rxc));
	pit8253.set_clk<2>(2'000'000);
	pit8253.out_handler<2>().set("speaker", FUNC(speaker_sound_device::level_w));

	I8255(config, "ppi").in_pb_callback().set_ioport("DSW");

	I8251(config, m_usart, 2'000'000); // TMP8251AP
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart->rxrdy_handler().set_inputline(m_maincpu, NSC800_RSTA); // assumed

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(vt52_serial_defaults));

	i8279_device &kbdc(I8279(config, "i8279", 2'000'000));
	kbdc.out_irq_callback().set_inputline(m_maincpu, NSC800_RSTC);
	kbdc.out_sl_callback().set(FUNC(i7000_state::scanlines_w));
	kbdc.in_rl_callback().set(FUNC(i7000_state::kbd_r));
	kbdc.in_shift_callback().set(FUNC(i7000_state::shift_r));
	kbdc.in_ctrl_callback().set(FUNC(i7000_state::ctrl_r));
	// the software clears the display RAM to 0x00 to light LOCK
	kbdc.out_disp_callback().set([this] (uint8_t data) { m_lock_led = BIT(~data, 0); });

	WD2793(config, m_fdc, 1'000'000);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	FLOPPY_CONNECTOR(config, "fdc:0", i7000_floppies, "525dd", i7000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", i7000_floppies, "525dd", i7000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", i7000_floppies, "525dd", i7000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", i7000_floppies, "525dd", i7000_state::floppy_formats);

	/* Cartridge slot */
	GENERIC_CARTSLOT(config, "cardslot", generic_romram_plain_slot, "i7000_card", "rom").set_device_load(FUNC(i7000_state::card_load));

	/* Software lists */
	SOFTWARE_LIST(config, "card_list").set_original("i7000_card");
	SOFTWARE_LIST(config, "flop_list").set_original("i7000_flop");
}

ROM_START( i7000 )
	ROM_REGION( 0x1000, "boot", 0 )
	ROM_LOAD( "i7000_boot_v1_4r02_15_10_85_d52d.rom",  0x0000, 0x1000, CRC(622412e5) SHA1(bf187a095600fd46a739c35132a85b5f39b2f867) )

	ROM_REGION( 0x0800, "chargen", 0 )
	/* Hand-repaired: 0x06a2 and 0x06a3 read 0xf7 and were set to 0xfe
	   (character 0xd4). Characters 0x05-0x08, 'Y' and 'x' may be bad too. */
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, BAD_DUMP CRC(fb7383e9) SHA1(71a6561bb9ff3cbf74711fa7ab445f9b43f15626) )

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME  FLAGS
COMP( 1982, i7000, 0,      0,      i7000,   i7000, i7000_state, empty_init, "Itautec", "I-7000", MACHINE_NOT_WORKING)
