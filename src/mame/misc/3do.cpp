// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol
/***************************************************************************

  3do.cpp

  Driver file to handle emulation of the 3DO systems

Hardware descriptions:

Processors:
- 32bit 12.5MHZ RISC CPU (ARM60 - ARM6 core)
- Separate BUS for video refresh updates (VRAM is dual ported)
- Super Fast BUS Speed (50 Megabytes per second)
- Math Co-Processor custom designed by NTG for accelerating fixed-point
  matrix operations (_not_ the ARM FPA)
- Multitaking 32-bit operating system

Resolution:
- 640x480 pixel resolution
- 16.7 million colors

Two accelerated video co-processors:
- 25MHZ clock rate (NTSC), 29.5MHZ clock rate (PAL)
- Capable of producing 9-16 million real pixels per second (36-64 Mpix/sec
  interpolated), distorted, scaled, rotated and texture mapped.
- able to map a rectangular bitmap onto any arbitrary 4-point polygon.
- texturemap source bitmaps can be 1, 2, 4, 6, 8, or 16 bits per pixel and
  are RLE compressed for a maximum combination of both high resolution and
  small storage space.
- supports transparency, translucency, and color-shading effects.

Custom 16bit DSP:
- specifically designed for mixing, manipulating, and synthesizing CD quality
  sound.
- can decompress sound 2:1 or 4:1 on the fly saving memory and bus bandwidth.
- 25MHz clock rate.
- pipelined CISC architecture
- 16bit register size
- 17 separate 16bit DMA channels to and from system memory.
- on chip instruction SRAM and register memory.
- 20bit internal processing.
- special filtering capable of creating effects such as 3D sound.

Sound:
- 16bit stereo sound
- 44.1 kHz sound sampling rate
- Fully support Dolby(tm) Surround Sound

Memory:
- 2 megabytes of DRAM
- 1 megabyte of VRAM (also capable of holding/executing code and data)
- 1 megabyte of ROM
- 32KB battery backed SRAM

CD-ROM drive:
- 320ms access time
- double speed 300kbps data transfer
- 32KB RAM buffer

Ports:
- 2 expansion ports:
  - 1 high-speed 68 pin x 1 AV I/O port (for FMV cartridge)
  - 1 high-speed 30 pin x 1 I/O expansion port
- 1 control port, capable of daisy chaining together up to 8 peripherals

Models:
- Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer (Japan, Asia, North America, Europe)
- Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer (Japan, North America, Europe)
- Goldstar 3DO Interactive Multiplayer (South Korea, North America, Europe)
- Goldstar 3DO ALIVE II (South Korea)
- Sanyo TRY 3DO Interactive Multiplayer (Japan)
- Creative 3DO Blaster - PC Card (ISA)

===========================================================================

Part list of Goldstar 3DO Interactive Multiplayer

- X1 = 50.0000 MHz KONY 95-08 50.0000 KCH089C
- X2 = 59.0000 MHz KONY 95-21 59.0000 KCH089C (NTSC would use 49.09MHz)
- IC303 BOB = 3DO BOB ADG 00919-001-IC 517A4611 - 100 pins
- IC1 ANVIL = 3DO Anvil rev4 00745-004-02 521U5L36 - 304 pins
- IC302 DSP = SONY CXD2500BQ 447HE5V - 80 pins
- IC601 ADAC = BB PCM1710U 9436 GG2553 - 28 pins
- X601 16.934MHz = 16.93440 KONY
- IC101/102/103/104 DRAM = Goldstar GM71C4800AJ70 9520 KOREA - 28 pins
- IC105/106/107/108 VRAM = Toshiba TC528267J-70 9513HBK - 40 pins
- IC3 ROM = Goldstar [202M] GM23C8000AFW-325 9524 - 32 pins
- IC4 SRAM = Goldstar GM76C256ALLFW70 - 28 pins
- IC2 ARM = ARM P60ARMCP 9516C - 100 pins
- IC6 = Philips 74HCT14D 974230Q - 14 pins
- IC301 u-COM = MC68HSC 705C8ACFB 3E20T HLAH9446 - 44 pins

***************************************************************************/

#include "emu.h"
#include "3do.h"

#include "cpu/arm/arm.h"
#include "cpu/arm7/arm7.h"
#include "imagedev/cdromimg.h"


#define X2_CLOCK_PAL    59000000
#define X2_CLOCK_NTSC   49090000
#define X601_CLOCK      XTAL(16'934'400)


void _3do_state::main_mem(address_map &map)
{
	map(0x00000000, 0x001FFFFF).bankrw(m_bank1);                                       /* DRAM */
	map(0x00200000, 0x003FFFFF).ram().share(m_vram);                                   /* VRAM */
	map(0x03000000, 0x030FFFFF).rom().region("bios", 0);                               /* BIOS */
	map(0x03100000, 0x0313FFFF).ram();                                                 /* Brooktree? */
	map(0x03140000, 0x0315FFFF).rw(FUNC(_3do_state::nvarea_r), FUNC(_3do_state::nvarea_w)).umask32(0x000000ff);                /* NVRAM */
	map(0x03180000, 0x031BFFFF).rw(FUNC(_3do_state::slow2_r), FUNC(_3do_state::slow2_w));               /* Slow bus - additional expansion */
	map(0x03200000, 0x0320FFFF).rw(FUNC(_3do_state::svf_r), FUNC(_3do_state::svf_w));                   /* special vram access1 */
	map(0x03300000, 0x033FFFFF).rw(FUNC(_3do_state::madam_r), FUNC(_3do_state::madam_w));               /* address decoder */
	map(0x03400000, 0x034FFFFF).rw(FUNC(_3do_state::clio_r), FUNC(_3do_state::clio_w));                 /* io controller */
}


static INPUT_PORTS_START( 3do )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END

void _3do_state::machine_start()
{
	m_nvram->set_base(&m_nvmem, sizeof(m_nvmem));

	/* configure overlay */
	// TODO: can overlay at 0-0x1FFFFF even be written to, or writes go to dram in any case?
	m_bank1->configure_entry(0, m_dram);
	m_bank1->configure_entry(1, memregion("overlay")->base());

	m_slow2_init();
	m_madam_init();
	m_clio_init();
}

void _3do_state::machine_reset()
{
	/* start with overlay enabled */
	m_bank1->set_entry(1);

	m_clio.cstatbits = 0x01; /* bit 0 = reset of clio caused by power on */
}

void _3do_state::_3do(machine_config &config)
{
	/* Basic machine hardware */
	ARM7_BE(config, m_maincpu, XTAL(50'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &_3do_state::main_mem);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "timer_x16").configure_periodic(FUNC(_3do_state::timer_x16_cb), attotime::from_hz(12000)); // TODO: timing

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(X2_CLOCK_NTSC / 2, 1592, 254, 1534, 263, 22, 262);
	m_screen->set_screen_update(FUNC(_3do_state::screen_update));

	CDROM(config, "cdrom");
}


void _3do_state::_3do_pal(machine_config &config)
{
	/* Basic machine hardware */
	ARM7_BE(config, m_maincpu, XTAL(50'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &_3do_state::main_mem);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "timer_x16").configure_periodic(FUNC(_3do_state::timer_x16_cb), attotime::from_hz(12000)); // TODO: timing

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(X2_CLOCK_PAL / 2, 1592, 254, 1534, 263, 22, 262); // TODO: proper params
	m_screen->set_screen_update(FUNC(_3do_state::screen_update));

	CDROM(config, "cdrom");
}

#if 0
#define NTSC_BIOS \
	ROM_REGION32_BE( 0x200000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "panafz10", "Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz10.bin", 0x000000, 0x100000, CRC(58242cee) SHA1(3c912300775d1ad730dc35757e279c274c0acaad), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "goldstar", "Goldstar 3DO Interactive Multiplayer v1.01m" ) \
	ROMX_LOAD( "goldstar.bin", 0x000000, 0x100000, CRC(b6f5028b) SHA1(c4a2e5336f77fb5f743de1eea2cda43675ee2de7), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "panafz1", "Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz1.bin", 0x000000, 0x100000, CRC(c8c8ff89) SHA1(34bf189111295f74d7b7dfc1f304d98b8d36325a), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "gsalive2", "Goldstar 3DO Alive II" ) \
	ROMX_LOAD( "gsalive2.bin", 0x000000, 0x100000, NO_DUMP, ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "sanyotry", "Sanyo TRY 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "sanyotry.bin", 0x000000, 0x100000, CRC(d5cbc509) SHA1(b01c53da256dde43ffec4ad3fc3adfa8d635e943), ROM_BIOS(4) )
#else
#define NTSC_BIOS \
	ROM_REGION32_BE( 0x200000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "panafz10", "Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz10.bin", 0x000000, 0x100000, CRC(58242cee) SHA1(3c912300775d1ad730dc35757e279c274c0acaad), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "goldstar", "Goldstar 3DO Interactive Multiplayer v1.01m" ) \
	ROMX_LOAD( "goldstar.bin", 0x000000, 0x100000, CRC(b6f5028b) SHA1(c4a2e5336f77fb5f743de1eea2cda43675ee2de7), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "panafz1", "Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "panafz1.bin", 0x000000, 0x100000, CRC(c8c8ff89) SHA1(34bf189111295f74d7b7dfc1f304d98b8d36325a), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "sanyotry", "Sanyo TRY 3DO Interactive Multiplayer" ) \
	ROMX_LOAD( "sanyotry.bin", 0x000000, 0x100000, CRC(d5cbc509) SHA1(b01c53da256dde43ffec4ad3fc3adfa8d635e943), ROM_BIOS(3) ) \
	ROM_REGION32_BE( 0x200000, "overlay", 0 ) \
	ROM_COPY( "bios", 0, 0, 0x200000 )
#endif

ROM_START(3do)
	NTSC_BIOS
ROM_END

ROM_START(3dobios)
	NTSC_BIOS
ROM_END

ROM_START(3do_pal)
	ROM_REGION32_BE( 0x200000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "panafz10", "Panasonic FZ-10 R.E.A.L. 3DO Interactive Multiplayer" )
	ROMX_LOAD( "panafz10.bin", 0x000000, 0x100000, CRC(58242cee) SHA1(3c912300775d1ad730dc35757e279c274c0acaad), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "goldstar", "Goldstar 3DO Interactive Multiplayer v1.01m" )
	ROMX_LOAD( "goldstar.bin", 0x000000, 0x100000, CRC(b6f5028b) SHA1(c4a2e5336f77fb5f743de1eea2cda43675ee2de7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "panafz1", "Panasonic FZ-1 R.E.A.L. 3DO Interactive Multiplayer" )
	ROMX_LOAD( "panafz1.bin", 0x000000, 0x100000, CRC(c8c8ff89) SHA1(34bf189111295f74d7b7dfc1f304d98b8d36325a), ROM_BIOS(2) )

	ROM_REGION32_BE( 0x200000, "overlay", 0 )
	ROM_COPY( "bios", 0, 0, 0x200000 )
ROM_END

ROM_START(orbatak)
	NTSC_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "orbatak", 0, SHA1(25cb3b889cf09dbe5faf2b0ca4aae5e03453da00) )
ROM_END

#define ALG_BIOS \
	ROM_REGION32_BE( 0x200000, "bios", 0 ) \
	/* TC544000AF-150, 1xxxxxxxxxxxxxxxxxx = 0xFF */ \
	ROM_LOAD( "saot_rom2.bin", 0x000000, 0x80000, CRC(b832da9a) SHA1(520d3d1b5897800af47f92efd2444a26b7a7dead) )  \
	ROM_REGION32_BE( 0x200000, "overlay", 0 ) \
	ROM_COPY( "bios", 0, 0, 0x200000 )


ROM_START(alg3do)
	ALG_BIOS
ROM_END


ROM_START(md23do)
	ALG_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "mad dog ii", 0, SHA1(0117c1fd279f42e942648ca55fa75dd45da37a4f) )
ROM_END

ROM_START(sht3do)
	ALG_BIOS

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "shootout at old tucson", 0, SHA1(bd42213c6b460b5b6153a8b2b41d0a114171e86e) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   STATE       INIT        COMPANY            FULLNAME      FLAGS */
// console section
CONS( 1993, 3do,     0,      0,      _3do,       3do,    _3do_state, empty_init, "The 3DO Company", "3DO (NTSC)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 1993, 3do_pal, 3do,    0,      _3do_pal,   3do,    _3do_state, empty_init, "The 3DO Company", "3DO (PAL)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

/*    YEAR  NAME     PARENT   MACHINE  INPUT  STATE       INIT        MONITOR   COMPANY                 FULLNAME               FLAGS */
// Misc 3do Arcade games
GAME( 1993, 3dobios, 0,       _3do,    3do,   _3do_state, empty_init, ROT0,     "The 3DO Company",      "3DO BIOS",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_BIOS_ROOT )

GAME( 1995, orbatak, 3dobios, _3do,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Orbatak (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// Beavis and Butthead (prototype)


// American Laser Games uses its own BIOS (with additional protection according to serial output?)
GAME( 1993, alg3do, 0,       _3do,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games / The 3DO Company", "ALG 3DO BIOS",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_BIOS_ROOT )

GAME( 199?, md23do,  alg3do, _3do,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Mad Dog II: The Lost Gold (3DO hardware)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, sht3do,  alg3do, _3do,    3do,   _3do_state, empty_init, ROT0,     "American Laser Games", "Shootout at Old Tucson (3DO hardware)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

