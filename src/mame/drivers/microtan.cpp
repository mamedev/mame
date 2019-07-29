// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller,Nigel Barnes
/******************************************************************************
 *  Microtan 65
 *
 *  system driver
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
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
 *  BAS       Start BASIC (You need to choose maximum memory via dipswitches)
 *  WAR       Re-enter BASIC (warm start)
 *
 *****************************************************************************/

#include "emu.h"
#include "includes/microtan.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"


void microtan_state::mt65_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_tanbus, FUNC(tanbus_device::read), FUNC(tanbus_device::write));
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x03ff).ram().w(FUNC(microtan_state::videoram_w)).share(m_videoram);
	map(0xbff0, 0xbfff).rw(FUNC(microtan_state::bffx_r), FUNC(microtan_state::bffx_w));
	map(0xf800, 0xffff).rom();
}

void microtan_state::spinv_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x03ff).ram().w(FUNC(microtan_state::videoram_w)).share(m_videoram);
	map(0xbc04, 0xbc04).rw(FUNC(microtan_state::sound_r), FUNC(microtan_state::sound_w));
	map(0xbff0, 0xbfff).rw(FUNC(microtan_state::bffx_r), FUNC(microtan_state::bffx_w));
	map(0xe800, 0xefff).rom().mirror(0x1000);
}

void mt6809_state::mt6809_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_tanbus, FUNC(tanbus_device::read), FUNC(tanbus_device::write));
	map(0xdc00, 0xdfff).ram();
	map(0xdfd8, 0xdfd8).r(FUNC(mt6809_state::keyboard_r));
	map(0xf800, 0xffff).rom();
}


INPUT_CHANGED_MEMBER(microtan_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);

	if (newval)
	{
		machine().schedule_soft_reset();
	}
}

static INPUT_PORTS_START( microtan )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "Input")
	PORT_CONFSETTING(0x00, "ASCII Keyboard")
	PORT_CONFSETTING(0x01, "Hex Keypad")

	PORT_START("KBD0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 '") PORT_CODE(KEYCODE_7)

	PORT_START("KBD1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 (") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK SPACE") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)

	PORT_START("KBD2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("KBD3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("KBD4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("KBD5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\ |") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT (L)") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("KBD6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT (R)") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KBD7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LINE FEED")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- (KP)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", (KP)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER (KP)") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". (KP)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 (KP)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 (KP)") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("KBD8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 (KP)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 (KP)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 (KP)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 (KP)") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 (KP)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 (KP)") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 (KP)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 (KP)") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("KPAD0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8 P") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C M") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KPAD1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5 O") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9 ESC") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D G") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("LF DEL") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KPAD2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6 C") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E S") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CR SP") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KPAD3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3 '") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7 R") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B L") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F N") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("BRK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, microtan_state, trigger_reset, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( spinveti )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x02, "Input")
	PORT_CONFSETTING(0x02, "ETI Keypad")
	PORT_CONFNAME(0x60, 0x20, "Difficulty")
	PORT_CONFSETTING(0x00, "Easy")
	PORT_CONFSETTING(0x20, "Normal")
	PORT_CONFSETTING(0x40, "Hard")
	PORT_CONFSETTING(0x60, "Hardest")

	PORT_START("KEYPAD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Play") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fire") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Hold") PORT_CODE(KEYCODE_H)

	PORT_START("BRK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, microtan_state, trigger_reset, 0)
INPUT_PORTS_END

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

	TIMER(config, "kbd_timer").configure_periodic(FUNC(microtan_state::kbd_scan), attotime::from_hz(45));

	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

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
	snapshot.set_load_callback(FUNC(microtan_state::snapshot_cb), this);
	snapshot.set_interface("mt65_snap");
	QUICKLOAD(config, "quickload", "hex").set_load_callback(FUNC(microtan_state::quickload_cb), this);

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

	/* video hardware - include overscan */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6_MHz_XTAL, 384, 0, 32*8, 312, 0, 16*16);
	screen.set_screen_update(FUNC(microtan_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_microtan);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* TODO: Sound hardware */
}


static DEVICE_INPUT_DEFAULTS_START(ra32k_def_ram0000)
	/* 32K RAM configured 0x0000-0x7fff */
	DEVICE_INPUT_DEFAULTS("SW1", 0xff, 0xff) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("SW2", 0xff, 0xff) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("SW3", 0x0f, 0x0f) // start address $0000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x03, 0x01)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(ra32k_def_ram8000)
	/* 32K RAM configured 0x8000-0xffff */
	DEVICE_INPUT_DEFAULTS("SW1", 0xff, 0x7f) // disable block $F800
	DEVICE_INPUT_DEFAULTS("SW2", 0xff, 0xff) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("SW3", 0x0f, 0x0d) // start address $8000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x03, 0x00)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(ra32k_def_rom2000)
	/* 8K EPROM configured 0x2000-0x3fff */
	DEVICE_INPUT_DEFAULTS("SW1", 0xff, 0xff) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("SW2", 0xff, 0xff) // all blocks enabled
	DEVICE_INPUT_DEFAULTS("SW3", 0x0f, 0x04) // start address $2000
	DEVICE_INPUT_DEFAULTS("LNK1", 0x03, 0x02)
DEVICE_INPUT_DEFAULTS_END

void mt6809_state::mt6809(machine_config &config)
{
	/* Ralph Allen 6809 CPU Board */
	MC6809(config, m_maincpu, 6_MHz_XTAL / 4); // TODO: verify divider
	m_maincpu->set_addrmap(AS_PROGRAM, &mt6809_state::mt6809_map);

	TIMER(config, "kbd_timer").configure_periodic(FUNC(microtan_state::kbd_scan), attotime::from_hz(45));

	INPUT_MERGER_ANY_HIGH(config, m_irq_line).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	/* Microtan Motherboard */
	TANBUS(config, m_tanbus, 6_MHz_XTAL);
	m_tanbus->out_irq_callback().set(m_irq_line, FUNC(input_merger_device::in_w<IRQ_TANBUS>));
	m_tanbus->out_nmi_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	TANBUS_SLOT(config, "tanbus:tanex", -1, tanex_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:0", 0, tanbus6809_devices, "ra32k").set_option_device_input_defaults("ra32k", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_ram0000));
	TANBUS_SLOT(config, "tanbus:1", 1, tanbus6809_devices, "ra32k").set_option_device_input_defaults("ra32k", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_ram8000));
	TANBUS_SLOT(config, "tanbus:2", 2, tanbus6809_devices, "ravdu");
	TANBUS_SLOT(config, "tanbus:3", 3, tanbus6809_devices, "radisc");
	TANBUS_SLOT(config, "tanbus:4", 4, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:5", 5, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:6", 6, tanbus6809_devices, nullptr);
	TANBUS_SLOT(config, "tanbus:7", 7, tanbus6809_devices, "ra32k").set_option_device_input_defaults("ra32k", DEVICE_INPUT_DEFAULTS_NAME(ra32k_def_rom2000));
	TANBUS_SLOT(config, "tanbus:exp", 8, tanbus6809_devices, nullptr);

	/* software lists */
	//SOFTWARE_LIST(config, "flop_list").set_original("mt65_flop").set_filter("6809");
}


ROM_START( mt65 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("tanbug23")
	/* Microtan 65 (MT0016 Iss 1) */
	ROM_SYSTEM_BIOS(0, "tanbug23", "TANBUG V2.3")
	ROMX_LOAD("tanbug23.k3", 0xf800, 0x0800, CRC(7f29845d) SHA1(de277e942eefa11a9a6defe3d7d03071d5100b68), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "tanbug23p", "TANBUG V2.3 (patched)")
	ROMX_LOAD("tanbug_2.rom", 0xf800, 0x0400, CRC(7e215313) SHA1(c8fb3d33ce2beaf624dc75ec57d34c216b086274), ROM_BIOS(1))
	ROMX_LOAD("tanbug.rom", 0xfc00, 0x0400, CRC(c8221d9e) SHA1(c7fe4c174523aaaab30be7a8c9baf2bc08b33968), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "tanbug31", "TANBUG V3.1 (TANDOS)")
	ROMX_LOAD("tanbug31.k3", 0xf800, 0x0800, CRC(5943e427) SHA1(55fe645363cd5f2015a537be6f7a00de33d1bea5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "tanbug3b", "TANBUG V.3B (MPVDU)")
	ROMX_LOAD("tanbug3b.k3", 0xf800, 0x0800, CRC(5b49f861) SHA1(b37cddf97b6324ab0647479b5b013a9b3d47ddda), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "tugbug11", "TUGBUG V1.1 (VID8082)")
	ROMX_LOAD("tugbug11.k3", 0xf800, 0x0800, CRC(23b02439) SHA1(9e56fbd6c07b1dc5c10e7b574a5ea26405781a3d), ROM_BIOS(4))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
	// initialized in init_gfx2
ROM_END

ROM_START( micron )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("tanbug1")
	/* MT65 (Iss 0) */
	ROM_SYSTEM_BIOS(0, "tanbug1", "TANBUG V1")
	ROMX_LOAD("tanbug1.g2", 0xf800, 0x0400, CRC(6f45aaca) SHA1(c1a020d86830ee475ee75c847e98f7a02d467659), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROM_RELOAD(0xfc00, 0x0400)
	ROMX_LOAD("tanbug1.h2", 0xf800, 0x0400, CRC(1ccd6b8f) SHA1(49784749599255458ba5513d8c2fc58c4df39515), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROM_RELOAD(0xfc00, 0x0400)

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
	// initialized in init_gfx2
ROM_END

ROM_START( spinveti )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spaceinv.ic20", 0xe800, 0x0800, CRC(3e235077) SHA1(ae7d2788125d01a3c5d57ab0d992bd708abd6ade))

	ROM_REGION(0x1000, "gfx1", 0)
	ROM_LOAD("charset.rom",  0x0000, 0x0800, CRC(3b3c5360) SHA1(a3a2f74149107f8b8f35b15069c71f3aa843d12f))
	ROM_RELOAD(0x0800, 0x0800)

	ROM_REGION(0x1000, "gfx2", ROMREGION_ERASEFF)
	// initialized in init_gfx2
ROM_END

ROM_START( mt6809 )
	/* 6809 CPU Card (Ralph Allen Engineering) */
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "ralbug14", "RALBUG V1.4")
	ROMX_LOAD("ralbug14.rom", 0xf800, 0x0800, CRC(8bbc87d8) SHA1(3d53a6ffd4a8d7c8edf4f2e23946011ed29146ce), ROM_BIOS(0))
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY          FULLNAME        FLAGS
COMP( 1979, mt65,     0,      0,      mt65,     microtan, microtan_state, init_microtan, "Tangerine",     "Microtan 65",            MACHINE_NO_SOUND_HW )
COMP( 1980, micron,   mt65,   0,      micron,   microtan, microtan_state, init_microtan, "Tangerine",     "Micron",                 MACHINE_NO_SOUND_HW )
COMP( 1980, spinveti, mt65,   0,      spinveti, spinveti, microtan_state, init_gfx2,     "Tangerine/ETI", "Space Invasion (ETI)",   MACHINE_NO_SOUND )
COMP( 1984, mt6809,   mt65,   0,      mt6809,   microtan, mt6809_state,   empty_init,    "Tangerine",     "Microtan 6809 System",   MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
