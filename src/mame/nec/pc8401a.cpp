// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
/*

    NEC PC-8401A-LS "Starlet"
    NEC PC-8500 "Studley"

    TODO:

    - blinking, uses unimplemented cursor in graphics mode for SED1330;
    - RTC TP pulse;
    - sleep mode ignores wake up, does it needs an alarm from RTC?
    - some unclear bits in the banking scheme;
    - mirror e800-ffff to 6800-7fff (why? -AS);
    - soft power on/off;
    - idle sleep timer off by a bunch of seconds ("option" -> "power" to test);
    - complete keyboard mapping;
    \- ALT key is guessed: it seems to match manual (ALT mode for math keys, ALT + SHIFT for symbols),
       but PC-8500 seems to have no support for those so it just sends regular keyboard chars.
       Note: holding 0-0 & 0-1 at boot will draw the aforementioned special chars *only*.
       Is it related to not having CALC app as well? (-> verify when PC-8401* dump surfaces)
    \- LEDs, if any;
    - 8251 USART
    - modem (OKI M6946)
    - PC-8431A FDC is same family as PC-80S31K, basically the 3.5" version of it.
      Likely none of the available BIOSes fits here.

    Notes:
    - Need to format internal RAM before using WS, "format -> F1 -> y"

    - peripherals
        * PC-8431A Dual Floppy Drive
        * PC-8441A CRT / Disk Interface (MC6845, monochrome & color variants)
        * PC-8461A 1200 Baud Modem
        * PC-8407A 128KB RAM Expansion
        * PC-8508A ROM/RAM Cartridge (32K & 128K versions)

    - Use the 600 baud save rate (PIP CAS2:=A:<filename.ext> this is more reliable than the 1200 baud (PIP CAS:=A:<filename.ext> rate.

*/

#include "emu.h"
#include "bus/rs232/rs232.h"

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "video/mc6845.h"
#include "video/sed1330.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "utf8.h"

#define SCREEN_TAG      "screen"

#define IPL_TAG         "ipl"
#define UPD1990A_TAG    "upd1990a"
#define AY8910_TAG      "ay8910"
#define SED1330_TAG     "sed1330"
#define MC6845_TAG      "mc6845"
#define I8251_TAG       "i8251"
#define RS232_TAG       "rs232"

namespace {

class pc8401a_state : public driver_device
{
public:
	pc8401a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, UPD1990A_TAG)
		, m_lcdc(*this, SED1330_TAG)
		, m_screen(*this, SCREEN_TAG)
		, m_cart(*this, "cartslot")
		, m_io_cart(*this, "io_cart")
		, m_nvram(*this, "nvram")
		, m_rom(*this, IPL_TAG)
		, m_io_y(*this, "Y.%u", 0)
		, m_bankdev0(*this, "bankdev0")
		, m_bankdev8(*this, "bankdev8")
		, m_crt_view(*this, "crt_view")
	{ }

	void pc8500(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	required_device<sed1330_device> m_lcdc;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
	required_device<generic_slot_device> m_io_cart;
	required_device<nvram_device> m_nvram;
	required_memory_region m_rom;
	required_ioport_array<10> m_io_y;
	required_device<address_map_bank_device> m_bankdev0;
	required_device<address_map_bank_device> m_bankdev8;
	memory_view m_crt_view;

	std::unique_ptr<uint8_t[]> m_internal_nvram;
	memory_region *m_cart_rom = nullptr;

	void mmr_w(uint8_t data);
	uint8_t mmr_r();
	void port31_w(uint8_t data);
	uint8_t port31_r();
	uint8_t rtc_r();
	void rtc_cmd_w(uint8_t data);
	void rtc_ctrl_w(uint8_t data);
	uint8_t io_rom_data_r();
	void io_rom_addr_w(offs_t offset, uint8_t data);
	uint8_t port70_r();
	uint8_t port71_r();
	void port70_w(uint8_t data);
	void port71_w(uint8_t data);
	void palette_init(palette_device &palette) const;

	void scan_keyboard();
	void bankswitch(uint8_t data);

	// keyboard state
	int m_key_strobe = 0;           // key pressed

	// memory state
	uint8_t m_mmr = 0;                // memory mapping register
	uint8_t m_ext_mmr = 0;
	uint32_t m_io_addr = 0;           // I/O ROM address counter

	uint8_t m_key_latch = 0;
	bool m_key_irq_enable = false;
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	[[maybe_unused]] void pc8401a_lcdc(address_map &map);
	void pc8401a_mem(address_map &map);
	void pc8500_io(address_map &map);
	void pc8500_lcdc(address_map &map);

	void bankdev0_map(address_map &map);
	void bankdev8_map(address_map &map);

	template <unsigned StartBase> uint8_t ram_r(address_space &space, offs_t offset)
	{
		return m_internal_nvram[StartBase + offset];
	}

	template <unsigned StartBase> void ram_w(offs_t offset, uint8_t data)
	{
		m_internal_nvram[StartBase + offset] = data;
	}
};


void pc8401a_state::palette_init(palette_device &palette) const
{
	// TODO: actual values
	palette.set_pen_color(0, rgb_t(160, 168, 160));
	palette.set_pen_color(1, rgb_t(48, 56, 16));
}

void pc8401a_state::pc8401a_lcdc(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).ram();
}

void pc8401a_state::pc8500_lcdc(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).ram();
}

void pc8401a_state::scan_keyboard()
{
	/* scan keyboard */
	// TODO: is this just a generic key pressed shortcut rather than being MCU based?
	for (int row = 0; row < 10; row++)
	{
		uint8_t data = m_io_y[row]->read();

		if (data != 0xff)
		{
			//strobe = 1;
			m_key_latch |= 1;
		}
	}

	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xef); // Z80 - RST 28h
	m_key_strobe = 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(pc8401a_state::keyboard_tick)
{
	if (m_key_irq_enable)
		scan_keyboard();
}

/*
 *
 * bit     description
 * 0       key pressed
 * 1
 * 2
 * 3       <unknown>, disables all keys if on
 * 4       must be 1 or CPU goes to HALT (power status)
 * 5
 * 6
 * 7
 *
 */
uint8_t pc8401a_state::port70_r()
{
	return 0x10 | (m_key_strobe & 1);
}

uint8_t pc8401a_state::port71_r()
{
	return m_key_latch;
}

void pc8401a_state::port70_w(uint8_t data)
{
	m_key_strobe = 0;
}

void pc8401a_state::port71_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	// bit 3 definitely enables/disables irq (without latter it will crash as soon as user presses shift key)
	if (BIT(data, 3) && !BIT(m_key_latch, 3))
		m_key_irq_enable = true;

	if (!BIT(data, 3) && BIT(m_key_latch, 3))
		m_key_irq_enable = false;

	m_key_latch = data;
}

void pc8401a_state::bankswitch(uint8_t data)
{
	// set up A0/A1 memory banking
	m_bankdev0->set_bank(data & 0xf);

	// A2
	m_bankdev8->set_bank((data >> 4) & 0x03);

	// A3
	m_crt_view.select(BIT(data, 6));

	if (BIT(data, 7))
		throw emu_fatalerror("Unknown bank bit 7 set");
}

/*
 *
 * bit     description
 * 0       ROM section bit 0
 * 1       ROM section bit 1
 * 2       mapping for CPU addresses 0000H to 7FFFH bit 0
 * 3       mapping for CPU addresses 0000H to 7FFFH bit 1
 * 4       mapping for CPU addresses 8000H to BFFFH bit 0
 * 5       mapping for CPU addresses 8000H to BFFFH bit 1
 * 6       mapping for CPU addresses C000H to E7FFH
 * 7
 */
void pc8401a_state::mmr_w(uint8_t data)
{
	if (data != m_mmr)
	{
		bankswitch(data);
	}

	m_mmr = data;
}

uint8_t pc8401a_state::mmr_r()
{
	return m_mmr;
}

void pc8401a_state::port31_w(uint8_t data)
{
	m_ext_mmr = data & 7;
	//membank("extram_bank")->set_entry(m_ext_mmr);
	if (data & 0xf8)
		throw emu_fatalerror("Unknown ext bank %02x set", data);
}

uint8_t pc8401a_state::port31_r()
{
	return m_ext_mmr;
}

/*
 * bit     description
 * 0       RTC TP?
 * 1       RTC DATA OUT
 * 2       ?
 * 3
 * 4
 * 5
 * 6
 * 7
 */
uint8_t pc8401a_state::rtc_r()
{
	return (m_rtc->data_out_r() << 1) | (m_rtc->tp_r() << 2);
}

// Virtually same as pc8001_state::port10_w
void pc8401a_state::rtc_cmd_w(uint8_t data)
{
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));

	// TODO: centronics port?
}

/*
 * ---- -x-- RTC CLK
 * ---- --x- RTC STB
 * ---- ---x RTC OE?
 */
void pc8401a_state::rtc_ctrl_w(uint8_t data)
{
	m_rtc->oe_w(BIT(data, 0));
	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));
}

uint8_t pc8401a_state::io_rom_data_r()
{
	//logerror("I/O ROM read from %05x\n", m_io_addr);
	return m_io_cart->read_rom(m_io_addr);
}

void pc8401a_state::io_rom_addr_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: /* A17..A16 */
		m_io_addr = ((data & 0x03) << 16) | (m_io_addr & 0xffff);
		break;

	case 1: /* A15..A8 */
		m_io_addr = (m_io_addr & 0x300ff) | (data << 8);
		break;

	case 2: /* A7..A0 */
		m_io_addr = (m_io_addr & 0x3ff00) | data;
		break;

	case 3:
		/* the same data is written here as to 0xb2, maybe this latches the address value? */
		break;
	}
}

void pc8401a_state::bankdev0_map(address_map &map)
{
	map(0x00000, 0x17fff).rom().region(IPL_TAG, 0);
	map(0x18000, 0x1ffff).r(m_cart, FUNC(generic_slot_device::read_rom));
	map(0x20000, 0x2ffff).rw(FUNC(pc8401a_state::ram_r<0x0000>), FUNC(pc8401a_state::ram_w<0x0000>));
	map(0x30000, 0x3ffff).unmaprw();
}

void pc8401a_state::bankdev8_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x0bfff).rw(FUNC(pc8401a_state::ram_r<0x0000>), FUNC(pc8401a_state::ram_w<0x0000>));
	map(0x0c000, 0x0ffff).unmaprw(); // external RAM cartridge
}

void pc8401a_state::pc8401a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).m(m_bankdev0, FUNC(address_map_bank_device::amap8));
	map(0x8000, 0xbfff).m(m_bankdev8, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xe7ff).view(m_crt_view);
	m_crt_view[0](0xc000, 0xe7ff).rw(FUNC(pc8401a_state::ram_r<0xc000>), FUNC(pc8401a_state::ram_w<0xc000>));
	m_crt_view[1](0xc000, 0xdfff).unmaprw(); // RAM for PC-8441A?
	m_crt_view[1](0xe000, 0xe7ff).unmaprw();
	// TODO: correct? May as well view select with the 0xc*** range ...
	map(0xe800, 0xffff).rw(FUNC(pc8401a_state::ram_r<0xe800>), FUNC(pc8401a_state::ram_w<0xe800>));
}

void pc8401a_state::pc8500_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).portr("Y.0");
	map(0x01, 0x01).portr("Y.1");
	map(0x02, 0x02).portr("Y.2");
	map(0x03, 0x03).portr("Y.3");
	map(0x04, 0x04).portr("Y.4");
	map(0x05, 0x05).portr("Y.5");
	map(0x06, 0x06).portr("Y.6");
	map(0x07, 0x07).portr("Y.7");
	map(0x08, 0x08).portr("Y.8");
	map(0x09, 0x09).portr("Y.9");
	map(0x10, 0x10).w(FUNC(pc8401a_state::rtc_cmd_w));
	map(0x20, 0x21).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x30).rw(FUNC(pc8401a_state::mmr_r), FUNC(pc8401a_state::mmr_w));
	map(0x31, 0x31).rw(FUNC(pc8401a_state::port31_r), FUNC(pc8401a_state::port31_w));
	map(0x40, 0x40).rw(FUNC(pc8401a_state::rtc_r), FUNC(pc8401a_state::rtc_ctrl_w));
//  map(0x41, 0x41)
//  map(0x50, 0x51)
	map(0x60, 0x60).rw(m_lcdc, FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x61, 0x61).rw(m_lcdc, FUNC(sed1330_device::data_r), FUNC(sed1330_device::command_w));
	map(0x70, 0x70).rw(FUNC(pc8401a_state::port70_r), FUNC(pc8401a_state::port70_w));
	map(0x71, 0x71).rw(FUNC(pc8401a_state::port71_r), FUNC(pc8401a_state::port71_w));
//  map(0x80, 0x80) modem status, set to 0xff to boot
//  map(0x8b, 0x8b)
//  map(0x90, 0x93) CRTC system comms?
//  map(0x98, 0x98).w(m_crtc, FUNC(mc6845_device::address_w));
//  map(0x99, 0x99).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x98, 0x99).noprw();
//  map(0xa0, 0xa1)
	map(0xb0, 0xb3).w(FUNC(pc8401a_state::io_rom_addr_w));
	map(0xb3, 0xb3).r(FUNC(pc8401a_state::io_rom_data_r));
//  map(0xc8, 0xc8)
//  map(0xfc, 0xff).rw(I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfc, 0xff).noprw();
}

static INPUT_PORTS_START( pc8401a )
	PORT_START("Y.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") // PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0-0")

	PORT_START("Y.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y.2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_6_PAD)  PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("Y.3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("Y.4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y.5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("Y.6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))

	PORT_START("Y.7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7-0") // ^C

	PORT_START("Y.8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NUM") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ALT?") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("Y.9")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9-7")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9-6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9-5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END

void pc8401a_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* initialize RTC */
	m_rtc->cs_w(1);

	// PC-8401A & PC-8500 ships with 64K of internal RAM
	m_internal_nvram = std::make_unique<uint8_t[]>(0x10000);
	m_nvram->set_base(m_internal_nvram.get(), 0x10000);

	/* register for state saving */
	save_item(NAME(m_mmr));
	save_item(NAME(m_io_addr));
}

void pc8401a_state::machine_reset()
{
	m_key_irq_enable = false;
	bankswitch(0);
}

void pc8401a_state::pc8500(machine_config &config)
{
	Z80(config, m_maincpu, 7.987_MHz_XTAL / 2); // NEC uPD70008C
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8401a_state::pc8401a_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8401a_state::pc8500_io);

	// unknown frequency, roughly fits idle sleep mode
	TIMER(config, "keyboard").configure_periodic(FUNC(pc8401a_state::keyboard_tick), attotime::from_hz(60));

	UPD1990A(config, m_rtc);

	i8251_device &uart(I8251(config, I8251_TAG, 0));
	uart.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));

	PALETTE(config, "palette", FUNC(pc8401a_state::palette_init), 2);

	// TODO: pc8401a uses 128 display lines instead of 200
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(44);
	m_screen->set_screen_update(SED1330_TAG, FUNC(sed1330_device::screen_update));
	m_screen->set_size(480, 208);
	m_screen->set_visarea(0, 480-1, 0, 200-1);
	m_screen->set_palette("palette");

	SED1330(config, m_lcdc, 7.987_MHz_XTAL);
	m_lcdc->set_screen(SCREEN_TAG);
	m_lcdc->set_addrmap(0, &pc8401a_state::pc8500_lcdc);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);

	ADDRESS_MAP_BANK(config, m_bankdev0).set_map(&pc8401a_state::bankdev0_map).set_options(ENDIANNESS_LITTLE, 8, 15 + 8, 0x8000);
	ADDRESS_MAP_BANK(config, m_bankdev8).set_map(&pc8401a_state::bankdev8_map).set_options(ENDIANNESS_LITTLE, 8, 16, 0x4000);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, nullptr, "bin,rom");

	/* I/O ROM cartridge */
	GENERIC_CARTSLOT(config, m_io_cart, generic_linear_slot, nullptr, "bin,rom");

	// TODO: wrong, should touch external cart only and have 32K & 128K options, plus be a slot NVRAM anyway.
	// RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("96K,192K");
}

ROM_START( pc8500 )
	ROM_REGION( 0x20000, IPL_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "pc8500.bin", 0x0000, 0x10000, CRC(c2749ef0) SHA1(f766afce9fda9ec84ed5b39ebec334806798afb3) )

	//ROM_REGION( 0x1000, "chargen", 0 )
	//ROM_LOAD( "pc8441a.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

} // anonymous namespace

/* System Drivers */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME */
//COMP( 1984, pc8401a,  0,       0,      pc8401a, pc8401a, pc8401a_state, empty_init, "NEC",   "PC-8401A-LS", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
//COMP( 1984, pc8401bd, pc8401a, 0,      pc8401a, pc8401a, pc8401a_state, empty_init, "NEC",   "PC-8401BD", MACHINE_NOT_WORKING)
COMP( 1985, pc8500,   0,       0,      pc8500,  pc8401a, pc8401a_state,  empty_init, "NEC",   "PC-8500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
