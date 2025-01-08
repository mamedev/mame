// license: BSD-3-Clause
// copyright-holders: AJR, Dirk Best
/***************************************************************************

    Tab Products "E22" Terminal

    VT52/VT100/VT220

    Hardware:
    - P8085AH
    - SCN2674B with SCB2675T
    - SCN2681
    - P8251A
    - 3x HM6264LP-12 (8k)
    - 3x HM6116LP-2 (2k)
    - XTAL: 3.6864 MHz, 10.0000 MHz, 21.7566 MHz, 35.8344 MHz

    TODO:
    - Dump keyboard controller and emulate it (currently HLE'd)
    - NVRAM / memory layout
    - Needs a hack to send out data on the RS232 port:
        Set $f7a3 = 0 after startup

    Notes:
    - The hardware has some similarities to cit220.cpp
    - Everything here is guessed (including the system name), no docs available
    - Other (undumped) terminals from Tab:
      * 132/15: VT52/VT100/VT132 (1982)
      * 132/15-G: Tektronix graphics (1982)
      * 132/15-H: Honeywell (1983)

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/bankdev.h"
#include "e22_kbd.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/printer.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tabe22_state : public driver_device
{
public:
	tabe22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_avdc(*this, "avdc"),
		m_chargen(*this, "chargen"),
		m_vram_bank(*this, "vrambank")
	{ }

	void tabe22(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2674_device> m_avdc;
	required_region_ptr<uint8_t> m_chargen;
	required_device<address_map_bank_device> m_vram_bank;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	void video_ctrl_w(uint8_t data);
	void crt_brightness_w(uint8_t data);
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);
	void palette(palette_device &palette) const;

	bool m_screen_light = false;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void tabe22_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xc000, 0xcfff).rw(m_vram_bank, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xffff).ram();
}

void tabe22_state::vram_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
	map(0x1000, 0x1fff).ram().share("attrram");
}

void tabe22_state::io_map(address_map &map)
{
	map(0x00, 0x0f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x20, 0x27).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x40, 0x41).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x60).w(FUNC(tabe22_state::video_ctrl_w));
	map(0x61, 0x61).w(FUNC(tabe22_state::crt_brightness_w));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void tabe22_state::char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
}

void tabe22_state::attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("attrram");
}

void tabe22_state::video_ctrl_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  screen light/dark
	// --5-----  char ram/attribute ram switch
	// ---4----  unknown
	// ----3---  80/132 col switch
	// -----210  unknown

	m_screen_light = bool(BIT(data, 6));
	m_vram_bank->set_bank(BIT(data, 5));
	m_avdc->set_unscaled_clock(BIT(data, 3) ? 35.8344_MHz_XTAL / 9 : 21.7566_MHz_XTAL / 9);
}

void tabe22_state::crt_brightness_w(uint8_t data)
{
	// unknown algorithm for the brightness
	// default value is 7, range is 15 (off) to 0 (brightest)
	m_screen->set_brightness(0xff - ((data & 0x0f) * 6));
}

SCN2674_DRAW_CHARACTER_MEMBER( tabe22_state::draw_character )
{
	// 7-------  chargen high bit
	// -6------  unknown
	// --5-----  shaded
	// ---4----  unknown
	// ----3---  bold
	// -----2--  blink
	// ------1-  underline
	// -------0  reverse

	uint16_t data = m_chargen[(BIT(attrcode, 7) << 12) | charcode << 4 | linecount] << 2;
	const pen_t *const pen = m_palette->pens();

	if (ul && (BIT(attrcode, 1)))
		data = 0x1ff;

	if (blink && (BIT(attrcode, 2)))
		data = 0x000;

	if (BIT(attrcode, 0))
		data = ~data;

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = BIT(attrcode, 5) ? pen[1] : BIT(attrcode, 3) ? pen[3] : pen[2];
	rgb_t bg = m_screen_light ? pen[1] : pen[0];

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, 8 - i) ? fg : bg;
}

static const gfx_layout char_layout =
{
	8,16,
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

void tabe22_state::palette(palette_device &palette) const
{
	static constexpr rgb_t pens[4] =
	{
		{ 0x00, 0x00, 0x00 }, // black
		{ 0x7f, 0x7f, 0x7f }, // gray
		{ 0xcf, 0xcf, 0xcf }, // white
		{ 0xff, 0xff, 0xff }, // highlight
	};

	palette.set_pen_colors(0, pens);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void tabe22_state::machine_start()
{
	// register for save states
	save_item(NAME(m_screen_light));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void printer_devices(device_slot_interface &device)
{
	device.option_add("printer", SERIAL_PRINTER);
}

static DEVICE_INPUT_DEFAULTS_START( printer_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void tabe22_state::tabe22(machine_config &config)
{
	I8085A(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tabe22_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tabe22_state::io_map);

	ADDRESS_MAP_BANK(config, m_vram_bank, 0);
	m_vram_bank->set_map(&tabe22_state::vram_map);
	m_vram_bank->set_addr_width(13);
	m_vram_bank->set_data_width(8);
	m_vram_bank->set_stride(0x1000);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber());
	m_screen->set_raw(21.7566_MHz_XTAL, 918, 0, 720, 395, 0, 378); // 80 column mode
//  m_screen->set_raw(35.8344_MHz_XTAL, 1494, 0, 1188, 395, 0, 378); // 132 column mode
	m_screen->set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	PALETTE(config, m_palette, FUNC(tabe22_state::palette), 4);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2674(config, m_avdc, 21.7566_MHz_XTAL / 9);
	m_avdc->intr_callback().set_inputline(m_maincpu, I8085_RST75_LINE);
	m_avdc->set_character_width(9);
	m_avdc->set_display_callback(FUNC(tabe22_state::draw_character));
	m_avdc->set_addrmap(0, &tabe22_state::char_map);
	m_avdc->set_addrmap(1, &tabe22_state::attr_map);
	m_avdc->set_screen("screen");

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, I8085_RST65_LINE);
	duart.a_tx_cb().set("printer", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("host", FUNC(rs232_port_device::write_txd));
	duart.outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3);
	duart.outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);

	rs232_port_device &rs232_host(RS232_PORT(config, "host", default_rs232_devices, nullptr));
	rs232_host.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));
	rs232_host.cts_handler().set("duart", FUNC(scn2681_device::ip1_w));

	rs232_port_device &rs232_printer(RS232_PORT(config, "printer", printer_devices, nullptr));
	rs232_printer.set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(printer_defaults));

	i8251_device &usart(I8251(config, "usart", 10_MHz_XTAL / 4)); // divider guessed
	usart.rxrdy_handler().set_inputline(m_maincpu, I8085_RST55_LINE);
	usart.txd_handler().set("kbd", FUNC(e22_kbd_hle_device::rx_w));

	e22_kbd_hle_device &kbd(E22_KBD_HLE(config, "kbd"));
	kbd.tx_handler().set("usart", FUNC(i8251_device::write_rxd));
	kbd.cts_handler().set("usart", FUNC(i8251_device::write_cts));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START(tabe22)
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("e22_u3__2.17.86__v2.00.bin", 0x0000, 0x8000, CRC(fd931bc5) SHA1(013d5a5ed759bb9684bff18a3e848fa6d1167e10))
	ROM_LOAD("e22_u2__2.17.86__v2.00.bin", 0x8000, 0x4000, CRC(45d5e895) SHA1(a2b4e04dbc881230462a38de98ed843d5683c1b9))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("e22_u43__char_gen.bin", 0x0000, 0x2000, CRC(33880908) SHA1(7f26dede2feaf3591312d67e3dabfc1ad8bb3181))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY         FULLNAME                 FLAGS
COMP( 1986, tabe22, 0,       0,      tabe22,  0,     tabe22_state, empty_init, "Tab Products", "E-22 Display Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
