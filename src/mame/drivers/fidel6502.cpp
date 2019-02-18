// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco,hap
// thanks-to:Berger,yoyo_chessboard
/******************************************************************************

    Fidelity Electronics 6502 based board driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - Source organization is a big mess. Each machine family could be in its own
      sub driverclass, and separate files.
    - verify cpu speed and rom labels where unknown
    - improve EAS/SC12/etc CPU divider? it seems a little bit slower than the real machine.
      Currently, a dummy timer workaround is needed, or it's much worse.
      Is the problem here is due to timing of CPU addressbus changes? We can only 'sense'
      the addressbus at read or write accesses.
    - finish fphantom emulation

******************************************************************************

Champion Sensory Chess Challenger (CSC)
---------------------------------------

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

*: 101-64019 is also used on the VSC(fidelz80.cpp). It contains the opening book
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


******************************************************************************

Super 9 Sensory Chess Challenger (SU9/DS9)
This is basically the Fidelity Elite A/S program on CSC hardware.
Model DS9(Deluxe) has a 5MHz XTAL, but is otherwise same.
---------------------------------

R6502AP CPU, 1.95MHz(3.9MHz resonator)
2 RAM chips, assume 4KB
2*8KB ROM + 1*2KB ROM
built-in CB9 module

See CSC description above for more information.


******************************************************************************

Reversi Sensory Challenger (RSC)
The 1st version came out in 1980, a program revision was released in 1981.
Another distinction is the board color and layout, the 1981 version is green.
---------------------------------

8*(8+1) buttons, 8*8+1 LEDs
1KB RAM(2*2114), 4KB ROM
MOS MPS 6502B CPU, frequency unknown
MOS MPS 6520 PIA, I/O is nearly same as CSC's PIA 0
PCB label 510-1035A01


******************************************************************************

Elite A/S Challenger (EAS)
This came out in 1982. 2 program updates were released in 1983 and 1984,
named Budapest and Glasgow, places where Fidelity won chess computer matches.
A/S stands for auto sensory, it's the 1st Fidelity board with magnet sensors.
---------------------------------

8*8 magnet sensors, 11 buttons, 8*(8+1) LEDs + 4*7seg LEDs
R65C02P4 or R6502BP CPU, default frequency 3MHz*
4KB RAM (2*HM6116), 24KB ROM
TSI S14001A + speech ROM
I/O with 8255 PPI and bunch of TTL
module slot and printer port
PCB label 510-1071A01

*It was advertised as 3.2, 3.6, or 4MHz, with unofficial modifications up to 8MHz.
PCB photos show only a 3MHz XTAL.

A condensator keeps RAM contents alive for a few hours when powered off.

Elite Avant Garde (models 6081,6088,6089) is on the same hardware.

Prestige Challenger (PC) hardware is very similar. They stripped the 8255 PPI,
and added more RAM(7*TMM2016P). Some were released at 3.6MHz instead of 4MHz,
perhaps due to hardware instability?


******************************************************************************

Sensory Chess Challenger "9" (SC9)
3 versions were available, the newest "B" version was 2MHz and included the Budapest program.
The Playmatic S was only released in Germany, it's basically a 'deluxe' version of SC9
with magnet sensors and came with CB9 and CB16.
---------------------------------

8*(8+1) buttons, 8*8+1 LEDs
36-pin edge connector, assume same as SC12
2KB RAM(TMM2016P), 2*8KB ROM(HN48364P)
R6502-13, 1.4MHz from resonator, another pcb with the same resonator was measured 1.49MHz*
PCB label 510-1046C01 2-1-82

*: 2 other boards were measured 1.60MHz and 1.88MHz(newest serial). Online references
suggest 3 versions of SC9(C01) total: 1.5MHz, 1.6MHz, and 1.9MHz.

I/O is via TTL, not further documented here


******************************************************************************

Sensory 12 Chess Challenger (SC12-B, 6086)
4 versions are known to exist: A,B,C, and X, with increasing CPU speed.
---------------------------------
RE information from netlist by Berger

8*(8+1) buttons, 8+8+2 red LEDs
DIN 41524C printer port
36-pin edge connector
CPU is a R65C02P4, running at 4MHz*

*By default, the CPU frequency is lowered on A13/A14 access, with a factory-set jumper:
/2 on model SC12(1.5MHz), /4 on model 6086(1MHz)

NE556 dual-timer IC:
- timer#1, one-shot at power-on, to CPU _RESET
- timer#2: R1=82K+50K pot at 26K, R2=1K, C=22nF, to CPU _IRQ: ~596Hz, active low=15.25us

Memory map:
-----------
0000-0FFF: 4K RAM (2016 * 2)
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

Phantom (model 6100)
----------------
R65C02P4, XTAL marked 4.91?200
2*32KB ROM 27C256-15, 8KB RAM MS6264L-10
LCD driver, display panel for digits
magnetized x/y motor under chessboard, chesspieces have magnet underneath
piezo speaker, LEDs, 8*8 chessboard buttons
PCB label 510.1128A01

Fidelity licensed the design of the Milton/Phantom motorized chessboard and released
their own version. It has a small LCD panel added, the rest looks nearly the same from
the outside. After Fidelity was taken over by H&G, it was rereleased in 1990 as the
Mephisto Phantom. This is assumed to be identical.


******************************************************************************

Chesster (model 6120)
There is also a German version titled Kishon Chesster (model 6120G, or 6127)
----------------

8*(8+1) buttons, 8+8+1 LEDs
8KB RAM(UM6264-12), 32KB ROM(M27C256B)
Ricoh RP65C02G CPU, 5MHz XTAL
8-bit DAC speech timed via IRQ, 128KB ROM(AMI custom label)
PCB label 510.1141C01

I/O is via TTL, very similar to Designer Display

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/bankdev.h"
#include "machine/6821pia.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_as12.lh" // clickable
#include "fidel_chesster.lh" // clickable
#include "fidel_csc.lh" // clickable, with preliminary boardpieces simulation
#include "fidel_eag.lh" // clickable
#include "fidel_eas.lh" // clickable
#include "fidel_pc.lh" // clickable
#include "fidel_playmatic.lh" // clickable
#include "fidel_rsc_v2.lh" // clickable
#include "fidel_sc9.lh" // clickable
#include "fidel_sc12.lh" // clickable
#include "fidel_su9.lh" // clickable


class fidel6502_state : public fidelbase_state
{
public:
	fidel6502_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_mainmap(*this, "mainmap"),
		m_div_config(*this, "div_config")
	{ }

	void eas_base(machine_config &config);
	void eas(machine_config &config);
	void eag(machine_config &config);
	void pc(machine_config &config);
	void init_eag();

	void sc12_map(address_map &map);
	void sc12(machine_config &config);
	void sc12b(machine_config &config);

	void as12(machine_config &config);

	void fphantom(machine_config &config);
	void init_fphantom();

private:
	// devices/pointers
	optional_device<i8255_device> m_ppi8255;
	optional_device<address_map_bank_device> m_mainmap;
	optional_ioport m_div_config;

	// common
	void div_trampoline_w(offs_t offset, u8 data);
	u8 div_trampoline_r(offs_t offset);
	void div_set_cpu_freq(offs_t offset);
	void div_trampoline(address_map &map);
	u16 m_div_status;

	// EAS, EAG, PC
	void eas_prepare_display();
	DECLARE_READ8_MEMBER(eas_speech_r);
	DECLARE_WRITE8_MEMBER(eas_segment_w);
	DECLARE_WRITE8_MEMBER(eas_led_w);
	DECLARE_READ8_MEMBER(eas_input_r);
	DECLARE_WRITE8_MEMBER(eas_ppi_porta_w);
	DECLARE_READ8_MEMBER(eas_ppi_portb_r);
	DECLARE_WRITE8_MEMBER(eas_ppi_portc_w);
	void eas_map(address_map &map);
	void eag_map(address_map &map);
	void pc_map(address_map &map);

	// SC12
	DECLARE_WRITE8_MEMBER(sc12_control_w);
	DECLARE_READ8_MEMBER(sc12_input_r);

	// AS12
	void as12_prepare_display();
	DECLARE_WRITE8_MEMBER(as12_control_w);
	DECLARE_WRITE8_MEMBER(as12_led_w);
	DECLARE_READ8_MEMBER(as12_input_r);
	void as12_map(address_map &map);



	// Phantom
	DECLARE_MACHINE_RESET(fphantom);
	void fphantom_map(address_map &map);


protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void fidel6502_state::machine_start()
{
	fidelbase_state::machine_start();

	// register for savestates
	save_item(NAME(m_div_status));
}

void fidel6502_state::machine_reset()
{
	fidelbase_state::machine_reset();

	m_div_status = ~0;
}


class sc9_state : public fidelbase_state
{
public:
	sc9_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

	void sc9b(machine_config &config);
	void sc9c(machine_config &config);
	void sc9d(machine_config &config);
	void playmatic(machine_config &config);

protected:
	// SC9
	void sc9_prepare_display();
	DECLARE_WRITE8_MEMBER(sc9_control_w);
	DECLARE_WRITE8_MEMBER(sc9_led_w);
	DECLARE_READ8_MEMBER(sc9_input_r);
	DECLARE_READ8_MEMBER(sc9d_input_r);
	void sc9_map(address_map &map);
	void sc9d_map(address_map &map);
};

class sc9c_state : public sc9_state
{
public:
	sc9c_state(const machine_config &mconfig, device_type type, const char *tag) :
		sc9_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(sc9c_cpu_freq) { sc9c_set_cpu_freq(); }

protected:
	virtual void machine_reset() override;
	void sc9c_set_cpu_freq();
};

void sc9c_state::machine_reset()
{
	sc9_state::machine_reset();
	sc9c_set_cpu_freq();
}

void sc9c_state::sc9c_set_cpu_freq()
{
	// SC9(C01) was released with 1.5MHz, 1.6MHz, or 1.9MHz CPU
	u8 inp = ioport("FAKE")->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 1900000 : ((inp & 1) ? 1600000 : 1500000));
}






class chesster_state : public fidelbase_state
{
public:
	chesster_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

	void chesster(machine_config &config);
	void kishon(machine_config &config);
	void init_chesster();

private:
	int m_numbanks;

	DECLARE_WRITE8_MEMBER(chesster_control_w);
	DECLARE_READ8_MEMBER(chesster_input_r);
	void chesster_map(address_map &map);
};



class csc_state : public fidelbase_state
{
public:
	csc_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_pia(*this, "pia%u", 0)
	{ }

	void csc(machine_config &config);
	void su9(machine_config &config);
	void rsc(machine_config &config);

protected:
	optional_device_array<pia6821_device, 2> m_pia;
	void csc_prepare_display();
	DECLARE_READ8_MEMBER(csc_speech_r);
	DECLARE_WRITE8_MEMBER(csc_pia0_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia0_pb_w);
	DECLARE_READ8_MEMBER(csc_pia0_pa_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia0_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(csc_pia0_cb2_w);
	DECLARE_READ_LINE_MEMBER(csc_pia0_ca1_r);
	DECLARE_READ_LINE_MEMBER(csc_pia0_cb1_r);
	DECLARE_WRITE8_MEMBER(csc_pia1_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia1_pb_w);
	DECLARE_READ8_MEMBER(csc_pia1_pb_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia1_ca2_w);
	void csc_map(address_map &map);
	void su9_map(address_map &map);
	void rsc_map(address_map &map);
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
	// SU9 CPU is clocked 1.95MHz, DS9 is 2.5MHz
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? (5_MHz_XTAL/2) : (3.9_MHz_XTAL/2));
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// Offset-dependent CPU divider on some machines

void fidel6502_state::div_set_cpu_freq(offs_t offset)
{
	static const u16 mask = 0x6000;
	u16 status = (offset & mask) | m_div_config->read();

	if (status != m_div_status && status & 2)
	{
		// when a13/a14 is high, XTAL goes through divider(s)
		// (depending on factory-set jumper, either one or two 7474)
		float div = (status & 1) ? 0.25 : 0.5;
		m_maincpu->set_clock_scale((offset & mask) ? div : 1.0);
	}

	m_div_status = status;
}

void fidel6502_state::div_trampoline_w(offs_t offset, u8 data)
{
	div_set_cpu_freq(offset);
	m_mainmap->write8(offset, data);
}

u8 fidel6502_state::div_trampoline_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		div_set_cpu_freq(offset);

	return m_mainmap->read8(offset);
}

void fidel6502_state::div_trampoline(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(fidel6502_state::div_trampoline_r), FUNC(fidel6502_state::div_trampoline_w));
}

INPUT_PORTS_START( fidel_cpu_div_2 )
	PORT_START("div_config") // hardwired, default to /2
	PORT_CONFNAME( 0x03, 0x02, "CPU Divider" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END

INPUT_PORTS_START( fidel_cpu_div_4 )
	PORT_START("div_config") // hardwired, default to /4
	PORT_CONFNAME( 0x03, 0x03, "CPU Divider" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END



// Devices, I/O

/******************************************************************************
    CSC, SU9, RSC
******************************************************************************/

// misc handlers

void csc_state::csc_prepare_display()
{
	// 7442 0-8: led select, input mux
	m_inp_mux = 1 << m_led_select & 0x3ff;

	// 7442 9: speaker out
	m_dac->write(BIT(m_inp_mux, 9));

	// 7seg leds+H (not on all models), 8*8(+1) chessboard leds
	set_display_segmask(0xf, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | m_7seg_data, m_inp_mux);
}

READ8_MEMBER(csc_state::csc_speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}


// 6821 PIA 0

READ8_MEMBER(csc_state::csc_pia0_pa_r)
{
	// d0-d5: button row 0-5 (active low)
	return (read_inputs(9) & 0x3f) ^ 0xff;
}

WRITE8_MEMBER(csc_state::csc_pia0_pa_w)
{
	// d6,d7: 7442 A0,A1
	m_led_select = (m_led_select & ~3) | (data >> 6 & 3);
	csc_prepare_display();
}

WRITE8_MEMBER(csc_state::csc_pia0_pb_w)
{
	// d0-d7: led row data
	m_led_data = data;
	csc_prepare_display();
}

READ_LINE_MEMBER(csc_state::csc_pia0_ca1_r)
{
	// button row 6 (active low)
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(csc_state::csc_pia0_cb1_r)
{
	// button row 7 (active low)
	return ~read_inputs(9) >> 7 & 1;
}

WRITE_LINE_MEMBER(csc_state::csc_pia0_cb2_w)
{
	// 7442 A2
	m_led_select = (m_led_select & ~4) | (state ? 4 : 0);
	csc_prepare_display();
}

WRITE_LINE_MEMBER(csc_state::csc_pia0_ca2_w)
{
	// 7442 A3
	m_led_select = (m_led_select & ~8) | (state ? 8 : 0);
	csc_prepare_display();
}


// 6821 PIA 1

WRITE8_MEMBER(csc_state::csc_pia1_pa_w)
{
	// d0-d5: TSI C0-C5
	m_speech->data_w(space, 0, data & 0x3f);

	// d0-d7: data for the 4 7seg leds, bits are ABFGHCDE (H is extra led)
	m_7seg_data = bitswap<8>(data,0,1,5,6,7,2,3,4);
	csc_prepare_display();
}

WRITE8_MEMBER(csc_state::csc_pia1_pb_w)
{
	// d0: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data & 1;

	// d1: TSI START line
	m_speech->start_w(data >> 1 & 1);

	// d4: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x10) ? 0.5 : 1.0);
}

READ8_MEMBER(csc_state::csc_pia1_pb_r)
{
	// d2: printer?
	u8 data = 0x04;

	// d3: TSI BUSY line
	if (m_speech->busy_r())
		data |= 0x08;

	// d5: button row 8 (active low)
	// d6,d7: language switches
	data |= (~read_inputs(9) >> 3 & 0x20) | (m_inp_matrix[9]->read() << 6 & 0xc0);

	return data;
}

WRITE_LINE_MEMBER(csc_state::csc_pia1_ca2_w)
{
	// printer?
}




/******************************************************************************
    EAS, EAG, PC
******************************************************************************/

// TTL/generic

void fidel6502_state::eas_prepare_display()
{
	// 4/8 7seg leds+H, 8*8(+1) chessboard leds
	set_display_segmask(0x1ef, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | m_7seg_data, m_led_select);
}

READ8_MEMBER(fidel6502_state::eas_speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}

WRITE8_MEMBER(fidel6502_state::eas_segment_w)
{
	// a0-a2,d7: digit segment
	m_7seg_data = (data & 0x80) >> offset;
	m_7seg_data = bitswap<8>(m_7seg_data,7,6,4,5,0,2,1,3);
	eas_prepare_display();
}

WRITE8_MEMBER(fidel6502_state::eas_led_w)
{
	// a0-a2,d0: led data
	m_led_data = (data & 1) << offset;
	eas_prepare_display();
}

READ8_MEMBER(fidel6502_state::eas_input_r)
{
	// multiplexed inputs (active low)
	return read_inputs(9) ^ 0xff;
}

void fidel6502_state::init_eag()
{
	m_rombank->configure_entries(0, 4, memregion("rombank")->base(), 0x2000);
}


// 8255 PPI (PC: done with TTL instead)

WRITE8_MEMBER(fidel6502_state::eas_ppi_porta_w)
{
	// d0-d5: TSI C0-C5
	// d6: TSI START line
	m_speech->data_w(space, 0, data & 0x3f);
	m_speech->start_w(data >> 6 & 1);

	// d7: printer? (black wire to LED pcb)
}

WRITE8_MEMBER(fidel6502_state::eas_ppi_portc_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led select, input mux
	m_led_select = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = m_led_select & 0x1ff;
	eas_prepare_display();

	// 7442 9: speaker out
	m_dac->write(BIT(m_led_select, 9));

	// d4: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data >> 4 & 1;

	// d5: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x20) ? 0.5 : 1.0);

	// d6,d7: bookrom bankswitch (model EAG)
	if (m_rombank != nullptr)
		m_rombank->set_entry(data >> 6 & 3);
}

READ8_MEMBER(fidel6502_state::eas_ppi_portb_r)
{
	// d0: printer? white wire from LED pcb
	u8 data = 1;

	// d1: TSI BUSY line
	data |= (m_speech->busy_r()) ? 2 : 0;

	// d2,d3: language switches
	data |= m_inp_matrix[9]->read() << 2 & 0x0c;

	// d5: multiplexed inputs highest bit
	data |= (read_inputs(9) & 0x100) ? 0 : 0x20;

	// other: ?
	return data | 0xd0;
}



/******************************************************************************
    SC9
******************************************************************************/

// TTL/generic

void sc9_state::sc9_prepare_display()
{
	// 8*8 chessboard leds + 1 corner led
	display_matrix(8, 9, m_led_data, m_inp_mux);
}

WRITE8_MEMBER(sc9_state::sc9_control_w)
{
	// d0-d3: 74245 P0-P3
	// 74245 Q0-Q8: input mux, led select
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;
	sc9_prepare_display();

	// 74245 Q9: speaker out
	m_dac->write(BIT(sel, 9));

	// d4,d5: ?
	// d6,d7: N/C
}

WRITE8_MEMBER(sc9_state::sc9_led_w)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (data & 1) << offset;
	sc9_prepare_display();
}

READ8_MEMBER(sc9_state::sc9_input_r)
{
	// multiplexed inputs (active low)
	return read_inputs(9) ^ 0xff;
}

READ8_MEMBER(sc9_state::sc9d_input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    SC12
******************************************************************************/

// TTL/generic

WRITE8_MEMBER(fidel6502_state::sc12_control_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));

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



/******************************************************************************
    AS12
******************************************************************************/

// TTL/generic

void fidel6502_state::as12_prepare_display()
{
	// 8*8(+1) chessboard leds
	display_matrix(8, 9, m_led_data, m_inp_mux);
}

WRITE8_MEMBER(fidel6502_state::as12_control_w)
{
	// d0-d3: 74245 P0-P3
	// 74245 Q0-Q8: input mux, led select
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = bitswap<9>(sel,5,8,7,6,4,3,1,0,2);
	as12_prepare_display();

	// 74245 Q9: speaker out
	m_dac->write(BIT(sel, 9));

	// d4,d5: printer?
	// d6,d7: N/C?
}

WRITE8_MEMBER(fidel6502_state::as12_led_w)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (data & 1) << offset;
	as12_prepare_display();
}

READ8_MEMBER(fidel6502_state::as12_input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	u8 inp = bitswap<8>(read_inputs(9),4,3,2,1,0,5,6,7);
	return (inp >> offset & 1) ? 0 : 0x80;
}







/******************************************************************************
    Phantom
******************************************************************************/

// TTL/generic

MACHINE_RESET_MEMBER(fidel6502_state, fphantom)
{
	fidel6502_state::machine_reset();
	m_rombank->set_entry(0);
}

void fidel6502_state::init_fphantom()
{
	m_rombank->configure_entries(0, 2, memregion("rombank")->base(), 0x4000);
}



/******************************************************************************
    Chesster
******************************************************************************/

// TTL/generic

WRITE8_MEMBER(chesster_state::chesster_control_w)
{
	// a0-a2,d7: 74259(1)
	u8 mask = 1 << offset;
	m_led_select = (m_led_select & ~mask) | ((data & 0x80) ? mask : 0);

	// 74259 Q4-Q7: 7442 a0-a3
	// 7442 0-8: led data, input mux
	u16 sel = 1 << (m_led_select >> 4 & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;

	// 74259 Q0,Q1: led select (active low)
	display_matrix(9, 2, m_inp_mux, ~m_led_select & 3);

	// 74259 Q2,Q3: speechrom A14,A15
	// a0-a2,d0: 74259(2) Q3,Q2,Q0 to A16,A17,A18
	m_speech_bank = (m_speech_bank & ~mask) | ((data & 1) ? mask : 0);
	u8 bank = (m_led_select >> 2 & 3) | bitswap<3>(m_speech_bank, 0,2,3) << 2;
	m_rombank->set_entry(bank & (m_numbanks - 1));
}

READ8_MEMBER(chesster_state::chesster_input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}

void chesster_state::init_chesster()
{
	m_numbanks = memregion("rombank")->bytes() / 0x4000;
	m_rombank->configure_entries(0, m_numbanks, memregion("rombank")->base(), 0x4000);
}



/******************************************************************************
    Address Maps
******************************************************************************/

// CSC, SU9, RSC

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
	map(0xa000, 0xa7ff).rom();
	map(0xc000, 0xffff).rom();
}

void csc_state::rsc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x2000, 0x2003).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf000, 0xffff).rom();
}


// EAS, EAG, PC

void fidel6502_state::eas_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x2000, 0x5fff).r(FUNC(fidel6502_state::cartridge_r));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(fidel6502_state::eas_segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(fidel6502_state::eas_led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(fidel6502_state::eas_input_r));
	map(0x8000, 0x9fff).rom();
	map(0xc000, 0xffff).rom();
}

void fidel6502_state::eag_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x5fff).r(FUNC(fidel6502_state::cartridge_r));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(fidel6502_state::eas_segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(fidel6502_state::eas_led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(fidel6502_state::eas_input_r));
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).rom();
}

void fidel6502_state::pc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x17ff).ram();
	map(0x2000, 0x5fff).r(FUNC(fidel6502_state::cartridge_r));
	map(0x7000, 0x7000).w(FUNC(fidel6502_state::eas_ppi_porta_w));
	map(0x7010, 0x7010).r(FUNC(fidel6502_state::eas_ppi_portb_r));
	map(0x7020, 0x7027).w(FUNC(fidel6502_state::eas_segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(fidel6502_state::eas_led_w)).nopr();
	map(0x7040, 0x7040).w(FUNC(fidel6502_state::eas_ppi_portc_w));
	map(0x7050, 0x7050).r(FUNC(fidel6502_state::eas_input_r));
	map(0x8000, 0x9fff).ram();
	map(0xb000, 0xffff).rom();
}


// SC9

void sc9_state::sc9_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x5fff).r(FUNC(sc9_state::cartridge_r));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(sc9_state::sc9_control_w));
	map(0x8000, 0x8007).mirror(0x1ff8).w(FUNC(sc9_state::sc9_led_w)).nopr();
	map(0xa000, 0xa000).mirror(0x1fff).r(FUNC(sc9_state::sc9_input_r));
	map(0xc000, 0xffff).rom();
}

void sc9_state::sc9d_map(address_map &map)
{
	sc9_map(map);
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(sc9_state::sc9d_input_r));
}


// SC12, AS12

void fidel6502_state::sc12_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x2000, 0x5fff).r(FUNC(fidel6502_state::cartridge_r));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(fidel6502_state::sc12_control_w));
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(fidel6502_state::sc12_input_r));
	map(0xc000, 0xcfff).mirror(0x1000).rom();
	map(0xe000, 0xffff).rom();
}

void fidel6502_state::as12_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x1800, 0x1807).w(FUNC(fidel6502_state::as12_led_w)).nopr();
	map(0x2000, 0x5fff).r(FUNC(fidel6502_state::cartridge_r));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(fidel6502_state::as12_control_w));
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(fidel6502_state::as12_input_r));
	map(0xc000, 0xffff).rom();
}





// Designer Display, Phantom, Chesster

void fidel6502_state::fphantom_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom();
}

void chesster_state::chesster_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).mirror(0x1ff8).rw(FUNC(chesster_state::chesster_input_r), FUNC(chesster_state::chesster_control_w));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x6000, 0x6000).mirror(0x1fff).w("dac8", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0xffff).rom();
}




/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( rsc )
	PORT_INCLUDE( fidel_cb_buttons )

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


static INPUT_PORTS_START( csc )
	PORT_INCLUDE( fidel_cb_buttons )

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

	PORT_START("IN.9") // language setting, hardwired with 2 resistors/jumpers (0: Spanish, 1: French, 2: German, 3: English)
	PORT_BIT(0x03, 0x03, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( cscsp )
	PORT_INCLUDE( csc )

	PORT_MODIFY("IN.9") // set to Spanish
	PORT_BIT(0x03, 0x00, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( cscg )
	PORT_INCLUDE( csc )

	PORT_MODIFY("IN.9") // set to German
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( cscfr )
	PORT_INCLUDE( csc )

	PORT_MODIFY("IN.9") // set to French
	PORT_BIT(0x03, 0x01, IPT_CUSTOM)
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
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, su9_state, su9_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "1.95MHz (SU9)" )
	PORT_CONFSETTING(    0x01, "2.5MHz (DS9)" )
INPUT_PORTS_END

static INPUT_PORTS_START( su9sp )
	PORT_INCLUDE( su9 )

	PORT_MODIFY("IN.9") // set to Spanish
	PORT_BIT(0x03, 0x00, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( su9g )
	PORT_INCLUDE( su9 )

	PORT_MODIFY("IN.9") // set to German
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( su9fr )
	PORT_INCLUDE( su9 )

	PORT_MODIFY("IN.9") // set to French
	PORT_BIT(0x03, 0x01, IPT_CUSTOM)
INPUT_PORTS_END


static INPUT_PORTS_START( eas )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( fidel_cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Game Control") // labeled RESET on the Prestige, but led display still says - G C -
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("LV / Pawn")

	PORT_START("IN.9") // language setting, hardwired (0: Spanish, 1: French, 2: German, 3: English)
	PORT_BIT(0x03, 0x03, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( eassp )
	PORT_INCLUDE( eas )

	PORT_MODIFY("IN.9") // set to Spanish
	PORT_BIT(0x03, 0x00, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( easg )
	PORT_INCLUDE( eas )

	PORT_MODIFY("IN.9") // set to German
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( easfr )
	PORT_INCLUDE( eas )

	PORT_MODIFY("IN.9") // set to French
	PORT_BIT(0x03, 0x01, IPT_CUSTOM)
INPUT_PORTS_END


static INPUT_PORTS_START( eag )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( fidel_cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")

	PORT_START("IN.9") // language setting, hardwired (0: Spanish, 1: French, 2: German, 3: English)
	PORT_BIT(0x03, 0x03, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( eagsp )
	PORT_INCLUDE( eag )

	PORT_MODIFY("IN.9") // set to Spanish
	PORT_BIT(0x03, 0x00, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( eagg )
	PORT_INCLUDE( eag )

	PORT_MODIFY("IN.9") // set to German
	PORT_BIT(0x03, 0x02, IPT_CUSTOM)
INPUT_PORTS_END

static INPUT_PORTS_START( eagfr )
	PORT_INCLUDE( eag )

	PORT_MODIFY("IN.9") // set to French
	PORT_BIT(0x03, 0x01, IPT_CUSTOM)
INPUT_PORTS_END


static INPUT_PORTS_START( sc12_sidepanel )
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

static INPUT_PORTS_START( sc12 )
	PORT_INCLUDE( fidel_cpu_div_2 )
	PORT_INCLUDE( fidel_cb_buttons )
	PORT_INCLUDE( sc12_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( sc12b )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( fidel_cb_buttons )
	PORT_INCLUDE( sc12_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( as12 )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( fidel_cb_magnets )

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


static INPUT_PORTS_START( sc9_sidepanel )
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

static INPUT_PORTS_START( playmatic )
	PORT_INCLUDE( fidel_cb_magnets )
	PORT_INCLUDE( sc9_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( sc9 )
	PORT_INCLUDE( fidel_cb_buttons )
	PORT_INCLUDE( sc9_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( sc9c )
	PORT_INCLUDE( sc9 )

	PORT_START("FAKE")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sc9c_state, sc9c_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "1.5MHz" )
	PORT_CONFSETTING(    0x01, "1.6MHz" )
	PORT_CONFSETTING(    0x02, "1.9MHz" )
INPUT_PORTS_END



static INPUT_PORTS_START( chesster )
	PORT_INCLUDE( fidel_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Move / No")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Hint / Yes")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Take Back / Repeat")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Level / New")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Option / Replay")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Verify / Problem")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift")
INPUT_PORTS_END


static INPUT_PORTS_START( fphantom )
	PORT_INCLUDE( fidel_cb_buttons )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

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
	m_pia[0]->readpa_handler().set(FUNC(csc_state::csc_pia0_pa_r));
	m_pia[0]->readca1_handler().set(FUNC(csc_state::csc_pia0_ca1_r));
	m_pia[0]->readcb1_handler().set(FUNC(csc_state::csc_pia0_cb1_r));
	m_pia[0]->writepa_handler().set(FUNC(csc_state::csc_pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(csc_state::csc_pia0_pb_w));
	m_pia[0]->ca2_handler().set(FUNC(csc_state::csc_pia0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(csc_state::csc_pia0_cb2_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_rsc_v2);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

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
	m_pia[0]->readpa_handler().set(FUNC(csc_state::csc_pia0_pa_r));
	m_pia[0]->readca1_handler().set(FUNC(csc_state::csc_pia0_ca1_r));
	m_pia[0]->readcb1_handler().set(FUNC(csc_state::csc_pia0_cb1_r));
	m_pia[0]->writepa_handler().set(FUNC(csc_state::csc_pia0_pa_w));
	m_pia[0]->writepb_handler().set(FUNC(csc_state::csc_pia0_pb_w));
	m_pia[0]->ca2_handler().set(FUNC(csc_state::csc_pia0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(csc_state::csc_pia0_cb2_w));

	PIA6821(config, m_pia[1], 0);
	m_pia[1]->readpb_handler().set(FUNC(csc_state::csc_pia1_pb_r));
	m_pia[1]->writepa_handler().set(FUNC(csc_state::csc_pia1_pa_w));
	m_pia[1]->writepb_handler().set(FUNC(csc_state::csc_pia1_pb_w));
	m_pia[1]->ca2_handler().set(FUNC(csc_state::csc_pia1_ca2_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_csc);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(csc_state::csc_speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void csc_state::su9(machine_config &config)
{
	csc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &csc_state::su9_map);
	config.set_default_layout(layout_fidel_su9);
}

void fidel6502_state::eas_base(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 3_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &fidel6502_state::div_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&fidel6502_state::eas_map).set_options(ENDIANNESS_LITTLE, 8, 16);

	TIMER(config, "dummy_timer").configure_periodic(timer_device::expired_delegate(), attotime::from_hz(3_MHz_XTAL));

	const attotime irq_period = attotime::from_hz(38.4_kHz_XTAL/64); // through 4060 IC, 600Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(fidel6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_hz(38.4_kHz_XTAL*2)); // edge!
	TIMER(config, "irq_off").configure_periodic(FUNC(fidel6502_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_eas);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(fidel6502_state::eas_speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc", "bin,dat"));
	cartslot.set_device_load(device_image_load_delegate(&fidelbase_state::device_image_load_scc_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void fidel6502_state::eas(machine_config &config)
{
	eas_base(config);

	I8255(config, m_ppi8255); // port B: input, port A & C: output
	m_ppi8255->out_pa_callback().set(FUNC(fidel6502_state::eas_ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set(FUNC(fidel6502_state::eas_ppi_portb_r));
	m_ppi8255->out_pc_callback().set(FUNC(fidel6502_state::eas_ppi_portc_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void fidel6502_state::pc(machine_config &config)
{
	eas_base(config);

	/* basic machine hardware */
	m_maincpu->set_clock(4_MHz_XTAL); // R65C02P4
	m_mainmap->set_addrmap(AS_PROGRAM, &fidel6502_state::pc_map);
	TIMER(config.replace(), "dummy_timer").configure_periodic(timer_device::expired_delegate(), attotime::from_hz(4_MHz_XTAL));

	config.set_default_layout(layout_fidel_pc);
}

void fidel6502_state::eag(machine_config &config)
{
	eas(config);

	/* basic machine hardware */
	m_maincpu->set_clock(5_MHz_XTAL); // R65C02P4
	m_mainmap->set_addrmap(AS_PROGRAM, &fidel6502_state::eag_map);
	TIMER(config.replace(), "dummy_timer").configure_periodic(timer_device::expired_delegate(), attotime::from_hz(5_MHz_XTAL));

	config.set_default_layout(layout_fidel_eag);
}

void sc9_state::sc9d(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 3.9_MHz_XTAL/2); // R6502AP, 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9d_map);

	const attotime irq_period = attotime::from_hz(610); // from 555 timer (22nF, 102K, 2.7K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(sc9_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(41)); // active for 41us
	TIMER(config, "irq_off").configure_periodic(FUNC(sc9_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_sc9);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc", "bin,dat"));
	cartslot.set_device_load(device_image_load_delegate(&fidelbase_state::device_image_load_scc_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void sc9_state::sc9b(machine_config &config)
{
	sc9d(config);

	/* basic machine hardware */
	m_maincpu->set_clock(1500000); // from ceramic resonator "681 JSA", measured
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9_map);
}

void sc9_state::playmatic(machine_config &config)
{
	sc9b(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3100000); // approximation
	config.set_default_layout(layout_fidel_playmatic);
}

void fidel6502_state::sc12(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 3_MHz_XTAL); // R65C02P3
	m_maincpu->set_addrmap(AS_PROGRAM, &fidel6502_state::div_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&fidel6502_state::sc12_map).set_options(ENDIANNESS_LITTLE, 8, 16);

	TIMER(config, "dummy_timer").configure_periodic(timer_device::expired_delegate(), attotime::from_hz(3_MHz_XTAL));

	const attotime irq_period = attotime::from_hz(630); // from 556 timer (22nF, 102K, 1K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(fidel6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config, "irq_off").configure_periodic(FUNC(fidel6502_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_sc12);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc", "bin,dat"));
	cartslot.set_device_load(device_image_load_delegate(&fidelbase_state::device_image_load_scc_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void fidel6502_state::sc12b(machine_config &config)
{
	sc12(config);

	/* basic machine hardware */
	m_maincpu->set_clock(4_MHz_XTAL); // R65C02P4
	TIMER(config.replace(), "dummy_timer").configure_periodic(timer_device::expired_delegate(), attotime::from_hz(4_MHz_XTAL));

	// change irq timer frequency
	const attotime irq_period = attotime::from_hz(596); // from 556 timer (22nF, 82K+26K, 1K)
	TIMER(config.replace(), m_irq_on).configure_periodic(FUNC(fidel6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config.replace(), "irq_off").configure_periodic(FUNC(fidel6502_state::irq_off<M6502_IRQ_LINE>), irq_period);
}

void fidel6502_state::as12(machine_config &config)
{
	sc12b(config);

	/* basic machine hardware */
	m_mainmap->set_addrmap(AS_PROGRAM, &fidel6502_state::as12_map);

	// change irq timer frequency
	const attotime irq_period = attotime::from_hz(585); // from 556 timer (22nF, 110K, 1K)
	TIMER(config.replace(), m_irq_on).configure_periodic(FUNC(fidel6502_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(15250)); // active for 15.25us
	TIMER(config.replace(), "irq_off").configure_periodic(FUNC(fidel6502_state::irq_off<M6502_IRQ_LINE>), irq_period);

	config.set_default_layout(layout_fidel_as12);
}



void fidel6502_state::fphantom(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL); // R65C02P4
	m_maincpu->set_periodic_int(FUNC(fidel6502_state::irq0_line_hold), attotime::from_hz(600)); // guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &fidel6502_state::fphantom_map);

	MCFG_MACHINE_RESET_OVERRIDE(fidel6502_state, fphantom)

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	//config.set_default_layout(layout_fidel_phantom);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void chesster_state::chesster(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 5_MHz_XTAL); // RP65C02G
	m_maincpu->set_addrmap(AS_PROGRAM, &chesster_state::chesster_map);

	const attotime irq_period = attotime::from_hz(9615); // R/C circuit, measured
	TIMER(config, m_irq_on).configure_periodic(FUNC(chesster_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(2600)); // active for 2.6us
	TIMER(config, "irq_off").configure_periodic(FUNC(chesster_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_chesster);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac8", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // m74hc374b1.ic1 + 8l513_02.z2
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.set_output(5.0);
	vref.add_route(0, "dac8", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac8", -1.0, DAC_VREF_NEG_INPUT);
}

void chesster_state::kishon(machine_config &config)
{
	chesster(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3.579545_MHz_XTAL);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( reversic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1000a01", 0xf000, 0x1000, CRC(ca7723a7) SHA1(bd92330f2d9494fa408f5a2ca300d7a755bdf489) )
ROM_END


ROM_START( csc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) )
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( cscsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( cscg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( cscfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( super9cc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1050a01", 0x2000, 0x2000, CRC(421147e8) SHA1(ccf62f6f218e8992baf30973fe41b35e14a1cc1a) )
	ROM_LOAD("101-1024b03", 0xa000, 0x0800, CRC(e8c97455) SHA1(ed2958fc5474253ee8c2eaf27fc64226e12f80ea) )
	ROM_LOAD("101-1024b02", 0xc000, 0x2000, CRC(95004699) SHA1(ea79f43da73267344545df8ad61730f613876c2e) )
	ROM_LOAD("101-1024c01", 0xe000, 0x2000, CRC(03904e86) SHA1(bfa0dd9d8541e3ec359a247a3eba543501f727bc) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, BAD_DUMP CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // taken from csc, assume correct
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( super9ccsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1050a01", 0x2000, 0x2000, CRC(421147e8) SHA1(ccf62f6f218e8992baf30973fe41b35e14a1cc1a) )
	ROM_LOAD("101-1024b03", 0xa000, 0x0800, CRC(e8c97455) SHA1(ed2958fc5474253ee8c2eaf27fc64226e12f80ea) )
	ROM_LOAD("101-1024b02", 0xc000, 0x2000, CRC(95004699) SHA1(ea79f43da73267344545df8ad61730f613876c2e) )
	ROM_LOAD("101-1024c01", 0xe000, 0x2000, CRC(03904e86) SHA1(bfa0dd9d8541e3ec359a247a3eba543501f727bc) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( super9ccg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1050a01", 0x2000, 0x2000, CRC(421147e8) SHA1(ccf62f6f218e8992baf30973fe41b35e14a1cc1a) )
	ROM_LOAD("101-1024b03", 0xa000, 0x0800, CRC(e8c97455) SHA1(ed2958fc5474253ee8c2eaf27fc64226e12f80ea) )
	ROM_LOAD("101-1024b02", 0xc000, 0x2000, CRC(95004699) SHA1(ea79f43da73267344545df8ad61730f613876c2e) )
	ROM_LOAD("101-1024c01", 0xe000, 0x2000, CRC(03904e86) SHA1(bfa0dd9d8541e3ec359a247a3eba543501f727bc) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( super9ccfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1050a01", 0x2000, 0x2000, CRC(421147e8) SHA1(ccf62f6f218e8992baf30973fe41b35e14a1cc1a) )
	ROM_LOAD("101-1024b03", 0xa000, 0x0800, CRC(e8c97455) SHA1(ed2958fc5474253ee8c2eaf27fc64226e12f80ea) )
	ROM_LOAD("101-1024b02", 0xc000, 0x2000, CRC(95004699) SHA1(ea79f43da73267344545df8ad61730f613876c2e) )
	ROM_LOAD("101-1024c01", 0xe000, 0x2000, CRC(03904e86) SHA1(bfa0dd9d8541e3ec359a247a3eba543501f727bc) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( feasbu )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_bu.6", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) ) // ST M27C64A, unknown label
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02.4", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) ) // Mostek MK36C63N-5
	ROM_LOAD("101-1052a01.5", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) ) // Mostek MK36C63N-5

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // NEC D2332C
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( feasbusp )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_bu.6", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02.4", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) )
	ROM_LOAD("101-1052a01.5", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( feasbug )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_bu.6", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02.4", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) )
	ROM_LOAD("101-1052a01.5", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( feasbufr )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_bu.6", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02.4", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) )
	ROM_LOAD("101-1052a01.5", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( feasgla )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_gla.6", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_gla.4", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("eli_gla.5", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // NEC D2332C
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( feasglasp )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_gla.6", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_gla.4", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("eli_gla.5", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( feasglag )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_gla.6", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_gla.4", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("eli_gla.5", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( feasglafr )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_gla.6", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_gla.4", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("eli_gla.5", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( fpres )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, BAD_DUMP CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // taken from csc, assume correct
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( fpressp )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( fpresg )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( fpresfr )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( fpresbu )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, BAD_DUMP CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // taken from csc, assume correct
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( fpresbusp )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( fpresbug )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( fpresbufr )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( feag2100 )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("el2100.2",  0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100.3",  0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100.1",  0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, BAD_DUMP CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // taken from csc, assume correct
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( feag2100sp )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("el2100.2",  0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100.3",  0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100.1",  0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, BAD_DUMP CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // taken from vcc/fexcelv, assume correct
ROM_END

ROM_START( feag2100g )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("el2100.2",  0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100.3",  0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100.1",  0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( feag2100fr )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("el2100.2",  0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100.3",  0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100.1",  0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END


ROM_START( fscc9 ) // PCB label 510-1046D01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034b01", 0xc000, 0x2000, CRC(65288753) SHA1(651f5ca5969ddd72a20cbebdec2de83c4bf10650) )
	ROM_LOAD("101-1034c02", 0xe000, 0x2000, CRC(238b092f) SHA1(7ddffc6dba822aee9d8ad6815b23024ed5cdfd26) )
ROM_END

ROM_START( fscc9b ) // PCB label 510-1046B01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034a01", 0xc000, 0x2000, CRC(b845c458) SHA1(d3fda65dbd9fae44fa4b93f8207839d8fa0c367a) )
	ROM_LOAD("101-1034a02", 0xe000, 0x2000, CRC(ecfa0a4c) SHA1(738df99a250fad0b1da5ebeb8c92a9ad1461417b) )
ROM_END

ROM_START( fscc9c ) // PCB label 510-1046C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034a01", 0xc000, 0x2000, CRC(b845c458) SHA1(d3fda65dbd9fae44fa4b93f8207839d8fa0c367a) ) // HN48364P
	ROM_LOAD("101-1034b02", 0xe000, 0x2000, CRC(cbaf97d7) SHA1(7ed8e68bb74713d9e2ff1d9c037012320b7bfcbf) ) // "
ROM_END

ROM_START( fscc9ps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("play64c1.bin", 0xc000, 0x2000, CRC(e96aa95d) SHA1(16d90cf0ef166aef579d442671290a2c43e24dfe) )
	ROM_LOAD("play64en.bin", 0xe000, 0x2000, CRC(6fa188d2) SHA1(1b9b0209c496c89ecb7f9ec07bfd9429ff9b275e) )
ROM_END


ROM_START( fscc12 ) // model SC12, PCB label 510-1084B01
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("101-1068a01.ic15", 0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("orange.ic13",      0xc000, 0x1000, CRC(ed5289b2) SHA1(9b0c7f9ae4102d4a66eb8c91d4e84b9eec2ffb3d) ) // TI TMS2732AJL-45, no label, orange sticker
	ROM_LOAD("red.ic14",         0xe000, 0x2000, CRC(0c4968c4) SHA1(965a66870b0f8ce9549418cbda09d2ff262a1504) ) // TI TMS2764JL-25, no label, red sticker
ROM_END

ROM_START( fscc12b ) // model 6086, PCB label 510-1084B01
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("101-1068a01.ic15", 0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("orange.ic13",      0xc000, 0x1000, CRC(45070a71) SHA1(8aeecff828f26fb7081902c757559903be272649) ) // TI TMS2732AJL-45, no label, orange sticker
	ROM_LOAD("red.ic14",         0xe000, 0x2000, CRC(183d3edc) SHA1(3296a4c3bce5209587d4a1694fce153558544e63) ) // Toshiba TMM2764D-2, no label, red sticker
ROM_END


ROM_START( feleg ) // model AS12(or 6085), PCB label 510-1084B01
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("feleg.1", 0x8000, 0x2000, CRC(e9df31e8) SHA1(31c52bb8f75580c82093eb950959c1bc294189a8) ) // TMM2764, no label
	ROM_LOAD("feleg.2", 0xc000, 0x2000, CRC(bed9c84b) SHA1(c12f39765b054d2ad81f747e698715ad4246806d) ) // "
	ROM_LOAD("feleg.3", 0xe000, 0x2000, CRC(b1fb49aa) SHA1(d8c9687dd564f0fa603e6d684effb1d113ac64b4) ) // "
ROM_END



ROM_START( fphantom ) // model 6100, PCB label 510.1128A01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u_3c_yellow.u3", 0x8000, 0x8000, CRC(fb7c38ae) SHA1(a1aa7637705052cb4eec92644dc79aee7ba4d77c) ) // 27C256

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("u_4_white.u4",  0x0000, 0x8000, CRC(e4181ba2) SHA1(1f77d1867c6f566be98645fc252a01108f412c96) ) // 27C256
ROM_END


ROM_START( chesster ) // model 6120, PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ch_1.3.ic9", 0x8000, 0x8000, CRC(8b42d1ad) SHA1(2161fc5ab2476fe7ca4ffc226e3cb329b8a57a01) ) // 27256, CH 1.3 on sticker

	ROM_REGION( 0x20000, "rombank", 0 )
	ROM_LOAD("101-1091b02.ic10", 0x0000, 0x20000, CRC(fa370e88) SHA1(a937c8f1ec295cf9539d12466993974e40771493) ) // AMI, 27C010 or equivalent
ROM_END

ROM_START( chesstera ) // model 6120, PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chesster.ic9", 0x8000, 0x8000, CRC(29f9a698) SHA1(4c83ca46fd5fc9c40302e9c7f16b4ae2c18b06e6) ) // M27C256B, sticker but no label

	ROM_REGION( 0x20000, "rombank", 0 )
	ROM_LOAD("101-1091a02.ic10", 0x0000, 0x20000, CRC(2b4d243c) SHA1(921e51978facb502b207b4f64a73b1e74127e826) ) // AMI, 27C010 or equivalent
ROM_END

ROM_START( kishon ) // model 6120G or 6127(same), PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("kishon.ic9", 0x8000, 0x8000, CRC(121c007f) SHA1(652e9ea47b6bb1632d10eb0fcd7f98cdba22fce7) ) // 27C256

	ROM_REGION( 0x80000, "rombank", 0 )
	ROM_LOAD("kishon_v2.6_1-14-91.ic10", 0x0000, 0x80000, CRC(50598869) SHA1(2087e0c2f40a2408fe217a6502c8c3a247bdd063) ) // Toshiba TC544000P-12, aka 101-1094A01
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME        PARENT   CMP MACHINE    INPUT      STATE            INIT           COMPANY                 FULLNAME                                      FLAGS
CONS( 1981, reversic,   0,        0, rsc,       rsc,       csc_state, empty_init,    "Fidelity Electronics", "Reversi Sensory Challenger (green version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1981, csc,        0,        0, csc,       csc,       csc_state, empty_init,    "Fidelity Electronics", "Champion Sensory Chess Challenger (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1981, cscsp,      csc,      0, csc,       cscsp,     csc_state, empty_init,    "Fidelity Electronics", "Champion Sensory Chess Challenger (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1981, cscg,       csc,      0, csc,       cscg,      csc_state, empty_init,    "Fidelity Electronics", "Champion Sensory Chess Challenger (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1981, cscfr,      csc,      0, csc,       cscfr,     csc_state, empty_init,    "Fidelity Electronics", "Champion Sensory Chess Challenger (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1983, super9cc,   0,        0, su9,       su9,       su9_state, empty_init,    "Fidelity Electronics", "Super 9 Sensory Chess Challenger (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, super9ccsp, super9cc, 0, su9,       su9sp,     su9_state, empty_init,    "Fidelity Electronics", "Super 9 Sensory Chess Challenger (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, super9ccg,  super9cc, 0, su9,       su9g,      su9_state, empty_init,    "Fidelity Electronics", "Super 9 Sensory Chess Challenger (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, super9ccfr, super9cc, 0, su9,       su9fr,     su9_state, empty_init,    "Fidelity Electronics", "Super 9 Sensory Chess Challenger (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1983, feasbu,     0,        0, eas,       eas,       fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Budapest program, English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, feasbusp,   feasbu,   0, eas,       eassp,     fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Budapest program, Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, feasbug,    feasbu,   0, eas,       easg,      fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Budapest program, German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, feasbufr,   feasbu,   0, eas,       easfr,     fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Budapest program, French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1984, feasgla,    feasbu,   0, eas,       eas,       fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1984, feasglasp,  feasbu,   0, eas,       eassp,     fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1984, feasglag,   feasbu,   0, eas,       easg,      fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1984, feasglafr,  feasbu,   0, eas,       easfr,     fidel6502_state, empty_init,    "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1982, fpres,      0,        0, pc,        eas,       fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (original program, English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1982, fpressp,    fpres,    0, pc,        eassp,     fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (original program, Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1982, fpresg,     fpres,    0, pc,        easg,      fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (original program, German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1982, fpresfr,    fpres,    0, pc,        easfr,     fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (original program, French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fpresbu,    fpres,    0, pc,        eas,       fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (Budapest program, English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fpresbusp,  fpres,    0, pc,        eassp,     fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (Budapest program, Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fpresbug,   fpres,    0, pc,        easg,      fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (Budapest program, German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fpresbufr,  fpres,    0, pc,        easfr,     fidel6502_state, empty_init,    "Fidelity Electronics", "Prestige Challenger (Budapest program, French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1986, feag2100,   0,        0, eag,       eag,       fidel6502_state, init_eag,      "Fidelity Electronics", "Elite Avant Garde 2100 (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1986, feag2100sp, feag2100, 0, eag,       eagsp,     fidel6502_state, init_eag,      "Fidelity Electronics", "Elite Avant Garde 2100 (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1986, feag2100g,  feag2100, 0, eag,       eagg,      fidel6502_state, init_eag,      "Fidelity Electronics", "Elite Avant Garde 2100 (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1986, feag2100fr, feag2100, 0, eag,       eagfr,     fidel6502_state, init_eag,      "Fidelity Electronics", "Elite Avant Garde 2100 (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1982, fscc9,      0,        0, sc9d,      sc9,       sc9_state, empty_init,    "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. D)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // aka version "B"
CONS( 1982, fscc9b,     fscc9,    0, sc9b,      sc9,       sc9_state, empty_init,    "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1982, fscc9c,     fscc9,    0, sc9b,      sc9c,      sc9c_state, empty_init,    "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. C)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fscc9ps,    fscc9,    0, playmatic, playmatic, sc9_state, empty_init,    "Fidelity Electronics", "Sensory 9 Playmatic 'S'", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // Fidelity West Germany

CONS( 1984, fscc12,     0,        0, sc12,      sc12,      fidel6502_state, empty_init,    "Fidelity Electronics", "Sensory Chess Challenger 12", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 1984, fscc12b,    fscc12,   0, sc12b,     sc12b,     fidel6502_state, empty_init,    "Fidelity Electronics", "Sensory Chess Challenger 12-B", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )

CONS( 1985, feleg,      0,        0, as12,      as12,      fidel6502_state, empty_init,    "Fidelity Electronics", "Elegance Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )

CONS( 1988, fphantom,   0,        0, fphantom,  fphantom,  fidel6502_state, init_fphantom, "Fidelity Electronics", "Phantom (Fidelity)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )

CONS( 1990, chesster,   0,        0, chesster,  chesster,  chesster_state, init_chesster, "Fidelity Electronics", "Chesster Challenger (V1.3)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, chesstera,  chesster, 0, chesster,  chesster,  chesster_state, init_chesster, "Fidelity Electronics", "Chesster Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1991, kishon,     chesster, 0, kishon,    chesster,  chesster_state, init_chesster, "Fidelity Electronics", "Kishon Chesster", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
