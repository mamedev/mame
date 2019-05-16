// license:BSD-3-Clause
// copyright-holders:Kevin Horton, Jonathan Gevaryahu, Sandro Ronco, hap
// thanks-to:Berger, yoyo_chessboard
/******************************************************************************

* fidel_csc.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

Fidelity CSC(and derived) hardware
- Champion Sensory Chess Challenger
- Elite Champion Challenger
- Super 9 Sensory Chess Challenger
- Reversi Sensory Challenger

TODO:
- verify csce original roms (current set came from an overclock mod)
- hook up csce I/O properly, it doesn't have PIAs

*******************************************************************************

Champion Sensory Chess Challenger (CSC)
---------------------------------------
RE notes by Kevin Horton

Memory map:
-----------
0000-07FF: 2K of RAM
0800-0FFF: 1K of RAM (note: mirrored twice)
1000-17FF: PIA 1 (display, TSI speech chip)
1800-1FFF: PIA 0 (keypad, LEDs)
2000-3FFF: 101-64019 ROM*
4000-7FFF: mirror of 0000-3FFF
8000-9FFF: not used
A000-BFFF: 101-1025A03 ROM
C000-DFFF: 101-1025A02 ROM
E000-FDFF: 101-1025A01 ROM
FE00-FFFF: 512 byte 74S474 PROM

*: 101-64019 is also used on the VSC(fidel_vsc.cpp). It contains the opening book
and "64 greatest games", as well as some Z80 code. Obviously the latter is unused
on the CSC.

CPU is a 6502 running at 1.95MHz (3.9MHz resonator, divided by 2)

NMI is not used.
IRQ is connected to a 600Hz oscillator (38.4KHz divided by 64).
Reset is connected to a power-on reset circuit.

PIA 1:
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

PIA 0:
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

Selector: (attached to PIA 0, outputs 1 of 10 pins low. 7442)
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

these 6 buttons use row 9 (connects to PIA 1)

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

*******************************************************************************

Elite Champion Challenger
This is a limited-release chess computer based on the CSC. They removed the PIAs
and did the I/O with TTL instead (PIAs will still work from software point of view).
---------------------------------
R6502B? CPU @ 4MHz
32KB ROMs total size, 4KB RAM(8*HM6147P)

In the 90s, Wilfried Bucke provided an upgrade to make it more similar to the one
that won the 1981 Travemünde contest. CPU was changed to R65C02 or UM6502C at 5MHz.

*******************************************************************************

Super 9 Sensory Chess Challenger (SU9/DS9)
This is basically the Fidelity Elite A/S program on CSC hardware.
Model DS9(Deluxe) has a 5MHz XTAL, but is otherwise same.
Septennial(SCC) is the same as well, but clocked even higher.
---------------------------------
R6502AP CPU, 1.95MHz(3.9MHz resonator)
2 RAM chips, assume 4KB
2*8KB ROM + 1*2KB ROM
built-in CB9 module

See CSC description above for more information.

*******************************************************************************

Reversi Sensory Challenger (RSC)
The 1st version came out in 1980, a program revision was released in 1981.
Another distinction is the board color and layout, the 1981 version is green.
---------------------------------
8*(8+1) buttons, 8*8+1 LEDs
1KB RAM(2*2114), 4KB ROM
MOS MPS 6502B CPU, frequency unknown
MOS MPS 6520 PIA, I/O is nearly same as CSC's PIA 0
PCB label 510-1035A01

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_csc.lh" // clickable
#include "fidel_rsc_v2.lh" // clickable
#include "fidel_su9.lh" // clickable


namespace {

class csc_state : public fidelbase_state
{
public:
	csc_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_pia(*this, "pia%u", 0)
	{ }

	// machine drivers
	void csc(machine_config &config);
	void csce(machine_config &config);
	void su9(machine_config &config);
	void rsc(machine_config &config);

protected:
	// devices/pointers
	optional_device_array<pia6821_device, 2> m_pia;

	// address maps
	void csc_map(address_map &map);
	void su9_map(address_map &map);
	void rsc_map(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_READ8_MEMBER(speech_r);
	DECLARE_WRITE8_MEMBER(pia0_pa_w);
	DECLARE_WRITE8_MEMBER(pia0_pb_w);
	DECLARE_READ8_MEMBER(pia0_pa_r);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia0_cb2_w);
	DECLARE_READ_LINE_MEMBER(pia0_ca1_r);
	DECLARE_READ_LINE_MEMBER(pia0_cb1_r);
	DECLARE_WRITE8_MEMBER(pia1_pa_w);
	DECLARE_WRITE8_MEMBER(pia1_pb_w);
	DECLARE_READ8_MEMBER(pia1_pb_r);
	DECLARE_WRITE_LINE_MEMBER(pia1_ca2_w);
};

class su9_state : public csc_state
{
public:
	su9_state(const machine_config &mconfig, device_type type, const char *tag) :
		csc_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(su9_cpu_freq) { su9_set_cpu_freq(); }

protected:
	virtual void machine_reset() override;
	void su9_set_cpu_freq();
};

void su9_state::machine_reset()
{
	csc_state::machine_reset();
	su9_set_cpu_freq();
}

void su9_state::su9_set_cpu_freq()
{
	// SU9 CPU is clocked 1.95MHz, DS9 is 2.5MHz, SCC is 3MHz
	u8 inp = ioport("FAKE")->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? (3_MHz_XTAL) : ((inp & 1) ? (5_MHz_XTAL/2) : (3.9_MHz_XTAL/2)));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// misc handlers

void csc_state::prepare_display()
{
	// 7442 0-8: led select, input mux
	m_inp_mux = 1 << m_led_select & 0x3ff;

	// 7442 9: speaker out
	m_dac->write(BIT(m_inp_mux, 9));

	// 7seg leds+H (not on all models), 8*8(+1) chessboard leds
	set_display_segmask(0xf, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | m_7seg_data, m_inp_mux);
}

READ8_MEMBER(csc_state::speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}


// 6821 PIA 0

READ8_MEMBER(csc_state::pia0_pa_r)
{
	// d0-d5: button row 0-5 (active low)
	return (read_inputs(9) & 0x3f) ^ 0xff;
}

WRITE8_MEMBER(csc_state::pia0_pa_w)
{
	// d6,d7: 7442 A0,A1
	m_led_select = (m_led_select & ~3) | (data >> 6 & 3);
	prepare_display();
}

WRITE8_MEMBER(csc_state::pia0_pb_w)
{
	// d0-d7: led row data
	m_led_data = data;
	prepare_display();
}

READ_LINE_MEMBER(csc_state::pia0_ca1_r)
{
	// button row 6 (active low)
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(csc_state::pia0_cb1_r)
{
	// button row 7 (active low)
	return ~read_inputs(9) >> 7 & 1;
}

WRITE_LINE_MEMBER(csc_state::pia0_cb2_w)
{
	// 7442 A2
	m_led_select = (m_led_select & ~4) | (state ? 4 : 0);
	prepare_display();
}

WRITE_LINE_MEMBER(csc_state::pia0_ca2_w)
{
	// 7442 A3
	m_led_select = (m_led_select & ~8) | (state ? 8 : 0);
	prepare_display();
}


// 6821 PIA 1

WRITE8_MEMBER(csc_state::pia1_pa_w)
{
	// d0-d5: TSI C0-C5
	m_speech->data_w(space, 0, data & 0x3f);

	// d0-d7: data for the 4 7seg leds, bits are ABFGHCDE (H is extra led)
	m_7seg_data = bitswap<8>(data,0,1,5,6,7,2,3,4);
	prepare_display();
}

WRITE8_MEMBER(csc_state::pia1_pb_w)
{
	// d0: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data & 1;

	// d1: TSI START line
	m_speech->start_w(data >> 1 & 1);

	// d4: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x10) ? 0.25 : 1.0);
}

READ8_MEMBER(csc_state::pia1_pb_r)
{
	// d2: printer?
	u8 data = 0x04;

	// d3: TSI BUSY line
	if (m_speech->busy_r())
		data |= 0x08;

	// d5: button row 8 (active low)
	// d6,d7: language switches(hardwired with 2 resistors/jumpers)
	data |= (~read_inputs(9) >> 3 & 0x20) | (*m_language << 6 & 0xc0);

	return data;
}

WRITE_LINE_MEMBER(csc_state::pia1_ca2_w)
{
	// printer?
}



/******************************************************************************
    Address Maps
******************************************************************************/

void csc_state::csc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x4000).ram();
	map(0x0800, 0x0bff).mirror(0x4400).ram();
	map(0x1000, 0x1003).mirror(0x47fc).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1800, 0x1803).mirror(0x47fc).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x3fff).mirror(0x4000).rom();
	map(0xa000, 0xffff).rom();
}

void csc_state::su9_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1003).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1800, 0x1803).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x3fff).rom();
	map(0xa000, 0xffff).rom();
}

void csc_state::rsc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x2000, 0x2003).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( csc )
	PORT_INCLUDE( generic_cb_buttons )

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

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( su9 )
	PORT_INCLUDE( csc )

	PORT_MODIFY("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")

	PORT_START("FAKE")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, su9_state, su9_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "1.95MHz (original)" )
	PORT_CONFSETTING(    0x01, "2.5MHz (Deluxe)" )
	PORT_CONFSETTING(    0x02, "3MHz (Septennial)" )
INPUT_PORTS_END

static INPUT_PORTS_START( rsc )
	PORT_INCLUDE( generic_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("ST")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("RV")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("DM")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("CL")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("LV")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PV")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void csc_state::csc(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 3.9_MHz_XTAL/2); // from 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &csc_state::csc_map);

	const attotime irq_period = attotime::from_hz(38.4_kHz_XTAL/64); // through 4060 IC, 600Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(csc_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_hz(38.4_kHz_XTAL*2)); // edge!
	TIMER(config, "irq_off").configure_periodic(FUNC(csc_state::irq_off<M6502_IRQ_LINE>), irq_period);

	PIA6821(config, m_pia[0], 0);
	m_pia[0]->readpa_handler().set(FUNC(csc_state::pia0_pa_r));
	m_pia[0]->readca1_handler().set(FUNC(csc_state::pia0_ca1_r));
	m_pia[0]->readcb1_handler().set(FUNC(csc_state::pia0_cb1_r));
	m_pia[0]->writepa_handler().set(FUNC(csc_state::pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(csc_state::pia0_pb_w));
	m_pia[0]->ca2_handler().set(FUNC(csc_state::pia0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(csc_state::pia0_cb2_w));

	PIA6821(config, m_pia[1], 0);
	m_pia[1]->readpb_handler().set(FUNC(csc_state::pia1_pb_r));
	m_pia[1]->writepa_handler().set(FUNC(csc_state::pia1_pa_w));
	m_pia[1]->writepb_handler().set(FUNC(csc_state::pia1_pb_w));
	m_pia[1]->ca2_handler().set(FUNC(csc_state::pia1_ca2_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(csc_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_csc);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(csc_state::speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void csc_state::csce(machine_config &config)
{
	csc(config);

	/* basic machine hardware */
	m_maincpu->set_clock(4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &csc_state::su9_map);
}

void csc_state::su9(machine_config &config)
{
	csc(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &csc_state::su9_map);
	config.set_default_layout(layout_fidel_su9);
}

void csc_state::rsc(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1800000); // measured approx 1.81MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &csc_state::rsc_map);

	const attotime irq_period = attotime::from_hz(546); // from 555 timer, measured
	TIMER(config, m_irq_on).configure_periodic(FUNC(csc_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(38)); // active for 38us
	TIMER(config, "irq_off").configure_periodic(FUNC(csc_state::irq_off<M6502_IRQ_LINE>), irq_period);

	PIA6821(config, m_pia[0], 0); // MOS 6520
	m_pia[0]->readpa_handler().set(FUNC(csc_state::pia0_pa_r));
	m_pia[0]->readca1_handler().set(FUNC(csc_state::pia0_ca1_r));
	m_pia[0]->readcb1_handler().set(FUNC(csc_state::pia0_cb1_r));
	m_pia[0]->writepa_handler().set(FUNC(csc_state::pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(csc_state::pia0_pb_w));
	m_pia[0]->ca2_handler().set(FUNC(csc_state::pia0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(csc_state::pia0_cb2_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(csc_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_rsc_v2);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( csc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( csce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("03", 0x2000, 0x2000, CRC(22e43531) SHA1(696dc019bea3812ae6cf9c2b2c4d3a7b9017807d) )
	ROM_LOAD("02", 0xa000, 0x2000, CRC(e593f114) SHA1(4dc5a2456a87c128235958f046cee9502cb3ac65) )
	ROM_LOAD("06", 0xc000, 0x0800, CRC(5d41b1e5) SHA1(fe95d8811d8894688336b798212c397bdb216956) )
	ROM_LOAD("07", 0xc800, 0x0800, CRC(9078d40a) SHA1(4ffd36a4fcde1988e42543652e29463bc6ad5a8f) )
	ROM_LOAD("08", 0xd000, 0x0800, CRC(c9472cc1) SHA1(ef4b1ae99e81689efeae323fe6ed58cf2c773fd6) )
	ROM_LOAD("09", 0xd800, 0x0800, CRC(255c94a0) SHA1(d8e79213b69710e9d94c698492ec7b7420c9c7d8) )
	ROM_LOAD("04", 0xe000, 0x1000, CRC(098873bd) SHA1(86001129a57db390e565f59a5677ec0b34b41d99) )
	ROM_LOAD("05", 0xf000, 0x1000, CRC(1a516bfe) SHA1(2a2b252ca5d425fdf162cbc53077aee448b94437) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( super9cc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("101-1050a01", 0x2000, 0x2000, CRC(421147e8) SHA1(ccf62f6f218e8992baf30973fe41b35e14a1cc1a) )
	ROM_LOAD("101-1024b03", 0xa000, 0x0800, CRC(e8c97455) SHA1(ed2958fc5474253ee8c2eaf27fc64226e12f80ea) )
	ROM_LOAD("101-1024b02", 0xc000, 0x2000, CRC(95004699) SHA1(ea79f43da73267344545df8ad61730f613876c2e) )
	ROM_LOAD("101-1024c01", 0xe000, 0x2000, CRC(03904e86) SHA1(bfa0dd9d8541e3ec359a247a3eba543501f727bc) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( reversic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1000a01", 0xf000, 0x1000, CRC(ca7723a7) SHA1(bd92330f2d9494fa408f5a2ca300d7a755bdf489) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, csc,      0,      0, csc,      csc,  csc_state, empty_init, "Fidelity Electronics", "Champion Sensory Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1981, csce,     0,      0, csce,     csc,  csc_state, empty_init, "Fidelity Electronics", "Elite Champion Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1983, super9cc, 0,      0, su9,      su9,  su9_state, empty_init, "Fidelity Electronics", "Super 9 Sensory Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1981, reversic, 0,      0, rsc,      rsc,  csc_state, empty_init, "Fidelity Electronics", "Reversi Sensory Challenger (green version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
