// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/******************************************************************************

    kc.c
    system driver

    A big thankyou to Torsten Paul for his great help with this
    driver!


    Kevin Thacker [MESS driver]

 ******************************************************************************/


/* Core includes */
#include "emu.h"
#include "includes/kc.h"
#include "softlist.h"

static ADDRESS_MAP_START(kc85_4_io, AS_IO, 8, kc85_4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0084, 0x0085) AM_MIRROR(0xff00) AM_READWRITE(kc85_4_84_r, kc85_4_84_w)
	AM_RANGE(0x0086, 0x0087) AM_MIRROR(0xff00) AM_READWRITE(kc85_4_86_r, kc85_4_86_w)
	AM_RANGE(0x0088, 0x008b) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
	AM_RANGE(0x008c, 0x008f) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)

	AM_RANGE(0x0000, 0xffff) AM_READWRITE(expansion_io_read, expansion_io_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(kc85_4_mem, AS_PROGRAM, 8, kc85_4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE_BANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE_BANK("bank2")
	AM_RANGE(0x8000, 0xa7ff) AM_READWRITE_BANK("bank3")
	AM_RANGE(0xa800, 0xbfff) AM_READWRITE_BANK("bank6")
	AM_RANGE(0xc000, 0xdfff) AM_READ_BANK("bank4")
	AM_RANGE(0xe000, 0xffff) AM_READ_BANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START(kc85_3_mem, AS_PROGRAM, 8, kc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE_BANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE_BANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE_BANK("bank3")
	AM_RANGE(0xc000, 0xdfff) AM_READ_BANK("bank4")
	AM_RANGE(0xe000, 0xffff) AM_READ_BANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START(kc85_3_io, AS_IO, 8, kc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0088, 0x008b) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
	AM_RANGE(0x008c, 0x008f) AM_MIRROR(0xff00) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)

	AM_RANGE(0x0000, 0xffff) AM_READWRITE(expansion_io_read, expansion_io_write)
ADDRESS_MAP_END

static INPUT_PORTS_START( kc85 )
INPUT_PORTS_END


/* priority derived from schematics */
static const z80_daisy_config kc85_daisy_chain[] =
{
	{ "z80ctc" },
	{ "z80pio" },
	{ nullptr }
};

extern SLOT_INTERFACE_START(kc85_cart)
	SLOT_INTERFACE("standard", KC_STANDARD) // standard 8KB ROM module
	SLOT_INTERFACE("m006", KC_M006)         // BASIC
	SLOT_INTERFACE("m011", KC_M011)         // 64KB RAM
	SLOT_INTERFACE("m022", KC_M022)         // 16KB RAM
	SLOT_INTERFACE("m032", KC_M032)         // 256KB segmented RAM
	SLOT_INTERFACE("m033", KC_M033)         // TypeStar
	SLOT_INTERFACE("m034", KC_M034)         // 512KB segmented RAM
	SLOT_INTERFACE("m035", KC_M035)         // 1MB segmented RAM
	SLOT_INTERFACE("m036", KC_M036)         // 128KB segmented RAM
SLOT_INTERFACE_END

extern SLOT_INTERFACE_START(kc85_exp)
	SLOT_INTERFACE("d002", KC_D002)         // D002 Bus Driver
	SLOT_INTERFACE("d004", KC_D004)         // D004 Floppy Disk Interface
	SLOT_INTERFACE("d004gide", KC_D004_GIDE) // D004 Floppy Disk + GIDE Interface
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( kc85_3, kc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, KC85_3_CLOCK)
	MCFG_CPU_PROGRAM_MAP(kc85_3_mem)
	MCFG_CPU_IO_MAP(kc85_3_io)
	MCFG_CPU_CONFIG(kc85_daisy_chain)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, KC85_3_CLOCK)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", 0))
	MCFG_Z80PIO_IN_PA_CB(READ8(kc_state, pio_porta_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(kc_state, pio_porta_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(kc_state, pio_ardy_cb))
	MCFG_Z80PIO_IN_PB_CB(READ8(kc_state, pio_portb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(kc_state, pio_portb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(kc_state, pio_brdy_cb))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, KC85_3_CLOCK)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", 0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(kc_state, ctc_zc0_callback))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(kc_state, ctc_zc1_callback))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(kc_state, video_toggle_blink_state))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_28_37516MHz/2, 908, 0, 320, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(kc_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", kc_state, kc_scanline, "screen", 0, 1)

	MCFG_PALETTE_ADD("palette", KC85_PALETTE_SIZE)
	MCFG_PALETTE_INIT_OWNER(kc_state, kc85 )

	MCFG_DEVICE_ADD("keyboard", KC_KEYBOARD, XTAL_4MHz)
	MCFG_KC_KEYBOARD_OUT_CALLBACK(WRITELINE(kc_state, keyboard_cb))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", kc_state, kc, "kcc", 2)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(kc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("kc_cass")

	/* cartridge slot */
	MCFG_DEVICE_ADD("m8", KCCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_cart, "m011", false)
	MCFG_KCCART_SLOT_NEXT_SLOT("mc")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))
	MCFG_DEVICE_ADD("mc", KCCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_cart, nullptr, false)
	MCFG_KCCART_SLOT_NEXT_SLOT("exp")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))

	/* expansion interface */
	MCFG_DEVICE_ADD("exp", KCEXP_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_exp, nullptr, false)
	MCFG_KCCART_SLOT_NEXT_SLOT("")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "kc_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "kc_flop")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "kc_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( kc85_4, kc85_4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, KC85_4_CLOCK)
	MCFG_CPU_PROGRAM_MAP(kc85_4_mem)
	MCFG_CPU_IO_MAP(kc85_4_io)
	MCFG_CPU_CONFIG(kc85_daisy_chain)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, KC85_4_CLOCK)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", 0))
	MCFG_Z80PIO_IN_PA_CB(READ8(kc_state, pio_porta_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(kc_state, pio_porta_w))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(kc_state, pio_ardy_cb))
	MCFG_Z80PIO_IN_PB_CB(READ8(kc_state, pio_portb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(kc_state, pio_portb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(kc_state, pio_brdy_cb))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, KC85_4_CLOCK)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", 0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(kc_state, ctc_zc0_callback))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(kc_state, ctc_zc1_callback))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(kc_state, video_toggle_blink_state))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_28_37516MHz/2, 908, 0, 320, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(kc85_4_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", kc85_4_state, kc_scanline, "screen", 0, 1)

	MCFG_PALETTE_ADD("palette", KC85_PALETTE_SIZE)
	MCFG_PALETTE_INIT_OWNER(kc85_4_state, kc85 )

	MCFG_DEVICE_ADD("keyboard", KC_KEYBOARD, XTAL_4MHz)
	MCFG_KC_KEYBOARD_OUT_CALLBACK(WRITELINE(kc_state, keyboard_cb))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_QUICKLOAD_ADD("quickload", kc_state, kc, "kcc", 2)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(kc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("kc_cass")

	/* cartridge slot */
	MCFG_DEVICE_ADD("m8", KCCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_cart, "m011", false)
	MCFG_KCCART_SLOT_NEXT_SLOT("mc")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))
	MCFG_DEVICE_ADD("mc", KCCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_cart, nullptr, false)
	MCFG_KCCART_SLOT_NEXT_SLOT("exp")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))

	/* expansion interface */
	MCFG_DEVICE_ADD("exp", KCEXP_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(kc85_exp, nullptr, false)
	MCFG_KCCART_SLOT_NEXT_SLOT("")
	MCFG_KCCART_SLOT_OUT_IRQ_CB(INPUTLINE("maincpu", 0))
	MCFG_KCCART_SLOT_OUT_NMI_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_KCCART_SLOT_OUT_HALT_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "kc_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "kc_flop")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "kc_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kc85_5, kc85_4 )
	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
MACHINE_CONFIG_END


ROM_START(kc85_2)
	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "hc900", "HC900 CAOS" )
	ROMX_LOAD( "hc900.852",    0x2000, 0x2000, CRC(e6f4c0ab) SHA1(242a777788c774c5f764313361b1e0a65139ab32), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "caos22", "CAOS 2.2" )
	ROMX_LOAD( "caos__e0.852", 0x2000, 0x2000, CRC(48d5624c) SHA1(568dd59bfad4c604ba36bc05b094fc598a642f85), ROM_BIOS(2))
ROM_END

ROM_START(kc85_3)
	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD( "basic_c0.853", 0x0000, 0x2000, CRC(dfe34b08) SHA1(c2e3af55c79e049e811607364f88c703b0285e2e))

	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos31", "CAOS 3.1" )
	ROMX_LOAD( "caos__e0.853", 0x2000, 0x2000, CRC(639e4864) SHA1(efd002fc9146116936e6e6be0366d2afca33c1ab), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "caos33", "CAOS 3.3" )
	ROMX_LOAD( "caos33.853",   0x2000, 0x2000, CRC(ca0fecad) SHA1(20447d27c9aa41a1c7a3d6ad0699edb06a207aa6), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "caos34", "CAOS 3.4" )
	ROMX_LOAD( "caos34.853",   0x2000, 0x2000, CRC(d0245a3e) SHA1(ee9f8e7427b9225ae2cecbcfb625d629ab6a601d), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "pi88ge", "OS PI/88 (yellow/blue)" )
	ROMX_LOAD( "pi88_ge.853",  0x2000, 0x2000, CRC(4bf0cfde) SHA1(b8373a44e4553197e3dd23008168d5214b878837), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "pi88sw", "OS PI/88 (black/white)" )
	ROMX_LOAD( "pi88_sw.853",  0x2000, 0x2000, CRC(f7d2e8fc) SHA1(9b5c068f10ff34bc3253f5b51abad51c8da9dd5d), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "pi88ws", "OS PI/88 (white/blue)" )
	ROMX_LOAD( "pi88_ws.853",  0x2000, 0x2000, CRC(9ef4efbf) SHA1(b8b6f606b76bce9fb7fcd61a14120e5e026b6b6e), ROM_BIOS(6))
ROM_END

ROM_START(kc85_4)
	ROM_REGION(0x2000, "basic", 0)
	ROM_LOAD("basic_c0.854", 0x0000, 0x2000, CRC(dfe34b08) SHA1(c2e3af55c79e049e811607364f88c703b0285e2e))
	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos42", "CAOS 4.2" )
	ROMX_LOAD("caos__c0.854", 0x0000, 0x1000, CRC(57d9ab02) SHA1(774fc2496a59b77c7c392eb5aa46420e7722797e), ROM_BIOS(1))
	ROMX_LOAD("caos__e0.854", 0x2000, 0x2000, CRC(ee273933) SHA1(4300f7ff813c1fb2d5c928dbbf1c9e1fe52a9577), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "caos41", "CAOS 4.1" )
	ROMX_LOAD( "caos41c.854", 0x0000, 0x1000, CRC(c7e1c011) SHA1(acd998e3d9e8f592cd884aafc8ac4d291e40e097), ROM_BIOS(2))
	ROMX_LOAD( "caos41e.854", 0x2000, 0x2000, CRC(60e045e5) SHA1(e19819fb477dcb742a13729a9bf5943d63abe863), ROM_BIOS(2))
ROM_END

ROM_START(kc85_5)
	ROM_REGION(0x8000, "basic", 0)
	ROM_LOAD("basic_c0.855", 0x0000, 0x8000, CRC(0ed9f8b0) SHA1(be2c68a5b461014c57e33a127c3ffb32b0ff2346))

	ROM_REGION(0x4000, "caos", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "caos44", "CAOS 4.4" )
	ROMX_LOAD( "caos__c0.855",0x0000, 0x2000, CRC(f56d5c18) SHA1(2cf8023ee71ca50b92f9f151b7519f59727d1c79), ROM_BIOS(1))
	ROMX_LOAD( "caos__e0.855",0x2000, 0x2000, CRC(1dbc2e6d) SHA1(53ba4394d96e287ff8af01322af1e9879d4e77c4), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "caos43", "CAOS 4.3" )
	ROMX_LOAD( "caos43c.855", 0x0000, 0x2000, CRC(2f0f9eaa) SHA1(5342be5104206d15e7471b094c7749a8a3d708ad), ROM_BIOS(2))
	ROMX_LOAD( "caos43e.855", 0x2000, 0x2000, CRC(b66fc6c3) SHA1(521ac2fbded4148220f8af2d5a5ab99634364079), ROM_BIOS(2))
ROM_END

/*     YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT     INIT  COMPANY   FULLNAME */
COMP( 1987, kc85_2,   0,       0,       kc85_3,  kc85, driver_device,     0,    "VEB Mikroelektronik", "HC900 / KC 85/2", MACHINE_NOT_WORKING)
COMP( 1987, kc85_3,   kc85_2,  0,       kc85_3,  kc85, driver_device,     0,    "VEB Mikroelektronik", "KC 85/3", MACHINE_NOT_WORKING)
COMP( 1989, kc85_4,   kc85_2,  0,       kc85_4,  kc85, driver_device,     0,    "VEB Mikroelektronik", "KC 85/4", MACHINE_NOT_WORKING)
COMP( 1989, kc85_5,   kc85_2,  0,       kc85_5,  kc85, driver_device,     0,    "VEB Mikroelektronik", "KC 85/5", MACHINE_NOT_WORKING)
