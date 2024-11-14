// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Liberty Electronics Freedom 200 Video Display Terminal
    Serial terminal

    Liberty Electronics Freedom 220 Video Display Terminal
    VT220 compatible serial terminal

    Hardware:
    - Z8400A Z80A
    - 4 MHz XTAL (next to CPU)
    - 3x 2764 (next to CPU)
    - 6x TMM2016AP-10 or D446C-2 (2k)
    - SCN2674B C4N40
    - SCB2675B C5N40
    - 2x 2732A (next to CRT controller)
    - 2x S68B10P (128 byte SRAM)
    - M5L8253P-5
    - 3x D8251AC
    - 18.432 MHz XTAL

    External:
    - DB25 connector "Main Port"
    - DB25 connector "Auxialiary Port"
    - Keyboard connector
    - Expansion slot

    TODO:
    - Light/dark background
    - Soft scroll
    - Pixel clock, characters should be 9 pixels?
    - I/O write to 0xc0

    Notes:
    - Use Set-Up for status line setup, Shift+Set-Up for fullscreen setup
    - On first boot you will get an "error 8" - this is because
      RAM is uninitialized.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "video/scn2674.h"

#include "freedom220_kbd.h"

#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class freedom200_state : public driver_device
{
public:
	freedom200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_avdc(*this, "avdc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_usart(*this, "usart%u", 0U),
		m_chargen(*this, "chargen"),
		m_translate(*this, "translate"),
		m_charram(*this, "charram%u", 0U),
		m_attrram(*this, "attrram%u", 0U),
		m_video_ctrl(0x00)
	{ }

	void freedom200(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<scn2674_device> m_avdc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<i8251_device, 3> m_usart;
	required_region_ptr<uint8_t> m_chargen;
	required_region_ptr<uint8_t> m_translate;
	required_shared_ptr_array<uint8_t, 2> m_charram;
	required_shared_ptr_array<uint8_t, 2> m_attrram;

	uint8_t m_video_ctrl;

	// double width support
	bool m_dw_active;
	uint8_t m_dw_char;
	uint8_t m_dw_attr;
	bool m_dw_ul;
	bool m_dw_blink;
	bool m_dw_cursor;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;
	void avdc_intr_w(int state);
	void video_ctrl_w(uint8_t data);
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void freedom200_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0x87ff).ram().share(m_charram[0]);
	map(0x8800, 0x8fff).ram().share(m_attrram[0]);
	map(0x9000, 0x97ff).ram().share(m_charram[1]);
	map(0x9800, 0x9fff).ram().share(m_attrram[1]);
	map(0xa000, 0xa7ff).ram().share("nvram");
	map(0xa800, 0xafff).ram().share("workram");
}

void freedom200_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x20, 0x23).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x40, 0x41).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x61).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x80, 0x81).rw(m_usart[2], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xa0, 0xa0).w(FUNC(freedom200_state::video_ctrl_w));
	// c0 - used by free200 only
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void freedom200_state::char_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share(m_charram[0]);
	map(0x1000, 0x17ff).ram().share(m_charram[1]);
	map(0x2000, 0x27ff).ram().share("nvram");
	map(0x2800, 0x2fff).ram().share("workram");
}

void freedom200_state::attr_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share(m_attrram[0]);
	map(0x1000, 0x17ff).ram().share(m_attrram[1]);
	map(0x2000, 0x27ff).ram().share("workram");
	map(0x2800, 0x2fff).ram().share("workram");
}

void freedom200_state::avdc_intr_w(int state)
{
	if (state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void freedom200_state::video_ctrl_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  unknown
	// --5-----  unknown
	// ---432--  translation table bit 210
	// ------1-  normal/reverse video
	// -------0  translation table bit 3

	logerror("video_ctrl_w: %02x\n", data);

	m_video_ctrl = data;
}

SCN2674_DRAW_CHARACTER_MEMBER( freedom200_state::draw_character )
{
	// 765-----  unknown
	// ---4----  normal/bold
	// ----3---  underline
	// -----2--  invert
	// ------1-  blink
	// -------0  invisible

	const pen_t *const pen = m_palette->pens();

	// either save or restore attributes for double-width
	if (dw)
	{
		if (m_dw_active)
		{
			charcode = m_dw_char;
			attrcode = m_dw_attr;
			ul = m_dw_ul;
			blink = m_dw_blink;
			cursor = m_dw_cursor;
		}
		else
		{
			m_dw_char = charcode;
			m_dw_attr = attrcode;
			m_dw_ul = ul;
			m_dw_blink = blink;
			m_dw_cursor = cursor;
		}
	}

	// apply translation table
	const int table = bitswap<4>(m_video_ctrl, 0, 4, 3, 2);
	charcode = m_translate[(table << 8) | charcode];

	uint8_t data = m_chargen[charcode << 4 | linecount];

	if (ul && (BIT(attrcode, 3)))
		data = 0xff;

	if (blink && (BIT(attrcode, 1)))
		data = 0x00;

	if (BIT(attrcode, 0))
		data = 0x00;

	if (BIT(attrcode, 2))
		data = ~data;

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = BIT(attrcode, 4) ? pen[1] : pen[2];
	rgb_t bg = pen[0];

	// reverse video?
	if (BIT(m_video_ctrl, 1))
	{
		using std::swap;
		swap(fg, bg);
	}

	// draw 8 pixels of the character
	if (dw)
	{
		// first or second half of char
		int b = m_dw_active ? 3 : 7;

		for (int i = 0; i < 4; i++)
		{
			bitmap.pix(y, x + i * 2 + 0) = BIT(data, b - i) ? fg : bg;
			bitmap.pix(y, x + i * 2 + 1) = BIT(data, b - i) ? fg : bg;
		}

		m_dw_active = !m_dw_active;
	}
	else
	{
		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x + i) = BIT(data, 7 - i) ? fg : bg;
	}
}

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void freedom200_state::machine_start()
{
	// register for save states
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_dw_active));
	save_item(NAME(m_dw_char));
	save_item(NAME(m_dw_attr));
	save_item(NAME(m_dw_ul));
	save_item(NAME(m_dw_blink));
	save_item(NAME(m_dw_cursor));
}

void freedom200_state::machine_reset()
{
	m_dw_active = false;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void freedom200_state::freedom200(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &freedom200_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &freedom200_state::io_map);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irq"));
	irq.output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(16000000, 768, 0, 640, 321, 0, 300); // clock unverified
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2674(config, m_avdc, 16000000 / 8); // clock unverified
	m_avdc->intr_callback().set(FUNC(freedom200_state::avdc_intr_w));
	m_avdc->set_screen(m_screen);
	m_avdc->set_character_width(8); // unverified
	m_avdc->set_addrmap(0, &freedom200_state::char_map);
	m_avdc->set_addrmap(1, &freedom200_state::attr_map);
	m_avdc->set_display_callback(FUNC(freedom200_state::draw_character));

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(18.432_MHz_XTAL / 10);
	pit.set_clk<1>(18.432_MHz_XTAL / 10);
	pit.set_clk<2>(18.432_MHz_XTAL / 10);
	pit.out_handler<0>().set(m_usart[2], FUNC(i8251_device::write_txc));
	pit.out_handler<0>().append(m_usart[2], FUNC(i8251_device::write_rxc));
	pit.out_handler<1>().set(m_usart[1], FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append(m_usart[1], FUNC(i8251_device::write_rxc));
	pit.out_handler<2>().set(m_usart[0], FUNC(i8251_device::write_txc));
	pit.out_handler<2>().append(m_usart[0], FUNC(i8251_device::write_rxc));

	I8251(config, m_usart[0], 0); // unknown clock
	m_usart[0]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<1>));
	m_usart[0]->txd_handler().set("mainport", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("mainport", FUNC(rs232_port_device::write_rts));

	I8251(config, m_usart[1], 0); // unknown clock
	m_usart[1]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<2>));
	m_usart[1]->txrdy_handler().set("irq", FUNC(input_merger_device::in_w<3>));
	m_usart[1]->txd_handler().set("auxport", FUNC(rs232_port_device::write_txd));
	m_usart[1]->rts_handler().set("auxport", FUNC(rs232_port_device::write_rts));

	I8251(config, m_usart[2], 0); // unknown clock
	m_usart[2]->rxrdy_handler().set("irq", FUNC(input_merger_device::in_w<4>));
	m_usart[2]->txd_handler().set("kbd", FUNC(freedom220_kbd_device::rxd_w));

	rs232_port_device &mainport(RS232_PORT(config, "mainport", default_rs232_devices, nullptr));
	mainport.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	mainport.cts_handler().set(m_usart[0], FUNC(i8251_device::write_cts));

	rs232_port_device &auxport(RS232_PORT(config, "auxport", default_rs232_devices, nullptr));
	auxport.rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	auxport.cts_handler().set(m_usart[1], FUNC(i8251_device::write_cts));

	freedom220_kbd_device &kbd(FREEDOM220_KBD(config, "kbd"));
	kbd.txd_cb().set(m_usart[2], FUNC(i8251_device::write_rxd));
	kbd.cts_cb().set(m_usart[2], FUNC(i8251_device::write_cts));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( free200 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("m120020.ic213", 0x0000, 0x2000, CRC(869de37e) SHA1(22f9c847aa8d99c22791df7a40a7c1d67f21516b))
	ROM_LOAD("m220020.ic212", 0x2000, 0x2000, CRC(85095c20) SHA1(e7515c8b188732e391015f20ad38207876850589))
	ROM_LOAD("m320020.ic214", 0x4000, 0x2000, CRC(827cafe6) SHA1(887d78bfbfcdc07efc35a7c560822b1386300632))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("g020010.bin", 0x0000, 0x1000, CRC(b99bf3b1) SHA1(799395694d2f88c68230ce7f6bf5453b799926f4))

	ROM_REGION(0x1000, "translate", 0)
	ROM_LOAD("t020010.bin", 0x0000, 0x1000, CRC(19f9c677) SHA1(903eb82b2a7b63d79c00085c8c7ec2cb9583e2a0))
ROM_END

ROM_START( free220 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("m122010__8cdd.ic213", 0x0000, 0x2000, CRC(a1181809) SHA1(0ec0fd30c8a55f0bb9e1c6453120ab9a696f9041))
	ROM_LOAD("m222010__04c8.ic212", 0x2000, 0x2000, CRC(ddd1e5eb) SHA1(3e3998035721050cd2019474343f072dade6589d))
	ROM_LOAD("m322010__8121.ic214", 0x4000, 0x2000, CRC(eeaa4b44) SHA1(93402e00205d7220f5e248a902ed92de4bbe6dd8))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("g022010__d64e.bin", 0x0000, 0x1000, CRC(a4482adc) SHA1(98479f6396743da6cf23909ff5a0097e9f021e3b))

	ROM_REGION(0x1000, "translate", 0)
	ROM_LOAD("t022010__61f0.bin", 0x0000, 0x1000, CRC(00461116) SHA1(79a53a557ea4386b3e85a312731c6c0763ab46cc))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY                FULLNAME       FLAGS
COMP( 1983, free200, 0,      0,      freedom200, 0,     freedom200_state, empty_init, "Liberty Electronics", "Freedom 200", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1984, free220, 0,      0,      freedom200, 0,     freedom200_state, empty_init, "Liberty Electronics", "Freedom 220", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
