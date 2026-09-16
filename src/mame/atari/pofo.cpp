// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Atari Portfolio

    https://www.pofowiki.de/doku.php
    https://www.best-electronics-ca.com/portfoli.htm

    Command line for dual RAM expansion with A: File Manager ROM card and B: RAM card
    ./mame pofo -exp ram -exp:ram:exp ram2 -memc fileman -exp:ram:ccmb ram

*/

#include "emu.h"
#include "bus/pofo/ccm.h"
#include "bus/pofo/exp.h"
#include "cpu/i86/i86.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/pcd3311.h"
#include "video/82c425.h"
#include "video/hd61830.h"
#include "emupal.h"
#include "pofo_asic.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"

namespace {

#define M80C88A_TAG     "u1"
#define HD61830_TAG     "hd61830"
#define PCD3311T_TAG    "pcd3311t"
#define SCREEN_TAG      "screen"

class portfolio_base_state : public driver_device
{
public:
	portfolio_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M80C88A_TAG),
		m_irqs(*this, "irqs"),
		m_asic(*this, "asic"),
		m_ram(*this, RAM_TAG),
		m_dtmf(*this, PCD3311T_TAG),
		m_palette(*this, "palette"),
		m_ccma(*this, PORTFOLIO_MEMORY_CARD_SLOT_A_TAG),
		m_exp(*this, "exp"),
		m_nvram(*this, "nvram")
	{ }

	void portfolio_base(machine_config &config);
	void portfolio(machine_config &config);

	ioport_value wake_r() { return m_wake; }

protected:
	virtual void machine_start() override ATTR_COLD;

	IRQ_CALLBACK_MEMBER(iack);

	void wake_w(int state) { m_wake = state; }
	void dtmf_w(offs_t offset, uint8_t data);
	void pint_w(int state) { m_pint = state; }

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_any_high_device> m_irqs;
	required_device<portfolio_asic_device> m_asic;
	required_device<ram_device> m_ram;
	required_device<pcd3311_device> m_dtmf;
	required_device<palette_device> m_palette;
	required_device<portfolio_memory_card_slot_device> m_ccma;
	required_device<portfolio_expansion_slot_device> m_exp;
	required_device<nvram_device> m_nvram;

	bool m_pint;
	bool m_wake;
};

class portfolio_state : public portfolio_base_state
{
public:
	portfolio_state(const machine_config &mconfig, device_type type, const char *tag) :
		portfolio_base_state(mconfig, type, tag),
		m_lcdc(*this, HD61830_TAG),
		m_char_rom(*this, HD61830_TAG)
	{ }

	void portfolio(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void portfolio_mem(address_map &map) ATTR_COLD;
	void portfolio_io(address_map &map) ATTR_COLD;

	void portfolio_lcdc(address_map &map) ATTR_COLD;
	void portfolio_palette(palette_device &palette) const;

	uint8_t hd61830_rd_r(offs_t offset) { return m_char_rom->base()[((offset & 0xff) << 4) | ((offset >> 12) & 0x0f)]; }
	void contrast_w(uint8_t data);

	required_device<hd61830_device> m_lcdc;
	required_memory_region m_char_rom;
};

class portfolio2_state : public portfolio_base_state
{
public:
	portfolio2_state(const machine_config &mconfig, device_type type, const char *tag) :
		portfolio_base_state(mconfig, type, tag),
		m_82c425(*this, "82c425"),
		m_char_rom(*this, "chargen")
	{ }

	void portfolio2(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	void portfolio2_mem(address_map &map) ATTR_COLD;
	void portfolio2_io(address_map &map) ATTR_COLD;
	void dispfont_map(address_map &map) ATTR_COLD;

	void portfolio2_palette(palette_device &palette) const;
	void contrast_w(uint8_t data);

	required_device<f82c425_device> m_82c425;
	required_memory_region m_char_rom;
};

void portfolio_state::portfolio_mem(address_map &map)
{
	map(0x00000, 0xfffff).m(m_asic, FUNC(portfolio_asic_device::mem_map));
}

void portfolio_state::portfolio_io(address_map &map)
{
	map(0x0000, 0xffff).m(m_asic, FUNC(portfolio_asic_device::io_map));
}

void portfolio2_state::portfolio2_mem(address_map &map)
{
	map(0x00000, 0xfffff).m(m_asic, FUNC(portfolio_asic_device::mem_map));
	map(0xb8000, 0xbbfff).rw(m_82c425, FUNC(f82c425_device::mem_r), FUNC(f82c425_device::mem_w));
}

void portfolio2_state::portfolio2_io(address_map &map)
{
	map(0x0000, 0xffff).m(m_asic, FUNC(portfolio_asic_device::io_map));
	map(0x03d0, 0x03df).m(m_82c425, FUNC(f82c425_device::io_map));
}

void portfolio_state::portfolio_lcdc(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).ram();
}

void portfolio2_state::dispfont_map(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x5fff).rom().region("chargen", 0).nopw();
}

static INPUT_PORTS_START( portfolio )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Atari") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(portfolio_base_state::wake_r))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('"') PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Fn") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x00, "Low Battery" )
	PORT_CONFNAME( 0x02, 0x00, "Boot" )
	PORT_CONFSETTING( 0x02, "Cold" )
	PORT_CONFSETTING( 0x00, "Warm" )
INPUT_PORTS_END

IRQ_CALLBACK_MEMBER(portfolio_base_state::iack)
{
	if (m_pint == ASSERT_LINE)
	{
		return m_asic->mack_r();
	}
	else
	{
		return m_exp->eack_r();
	}
}

void portfolio_state::portfolio_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(142, 193, 172));
	palette.set_pen_color(1, rgb_t(67, 71, 151));
}

void portfolio2_state::portfolio2_palette(palette_device &palette) const
{
	// estimated LCD palette
	palette.set_pen_color(0, rgb_t(0xa0, 0xd0, 0x08));
	palette.set_pen_color(1, rgb_t(0x90, 0xbb, 0x07));
	palette.set_pen_color(2, rgb_t(0x80, 0xa6, 0x06));
	palette.set_pen_color(3, rgb_t(0x70, 0x92, 0x06));
	palette.set_pen_color(4, rgb_t(0x60, 0x7d, 0x05));
	palette.set_pen_color(5, rgb_t(0x50, 0x68, 0x04));
	palette.set_pen_color(6, rgb_t(0x40, 0x53, 0x03));
	palette.set_pen_color(7, rgb_t(0x30, 0x3e, 0x02));
}

void portfolio_state::contrast_w(uint8_t data)
{
	double const level = data / double(0xfc);
	rgb_t const off(142, 193, 172);
	rgb_t const on(67, 71, 151);

	m_palette->set_pen_color(1, rgb_t(
			off.r() + int((on.r() - off.r()) * level),
			off.g() + int((on.g() - off.g()) * level),
			off.b() + int((on.b() - off.b()) * level)));
}

void portfolio2_state::contrast_w(uint8_t data)
{
	static rgb_t const on[8] =
	{
		rgb_t(0xa0, 0xd0, 0x08), rgb_t(0x90, 0xbb, 0x07), rgb_t(0x80, 0xa6, 0x06), rgb_t(0x70, 0x92, 0x06),
		rgb_t(0x60, 0x7d, 0x05), rgb_t(0x50, 0x68, 0x04), rgb_t(0x40, 0x53, 0x03), rgb_t(0x30, 0x3e, 0x02)
	};

	double const level = data / double(0xfc);
	rgb_t const off = on[0];

	for (int pen = 0; pen < 8; pen++)
	{
		m_palette->set_pen_color(pen, rgb_t(
				off.r() + int((on[pen].r() - off.r()) * level),
				off.g() + int((on[pen].g() - off.g()) * level),
				off.b() + int((on[pen].b() - off.b()) * level)));
	}
}

void portfolio_base_state::dtmf_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       PCD3311T D0
	    1       PCD3311T D1
	    2       PCD3311T D2
	    3       PCD3311T D3
	    4       PCD3311T D4
	    5       PCD3311T D5
	    6       PCD3311T STROBE
	    7       PCD3311T VDD,MODE,A0

	*/

	m_dtmf->mode_w(!BIT(data, 7));
	m_dtmf->a0_w(!BIT(data, 7));
	m_dtmf->write(data & 0x3f);
	m_dtmf->strobe_w(BIT(data, 6));
}

void portfolio_base_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	const offs_t vram_offset = m_ram->size() - 0x1000;
	m_maincpu->space(AS_PROGRAM).install_ram(0x00000, vram_offset - 1, m_ram->pointer());
}

void portfolio_base_state::portfolio_base(machine_config &config)
{
	// basic machine hardware
	I8088(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_irq_acknowledge_callback(FUNC(portfolio_base_state::iack));

	INPUT_MERGER_ANY_HIGH(config, m_irqs);
	m_irqs->output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_irqs->output_handler().append(m_exp, FUNC(portfolio_expansion_slot_device::iint_w));

	PORTFOLIO_ASIC(config, m_asic);
	m_asic->set_rom_tag(m_maincpu->tag());
	m_asic->pint_wr_cb().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_asic->pint_wr_cb().append(FUNC(portfolio_base_state::pint_w));
	m_asic->nmio_wr_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_asic->nmio_wr_cb().append(m_exp, FUNC(portfolio_expansion_slot_device::nmio_w));
	m_asic->nmd1_rd_cb().set(m_exp, FUNC(portfolio_expansion_slot_device::nmd1_r));
	m_asic->pdet_rd_cb().set(m_exp, FUNC(portfolio_expansion_slot_device::pdet_r));
	m_asic->ncc1_wr_cb().set(m_exp, FUNC(portfolio_expansion_slot_device::ncc1_w));
	m_asic->ncc2_wr_cb().set(m_ccma, FUNC(portfolio_memory_card_slot_device::ncc2_w));
	m_asic->dtmf_wr_cb().set(FUNC(portfolio_state::dtmf_w));
	m_asic->kop0_rd_cb().set_ioport("Y0");
	m_asic->kop1_rd_cb().set_ioport("Y1");
	m_asic->kop2_rd_cb().set_ioport("Y2");
	m_asic->kop3_rd_cb().set_ioport("Y3");
	m_asic->kop4_rd_cb().set_ioport("Y4");
	m_asic->kop5_rd_cb().set_ioport("Y5");
	m_asic->kop6_rd_cb().set_ioport("Y6");
	m_asic->kop7_rd_cb().set_ioport("Y7");
	m_asic->battery_rd_cb().set_ioport("BATTERY");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	PCD3311(config, m_dtmf, XTAL(3'578'640)).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	PORTFOLIO_MEMORY_CARD_SLOT(config, m_ccma, portfolio_memory_cards, nullptr);
	m_ccma->set_memspace(m_maincpu, AS_PROGRAM);

	PORTFOLIO_EXPANSION_SLOT(config, m_exp, XTAL(4'915'200), portfolio_expansion_cards, nullptr);
	m_exp->set_memspace(m_maincpu, AS_PROGRAM);
	m_exp->set_iospace(m_maincpu, AS_IO);
	m_exp->eint_wr_cb().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_exp->wake_wr_cb().set(FUNC(portfolio_state::wake_w));

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("pofo");

	// internal ram
	RAM(config, m_ram);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void portfolio_state::portfolio(machine_config &config)
{
	portfolio_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &portfolio_state::portfolio_mem);
	m_maincpu->set_addrmap(AS_IO, &portfolio_state::portfolio_io);

	m_asic->lcdc_rd_cb().set(m_lcdc, FUNC(hd61830_device::read));
	m_asic->lcdc_wr_cb().set(m_lcdc, FUNC(hd61830_device::write));
	m_asic->contrast_wr_cb().set(FUNC(portfolio_state::contrast_w));

	m_ram->set_default_size("128K");

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_lcd());
	screen.set_refresh_hz(72);
	screen.set_screen_update(HD61830_TAG, FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(portfolio_state::portfolio_palette), 2);

	HD61830(config, m_lcdc, XTAL(4'915'200)/2/2);
	m_lcdc->set_addrmap(0, &portfolio_state::portfolio_lcdc);
	m_lcdc->rd_rd_callback().set(FUNC(portfolio_state::hd61830_rd_r));
	m_lcdc->set_screen(SCREEN_TAG);
}

void portfolio_state::machine_start()
{
	portfolio_base_state::machine_start();

	// mirrored 4K VRAM window, repeated across the full 0xb0000-0xbffff range
	const offs_t vram_offset = m_ram->size() - 0x1000;
	m_maincpu->space(AS_PROGRAM).install_ram(0xb0000, 0xb0fff, 0xf000, m_ram->pointer() + vram_offset);
}

void portfolio2_state::portfolio2(machine_config &config)
{
	portfolio_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &portfolio2_state::portfolio2_mem);
	m_maincpu->set_addrmap(AS_IO, &portfolio2_state::portfolio2_io);

	m_asic->contrast_wr_cb().set(FUNC(portfolio2_state::contrast_w));

	m_ram->set_default_size("512K");

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_lcd());
	screen.set_refresh_hz(56);
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_physical_aspect(640, 400);
	screen.set_screen_update(m_82c425, FUNC(f82c425_device::screen_update));

	PALETTE(config, "palette", FUNC(portfolio2_state::portfolio2_palette), 8);

	F82C425(config, m_82c425, 14.318181_MHz_XTAL);
	m_82c425->set_addrmap(0, &portfolio2_state::dispfont_map);
	m_82c425->set_screen(SCREEN_TAG);
	m_82c425->set_lcd_palette("palette");
}

void portfolio2_state::machine_start()
{
	portfolio_base_state::machine_start();

	// mirrored 4K VRAM window, repeated across 0xb0000-0xbffff except for the
	// 0xb8000-0xbbfff hole reserved for the F82C425's own video RAM (see portfolio2_mem)
	const offs_t vram_offset = m_ram->size() - 0x1000;
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_ram(0xb0000, 0xb0fff, 0x7000, m_ram->pointer() + vram_offset); // 0xb0000-0xb7fff
	program.install_ram(0xbc000, 0xbcfff, 0x3000, m_ram->pointer() + vram_offset); // 0xbc000-0xbffff
}

ROM_START( pofo )
	ROM_REGION( 0x40000, M80C88A_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "dip1072", "DIP DOS 1.072" )
	ROMX_LOAD( "c101782-007.u4", 0x00000, 0x20000, CRC(c9852766) SHA1(c74430281bc717bd36fd9b5baec1cc0f4489fe82), ROM_BIOS(0) )
	ROMX_LOAD( "c101781-007.u3", 0x20000, 0x20000, CRC(b8fb730d) SHA1(1b9d82b824cab830256d34912a643a7d048cd401), ROM_BIOS(0) )

	ROM_REGION( 0x8000, HD61830_TAG, 0 )
	ROM_LOAD( "c101783-001a-01.u3", 0x0000, 0x8000, CRC(61fdaff1) SHA1(5eb99e7a19af7b8d77ea8a2f1f554e6e3d382fa2) )
ROM_END

ROM_START( pofo2 )
	ROM_REGION( 0x40000, M80C88A_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "dip1680", "DIP DOS 1.680" )
	ROMX_LOAD( "bd_1.68_c0_15aug90.bin", 0x00000, 0x20000, CRC(c883def4) SHA1(15219b769f0e75f93d54c75cb7481d3b798a955a), ROM_BIOS(0) )
	ROMX_LOAD( "bd_1.68_e0_15aug90.bin", 0x20000, 0x20000, CRC(76df504c) SHA1(45a088507672d55fcc6d41a4ec495f23abe40095), ROM_BIOS(0) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "font.bin", 0x0000, 0x2000, BAD_DUMP CRC(736583b0) SHA1(5fce700fe1ad61a88381de6665800970818fa4af) ) // partially taken from mc600
ROM_END

} // anonymous namespace

//    YEAR  NAME   PARENT  COMPAT  MACHINE     INPUT      CLASS             INIT        COMPANY  FULLNAME       FLAGS
COMP( 1989, pofo,  0,      0,      portfolio,  portfolio, portfolio_state,  empty_init, "Atari", "Portfolio",   MACHINE_SUPPORTS_SAVE )
COMP( 1990, pofo2, pofo,   0,      portfolio2, portfolio, portfolio2_state, empty_init, "Atari", "Portfolio 2 (prototype)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
