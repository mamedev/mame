// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Open University Hektor

    Hektor usage notes:
    - G197B - tune machine (requires PT502 Peripheral Board)

    Hektor 2 usage notes:
    - G1B38 - tune machine (via TV speaker and PT502 Peripheral Board)

    Hektor/Hektor 2 monitor commands:
    - E - execute editor
    - G - (go to) execute from addr
    - T - test subsystems
    - H - (high-level language) enter BASIC
    - W - (warm start) re-enter BASIC

    Hektor III usage notes:
    - t  - system test
    - a1 - audio test (plays The Entertainer)
    - a3 - graphic test
    - ay - run analyser software in ROM1
    - az - run analyser software in ROM2


    Known PT502 Modules:

        ME83A - Memory Expansion Module:

            RAM: 6166/400ns
            IC6 4000 - 47FF
            IC5 4800 - 4FFF
            IC4 5000 - 57FF
            IC3 5800 - 5FFF

            EPROM: 2732/400ns
            IC2 C000
            IC1 D000

        EP83 - Eprom Programmer:

            Hektor 1:
            Read 2716 into RAM  ... G1BBC
            Write 2716 from RAM ... G1BBF
            Read 2732 into RAM  ... G1ACD
            Write 2732 from RAM ... G1AD0

            Hektor 1 RAM area 3000-3FFF

            Hektor 2:
            Read 2732 into RAM  ... G1EA7
            Write 2732 from RAM ... G1EAA

            Hektor 2 RAM area 3000-3FFF

        IO83 - Input/Output Prototyping Module:

            8155 Register Addresses:
            1. CS Register.........40H
            2. PA Register.........41H
            3. PB Register.........42H
            4. PC Register.........43H
            5. Timer Low Byte......44H
            6. Timer High Byte.....45H

            Notes:
            1. The CS register sets the
               port configuration:
               Starts and stops the timer.
            2. Bits 0 to 13 in timer register
               set count length; bits 14,15
               set timer mode.


    TODO:
    - Fix Hektor cassette interface, test with T.
    - Add Expansion Bus and Peripheral Boards (probably not useful).

**********************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8255.h"
#include "sound/spkrdev.h"
#include "video/ef9364.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "speaker.h"
#include "emupal.h"
#include "screen.h"


namespace {

class hektor_base_state : public driver_device
{
public:
	hektor_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i8155(*this, "i8155")
		, m_kbd(*this, "KEY%u", 0)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_rs232(*this, "rs232")
		, m_kbd_row(0)
		, m_motor(0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_rst65);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void i8155_porta_w(uint8_t data);
	uint8_t i8155_portb_r();

	int sid_r();
	void sod_w(int state);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8155_device> m_i8155;
	required_ioport_array<8> m_kbd;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<rs232_port_device> m_rs232;

	uint8_t m_kbd_row;
	bool m_motor;
};


class hektor_state : public hektor_base_state
{
public:
	hektor_state(const machine_config &mconfig, device_type type, const char *tag)
		: hektor_base_state(mconfig, type, tag)
		, m_ef9364(*this, "ef9364")
		, m_pt_i8155(*this, "pt_i8155")
		, m_ef9364_st(0)
	{ }

	void hektor(machine_config &config);
	void hektor2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void hektor_mem(address_map &map) ATTR_COLD;
	void hektor_io(address_map &map) ATTR_COLD;
	void hektor2_mem(address_map &map) ATTR_COLD;

	void i8155_portc_w(uint8_t data);

	required_device<ef9364_device> m_ef9364;
	required_device<i8155_device> m_pt_i8155;

	int m_ef9364_st;
};


class hektor3_state : public hektor_base_state
{
public:
	hektor3_state(const machine_config &mconfig, device_type type, const char *tag)
		: hektor_base_state(mconfig, type, tag)
		, m_rom(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_bank(*this, "bank")
		, m_hd6845(*this, "hd6845")
		, m_i8255(*this, "i8255")
	{ }

	void hektor3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void hektor3_mem(address_map &map) ATTR_COLD;
	void hektor3_io(address_map &map) ATTR_COLD;

	void i8155_portc_w(uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);

	required_memory_region m_rom;
	required_shared_ptr<uint8_t> m_ram;
	required_memory_bank m_bank;
	required_device<hd6845s_device> m_hd6845;
	required_device<i8255_device> m_i8255;
};


void hektor_base_state::i8155_porta_w(uint8_t data)
{
	m_kbd_row = data;
}

uint8_t hektor_base_state::i8155_portb_r()
{
	for (int col = 0; col < 8; col++)
	{
		if (!BIT(m_kbd_row, col)) return m_kbd[col]->read();
	}

	return 0xff;
}

void hektor_state::i8155_portc_w(uint8_t data)
{
	/* bit 0: EF9364 strobe ST */
	/* bit 1: EF9364 command C0 */
	/* bit 2: EF9364 command C1 */
	/* bit 3: EF9364 command C2 */
	if (!m_ef9364_st && BIT(data, 0))
		m_ef9364->command_w(data >> 1);
	m_ef9364_st = BIT(data, 0);

	/* bit 4: enable latch output */

	/* bit 5: cassette motor */
	m_motor = BIT(data, 5);
	m_cassette->change_state(m_motor ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void hektor3_state::i8155_portc_w(uint8_t data)
{
	/* bit 2: audio out */
	m_speaker->level_w(BIT(data, 2));

	/* bit 3: cassette motor */
	m_motor = BIT(data, 3);
	m_cassette->change_state(m_motor ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	/* bit 5: ROM enable */
	if (BIT(data, 5))
		m_bank->set_base(m_rom->base());
	else
		m_bank->set_base(m_ram);
}


int hektor_base_state::sid_r()
{
	if (m_motor)
		return (m_cassette->input() > 0.0 ? 1 : 0);
	else
		return m_rs232->rxd_r();
}

void hektor_base_state::sod_w(int state)
{
	/* tv out (confirmed for Hektor 2 only) */
	m_speaker->level_w(state);

	if (m_motor)
		m_cassette->output(state ? -1.0 : +1.0);
	else
		m_rs232->write_txd(state);
}


MC6845_UPDATE_ROW(hektor3_state::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		uint16_t offset = (ra << 11) | ((ma + x) & 0x7ff);
		uint8_t data = m_ram[~offset & 0xffff];
		if (x == cursor_x) data ^= 0xff;

		bitmap.pix(y, x * 8 + 0) = pen[BIT(data, 7)];
		bitmap.pix(y, x * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix(y, x * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix(y, x * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix(y, x * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix(y, x * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix(y, x * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix(y, x * 8 + 7) = pen[BIT(data, 0)];
	}
}


void hektor_state::hektor_mem(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x8000).rom().region("maincpu", 0x0000); // skt0
	map(0x1000, 0x1fff).mirror(0x8000).rom().region("maincpu", 0x1000); // skt1
	map(0x2800, 0x2bff).mirror(0x8000).w(m_ef9364, FUNC(ef9364_device::char_latch_w));
	map(0x2c00, 0x2cff).mirror(0x0300).rw(m_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x3000, 0x33ff).mirror(0x8000).ram(); // skt2
	map(0x3400, 0x37ff).mirror(0x8000).ram(); // skt3
	map(0x3800, 0x3bff).mirror(0x8000).ram(); // skt4
	map(0x3c00, 0x3fff).mirror(0x8000).ram(); // skt5
	map(0x4000, 0x40ff).mirror(0x8000).rw(m_pt_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w)); // pt502 peripheral board
	map(0xac00, 0xac07).mirror(0x03f8).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

void hektor_state::hektor_io(address_map &map)
{
	// pt502 peripheral board - PA = unknown, PB = b4 sound, PC = unknown
	map(0x40, 0x47).rw(m_pt_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));

	// tm222 peripheral board - PA = board switches, PB = board leds, PC = board pushbuttons
	//map(0x80, 0x83).rw(m_tm_i8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}


void hektor_state::hektor2_mem(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x8000).rom().region("maincpu", 0x0000); // skt0
	map(0x1000, 0x1fff).mirror(0x8000).rom().region("maincpu", 0x1000); // skt1
	map(0x2000, 0x23ff).mirror(0x8000).w(m_ef9364, FUNC(ef9364_device::char_latch_w));
	map(0x2400, 0x24ff).mirror(0x0300).rw(m_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x2800, 0x2fff).mirror(0x8000).ram(); // skt3
	map(0x3000, 0x37ff).mirror(0x8000).ram(); // skt4
	map(0x3800, 0x3fff).mirror(0x8000).ram(); // skt5
	map(0x4000, 0x40ff).mirror(0x8000).rw(m_pt_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w)); // pt502 peripheral board
	map(0x7000, 0x7fff).mirror(0x8000).rom().region("maincpu", 0x2000); // skt2
	map(0xa400, 0xa407).mirror(0x03f8).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}


void hektor3_state::hektor3_mem(address_map &map)
{
	map(0x0000, 0xffff).bankr("bank").writeonly().share("ram");
}

void hektor3_state::hektor3_io(address_map &map)
{
	map(0x00, 0x00).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));
	map(0x01, 0x01).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x08, 0x0f).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x10, 0x13).rw(m_i8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}


static INPUT_PORTS_START(hektor)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, hektor_base_state, trigger_reset, 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_TAB) PORT_CHANGED_MEMBER(DEVICE_SELF, hektor_base_state, trigger_rst65, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(hektor_base_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(hektor_base_state::trigger_rst65)
{
	m_maincpu->set_input_line(I8085_RST65_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START(hektor3)
	PORT_INCLUDE(hektor)

	PORT_MODIFY("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_MODIFY("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')

	PORT_MODIFY("KEY6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 _") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHANGED_MEMBER(DEVICE_SELF, hektor_base_state, trigger_reset, 0)
INPUT_PORTS_END


void hektor_base_state::machine_start()
{
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_motor));
}

void hektor_state::machine_start()
{
	hektor_base_state::machine_start();

	save_item(NAME(m_ef9364_st));
}

void hektor3_state::machine_start()
{
	hektor_base_state::machine_start();

	m_bank->set_base(m_rom->base());
}

void hektor_base_state::machine_reset()
{
	m_maincpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);
}


void hektor_state::hektor(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hektor_state::hektor_mem);
	m_maincpu->set_addrmap(AS_IO, &hektor_state::hektor_io);
	m_maincpu->in_sid_func().set(FUNC(hektor_state::sid_r));
	m_maincpu->out_sod_func().set(FUNC(hektor_state::sod_w));
	m_maincpu->set_clk_out("i8155", FUNC(i8155_device::set_unscaled_clock_int));

	I8155(config, m_i8155, 6.144_MHz_XTAL / 2);
	m_i8155->out_pa_callback().set(FUNC(hektor_state::i8155_porta_w));
	m_i8155->in_pb_callback().set(FUNC(hektor_state::i8155_portb_r));
	m_i8155->out_pc_callback().set(FUNC(hektor_state::i8155_portc_w));
	m_i8155->out_to_callback().set_inputline(m_maincpu, I8085_TRAP_LINE);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->cts_handler().set_inputline(m_maincpu, I8085_RST55_LINE);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	SPEAKER(config, "mono").front_center();

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_screen_update("ef9364", FUNC(ef9364_device::screen_update));
	m_screen->set_size(64 * 8, 16 * (8 + 4));
	m_screen->set_visarea(0, 64 * 8 - 1, 0, 16 * (8 + 4) - 1);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	EF9364(config, m_ef9364, 6.144_MHz_XTAL / 6);
	m_ef9364->set_screen(m_screen);
	m_ef9364->set_palette_tag("palette");
	m_ef9364->set_nb_of_pages(1);
	m_ef9364->set_erase(0x20);

	// pt502 peripheral board
	I8155(config, m_pt_i8155, 6.144_MHz_XTAL / 2);
	m_pt_i8155->out_pb_callback().set(m_speaker, FUNC(speaker_sound_device::level_w)).bit(4);
}


void hektor_state::hektor2(machine_config &config)
{
	hektor(config);

	m_maincpu->set_clock(6.048_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hektor_state::hektor2_mem);
	m_maincpu->set_addrmap(AS_IO, &hektor_state::hektor_io);

	m_ef9364->set_clock(6.048_MHz_XTAL / 6);
}


void hektor3_state::hektor3(machine_config &config)
{
	I8085A(config, m_maincpu, 16_MHz_XTAL / 2); // TODO: divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &hektor3_state::hektor3_mem);
	m_maincpu->set_addrmap(AS_IO, &hektor3_state::hektor3_io);
	m_maincpu->in_sid_func().set(FUNC(hektor3_state::sid_r));
	m_maincpu->out_sod_func().set(FUNC(hektor3_state::sod_w));
	m_maincpu->set_clk_out("i8155", FUNC(i8155_device::set_unscaled_clock_int));

	I8155(config, m_i8155, 16_MHz_XTAL / 4);
	m_i8155->out_pa_callback().set(FUNC(hektor3_state::i8155_porta_w));
	m_i8155->in_pb_callback().set(FUNC(hektor3_state::i8155_portb_r));
	m_i8155->out_pc_callback().set(FUNC(hektor3_state::i8155_portc_w));
	m_i8155->out_to_callback().set_inputline(m_maincpu, I8085_TRAP_LINE);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->cts_handler().set_inputline(m_maincpu, I8085_RST55_LINE);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	SPEAKER(config, "mono").front_center();

	I8255(config, m_i8255); // I/O Port

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 240);
	m_screen->set_screen_update("hd6845", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_hd6845, 16_MHz_XTAL / 8);
	m_hd6845->set_screen(m_screen);
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(8);
	m_hd6845->set_update_row_callback(FUNC(hektor3_state::crtc_update_row));
}


ROM_START(hektor)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("hek1a_skt0.rom", 0x0000, 0x1000, CRC(cadf4d57) SHA1(ffcecac16cc8cf12e65acb359c3dfe26b06e43d1))
	ROM_LOAD("hek1a_skt1.rom", 0x1000, 0x1000, CRC(dbcaa026) SHA1(fa83e65f6125a3ef2a219ef69a3dd939888cb545))

	ROM_REGION(0x800, "ef9364", 0)
	ROM_LOAD("hek1a.ic5", 0x0000, 0x0800, CRC(a04e2061) SHA1(01d95d7e2f62d9c1b952888133e6b0dedf971987))
ROM_END

ROM_START(hektor2)
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("hek2_skt0.rom", 0x0000, 0x1000, CRC(2cc5cf07) SHA1(ce40a594be17532e636a7cbcc75c7d18377f3dca))
	ROM_LOAD("hek2_skt1.rom", 0x1000, 0x1000, CRC(b2388185) SHA1(d084df17aa5e333831fa94e02699673b088ee5ca))
	ROM_LOAD("hek2_skt2.rom", 0x2000, 0x1000, CRC(805c62a4) SHA1(f698d78274133124ee9f5420a5b5142b84a22922))

	ROM_REGION(0x800, "ef9364", 0)
	ROM_LOAD("hek2.ic5", 0x0000, 0x0800, CRC(1e9ec285) SHA1(f1cbbff8460d69b231e41e0779f2c3572982051a))
ROM_END

ROM_START(hektor3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("system.rom0",   0x0000, 0x4000, CRC(bfc28204) SHA1(506dd25cbd83a95f53d61d611eb82aea0d80900b))
	ROM_LOAD("fra17_ay.rom1", 0x4000, 0x2000, CRC(e1e0d26e) SHA1(8b31c08896b00c018911b20b6ba7375befcaf320))
	ROM_RELOAD(               0x6000, 0x2000)
	ROM_LOAD("fra24_az.rom2", 0x8000, 0x4000, CRC(5cbf89d6) SHA1(b4a94eb0ba548e281c24ff118ddaca4fe66802fa))
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT   COMPAT   MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME             FLAGS */
COMP( 1981, hektor,  hektor2, 0,       hektor,   hektor,   hektor_state,   empty_init, "The Open University",  "Hektor",            0 )
COMP( 1982, hektor2, 0,       0,       hektor2,  hektor,   hektor_state,   empty_init, "The Open University",  "Hektor II",         0 )
COMP( 1984, hektor3, 0,       0,       hektor3,  hektor3,  hektor3_state,  empty_init, "The Open University",  "Hektor III",        0 )
