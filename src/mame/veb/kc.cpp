// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/******************************************************************************

    kc.cpp
    system driver

    A big thankyou to Torsten Paul for his great help with this
    driver!


    Kevin Thacker [MESS driver]

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/kc.h"

#include "machine/input_merger.h"
#include "softlist_dev.h"
#include "screen.h"
#include "speaker.h"


void kc85_4_state::kc85_4_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(kc85_4_state::expansion_io_read), FUNC(kc85_4_state::expansion_io_write));

	map(0x0084, 0x0085).mirror(0xff00).rw(FUNC(kc85_4_state::kc85_4_84_r), FUNC(kc85_4_state::kc85_4_84_w));
	map(0x0086, 0x0087).mirror(0xff00).rw(FUNC(kc85_4_state::kc85_4_86_r), FUNC(kc85_4_state::kc85_4_86_w));
	map(0x0088, 0x008b).mirror(0xff00).rw(m_z80pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x008c, 0x008f).mirror(0xff00).rw(m_z80ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void kc85_4_state::kc85_4_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xa7ff).bankrw("bank3");
	map(0xa800, 0xbfff).bankrw("bank6");
	map(0xc000, 0xdfff).bankr("bank4");
	map(0xe000, 0xffff).bankr("bank5");
}

void kc_state::kc85_3_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xdfff).bankr("bank4");
	map(0xe000, 0xffff).bankr("bank5");
}

void kc_state::kc85_3_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(kc_state::expansion_io_read), FUNC(kc_state::expansion_io_write));

	map(0x0088, 0x008b).mirror(0xff00).rw(m_z80pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x008c, 0x008f).mirror(0xff00).rw(m_z80ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START( kc85 )
INPUT_PORTS_END


/* priority derived from schematics */
static const z80_daisy_config kc85_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ nullptr }
};

void kc85_cart(device_slot_interface &device)
{
	device.option_add("standard", KC_STANDARD); // standard 8KB ROM module
	device.option_add("m006", KC_M006);         // BASIC
	device.option_add("m011", KC_M011);         // 64KB RAM
	device.option_add("m022", KC_M022);         // 16KB RAM
	device.option_add("m032", KC_M032);         // 256KB segmented RAM
	device.option_add("m033", KC_M033);         // TypeStar
	device.option_add("m034", KC_M034);         // 512KB segmented RAM
	device.option_add("m035", KC_M035);         // 1MB segmented RAM
	device.option_add("m036", KC_M036);         // 128KB segmented RAM
}

void kc85_exp(device_slot_interface &device)
{
	device.option_add("d002", KC_D002);         // D002 Bus Driver
	device.option_add("d004", KC_D004);         // D004 Floppy Disk Interface
	device.option_add("d004gide", KC_D004_GIDE); // D004 Floppy Disk + GIDE Interface
}


void kc_state::kc85_slots(machine_config &config)
{
	/* devices */
	QUICKLOAD(config, "quickload", "kcc", attotime::from_seconds(2)).set_load_callback(FUNC(kc_state::quickload_cb));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(kc_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("kc_cass");

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline(m_maincpu, 0);
	INPUT_MERGER_ANY_HIGH(config, "nmi").output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	INPUT_MERGER_ANY_HIGH(config, "halt").output_handler().set_inputline(m_maincpu, INPUT_LINE_HALT);

	/* cartridge slot */
	KCCART_SLOT(config, m_expansions[0], kc85_cart, "m011");
	m_expansions[0]->set_next_slot(m_expansions[1]);
	m_expansions[0]->irq().set("irq", FUNC(input_merger_device::in_w<0>));
	m_expansions[0]->nmi().set("nmi", FUNC(input_merger_device::in_w<0>));
	m_expansions[0]->halt().set("halt", FUNC(input_merger_device::in_w<0>));

	KCCART_SLOT(config, m_expansions[1], kc85_cart, nullptr);
	m_expansions[1]->set_next_slot(m_expansions[2]);
	m_expansions[1]->irq().set("irq", FUNC(input_merger_device::in_w<1>));
	m_expansions[1]->nmi().set("nmi", FUNC(input_merger_device::in_w<1>));
	m_expansions[1]->halt().set("halt", FUNC(input_merger_device::in_w<1>));

	/* expansion interface */
	KCEXP_SLOT(config, m_expansions[2], kc85_exp, nullptr);
	m_expansions[2]->irq().set("irq", FUNC(input_merger_device::in_w<2>));
	m_expansions[2]->nmi().set("nmi", FUNC(input_merger_device::in_w<2>));
	m_expansions[2]->halt().set("halt", FUNC(input_merger_device::in_w<2>));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("kc_cart");
	SOFTWARE_LIST(config, "flop_list").set_original("kc_flop");
	SOFTWARE_LIST(config, "cass_list").set_original("kc_cass");
}


void kc_state::kc85_3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, KC85_3_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &kc_state::kc85_3_mem);
	m_maincpu->set_addrmap(AS_IO, &kc_state::kc85_3_io);
	m_maincpu->set_daisy_config(kc85_daisy_chain);

	config.set_maximum_quantum(attotime::from_hz(60));

	Z80PIO(config, m_z80pio, KC85_3_CLOCK);
	m_z80pio->out_int_callback().set_inputline(m_maincpu, 0);
	m_z80pio->in_pa_callback().set(FUNC(kc_state::pio_porta_r));
	m_z80pio->out_pa_callback().set(FUNC(kc_state::pio_porta_w));
	m_z80pio->out_ardy_callback().set(FUNC(kc_state::pio_ardy_cb));
	m_z80pio->in_pb_callback().set(FUNC(kc_state::pio_portb_r));
	m_z80pio->out_pb_callback().set(FUNC(kc_state::pio_portb_w));
	m_z80pio->out_brdy_callback().set(FUNC(kc_state::pio_brdy_cb));

	Z80CTC(config, m_z80ctc, KC85_3_CLOCK);
	m_z80ctc->intr_callback().set_inputline(m_maincpu, 0);
	m_z80ctc->zc_callback<0>().set(FUNC(kc_state::ctc_zc0_callback));
	m_z80ctc->zc_callback<1>().set(FUNC(kc_state::ctc_zc1_callback));
	m_z80ctc->zc_callback<2>().set(FUNC(kc_state::video_toggle_blink_state));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'375'160)/2, 908, 0, 320, 312, 0, 256);
	m_screen->set_screen_update(FUNC(kc_state::screen_update));
	m_screen->set_palette("palette");
	TIMER(config, "scantimer").configure_scanline(FUNC(kc_state::kc_scanline), "screen", 0, 1);

	PALETTE(config, "palette", FUNC(kc_state::kc85_palette), KC85_PALETTE_SIZE);

	kc_keyboard_device &keyboard(KC_KEYBOARD(config, "keyboard", XTAL(4'000'000)));
	keyboard.out_wr_callback().set(FUNC(kc_state::keyboard_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	kc85_slots(config);

	/* internal ram */
	RAM(config, m_ram).set_default_size("16K");
}

void kc85_4_state::kc85_4(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, KC85_4_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &kc85_4_state::kc85_4_mem);
	m_maincpu->set_addrmap(AS_IO, &kc85_4_state::kc85_4_io);
	m_maincpu->set_daisy_config(kc85_daisy_chain);
	config.set_maximum_quantum(attotime::from_hz(60));

	Z80PIO(config, m_z80pio, KC85_4_CLOCK);
	m_z80pio->out_int_callback().set_inputline(m_maincpu, 0);
	m_z80pio->in_pa_callback().set(FUNC(kc_state::pio_porta_r));
	m_z80pio->out_pa_callback().set(FUNC(kc_state::pio_porta_w));
	m_z80pio->out_ardy_callback().set(FUNC(kc_state::pio_ardy_cb));
	m_z80pio->in_pb_callback().set(FUNC(kc_state::pio_portb_r));
	m_z80pio->out_pb_callback().set(FUNC(kc_state::pio_portb_w));
	m_z80pio->out_brdy_callback().set(FUNC(kc_state::pio_brdy_cb));

	Z80CTC(config, m_z80ctc, 0);
	m_z80ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80ctc->zc_callback<0>().set(FUNC(kc_state::ctc_zc0_callback));
	m_z80ctc->zc_callback<1>().set(FUNC(kc_state::ctc_zc1_callback));
	m_z80ctc->zc_callback<2>().set(FUNC(kc_state::video_toggle_blink_state));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(28'375'160)/2, 908, 0, 320, 312, 0, 256);
	m_screen->set_screen_update(FUNC(kc85_4_state::screen_update));
	m_screen->set_palette("palette");
	TIMER(config, "scantimer").configure_scanline(FUNC(kc85_4_state::kc_scanline), "screen", 0, 1);

	PALETTE(config, "palette", FUNC(kc85_4_state::kc85_palette), KC85_PALETTE_SIZE);

	kc_keyboard_device &keyboard(KC_KEYBOARD(config, "keyboard", XTAL(4'000'000)));
	keyboard.out_wr_callback().set(FUNC(kc_state::keyboard_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	kc85_slots(config);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K");
}

void kc85_4_state::kc85_5(machine_config &config)
{
	kc85_4(config);
	/* internal ram */
	m_ram->set_default_size("256K");
}


ROM_START(kc85_2)
	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "hc900", "HC900 CAOS" )
	ROMX_LOAD("hc900.852",    0x2000, 0x2000, CRC(e6f4c0ab) SHA1(242a777788c774c5f764313361b1e0a65139ab32), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "caos22", "CAOS 2.2" )
	ROMX_LOAD("caos__e0.852", 0x2000, 0x2000, CRC(48d5624c) SHA1(568dd59bfad4c604ba36bc05b094fc598a642f85), ROM_BIOS(1))
ROM_END

ROM_START(kc85_3)
	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD( "basic_c0.853", 0x0000, 0x2000, CRC(dfe34b08) SHA1(c2e3af55c79e049e811607364f88c703b0285e2e))

	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos31", "CAOS 3.1" )
	ROMX_LOAD("caos__e0.853", 0x2000, 0x2000, CRC(639e4864) SHA1(efd002fc9146116936e6e6be0366d2afca33c1ab), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "caos33", "CAOS 3.3" )
	ROMX_LOAD("caos33.853",   0x2000, 0x2000, CRC(ca0fecad) SHA1(20447d27c9aa41a1c7a3d6ad0699edb06a207aa6), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "caos34", "CAOS 3.4" )
	ROMX_LOAD("caos34.853",   0x2000, 0x2000, CRC(d0245a3e) SHA1(ee9f8e7427b9225ae2cecbcfb625d629ab6a601d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "pi88ge", "OS PI/88 (yellow/blue)" )
	ROMX_LOAD("pi88_ge.853",  0x2000, 0x2000, CRC(4bf0cfde) SHA1(b8373a44e4553197e3dd23008168d5214b878837), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "pi88sw", "OS PI/88 (black/white)" )
	ROMX_LOAD("pi88_sw.853",  0x2000, 0x2000, CRC(f7d2e8fc) SHA1(9b5c068f10ff34bc3253f5b51abad51c8da9dd5d), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "pi88ws", "OS PI/88 (white/blue)" )
	ROMX_LOAD("pi88_ws.853",  0x2000, 0x2000, CRC(9ef4efbf) SHA1(b8b6f606b76bce9fb7fcd61a14120e5e026b6b6e), ROM_BIOS(5))
ROM_END

ROM_START(kc85_4)
	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD("basic_c0.854", 0x0000, 0x2000, CRC(dfe34b08) SHA1(c2e3af55c79e049e811607364f88c703b0285e2e))
	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos42", "CAOS 4.2" )
	ROMX_LOAD("caos__c0.854", 0x0000, 0x1000, CRC(57d9ab02) SHA1(774fc2496a59b77c7c392eb5aa46420e7722797e), ROM_BIOS(0))
	ROMX_LOAD("caos__e0.854", 0x2000, 0x2000, CRC(ee273933) SHA1(4300f7ff813c1fb2d5c928dbbf1c9e1fe52a9577), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "caos41", "CAOS 4.1" )
	ROMX_LOAD("caos41c.854", 0x0000, 0x1000, CRC(c7e1c011) SHA1(acd998e3d9e8f592cd884aafc8ac4d291e40e097), ROM_BIOS(1))
	ROMX_LOAD("caos41e.854", 0x2000, 0x2000, CRC(60e045e5) SHA1(e19819fb477dcb742a13729a9bf5943d63abe863), ROM_BIOS(1))
ROM_END

ROM_START(kc85_5)
	ROM_REGION(0x8000, "basic", 0)
	ROM_LOAD("basic_c0.855", 0x0000, 0x8000, CRC(0ed9f8b0) SHA1(be2c68a5b461014c57e33a127c3ffb32b0ff2346))

	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos44", "CAOS 4.4" )
	ROMX_LOAD("caos__c0.855",0x0000, 0x2000, CRC(f56d5c18) SHA1(2cf8023ee71ca50b92f9f151b7519f59727d1c79), ROM_BIOS(0))
	ROMX_LOAD("caos__e0.855",0x2000, 0x2000, CRC(1dbc2e6d) SHA1(53ba4394d96e287ff8af01322af1e9879d4e77c4), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "caos43", "CAOS 4.3" )
	ROMX_LOAD("caos43c.855", 0x0000, 0x2000, CRC(2f0f9eaa) SHA1(5342be5104206d15e7471b094c7749a8a3d708ad), ROM_BIOS(1))
	ROMX_LOAD("caos43e.855", 0x2000, 0x2000, CRC(b66fc6c3) SHA1(521ac2fbded4148220f8af2d5a5ab99634364079), ROM_BIOS(1))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY, FULLNAME, FLAGS
COMP( 1987, kc85_2, 0,      0,      kc85_3,  kc85,  kc_state,     empty_init, u8"VEB Mikroelektronik \"Wilhelm Pieck\" Mühlhausen", "HC900 / KC 85/2", MACHINE_NOT_WORKING)
COMP( 1987, kc85_3, kc85_2, 0,      kc85_3,  kc85,  kc_state,     empty_init, u8"VEB Mikroelektronik \"Wilhelm Pieck\" Mühlhausen", "KC 85/3",         MACHINE_NOT_WORKING)
COMP( 1989, kc85_4, kc85_2, 0,      kc85_4,  kc85,  kc85_4_state, empty_init, u8"VEB Mikroelektronik \"Wilhelm Pieck\" Mühlhausen", "KC 85/4",         MACHINE_NOT_WORKING)
COMP( 1989, kc85_5, kc85_2, 0,      kc85_5,  kc85,  kc85_4_state, empty_init, u8"VEB Mikroelektronik \"Wilhelm Pieck\" Mühlhausen", "KC 85/5",         MACHINE_NOT_WORKING)
