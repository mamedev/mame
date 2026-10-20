// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Siemens 97801 detached keyboard, International variant V1 -- LLE

    This is the 1985-generation complete keyboard 97801-111 (Siemens
    S26361-K111-V1): the national layout
    lives IN the keyboard firmware, which sends fully-resolved final codes on
    make only.  National variants V2-V10 are different K111-Vn EPROMs (dumps
    wanted).  The later 1987 generation (97801-131 + keycap sets) instead
    sends make/break position codes with terminal-side downloadable layout
    tables -- a different protocol on the same link.

    A Philips MAB 8035HL (MCS-48, ROM-less) executes the K111 EPROM
    (P26361-K111, AM2732A/D2732A, 2 KB used) unmodified.  The firmware RE
    (key positions cross-checked against the SINIX /etc/termcap "standard" entry
    documented in SINIX Schnittstellen Benutzerhandbuch U2300-J-Z95-1) established:

      BUS  in  key matrix row sense for the column selected on P1.0-3
               (16 columns x 8 rows, codes = rowbit*16+column -> the char/
               class tables at ROM pages 6/7); with P1.4 set instead the
               modifier column: b1 mode-lock, b2 CTRL, b3 shift-lock,
               b4 shift, b5/b6 config straps (read at reset; reported by
               status command $2A as straps^$DF -> $DC with straps open)
      BUS  out inverting latch driving 7 indicator lamps (commands $01-$0F)
      P1.5 out serial TX to the terminal (P1.6 carries the complement)
      P1.7 out spare, set/cleared by commands $21/$22
      P2.5 out bell one-shot trigger (command $24 pulses it low)
      P2.6/7   shift-lock / mode-lock LEDs
      P2.2/4   firmware self-flag / table-download service strap
      T0   in  serial RX from the terminal (async, sampled in the timer ISR)
      T1   in  complement of RX (used by cmd $27 line test + download mode)
      INT  in  RX line again (edge source for the synchronous download mode)

    Both link directions are firmware-timed at f_xtal/9600 (t_bit = 4 timer
    ISRs x 5 prescaler ticks x 32 t_cy x 15 clocks); the terminal's side is
    f_cpu/18432, so terminal and keyboard crystals pair at a fixed 1.92 ratio.
    The ROM-source keyboard carries a 5.760 MHz crystal -> 600.0 baud, exactly
    matching its companion W26361-D253 terminal board (22.1184/2 = 11.0592 MHz
    CPU) that supplied the terminal ROMs.
    The command set decoded from the dispatch at $311: $20 soft-reset,
    $21/$22 P1.7, $23/$29 disable, $28 enable, $24 bell, $25/$26 lamps off,
    $27 line test ($F5/$EF), $2A status, $2B/$2C download arm/abort,
    $2D ROM checksum, $2E RAM test ($AA pass/$EE fail), $2F ident "800105".

***************************************************************************/

#include "emu.h"
#include "machine/s97801_kbd.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(SIEMENS_97801_KBD, s97801_kbd_device, "s97801_kbd", "Siemens 97801 Keyboard (International V1)")

s97801_kbd_device::s97801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SIEMENS_97801_KBD, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_beep(*this, "beep")
	, m_cols(*this, "COL%u", 0U)
	, m_mod(*this, "MOD")
	, m_txd_cb(*this)
	, m_lamps(*this, "kbd_lamp%u", 0U)
	, m_led_caps(*this, "led_caps")
	, m_led_lock(*this, "led_lock")
	, m_bell_timer(nullptr)
	, m_p1(0xff)
	, m_p2(0xff)
	, m_rxd(1)
{
}

void s97801_kbd_device::device_start()
{
	m_bell_timer = timer_alloc(FUNC(s97801_kbd_device::bell_off), this);
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_rxd));
}

void s97801_kbd_device::rxd_w(int state)
{
	m_rxd = state;
	// the line also reaches the INT pin (per-bit edges for the download mode; idle in normal use)
	m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}

u8 s97801_kbd_device::bus_r()
{
	if (BIT(m_p1, 4)) // modifier-column strobe (columns decoded off)
		return m_mod->read();
	return m_cols[m_p1 & 0x0f]->read();
}

void s97801_kbd_device::bus_w(u8 data)
{
	// inverting latch driving the indicator lamps (host commands $01-$0F set/clear them)
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(~data, i);
}

void s97801_kbd_device::p1_w(u8 data)
{
	if (BIT(m_p1 ^ data, 5))
		m_txd_cb(BIT(data, 5)); // serial TX to the terminal
	m_p1 = data;
}

void s97801_kbd_device::p2_w(u8 data)
{
	// command $24 pulses P2.5 low; the rising edge fires the beeper's hardware one-shot
	if (BIT(data, 5) && !BIT(m_p2, 5))
	{
		m_beep->set_state(1);
		m_bell_timer->adjust(attotime::from_msec(100));
	}
	m_led_lock = BIT(data, 6);
	m_led_caps = BIT(data, 7);
	m_p2 = data;
}

TIMER_CALLBACK_MEMBER(s97801_kbd_device::bell_off)
{
	m_beep->set_state(0);
}

void s97801_kbd_device::prg_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("mcu", 0); // K111 (2 KB used, top half blank)
}

void s97801_kbd_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_mcu, 5.76_MHz_XTAL); // photographed on the ROM-source keyboard PCB (600 baud)
	m_mcu->set_addrmap(AS_PROGRAM, &s97801_kbd_device::prg_map);
	m_mcu->p1_in_cb().set_constant(0xff);  // quasi-bidirectional: IN A,Pp = latch & pull-ups
	m_mcu->p1_out_cb().set(FUNC(s97801_kbd_device::p1_w));
	m_mcu->p2_in_cb().set_constant(0xff);  // P2.4 high = download service strap open
	m_mcu->p2_out_cb().set(FUNC(s97801_kbd_device::p2_w));
	m_mcu->bus_in_cb().set(FUNC(s97801_kbd_device::bus_r));
	m_mcu->bus_out_cb().set(FUNC(s97801_kbd_device::bus_w));
	m_mcu->t0_in_cb().set(FUNC(s97801_kbd_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(s97801_kbd_device::t1_r));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1500).add_route(ALL_OUTPUTS, "mono", 0.25); // bell (command $24 / host BEL)
}

ROM_START(s97801_kbd)
	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("p26361_k111_v1_3.d3", 0x0000, 0x1000, CRC(aba8f4b7) SHA1(970a45b509081603e25319e1bbf0f7941f91a056))
ROM_END

const tiny_rom_entry *s97801_kbd_device::device_rom_region() const
{
	return ROM_NAME(s97801_kbd);
}

// Physical matrix recovered from the K111 tables (ROM $602-$7FF): key at (column, BUS bit) sends
// code (7-bit)*16+column; class/unshifted/shifted/control values per code are in the RE log.
// Key names follow the SINIX /etc/termcap 'standard' entry's key table (interface manual
// U2300-J-Z95-1 p.1-26); remaining bracketed hex names are local-function keys (MODE, PRINT,
// MENU, CH CODE, C_E, ...) whose cap-to-code assignment is not yet confirmed.
// PORT_CHARs are the HOST-EFFECTIVE characters: the keyboard's own (International V1) table
// value passed through the terminal's power-on d21 recode, so the natural keyboard types what
// the host receives (y/z and about a dozen punctuation caps differ from the physical legend --
// those keys carry the cap legend in PORT_NAME).  This matches SINIX's German default tables;
// a host that downloads the International table gets the physical caps instead.
static INPUT_PORTS_START(s97801_kbd)
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_NAME("Esc") PORT_CHAR(0x1b)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME("\\ |") PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 20 [ESC e/h]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6") PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1") PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("z Z") PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 21 [ESC 7/8]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2") PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2") PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3 #") PORT_CHAR('3') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Wrap Back [ESC R]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5") PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D8")

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3") PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 23 [ESC S]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1") PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad SP")

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4") PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CH CODE (INT LED)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0") PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Jump Word Left [ESC :]")

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5") PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("y Y") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F18) PORT_NAME("F18") PORT_CHAR(UCHAR_MAMEKEY(F18))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DELETE CHAR [ESC V]") PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Cursor Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("F6") PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7 '") PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 26 (termcap Pl) [ESC g]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad - [EA]") PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Jump Word Right [ESC 9]")

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("F7") PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F20) PORT_NAME("F20") PORT_CHAR(UCHAR_MAMEKEY(F20))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9") PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Thick Arrow Down [CSI T]")

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("F8") PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(", <") PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS LINE / DEL LINE [CSI L/M]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("EB [ESC ^]")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS WORD / DEL WORD [ESC o/p]")

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("F9") PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0 _") PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME("; +") PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(". >") PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 29 [ESC 6/l]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C2 [-]")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("START [ESC m]")

	PORT_START("COL10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_NAME("F10") PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("- =") PORT_CHAR('~') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("@ `") PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME(": *") PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/ ?") PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F19) PORT_NAME("F19") PORT_CHAR(UCHAR_MAMEKEY(F19))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4") PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Cursor Right") PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("COL11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15) PORT_NAME("F15") PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 000") PORT_CODE(KEYCODE_000_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad %")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3") PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MENU) PORT_NAME("MENU [ESC X]") PORT_CHAR(UCHAR_MAMEKEY(MENU))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F16) PORT_NAME("F16") PORT_CHAR(UCHAR_MAMEKEY(F16))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("Keypad *")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 0B [ESC f/i]")

	PORT_START("COL12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14) PORT_NAME("F14") PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("HELP [ESC >]") PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("INS CHAR / DEL CHAR [CSI @/P]") PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Cursor Up") PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 2C (INT lamp panel) [ESC 4/5]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Keypad /")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a)

	PORT_START("COL13")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13) PORT_NAME("F13") PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("RUBOUT") PORT_CHAR(0x7f)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Back Tab [CSI Z]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Thick Arrow Up [CSI S]")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Cursor Home") PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("END [Ctrl-D]") PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7") PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_00_PAD)

	PORT_START("COL14")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_NAME("F12") PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(0x08)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Return") PORT_CHAR(0x0d)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_NAME("Tab") PORT_CHAR(0x09)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Cursor Left") PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F17) PORT_NAME("F17") PORT_CHAR(UCHAR_MAMEKEY(F17))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +") PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_EQUALS_PAD)

	PORT_START("COL15")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_NAME("F11") PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("^ ~") PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("[ {") PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("] }") PORT_CHAR('#') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR) PORT_NAME("PRINT [ESC Z]") PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Unfitted 2F [ESC _/2]")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8") PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Keypad Enter") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START("MOD") // modifier column (BUS read with P1.4 strobe); b5/b6 = config straps (in bus_r)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAPS (LED)") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // toggle; second-row key per the V1 layout
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("CTRL") PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOCK (LED)") // shift lock left of A, released by Shift
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1)
	// b5/b6: the key-operated lock switch beside the keypad.  The firmware samples these bits
	// every scan pass and reports changes unsolicited; the status command $2A returns
	// swap(bits)^$DF, so the four positions read $DC..$DF at the host.
	PORT_CONFNAME(0x60, 0x60, "Keylock switch")
	PORT_CONFSETTING(0x60, "Position 1 (status DC)")
	PORT_CONFSETTING(0x40, "Position 2 (status DD)")
	PORT_CONFSETTING(0x20, "Position 3 (status DE)")
	PORT_CONFSETTING(0x00, "Position 4 (status DF)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor s97801_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s97801_kbd);
}
