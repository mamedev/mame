// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
/************************************************\
* Multitech Micro Professor 1                    *
*                                                *
*     CPU: Z80 @ 1.79 MHz                        *
*     ROM: 4-kilobyte ROM monitor                *
*     RAM: 4 kilobytes                           *
*   Input: Hex keypad                            *
* Storage: Cassette tape                         *
*   Video: 6x 7-segment LED display              *
*   Sound: Speaker                               *
\************************************************/

/*

    Keys:
        0-9,A-F : hexadecimal numbers
        ADR : enter an address to work with. After the 4 digits are entered,
              the data at that address shows, and you can modify the data.
        +   : Enter the data into memory, and increment the address by 1.
        GO  : execute the program located at the current address.

    Pasting:
        0-F : as is
        +   : ^
        -   : V
        ADDR : -
        DATA : =
        GO : X
        PC : P

    Test Paste:
        -1800=11^22^33^44^55^66^77^88^99^-1800
        Now press up-arrow to confirm the data has been entered.

    TODO:
    - crt board
    - clickable artwork
    - computer can't keep up with paste
    - paste only set up for mpf1

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/mpf1/slot.h"
#include "imagedev/cassette.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "mpf1.lh"
#include "mpf1b.lh"
#include "mt80z.lh"


namespace {

class mpf1_state : public driver_device
{
public:
	mpf1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_7seg_pwm(*this, "7seg_pwm")
		, m_cassette(*this, "cassette")
		, m_rom_region(*this, "maincpu")
		, m_rom_u7(*this, "rom_u7")
		, m_pc(*this, "PC%u", 0U)
		, m_special(*this, "SPECIAL")
		, m_leds(*this, "led%u", 0U)
	{ }

	void mpf1(machine_config &config);
	void mpf1b(machine_config &config);
	void mt80z(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_special );

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pwm_display_device> m_7seg_pwm;
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<uint8_t> m_rom_region;
	required_device<generic_slot_device> m_rom_u7;
	required_ioport_array<6> m_pc;
	required_ioport m_special;
	output_finder<2> m_leds;

	void mpf1_io_map(address_map &map) ATTR_COLD;
	void mpf1_map(address_map &map) ATTR_COLD;
	void mpf1_step(address_map &map) ATTR_COLD;

	uint8_t rom_r(offs_t offset);
	uint8_t step_r(offs_t offset);
	uint8_t ppi_pa_r();
	void ppi_pb_w(uint8_t data);
	void ppi_pc_w(uint8_t data);

	int m_break = 0;
	int m_m1 = 0;

	uint8_t m_select = 0;
};

/* Address Maps */

void mpf1_state::mpf1_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x2fff).r(FUNC(mpf1_state::rom_r));
}

void mpf1_state::mpf1_step(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(mpf1_state::step_r));
}

void mpf1_state::mpf1_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x3c).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).mirror(0x3c).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x80, 0x83).mirror(0x3c).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

uint8_t mpf1_state::rom_r(offs_t offset)
{
	if (m_rom_u7->exists())
		return m_rom_u7->read_rom(offset & 0x1fff);
	else
		return m_rom_region[offset + 0x2000];
}

/* Input Ports */

INPUT_CHANGED_MEMBER( mpf1_state::trigger_special )
{
	m_maincpu->set_input_line(param, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( mpf1 )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 HL") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 HL'") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B I.IF") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F .PNC'") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 DE") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 DE'") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A SP") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E SZ.H'") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 BC") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 BC'") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 IY") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D .PNC") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE RD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 AF") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 AF'") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 IX") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C SZ.H") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE WR") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBR") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADDR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RELA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SBR") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DATA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MOVE") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("USER KEY") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_NMI)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_IRQ0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_RESET)
INPUT_PORTS_END

static INPUT_PORTS_START( mpf1b )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 /") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 >") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B STOP") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F LET") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 *") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 <") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A CALL") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E INPUT") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 -") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 =") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 P") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D PRINT") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONT") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 +") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 * *") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 M") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C NEXT") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SAVE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"IF/\u2227") PORT_CODE(KEYCODE_PGUP)   // U+2227 = ∧
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"TO/\u21e9") PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_DOWN) // U+21E9 = ⇩
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"THEN/\u2228") PORT_CODE(KEYCODE_PGDN) // U+2228 = ∨
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOTO") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET/~") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOSUB") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"FOR/\u21e7") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_UP) // U+21E7 = ⇧
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LIST") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"CLR/\u21e8") PORT_CODE(KEYCODE_INSERT) PORT_CODE(KEYCODE_RIGHT) // U+21E8 = ⇨
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"DEL/\u21e6") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_LEFT)     // U+21E6 = ⇦
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_NMI)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_IRQ0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_RESET)
INPUT_PORTS_END

static INPUT_PORTS_START( mt80z )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DUMP") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEXT") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PREV") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR BRK PT") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DATA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 IY") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C SZ.H") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SET BRK PT") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RELA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADDR") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D .PNC") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 IX") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 DE'") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 HL") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 HL'") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F .PNC'") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 BC'") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 AF'") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 DE") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A SP") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B I.IF") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E SZ.H'") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 BC") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 AF") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("USER KEY") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_NMI)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_IRQ0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1_state, trigger_special, INPUT_LINE_RESET)
INPUT_PORTS_END


/* Intel 8255A Interface */

uint8_t mpf1_state::ppi_pa_r()
{
	uint8_t data = 0x7f;

	/* bit 0 to 5, keyboard rows 0 to 5 */
	for (int row = 0; row < 6; row++)
		if (!BIT(m_select, row))
			data &= m_pc[row]->read();

	/* bit 6, user key */
	data &= m_special->read() & 1 ? 0xff : 0xbf;

	/* bit 7, tape input */
	data |= (m_cassette->input() > 0) ? 0x80 : 0x00;

	return data;
}

void mpf1_state::ppi_pb_w(uint8_t data)
{
	/* swap bits around for the 7-segment emulation */
	m_7seg_pwm->write_mx(bitswap<8>(data, 6, 1, 2, 0, 7, 5, 4, 3));
}

void mpf1_state::ppi_pc_w(uint8_t data)
{
	/* bits 0-5, led select and keyboard latch */
	m_select = data & 0x3f;
	m_7seg_pwm->write_my(m_select);

	/* bit 6, monitor break control */
	m_break = BIT(data, 6);

	if (m_break)
	{
		m_m1 = 0;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}

	/* bit 7, tape output, tone and led */
	m_leds[0] = !BIT(data, 7);
	m_speaker->level_w(BIT(data, 7));
	m_cassette->output(BIT(data, 7) ? 1.0 : -1.0);
}

uint8_t mpf1_state::step_r(offs_t offset)
{
	if (!m_break)
	{
		m_m1++;

		if (m_m1 == 5)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


/* Z80 Daisy Chain */

static const z80_daisy_config mpf1_daisy_chain[] =
{
	{ "ctc" },
	{ "pio" },
	{ nullptr }
};


/* Machine Initialization */

void mpf1_state::machine_start()
{
	m_leds.resolve();

	/* register for state saving */
	save_item(NAME(m_break));
	save_item(NAME(m_m1));
	save_item(NAME(m_select));
}

void mpf1_state::machine_reset()
{
	m_select = 0;
}


/* Machine Drivers */

void mpf1_state::mpf1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3.579545_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpf1_state::mpf1_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mpf1_state::mpf1_step);
	m_maincpu->set_addrmap(AS_IO, &mpf1_state::mpf1_io_map);
	m_maincpu->halt_cb().set_output("led1");
	m_maincpu->set_daisy_config(mpf1_daisy_chain);

	/* devices */
	z80pio_device &pio(Z80PIO(config, "pio", 3.579545_MHz_XTAL / 2));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 3.579545_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.in_pa_callback().set(FUNC(mpf1_state::ppi_pa_r));
	ppi.out_pb_callback().set(FUNC(mpf1_state::ppi_pb_w));
	ppi.out_pc_callback().set(FUNC(mpf1_state::ppi_pc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);

	/* video hardware */
	PWM_DISPLAY(config, m_7seg_pwm).set_size(6, 8);
	m_7seg_pwm->set_segmask(0x3f, 0xff);

	config.set_default_layout(layout_mpf1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* expansion */
	mpf1_exp_device &exp(MPF1_EXP(config, "exp", 3.579545_MHz_XTAL / 2, mpf1_exp_devices, nullptr));
	exp.set_program_space(m_maincpu, AS_PROGRAM);
	exp.set_io_space(m_maincpu, AS_IO);
	exp.int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	exp.nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	exp.wait_handler().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT);

	GENERIC_SOCKET(config, m_rom_u7, generic_linear_slot, "mpf1_rom", "bin,rom");

	SOFTWARE_LIST(config, "rom_ls").set_original("mpf1_rom").set_filter("IA");
}

void mpf1_state::mpf1b(machine_config &config)
{
	mpf1(config);

	config.set_default_layout(layout_mpf1b);

	subdevice<software_list_device>("rom_ls")->set_filter("IA,IB");
}

void mpf1_state::mt80z(machine_config &config)
{
	mpf1b(config);

	config.set_default_layout(layout_mt80z);

	// TODO: MT-80Z expansion board with switches (DIPs) and indicators (LEDs).
}


ROM_START( mpf1 )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("mpf1.u6", 0x0000, 0x0800, CRC(820030b1) SHA1(f618e2044b8f6e055e4d9a4c719b53bf16e22330))
ROM_END

ROM_START( mpf1b )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "basic", "BASIC")
	ROMX_LOAD("mpf1b.u6",    0x0000, 0x1000, CRC(28b06dac) SHA1(99cfbab739d71a914c39302d384d77bddc4b705b), ROM_BIOS(0))
	ROMX_LOAD("unknown.u7",  0x2000, 0x1000, CRC(d276ed6b) SHA1(a45fb98961be5e5396988498c6ed589a35398dcf), ROM_BIOS(0)) // TODO: this was previously labelled as BASIC, it's not and is unknown.
	ROM_SYSTEM_BIOS(1, "olsl1", "OLS-L1")
	ROMX_LOAD("ols_l1.u6",   0x0000, 0x1000, CRC(b60249ce) SHA1(78e0e8874d1497fabfdd6378266d041175e3797f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "olsl2", "OLS-L2")
	ROMX_LOAD("ols_l1-3.u6", 0x0000, 0x1000, CRC(2f038b4b) SHA1(58ba895d2683d26d48da36afdcee9f1dae2f7b7c), ROM_BIOS(2))
	ROMX_LOAD("ols_l2.u7",   0x2000, 0x1000, CRC(77e51dde) SHA1(45d2c4b520291d5a346a0c3567bcb4b4406675f9), ROM_BIOS(2))
ROM_END

ROM_START( cs113 )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cs113.bin", 0x0000, 0x2000, CRC(d293a83b) SHA1(890e8b478e38fed6326b4151bf68c587f82f6a94))
ROM_END

ROM_START( mt80z )
	ROM_REGION(0x3000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("fox.u6", 0x0000, 0x0800, CRC(298ffc59) SHA1(26d144aa63581407dd2f14437646afd807d2bb34))
ROM_END

} // anonymous namespace


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY            FULLNAME                     FLAGS */
COMP( 1981, mpf1,   0,      0,      mpf1,    mpf1,  mpf1_state, empty_init, "Multitech",       "Micro-Professor 1",         0 )
COMP( 1982, mpf1b,  mpf1,   0,      mpf1b,   mpf1b, mpf1_state, empty_init, "Multitech",       "Micro-Professor 1B",        0 )
COMP( 198?, cs113,  0,      0,      mpf1,    mpf1,  mpf1_state, empty_init, "Sciento b.v.",    "Robot Training Arm CS-113", MACHINE_NOT_WORKING )
COMP( 1982, mt80z,  mpf1,   0,      mt80z,   mt80z, mpf1_state, empty_init, "E&L Instruments", "MT-80Z",                    0 )
