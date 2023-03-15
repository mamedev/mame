// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************

Ondra driver by Miodrag Milanovic

2008-09-08 Preliminary driver.

ToDo:
- Paste/Natural keyboard are useless because 3rd modifier key is not supported.
- Add 2x i8253 pits which are part of the video timing circuit. They are not
  connected to the data bus, and are always selected.
- The video is somewhat similar to the standard super80, in that the CPU is
  turned off by BUSRQ about half the time, so that the video can be drawn
  without causing snow. The CPU can gain full control by disabling the video.
- Sound is a speaker connected to a multivibrator circuit. There are 3 diodes
  from this circuit to allow a choice of 7 frequencies. We have used a buzzer
  with selected arbitrary frequencies, not having any idea what they should be.
- Ondrav: can load from softlist. Load the tape then press Enter.
- Ondrav: doesn't seem to accept any other commands, need instructions.
  You have to press Esc before trying again.
- Ondrat: Doesn't load from softlist
- Cassette: don't know how to save.

******************************************************************************/


#include "emu.h"
#include "ondra.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/* Address maps */
void ondra_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0xdfff).bankrw("bank2");
	map(0xe000, 0xffff).bankrw("bank3");
}

void ondra_state::io_map(address_map &map)
{
	//map.global_mask(0x0b);
	map.unmap_value_high();
	map(0x03, 0x03).mirror(0xff00).w(FUNC(ondra_state::port03_w));
	//map(0x09, 0x09).mirror(0xff00).r(FUNC(ondra_state::port09_r));
	map(0x0a, 0x0a).mirror(0xff00).w(FUNC(ondra_state::port0a_w));
}

/* Input ports */
static INPUT_PORTS_START( ondra )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R 4 $") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E 3 #") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W 2 \"") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR('2') PORT_CHAR('\"')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T 5 %") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q 1 !") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR('1') PORT_CHAR('!')
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F ^") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR('^')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D =") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR('=')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S +") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G _") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR('_')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A -") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR('-')
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C :") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c') PORT_CHAR(':')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X /") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR('/')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z *") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR('*')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V ;") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(';')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) //PORT_CHAR(UCHAR_SHIFT_3)  not supported yet
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0-9") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS")  PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UpCase") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J >") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR('>')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR('[')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L ]") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(']')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H <") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h') PORT_CHAR('<')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U 7 \'") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR('7') PORT_CHAR('\'')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I 8 (") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i') PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O 9 )") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y 6 &") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P 0 @") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR('0') PORT_CHAR('@')
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N ,") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(',')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M .") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m') PORT_CHAR('.')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B ?") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR('?')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)   PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1)       PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1)        PORT_PLAYER(1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_START("NMI")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, ondra_state, nmi_button, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(ondra_state::nmi_button)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

u32 ondra_state::screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const r = m_ram->pointer();

	u8 code1=0,code2=0;
	int Vaddr = 0x2800;

	for (int x = 0; x < 40; x++)
	{
		for (int y = 127; y >=0; y--)
		{
			if (m_video_enable)
			{
				code1 = r[0xd700 + Vaddr + 0x80];
				code2 = r[0xd700 + Vaddr + 0x00];
			}
			for (int b = 0; b < 8; b++)
			{
				bitmap.pix(2*y, x*8+b) =  ((code1 << b) & 0x80) ? 1 : 0;
				bitmap.pix(2*y+1, x*8+b) =  ((code2 << b) & 0x80) ? 1 : 0;
			}
			Vaddr++;
		}
		Vaddr = (Vaddr - 128) - 256;
	}
	return 0;
}

WRITE_LINE_MEMBER(ondra_state::vblank_irq)
{
	if (state)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

/* Machine driver */
void ondra_state::ondra(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ondra_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ondra_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(320, 256);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update(FUNC(ondra_state::screen_update_ondra));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(ondra_state::vblank_irq));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 950); // guess
	m_beep->add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("ondra_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("ondra");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_default_value(0x00);
}

/* ROM definition */

ROM_START( ondrat )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "tesla_a.d22", 0x0000, 0x0800, CRC(6d56b815) SHA1(7feb4071d5142e4c2f891747b75fa4d48ccad262) )
	ROM_LOAD( "tesla_b.d21", 0x2000, 0x0800, CRC(5f145eaa) SHA1(c1eac68b13fedc4d0d6f98b15e2a5397f0139dc3) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mh74188.d27", 0x0000, 0x0040, CRC(7faceafe) SHA1(597f867e38b1c66d4622662cb01b3aefa680f234) )
ROM_END

ROM_START( ondrav )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vili_a.d22",  0x0000, 0x0800, CRC(76932657) SHA1(1f3700f670f158e4bed256aed751e2c1331a28e8) )
	ROM_RELOAD(0x0800, 0x0800)
	ROM_RELOAD(0x1000, 0x0800)
	ROM_RELOAD(0x1800, 0x0800)
	ROM_LOAD( "vili_b.d21",  0x2000, 0x0800, CRC(03a6073f) SHA1(66f198e63f473e09350bcdbb10fe0cf440111bec) )
	ROM_RELOAD(0x2800, 0x0800)
	ROM_RELOAD(0x3000, 0x0800)
	ROM_RELOAD(0x3800, 0x0800)

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mh74188.d27", 0x0000, 0x0040, CRC(7faceafe) SHA1(597f867e38b1c66d4622662cb01b3aefa680f234) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME      FLAGS
COMP( 1989, ondrat, 0,      0,      ondra,   ondra, ondra_state, empty_init, "Tesla", "Ondra",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1989, ondrav, ondrat, 0,      ondra,   ondra, ondra_state, empty_init, "ViLi",  "Ondra ViLi", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
