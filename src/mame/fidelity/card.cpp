// license:BSD-3-Clause
// copyright-holders:Kevin Horton, Jonathan Gevaryahu, Sandro Ronco, hap
/*******************************************************************************

Fidelity electronic card games
- Bridge Challenger (BRC)
- Advanced Bridge Challenger (UBC/UB2)
- Voice Bridge Challenger (VBRC/BV2)
- Bridge Challenger III (BV3)
- Gin & Cribbage Challenger (GIN)
- *Skat Challenger (SKT)

*: not dumped yet

Card Challenger (model CDC) was announced but not released, it was supposed to be
a card games console, with games on separate cartridges. Maybe some of the games
were already finished, and used later for GIN and SKT.

NOTE: The card scanner is simulated, but the player is kind of forced to cheat
and has to peek at the card before it is scanned.

BTANB:
- on BRC, the 2 jokers are identified as Spade 1 and Spade 2

TODO:
- verify if BV2 is a newer program version than VBRC
- the VFD scrolls around 30% too slow compared to the real one, probably depends
  on how many T1 clock edges the 8041 can detect (see mcu_t1_r)

================================================================================

Voice Bridge Challenger (Model VBRC, later reissued as Model 7002/BV2)
and Bridge Challenger 3 (Model 7014)
(which both share the same* hardware)
--------------------------------
* The Bridge Challenger 3 does not actually have the 8 LEDs nor the
latches which operate them populated and the plastic indicator cap locations
are instead are covered by a piece of plastic, but they do work if manually
added.

RE notes by Kevin Horton

This unit is similar in construction kinda to the chess challengers, however it
has an 8041 which does ALL of the system I/O. The Z80 has NO IO AT ALL other than
what is performed through the 8041!

The main CPU is a Z80 running at 2.5MHz

INT connects to VCC (not used)
NMI connects to VCC (not used)
RST connects to power on reset, and reset button

The 8041 runs at 5MHz.

Memory Map:
-----------
0000-1FFF: 8K 101-64108 ROM
2000-3FFF: 8K 101-64109 ROM
4000-5FFF: 8K 101-64110 ROM
6000-7FFF: 1K of RAM (2114 * 2)
8000-DFFF: unused
E000-FFFF: write to TSI chip

NOTE: when the TSI chip is written to, the CPU IS STOPPED. The CPU will run again
when the word is done being spoken. This is because D0-D5 run to the TSI chip directly.

The TSI chip's ROM is 4K, and is marked 101-32118. The clock is the same as the Chess
Challengers- 470K/100pf which gives a frequency around 25KHz or so.

Port Map:
---------
00-FF: 8041 I/O ports (A0 selects between the two)

8041 pinout:
------------
(note: columns are pulled up with 10K resistors)

P10 - column H, RD LED, VFD grid 0
P11 - column G, DB LED, VFD grid 1
P12 - column F, <>V LED, VFD grid 2
P13 - column E, ^V LED, VFD grid 3
P14 - column D, W LED, VFD grid 4
P15 - column C, S LED, VFD grid 5
P16 - column B, E LED, VFD grid 6
P17 - column A, N LED, VFD grid 7

P20 - I/O expander
P21 - I/O expander
P22 - I/O expander
P23 - I/O expander
P24 - row 0 through inverter
P25 - row 1 through inverter
P26 - row 2 through inverter
P27 - row 3 through inverter

PROG - I/O expander

T0 - optical card sensor (high = bright/reflective, low = dark/non reflective)
T1 - connects to inverter, then 5MHz/4

D8243C I/O expander:
--------------------
P4.0 - segment M
P4.1 - segment L
P4.2 - segment N
P4.3 - segment E

P5.0 - segment D
P5.1 - segment I
P5.2 - segment K
P5.3 - segment J

P6.0 - segment A
P6.1 - segment B
P6.2 - segment F
P6.3 - segment G

P7.0 - LED enable (high = LEDs can be lit. low = LEDs will not light)
P7.1 - goes through inverter, to pads that are not used
P7.2 - segment C
P7.3 - segment H

Button matrix:
--------------
the matrix is composed of 8 columns by 4 rows.

     A  B  C  D     E  F  G  H
     -------------------------
0-   RE xx CL EN    J  Q  K  A
1-   BR PB DB SC    7  8  9 10
2-   DL CV VL PL    3  4  5  6
3-   cl di he sp   NT  P  1  2

xx - speaker symbol
cl - clubs symbol
di - diamonds symbol
he - hearts symbol
sp - spades symbol

NOTE: RE is not wired into the matrix, and is run separately out.

There are 8 LEDs, and an 8 digit 14 segment VFD with commas and periods.
This display is the same one as can be found on the speak and spell.

       A       * comma
  ***********  *
 * *I  *J K* *
F*  *  *  *  *B
 *   * * *   *
  G**** *****H
 *   * * *   *
E*  *  *  *  *C
 * *N  *M L* *
  ***********  *decimal point
       D

The digits of the display are numbered left to right, 0 through 7 and are controlled
by the grids. hi = grid on, hi = segment on.

A detailed description of the hardware can be found also in the patent 4,373,719.

cards:
------
Playing cards have a 9-bit barcode on the face side near the edge. Swipe them downward
against the card scanner and the game will detect the card.
Barcode sync bits(msb and lsb) are the same for each card so that leaves 7 bits of data:
2 for suit, 4 for value, and 1 for parity so the card can be scanned backwards.

Two card decks exist (red and blue), each has the same set of barcodes.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_brc.lh"
#include "fidel_bv3.lh"
#include "fidel_gin.lh"


namespace {

class card_state : public driver_device
{
public:
	card_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_i8243(*this, "i8243"),
		m_display(*this, "display"),
		m_speech(*this, "speech"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void brc(machine_config &config);
	void vbrc(machine_config &config);
	void bv3(machine_config &config);
	void gin(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(start_scan);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void brc_base(machine_config &config);

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<i8041a_device> m_mcu;
	required_device<i8243_device> m_i8243;
	required_device<pwm_display_device> m_display;
	optional_device<s14001a_device> m_speech;
	optional_device<dac_1bit_device> m_dac;
	required_ioport_array<8> m_inputs;

	u32 m_barcode = 0;
	u16 m_vfd_data = 0;
	u8 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(barcode_shift) { m_barcode >>= 1; }

	// I/O handlers
	void update_display();
	void speech_w(u8 data);
	void mcu_p1_w(u8 data);
	u8 mcu_p2_r();
	int mcu_t0_r();
	int mcu_t1_r();
	template<int P> void ioexp_port_w(uint8_t data);
};

void card_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_barcode));
	save_item(NAME(m_vfd_data));
	save_item(NAME(m_inp_mux));
}

INPUT_CHANGED_MEMBER(card_state::reset_button)
{
	// reset button is directly wired to maincpu/mcu RESET pins
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	m_mcu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// misc handlers

void card_state::update_display()
{
	// 14seg VFD segments, d15(12) is extra LED
	u16 outdata = bitswap<16>(m_vfd_data,12,13,1,6,5,2,0,7,15,11,10,14,4,3,9,8);
	m_display->matrix(m_inp_mux, outdata);

	// I8243 P71 + I8041 P17 is tone (not on speech model)
	if (m_dac != nullptr)
		m_dac->write(BIT(~m_inp_mux, 7) & BIT(m_vfd_data, 13));
}

void card_state::speech_w(u8 data)
{
	if (m_speech == nullptr)
		return;

	m_speech->data_w(data & 0x3f);
	m_speech->start_w(1);
	m_speech->start_w(0);
}


// card scanner

INPUT_CHANGED_MEMBER(card_state::start_scan)
{
	if (!newval)
		return;

	u32 code = (u32)param;
	m_barcode = 0;

	// convert bits to rising/falling edges
	for (int i = 0; i < 9; i++)
	{
		m_barcode <<= 2;
		m_barcode |= 1 << (code & 1);
		code >>= 1;
	}

	// 6*white at msb for card edge
	m_barcode <<= 8;
	m_barcode = ~m_barcode;
}


// I8243 I/O expander

template<int P>
void card_state::ioexp_port_w(uint8_t data)
{
	// P4x-P7x: digit segment data
	m_vfd_data = (m_vfd_data & ~(0xf << (4*P))) | ((data & 0xf) << (4*P));
	update_display();
}


// I8041 MCU

void card_state::mcu_p1_w(u8 data)
{
	// P10-P17: input mux, digit select
	m_inp_mux = data;
	update_display();
}

u8 card_state::mcu_p2_r()
{
	// P20-P23: I8243 P2
	u8 data = m_i8243->p2_r() & 0x0f;

	// P24-P27: multiplexed inputs (active low)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 4;

	return data ^ 0xf0;
}

int card_state::mcu_t0_r()
{
	// T0: card scanner light sensor (1=white, 0=black/none)
	return m_barcode & 1;
}

int card_state::mcu_t1_r()
{
	// T1: xtal / 4 (do *2 for high-low transitions)
	return (machine().time().as_ticks(5_MHz_XTAL / 4 * 2)) & 1;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void card_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x63ff).mirror(0x1c00).ram();
	map(0xe000, 0xe000).mirror(0x1fff).w(FUNC(card_state::speech_w));
}

void card_state::main_io(address_map &map)
{
	map.global_mask(0x01);
	map(0x00, 0x01).rw(m_mcu, FUNC(i8041a_device::upi41_master_r), FUNC(i8041a_device::upi41_master_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( scanner )
	PORT_START("CARDS.0") // spades + jokers
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x6f) PORT_NAME("Scan: Spades A")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x47) PORT_NAME("Scan: Spades 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xd7) PORT_NAME("Scan: Spades 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x27) PORT_NAME("Scan: Spades 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xb7) PORT_NAME("Scan: Spades 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x77) PORT_NAME("Scan: Spades 6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xe7) PORT_NAME("Scan: Spades 7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x0f) PORT_NAME("Scan: Spades 8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x9f) PORT_NAME("Scan: Spades 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x5f) PORT_NAME("Scan: Spades 10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xcf) PORT_NAME("Scan: Spades J")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x3f) PORT_NAME("Scan: Spades Q")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xaf) PORT_NAME("Scan: Spades K")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xf9) PORT_NAME("Scan: Joker 1")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xed) PORT_NAME("Scan: Joker 2")

	PORT_START("CARDS.1") // hearts
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x7b) PORT_NAME("Scan: Hearts A")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x53) PORT_NAME("Scan: Hearts 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xc3) PORT_NAME("Scan: Hearts 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x33) PORT_NAME("Scan: Hearts 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xa3) PORT_NAME("Scan: Hearts 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x63) PORT_NAME("Scan: Hearts 6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xf3) PORT_NAME("Scan: Hearts 7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x1b) PORT_NAME("Scan: Hearts 8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x8b) PORT_NAME("Scan: Hearts 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x4b) PORT_NAME("Scan: Hearts 10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xdb) PORT_NAME("Scan: Hearts J")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x2b) PORT_NAME("Scan: Hearts Q")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xbb) PORT_NAME("Scan: Hearts K")

	PORT_START("CARDS.2") // clubs
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x69) PORT_NAME("Scan: Clubs A")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x95) PORT_NAME("Scan: Clubs 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xd1) PORT_NAME("Scan: Clubs 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x93) PORT_NAME("Scan: Clubs 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xb1) PORT_NAME("Scan: Clubs 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x71) PORT_NAME("Scan: Clubs 6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xe1) PORT_NAME("Scan: Clubs 7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x87) PORT_NAME("Scan: Clubs 8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x99) PORT_NAME("Scan: Clubs 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x59) PORT_NAME("Scan: Clubs 10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xc9) PORT_NAME("Scan: Clubs J")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x39) PORT_NAME("Scan: Clubs Q")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xa9) PORT_NAME("Scan: Clubs K")

	PORT_START("CARDS.3") // diamonds
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x7d) PORT_NAME("Scan: Diamonds A")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x55) PORT_NAME("Scan: Diamonds 2")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xc5) PORT_NAME("Scan: Diamonds 3")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x35) PORT_NAME("Scan: Diamonds 4")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xa5) PORT_NAME("Scan: Diamonds 5")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x65) PORT_NAME("Scan: Diamonds 6")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xf5) PORT_NAME("Scan: Diamonds 7")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x1d) PORT_NAME("Scan: Diamonds 8")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x8d) PORT_NAME("Scan: Diamonds 9")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x4d) PORT_NAME("Scan: Diamonds 10")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xdd) PORT_NAME("Scan: Diamonds J")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0x2d) PORT_NAME("Scan: Diamonds Q")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, start_scan, 0xbd) PORT_NAME("Scan: Diamonds K")
INPUT_PORTS_END

static INPUT_PORTS_START( brc )
	PORT_INCLUDE( scanner )

	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("A")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("K")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("Q")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("P")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("J")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("NT")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("EN")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("SC")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("PL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Spades")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("DB")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("VL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Hearts")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("PB")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("CV")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Diamonds")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("BR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("DL")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Clubs")

	PORT_START("RESET") // is not on matrix IN.7 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, reset_button, 0) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( bv3 )
	PORT_INCLUDE( brc )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Ace")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("King")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Quit")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Jack")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("No Trump")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Yes/Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("No/Pass")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Player")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Double")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Score")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Auto")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Conv")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Review")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Dealer")

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, card_state, reset_button, 0) PORT_NAME("Reset")
INPUT_PORTS_END

static INPUT_PORTS_START( gin )
	PORT_INCLUDE( bv3 )

	PORT_MODIFY("IN.2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Human")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Computer")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Yes/Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("No")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Hand")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Score")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Conv")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Quit")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Language")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Knock")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Dealer")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void card_state::brc_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &card_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &card_state::main_io);
	config.set_perfect_quantum(m_maincpu);

	I8041A(config, m_mcu, 5_MHz_XTAL);
	m_mcu->p1_out_cb().set(FUNC(card_state::mcu_p1_w));
	m_mcu->p2_in_cb().set(FUNC(card_state::mcu_p2_r));
	m_mcu->p2_out_cb().set(m_i8243, FUNC(i8243_device::p2_w));
	m_mcu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));
	m_mcu->t0_in_cb().set(FUNC(card_state::mcu_t0_r));
	m_mcu->t1_in_cb().set(FUNC(card_state::mcu_t1_r));

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(card_state::ioexp_port_w<0>));
	m_i8243->p5_out_cb().set(FUNC(card_state::ioexp_port_w<1>));
	m_i8243->p6_out_cb().set(FUNC(card_state::ioexp_port_w<2>));
	m_i8243->p7_out_cb().set(FUNC(card_state::ioexp_port_w<3>));

	TIMER(config, "barcode_shift").configure_periodic(FUNC(card_state::barcode_shift), attotime::from_msec(2));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_segmask(0xff, 0x3fff);
	config.set_default_layout(layout_fidel_brc);
}

void card_state::brc(machine_config &config)
{
	brc_base(config);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void card_state::vbrc(machine_config &config)
{
	brc_base(config);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->bsy().set_inputline("maincpu", Z80_INPUT_LINE_WAIT);
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}

void card_state::bv3(machine_config &config)
{
	brc(config);
	config.set_default_layout(layout_fidel_bv3);
}

void card_state::gin(machine_config &config)
{
	brc(config);
	config.set_default_layout(layout_fidel_gin);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bridgec ) // model BRC, PCB label 510-4020-1C
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("bridge-1", 0x0000, 0x2000, CRC(83319c59) SHA1(db2dffb99320cbd60d33dcf9bb56a51266a041b2) ) // NEC 2364C 069
	ROM_LOAD("bridge-2", 0x2000, 0x2000, CRC(7c54f9bc) SHA1(e57221ea3e22238192bb260ad5e385f5179bb34e) ) // NEC 2364C 070
	ROM_LOAD("bridge-3", 0x4000, 0x1000, CRC(d3cda2e3) SHA1(69b62fa22b388a922abad4e89c78bdb01a5fb322) ) // NEC 2332C 188

	ROM_REGION( 0x0400, "mcu", 0 )
	ROM_LOAD("d8041c_531", 0x0000, 0x0400, CRC(b2baaaec) SHA1(bb0764d91dc1dcb143213faba204c2f2ff80aa33) ) // no custom label
ROM_END


ROM_START( bridgeca ) // model UBC, PCB label 510-4020-1C
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64108", 0x0000, 0x2000, CRC(08472223) SHA1(859865b13c908dbb474333263dc60f6a32461141) ) // NEC 2364C 210
	ROM_LOAD("101-64109", 0x2000, 0x2000, CRC(320afa0f) SHA1(90edfe0ac19b108d232cda376b03a3a24befad4c) ) // NEC 2364C 211
	ROM_LOAD("101-64110", 0x4000, 0x2000, CRC(3040d0bd) SHA1(caa55fc8d9196e408fb41e7171a68e5099519813) ) // NEC 2364C 212

	ROM_REGION( 0x0400, "mcu", 0 )
	ROM_LOAD("100-1009", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a) ) // NEC D8041C 563
ROM_END

ROM_START( bridgecv ) // model VBRC aka 7002/BV2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64108", 0x0000, 0x2000, CRC(08472223) SHA1(859865b13c908dbb474333263dc60f6a32461141) ) // NEC 2364C 210
	ROM_LOAD("101-64109", 0x2000, 0x2000, CRC(320afa0f) SHA1(90edfe0ac19b108d232cda376b03a3a24befad4c) ) // NEC 2364C 211
	ROM_LOAD("101-64110", 0x4000, 0x2000, CRC(3040d0bd) SHA1(caa55fc8d9196e408fb41e7171a68e5099519813) ) // NEC 2364C 212

	ROM_REGION( 0x0400, "mcu", 0 )
	ROM_LOAD("100-1009", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a) ) // NEC D8041C 563

	ROM_REGION( 0x1000, "speech", 0 )
	ROM_LOAD("101-32118", 0x0000, 0x1000, CRC(a0b8bb8f) SHA1(f56852108928d5c6caccfc8166fa347d6760a740) )
ROM_END


ROM_START( bridgec3 ) // model BV3 aka 7014, PCB label 510-1016 Rev.1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("7014_white", 0x0000, 0x2000, CRC(eb1620ef) SHA1(987a9abc8c685f1a68678ea4ee65ec4a99419179) ) // TMM2764AD-20, white sticker
	ROM_LOAD("7014_red",   0x2000, 0x2000, CRC(74af0019) SHA1(8dc05950c254ca050b95b93e5d0cf48f913a6d49) ) // TMM2764AD-20, red sticker
	ROM_LOAD("7014_blue",  0x4000, 0x2000, CRC(341d9ca6) SHA1(370876573bb9408e75f4fc797304b6c64af0590a) ) // TMM2764AD-20, blue sticker

	ROM_REGION( 0x0400, "mcu", 0 )
	ROM_LOAD("100-1009", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a) ) // NEC P07021-027 || D8041C 563 100-1009

	ROM_REGION( 0x1000, "speech", 0 )
	ROM_LOAD("101-32118", 0x0000, 0x1000, CRC(a0b8bb8f) SHA1(f56852108928d5c6caccfc8166fa347d6760a740) ) // ea 101-32118 || (C) 1980 || EA 8332A247-4 || 8034
ROM_END


ROM_START( gincribc ) // model GIN, PCB label 510-4020-1C
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("101-1036a01", 0x0000, 0x2000, CRC(30d8d900) SHA1(b31a4acc52143baad28a35ec515ab30d7b39683a) ) // MOSTEK MK36974N-5
	ROM_LOAD("101-1037a02", 0x2000, 0x2000, CRC(8802a71b) SHA1(416350acc1cbf38ff74194d49916b848bf6c2330) ) // MOSTEK MK36976N-5
	ROM_LOAD("bridge-3",    0x4000, 0x1000, CRC(d3cda2e3) SHA1(69b62fa22b388a922abad4e89c78bdb01a5fb322) ) // NEC 2332C 188

	ROM_REGION( 0x0400, "mcu", 0 )
	ROM_LOAD("100-1009", 0x0000, 0x0400, CRC(60eb343f) SHA1(8a63e95ebd62e123bdecc330c0484a47c354bd1a) ) // NEC D8041C 563
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, bridgec,  0,        0,      brc,     brc,   card_state, empty_init, "Fidelity Electronics", "Bridge Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )

SYST( 1979, bridgeca, 0,        0,      brc,     brc,   card_state, empty_init, "Fidelity Electronics", "Advanced Bridge Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )
SYST( 1979, bridgecv, bridgeca, 0,      vbrc,    brc,   card_state, empty_init, "Fidelity Electronics", "Voice Bridge Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )

SYST( 1982, bridgec3, 0,        0,      bv3,     bv3,   card_state, empty_init, "Fidelity Electronics", "Bridge Challenger III", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )

SYST( 1982, gincribc, 0,        0,      gin,     gin,   card_state, empty_init, "Fidelity Electronics", "Gin & Cribbage Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )
