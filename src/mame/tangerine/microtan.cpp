// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller,Nigel Barnes
/******************************************************************************
 *  Microtan 65
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http://www.geoff.org.uk/microtan/index.htm
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://oric.free.fr/microtan.html
 *
 *  Microtan65 memory map
 *
 *  range     short     description
 *  0000-01ff SYSRAM    system ram
 *                      0000-003f variables
 *                      0040-00ff basic
 *                      0100-01ff stack
 *  0200-03ff VIDEORAM  display
 *  0400-afff RAM       main memory
 *  bc00-bc01 AY8912-0  sound chip #0
 *  bc02-bc03 AY8912-1  sound chip #1
 *  bc04      SPACEINV  space invasion sound (?)
 *  bfc0-bfcf VIA6522-0 VIA 6522 #0
 *  bfd0-bfd3 SIO       serial i/o
 *  bfe0-bfef VIA6522-1 VIA 6522 #1
 *  bff0      GFX_KBD   R: chunky graphics on W: reset KBD interrupt
 *  bff1      NMI       W: start delayed NMI
 *  bff2      HEX       W: hex. keypad column
 *  bff3      KBD_GFX   R: ASCII KBD / hex. keypad row W: chunky graphics off
 *  c000-e7ff BASIC     BASIC Interpreter ROM
 *  f000-f7ff XBUG      XBUG ROM
 *  f800-ffff TANBUG    TANBUG ROM
 *
 *  Tanbug commands:
 *  B         Set breakpoint
 *  C         copy (move) memory block
 *  G         Go
 *  L         Hex dump
 *  M         Modify memory
 *  N         Exit single-step mode
 *  O         Hex calculator
 *  P         Step once
 *  R         Register examine/modify
 *  S         Enter single-step mode
 *  BAS       Start BASIC
 *  WAR       Re-enter BASIC (warm start)
 *
 *  Ralbug commnds:
 *  COP       Copy
 *  MEM       Modify memory
 *  SYW       Warm start to DOS system
 *  SYC       Cold start to DOS system
 *  SYB       Boot DOS system
 *  LIS       List a line of memory
 *  JMP       Go to address that follows
 *  JSR       JSR to following address
 *  JMI       Jump indirect
 *  JSI       JSR indirect
 *  TES       Memory test
 *  CLR       Clear screen
 *  COL       Colour change
 *  HEL       Help display menu
 *  VEC       List vector jump table
 *  FIL       Fill Memory
 *  CUR       Cursor move
 *  ASC       ASCII table display
 *  SYN       Command syntax
 *  HOM       Home cursor
 *  WAR       Warm start $0003
 *  STA       Start $0000
 *  EXP       Expand monitor
 *  BLE       Keyboard bleep
 *  KIL       Kill off the cache
 *
 *****************************************************************************/

#include "emu.h"
#include "microtan.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#include "utf8.h"


void microtan_state::mt65_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_tanbus, FUNC(tanbus_device::read), FUNC(tanbus_device::write));
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x03ff).ram().w(FUNC(microtan_state::videoram_w)).share(m_videoram);
	map(0xbff0, 0xbfff).rw(FUNC(microtan_state::bffx_r), FUNC(microtan_state::bffx_w));
	map(0xf800, 0xffff).rom().region("maincpu", 0);
}

void microtan_state::spinv_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x03ff).ram().w(FUNC(microtan_state::videoram_w)).share(m_videoram);
	map(0xbff0, 0xbfff).rw(FUNC(microtan_state::bffx_r), FUNC(microtan_state::bffx_w));
	map(0xf800, 0xffff).rom().region("maincpu", 0);
}

void mt6809_state::mt6809_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_tanbus, FUNC(tanbus_device::read), FUNC(tanbus_device::write));
	map(0x0000, 0x03ff).ram();
	map(0xbff0, 0xbfff).rw(FUNC(mt6809_state::bffx_r), FUNC(mt6809_state::bffx_w));
	map(0xf800, 0xffff).rom().region("maincpu", 0);
}


void microtan_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	if (state)
	{
		machine().schedule_soft_reset();
	}
}

void mt6809_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	if (state)
	{
		machine().schedule_soft_reset();
	}
}


static const gfx_layout char_layout =
{
	8, 16,      /* 8 x 16 graphics */
	256,        /* 256 codes */
	1,          /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8 * 16      /* code takes 8 times 16 bits */
};

static GFXDECODE_START( gfx_microtan )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0, 1 )
GFXDECODE_END


void microtan_state::mt65(machine_config &config)
{
	/* Microtan 65 CPU board */
	M6502(config, m_maincpu, 6_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &microtan_state::mt65_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MICROTAN_KBD_SLOT(config, m_keyboard, microtan_kbd_devices, "mt009");
	m_keyboard->strobe_handler().set(FUNC(microtan_state::kbd_int));
	m_keyboard->reset_handler().set(FUNC(microtan_state::trigger_reset));

	/* video hardware - include overscan */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6_MHz_XTAL, 384, 0, 32*8, 312, 0, 16*16);
	screen.set_screen_update(FUNC(microtan_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_microtan);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Microtan Motherboard */
	TANBUS(config, m_tanbus, 6_MHz_XTAL);
	m_tanbus->out_irq_callback().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_TANBUS>));
	m_tanbus->out_so_callback().set_inputline(m_maincpu, M6502_SET_OVERFLOW);
	m_tanbus->out_nmi_callback().set_inputline(m_maincpu, M6502_NMI_LINE);
	m_tanbus->out_pgm_callback().set(FUNC(microtan_state::pgm_chargen_w));

	TANBUS_SLOT(config, "tanbus:tanex", -1, tanex_devices, "tanex");
	TANBUS_SLOT(config, "tanbus:0", 0, tanbus_devices, "tanram");
	TANBUS_SLOT(config, "tanbus:1", 1, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:2", 2, tanbus_devices, "bullsnd");
	TANBUS_SLOT(config, "tanbus:3", 3, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:4", 4, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:5", 5, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:6", 6, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:7", 7, tanbus_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:exp", 8, tanbus_devices, nullptr);

	/* snapshot/quickload */
	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "dmp,m65"));
	snapshot.set_load_callback(FUNC(microtan_state::snapshot_cb));
	snapshot.set_interface("mt65_snap");
	QUICKLOAD(config, "quickload", "hex").set_load_callback(FUNC(microtan_state::quickload_cb));

	/* software lists */
	SOFTWARE_LIST(config, "rom_list").set_original("mt65_rom");
	SOFTWARE_LIST(config, "cass_list").set_original("mt65_cass");
	//SOFTWARE_LIST(config, "flop_list").set_original("mt65_flop").set_filter("6502");
	SOFTWARE_LIST(config, "snap_list").set_original("mt65_snap");
}

void microtan_state::micron(machine_config &config)
{
	mt65(config);

	/* Microtan Mini-Motherboard connects MT65 CPU card and TANEX, no extra slots */
	config.device_remove("tanbus:0");
	config.device_remove("tanbus:1");
	config.device_remove("tanbus:2");
	config.device_remove("tanbus:3");
	config.device_remove("tanbus:4");
	config.device_remove("tanbus:5");
	config.device_remove("tanbus:6");
	config.device_remove("tanbus:7");
	config.device_remove("tanbus:exp");
}

void microtan_state::spinveti(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 6_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &microtan_state::spinv_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MICROTAN_KBD_SLOT(config, m_keyboard, microtan_kbd_devices, "spinveti");
	m_keyboard->strobe_handler().set(FUNC(microtan_state::kbd_int));
	m_keyboard->reset_handler().set(FUNC(microtan_state::trigger_reset));

	/* video hardware - include overscan */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6_MHz_XTAL, 384, 0, 32*8, 312, 0, 16*16);
	screen.set_screen_update(FUNC(microtan_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_microtan);

	PALETTE(config, "palette", palette_device::MONOCHROME);
}


static DEVICE_INPUT_DEFAULTS_START(ra32k_def_ram0000)
	/* 32K RAM configured 0x0000-0x7fff */
	DEVICE_INPUT_DEFAULTS("DSW1", 0x01, 0x01) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("DSW1", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x80, 0x80)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x01, 0x01) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("DSW2", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x80, 0x80)
	DEVICE_INPUT_DEFAULTS("DSW3", 0x0f, 0x0f) // start address $0000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x01, 0x01) // paged
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(ra32k_def_ram8000)
	/* 32K RAM configured 0x8000-0xffff */
	DEVICE_INPUT_DEFAULTS("DSW1", 0x01, 0x01)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x80, 0x00) // disable block $F800
	DEVICE_INPUT_DEFAULTS("DSW2", 0x01, 0x01) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("DSW2", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x80, 0x80)
	DEVICE_INPUT_DEFAULTS("DSW3", 0x0f, 0x0d) // start address $8000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x01, 0x00) // permanent
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(ra32k_def_rom0000)
	/* 8K EPROM configured 0x2000-0x3fff */
	DEVICE_INPUT_DEFAULTS("DSW1", 0x01, 0x01) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("DSW1", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW1", 0x80, 0x80)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x01, 0x01) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("DSW2", 0x02, 0x02)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x04, 0x04)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x08, 0x08)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x10, 0x10)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x20, 0x20)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x40, 0x40)
	DEVICE_INPUT_DEFAULTS("DSW2", 0x80, 0x80)
	DEVICE_INPUT_DEFAULTS("DSW3", 0x0f, 0x0f) // start address $0000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x01, 0x01) // paged
DEVICE_INPUT_DEFAULTS_END

void mt6809_state::mt6809(machine_config &config)
{
	/* Ralph Allen 6809 CPU Board */
	MC6809(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mt6809_state::mt6809_map);

	MICROTAN_KBD_SLOT(config, m_keyboard, microtan_kbd_devices, "mt009");
	m_keyboard->strobe_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_keyboard->reset_handler().set(FUNC(mt6809_state::trigger_reset));

	/* Microtan Motherboard */
	TANBUS(config, m_tanbus, 6_MHz_XTAL);
	m_tanbus->out_irq_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);
	m_tanbus->out_nmi_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	TANBUS_SLOT(config, "tanbus:tanex", -1, tanex_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:0", 0, tanbus6809_devices, "ra32kram").set_option_device_input_defaults("ra32kram", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_ram0000));
	TANBUS_SLOT(config, "tanbus:1", 1, tanbus6809_devices, "ra32kram").set_option_device_input_defaults("ra32kram", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_ram8000));
	TANBUS_SLOT(config, "tanbus:2", 2, tanbus6809_devices, "ravdu");
	TANBUS_SLOT(config, "tanbus:3", 3, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:4", 4, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:5", 5, tanbus6809_devices, "radisc");
	TANBUS_SLOT(config, "tanbus:6", 6, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:7", 7, tanbus6809_devices, "ra32krom").set_option_device_input_defaults("ra32krom", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_rom0000));
	TANBUS_SLOT(config, "tanbus:exp", 8, tanbus6809_devices, nullptr);

	/* software lists */
	//SOFTWARE_LIST(config, "flop_list").set_original("mt65_flop").set_filter("6809");
}


ROM_START( mt65 )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_DEFAULT_BIOS("tanbug23")
	/* Microtan 65 (MT0016 Iss 1) */
	ROM_SYSTEM_BIOS(0, "tanbug23", "TANBUG V2.3")
	ROMX_LOAD("tanbug23.k3", 0x0000, 0x0800, CRC(7f29845d) SHA1(de277e942eefa11a9a6defe3d7d03071d5100b68), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "tanbug23p", "TANBUG V2.3 (patched)")
	ROMX_LOAD("tanbug_2.rom", 0x0000, 0x0400, CRC(7e215313) SHA1(c8fb3d33ce2beaf624dc75ec57d34c216b086274), ROM_BIOS(1))
	ROMX_LOAD("tanbug.rom",   0x0400, 0x0400, CRC(c8221d9e) SHA1(c7fe4c174523aaaab30be7a8c9baf2bc08b33968), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "tanbug31", "TANBUG V3.1 (TANDOS)")
	ROMX_LOAD("tanbug31.k3", 0x0000, 0x0800, CRC(5943e427) SHA1(55fe645363cd5f2015a537be6f7a00de33d1bea5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "tanbug3b", "TANBUG V.3B (MPVDU)")
	ROMX_LOAD("tanbug3b.k3", 0x0000, 0x0800, CRC(5b49f861) SHA1(b37cddf97b6324ab0647479b5b013a9b3d47ddda), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "tugbug11", "TUGBUG V1.1 (VID8082)")
	ROMX_LOAD("tugbug11.k3", 0x0000, 0x0800, CRC(23b02439) SHA1(9e56fbd6c07b1dc5c10e7b574a5ea26405781a3d), ROM_BIOS(4))

	/* DM8678BWF (upper case) and DM8678CAE (lower case) */
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
ROM_END

ROM_START( micron )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_DEFAULT_BIOS("tanbug1")
	/* MT65 (Iss 0) */
	ROM_SYSTEM_BIOS(0, "tanbug1", "TANBUG V1")
	ROMX_LOAD("tanbug1.g2", 0x0000, 0x0400, CRC(6f45aaca) SHA1(c1a020d86830ee475ee75c847e98f7a02d467659), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROM_RELOAD(0x0400, 0x0400)
	ROMX_LOAD("tanbug1.h2", 0x0000, 0x0400, CRC(1ccd6b8f) SHA1(49784749599255458ba5513d8c2fc58c4df39515), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROM_RELOAD(0x0400, 0x0400)

	/* DM8678BWF (upper case) and DM8678CAE (lower case) */
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
ROM_END

ROM_START( spinveti )
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("spaceinv.ic20", 0x0000, 0x0800, CRC(c0b074e0) SHA1(464be4082bd60d825c0c1e7ae033d7ab095c8e70))

	/* DM8678BWF only */
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom",  0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
ROM_END

ROM_START( mt6809 )
	/* 6809 CPU Card (Ralph Allen Engineering) */
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ralbug14", "RALBUG V1.4")
	ROMX_LOAD("ralbug14.rom", 0x0000, 0x0800, CRC(8bbc87d8) SHA1(3d53a6ffd4a8d7c8edf4f2e23946011ed29146ce), ROM_BIOS(0))
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY          FULLNAME                  FLAGS
COMP( 1979, mt65,     0,      0,      mt65,     0,        microtan_state, empty_init,    "Tangerine",     "Microtan 65",            MACHINE_NO_SOUND_HW )
COMP( 1980, micron,   mt65,   0,      micron,   0,        microtan_state, empty_init,    "Tangerine",     "Micron",                 MACHINE_NO_SOUND_HW )
COMP( 1980, spinveti, 0,      0,      spinveti, 0,        microtan_state, empty_init,    "Tangerine/ETI", "Space Invasion (ETI)",   MACHINE_NO_SOUND )
COMP( 1984, mt6809,   mt65,   0,      mt6809,   0,        mt6809_state,   empty_init,    "Tangerine",     "Microtan 6809 System",   MACHINE_NO_SOUND_HW )
