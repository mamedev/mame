// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Nathan Woods
/*******************************************************************************

DAI driver by Krzysztof Strzecha and Nathan Woods

What's new:
-----------
21.05.2004  TMS5501 fixes. Debug code cleanups.
06.03.2004  Stack overflow interrupt added.
05.09.2003  Random number generator added. Few video hardware bugs fixed.
        Fixed few I8080 instructions, making much more BASIC games playable.

Notes on emulation status and to do list:
-----------------------------------------
1. A lot to do. Too much to list.

DAI technical information
==========================

CPU:
----
    I8080 2MHz


Memory map:
-----------
    0000-bfff RAM
    c000-dfff ROM (non-switchable)
    e000-efff ROM (4 switchable banks)
    f000-f7ff ROM extension (optional)
    f800-f8ff SRAM (stack)
    f900-ffff I/O
        f900-faff spare
        fb00-fbff AMD9511 math chip (optional)
        fc00-fcff 8253 programmable interval timer
        fd00-fdff discrete devices
        fe00-feff 8255 PIO (DCE bus)
        ff00-ffff timer + 5501 interrupt controller

Interrupts:
-----------


Keyboard:
---------


Video:
-----


Sound:
------


Timings:
--------


*******************************************************************************/

#include "emu.h"
#include "dai.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/* memory w/r functions */
void dai_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("mainram");
	map(0xc000, 0xdfff).rom().region("maincpu",0);
	map(0xe000, 0xefff).bankr("bank2");
	map(0xf000, 0xf7ff).w(FUNC(dai_state::stack_interrupt_circuit_w));
	map(0xf800, 0xf8ff).ram();
	map(0xfb00, 0xfbff).rw(FUNC(dai_state::amd9511_r), FUNC(dai_state::amd9511_w));
	map(0xfc00, 0xfcff).rw(FUNC(dai_state::pit_r), FUNC(dai_state::pit_w)); // .rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xfd00, 0xfdff).rw(FUNC(dai_state::io_discrete_devices_r), FUNC(dai_state::io_discrete_devices_w));
	map(0xfe00, 0xfeff).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xff00, 0xff0f).mirror(0xf0).m(m_tms5501, FUNC(tms5501_device::io_map));
}


/* keyboard input */
static INPUT_PORTS_START (dai)
	PORT_START("IN0") /* [0] - port ff07 bit 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('H') PORT_CHAR('h')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('P') PORT_CHAR('p')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('X') PORT_CHAR('x')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN1") /* [1] - port ff07 bit 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('A') PORT_CHAR('a')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('I') PORT_CHAR('i')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q') PORT_CHAR('q')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('Y') PORT_CHAR('y')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN2") /* [2] - port ff07 bit 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('B') PORT_CHAR('b')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('J') PORT_CHAR('j')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('R') PORT_CHAR('r')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('Z') PORT_CHAR('z')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN3") /* [3] - port ff07 bit 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('C') PORT_CHAR('c')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('K') PORT_CHAR('k')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('S') PORT_CHAR('s')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('[') PORT_CHAR(']')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN4") /* [4] - port ff07 bit 4 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('D') PORT_CHAR('d')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('L') PORT_CHAR('l')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('T') PORT_CHAR('t')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('^') PORT_CHAR('~')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR('\t')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN5") /* [5] - port ff07 bit 5 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('-') PORT_CHAR('=')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('E') PORT_CHAR('e')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('M') PORT_CHAR('m')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('U') PORT_CHAR('u')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN6") /* [6] - port ff07 bit 6 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('F') PORT_CHAR('f')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('N') PORT_CHAR('n')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('V') PORT_CHAR('v')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Rept") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN7") /* [7] - port ff07 bit 7 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('G') PORT_CHAR('g')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('O') PORT_CHAR('o')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('W') PORT_CHAR('w')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Char del") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("IN8") /* [8] */
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
		PORT_BIT(0xcb, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

/* F4 Character Displayer */
static const gfx_layout dai_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_dai )
	GFXDECODE_ENTRY( "chargen", 0x0000, dai_charlayout, 0, 8 )
GFXDECODE_END

/* machine definition */
void dai_state::dai(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dai_state::mem_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(dai_state::int_ack));
	config.set_maximum_quantum(attotime::from_hz(60));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(2000000);
	m_pit->out_handler<0>().set(m_sound, FUNC(dai_sound_device::set_input_ch0));
	m_pit->set_clk<1>(2000000);
	m_pit->out_handler<1>().set(m_sound, FUNC(dai_sound_device::set_input_ch1));
	m_pit->set_clk<2>(2000000);
	m_pit->out_handler<2>().set(m_sound, FUNC(dai_sound_device::set_input_ch2));

	I8255(config, "ppi");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1056, 542);
	screen.set_visarea(0, 1056-1, 0, 302-1);
	screen.set_screen_update(FUNC(dai_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_dai);
	PALETTE(config, m_palette, FUNC(dai_state::dai_palette), std::size(s_palette));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER(config, "speaker", 2).front();
	DAI_SOUND(config, m_sound).add_route(0, "speaker", 0.50, 0).add_route(1, "speaker", 0.50, 1);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("dai_cass");

	/* tms5501 */
	TMS5501(config, m_tms5501, 2000000);
	m_tms5501->int_callback().set_inputline("maincpu", I8085_INTR_LINE);
	m_tms5501->xi_callback().set(FUNC(dai_state::keyboard_r));
	m_tms5501->xo_callback().set(FUNC(dai_state::keyboard_w));
	TIMER(config, m_tms_timer).configure_generic(FUNC(dai_state::tms_timer));

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("dai_cass");
}


ROM_START(dai)
	ROM_REGION(0x6000,"maincpu",0)
	ROM_LOAD("dai.bin",   0x0000, 0x2000, CRC(ca71a7d5) SHA1(6bbe2336c717354beab2ae201debeb4fd055bdcb))
	ROM_LOAD("dai00.bin", 0x2000, 0x1000, CRC(fa7d39ac) SHA1(3d1824a1f273882f934249ef3cb1b38ef99de7b9))
	ROM_LOAD("dai01.bin", 0x3000, 0x1000, CRC(cb5809f2) SHA1(523656f0a9d98888cd3e2bd66886c589e9ae75b4))
	ROM_LOAD("dai02.bin", 0x4000, 0x1000, CRC(03f72d4a) SHA1(573d65dc82321970dcaf81d7638a02252ea18a7a))
	ROM_LOAD("dai03.bin", 0x5000, 0x1000, CRC(c475c96f) SHA1(96fc3cc4b8a2873f0d044bd8033d1e7b7197dd97))

	ROM_REGION(0x2000, "chargen",0)
	ROM_LOAD ("nch.bin", 0x0000, 0x1000, CRC(a9f5b30b) SHA1(24119b2984ab4e50dc0dabae1065ff6d6c1f237d))
ROM_END

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                            FULLNAME */
COMP( 1978, dai,  0,      0,      dai,     dai,   dai_state, empty_init, "Data Applications International", "DAI Personal Computer", MACHINE_SUPPORTS_SAVE )
