// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Poly-88 driver by Miodrag Milanovic

2009-05-18 Initial implementation
2019-05-25 Poly8813 new roms

All input must be UPPERcase.

11K is the minimal amount of user RAM required to run the later version of
BASIC. PolyMorphic's "System 16" package shipped with 16K of RAM (as did
the 8813), though their earlier systems had only 8K or less.

ToDo:
- More accurate interrupt emulation.
- Single-step control.
- .CAS file format support (http://deramp.com/polymorphic-computers/emu88.html).

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

#include "bus/s100/poly16k.h"
#include "bus/s100/polyfdc.h"
#include "bus/s100/polyvti.h"
#include "bus/s100/seals8k.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "speaker.h"


void poly88_state::s100_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(poly88_state::mem_r), FUNC(poly88_state::mem_w));
}

void poly88_state::s100_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(poly88_state::in_r), FUNC(poly88_state::out_w));
}

void poly88_state::poly88_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).mirror(2).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x04).mirror(3).w(FUNC(poly88_state::baud_rate_w));
	map(0x08, 0x08).mirror(3).w(FUNC(poly88_state::intr_w));
}

void poly88_state::poly8813_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).mirror(2).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x04).mirror(3).w(FUNC(poly88_state::baud_rate_w));
	map(0x08, 0x0b); //.r(ROM on for CPM).w(RTC reset);
	map(0x0c, 0x0f); //.r(ROM off for CPM).w(Single-step trigger);
}

/* Input ports */
static INPUT_PORTS_START( poly88 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x80, 0x00, "Tape Mode")
	PORT_CONFSETTING(   0x00, "Byte (300 baud)")
	PORT_CONFSETTING(   0x80, "Polyphase (2400 baud)")

	PORT_START("ONBOARD")
	PORT_CONFNAME(7, 0, "Onboard Addresses")
	PORT_CONFSETTING(0, "0000-0FFF (M), 00-0F (I/O)") // jumper J
	PORT_CONFSETTING(4, "8000-8FFF (M), 80-8F (I/O)") // jumper S
	PORT_CONFSETTING(7, "E000-EFFF (M), E0-EF (I/O)") // jumper T
INPUT_PORTS_END


static void poly88_s100_devices(device_slot_interface &device)
{
	device.option_add("vti", S100_POLY_VTI);
	device.option_add("8ksc", S100_8K_SC);
	device.option_add("8kscbb", S100_8K_SC_BB);
	device.option_add("poly16k", S100_POLY_16K);
	device.option_add("polyfdc", S100_POLY_FDC);
}

DEVICE_INPUT_DEFAULTS_START(poly88_vti_1800)
	DEVICE_INPUT_DEFAULTS("ADDRESS", 0x3f, 0x06) // 1800-1FFF
DEVICE_INPUT_DEFAULTS_END

DEVICE_INPUT_DEFAULTS_START(poly88_16k_2000)
	DEVICE_INPUT_DEFAULTS("DSW", 0xf, 0xd) // 2000-5FFF
DEVICE_INPUT_DEFAULTS_END

DEVICE_INPUT_DEFAULTS_START(poly88_16k_6000)
	DEVICE_INPUT_DEFAULTS("DSW", 0xf, 0x9) // 6000-9FFF
DEVICE_INPUT_DEFAULTS_END

DEVICE_INPUT_DEFAULTS_START(poly88_16k_a000)
	DEVICE_INPUT_DEFAULTS("DSW", 0xf, 0x5) // A000-DFFF
DEVICE_INPUT_DEFAULTS_END

void poly88_state::poly88(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 16.5888_MHz_XTAL / 9); // uses 8224 clock generator
	m_maincpu->set_addrmap(AS_PROGRAM, &poly88_state::s100_mem);
	m_maincpu->set_addrmap(AS_IO, &poly88_state::s100_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(poly88_state::poly88_irq_callback));

	ADDRESS_MAP_BANK(config, m_onboard_io);
	m_onboard_io->set_addrmap(0, &poly88_state::poly88_io);
	m_onboard_io->set_data_width(8);
	m_onboard_io->set_addr_width(4);

	TIMER(config, "rtc").configure_periodic(FUNC(poly88_state::rtc_tick), attotime::from_hz(60)); // from AC power

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(poly88_state::kansas_r), attotime::from_hz(38400));

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

	S100_BUS(config, m_s100, 16.5888_MHz_XTAL / 9);
	m_s100->vi2().set(FUNC(poly88_state::vi2_w));
	m_s100->vi5().set(FUNC(poly88_state::vi5_w));

	// Poly-88 backplane has 5 slots, but CPU uses one
	S100_SLOT(config, m_s100_slot[0], poly88_s100_devices, "vti");
	S100_SLOT(config, m_s100_slot[1], poly88_s100_devices, "poly16k");
	S100_SLOT(config, m_s100_slot[2], poly88_s100_devices, nullptr);
	S100_SLOT(config, m_s100_slot[3], poly88_s100_devices, nullptr);

	m_s100_slot[1]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_2000));
	m_s100_slot[2]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_6000));
	m_s100_slot[3]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_a000));
}

void poly88_state::poly8813(machine_config &config)
{
	poly88(config);
	m_onboard_io->set_addrmap(0, &poly88_state::poly8813_io);

	m_s100_slot[0]->set_option_device_input_defaults("vti", DEVICE_INPUT_DEFAULTS_NAME(poly88_vti_1800));
	m_s100_slot[1]->set_default_option("polyfdc");
	m_s100_slot[2]->set_default_option("poly16k");

	// Poly-8813 backplane has 10 slots, but CPU uses one
	S100_SLOT(config, m_s100_slot[4], poly88_s100_devices, nullptr);
	S100_SLOT(config, m_s100_slot[5], poly88_s100_devices, nullptr);
	S100_SLOT(config, m_s100_slot[6], poly88_s100_devices, nullptr);
	S100_SLOT(config, m_s100_slot[7], poly88_s100_devices, nullptr);
	S100_SLOT(config, m_s100_slot[8], poly88_s100_devices, nullptr);

	m_s100_slot[2]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_2000));
	m_s100_slot[3]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_6000));
	m_s100_slot[4]->set_option_device_input_defaults("poly16k", DEVICE_INPUT_DEFAULTS_NAME(poly88_16k_a000));
}

/* ROM definition */
ROM_START( poly88 )
	ROM_REGION( 0xc00, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "polymon4.bin", 0x0000, 0x0400, CRC(0baa1a4c) SHA1(c6cf4b89bdde200813d34aab08150d5f3025ce33))
	ROM_LOAD( "tbasic_1.rom", 0x0400, 0x0400, CRC(ec22740e) SHA1(bc606c58ef5f046200bdf402eda66ec070464306))
	ROM_LOAD( "tbasic_2.rom", 0x0800, 0x0400, CRC(f2619232) SHA1(eb6fb0356d2fb153111cfddf39eab10253cb4c53))
ROM_END

ROM_START( poly8813 )
	ROM_REGION( 0xc00, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "poly8813.27",  0x0000, 0x0400, CRC(0baa1a4c) SHA1(c6cf4b89bdde200813d34aab08150d5f3025ce33) )
	ROM_LOAD( "poly8813.26",  0x0400, 0x0400, CRC(7011f3a3) SHA1(228eb54b9f62649b3b674e9f1bf21f2981e12c03) )
	ROM_LOAD( "poly8813.25",  0x0800, 0x0400, CRC(9f7570e2) SHA1(767f2111b4eb856a077b1b4afe9209aca3866e52) )
	//ROM_LOAD( "poly8813-1.bin", 0x0000, 0x0400, CRC(7fd980a0) SHA1(a71d5999deb4323a11db1c0ea0dcb1dacfaf47ef))
	//ROM_LOAD( "poly8813-2.rom", 0x0400, 0x0400, CRC(1ad7c06c) SHA1(c96b8f03c184de58dbdcee18d297dbccf2d77176))
	//ROM_LOAD( "poly8813-3.rom", 0x0800, 0x0400, CRC(3df57e5b) SHA1(5b0c4febfc7515fc07e63dcb21d0ab32bc6a2e46))
ROM_END
/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT         COMPANY                FULLNAME     FLAGS
COMP( 1976, poly88,   0,      0,      poly88,   poly88, poly88_state, empty_init,  "PolyMorphic Systems", "Poly-88",   0 )
COMP( 1977, poly8813, poly88, 0,      poly8813, poly88, poly88_state, empty_init,  "PolyMorphic Systems", "Poly-8813", MACHINE_NOT_WORKING )
