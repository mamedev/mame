// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Pecom driver by Miodrag Milanovic

2008-11-08 Preliminary driver.

- All commands to be in UPPERCASE.
- Change background colour: SCR n
- Enter monitor: PROB     (B to exit)
- If Capslock is engaged, then Shift doesn't work.
- Control hangs the machine while it is pressed. It doesn't work in the
  expected way.
- Don't touch the Shift key while loading a tape because it will corrupt
  the data.
- The screen will flash in a crazy epileptic fashion while loading a tape.
  Beware!

TODO:
- Cassette: can load its own recordings, but not those from software list
  (software-list tapes are slower & wobbly)
- Both machines currently have 32k ram.
- Autorepeat seems a bit fast

****************************************************************************/

#include "emu.h"
#include "pecom.h"
#include "softlist_dev.h"
#include "speaker.h"


/* Address maps */
void pecom_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x8000, 0xefff).rom().region("maincpu",0);
	map(0xf000, 0xf7ff).bankrw("bank3"); // CDP1869 / ROM
	map(0xf800, 0xffff).bankrw("bank4"); // CDP1869 / ROM
}

void pecom_state::io_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(pecom_state::bank_w));
	map(0x03, 0x03).r(FUNC(pecom_state::keyboard_r));
	map(0x03, 0x07).w(FUNC(pecom_state::cdp1869_w));
}

void pecom_state::cdp1869_page_ram(address_map &map)
{
	map(0x000, 0x3ff).mirror(0x400).ram();
}

/* Input ports */
/* Pecom 64 keyboard layout is as follows

    1!     2"     3#     4$     5%     6&     7'     8(     9)     0     BREAK

   DEL  Q      W      E      R      T      Y      U      I      O      P   ESC

    CAPS    A      S      D      F      G      H      J      K      L   RETURN

  CTRL  ,<     Z      X      C      V      B      N      M      :*     /?   LF

     SHIFT  .>    Down   Left       SPACEBAR       Right    Up      ;+    =-

Being keys distributed on four lines, it makes a bit difficult to accurately remap them
on modern keyboards. Hence, we move by default Up/Down/Left/Right to Cursor Keys and
use Left/Right Ctrl/Alt keys for the remaining keys. Due to the unnatural emulated keyboard
mappings, this is another situation where natural keyboard comes very handy!          */

INPUT_CHANGED_MEMBER(pecom_state::ef_w)
{
	m_maincpu->set_input_line((int)param, newval);
}

static INPUT_PORTS_START( pecom )
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_COLON) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // Actually this is again / ? - same key connected as on SLASH

	PORT_START("LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR('=') PORT_CHAR('-')

	PORT_START("LINE9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("LINE11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("LINE12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')

	PORT_START("LINE13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("LINE15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("LINE16")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("LINE17")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')

	PORT_START("LINE19")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("LINE20")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')

	PORT_START("LINE21")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE22")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')

	PORT_START("LINE23")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Sh") PORT_CODE(KEYCODE_DOWN) // sh up

	PORT_START("LINE24")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Dj") PORT_CODE(KEYCODE_LEFT) // dj left
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cj") PORT_CODE(KEYCODE_RIGHT) // cj right

	PORT_START("LINE25")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ch") PORT_CODE(KEYCODE_UP)  //ch up
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("CNT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_MINUS) PORT_CHANGED_MEMBER(DEVICE_SELF, pecom_state, ef_w, COSMAC_INPUT_LINE_EF4)
INPUT_PORTS_END

/* Machine driver */
void pecom_state::pecom64(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, cdp1869_device::DOT_CLK_PAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pecom_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pecom_state::io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(pecom_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(pecom_state::ef2_r));
	m_maincpu->q_cb().set(FUNC(pecom_state::q_w));
	m_maincpu->sc_cb().set(FUNC(pecom_state::sc_w));

	SPEAKER(config, "mono").front_center();

	CDP1869(config, m_cdp1869, cdp1869_device::DOT_CLK_PAL, &pecom_state::cdp1869_page_ram);
	m_cdp1869->add_pal_screen(config, "screen", cdp1869_device::DOT_CLK_PAL);
	m_cdp1869->set_color_clock(cdp1869_device::COLOR_CLK_PAL);
	m_cdp1869->set_pcb_read_callback(FUNC(pecom_state::pcb_r));
	m_cdp1869->set_char_ram_read_callback(FUNC(pecom_state::char_ram_r));
	m_cdp1869->set_char_ram_write_callback(FUNC(pecom_state::char_ram_w));
	m_cdp1869->pal_ntsc_callback().set_constant(1);
	m_cdp1869->prd_callback().set(FUNC(pecom_state::prd_w));
	m_cdp1869->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("pecom_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("pecom_cass");
}

/* ROM definition */
ROM_START( pecom32 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "090786.bin", 0x0000, 0x4000, CRC(b3b1ea23) SHA1(de69f22568161ced801973345fa39d6d207b9e8c) )
ROM_END

ROM_START( pecom64 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "ver4", "version 4")
	ROMX_LOAD( "rom_1_g_24.02.88_l.bin", 0x0000, 0x4000, CRC(9a433b47) SHA1(dadb8c399e0a25a2693e10e42a2d7fc2ea9ad427), ROM_BIOS(0) )
	ROMX_LOAD( "rom_2_g_24.02.88_d.bin", 0x4000, 0x4000, CRC(2116cadc) SHA1(03f11055cd221d438a40a41874af8fba0fa116d9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "ver1", "version 1")
	ROMX_LOAD( "170887-rom1.bin", 0x0000, 0x4000, CRC(43710fb4) SHA1(f84f75061c9ac3e34af93141ecabd3c955881aa2), ROM_BIOS(1) )
	ROMX_LOAD( "170887-rom2.bin", 0x4000, 0x4000, CRC(d0d34f08) SHA1(7baab17d1e68771b8dcef97d0fffc655beabef28), ROM_BIOS(1) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME     FLAGS */
COMP( 1986, pecom32, 0,       0,      pecom64, pecom, pecom_state, empty_init, "Ei Nis (Elektronska Industrija Nis)", "Pecom 32", MACHINE_SUPPORTS_SAVE )
COMP( 1987, pecom64, pecom32, 0,      pecom64, pecom, pecom_state, empty_init, "Ei Nis (Elektronska Industrija Nis)", "Pecom 64", MACHINE_SUPPORTS_SAVE )
