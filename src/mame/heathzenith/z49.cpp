// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Zenith Z-49 video terminal.

    Skeleton driver, based on the Z-49 Service Manual (585-45, 1984). No
    ROMs have been dumped, and the manual has no schematics.

    Terminal logic board:
    - U119  68B09E CPU, clocked from the 24.8064 MHz video oscillator
    - U104, U105  program ROMs (28-pin sockets, 24K in all)
    - U111, U112  2K x 8 RAM (SW101 allows 8K x 8 with a different PAL)
    - U103  2816 EEPROM, holds the setup menus and the saved setup
    - U109  12L10 PAL, address decoder (equations in the manual)
    - U120  SCN2672 programmable video timing controller
    - U115  SCB2675 color/monochrome attributes controller
    - U107, U108, U116, U117  2K x 8 video RAM (4K x 16, two banks by VA13)
    - U110  character ROM (28-pin socket, 8K)
    - U122  SCN2681 DUART, 3.6864 MHz: computer port (DTE) and auxiliary port
    - U140  6850 ACIA for the keyboard, clocked at 19.7676 MHz / 64
    - U100  6821 PIA: brightness, keyboard reset, dot geometry, dot clock
            select, video blank and interlace

    The dot clock is 19.7676 MHz for 80 columns (10 x 12 cell) and
    24.8064 MHz for 132 columns (8 x 12 cell), selected by CLKSEL from the
    PIA. The CPU reaches video RAM only through the 2672's data and attribute
    buffers.

    Memory map, from the PAL equations:

    0000-1fff  RAM 1 (2K, mirrored)
    2000-3fff  RAM 2 (2K, mirrored)
    4000-47ff  EEPROM
    7f20-7f3f  parallel port B (not fitted)
    7f40-7f5f  PIA (parallel port A)
    7f60-7f7f  DUART
    7f80-7f9f  keyboard ACIA
    7fa0-7fbf  PVTC
    7fc0-7fdf  attribute latch
    7fe0-7fff  data latch
    8000-bfff  optional ROM socket
    c000-ffff  standard ROM socket

    The manual's memory map figure shows RAM at 0000 and "optional RAM"
    at 0800, which doesn't match the PAL; this driver follows the PAL.

    The keyboard has its own 6805-family MCU (U201) running at 3.579545 MHz,
    a 12 x 8 key matrix, seven LEDs and the beeper. SHIFT+RESET resets the
    terminal.

    TODO:
    - dump the program ROMs, character ROM, EEPROM and keyboard MCU
    - interrupt routing (all sources are merged onto IRQ for now)
    - PIA bit assignments (not given in the manual)
    - SCB2675 attribute controller; the attribute decoding below is a guess
    - 132-column mode (dot clock switching) and 50 Hz
    - keyboard emulation
    - parallel port B

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/eeprompar.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "video/scn2674.h"

#include "screen.h"


namespace {

class z49_state : public driver_device
{
public:
	z49_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pvtc(*this, "pvtc")
		, m_chargen(*this, "chargen")
	{
	}

	void z49(machine_config &config) ATTR_COLD;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	required_device<mc6809e_device> m_maincpu;
	required_device<scn2672_device> m_pvtc;
	required_region_ptr<u8> m_chargen;
};

/**
 * Attribute byte (from the manual's list of SCB2675 inputs)
 *
 * bit 0 - foreground intensity, low order
 * bit 1 - foreground intensity, high order
 * bit 2 - background intensity, low order
 * bit 3 - background intensity, high order
 * bit 4 - underline
 * bit 5 - blink
 * bit 6 - double wide
 * bit 7 - highest order character ROM address bit
 *
 * How the intensity bits map to levels, and the character ROM layout, are
 * guesses until the ROMs are dumped.
 */
SCN2672_DRAW_CHARACTER_MEMBER(z49_state::draw_character)
{
	u8 gfx = 0;

	if (!blink || !BIT(attrcode, 5))
		gfx = m_chargen[(BIT(attrcode, 7) << 12) | (charcode << 4) | linecount];

	if (BIT(attrcode, 4) && (linecount == 11))
		gfx = 0xff;

	if (cursor)
		gfx = ~gfx;

	// assume a foreground level of 0 means normal (full) intensity
	static constexpr u8 FG_LEVELS[4] = { 0xff, 0x60, 0xa0, 0xff };
	static constexpr u8 BG_LEVELS[4] = { 0x00, 0x30, 0x60, 0xa0 };
	rgb_t const fg = rgb_t::amber().scale8(FG_LEVELS[BIT(attrcode, 0, 2)]);
	rgb_t const bg = rgb_t::amber().scale8(BG_LEVELS[BIT(attrcode, 2, 2)]);

	// 8 dots from the ROM; the last two dots of the 10-dot cell are left blank
	u32 *const pix = &bitmap.pix(y, x);
	for (int i = 0; i < 10; i++)
		pix[i] = (i < 8 && BIT(gfx, 7 - i)) ? fg : bg;
}

void z49_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram(); // U111
	map(0x2000, 0x27ff).mirror(0x1800).ram(); // U112
	map(0x4000, 0x47ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x7f40, 0x7f43).mirror(0x1c).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x7f60, 0x7f6f).mirror(0x10).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x7f80, 0x7f81).mirror(0x1e).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x7fa0, 0x7fa7).mirror(0x18).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x7fc0, 0x7fc0).mirror(0x1f).rw(m_pvtc, FUNC(scn2672_device::attr_buffer_r), FUNC(scn2672_device::attr_buffer_w));
	map(0x7fe0, 0x7fe0).mirror(0x1f).rw(m_pvtc, FUNC(scn2672_device::buffer_r), FUNC(scn2672_device::buffer_w));
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

// used for both the character and attribute spaces; VA13 selects between the two banks of video RAM
void z49_state::vram_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x27ff).mirror(0x1800).ram();
}


static INPUT_PORTS_START(z49)
INPUT_PORTS_END


void z49_state::z49(machine_config &config)
{
	// 24.8064 MHz / 4 (U136) feeds U151, which generates E and Q
	MC6809E(config, m_maincpu, 24.8064_MHz_XTAL / 16); // U119 68B09E
	m_maincpu->set_addrmap(AS_PROGRAM, &z49_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	EEPROM_2816(config, "eeprom"); // U103

	PIA6821(config, "pia"); // U100

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // U122
	duart.irq_cb().set("irqs", FUNC(input_merger_device::in_w<0>));
	duart.a_tx_cb().set("dte", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("aux", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(duart, FUNC(scn2681_device::rx_a_w));

	rs232_port_device &aux(RS232_PORT(config, "aux", default_rs232_devices, nullptr));
	aux.rxd_handler().set(duart, FUNC(scn2681_device::rx_b_w));

	// keyboard link; the keyboard itself isn't emulated yet
	acia6850_device &acia(ACIA6850(config, "acia")); // U140
	acia.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	clock_device &kbdclk(CLOCK(config, "kbdclk", 19.7676_MHz_XTAL / 64)); // U147
	kbdclk.signal_handler().set(acia, FUNC(acia6850_device::write_txc));
	kbdclk.signal_handler().append(acia, FUNC(acia6850_device::write_rxc));

	// 80-column timing; the horizontal and vertical totals are guesses giving about 18.6 kHz and 60 Hz
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(19.7676_MHz_XTAL, 1060, 0, 800, 312, 0, 300);
	screen.set_screen_update(m_pvtc, FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 19.7676_MHz_XTAL / 10); // U120
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10);
	m_pvtc->set_display_callback(FUNC(z49_state::draw_character));
	m_pvtc->set_addrmap(0, &z49_state::vram_map);
	m_pvtc->set_addrmap(1, &z49_state::vram_map);
	m_pvtc->intr_callback().set("irqs", FUNC(input_merger_device::in_w<2>));
}


// Which program ROM sits in which socket, and their sizes, are guesses (24K in all per the block diagram)
ROM_START(z49)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("444-212-1.u105", 0x0000, 0x2000, NO_DUMP) // optional ROM socket, 8000-9fff
	ROM_LOAD("444-251-1.u104", 0x4000, 0x4000, NO_DUMP) // standard ROM socket, c000-ffff

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("444-210-1.u110", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("444-193.u103", 0x000, 0x800, NO_DUMP) // 2816 with the setup menus

	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD("444-211.u201", 0x0000, 0x1000, NO_DUMP) // keyboard MCU, not hooked up
ROM_END

} // anonymous namespace

//    year  name  parent compat  machine  input class      init        company                fullname         flags
COMP( 1984, z49,  0,     0,      z49,     z49,  z49_state, empty_init, "Zenith Data Systems", "Z-49 Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
