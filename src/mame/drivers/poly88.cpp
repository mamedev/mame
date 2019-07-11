// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Poly-88 driver by Miodrag Milanovic

2009-05-18 Initial implementation
2019-05-25 Poly8813 new roms

ToDo:
- POLY88 - Polyphase format not working because 8251 device doesn't support sync.
- POLY8813 - Schematic shows a 8251 on the main board.
- POLY8813 - Schematic of FDC shows a mc6852 and i8255, no dedicated fdc chip.


Poly-8813 is a disk-based computer with 3 mini-floppy drives.
Booting is done by pressing the "Load" button, mounted on the
front panel near the power switch. Although user manuals are easy
to obtain, technical information and drive controller schematics
are not. The disk format is known to be 256 bytes per sector, 10
sectors per track, 35 tracks, single sided, for a total of 89600
bytes.

Notes for old poly8813 roms:

The Poly-8813 BIOS makes use of undocumented instructions which we
do not currently emulate. These are at 006A (print a character
routine - ED ED 05); another is at 0100 (move memory routine -
ED ED 03); the last is at 087B (disk I/O routine - ED ED 01). The
code at 0100 can be replaced by 7E 12 13 23 03 79 B0 C2 00 01 C9,
which exactly fits into the available space. The routine at 006A is
likewise could be exactly replaced with F5 C5 D5 E5, which enters
a display routine that appears in other assembly listings but seems
to have no entry point here. Since the ED ED opcode is defined as
for CALLN in the NEC V20/V30's 8080 mode, it might be the case that
these are actually hooks patched into the original code for
emulation purposes. (There is also a slim possibility that this
opcode invokes an undocumented feature of the NEC uPD8080AF, which
at least some models of the Poly-88 are known to have used.)

****************************************************************************/

#include "emu.h"
#include "includes/poly88.h"

#include "cpu/i8085/i8085.h"
//#include "bus/s100/s100.h"
#include "imagedev/cassette.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


void poly88_state::poly88_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom(); // Monitor ROM
	map(0x0400, 0x0bff).rom(); // ROM Expansion
	map(0x0c00, 0x0dff).ram().mirror(0x200); // System RAM (mirrored)
	map(0x1000, 0x1fff).rom(); // System Expansion area
	map(0x2000, 0x3fff).ram(); // Minimal user RAM area
	map(0x4000, 0xf7ff).ram();
	map(0xf800, 0xfbff).ram().share("video_ram"); // Video RAM
}

void poly88_state::poly88_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x04).w(FUNC(poly88_state::baud_rate_w));
	map(0x08, 0x08).w(FUNC(poly88_state::intr_w));
	map(0xf8, 0xf8).r(FUNC(poly88_state::keyboard_r));
}

void poly88_state::poly8813_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom(); // Monitor ROM
	map(0x0400, 0x0bff).rom(); // Disk System ROM
	map(0x0c00, 0x0fff).ram(); // System RAM
	map(0x1800, 0x1bff).ram().share("video_ram"); // Video RAM
	map(0x2000, 0xffff).ram(); // RAM
}

void poly88_state::poly8813_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

/* Input ports */
static INPUT_PORTS_START( poly88 )
	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_CONFNAME( 0x80, 0x00, "Tape Mode")
	PORT_CONFSETTING(    0x00, "Byte (300 baud)")
	PORT_CONFSETTING(    0x80, "Polyphase (2400 baud)")

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~")  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END


/* F4 Character Displayer */
static const gfx_layout poly88_charlayout =
{
	8, 16,                  /* text = 7 x 9 */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_poly88 )
	GFXDECODE_ENTRY( "chargen", 0x0000, poly88_charlayout, 0, 1 )
GFXDECODE_END

void poly88_state::poly88(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 16.5888_MHz_XTAL / 9); // uses 8224 clock generator
	m_maincpu->set_addrmap(AS_PROGRAM, &poly88_state::poly88_mem);
	m_maincpu->set_addrmap(AS_IO, &poly88_state::poly88_io);
	m_maincpu->set_vblank_int("screen", FUNC(poly88_state::poly88_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(poly88_state::poly88_irq_callback));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*10, 16*15);
	screen.set_visarea(0, 64*10-1, 0, 16*15-1);
	screen.set_screen_update(FUNC(poly88_state::screen_update_poly88));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_poly88);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(poly88_state::kansas_r), attotime::from_hz(40000));

	/* uart */
	I8251(config, m_usart, 16.5888_MHz_XTAL / 9);
	m_usart->rxrdy_handler().set(FUNC(poly88_state::usart_ready_w));
	m_usart->txrdy_handler().set(FUNC(poly88_state::usart_ready_w));
	m_usart->txd_handler().set([this] (bool state) { m_txd = state; });
	m_usart->dtr_handler().set([this] (bool state) { m_dtr = state; });
	m_usart->rts_handler().set([this] (bool state) { m_rts = state; });

	MM5307AA(config, m_brg, 16.5888_MHz_XTAL / 18);
	m_brg->output_cb().set(FUNC(poly88_state::cassette_clock_w));

	/* snapshot */
	SNAPSHOT(config, "snapshot", "img", attotime::from_seconds(2)).set_load_callback(FUNC(poly88_state::snapshot_cb), this);
}

void poly88_state::poly8813(machine_config &config)
{
	poly88(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly88_state::poly8813_mem);
	m_maincpu->set_addrmap(AS_IO, &poly88_state::poly8813_io);
}

/* ROM definition */
ROM_START( poly88 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "polymon4.bin", 0x0000, 0x0400, CRC(0baa1a4c) SHA1(c6cf4b89bdde200813d34aab08150d5f3025ce33))
	ROM_LOAD( "tbasic_1.rom", 0x0400, 0x0400, CRC(ec22740e) SHA1(bc606c58ef5f046200bdf402eda66ec070464306))
	ROM_LOAD( "tbasic_2.rom", 0x0800, 0x0400, CRC(f2619232) SHA1(eb6fb0356d2fb153111cfddf39eab10253cb4c53))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )
ROM_END

ROM_START( poly8813 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "poly8813.27",  0x0000, 0x0400, CRC(0baa1a4c) SHA1(c6cf4b89bdde200813d34aab08150d5f3025ce33) )
	ROM_LOAD( "poly8813.26",  0x0400, 0x0400, CRC(7011f3a3) SHA1(228eb54b9f62649b3b674e9f1bf21f2981e12c03) )
	ROM_LOAD( "poly8813.25",  0x0800, 0x0400, CRC(9f7570e2) SHA1(767f2111b4eb856a077b1b4afe9209aca3866e52) )
	//ROM_LOAD( "poly8813-1.bin", 0x0000, 0x0400, CRC(7fd980a0) SHA1(a71d5999deb4323a11db1c0ea0dcb1dacfaf47ef))
	//ROM_LOAD( "poly8813-2.rom", 0x0400, 0x0400, CRC(1ad7c06c) SHA1(c96b8f03c184de58dbdcee18d297dbccf2d77176))
	//ROM_LOAD( "poly8813-3.rom", 0x0800, 0x0400, CRC(3df57e5b) SHA1(5b0c4febfc7515fc07e63dcb21d0ab32bc6a2e46))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )
ROM_END
/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT         COMPANY                FULLNAME     FLAGS
COMP( 1976, poly88,   0,      0,      poly88,   poly88, poly88_state, init_poly88, "PolyMorphic Systems", "Poly-88",   0 )
COMP( 1977, poly8813, poly88, 0,      poly8813, poly88, poly88_state, init_poly88, "PolyMorphic Systems", "Poly-8813", MACHINE_NOT_WORKING )
