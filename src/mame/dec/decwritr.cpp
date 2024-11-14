// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/*************************************************************************

    decwritr.c
    Digital Equipment Corporation
    DECwriter III (LA120) Teletype/Teleprinter, 1978

**************************************************************************/

// tech manual: http://manx.classiccmp.org/mirror/vt100.net/docs/la120-tm/la120tm1.pdf

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/74259.h"
#include "dc305.h"
#include "machine/er1400.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define KBD_VERBOSE 1
#define LED_VERBOSE 0
#define DC305_VERBOSE 0

//**************************************************************************
//  DRIVER STATE
//**************************************************************************

class decwriter_state : public driver_device
{
public:
	// constructor
	decwriter_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "beeper"),
		m_usart(*this, "usart"),
		m_nvm(*this, "nvm"),
		m_ledlatch(*this, "ledlatch"),
		m_prtlsi(*this, "prtlsi"),
		m_col_array(*this, "COL%X", 0U)
	{
	}

	void la120(machine_config &config);
private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	IRQ_CALLBACK_MEMBER(inta_cb);

	uint8_t la120_KBD_r(offs_t offset);
	void la120_LED_w(offs_t offset, uint8_t data);
	uint8_t la120_NVR_r();
	void la120_NVR_w(offs_t offset, uint8_t data);
	uint8_t la120_DC305_r(offs_t offset);
	void la120_DC305_w(offs_t offset, uint8_t data);

	void la120_io(address_map &map) ATTR_COLD;
	void la120_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	//virtual void machine_reset();

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;
	required_device<i8251_device> m_usart;
	required_device<er1400_device> m_nvm;
	required_device<ls259_device> m_ledlatch;
	required_device<dc305_device> m_prtlsi;

	required_ioport_array<16> m_col_array;
	uint8_t m_led_7seg_counter = 0;
	uint8_t m_led_7seg[4]{};
};

IRQ_CALLBACK_MEMBER( decwriter_state::inta_cb )
{
	// one wait state inserted
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-1);

	return m_prtlsi->inta();
}

uint8_t decwriter_state::la120_KBD_r(offs_t offset)
{
	/* for reading the keyboard array, addr bits 5-11 are ignored.
	 * a15 a14 a13 a12 a11 a10  a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
	 *   0   0   1   1   x   x   x   x   x   x   x   *   *   *   *   *
	 *                                               |   |   \---\---\--- column select
	 *                                               |   \--------------- kbd_banksel0
	 *                                               \------------------- kbd_banksel1
	 *      if both banks are selected, the resulting row read is a binary OR of both (LA120-TM1, page 4-5)
	 *
	 * d7 d6 d5 d4 d3 d2 d1 d0
	 *  \--\--\--\--\--\--\--\-- read from rows
	 */
	uint8_t code = 0;
	if (offset&0x8) code |= m_col_array[offset&0x7]->read();
	if (offset&0x10) code |= m_col_array[(offset&0x7)+8]->read();
	if (KBD_VERBOSE)
		logerror("Keyboard column %X read, returning %02X\n", offset&0xF, code);
	return code;
}

void decwriter_state::la120_LED_w(offs_t offset, uint8_t data)
{
	/* for writing the keyboard array, addr bits 5-11 are ignored.
	 * a15 a14 a13 a12 a11 a10  a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
	 *   0   0   1   1   x   x   x   x   x   x   x   0   *   *   *   0    turn OFF LED # <a3:a1> and clear 7seg counter
	 *   0   0   1   1   x   x   x   x   x   x   x   0   *   *   *   1    turn ON LED # <a3:a1> and clear 7seg counter
	 *   0   0   1   1   x   x   x   x   x   x   x   1   *   *   *   *    display a digit <a3:a0> in the digit pointed to by 7seg counter and increment counter
	 * data bus is UNUSED.
	 */
	if (!(offset&0x10)) // we're updating an led state
	{
		if (LED_VERBOSE)
			logerror("Updated LED status array: LED #%d is now %d\n", ((offset&0xe)>>1), (offset&1));
		m_ledlatch->write_bit((~offset & 0xe) >> 1, !BIT(offset, 0));
		m_led_7seg_counter = 0;
	}
	else // we're updating the 7segment display
	{
		m_led_7seg_counter++;
		m_led_7seg_counter &= 0xF;
		if (LED_VERBOSE)
			logerror("Updated 7seg display: displaying a digit of %d in position %d\n", (offset&0xF)^0xF, m_led_7seg_counter-1);
		if ((m_led_7seg_counter >= 1) && (m_led_7seg_counter <= 4))
		{
			m_led_7seg[m_led_7seg_counter-1] = (offset&0xF)^0xF;
		}
	}
	popmessage("LEDs: %c %c %c %c : %s  %s  %s  %s  %s  %s  %s  %s\n",
	m_led_7seg[3]+0x30, m_led_7seg[2]+0x30, m_led_7seg[1]+0x30, m_led_7seg[0]+0x30,
	(!m_ledlatch->q0_r())?"ON LINE":"-------",
	(!m_ledlatch->q1_r())?"LOCAL":"-----",
	(!m_ledlatch->q2_r())?"ALT CHR SET":"-----------",
	(!m_ledlatch->q3_r())?"<LED4>":"-----",
	(!m_ledlatch->q4_r())?"CTS":"---",
	(!m_ledlatch->q5_r())?"DSR":"---",
	(!m_ledlatch->q6_r())?"SET-UP":"------",
	(!m_ledlatch->q7_r())?"PAPER OUT":"---------" );
}

/* control lines:
   3 2 1
   0 0 0 Standby
   0 0 1 Read
   0 1 0 Erase
   0 1 1 Write
   1 0 0 <unused>
   1 0 1 Shift data out
   1 1 0 Accept address
   1 1 1 Accept data
   */
uint8_t decwriter_state::la120_NVR_r()
{
	// one wait state inserted
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-1);

	return (!m_nvm->data_r() << 7) | 0x7f;
}

void decwriter_state::la120_NVR_w(offs_t offset, uint8_t data)
{
	// one wait state inserted
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-1);

	// ER1400 has negative logic, but 7406 inverters are used
	m_nvm->c3_w(BIT(offset, 10));
	m_nvm->c2_w(BIT(offset, 9));
	m_nvm->c1_w(BIT(offset, 8));
	m_nvm->clock_w(BIT(offset, 0));

	// C2 is used to disable pullup on data line
	m_nvm->data_w(BIT(offset, 9) ? !BIT(data, 7) : 1);
}

/* todo: fully reverse engineer DC305 ASIC */
/* read registers: all 4 registers read the same set of 8 bits, but what register is being read may be selectable by writing
   Tech manual implies this register is an 8-bit position counter of where the carriage head currently is located.
   0 = 1 = 2 = 3
   data bits:
   76543210
   |||||||\- ?
   ||||||\-- ?
   |||||\--- ?
   ||||\---- ?
   |||\----- ?
   ||\------ ?
   |\------- ?
   \-------- ?
 */
uint8_t decwriter_state::la120_DC305_r(offs_t offset)
{
	// one wait state inserted
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-1);

	uint8_t data = m_prtlsi->read(offset);
	if (DC305_VERBOSE)
		logerror("DC305 Read from offset %01x, returning %02X\n", offset, data);
	return data;
}
/* write registers:
   0 = ? (a bunch of data written here on start, likely motor control and setup bits)
   1 = ? (one byte written here, possibly voltage control, 0x00 or could be dot fifo write?)
   2 = ?
   3 = ?
   there are at least two bits in here to enable the 2.5ms tick interrupt(rst3) and the dot interrupt/linefeed(rtc expired) interrupt(rst5)
   the dot fifo is 4 bytes long, dot int fires when it is half empty
   at least 3 bits control the speaker/buzzer which can be on or off, at least two volume levels, and at least two frequencies, 400hz or 2400hz
   two quadrature lines from the head servomotor connect here to allow the dc305 to determine motor position; one pulses when the motor turns clockwise and one when it turns counterclockwise. the head stop is found when the pulses stop, which firmware uses to find the zero position.
 */
void decwriter_state::la120_DC305_w(offs_t offset, uint8_t data)
{
	// one wait state inserted
	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-1);

	m_prtlsi->write(offset, data);
	if (DC305_VERBOSE)
		logerror("DC305 Write of %02X to offset %01X\n", data, offset);
}

/*
 * 8080  address map (x = ignored; * = selects address within this range)
   a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
   0   0   0   0   *   *   *   *   *   *   *   *   *   *   *   *     R      ROMS0 (e6 first half OR e6,e8)
   0   0   0   1   *   *   *   *   *   *   *   *   *   *   *   *     R      ROMS1 (e6 second half OR e12,e17)
   0   0   1   0   0   *   *   *   *   *   *   *   *   *   *   *     R      ROMS2 (e4)
   0   0   1   0   1   *   *   *   *   *   *   *   *   *   *   *     R      ROMS2 (open bus)
   0   0   1   1   x   x   x   x   x   x   x   *   *   *   *   *     RW     KBD(R)/LED(W)
   0   1   0   0   x   x   *   *   *   *   *   *   *   *   *   *     RW     RAM0 (e7,e13)
   0   1   0   1   x   x   *   *   *   *   *   *   *   *   *   *     RW     RAM1 (e9,e18)
   0   1   1   0   x   *   *   *   x   x   x   x   x   x   x   *     RW     NVM (ER1400,e39)
   0   1   1   1   x   x   x   x   x   x   x   x   x   x   *   *     RW     PTR (DC305 ASIC,e25)
   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *            Expansion space (open bus)
 */
void decwriter_state::la120_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x27ff).rom();
	map(0x3000, 0x301f).rw(FUNC(decwriter_state::la120_KBD_r), FUNC(decwriter_state::la120_LED_w)).mirror(0xFE0); // keyboard read, write to status and 7seg LEDS
	map(0x4000, 0x43ff).mirror(0x0c00).ram(); // 1k 'low ram'
	map(0x5000, 0x53ff).mirror(0x0c00).ram(); // 1k 'high ram'
	map(0x6000, 0x67ff) /*.mirror(0x08fe)*/ .mirror(0x800).rw(FUNC(decwriter_state::la120_NVR_r), FUNC(decwriter_state::la120_NVR_w)); // ER1400 EAROM; a10,9,8 are c3,2,1, a0 is clk, data i/o on d7, d0 always reads as 0 (there may have once been a second er1400 with data i/o on d0, sharing same address controls as the d7 one, not populated on shipping boards), d1-d6 read open bus
	map(0x7000, 0x7003).mirror(0x0ffc).rw(FUNC(decwriter_state::la120_DC305_r), FUNC(decwriter_state::la120_DC305_w)); // DC305 printer controller ASIC stuff; since this can generate interrupts (dot interrupt, lf interrupt, 2.5ms interrupt) this needs to be split to its own device.
	// 8000-ffff is reserved for expansion (i.e. unused, open bus)
}

/*
 * 8080 IO address map (x = ignored; * = selects address within this range)
 * (a15 to a8 are latched the same value as a7-a0 on the 8080 and 8085)
   a7  a6  a5  a4  a3  a2  a1  a0
   0   x   x   x   x   x   0   0     RW     8251 Data
   0   x   x   x   x   x   0   1     RW     8251 Status/Control
   0   x   x   x   x   x   1   x     RW     Flags Read/Write
   1   x   x   x   x   x   x   x     RW     Expansion (Open bus)
 */
void decwriter_state::la120_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).mirror(0x7C).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)); // 8251 Status/Control
	//map(0x02, 0x02).mirror(0x7D); // other io ports, serial loopback etc, see table 4-9 in TM
	// 0x80-0xff are reserved for expansion (i.e. unused, open bus)
	map.global_mask(0xff);
}

/* Input ports */
static INPUT_PORTS_START( la120 )
	PORT_START("COL0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Local LF") PORT_CODE(KEYCODE_F12)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_START("COL1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Local FF") PORT_CODE(KEYCODE_F11)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // this is actually the cover open interlock sensor
	PORT_START("COL2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HERE IS") PORT_CODE(KEYCODE_F10)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(alternate) Opt LF") PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // this is actually the paper out interlock sensor
	PORT_START("COL3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_LALT) PORT_CHAR(10)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE LOCAL") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("COL4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num -") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Caps lock") PORT_CODE(KEYCODE_CAPSLOCK) // This key has a physical toggle
	PORT_START("COL5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_START("COL6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_START("COL7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
		PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Set Up") PORT_CODE(KEYCODE_F5)
	PORT_START("COL8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COL9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLA")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLB")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLC")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLD")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLE")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("COLF")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num Enter") PORT_CODE(KEYCODE_ENTER_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) // this is actually the RO flag
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("View") PORT_CODE(KEYCODE_LWIN)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
		PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void decwriter_state::machine_start()
{
	m_led_7seg_counter = 0;
	m_led_7seg[0] = m_led_7seg[1] = m_led_7seg[2] = m_led_7seg[3] = 0xF;
}

/*
void decwriter_state::machine_reset()
{
}
*/

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void decwriter_state::la120(machine_config &config)
{
	I8080A(config, m_maincpu, XTAL(18'000'000) / 9); // 18Mhz xtal on schematics, using an i8224 clock divider/reset sanitizer IC
	m_maincpu->set_addrmap(AS_PROGRAM, &decwriter_state::la120_mem);
	m_maincpu->set_addrmap(AS_IO, &decwriter_state::la120_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(decwriter_state::inta_cb));

	/* video hardware */
	//TODO: no actual screen! has 8 leds above the keyboard (similar to vt100/vk100) and has 4 7segment leds for showing an error code.
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(decwriter_state::screen_update));
	screen.set_size(640,480);
	screen.set_visarea_full();
	screen.set_refresh_hz(30);

	DC305(config, m_prtlsi, XTAL(18'000'000) / 9);
	m_prtlsi->rxc_callback().set("usart", FUNC(i8251_device::write_rxc));
	m_prtlsi->txc_callback().set("usart", FUNC(i8251_device::write_txc));
	m_prtlsi->int_callback().set("mainint", FUNC(input_merger_device::in_w<0>));

	LS259(config, m_ledlatch); // E2 on keyboard
	m_ledlatch->q_out_cb<0>().set_output("led1").invert(); // ON LINE
	m_ledlatch->q_out_cb<1>().set_output("led2").invert(); // LOCAL
	m_ledlatch->q_out_cb<2>().set_output("led3").invert(); // ALT CHAR SET
	m_ledlatch->q_out_cb<3>().set_output("led4").invert();
	m_ledlatch->q_out_cb<4>().set_output("led5").invert(); // CTS
	m_ledlatch->q_out_cb<5>().set_output("led6").invert(); // DSR
	m_ledlatch->q_out_cb<6>().set_output("led7").invert(); // SETUP
	m_ledlatch->q_out_cb<7>().set_output("led8").invert(); // PAPER OUT

	//config.set_default_layout(layout_la120);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_speaker, 786).add_route(ALL_OUTPUTS, "mono", 0.50); // TODO: LA120 speaker is controlled by asic; VT100 has: 7.945us per serial clock = ~125865.324hz, / 160 clocks per char = ~ 786 hz

	/* i8251 */
	i8251_device &usart(I8251(config, "usart", XTAL(18'000'000) / 9));
	usart.txd_handler().set("eia", FUNC(rs232_port_device::write_txd));
	usart.dtr_handler().set("eia", FUNC(rs232_port_device::write_dtr));
	usart.rts_handler().set("eia", FUNC(rs232_port_device::write_rts));
	usart.rxrdy_handler().set("mainint", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "mainint").output_handler().set_inputline(m_maincpu, 0);

	rs232_port_device &rs232(RS232_PORT(config, "eia", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("usart", FUNC(i8251_device::write_dsr));

	ER1400(config, m_nvm);
}



//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( la120 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// later romset, with 23-003e2.e6, 23-004e2.e8, 23-005e2.e12, 23-006e2.e17 replaced by one rom, 23-038e4.e6 which may be a concatenation of the old roms, unclear.
	ROM_LOAD( "23-038e4-00.e6", 0x0000, 0x2000, CRC(cad4eb09) SHA1(d5db117da363d36817476f906251ea4ee1cb14b8))
	ROM_LOAD( "23-007e2-00.e4", 0x2000, 0x0800, CRC(41eaebf1) SHA1(c7d05417b24b853280d1636776d399a0aea34720)) // used by both earlier and later romset
	// there is an optional 3 roms, european and APL (and BOTH) rom which goes from 2000-2fff in e4, all undumped.
	// there is another romset used on the Bell Teleprinter 1000 (Model LAS12) which I believe is 23-004e4.e6 and 23-086e2.e4
ROM_END

} // anonymous namespace


//**************************************************************************
//  DRIVERS
//**************************************************************************
/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS            INIT        COMPANY                          FULLNAME                 FLAGS */
COMP( 1978, la120, 0,      0,      la120,   la120, decwriter_state, empty_init, "Digital Equipment Corporation", "DECwriter III (LA120)", MACHINE_IS_SKELETON )
