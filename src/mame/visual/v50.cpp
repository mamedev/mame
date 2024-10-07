// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Visual Technology Visual 50/55

    VT52 compatible terminal

    Hardware:
    - D780C (Z80)
    - 2764 and 2732 EPROM
    - 2x TMM314APL-1 (1k RAM)
    - X2210D NOVRAM (64x4) (X2210 for REV A)
    - Intel P8253-5 PIT
    - D8255AC-5 PPI
    - 2x D8251AC USART
    - SCN2672A (SCN2672 N for REV A)
    - 2673
    - TMM2016P-2 (2k VRAM)
    - C68100 IC240-001R00 (character generator?)
    - 17.320 MHz XTAL

    TODO:
    - Screen attributes (reverse sometimes works)
    - Smooth scroll
    - Screen brightness control
    - Most PPI connections are unknown
    - AUX port

    Notes:
    - PCB marked "PA015-A REV A" for R08 firmware
    - PCB marked "PA015 REV B 1183" for R11 firmware
    - Use TERM=vi50 and disable AUTO LF/CR in setup mode

***************************************************************************/

#include "emu.h"

#include "v50_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/x2212.h"
#include "video/scn2674.h"

#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class visual50_state : public driver_device
{
public:
	visual50_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_novram(*this, "novram"),
		m_pit(*this, "pit"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_pvtc(*this, "pvtc"),
		m_usart(*this, "usart%u", 0U),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
	{ }

	void visual50(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<x2210_device> m_novram;
	required_device<pit8253_device> m_pit;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2672_device> m_pvtc;
	required_device_array<i8251_device, 2> m_usart;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;

	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void ppi_porta_w(uint8_t data);
	void ppi_portb_w(uint8_t data);
	void ppi_portc_w(uint8_t data);

	uint8_t recall_r();
	void recall_w(uint8_t data);
	uint8_t store_r();
	void store_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void visual50_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x8000, 0x83ff).ram();
	map(0xa000, 0xa03f).rw(m_novram, FUNC(x2210_device::read), FUNC(x2210_device::write));
	map(0xc000, 0xc000).rw(FUNC(visual50_state::recall_r), FUNC(visual50_state::recall_w));
	map(0xe000, 0xe000).rw(FUNC(visual50_state::store_r), FUNC(visual50_state::store_w));
}

void visual50_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x10, 0x11).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x21).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x40, 0x40).r(m_pvtc, FUNC(scn2672_device::buffer_r));
	map(0x50, 0x50).w(m_pvtc, FUNC(scn2672_device::buffer_w));
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x70, 0x73).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

void visual50_state::char_map(address_map &map)
{
	map(0x000, 0x7ff).ram().share("vram");
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( visual50 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

SCN2672_DRAW_CHARACTER_MEMBER( visual50_state::draw_character )
{
	uint16_t data = m_chargen[(charcode & 0x7f) << 4 | linecount];
	const pen_t *const pen = m_palette->pens();

	if (BIT(charcode, 7))
		data = ~data; // wrong

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = pen[2];
	rgb_t bg = pen[0];

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, 8 - i) ? fg : bg;
}

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	8 * 16
};

static GFXDECODE_START( chars )
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void visual50_state::ppi_porta_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  unknown
	// --5-----  unknown
	// ---43210  brightness (0x00 = brightest, 0x1f = darkest)

	logerror("porta_w: %02x\n", data);
}

void visual50_state::ppi_portb_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  monitor mode
	// --5-----  unknown
	// ---4----  reverse video
	// ----3---  unknown
	// -----2--  local echo
	// ------1-  unknown
	// -------0  unknown

	logerror("portb_w: %02x\n", data);
}

void visual50_state::ppi_portc_w(uint8_t data)
{
	// 7-------  unknown
	// -6------  rs232/current loop
	// --5-----  unknown
	// ---4----  unknown, toggles
	// ----3---  reverse video
	// -----2--  unknown
	// ------1-  unknown
	// -------0  unknown

	logerror("portc_w: %02x\n", data);
}

uint8_t visual50_state::recall_r()
{
	if (!machine().side_effects_disabled())
	{
		m_novram->recall(1);
		m_novram->recall(0);
	}

	return 0xff;
}

void visual50_state::recall_w(uint8_t data)
{
	m_novram->recall(1);
	m_novram->recall(0);
}

uint8_t visual50_state::store_r()
{
	if (!machine().side_effects_disabled())
	{
		m_novram->store(1);
		m_novram->store(0);
	}

	return 0xff;
}

void visual50_state::store_w(uint8_t data)
{
	m_novram->store(1);
	m_novram->store(0);
}

void visual50_state::machine_start()
{
}

void visual50_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void visual50_state::visual50(machine_config &config)
{
	Z80(config, m_maincpu, 17.320_MHz_XTAL / 8); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &visual50_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &visual50_state::io_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	X2210(config, m_novram);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(17.320_MHz_XTAL / 8);
	m_pit->set_clk<1>(17.320_MHz_XTAL / 8);
	m_pit->set_clk<2>(17.320_MHz_XTAL / 8);
	m_pit->out_handler<0>().set(m_usart[0], FUNC(i8251_device::write_rxc)); // or txc?
	m_pit->out_handler<1>().set(m_usart[1], FUNC(i8251_device::write_rxc));
	m_pit->out_handler<1>().append(m_usart[1], FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().set(m_usart[0], FUNC(i8251_device::write_txc)); // or rxc?

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(visual50_state::ppi_porta_w));
	ppi.out_pb_callback().set(FUNC(visual50_state::ppi_portb_w));
	ppi.out_pc_callback().set(FUNC(visual50_state::ppi_portc_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(17.320_MHz_XTAL, 900, 0, 720, 321, 0, 300);
	m_screen->set_screen_update(m_pvtc, FUNC(scn2672_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2672(config, m_pvtc, 17.320_MHz_XTAL / 9);
	m_pvtc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9);
	m_pvtc->set_display_callback(FUNC(visual50_state::draw_character));
	m_pvtc->set_addrmap(0, &visual50_state::char_map);

	// modem port
	I8251(config, m_usart[0], 17.320_MHz_XTAL / 8); // divider not verified
	m_usart[0]->rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	modem.cts_handler().set(m_usart[0], FUNC(i8251_device::write_cts));

	// keyboard
	I8251(config, m_usart[1], 17.320_MHz_XTAL / 8); // divider not verified
	m_usart[1]->rxrdy_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	m_usart[1]->txd_handler().set("kbd", FUNC(v50_kbd_device::rxd_w));

	v50_kbd_device &kbd(V50_KBD(config, "kbd"));
	kbd.txd_cb().set(m_usart[1], FUNC(i8251_device::write_rxd));
	kbd.cts_cb().set(m_usart[1], FUNC(i8251_device::write_cts));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( visual50 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_DEFAULT_BIOS("r11")
	ROM_SYSTEM_BIOS(0, "r08", "Revision 0.08")
	ROMX_LOAD("e244-012_r08.u2", 0x0000, 0x2000, CRC(3931796d) SHA1(1f71b977e021a2f5eb5437c4056991cc7601768b), ROM_BIOS(0))
	ROMX_LOAD("e262-055_r08.u3", 0x2000, 0x1000, CRC(20067dc0) SHA1(bf8051117876246b3c3ba88c1c750e146800ca0b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "r11", "Revision 0.11")
	ROMX_LOAD("e244-012_r11.u2", 0x0000, 0x2000, CRC(755f0722) SHA1(80b90882c52c9ddcc1cf455c9195d4e1ccda8ce8), ROM_BIOS(1))
	ROMX_LOAD("e262-055_r11.u3", 0x2000, 0x1000, CRC(90a142e8) SHA1(5f4c403b7ab09dcb3cfdc8f57f65e0a52992feed), ROM_BIOS(1))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("ic240-001r00.u32", 0x000, 0x800, CRC(5041acd0) SHA1(ef3d952140c33c6cf2712463b53e8fca4a5400f4))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY              FULLNAME     FLAGS
COMP( 1983, visual50, 0,      0,      visual50, visual50, visual50_state, empty_init, "Visual Technology", "Visual 50", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
