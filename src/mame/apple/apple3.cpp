// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/***************************************************************************

    drivers/apple3.c

    Apple ///

    driver by Nathan Woods and R. Belmont

    Special thanks to Chris Smolinski (author of the Sara emulator)
    for his input about this poorly known system.

    Also thanks to Washington Apple Pi for the "Apple III DVD" containing the
    technical manual, schematics, and software.

***************************************************************************/

#include "emu.h"
#include "apple3.h"

#include "bus/a2bus/cards.h"

#include "bus/rs232/rs232.h"

#include "machine/applefdintf.h"
#include "machine/input_merger.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "utf8.h"

void apple3_state::apple3_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(apple3_state::apple3_memory_r), FUNC(apple3_state::apple3_memory_w));
}

void apple3_state::apple3(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 14.318181_MHz_XTAL / 7);        /* 2 MHz */
	m_maincpu->sync_cb().set(FUNC(apple3_state::apple3_sync_w));
	m_maincpu->set_addrmap(AS_PROGRAM, &apple3_state::apple3_map);

	config.set_maximum_quantum(attotime::from_hz(60));

	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq"));
	mainirq.output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	mainirq.output_handler().append(m_via[1], FUNC(via6522_device::write_pa7)).invert(); // this is active low

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL, 910, 0, 560, 262, 0, 192);
	m_screen->set_screen_update(FUNC(apple3_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(m_via[1], FUNC(via6522_device::write_cb1));
	m_screen->screen_vblank().append(m_via[1], FUNC(via6522_device::write_cb2));
	m_screen->screen_vblank().append(FUNC(apple3_state::vbl_w));

	PALETTE(config, m_palette, FUNC(apple3_state::palette_init), 32);

	/* keyboard controller */
	AY3600(config, m_ay3600, 0);
	m_ay3600->x0().set_ioport("X0");
	m_ay3600->x1().set_ioport("X1");
	m_ay3600->x2().set_ioport("X2");
	m_ay3600->x3().set_ioport("X3");
	m_ay3600->x4().set_ioport("X4");
	m_ay3600->x5().set_ioport("X5");
	m_ay3600->x6().set_ioport("X6");
	m_ay3600->x7().set_ioport("X7");
	m_ay3600->x8().set_ioport("X8");
	m_ay3600->shift().set(FUNC(apple3_state::ay3600_shift_r));
	m_ay3600->control().set(FUNC(apple3_state::ay3600_control_r));
	m_ay3600->data_ready().set(FUNC(apple3_state::ay3600_data_ready_w));
	m_ay3600->ako().set(FUNC(apple3_state::ay3600_ako_w));

	/* repeat timer */
	TIMER(config, m_repttimer).configure_generic(FUNC(apple3_state::ay3600_repeat));

	/* slot bus */
	A2BUS(config, m_a2bus, 0);
	m_a2bus->set_space(m_maincpu, AS_PROGRAM);
	m_a2bus->irq_w().set(FUNC(apple3_state::a2bus_irq_w));
	m_a2bus->nmi_w().set(FUNC(apple3_state::a2bus_nmi_w));
	m_a2bus->inh_w().set(FUNC(apple3_state::a2bus_inh_w));
	m_a2bus->dma_w().set_inputline(m_maincpu, INPUT_LINE_HALT);
	A2BUS_SLOT(config, "sl1", m_a2bus, apple3_cards, nullptr);
	A2BUS_SLOT(config, "sl2", m_a2bus, apple3_cards, nullptr);
	A2BUS_SLOT(config, "sl3", m_a2bus, apple3_cards, nullptr);
	A2BUS_SLOT(config, "sl4", m_a2bus, apple3_cards, nullptr);

	/* fdc */
	APPLEIII_FDC(config, m_fdc, 1021800*2);
	applefdintf_device::add_525(config, "0");
	applefdintf_device::add_525(config, "1");
	applefdintf_device::add_525(config, "2");
	applefdintf_device::add_525(config, "3");

	/* softlist for fdc */
	SOFTWARE_LIST(config, "flop525_list").set_original("apple3");

	/* acia */
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(XTAL(1'843'200)); // HACK: The schematic shows an external clock generator but using a XTAL is faster to emulate.
	m_acia->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	// TODO: remove cts kludge from machine/apple3.cpp and come up with a good way of coping with pull up resistors.

	/* paddle */
	TIMER(config, "pdltimer").configure_generic(FUNC(apple3_state::paddle_timer));

	/* rtc */
	MM58167(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_via[1], FUNC(via6522_device::write_ca1));

	/* via */
	MOS6522(config, m_via[0], 14.318181_MHz_XTAL / 14);
	m_via[0]->writepa_handler().set(FUNC(apple3_state::apple3_via_0_out_a));
	m_via[0]->writepb_handler().set(FUNC(apple3_state::apple3_via_0_out_b));
	m_via[0]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	MOS6522(config, m_via[1], 14.318181_MHz_XTAL / 14);
	m_via[1]->writepa_handler().set(FUNC(apple3_state::apple3_via_1_out_a));
	m_via[1]->writepb_handler().set(FUNC(apple3_state::apple3_via_1_out_b));
	m_via[1]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	/* sound */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_bell, 0).add_route(ALL_OUTPUTS, "speaker", 0.99);
	DAC_6BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.125); // 6522.b5(pb0-pb5) + 320k,160k,80k,40k,20k,10k

	TIMER(config, "c040").configure_periodic(FUNC(apple3_state::apple3_c040_tick), attotime::from_hz(2000));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("128K, 512K");
}


static INPUT_PORTS_START( apple3 )
	/*
	  KB3600 Keyboard matrix (KB3600 has custom layout mask ROM, Apple p/n 341-0035)

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  | ESC |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  | TAB |  Q  |  W  |  E  |  R  |  T  |  Y  |  U  |  I  |  O  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  |  A  |  S  |  D  |  F  |  H  |  G  |  J  |  K  |  ;: |  L  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  |  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  <, |  >. |  ?/ |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  |     | KP9 |     | KP8 |     | KP7 |  \| |  += |  0  |  -_ |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  |     | KP6 |     | KP5 |     | KP4 |  `~ |  ]} |  P  | [{  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  |     | KP3 | KP. | KP2 | KP0 | KP1 |RETRN| UP  |BACKS| '"  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |     |     |KPENT|SPACE|     | KP- |RIGHT|DOWN |LEFT |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	*/

/*
    Esc
    0x00

      `    1    2    3    4    5    6    7    8    9    0    -   =   BACKSPACE
    0x38 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x30 0x31 0x2f 0x104

     Tab   Q    W    E    R    T    Y    U   I     O    P    [    ]    \
    0x0a 0x0b 0x0c 0x0d 0x0e 0x0f 0x10 0x11 0x12 0x13 0x3a 0x3b 0x39 0x2e

      A    S    D    F    G    H    J    K    L    ;    '   ENTER
    0x14 0x15 0x16 0x17 0x19 0x18 0x1a 0x1b 0x1d 0x1c 0x105 0x102

      Z    X    C    V    B    N    M   ,     .    /
    0x1e 0x1f 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27

    SPACE   UP    LT    DN    RT   KP-   KP7  KP8  KP9
    0x109  0x103 0x10e 0x10d 0x10c 0x10b 0x2d 0x2b 0x29

    KP4  KP5  KP6   KP1  KP2  KP3  KPEN  KP0   KP.
    0x37 0x35 0x33 0x101 0x3f 0x3d 0x108 0x100 0x3e
*/
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")      PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)  PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)  PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)  PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")      PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)  PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)  PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)  PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)  PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)   PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')

	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)   PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)   PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)   PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)   PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)   PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)   PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)   PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keyb_special")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")  PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHANGED_MEMBER(DEVICE_SELF, apple3_state, apple3_state::keyb_special_changed, 0) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")  PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CHANGED_MEMBER(DEVICE_SELF, apple3_state, apple3_state::keyb_special_changed, 0) PORT_CODE(KEYCODE_F12)

	PORT_START("joy_1_x")      /* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_NAME("P1 Joystick X")
	PORT_MINMAX(0, 0xff) PORT_PLAYER(1)

	PORT_START("joy_1_y")      /* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_NAME("P1 Joystick Y")
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)

	PORT_START("joy_2_x")      /* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_NAME("P2 Joystick X")
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)

	PORT_START("joy_2_y")      /* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_NAME("P2 Joystick Y")
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)

	PORT_START("joy_buttons")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("Joystick 1 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("Joystick 1 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("Joystick 2 Button 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("Joystick 2 Button 2")
INPUT_PORTS_END

ROM_START(apple3)
	ROM_REGION(0x1000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "original", "Apple /// boot ROM")
	ROMX_LOAD( "apple3.rom", 0x0000, 0x1000, CRC(55e8eec9) SHA1(579ee4cd2b208d62915a0aa482ddc2744ff5e967), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "soshd", "Rob Justice SOSHDBOOT")
	ROMX_LOAD( "soshdboot.bin", 0x000000, 0x001000, CRC(fd5ac9e2) SHA1(ba466a54ddb7f618c4f18f344754343c5945b417), ROM_BIOS(1))
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY           FULLNAME */
COMP( 1980, apple3, 0,      0,      apple3,  apple3, apple3_state, init_apple3, "Apple Computer", "Apple ///", MACHINE_SUPPORTS_SAVE )
