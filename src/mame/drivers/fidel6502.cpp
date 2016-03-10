// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco,hap
/******************************************************************************

    Fidelity Electronics 6502 based board driver

******************************************************************************

Champion Sensory Chess Challenger (CSC)
---------------------------------------

Memory map:
-----------
0000-07FF: 2K of RAM
0800-0FFF: 1K of RAM (note: mirrored twice)
1000-17FF: PIA 0 (display, TSI speech chip)
1800-1FFF: PIA 1 (keypad, LEDs)
2000-3FFF: 101-64019 ROM (also used on the regular sensory chess challenger)
4000-7FFF: mirror of 0000-3FFF
8000-9FFF: not used
A000-BFFF: 101-1025A03 ROM
C000-DFFF: 101-1025A02 ROM
E000-FDFF: 101-1025A01 ROM
FE00-FFFF: 512 byte 74S474 PROM

CPU is a 6502 running at 1.95MHz (3.9MHz resonator, divided by 2)

NMI is not used.
IRQ is connected to a 600Hz oscillator (38.4KHz divided by 64).
Reset is connected to a power-on reset circuit.

PIA 0:
------
PA0 - 7seg segments E, TSI A0
PA1 - 7seg segments D, TSI A1
PA2 - 7seg segments C, TSI A2
PA3 - 7seg segments H, TSI A3
PA4 - 7seg segments G, TSI A4
PA5 - 7seg segments F, TSI A5
PA6 - 7seg segments B
PA7 - 7seg segments A

PB0 - A12 on speech ROM (if used... not used on this model, ROM is 4K)
PB1 - START line on TSI
PB2 - white wire
PB3 - BUSY line from TSI
PB4 - hi/lo TSI speaker volume
PB5 - button row 9
PB6 - selection jumper (resistor to 5V)
PB7 - selection jumper (resistor to ground)

CA1 - NC
CA2 - violet wire

CB1 - NC
CB2 - NC (connects to pin 14 of soldered connector)

PIA 1:
------
PA0 - button row 1
PA1 - button row 2
PA2 - button row 3
PA3 - button row 4
PA4 - button row 5
PA5 - button row 6
PA6 - 7442 selector bit 0
PA7 - 7442 selector bit 1

PB0 - LED row 1
PB1 - LED row 2
PB2 - LED row 3
PB3 - LED row 4
PB4 - LED row 5
PB5 - LED row 6
PB6 - LED row 7
PB7 - LED row 8

CA1 - button row 7
CA2 - selector bit 3

CB1 - button row 8
CB2 - selector bit 2

Selector: (attached to PIA 1, outputs 1 of 10 pins low.  7442)
---------
output # (selected turns this column on, and all others off)
0 - LED column A, button column A, 7seg digit 1
1 - LED column B, button column B, 7seg digit 2
2 - LED column C, button column C, 7seg digit 3
3 - LED column D, button column D, 7seg digit 4
4 - LED column E, button column E
5 - LED column F, button column F
6 - LED column G, button column G
7 - LED column H, button column H
8 - button column I
9 - Tone line (toggle to make a tone in the buzzer)

The rows/columns are indicated on the game board:

 ABCDEFGH   I
--------------
|            | 8
|            | 7
|            | 6
|            | 5
|            | 4
|            | 3
|            | 2
|            | 1
--------------

The "lone LED" is above the control column.
column I is the "control column" on the right for starting a new game, etc.

The upper 6 buttons are connected as such:

column A - speak
column B - RV
column C - TM
column D - LV
column E - DM
column F - ST

these 6 buttons use row 9 (connects to PIA 0)

LED display:
------------
43 21 (digit number)
-----
88:88

The LED display is four 7 segment digits.  normal ABCDEFG lettering is used for segments.

The upper dot is connected to digit 3 common
The lower dot is connected to digit 4 common
The lone LED is connected to digit 1 common

All three of the above are called "segment H".


******************************************************************************

Sensory Chess Challenger (SC12-B, 6086)
4 versions are known to exist: A,B,C, and X, with increasing CPU speed.
---------------------------------
RE information from netlist by Berger

8*(8+1) buttons, 8+8+2 red LEDs
DIN 41524C printer port
36-pin edge connector
CPU is a R65C02P4, running at 4MHz

NE556 dual-timer IC:
- timer#1, one-shot at power-on, to CPU _RESET
- timer#2: R1=82K, R2=1K, C=22nf, to CPU _IRQ: ~780Hz, active low=15.25us

Memory map:
-----------
6000-0FFF: 4K RAM (2016 * 2)
2000-5FFF: cartridge
6000-7FFF: control(W)
8000-9FFF: 8K ROM SSS SCM23C65E4
A000-BFFF: keypad(R)
C000-DFFF: 4K ROM TI TMS2732AJL-45
E000-FFFF: 8K ROM Toshiba TMM2764D-2

control: (74LS377)
--------
Q0-Q3: 7442 A0-A3
Q4: enable printer port pin 1 input
Q5: printer port pin 5 output
Q6,Q7: LEDs common anode

7442 0-8: input mux and LEDs cathode
7442 9: buzzer

The keypad is read through a 74HC251, where S0,1,2 is from CPU A0,1,2, Y is connected to CPU D7.
If control Q4 is set, printer data can be read from I0.


******************************************************************************

Voice Excellence (model 6092)
----------------
PCB 1: 510.1117A02, appears to be identical to other "Excellence" boards
CPU: GTE G65SC102P-3, 32 KB PRG ROM: AMI 101-1080A01(IC5), 8192x8 SRAM SRM2264C10(IC6)
2 rows of LEDs on the side: 1*8 green, 1*8 red

PCB 2: 510.1117A01
Speech: TSI S14001A, 32 KB ROM: AMI 101-1081A01(IC2)
Dip Switches set ROM A13 and ROM A14, on the side of the board

ROM A12 is tied to S14001A's A11 (yuck)
ROM A11 is however tied to the CPU's XYZ

0000_07FF - Spanish 1/4
0800_0FFF - Spanish 3/4
1000_17FF - Spanish 2/4
1800_1FFF - Spanish 4/4

2000_27FF - French 1/4
2800_2FFF - French 3/4
3000_3FFF - French 2/4
3800_3FFF - French 4/4

4000_47FF - German 1/4
4800_4FFF - German 3/4
5000_57FF - German 2/4
5800_5FFF - German 4/4

6000_67FF - English 1/2
6800_6FFF - Bridge Challenger 1/2
7000_77FF - English 2/2
7800_7FFF - Bridge Challenger 2/2

------------------
RE info by hap, based on PCB photos

Memory map:
-----------
0000-3FFF: 8K RAM (SRM2264)
4000-7FFF: control (R/W)
8000-FFFF: 32K ROM (M27256 compatible)

control (W):
------------
Z80 A0-A2 to 3*74259, Z80 Dx to D (_C unused)

Z80 D0:
- Q4,Q5: led commons
- Q6,Q7,Q2,Q1: 7seg panel digit select
- Q0-Q3: 7442 A0-A3
  + 0-7: led data
  + 0-8: keypad mux
  + 9: buzzer out

Z80 D1: (model 6093)
- Q0-Q7: 7seg data

Z80 D2: (model 6092)
- Q0-Q5: TSI C0-C5
- Q6: TSI START pin
- Q7: TSI ROM A11

A11 from TSI is tied to TSI ROM A12(!)
TSI ROM A13,A14 are hardwired to the 2 language switches.
Sound comes from the Audio out pin, digital out pins are N/C.

control (R):
------------
Z80 A0-A2 to 2*74251, Z80 Dx to output

Z80 D7 to Y:
- D0-D7: keypad row data

Z80 D6 to W: (model 6092, tied to VCC otherwise)
- D0,D1: language switches
- D2-D6: VCC
- D7: TSI BUSY

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/6821pia.h"

#include "includes/fidelz80.h"

// internal artwork
#include "fidel_csc.lh" // clickable
#include "fidel_fev.lh" // clickable
#include "fidel_sc12.lh" // clickable


class fidel6502_state : public fidelz80base_state
{
public:
	fidel6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelz80base_state(mconfig, type, tag),
		m_6821pia(*this, "6821pia"),
		m_cart(*this, "cartslot")
	{ }

	// devices/pointers
	optional_device<pia6821_device> m_6821pia;
	optional_device<generic_slot_device> m_cart;

	TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE); }

	// CSC
	void csc_prepare_display();
	DECLARE_READ8_MEMBER(csc_speech_r);
	DECLARE_WRITE8_MEMBER(csc_pia0_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia0_pb_w);
	DECLARE_READ8_MEMBER(csc_pia0_pb_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia0_ca2_w);
	DECLARE_WRITE8_MEMBER(csc_pia1_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia1_pb_w);
	DECLARE_READ8_MEMBER(csc_pia1_pa_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia1_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(csc_pia1_cb2_w);
	DECLARE_READ_LINE_MEMBER(csc_pia1_ca1_r);
	DECLARE_READ_LINE_MEMBER(csc_pia1_cb1_r);

	// SC12/6086
	DECLARE_WRITE8_MEMBER(sc12_control_w);
	DECLARE_READ8_MEMBER(sc12_input_r);
	DECLARE_READ8_MEMBER(sc12_cart_r);

	// 6080/6092/6093 (Excellence)
	DECLARE_INPUT_CHANGED_MEMBER(fexcelv_bankswitch);
	DECLARE_READ8_MEMBER(fexcelv_speech_r);
	DECLARE_WRITE8_MEMBER(fexcel_ttl_w);
	DECLARE_READ8_MEMBER(fexcel_ttl_r);
};



// Devices, I/O

/******************************************************************************
    CSC
******************************************************************************/

// misc handlers

void fidel6502_state::csc_prepare_display()
{
	// 7442 0-8: led select, input mux
	m_inp_mux = 1 << m_led_select & 0x3ff;

	// 7442 9: speaker out
	m_speaker->level_w(m_inp_mux >> 9 & 1);

	// 4 7seg leds + H
	for (int i = 0; i < 4; i++)
		m_display_state[i] = (m_inp_mux >> i & 1) ? m_7seg_data : 0;

	// 8*8 chessboard leds
	for (int i = 0; i < 8; i++)
		m_display_state[i+4] = (m_inp_mux >> i & 1) ? m_led_data : 0;

	set_display_size(8, 4+8);
	set_display_segmask(0xf, 0x7f);
	display_update();
}

READ8_MEMBER(fidel6502_state::csc_speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}


// 6821 PIA 0

WRITE8_MEMBER(fidel6502_state::csc_pia0_pa_w)
{
	// d0-d5: TSI C0-C5
	m_speech->data_w(space, 0, data & 0x3f);

	// d0-d7: data for the 4 7seg leds, bits are ABFGHCDE (H is extra led)
	m_7seg_data = BITSWAP8(data,0,1,5,6,7,2,3,4);
	csc_prepare_display();
}

WRITE8_MEMBER(fidel6502_state::csc_pia0_pb_w)
{
	// d0: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data & 1;

	// d1: TSI START line
	m_speech->start_w(data >> 1 & 1);

	// d4: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x10) ? 0.5 : 1.0);
}

READ8_MEMBER(fidel6502_state::csc_pia0_pb_r)
{
	// d2: printer?
	UINT8 data = 0x04;

	// d3: TSI BUSY line
	if (m_speech->busy_r())
		data |= 0x08;

	// d5: button row 8 (active low)
	// d6,d7: language switches
	data |= (~read_inputs(9) >> 3 & 0x20) | (~m_inp_matrix[9]->read() << 6 & 0xc0);

	return data;
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia0_ca2_w)
{
	// printer?
}


// 6821 PIA 1

READ8_MEMBER(fidel6502_state::csc_pia1_pa_r)
{
	// d0-d5: button row 0-5 (active low)
	return (read_inputs(9) & 0x3f) ^ 0xff;
}

WRITE8_MEMBER(fidel6502_state::csc_pia1_pa_w)
{
	// d6,d7: 7442 A0,A1
	m_led_select = (m_led_select & ~3) | (data >> 6 & 3);
	csc_prepare_display();
}

WRITE8_MEMBER(fidel6502_state::csc_pia1_pb_w)
{
	// d0-d7: led row data
	m_led_data = data;
	csc_prepare_display();
}

READ_LINE_MEMBER(fidel6502_state::csc_pia1_ca1_r)
{
	// button row 6 (active low)
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(fidel6502_state::csc_pia1_cb1_r)
{
	// button row 7 (active low)
	return ~read_inputs(9) >> 7 & 1;
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia1_cb2_w)
{
	// 7442 A2
	m_led_select = (m_led_select & ~4) | (state ? 4 : 0);
	csc_prepare_display();
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia1_ca2_w)
{
	// 7442 A3
	m_led_select = (m_led_select & ~8) | (state ? 8 : 0);
	csc_prepare_display();
}



/******************************************************************************
    SC12/6086
******************************************************************************/



// TTL/generic

WRITE8_MEMBER(fidel6502_state::sc12_control_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	UINT16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;

	// 7442 9: speaker out
	m_speaker->level_w(sel >> 9 & 1);

	// d6,d7: led select (active low)
	display_matrix(9, 2, sel & 0x1ff, ~data >> 6 & 3);

	// d4,d5: printer
	//..
}

READ8_MEMBER(fidel6502_state::sc12_input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}

READ8_MEMBER(fidel6502_state::sc12_cart_r)
{
	if (m_cart->exists())
		return m_cart->read_rom(space, offset);
	else
		return 0;
}



/******************************************************************************
    6080/6092/6093 (Excellence)
******************************************************************************/

// misc handlers

INPUT_CHANGED_MEMBER(fidel6502_state::fexcelv_bankswitch)
{
	// tied to speech ROM highest bits
	m_speech->force_update();
	m_speech_bank = (m_speech_bank & 1) | newval << 1;
}

READ8_MEMBER(fidel6502_state::fexcelv_speech_r)
{
	// TSI A11 is A12, program controls A11, user controls A13,A14(language switches)
	offset = (offset & 0x7ff) | (offset << 1 & 0x1000);
	return m_speech_rom[offset | (m_speech_bank << 11 & 0x800) | (~m_speech_bank << 12 & 0x6000)];
}


// TTL

WRITE8_MEMBER(fidel6502_state::fexcel_ttl_w)
{
	// a0-a2,d0: 74259(1)
	UINT8 mask = 1 << offset;
	m_led_select = (m_led_select & ~mask) | ((data & 1) ? mask : 0);

	// 74259 Q0-Q3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	UINT16 sel = 1 << (m_led_select & 0xf) & 0x3ff;
	UINT8 led_data = sel & 0xff;
	m_inp_mux = sel & 0x1ff;

	// 7442 9: speaker out (optional?)
	m_speaker->level_w(sel >> 9 & 1);

	// 74259 Q4-Q7,Q2,Q1: digit/led select (active low)
	UINT8 led_sel = ~BITSWAP8(m_led_select,0,3,1,2,7,6,5,4) & 0x3f;

	// a0-a2,d1: digit segment data (model 6093)
	m_7seg_data = (m_7seg_data & ~mask) | ((data & 2) ? mask : 0);
	UINT8 seg_data = BITSWAP8(m_7seg_data,0,1,3,2,7,5,6,4);

	// update display: 4 7seg leds, 2*8 chessboard leds
	for (int i = 0; i < 6; i++)
		m_display_state[i] = (led_sel >> i & 1) ? ((i < 2) ? led_data : seg_data) : 0;

	set_display_size(8, 2+4);
	set_display_segmask(0x3c, 0x7f);
	display_update();

	// speech (model 6092)
	if (m_speech != nullptr)
	{
		// a0-a2,d2: 74259(2) to speech board
		m_speech_data = (m_speech_data & ~mask) | ((data & 4) ? mask : 0);

		// 74259 Q6: TSI ROM A11
		m_speech->force_update(); // update stream to now
		m_speech_bank = (m_speech_bank & ~1) | (m_speech_data >> 6 & 1);

		// Q0-Q5: TSI C0-C5
		// Q7: TSI START line
		m_speech->data_w(space, 0, m_speech_data & 0x3f);
		m_speech->start_w(m_speech_data >> 7 & 1);
	}
}

READ8_MEMBER(fidel6502_state::fexcel_ttl_r)
{
	// a0-a2,d6: from speech board: language switches and TSI BUSY line, otherwise tied to VCC
	UINT8 d6 = (read_safe(m_inp_matrix[9], 0xff) >> offset & 1) ? 0x40 : 0;

	// a0-a2,d7: multiplexed inputs (active low)
	return d6 | ((read_inputs(9) >> offset & 1) ? 0 : 0x80);
}



/******************************************************************************
    Address Maps
******************************************************************************/

// CSC

static ADDRESS_MAP_START( csc_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x4000) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x4400) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x47fc) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x47fc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


// SC12/6086

static ADDRESS_MAP_START( sc12_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x2000, 0x5fff) AM_READ(sc12_cart_r)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_WRITE(sc12_control_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa007) AM_MIRROR(0x1ff8) AM_READ(sc12_input_r)
	AM_RANGE(0xc000, 0xcfff) AM_MIRROR(0x1000) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


// 6080/6092/6093 (Excellence)

static ADDRESS_MAP_START( fexcel_map, AS_PROGRAM, 8, fidel6502_state )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x2000) AM_RAM
	AM_RANGE(0x4000, 0x4007) AM_MIRROR(0x3ff8) AM_READWRITE(fexcel_ttl_r, fexcel_ttl_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sc12 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square a8")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square b8")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square c8")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square d8")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square e8")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square f8")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square g8")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Square h8")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( fexcel )
	PORT_INCLUDE( sc12 )

	PORT_MODIFY("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Options / Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Verify / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END

static INPUT_PORTS_START( fexcelv )
	PORT_INCLUDE( fexcel )

	PORT_START("IN.9")
	PORT_CONFNAME( 0x03, 0x00, "Language" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fidel6502_state, fexcelv_bankswitch, 0)
	PORT_CONFSETTING(    0x00, "English" )
	PORT_CONFSETTING(    0x01, "German" )
	PORT_CONFSETTING(    0x02, "French" )
	PORT_CONFSETTING(    0x03, "Spanish" )
	PORT_BIT(0x7c, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_READ_LINE_DEVICE_MEMBER("speech", s14001a_device, busy_r)
INPUT_PORTS_END


static INPUT_PORTS_START( csc )
	PORT_INCLUDE( sc12 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("TM")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LV")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("ST")

	PORT_MODIFY("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")

	PORT_START("IN.9") // hardwired
	PORT_CONFNAME( 0x01, 0x00, "Language" )
	PORT_CONFSETTING(    0x00, "English" )
	PORT_CONFSETTING(    0x01, "Other" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cscg )
	PORT_INCLUDE( csc )

	PORT_MODIFY("IN.9")
	PORT_CONFNAME( 0x01, 0x01, "Language" )
	PORT_CONFSETTING(    0x00, "English" )
	PORT_CONFSETTING(    0x01, "Other" )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( csc, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3900000/2) // from 3.9MHz resonator
	MCFG_CPU_PROGRAM_MAP(csc_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(fidelz80base_state, irq0_line_hold, 600) // 38400kHz/64

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(fidel6502_state, csc_pia0_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(fidel6502_state, csc_pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(fidel6502_state, csc_pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(fidel6502_state, csc_pia0_ca2_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(fidel6502_state, csc_pia1_pa_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(fidel6502_state, csc_pia1_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(fidel6502_state, csc_pia1_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(fidel6502_state, csc_pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(fidel6502_state, csc_pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(fidel6502_state, csc_pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(fidel6502_state, csc_pia1_cb2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_csc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, 25000) // R/C circuit, around 25khz
	MCFG_S14001A_EXT_READ_HANDLER(READ8(fidel6502_state, csc_speech_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sc12, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R65C02, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(sc12_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_on", fidel6502_state, irq_on, attotime::from_hz(780)) // from 556 timer
	MCFG_TIMER_START_DELAY(attotime::from_hz(780) - attotime::from_nsec(15250)) // active for 15.25us
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_off", fidel6502_state, irq_off, attotime::from_hz(780))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_sc12)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "fidel_scc")
	MCFG_GENERIC_EXTENSIONS("bin,dat")
	MCFG_GENERIC_LOAD(fidelz80base_state, scc_cartridge)
	MCFG_SOFTWARE_LIST_ADD("cart_list", "fidel_scc")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fexcel, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, XTAL_12MHz/4) // G65SC102P-3, 12.0M ceramic resonator
	MCFG_CPU_PROGRAM_MAP(fexcel_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_on", fidel6502_state, irq_on, attotime::from_hz(780)) // from 556 timer, PCB photo suggests it's same as sc12
	MCFG_TIMER_START_DELAY(attotime::from_hz(780) - attotime::from_nsec(15250)) // active for 15.25us
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_off", fidel6502_state, irq_off, attotime::from_hz(780))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_fev)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fexcelv, fexcel )

	/* sound hardware */
	MCFG_SOUND_ADD("speech", S14001A, 25000) // R/C circuit, around 25khz
	MCFG_S14001A_EXT_READ_HANDLER(READ8(fidel6502_state, fexcelv_speech_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( csc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03.bin",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02.bin",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01.bin",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474.bin",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107.bin", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) )
	ROM_RELOAD(               0x1000, 0x1000)
ROM_END

ROM_START( cscsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03.bin",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02.bin",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01.bin",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474.bin",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("vcc-spanish.bin", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( cscg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03.bin",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02.bin",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01.bin",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474.bin",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("vcc-german.bin", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( cscfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03.bin",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02.bin",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01.bin",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474.bin",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("vcc-french.bin", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( fscc12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1068a01",   0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("tms2732ajl-45", 0xc000, 0x1000, CRC(45070a71) SHA1(8aeecff828f26fb7081902c757559903be272649) ) // TI TMS2732AJL-45
	ROM_LOAD("tmm2764d-2",    0xe000, 0x2000, CRC(183d3edc) SHA1(3296a4c3bce5209587d4a1694fce153558544e63) ) // Toshiba TMM2764D-2
ROM_END


ROM_START( fexcel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) ) // same as fexcelv
ROM_END

ROM_START( fexcelv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) ) // PCB1, M27256

	ROM_REGION( 0x8000, "speech", 0 )
	ROM_LOAD("101-1081a01.ic2", 0x0000, 0x8000, CRC(c8ae1607) SHA1(6491ce6be60ed77f3dd931c0ca17616f13af943e) ) // PCB2, M27256
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1981, csc,     0,      0,      csc,     csc,     driver_device, 0, "Fidelity Electronics", "Champion Sensory Chess Challenger (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, cscsp,   csc,    0,      csc,     cscg,    driver_device, 0, "Fidelity Electronics", "Champion Sensory Chess Challenger (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, cscg,    csc,    0,      csc,     cscg,    driver_device, 0, "Fidelity Electronics", "Champion Sensory Chess Challenger (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, cscfr,   csc,    0,      csc,     cscg,    driver_device, 0, "Fidelity Electronics", "Champion Sensory Chess Challenger (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1984, fscc12,  0,      0,      sc12,    sc12,    driver_device, 0, "Fidelity Electronics", "Sensory Chess Challenger 12-B", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1987, fexcel,  0,      0,      fexcel,  fexcel,  driver_device, 0, "Fidelity Electronics", "Excellence (model 6080/6093)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1987, fexcelv, 0,      0,      fexcelv, fexcelv, driver_device, 0, "Fidelity Electronics", "Voice Excellence", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
