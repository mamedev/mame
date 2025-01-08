// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Pyldin-601

2009-05-12 Skeleton driver.
2012-04-22 Added sound, fixed keyboard, marked as working [Robbbert]

ToDo?
- PYL601 - command 'MODE80' does nothing

- PYL601a - most software looks odd (unplayable) because of the
            different design of the screen.
- PYL601A - command 'MODE40' doesn't go to 40-columns, instead
            there is a space between each letter.


The BASIC
- to get back to dos, enter SYSTEM
- It has its own internal monitor: MON to enter, Q to exit.



':maincpu' (F013): unmapped program memory write to E628 = 00 & FF
':maincpu' (FCBC): unmapped program memory write to E636 = 00 & FF
':maincpu' (FCBC): unmapped program memory write to E637 = 00 & FF
':maincpu' (FCC2): unmapped program memory write to E634 = 07 & FF
':maincpu' (FCC2): unmapped program memory write to E635 = FF & FF
':maincpu' (FCC7): unmapped program memory write to E637 = 34 & FF
':maincpu' (FCCC): unmapped program memory write to E637 = 3C & FF
':maincpu' (FCCF): unmapped program memory write to E636 = 3C & FF
':maincpu' (FCD3): unmapped program memory read from E634 & FF

****************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "softlist_dev.h"
#include "screen.h"
#include "speaker.h"

#include "formats/pyldin_dsk.h"

#include "utf8.h"


namespace {

class pyl601_state : public driver_device
{
public:
	pyl601_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "upd765")
		, m_floppy(*this, "upd765:%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
	{ }

	void pyl601(machine_config &config);
	void pyl601a(machine_config &config);
	void init_pyl601();

private:
	uint8_t rom_page_r();
	void rom_page_w(uint8_t data);
	void vdisk_page_w(uint8_t data);
	void vdisk_h_w(uint8_t data);
	void vdisk_l_w(uint8_t data);
	void vdisk_data_w(uint8_t data);
	uint8_t vdisk_data_r();
	uint8_t keyboard_r();
	uint8_t keycheck_r();
	void video_mode_w(uint8_t data);
	uint8_t video_mode_r();
	uint8_t timer_r();
	void speaker_w(uint8_t data);
	void led_w(uint8_t data);
	void floppy_w(uint8_t data);
	uint8_t floppy_r();
	MC6845_UPDATE_ROW(pyl601_update_row);
	MC6845_UPDATE_ROW(pyl601a_update_row);
	uint8_t selectedline(uint16_t data);

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	INTERRUPT_GEN_MEMBER(pyl601_interrupt);
	static void floppy_formats(format_registration &fr);
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_rom_page = 0U;
	uint32_t m_vdisk_addr = 0U;
	uint8_t m_key_code = 0U;
	uint8_t m_keyboard_clk = 0U;
	uint8_t m_video_mode = 0U;
	uint8_t m_tick50_mark = 0U;
	uint8_t m_floppy_ctrl = 0U;

	required_device<speaker_sound_device> m_speaker;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<ram_device> m_ram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};



uint8_t pyl601_state::rom_page_r()
{
	return m_rom_page;
}

void pyl601_state::rom_page_w(uint8_t data)
{
	m_rom_page = data;
	if (data & 8)
	{
		int chip = (data >> 4) % 5;
		int page = data & 7;
		membank("bank2")->set_base(memregion("romdisk")->base() + chip*0x10000 + page * 0x2000);
	}
	else
	{
		membank("bank2")->set_base(m_ram->pointer() + 0xc000);
	}
}


void pyl601_state::vdisk_page_w(uint8_t data)
{
	m_vdisk_addr = (m_vdisk_addr & 0x0ffff) | ((data & 0x0f)<<16);
}

void pyl601_state::vdisk_h_w(uint8_t data)
{
	m_vdisk_addr = (m_vdisk_addr & 0xf00ff) | (data<<8);
}

void pyl601_state::vdisk_l_w(uint8_t data)
{
	m_vdisk_addr = (m_vdisk_addr & 0xfff00) | data;
}

void pyl601_state::vdisk_data_w(uint8_t data)
{
	m_ram->pointer()[0x10000 + (m_vdisk_addr & 0x7ffff)] = data;
	m_vdisk_addr++;
	m_vdisk_addr&=0x7ffff;
}

uint8_t pyl601_state::vdisk_data_r()
{
	uint8_t retVal = m_ram->pointer()[0x10000 + (m_vdisk_addr & 0x7ffff)];
	m_vdisk_addr++;
	m_vdisk_addr &= 0x7ffff;
	return retVal;
}

uint8_t pyl601_state::selectedline(uint16_t data)
{
	uint8_t i;
	for(i = 0; i < 16; i++)
		if (BIT(data, i))
			return i;

	return 0;
}

uint8_t pyl601_state::keyboard_r()
{
	uint8_t ret = m_key_code;
	m_key_code = 0xff;
	return ret;
}

uint8_t pyl601_state::keycheck_r()
{
	uint8_t retVal = 0x3f;
	uint8_t *keyboard = memregion("keyboard")->base();
	uint16_t row1 = ioport("ROW1")->read();
	uint16_t row2 = ioport("ROW2")->read();
	uint16_t row3 = ioport("ROW3")->read();
	uint16_t row4 = ioport("ROW4")->read();
	uint16_t row5 = ioport("ROW5")->read();
	uint16_t all = row1 | row2 | row3 | row4 | row5;
	uint16_t addr = ioport("MODIFIERS")->read();
	if (all)
	{
		addr |= selectedline(all) << 2;

		addr |=  ((row5 == 0x00) ? 1 : 0) << 6;
		addr |=  ((row4 == 0x00) ? 1 : 0) << 7;
		addr |=  ((row3 == 0x00) ? 1 : 0) << 8;
		addr |=  ((row2 == 0x00) ? 1 : 0) << 9;
		addr |=  ((row1 == 0x00) ? 1 : 0) << 10;

		m_key_code = keyboard[addr];
		m_keyboard_clk = ~m_keyboard_clk;

		if (m_keyboard_clk)
			retVal |= 0x80;
	}
	return retVal;
}


void pyl601_state::video_mode_w(uint8_t data)
{
	m_video_mode = data;
}

uint8_t pyl601_state::video_mode_r()
{
	return m_video_mode;
}

uint8_t pyl601_state::timer_r()
{
	uint8_t retVal= m_tick50_mark | 0x37;
	m_tick50_mark = 0;
	return retVal;
}

void pyl601_state::speaker_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 3));
}

void pyl601_state::led_w(uint8_t data)
{
//  uint8_t caps_led = BIT(data,4);
}

void pyl601_state::floppy_w(uint8_t data)
{
	// bit 0 is reset (if zero)
	// bit 1 is TC state
	// bit 2 is drive selected
	// bit 3 is motor state

	if (BIT(data,0)==0)
		//reset
		m_fdc->reset();

	floppy_image_device *floppy = m_floppy[BIT(data, 2)]->get_device();
	if(floppy)
		floppy->mon_w(!BIT(data, 3));

	m_fdc->tc_w(BIT(data, 1));

	m_floppy_ctrl = data;
}

uint8_t pyl601_state::floppy_r()
{
	return m_floppy_ctrl;
}

void pyl601_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).bankrw("bank1");
	map(0xc000, 0xdfff).bankrw("bank2");
	map(0xe000, 0xe5ff).bankrw("bank3");
	map(0xe600, 0xe600).mirror(4).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xe601, 0xe601).mirror(4).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xe628, 0xe628).r(FUNC(pyl601_state::keyboard_r));
	map(0xe629, 0xe629).rw(FUNC(pyl601_state::video_mode_r), FUNC(pyl601_state::video_mode_w));
	map(0xe62a, 0xe62a).rw(FUNC(pyl601_state::keycheck_r), FUNC(pyl601_state::led_w));
	map(0xe62b, 0xe62b).rw(FUNC(pyl601_state::timer_r), FUNC(pyl601_state::speaker_w));
	map(0xe62d, 0xe62d).r(FUNC(pyl601_state::video_mode_r));
	map(0xe62e, 0xe62e).rw(FUNC(pyl601_state::keycheck_r), FUNC(pyl601_state::led_w));
	map(0xe680, 0xe680).w(FUNC(pyl601_state::vdisk_page_w));
	map(0xe681, 0xe681).w(FUNC(pyl601_state::vdisk_h_w));
	map(0xe682, 0xe682).w(FUNC(pyl601_state::vdisk_l_w));
	map(0xe683, 0xe683).rw(FUNC(pyl601_state::vdisk_data_r), FUNC(pyl601_state::vdisk_data_w));
	map(0xe6c0, 0xe6c0).rw(FUNC(pyl601_state::floppy_r), FUNC(pyl601_state::floppy_w));
	map(0xe6d0, 0xe6d1).m(m_fdc, FUNC(upd765a_device::map));
	map(0xe6f0, 0xe6f0).rw(FUNC(pyl601_state::rom_page_r), FUNC(pyl601_state::rom_page_w));
	map(0xe700, 0xefff).bankrw("bank4");
	map(0xf000, 0xffff).rom().region("maincpu",0).bankw("bank6");
}

/* Input ports */
/* A small note about natural keyboard mode: Ctrl is mapped to PGUP and the 'Lat/Cyr' key is mapped to PGDOWN */
static INPUT_PORTS_START( pyl601 )
	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Lat/Cyr") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')    // it should be the 4th key at right of 'L'
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("MODIFIERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
INPUT_PORTS_END

void pyl601_state::machine_reset()
{
	uint8_t *ram = m_ram->pointer();
	m_key_code = 0xff;
	membank("bank1")->set_base(ram + 0x0000);
	membank("bank2")->set_base(ram + 0xc000);
	membank("bank3")->set_base(ram + 0xe000);
	membank("bank4")->set_base(ram + 0xe700);
	membank("bank6")->set_base(ram + 0xf000);

	m_maincpu->reset();
}

void pyl601_state::machine_start()
{
	save_item(NAME(m_rom_page));
	save_item(NAME(m_vdisk_addr));
	save_item(NAME(m_key_code));
	save_item(NAME(m_keyboard_clk));
	save_item(NAME(m_video_mode));
	save_item(NAME(m_tick50_mark));
	save_item(NAME(m_floppy_ctrl));
}

MC6845_UPDATE_ROW( pyl601_state::pyl601_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const charrom = memregion("chargen")->base();

	if (BIT(m_video_mode, 5) == 0)
	{
		for (int column = 0; column < x_count; column++)
		{
			uint8_t code = m_ram->pointer()[(((ma + column) & 0x0fff) + 0xf000)];
			code = ((code << 1) | (code >> 7)) & 0xff;
			uint8_t data;
			if (column == cursor_x-2)
				data = 0xff;
			else
				data = charrom[((code << 3) | (ra & 0x07)) & 0x7ff];

			for (int bit = 0; bit < 8; bit++)
			{
				int x = (column * 8) + bit;

				bitmap.pix(y, x) = palette[BIT(data, 7)];

				data <<= 1;
			}
		}
	}
	else
	{
		for (int i = 0; i < x_count; i++)
		{
			uint8_t data = m_ram->pointer()[(((ma + i) << 3) | (ra & 0x07)) & 0xffff];
			for (int bit = 0; bit < 8; bit++)
			{
				bitmap.pix(y, (i * 8) + bit) = palette[BIT(data, 7)];
				data <<= 1;
			}
		}
	}
}

MC6845_UPDATE_ROW( pyl601_state::pyl601a_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const *const charrom = memregion("chargen")->base();

	if (BIT(m_video_mode, 5) == 0)
	{
		for (int column = 0; column < x_count; column++)
		{
			uint8_t const code = m_ram->pointer()[(((ma + column) & 0x0fff) + 0xf000)];
			uint8_t data = charrom[((code << 4) | (ra & 0x07)) & 0xfff];
			if (column == cursor_x)
				data = 0xff;

			for (int bit = 0; bit < 8; bit++)
			{
				int const x = (column * 8) + bit;

				bitmap.pix(y, x) = palette[BIT(data, 7)];

				data <<= 1;
			}
		}
	}
	else
	{
		for (int i = 0; i < x_count; i++)
		{
			uint8_t data = m_ram->pointer()[(((ma + i) << 3) | (ra & 0x07)) & 0xffff];
			for (int bit = 0; bit < 8; bit++)
			{
				bitmap.pix(y, (i * 8) + bit) = palette[BIT(data, 7)];
				data <<= 1;
			}
		}
	}
}



void pyl601_state::init_pyl601()
{
	memset(m_ram->pointer(), 0, 64 * 1024);
}

INTERRUPT_GEN_MEMBER(pyl601_state::pyl601_interrupt)
{
	m_tick50_mark = 0x80;
	device.execute().set_input_line(0, HOLD_LINE);
}

void pyl601_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_PYLDIN_FORMAT);
}

static void pyl601_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

/* F4 Character Displayer */
static const gfx_layout pyl601_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout pyl601a_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_pyl601 )
	GFXDECODE_ENTRY( "chargen", 0x0000, pyl601_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_pyl601a )
	GFXDECODE_ENTRY( "chargen", 0x0000, pyl601a_charlayout, 0, 1 )
GFXDECODE_END

void pyl601_state::pyl601(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pyl601_state::mem_map);
	m_maincpu->set_vblank_int("screen", FUNC(pyl601_state::pyl601_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea(0, 640 - 1, 0, 200 - 1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pyl601);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", 2_MHz_XTAL));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);   /* ? */
	crtc.set_update_row_callback(FUNC(pyl601_state::pyl601_update_row));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	FLOPPY_CONNECTOR(config, "upd765:0", pyl601_floppies, "525hd", pyl601_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", pyl601_floppies, "525hd", pyl601_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("pyl601");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("576K"); // 64 + 512
}

void pyl601_state::pyl601a(machine_config &config)
{
	pyl601(config);

	m_maincpu->set_clock(2_MHz_XTAL);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_pyl601a);

	subdevice<mc6845_device>("crtc")->set_update_row_callback(FUNC(pyl601_state::pyl601a_update_row));
}

/* ROM definition */
ROM_START( pyl601 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bios.rom",   0x0000, 0x1000, CRC(41fe4c4b) SHA1(d8ca92aea0eb283e8d7779cb976bcdfa03e81aea))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "video.rom",  0x0000, 0x0800, CRC(1c23ba43) SHA1(eb1cfc139858abd0aedbbf3d523f8ba55d27a11d))

	ROM_REGION(0x50000, "romdisk", 0 )
	ROM_LOAD( "rom0.rom", 0x00000, 0x10000, CRC(60103920) SHA1(ee5b4ee5b513c4a0204da751e53d63b8c6c0aab9))
	ROM_LOAD( "rom1.rom", 0x10000, 0x10000, CRC(cb4a9b22) SHA1(dd09e4ba35b8d1a6f60e6e262aaf2f156367e385))
	ROM_LOAD( "rom2.rom", 0x20000, 0x08000, CRC(0b7684bf) SHA1(c02ad1f2a6f484cd9d178d8b060c21c0d4e53442))
	ROM_COPY( "romdisk",  0x20000, 0x28000, 0x08000)
	ROM_LOAD( "rom3.rom", 0x30000, 0x08000, CRC(e4a86dfa) SHA1(96e6bb9ffd66f81fca63bf7491fbba81c4ff1fd2))
	ROM_COPY( "romdisk",  0x30000, 0x38000, 0x08000)
	ROM_LOAD( "rom4.rom", 0x40000, 0x08000, CRC(d88ac21d) SHA1(022db11fdcf8db81ce9efd9cd9fa50ebca88e79e))
	ROM_COPY( "romdisk",  0x40000, 0x48000, 0x08000)

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD( "keyboard.rom", 0x0000, 0x0800, CRC(41fbe5ca) SHA1(875adaef53bc37e92ad0b6b6ee3d8fd28344d358))
ROM_END

ROM_START( pyl601a )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bios_a.rom", 0x0000, 0x1000, CRC(e018b11e) SHA1(884d59abd5fa5af1295d1b5a53693facc7945b63))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "video_a.rom", 0x0000,0x1000, CRC(00fa4077) SHA1(d39d15969a08bdb768d08bea4ec9a9cb498232fd))

	ROM_REGION(0x50000, "romdisk", 0 )
	ROM_LOAD( "rom0.rom", 0x00000, 0x10000, CRC(60103920) SHA1(ee5b4ee5b513c4a0204da751e53d63b8c6c0aab9))
	ROM_LOAD( "rom1.rom", 0x10000, 0x10000, CRC(cb4a9b22) SHA1(dd09e4ba35b8d1a6f60e6e262aaf2f156367e385))
	ROM_LOAD( "rom2.rom", 0x20000, 0x08000, CRC(0b7684bf) SHA1(c02ad1f2a6f484cd9d178d8b060c21c0d4e53442))
	ROM_COPY( "romdisk",  0x20000, 0x28000, 0x08000)
	ROM_LOAD( "rom3.rom", 0x30000, 0x08000, CRC(e4a86dfa) SHA1(96e6bb9ffd66f81fca63bf7491fbba81c4ff1fd2))
	ROM_COPY( "romdisk",  0x30000, 0x38000, 0x08000)
	ROM_LOAD( "rom4.rom", 0x40000, 0x08000, CRC(d88ac21d) SHA1(022db11fdcf8db81ce9efd9cd9fa50ebca88e79e))
	ROM_COPY( "romdisk",  0x40000, 0x48000, 0x08000)

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD( "keyboard.rom", 0x0000, 0x0800, CRC(41fbe5ca) SHA1(875adaef53bc37e92ad0b6b6ee3d8fd28344d358))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY             FULLNAME       FLAGS */
COMP( 1989, pyl601,  0,      0,      pyl601,  pyl601, pyl601_state, init_pyl601, "Mikroelektronika", "Pyldin-601",  MACHINE_SUPPORTS_SAVE )
COMP( 1989, pyl601a, pyl601, 0,      pyl601a, pyl601, pyl601_state, init_pyl601, "Mikroelektronika", "Pyldin-601A", MACHINE_SUPPORTS_SAVE )
