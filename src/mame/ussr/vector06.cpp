// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

boot from ROM cart:
 hold F2 then system reset (press F11), then press F12

boot from FDD:
 press F12 after initial boot was load (indicated in screen lower part)
 hold Ctrl ("YC" key) during MicroDOS start to format RAM disk (required by some games)

48k MicroDos one-letter commands:
B   crash?
D   dir
E   erase
K   ?
O   some kind of status display
U   user

TODO:
 - correct CPU speed / latency emulation, each machine cycle takes here 4 clocks,
   i.e. INX B 4+1 will be 2*4=8clocks, SHLD addr is 4+3+3+3+3 so it will be 5*4=20clocks and so on
 - "Card Game" wont work, jump to 0 instead of vblank interrupt RST7, something banking related ?
 - border emulation
 - separate base unexpanded Vector06C configuration
 - slotify AY8910 sound boards ?
 - Rus/Lat key doesn't seem to be right?

****************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "cpu/i8085/i8085.h"

#include "imagedev/cassette.h"
#include "imagedev/floppy.h"

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

#include "sound/ay8910.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/vector06_dsk.h"


namespace {

class vector06_state : public driver_device
{
public:
	vector06_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ay(*this, "aysnd")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_pit(*this, "pit")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_io_keyboard(*this, "LINE.%u", 0U)
		, m_io_reset(*this, "RESET")
	{ }

	void vector06(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(f11_button);
	DECLARE_INPUT_CHANGED_MEMBER(f12_button);

private:
	static void floppy_formats(format_registration &fr);

	uint8_t ppi1_portb_r();
	uint8_t ppi1_portc_r();
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void color_set(uint8_t data);
	uint8_t ppi2_portb_r();
	void ppi2_portb_w(uint8_t data);
	void ppi2_porta_w(uint8_t data);
	void ppi2_portc_w(uint8_t data);
	void disc_w(uint8_t data);
	void status_callback(uint8_t data);
	void ramdisk_w(uint8_t data);
	void speaker_w(int state);
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_mem();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<i8080_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<kr1818vg93_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<ay8910_device> m_ay;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<i8255_device> m_ppi1, m_ppi2;
	required_device<pit8253_device> m_pit;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport_array<9> m_io_keyboard;
	required_ioport m_io_reset;

	uint8_t m_keyboard_mask = 0;
	uint8_t m_color_index = 0;
	uint8_t m_romdisk_msb = 0;
	uint8_t m_romdisk_lsb = 0;
	uint8_t m_vblank_state = 0;
	uint8_t m_rambank = 0;
	uint8_t m_aylatch = 0;
	bool m_video_mode = false;
	bool m_stack_state = false;
	bool m_romen = false;
};


uint8_t vector06_state::ppi1_portb_r()
{
	uint8_t key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_keyboard_mask, i))
			key &= m_io_keyboard[i]->read();

	return key;
}

uint8_t vector06_state::ppi1_portc_r()
{
	uint8_t ret = m_io_keyboard[8]->read();

	if (m_cassette->input() > 0)
		ret |= 0x10;

	return ret;
}

void vector06_state::ppi1_porta_w(uint8_t data)
{
	m_keyboard_mask = data ^ 0xff;
}

void vector06_state::ppi1_portb_w(uint8_t data)
{
	m_color_index = data & 0x0f;
	if (BIT(data, 4) != m_video_mode)
	{
		m_video_mode = BIT(data, 4);
		u16 width = m_video_mode ? 512 : 256;
		rectangle visarea(0, width+64-1, 0, 256+64-1);
		m_screen->configure(width+64, 256+64, visarea, m_screen->frame_period().attoseconds());
	}
}

void vector06_state::color_set(uint8_t data)
{
	uint8_t r = (data & 7) << 5;
	uint8_t g = ((data >> 3) & 7) << 5;
	uint8_t b = ((data >>6) & 3) << 6;
	m_palette->set_pen_color( m_color_index, rgb_t(r,g,b) );
}


uint8_t vector06_state::ppi2_portb_r()
{
	uint16_t addr = ((m_romdisk_msb & 0x7f) << 8) | m_romdisk_lsb;
	if ((m_romdisk_msb & 0x80) && m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(addr);
	else
		return m_ay->data_r();
}

void vector06_state::ppi2_portb_w(uint8_t data)
{
	m_aylatch = data;
}

void vector06_state::ppi2_porta_w(uint8_t data)
{
	m_romdisk_lsb = data;
}

void vector06_state::ppi2_portc_w (uint8_t data)
{
	if (data & 4)
		m_ay->address_data_w((data >> 1) & 1, m_aylatch);
	m_romdisk_msb = data;
}

IRQ_CALLBACK_MEMBER(vector06_state::irq_callback)
{
	// Interrupt is RST 7
	return 0xff;
}

INPUT_CHANGED_MEMBER(vector06_state::f11_button)
{
	if (newval)
	{
		m_romen = true;
		update_mem();
		m_maincpu->reset();
	}
}

INPUT_CHANGED_MEMBER(vector06_state::f12_button)
{
	if (newval)
	{
		m_romen = false;
		update_mem();
		m_maincpu->reset();
	}
}

void vector06_state::disc_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (data & 0x01)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		// something here needs to turn the motor on
		floppy->mon_w(0);
		floppy->ss_w(!BIT(data, 2));
	}
}

void vector06_state::update_mem()
{
	if (BIT(m_rambank, 4) && m_stack_state)
	{
		u8 sentry = ((m_rambank >> 2) & 3) + 1;
		m_bank1->set_entry(sentry);
		m_bank3->set_entry(sentry);
		m_bank2->set_entry(sentry + 1u);
	}
	else
	{
		m_bank1->set_entry(0);
		u8 ventry = 0;
		if (BIT(m_rambank, 5))
			ventry = (m_rambank & 3) + 1;
		m_bank3->set_entry(ventry);
		if (m_romen)
			m_bank2->set_entry(0);
		else
			m_bank2->set_entry(1);
	}
}

void vector06_state::ramdisk_w(uint8_t data)
{
	const uint8_t oldbank = m_rambank;
	m_rambank = data;
	if (oldbank != m_rambank)
		update_mem();
}

void vector06_state::status_callback(uint8_t data)
{
	const bool oldstate = m_stack_state;
	m_stack_state = bool(data & i8080_cpu_device::STATUS_STACK);
	if ((oldstate != m_stack_state) && BIT(m_rambank, 4))
		update_mem();
}

void vector06_state::speaker_w(int state)
{
	m_speaker->level_w(state);
}

void vector06_state::machine_start()
{
	u8 *r = m_ram->pointer();

	m_bank1->configure_entries(0, 5, r, 0x10000);
	m_bank2->configure_entry(0, m_rom);
	m_bank2->configure_entries(1, 5, r, 0x10000);
	m_bank3->configure_entries(0, 5, r + 0xa000, 0x10000);

	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_color_index));
	save_item(NAME(m_romdisk_msb));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_vblank_state));
	save_item(NAME(m_rambank));
	save_item(NAME(m_aylatch));
	save_item(NAME(m_video_mode));
	save_item(NAME(m_stack_state));
	save_item(NAME(m_romen));
}

void vector06_state::machine_reset()
{
	m_stack_state = false;
	m_rambank = 0;
	m_romen = true;

	update_mem();

	m_keyboard_mask = 0;
	m_color_index = 0;
	m_video_mode = 0;
	m_bank1->set_entry(0);
	m_bank2->set_entry(0);
	m_bank3->set_entry(0);
}


uint32_t vector06_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const ram = m_ram->pointer();

	u16 const width = (m_video_mode) ? 512 : 256;
	rectangle screen_area(0,width+64-1,0,256+64-1);
	// fill border color
	bitmap.fill(m_color_index, screen_area);

	// draw image
	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			// port A of 8255 also used as scroll
			int const draw_y = ((255-y-m_keyboard_mask) & 0xff) +32;
			uint8_t const code1 = ram[0x8000 + x*256 + y];
			uint8_t const code2 = ram[0xa000 + x*256 + y];
			uint8_t const code3 = ram[0xc000 + x*256 + y];
			uint8_t const code4 = ram[0xe000 + x*256 + y];
			for (int b = 0; b < 8; b++)
			{
				uint8_t const col = BIT(code1, b) * 8 + BIT(code2, b) * 4 + BIT(code3, b)* 2+ BIT(code4, b);
				if (!m_video_mode)
					bitmap.pix(draw_y, x*8+(7-b)+32) = col;
				else
				{
					bitmap.pix(draw_y, x*16+(7-b)*2+1+32) = BIT(code2, b) * 2;
					bitmap.pix(draw_y, x*16+(7-b)*2+32)   = BIT(code3, b) * 2;
				}
			}
		}
	}
	return 0;
}


/* Address maps */
void vector06_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).bankrw("bank1");
	map(0x0000, 0x7fff).bankr("bank2");
	map(0xa000, 0xdfff).bankrw("bank3");
}

void vector06_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x03).lrw8(NAME([this] (offs_t offset) -> u8 { return m_ppi1->read(offset^3); }), NAME([this] (offs_t offset, u8 data) { m_ppi1->write(offset^3, data); }));
	map(0x04, 0x07).lrw8(NAME([this] (offs_t offset) -> u8 { return m_ppi2->read(offset^3); }), NAME([this] (offs_t offset, u8 data) { m_ppi2->write(offset^3, data); }));
	map(0x08, 0x0b).lrw8(NAME([this] (offs_t offset) -> u8 { return m_pit->read(offset^3); }), NAME([this] (offs_t offset, u8 data) { m_pit->write(offset^3, data); }));
	map(0x0c, 0x0c).w(FUNC(vector06_state::color_set));
	map(0x10, 0x10).w(FUNC(vector06_state::ramdisk_w));
	map(0x14, 0x15).rw(m_ay, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x18, 0x18).rw(m_fdc, FUNC(kr1818vg93_device::data_r), FUNC(kr1818vg93_device::data_w));
	map(0x19, 0x19).rw(m_fdc, FUNC(kr1818vg93_device::sector_r), FUNC(kr1818vg93_device::sector_w));
	map(0x1a, 0x1a).rw(m_fdc, FUNC(kr1818vg93_device::track_r), FUNC(kr1818vg93_device::track_w));
	map(0x1b, 0x1b).rw(m_fdc, FUNC(kr1818vg93_device::status_r), FUNC(kr1818vg93_device::cmd_w));
	map(0x1c, 0x1c).w(FUNC(vector06_state::disc_w));
}

/* Input ports */
static INPUT_PORTS_START( vector06 )
	PORT_START("LINE.0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)         PORT_CHAR(9)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)         PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BkSp") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_START("LINE.1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgUp") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)         PORT_CHAR(27)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_START("LINE.2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR(164)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 \'") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_START("LINE.3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('-') PORT_CHAR('=')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_START("LINE.4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('@') PORT_CHAR('`')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)             PORT_CHAR('A') PORT_CHAR('a')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)             PORT_CHAR('B') PORT_CHAR('b')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)             PORT_CHAR('C') PORT_CHAR('c')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)             PORT_CHAR('D') PORT_CHAR('d')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)             PORT_CHAR('E') PORT_CHAR('e')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)             PORT_CHAR('F') PORT_CHAR('f')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)             PORT_CHAR('G') PORT_CHAR('g')
	PORT_START("LINE.5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)             PORT_CHAR('H') PORT_CHAR('h')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)             PORT_CHAR('I') PORT_CHAR('i')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)             PORT_CHAR('J') PORT_CHAR('j')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)             PORT_CHAR('K') PORT_CHAR('k')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)             PORT_CHAR('L') PORT_CHAR('l')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)             PORT_CHAR('M') PORT_CHAR('m')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)             PORT_CHAR('N') PORT_CHAR('n')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)             PORT_CHAR('O') PORT_CHAR('o')
	PORT_START("LINE.6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)             PORT_CHAR('P') PORT_CHAR('p')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)             PORT_CHAR('Q') PORT_CHAR('q')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)             PORT_CHAR('R') PORT_CHAR('r')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)             PORT_CHAR('S') PORT_CHAR('s')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)             PORT_CHAR('T') PORT_CHAR('t')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)             PORT_CHAR('U') PORT_CHAR('u')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)             PORT_CHAR('V') PORT_CHAR('v')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)             PORT_CHAR('W') PORT_CHAR('w')
	PORT_START("LINE.7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)             PORT_CHAR('X') PORT_CHAR('x')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)             PORT_CHAR('Y') PORT_CHAR('y')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)             PORT_CHAR('Z') PORT_CHAR('z')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('^') PORT_CHAR('~')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(32)
	PORT_START("LINE.8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)    PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl YC") PORT_CODE(KEYCODE_LCONTROL)   //PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT) // This acts as a caps lock in the CP/M screen
	PORT_START("RESET")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11)      PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vector06_state::f11_button), 0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AP2") PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(vector06_state::f12_button), 0)

INPUT_PORTS_END


void vector06_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_VECTOR06_FORMAT);
}

static void vector06_floppies(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}


/* Machine driver */
void vector06_state::vector06(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 3000000); // actual speed is wrong due to unemulated latency
	m_maincpu->set_addrmap(AS_PROGRAM, &vector06_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vector06_state::io_map);
	m_maincpu->out_status_func().set(FUNC(vector06_state::status_callback));
	m_maincpu->set_vblank_int("screen", FUNC(vector06_state::irq0_line_hold));
	m_maincpu->set_irq_acknowledge_callback(FUNC(vector06_state::irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(256+64, 256+64);
	m_screen->set_visarea(0, 256+64-1, 0, 256+64-1);
	m_screen->set_screen_update(FUNC(vector06_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK, 16);

	SPEAKER(config, "mono").front_center();

	/* devices */
	I8255(config, m_ppi1);
	m_ppi1->in_pb_callback().set(FUNC(vector06_state::ppi1_portb_r));
	m_ppi1->in_pc_callback().set(FUNC(vector06_state::ppi1_portc_r));
	m_ppi1->out_pa_callback().set(FUNC(vector06_state::ppi1_porta_w));
	m_ppi1->out_pb_callback().set(FUNC(vector06_state::ppi1_portb_w));

	I8255(config, m_ppi2);
	m_ppi2->in_pb_callback().set(FUNC(vector06_state::ppi2_portb_r));
	m_ppi2->out_pa_callback().set(FUNC(vector06_state::ppi2_porta_w));
	m_ppi2->out_pb_callback().set(FUNC(vector06_state::ppi2_portb_w));
	m_ppi2->out_pc_callback().set(FUNC(vector06_state::ppi2_portc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	KR1818VG93(config, m_fdc, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "fdc:0", vector06_floppies, "qd", vector06_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", vector06_floppies, "qd", vector06_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("vector06_flop");

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vector06_cart", "bin,emr");
	SOFTWARE_LIST(config, "cart_list").set_original("vector06_cart");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("320K").set_default_value(0);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(1500000);
	m_pit->set_clk<1>(1500000);
	m_pit->set_clk<2>(1500000);
	m_pit->out_handler<0>().set(FUNC(vector06_state::speaker_w));
	m_pit->out_handler<1>().set(FUNC(vector06_state::speaker_w));
	m_pit->out_handler<2>().set(FUNC(vector06_state::speaker_w));

	// optional
	AY8910(config, m_ay, 1773400).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/* ROM definition */

ROM_START( vector06 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "unboot32k", "Universal Boot 32K")
	ROMX_LOAD( "unboot32k.rt", 0x0000, 0x8000, CRC(28c9b5cd) SHA1(8cd7fb658896a7066ae93b10eaafa0f12139ad81), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "unboot2k", "Universal Boot 2K")
	ROMX_LOAD( "unboot2k.rt",  0x0000, 0x0800, CRC(4c80dc31) SHA1(7e5e3acfdbea2e52b0d64c5868821deaec383815), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "coman", "Boot Coman")
	ROMX_LOAD( "coman.rt",     0x0000, 0x0800, CRC(f8c4a85a) SHA1(47fa8b02f09a1d06aa63a2b90b2597b1d93d976f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "bootbyte", "Boot Byte")
	ROMX_LOAD( "bootbyte.rt",  0x0000, 0x0800, CRC(3b42fd9d) SHA1(a112f4fe519bc3dbee85b09040d4804a17c9eda2), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "bootos", "Boot OS")
	ROMX_LOAD( "bootos.rt",    0x0000, 0x0200, CRC(46bef038) SHA1(6732f4a360cd38112c53c458842d31f5b035cf59), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "boot512", "Boot 512")
	ROMX_LOAD( "boot512.rt",   0x0000, 0x0200, CRC(a0b1c6b2) SHA1(f6fe15cb0974aed30f9b7aa72133324a66d1ed3f), ROM_BIOS(5))
ROM_END

ROM_START( vec1200 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vec1200.bin", 0x0000, 0x2000, CRC(37349224) SHA1(060fbb2c1a89040c929521cfd58cb6f1431a8b75))

	ROM_REGION( 0x0200, "palette", 0 )
	ROM_LOAD( "palette.bin", 0x0000, 0x0200, CRC(74b7376b) SHA1(fb56b60babd7e6ed68e5f4e791ad2800d7ef6729))
ROM_END

ROM_START( pk6128c )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "6128.bin", 0x0000, 0x4000, CRC(d4f68433) SHA1(ef5ac75f9240ca8996689c23642d4e47e5e774d8))
ROM_END

ROM_START( krista2 ) // appears it wants to load a tape at boot
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "krista2.epr", 0x0000, 0x0200, CRC(df5440b0) SHA1(bcbbb3cc10aeb17c1262b45111d20279266b9ba4))

	ROM_REGION( 0x0200, "palette", 0 )
	ROM_LOAD( "krista2.pal", 0x0000, 0x0200, CRC(b243da33) SHA1(9af7873e6f8bf452c8d831833ffb02dce833c095))
ROM_END

} // anonymous namespace

/* Driver */

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME       FLAGS */
COMP( 1987, vector06, 0,        0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Vector 06c",  MACHINE_SUPPORTS_SAVE )
COMP( 1987, vec1200,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Vector 1200", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, pk6128c,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "PK-6128c",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, krista2,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Krista-2",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
