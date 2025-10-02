// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Keytronic L2207 83-key keyboard

    This was sold by Key Tronic Corp. as a generic VT100-lookalike
    ASCII keyboard. Its normal configuration communicates via 300 baud
    asynchronous TTL-level serial, but populating a few additional
    components and using an alternate connector gives it a parallel
    interface for outputs. The PCB additionally supports a "minimum"
    interface that eliminates the MCU and provides a raw interface to
    the capacitive key matrix ASICs (22-950-003 decoder & 22-908-003
    detector, as described in U.S. Patent 4,277,780) and LED latch.

    Keys generating non-ASCII codes:

        No Scroll       B0
        0 (keypad)      B1
        . (keypad)      B2
        1 (keypad)      C0
        2 (keypad)      C1
        3 (keypad)      C2
        Enter (keypad)  C3
        4 (keypad)      D0
        5 (keypad)      D1
        6 (keypad)      D2
        , (keypad)      D3
        Break           E0
        7 (keypad)      E1
        8 (keypad)      E2
        9 (keypad)      E3
        - (keypad)      E4
        Ctrl+3          E5
        ↑               F1
        ↓               F2
        ←               F3
        →               F4
        PF1             F6
        PF2             F7
        PF3             F8
        PF4             F9
        Set-Up          FE

    Several other codes (A0-A5, D4, D6, F0, F5) are assigned to
    9 additional switches which are not normally populated. The
    presently dumped firmware also sends AA as its ID byte.

    List of commands (delivered through serial input):

        D7 D6 D5 D4 D3 D2 D1 D0

        l8 l7 l6 l5 l4 l3 l2  1      Set LED data
         -  -  -  -  -  -  1  0      Short beep
         -  -  -  -  -  1  0  0      Long beep
         -  -  -  -  1  0  0  0      Key click off
         -  -  -  1  0  0  0  0      Keyboard ID (undocumented)
         -  -  -  0  0  0  0  0      Key click on

    An alternate version of this keyboard, whose solder side is
    labeled "PCB-251 / KTC A65-02675-051." has differently styled
    keytops and a different EPROM but is otherwise identical in layout
    and functionality.

    The Kaypro II 76-key detachable keyboard is practically identical
    to L2207 (the firmware is identical), but with no LEDs installed
    and seven keys removed.

**********************************************************************/

#include "emu.h"
#include "keytronic_l2207.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> keytronic_l2207_device

class keytronic_l2207_device : public device_t, public device_keytronic_interface
{
public:
	// device type constructor
	keytronic_l2207_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	keytronic_l2207_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_keytronics_interface implementation
	virtual void ser_in_w(int state) override;

	// MCU handlers
	u8 led_latch_r();
	u8 p1_r();
	void p2_w(u8 data);

	// address maps
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	// object finders
	required_device<mcs48_cpu_device> m_mcu;
	required_device<speaker_sound_device> m_beeper;
	required_ioport_array<12> m_keys;
	output_finder<8> m_leds;
	output_finder<> m_all_caps;

	// internal state
	u8 m_p1_in;
	u8 m_p2_out;
	bool m_beeper_latch;
};


// ======================> kayproii_keyboard_device

class kayproii_keyboard_device : public keytronic_l2207_device
{
public:
	// device type constructor
	kayproii_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

keytronic_l2207_device::keytronic_l2207_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_keytronic_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_beeper(*this, "beeper")
	, m_keys(*this, "KEYS%d", 0U)
	, m_leds(*this, "kbd_led%d", 1U)
	, m_all_caps(*this, "all_caps")
	, m_p1_in(0xff)
	, m_p2_out(0)
	, m_beeper_latch(false)
{
}

keytronic_l2207_device::keytronic_l2207_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: keytronic_l2207_device(mconfig, KEYTRONIC_L2207, tag, owner, clock)
{
}

kayproii_keyboard_device::kayproii_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: keytronic_l2207_device(mconfig, KAYPROII_KEYBOARD, tag, owner, clock)
{
}

void keytronic_l2207_device::device_resolve_objects()
{
	m_leds.resolve();
	m_all_caps.resolve();
}

void keytronic_l2207_device::device_start()
{
	save_item(NAME(m_p1_in));
	save_item(NAME(m_p2_out));
	save_item(NAME(m_beeper_latch));
}

void keytronic_l2207_device::ser_in_w(int state)
{
	m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}

u8 keytronic_l2207_device::led_latch_r()
{
	if (!machine().side_effects_disabled())
	{
		u8 led_data = m_mcu->p1_r();

		if (BIT(m_p2_out, 6))
			m_beeper->level_w(BIT(led_data, 0));
		m_beeper_latch = BIT(led_data, 0);

		m_leds[0] = !BIT(led_data, 1);
		for (int n = 1; n < 8; n++)
			m_leds[n] = BIT(led_data, n);
	}

	return 0;
}

u8 keytronic_l2207_device::p1_r()
{
	return m_p1_in;
}

void keytronic_l2207_device::p2_w(u8 data)
{
	if (BIT(data, 5) != BIT(m_p2_out, 5))
		ser_out_w(BIT(data, 5));

	if (!BIT(data, 4))
		m_p1_in = 0xff;
	else if (!BIT(m_p2_out, 4))
	{
		u8 j = m_mcu->p1_r() & 0x0f;
		if (j < 12)
			m_p1_in = m_keys[j]->read();
	}

	if (m_beeper_latch && BIT(data, 6) != BIT(m_p2_out, 6))
		m_beeper->level_w(BIT(data, 6));

	m_all_caps = BIT(data, 7);

	m_p2_out = data;
}

void keytronic_l2207_device::prog_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).rom().region("eprom", 0);
}

void keytronic_l2207_device::ext_map(address_map &map)
{
	map(0x00, 0xff).r(FUNC(keytronic_l2207_device::led_latch_r)).nopw(); // WR connected for parallel interface only
}

static INPUT_PORTS_START(keytronic_l2207)
	PORT_START("KEYS0")
	PORT_BIT(0x0d, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Set-Up") PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KEYS1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("No Scrl") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  @") PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR(0x00) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"3  # £") PORT_CHAR('3') PORT_CHAR('#', 0x00a3) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)

	PORT_START("KEYS3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  Bell") PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)

	PORT_START("KEYS4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  ^") PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR(0x1e) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CHAR('u') PORT_CHAR('u') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CHAR('j') PORT_CHAR('j') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CHAR('k') PORT_CHAR('k') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('b') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('n') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)

	PORT_START("KEYS5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEYS6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  _") PORT_CHAR('-') PORT_CHAR('_') PORT_CHAR(0x1f) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)

	PORT_START("KEYS7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |") PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_PGDN)

	PORT_START("KEYS8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("KEYS9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF2") PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF1") PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)

	PORT_START("KEYS10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF4") PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF3") PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)

	PORT_START("KEYS11")
	PORT_BIT(0x5f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END

static INPUT_PORTS_START(kayproii_keyboard)
	PORT_INCLUDE(keytronic_l2207)

	PORT_MODIFY("KEYS0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KEYS1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KEYS8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KEYS9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KEYS10")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor keytronic_l2207_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(keytronic_l2207);
}

ioport_constructor kayproii_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(kayproii_keyboard);
}

void keytronic_l2207_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_mcu, 3.579545_MHz_XTAL); // P8035HL + 48-00300-010 XTAL
	m_mcu->set_addrmap(AS_PROGRAM, &keytronic_l2207_device::prog_map);
	m_mcu->set_addrmap(AS_IO, &keytronic_l2207_device::ext_map);
	m_mcu->p1_in_cb().set(FUNC(keytronic_l2207_device::p1_r));
	m_mcu->p2_out_cb().set(FUNC(keytronic_l2207_device::p2_w));
	m_mcu->t0_in_cb().set_constant(1); // tied to EA
	m_mcu->t1_in_cb().set_constant(0); // pulled up for inverted parallel output
	m_mcu->bus_out_cb().set_nop(); // WR connected for parallel interface only
	m_mcu->prog_out_cb().set_nop(); // STB on parallel connector

	SPEAKER(config, "mono").front_center(); // 48-00125-000 + flyback diode
	SPEAKER_SOUND(config, m_beeper).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void kayproii_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 3.579545_MHz_XTAL);
	m_mcu->set_addrmap(AS_IO, &kayproii_keyboard_device::ext_map);
	m_mcu->p1_in_cb().set(FUNC(kayproii_keyboard_device::p1_r));
	m_mcu->p2_out_cb().set(FUNC(kayproii_keyboard_device::p2_w));
	m_mcu->t0_in_cb().set_constant(1);
	m_mcu->t1_in_cb().set_constant(0);
	m_mcu->bus_out_cb().set_nop();
	m_mcu->prog_out_cb().set_nop();

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_beeper).add_route(ALL_OUTPUTS, "mono", 0.5);
}

ROM_START(keytronic_l2207)
	ROM_REGION(0x800, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "a65_02207", "A65-02207-051/2510") // from Wicat System 150
	ROMX_LOAD("w150_ns2758.bin", 0x000, 0x400, CRC(f65e1ca5) SHA1(7919385fe8badbb610b793a3f5e4077982094aaa), ROM_BIOS(0))
	ROM_RELOAD(0x400, 0x400)
	ROM_SYSTEM_BIOS(1, "a65_02675", "A65-02675-051")
	ROMX_LOAD("key_05__162092859__8-23-82.bin", 0x000, 0x800, CRC(970b11a3) SHA1(a1c9c505eb3ccf132307b2ac0e04b0326f50621e), ROM_BIOS(1)) // MM2716Q
ROM_END

ROM_START(kayproii_keyboard)
	ROM_REGION(0x400, "mcu", 0)
	ROM_LOAD("kaypro_ii-ins8048.bin", 0x000, 0x400, CRC(f65e1ca5) SHA1(7919385fe8badbb610b793a3f5e4077982094aaa))
ROM_END

const tiny_rom_entry *keytronic_l2207_device::device_rom_region() const
{
	return ROM_NAME(keytronic_l2207);
}

const tiny_rom_entry *kayproii_keyboard_device::device_rom_region() const
{
	return ROM_NAME(kayproii_keyboard);
}

} // anonymous namespace

// device type definitions
DEFINE_DEVICE_TYPE_PRIVATE(KEYTRONIC_L2207, device_keytronic_interface, keytronic_l2207_device, "keytronic_l2207", "Keytronic L2207 serial keyboard")
DEFINE_DEVICE_TYPE_PRIVATE(KAYPROII_KEYBOARD, device_keytronic_interface, kayproii_keyboard_device, "kayproiikbd", "Kaypro II Keyboard")
