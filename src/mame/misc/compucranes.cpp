// license:BSD-3-Clause
// copyright-holders: Tomás García-Merás Capote (ClawGrip)
/********************************************************************************

 Driver for MCS51-based crane coinops from Compumatic.
 The same PCB was used on machines from different manufacturers, like OM Vending
 and Covielsa.

 Different hardware revisions from Compumatic, called "GANCHONEW" PCB.
 From V2 to V8 hardware with the following layout (minor changes, like the power
 supply connector, moving from PC AT to PC ATX):

 COMPUMATIC "GANCHONEW V8" CPU
							  CN7 COUNTERS
  ______________________________________
 |     ______  ______  ______  ········ |
 |     ST8251  ST8251  ST8251           |
 |      IC16    IC15    IC14            |
 | __________    ____                   |
 | ULN2803APG    LM358N                 |
 |   IC12         IC10                  |
 | __________    __________     __ __  _|_
 | SN74HC273N    SN74HC244N    | || | |   |
 |   IC6           IC7         | || | | C |
 | __________    __________    |F2 F1| O |
 | SN74HC273N    SN74HC244N    | || | | N |
 |   IC11          IC8         |_||_| | N |
 | __________    __________           |___|
 | SN74HC373N    |_GAL16V8_|            |
 |   IC2           IC4                  |
 | ________________    ____     ____  oo|
 || W29C020C  IC3 | TL7705ACP         oo|
 ||_______________|   IC9             oo|<- ATX Power
 |                    24C16  IC5      oo|   Supply conn
 | ___________________   XT1          oo|
 || TS80C32X2-MCA    |   12MHz  TEST  oo|
 ||_______IC1________|           SW1  oo|
 | ....  ...... ..  ..... .......       |
 |______________________________________|
  CN6     CN5    CN4  CN3     CN2
  + JP1
  DISPLAY SENSOR SPK  SELECT  JOYSTICK

 The MCU on the older PCBs can differ between 80C32 compatible models (found
 with a Winbond W78C32C-40 and with a TS80C32X2-MCA).  None of the programs
 touches CKCON, so the TS80C32X2 stays in its default 12 clocks per cycle mode.

 The "GANCHONEW-V2 COMP" board Octopussy runs on has the same part numbering,
 an AT PSU connector instead of the ATX one and the motor connector (CN8) on
 the opposite edge.  SW1, the test switch, is a slide one on both.  The edge
 connectors are the ones the OM Vending clone silkscreens by name: CN2
 joystick (5 ways), CN3 coin selector (7), CN4 speaker (2), CN5 sensors (7),
 CN6 display (5) and CN7 counters (12).

 "GANCHONEW/CPU-V1 COMP" PCB has a different layout, with the connectors on
 the side instead of the front edge:
			  __________
  ___________|   CN1    |_____CN8__________
 |           |_________| |||||||||||||  .|
 |:                ________   _______   :| CN2
 |:   ____        TD62783AP  HD74HC244P  |
 |:   BUZ12        ________   _______   :| CN3
 |:               TD62083AP  74HCT273N  :|
 |  ____________   ________   _______    | CN4
 | | EPROM  IC3|  HD74HC373P HD74HC244P :|
 | |___________|   ________   _______    | CN5
 |                PALCE16V8H HD74HC273P :|
 |  _______________    ____   _______   :| CN6
 | | TSC80C31-12CA|  24LC16B TD62083AP   | + JP1
 | |____IC1_______|    IC5      IC12    :|
 |     CN7           XT1 12MHz    P1    :|
 |_______________________________________|

 Its part numbering matches the later boards, JP1 (a three pin header here)
 sits again next to the display connector, and the motors and the claw magnet
 are driven by the Darlington arrays.

 The OM Vending clone is silkscreened "CPU GRUA V2  O. M. VENDING":

  _______________________________________________________
 |  ___     ___     ___        ______________            |
 | |IC13|  |IC15|  |IC14|     |__CN7________|  CONTADORES|
 | (Multiwatt-15 power devices) D8..D21 (flyback diodes) _|_
 | P1  R1 R2  ____                    __________        |   |
 |[#] o-o-o  |IC10|  JP1 JP2 JP3     |_SN74HC32N| IC16  | C |
 |  ___________ AR5                  ____    _______    | N |
 | |SN74HC273N| IC6      F1 (BOBINA 3A)  |  |ULN2803|   | 8 |
 |  _______________              _______________ IC12   |___|
 | |SST 39SF040   | IC3         |24C16WP| IC5    ______ MOTOR
 | |______________|              _______________ IC11  |
 |  ___________   _________     |TLC7705| IC9    ______ |
 | |SN74HC373N|  |ATF16V8B |     AR2 AR3         IC7    |
 |  IC2  AR1     |_IC4_____|                     ______ |
 |  _________________________   XT1 12MHz        IC8    |
 | |AT89S52 24PU            |                   ____    |
 | |____IC1__________________|                  SW1 TEST|
 |  [CN6]   [CN5]      [CN4] [CN3]        [CN2]         |
 |_______________________________________________________|
   DISPLAY SENSOR+V.RET. ALTAVOZ SELECTOR    JOYSTICK
	5 pins    7 pins     2 pins   7 pins

 On that board the flash /OE is the wired-OR of /PSEN and /RD (D2 and D3 next
 to the GAL), so code and samples come from the same device, and IC16 (a quad
 OR gate) drives the enable input of each motor bridge from its two direction
 bits, hence both bits low = coast and both high = brake.  JP1 and JP2 route
 the motor supply (CN7.9 / CN7.6 or CN8) and JP3 sets the coin selector input
 type ("C.A." open collector or TTL), which matches the two coin reading modes
 the firmware supports.

 --------------------------------------------------------------------------
 Hardware notes, from the disassembly of the six dumped program ROMs:

 The EPROM holds the code in its first 64 KBytes and the sound samples above
 them, read with MOVX while a bank line is low.  The power-on checksum adds up
 the whole EPROM through both paths and expects zero ("EPro" is shown
 otherwise); the OM Vending clone has no checksum.  The samples are 8 bit
 unsigned PCM terminated by a zero byte; the timer 0 interrupt (6.67 kHz)
 mixes two of them and writes the result to the DAC.

 The ATF16V8 dumped from the V2, V7 and V8 boards does the decoding in ext_r
 and ext_w if its pins are 1 /PSEN, 2 /RD, 3 /WR, 5 P3.5, 6 A15, 7 A14, 8 A13,
 9 A0 and 19 flash A16, an assignment that fits every access the firmware does
 (not checked on a PCB).  Pins 12 and 13 just repeat pin 11, whose signal is
 unknown.  The V1 and OM Vending PLDs aren't dumped.

 The "GANCHONEW" (V1) board has no DAC: the timer 0 interrupt pulses P3.4 for
 about 23 us at the frequency of the note, a train of narrow pulses rather
 than a square wave.  Its power-on checksum reads the input ports too, so it
 only passes with all the inputs idle: resetting it while the crane rests on
 its home switches shows "EPro".

 Display: the four digits are not multiplexed, the 32 segment lines are driven
 by a serial LED driver on the "Plumadig" board.  The firmware supports two
 different display boards, selected by JP1 (bit 5 of the 8001h port, P1.4 on
 the V1 board), and carries a different segment table for each:
  - bit 5 low: MM5450 style driver, segments active high.  Frames are 36
	clocks long, the 32 data bits followed by 0,0,0,1, that trailing '1' being
	the start bit of the next frame, so each frame latches the data sent on
	the previous one.  It's the one emulated, fitted on every board seen, and
	the same protocol as MAME's mm5445 family, but the chip hasn't been
	identified.  The firmware always sends a blank frame right before the data
	one, so the display is really blanked for about 0.5 ms on each refresh
	(every 12 ms), unnoticeable on the real LEDs but not when sampled at the
	frontend frame rate, hence the PWM display device.
  - bit 5 high: four dummy clocks with data low followed by the 32 bits shifted
	out by the MCS51 serial port in mode 0 (plus a latch strobe on P1.2 on the
	V1 board) to a shift register board, leftmost digit first, segments active
	low: bit 0 g, 1 f, 2 a, 3 b, 4 e, 5 d, 6 c, 7 dp.  Not emulated.

 The 24C16 must hold the machine type code at address 1 (and a valid BCD value
 at address 2) or the firmware wipes the last 256 bytes of it and hangs on
 purpose: that code is factory programmed and never rewritten by the game,
 while all the other settings are rebuilt by the machine itself when their
 checksums fail ("cLE" is shown on the display while doing so).  No SEEPROM
 has been dumped, so the ones loaded here are hand built: each one is what
 this driver leaves in a SEEPROM holding just those two bytes once the machine
 has initialized it and nothing else changes.

 The crane is simulated just enough for the game cycle and for the power-on
 self test, which drives every motor until its limit switch closes and shows
 an error ("F Fr", "F  I", "F do", "F uP"...) when one doesn't.  The travel
 times and the starting position are arbitrary, not taken from a real
 cabinet, and prizes aren't simulated.  That matters on the 2012 and later
 programs (because of the Spanish law), which keep replaying a credit until the
 prize sensor sees a prize:  up to 10 more games with the default "cArA" setting
 (no, 2, 5, 10, 20, 35, 50 or 75).

 TODO:
  - Emulate the shift register display board (needs the MCS51 serial port
	mode 0 output emulated on the port pins).
  - Dump a real SEEPROM and the missing PLDs.

********************************************************************************/

#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/i2cmem.h"
#include "sound/dac.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

#include <algorithm>

#include "compucranes.lh"


namespace
{

class compucranes_state : public driver_device
{
public:
	compucranes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i2cmem(*this, "i2cmem")
		, m_dac(*this, "dac")
		, m_speaker(*this, "speaker")
		, m_display(*this, "display")
		, m_rom(*this, "program")
		, m_inputs(*this, "IN%u", 0U)
		, m_conf(*this, "CONF")
		, m_outputs(*this, "out%u", 0U)
		, m_motors(*this, "motor%u", 0U)
		, m_claw(*this, "claw")
		, m_crane_fb(*this, "crane_fb")
		, m_crane_lr(*this, "crane_lr")
		, m_crane_z(*this, "crane_z")
	{
	}

	ioport_value limits_r();
	ioport_value limits_v1_r();

	void ganchonew(machine_config &config) ATTR_COLD;
	void ganchonew_v1(machine_config &config) ATTR_COLD;
	void toyshop(machine_config &config) ATTR_COLD;

	void init_toyshop() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<mcs51_cpu_device> m_maincpu;
	required_device<i2cmem_device> m_i2cmem;
	optional_device<dac_8bit_r2r_device> m_dac;
	optional_device<speaker_sound_device> m_speaker;
	required_device<pwm_display_device> m_display;
	required_region_ptr<u8> m_rom;
	required_ioport_array<2> m_inputs;
	optional_ioport m_conf;
	output_finder<8> m_outputs;
	output_finder<6> m_motors;
	output_finder<> m_claw;
	output_finder<> m_crane_fb;
	output_finder<> m_crane_lr;
	output_finder<> m_crane_z;

	void common(machine_config &config) ATTR_COLD;

	void program_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void ext_v1_map(address_map &map) ATTR_COLD;

	u8 ext_r(offs_t offset);
	u8 ext_v1_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);
	void ext_v1_w(offs_t offset, u8 data);

	u8 p1_r();
	u8 p1_v1_r();
	void p1_w(u8 data);
	void p1_v1_w(u8 data);
	void p3_w(u8 data);
	void p3_v1_w(u8 data);
	void p3_toyshop_w(u8 data);

	void motors_w(u8 data);
	void outputs_w(u8 data);
	void display_w(u8 data);
	void set_motors(u8 data);
	void mech_update();

	u32 m_bank = 0;
	u64 m_shifter = 0;
	bool m_disp_clk = false;
	u8 m_p3 = 0xff;
	bool m_claw_on_latch = false;

	// crane position from 0.0 to 1.0: front to back, left to right and claw up
	// to down; starts clear of every limit switch, which the V1 checksum reads
	double m_pos[3] = { 0.5, 0.5, 0.1 };
	u8 m_motor_state = 0;
	attotime m_mech_time;
};


void compucranes_state::machine_start()
{
	m_mech_time = machine().time();

	save_item(NAME(m_pos));
	save_item(NAME(m_motor_state));
	save_item(NAME(m_mech_time));
	save_item(NAME(m_bank));
	save_item(NAME(m_shifter));
	save_item(NAME(m_disp_clk));
	save_item(NAME(m_p3));
}

void compucranes_state::machine_reset()
{
	// the CPU reset has already written all ones to the ports

	// assumed: the reset line clears both 74HC273 latches
	outputs_w(0);
	if (m_dac)
		m_dac->write(0);
	else
		motors_w(0);

	mech_update();
}

void compucranes_state::init_toyshop()
{
	// EA presumably tied low: run from the external flash, not the internal one
	m_maincpu->space(AS_PROGRAM).install_rom(0x0000, 0xffff, &m_rom[0]);

	// the claw magnet ("BOBINA 3A") is on bit 7 of the A001h latch here
	m_claw_on_latch = true;
}


/********************************************************************************
	Memory maps
********************************************************************************/

void compucranes_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region(m_rom, 0);
}

void compucranes_state::ext_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(compucranes_state::ext_r), FUNC(compucranes_state::ext_w));
}

void compucranes_state::ext_v1_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(compucranes_state::ext_v1_r), FUNC(compucranes_state::ext_v1_w));
}

u8 compucranes_state::ext_r(offs_t offset)
{
	if (m_bank)
		return m_rom[((m_bank << 16) | offset) & (m_rom.bytes() - 1)];
	else if ((offset & 0xe000) == 0x8000)
		return m_inputs[BIT(offset, 0)]->read();
	else
		return 0xff;
}

u8 compucranes_state::ext_v1_r(offs_t offset)
{
	switch (offset)
	{
	case 0x8000: return m_inputs[0]->read();
	case 0x8001: return m_inputs[1]->read();
	}

	return m_rom[((m_bank << 16) | offset) & (m_rom.bytes() - 1)];
}

void compucranes_state::ext_w(offs_t offset, u8 data)
{
	if (m_bank || ((offset & 0xe000) != 0xa000))
		return;

	if (BIT(offset, 0))
		outputs_w(data);
	else
		m_dac->write(data);
}

void compucranes_state::ext_v1_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0xa000: motors_w(data); break;
	case 0xa001: outputs_w(data); break;
	}
}


/********************************************************************************
	Crane mechanics simulation
********************************************************************************/

void compucranes_state::mech_update()
{
	// arbitrary speeds, in full travels per second
	static constexpr double SPEED[3] = { 1.0 / 3.0, 1.0 / 3.0, 1.0 / 2.0 };

	attotime const now = machine().time();
	double const elapsed = (now - m_mech_time).as_double();
	m_mech_time = now;

	for (int axis = 0; axis < 3; axis++)
	{
		int const dir = BIT(m_motor_state, axis * 2) - BIT(m_motor_state, axis * 2 + 1);
		m_pos[axis] = std::clamp(m_pos[axis] + dir * SPEED[axis] * elapsed, 0.0, 1.0);
	}

	m_crane_fb = int(m_pos[0] * 100.0 + 0.5);
	m_crane_lr = int(m_pos[1] * 100.0 + 0.5);
	m_crane_z = int(m_pos[2] * 100.0 + 0.5);
}

void compucranes_state::set_motors(u8 data)
{
	// bit 0 back, 1 front, 2 right, 3 left, 4 claw down, 5 claw up
	mech_update();
	m_motor_state = data & 0x3f;

	for (int i = 0; i < 6; i++)
		m_motors[i] = BIT(data, i);
}

ioport_value compucranes_state::limits_r()
{
	// the firmware knows which end of each horizontal axis it's heading to
	if (!machine().side_effects_disabled())
		mech_update();
	return
			((m_pos[2] <= 0.0) ? 0 : 0x01) |
			((m_pos[2] >= 1.0) ? 0 : 0x02) |
			((m_pos[1] <= 0.0 || m_pos[1] >= 1.0) ? 0 : 0x04) |
			((m_pos[0] <= 0.0 || m_pos[0] >= 1.0) ? 0 : 0x08);
}

ioport_value compucranes_state::limits_v1_r()
{
	if (!machine().side_effects_disabled())
		mech_update();
	return
			((m_pos[0] >= 1.0) ? 0 : 0x01) |
			((m_pos[0] <= 0.0) ? 0 : 0x02) |
			((m_pos[1] >= 1.0) ? 0 : 0x04) |
			((m_pos[1] <= 0.0) ? 0 : 0x08) |
			((m_pos[2] >= 1.0) ? 0 : 0x10) |
			((m_pos[2] <= 0.0) ? 0 : 0x20);
}


/********************************************************************************
	I/O
********************************************************************************/

void compucranes_state::motors_w(u8 data)
{
	// V1 board; bit 7 is unknown, the firmware toggles it during the game
	set_motors(data);

	m_claw = BIT(data, 6);
}

void compucranes_state::outputs_w(u8 data)
{
	// lamps, counters and token hopper, through a ULN2803
	for (int i = 0; i < 8; i++)
		m_outputs[i] = BIT(data, i);

	if (m_claw_on_latch)
		m_claw = BIT(data, 7);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

void compucranes_state::display_w(u8 data)
{
	bool const clk = BIT(data, 1);

	if (clk && !m_disp_clk)
	{
		m_shifter = (m_shifter << 1) | BIT(data, 0);

		// start bit at the end of the 36 bit shift register
		if (BIT(m_shifter, 35))
		{
			for (int digit = 0; digit < 4; digit++)
			{
				// bits as sent: a f g e d dp c b
				m_display->write_row(digit, bitswap<8>(u8(m_shifter >> (27 - 8 * digit)), 2, 5, 6, 4, 3, 1, 0, 7));
			}
			m_shifter = 0;
		}
	}

	m_disp_clk = clk;
}

u8 compucranes_state::p1_r()
{
	return 0xfd | (m_i2cmem->read_sda() << 1);
}

u8 compucranes_state::p1_v1_r()
{
	return 0xe5 | (m_i2cmem->read_sda() << 1) | (m_conf->read() & 0x18);
}

void compucranes_state::p1_w(u8 data)
{
	m_i2cmem->write_scl(BIT(data, 0));
	m_i2cmem->write_sda(BIT(data, 1));
	set_motors(data >> 2);
}

void compucranes_state::p1_v1_w(u8 data)
{
	m_i2cmem->write_scl(BIT(data, 0));
	m_i2cmem->write_sda(BIT(data, 1));

	m_bank = BIT(data, 7); // probably the EPROM A16
}

void compucranes_state::p3_w(u8 data)
{
	display_w(data);

	m_bank = BIT(~data, 5); // EPROM A16
	m_claw = BIT(data, 4);  // claw magnet, PWMed to set its strength

	m_p3 = data;
}

void compucranes_state::p3_v1_w(u8 data)
{
	display_w(data);

	if (BIT(data ^ m_p3, 4))
		m_speaker->level_w(BIT(data, 4));

	m_p3 = data;
}

void compucranes_state::p3_toyshop_w(u8 data)
{
	display_w(data);

	m_bank = bitswap<3>(u8(~data), 5, 4, 3); // EPROM A16-A18

	m_p3 = data;
}


/********************************************************************************
	Inputs
********************************************************************************/

static INPUT_PORTS_START(ganchonew)
	PORT_START("IN0")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(compucranes_state::limits_r))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1) // only used when not set to start automatically
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Prize Sensor")  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Hopper Sensor") PORT_CODE(KEYCODE_H)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    // towards the back
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  // towards the front
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_CONFNAME(0x20, 0x00, "JP1 - Display Board")
	PORT_CONFSETTING(   0x00, "Serial LED driver (MM5450 type)")
	PORT_CONFSETTING(   0x20, "Shift registers (not emulated)")
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Alarm Sensor")  PORT_CODE(KEYCODE_A)

	PORT_START("COINS") // polled, not used as interrupts
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0xf3, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(ganchonew_v1)
	PORT_START("IN0")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(compucranes_state::limits_v1_r))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Prize Sensor")  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Hopper Sensor") PORT_CODE(KEYCODE_H)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    // towards the back
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  // towards the front
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_SERVICE(0x40, IP_ACTIVE_LOW)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START1) // only used when not set to start automatically

	PORT_START("COINS")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CONF")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Alarm Sensor")  PORT_CODE(KEYCODE_A)
	PORT_CONFNAME(0x10, 0x00, "Display Board")
	PORT_CONFSETTING(   0x00, "Serial LED driver (MM5450 type)")
	PORT_CONFSETTING(   0x10, "Shift registers (not emulated)")
	PORT_BIT(0xe7, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/********************************************************************************
	Machine configs
********************************************************************************/

void compucranes_state::common(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &compucranes_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &compucranes_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(compucranes_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(compucranes_state::p1_w));
	m_maincpu->port_in_cb<3>().set_ioport("COINS");

	I2C_24C16(config, m_i2cmem);

	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0xff);
	m_display->set_interpolation(1.0);
	m_display->set_bri_levels(0.5); // ignore the sub-millisecond blanking between frames

	SPEAKER(config, "mono").front_center();
}

void compucranes_state::ganchonew(machine_config &config)
{
	I80C32(config, m_maincpu, 12_MHz_XTAL);

	common(config);

	m_maincpu->port_out_cb<3>().set(FUNC(compucranes_state::p3_w));

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.5); // 74HC273 + resistor ladder + LM358
}

void compucranes_state::ganchonew_v1(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL);

	common(config);

	m_maincpu->set_addrmap(AS_DATA, &compucranes_state::ext_v1_map);
	m_maincpu->port_in_cb<1>().set(FUNC(compucranes_state::p1_v1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(compucranes_state::p1_v1_w));
	m_maincpu->port_out_cb<3>().set(FUNC(compucranes_state::p3_v1_w));

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void compucranes_state::toyshop(machine_config &config)
{
	AT89S52(config, m_maincpu, 12_MHz_XTAL);

	common(config);

	m_maincpu->port_out_cb<3>().set(FUNC(compucranes_state::p3_toyshop_w));

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/********************************************************************************
	ROM definitions
********************************************************************************/

// "GANCHONEW/CPU-V1 COMP" PCB. Temic TSC80C31-12CA CPU, 32 pin windowed EPROM.
ROM_START(crsauruss)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("30.01.ic3",   0x00000, 0x20000, CRC(c735e024) SHA1(63dd3a71472bde7f9dead49a8dc889365fd024ef)) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("palce16v8h.ic4", 0x00000, 0x00117, NO_DUMP) // AMD PALCE16V8H-25

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24lc16b.ic5", 0x00000, 0x00800, BAD_DUMP CRC(7213cbb9) SHA1(7417c83c5a5254f86f3d56529341ae8a254e8e53)) // hand built, see the notes at the top
ROM_END

// "GANCHONEW-V8" PCB with ATX PSU connector. TS80C32X2-MCA CPU, Winbond W29C020C flash, Lattice GAL16V8A or Atmel ATF16V8B at IC4.
ROM_START(mastcrane)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("v8.ic3",      0x00000, 0x40000, CRC(733dfcbc) SHA1(d18d7945e9b8f189f2169d3d90c3cfea97d3b39c)) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("gal16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7))

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24c16_v8.ic5", 0x00000, 0x00800, BAD_DUMP CRC(9b919023) SHA1(aafbabfc70f33e0a453c6bd9bec2c7127733fb15)) // hand built, see the notes at the top
ROM_END

// "GANCHONEW V7" PCB with AT PSU connector
ROM_START(mastcranea)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("v7.ic3",      0x00000, 0x40000, CRC(299c9ad1) SHA1(b0ba2ab588151dba89307e118ba061cad2b8116b)) // 1ST AND 2ND HALF IDENTICAL (W29C020C)

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7))

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24c16_v7.ic5", 0x00000, 0x00800, BAD_DUMP CRC(eebe1da3) SHA1(472650d0884aff0b3d406c17bbca32af41468070)) // hand built, see the notes at the top
ROM_END

// "GANCHONEW V2" PCB with AT PSU connector. W78C32C-40 CPU.
ROM_START(mastcraneb)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("505.ic3",     0x00000, 0x20000, CRC(3dbb83f1) SHA1(3536762937332add0ca942283cc22ff301884a4a))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf168b.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7))

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24c16_v2.ic5", 0x00000, 0x00800, BAD_DUMP CRC(2d4ce67d) SHA1(77f2cd20f057dbfe5cd99e0eb7f14274781bd8ad)) // hand built, see the notes at the top
ROM_END

/* "GANCHONEW-V2 COMP" PCB with AT PSU connector, machine number sticker "NºMAQ. 00-356  13/06/00".
   The flash is at IC3 (it was listed as IC5, which is the SEEPROM, when the set was added). */
ROM_START(octopussy)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("w29c011.ic3", 0x00000, 0x20000, CRC(47da93e8) SHA1(aa821dd22c1912ec2942ca6afd989d61df4387d7))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, CRC(4d665a06) SHA1(504f0107482f636cd216579e982c6162c0b120a7))

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24c16.ic5",   0x00000, 0x00800, BAD_DUMP CRC(1c93e051) SHA1(e1cd62da24b049377d3103f1aae088d4581789c2)) // hand built, see the notes at the top
ROM_END

/* Direct clone of the GANCHONEW PCB by OM Vending, silkscreened "CPU GRUA V2  O. M. VENDING".
   The whole program, vectors included, is in the external flash, so the AT89S52 EA pin is
   presumably tied low (see init_toyshop), but it hasn't been traced on the PCB. */
ROM_START(toyshop)
	ROM_REGION(0x02000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("89s52.ic1",   0x00000, 0x02000, NO_DUMP)

	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("39sf040.ic3", 0x00000, 0x80000, CRC(0d9d157d) SHA1(e70f095d3524e3a4c8d5d07857bb2692b6260cc1))

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD("atf16v8.ic4", 0x00000, 0x00117, NO_DUMP)

	ROM_REGION(0x00800, "i2cmem", 0)
	ROM_LOAD("24c16.ic5",   0x00000, 0x00800, BAD_DUMP CRC(ab4445d8) SHA1(5ee38c6ac64442b25e6707f492ac92d753ee111c)) // hand built, see the notes at the top
ROM_END

} // anonymous namespace

// Years and versions are the ones the programs show on the display (or store in the SEEPROM) at power on
//     YEAR  NAME        PARENT     MACHINE       INPUT         CLASS              INIT          ROT   COMPANY               FULLNAME                       FLAGS                                                             LAYOUT
GAMEL( 2002, crsauruss,  0,         ganchonew_v1, ganchonew_v1, compucranes_state, empty_init,   ROT0, "Recreativos Presas", "Cranesaurus Single (v30.01)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 28/01/2002
GAMEL( 2012, mastcrane,  0,         ganchonew,    ganchonew,    compucranes_state, empty_init,   ROT0, "Compumatic",         "Master Crane (v44.12)",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 30/04/2012
GAMEL( 2016, mastcranea, mastcrane, ganchonew,    ganchonew,    compucranes_state, empty_init,   ROT0, "Compumatic",         "Master Crane (v46.11)",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 05/12/2016
GAMEL( 2001, mastcraneb, mastcrane, ganchonew,    ganchonew,    compucranes_state, empty_init,   ROT0, "Compumatic",         "Master Crane (v05.05)",       MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 16/10/2001
GAMEL( 2000, octopussy,  0,         ganchonew,    ganchonew,    compucranes_state, empty_init,   ROT0, "Covielsa",           "Octopussy (v21.01)",          MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 29/05/2000
GAMEL( 2016, toyshop,    0,         toyshop,      ganchonew,    compucranes_state, init_toyshop, ROT0, "OM Vending",         "Toy Shop (v17.01)",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_compucranes ) // 09/12/2016
