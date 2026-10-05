// license:BSD-3-Clause
// copyright-holders: Tomás García-Merás Capote (ClawGrip)

/*
  Gaelco 'Futbol-3' hardware for kiddie rides, pinballs, and electromechanicals
  from Gaelco, Cresmatic, Rumatic, and other manufacturers.

  The PCB is very compact and has few components. The main ones are:

  PIC16C56 or PIC16C54 as main CPU (RC oscillator, no crystal)
  OKI M6295 for sound (44-pin QFP, no oscillator of its own)
  AMD Am27C020 sound ROM
  MC74HCT273A output latch driving an ULN2803A
  SN74LS365AN buffer for the DIP switches
  2x TLP504A optocouplers
  1 bank of 6 dips

  Gaelco FUTBOL-3 PCB
  _____________________________________________________
  |JP1  JP2         __________                        |
  | __   ___        |ULN2803A_|                       |
  || |  |  |        ___________                       |
  || |  |  |        |MC74HCT273A                      |
  ||_|  |  |        _________   _________             |
  |JP3  |  |        |TLP504A_| |TLP504A_|             |
  | __  |  |        __________                        |
  || |  |  |  C11   |PIC16C56|                        |
  ||_|  |  |                                          |
  | ___ |__|   ___ <-SN74LS365AN                      |
  | VOL        |  |   ______    ___________________   |
  |  _______   |  |  | OKI |   |ROM U1             |  |
  | |DIPSx6|   |  |  |6295_|   |___________________|  |
  |            |__|                                   |
  |___________________________________________________|

  JP1 = 10 pin [+5V, GND, DAT, CLK, ENA, PU1, PU2, PU3, PU4, GND]
  JP2 = 14 pin [12VA, 12VA, +5V, ALT, CON, BOM, MOT, N/U, BOM, POT, ALT, 12V, GND, GND]
  JP3 =  5 pin [PU5, PU6, PU7, PU8, GND]
  C11 =  Trimmer

  There is a newer version of the PCB with the same components (Gaelco REF.920505, from 1992).
  It adds a fuse, a LED for PCB control, and better connectors, but it only has the single
  15-pin connector, without connector for the external display board.

  'autopapa', 'mueve', 'donpepito', and 'obladi' were found also with 27C040 EPROMs instead of 27C020, with
  1st and 2nd half identical and the same contents as the 27C020 versions.

  'IRN' kiddie ride program:
	- Idle: with demo sounds enabled, phrase 1 plays about every 4 minutes while the lamps blink.
	- A credit starts the ride: phrase 2, then phrase 8 (the song) loops on voice 1, the motor runs, the lamps
	  alternate and the time display counts from 99 down to 0.
	- At 75, 50 and 25 Q7 goes off for about 0.7 seconds, then Q4 toggles, and phrase 9 plays on voice 4, with the
	  buttons ignored meanwhile. The driver takes Q7 as the motor, but the real rides don't stop there: Q4 and Q7
	  are the MOT and POT pins of JP2, and which one is which, and what POT does, is not known.
	- Button 1 plays phrase 3 on voice 2, button 2 plays phrase 4 on voice 3.
	- End of ride: phrase 5, unless there are credits left; then phrase 7 plays and the next ride starts after
	  pressing start or after about 30 seconds.

  Its sound ROMs share phrases 2, 3, 4, 5 and 7 byte for byte, and phrase 9, which 'donpepito', 'mueve' and
  'obladi' leave empty. 'autopapa', 'donpepito', 'mueve' and 'obladi' were also found with 27C040 EPROMs holding
  the same data twice. The hex number on the sound ROM labels is the 16-bit sum of the whole EPROM.

  Pinball: the ten targets are the knocked down players of the table, the game adds the newly closed ones to the
  knockdown display of the team playing. D5 looks like the sensor of the ball in front of a goal: the game waits
  for it before kicking off, and if it stays closed the other team scores. A game has two periods, a goal is worth
  1, 2 or 5 points depending on how many targets are down, and at the end the program plays the winner phrase.
  The 'futbolt' diagnostic program plays phrase N for target N (phrase 9 for target 10 and 10 for target 9). It
  reads the M6295 status with port B still set as an output, so now and then it skips a sound; the real thing
  behaves the same.

  Crane: the joystick is not read by the PIC. Q0 and Q1 drive the crane home at power on and after the grab, Q4
  and Q5 are on during play, Q6 lights up while idle after some games and Q7 goes with the D4 check. If the crane
  does not get home in time, all the outputs go off and phrase 5 sounds. The 'GR2' program adds the D4 check
  before every game, with a 30 second timeout. Phrase 3 of its sound ROM is not used.

  Later kiddie ride PCB, with the 'I3' program: RA0 and RA2 are swapped (the program only lowers RA2 around the
  M6295 accesses) and there is no external display board, the latch drives four lamps that run a light sequence
  during a ride. Each credit adds ride time, and the songs in phrases 8 and 9 play on alternate rides. With the
  gift DIP switch on, a gift motor runs after the ride until the gift delivered input closes, or for about 16
  seconds. The 'I4' program drops the gift: when the time runs out, the lamps and the music stop but the motor
  keeps running for about 16 seconds, unless the stop input is activated.
  The Gaelco kiddie ride manual (12 VAC series, 1995) gives the same DIP switch settings, and this connector:
  JP1 = 15 pin [12VAC, GND, GND, speaker, speaker, coin, push 1, push 2, push aux, motor (to an external driver),
				light 4 or gift motor (selected with a jumper), light 3, light 2, light 1, coin counter]

  TODO:
  - Verify the M6295 SS pin: with PIN7_HIGH the known songs of 'mueve', 'donpepito' and 'obladi' play at their
	original tempo, with PIN7_LOW they would be 20% slower.
  - Verify the DIP switch order of the original PCB, and which latch output goes to each connector pin.
  - Dump the 'FUTBOL.N' PIC of the REF.920505 PCB, and find out what SW2 does. SW3 (MOT/LAMP) is probably the
	jumper of the kiddie ride manual that turns light 4 into the gift motor output.
  - Find out which of Q4 and Q7 drive the MOT and POT pins of the kiddie rides, and what POT does.
  - Find out what the crane outputs drive and what its D2 and D4 lines are.
  - Find out what the pinball outputs Q3 to Q7 drive, and which switch each of its D2, D5 and D7 lines is.
*/

#include "emu.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"

#include "speaker.h"

#include "futbol3_fut.lh"
#include "futbol3_kid.lh"
#include "futbol3_kid_i.lh"

#define LOG_OKI     (1U << 1)
#define LOG_DISPLAY (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// RC oscillator, the programs expect 4 MHz
static constexpr u32 PIC_CLOCK = 4'000'000;

class gaelcof3_state : public driver_device
{
public:
	gaelcof3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_inputs(*this, "IN0"),
		m_dsw(*this, "DSW1"),
		m_lamps(*this, "lamp%u", 0U),
		m_motor(*this, "motor"),
		m_aux(*this, "aux"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void gaelcof3(machine_config &config) ATTR_COLD;
	void gaelcof3_c54(machine_config &config) ATTR_COLD;

	void init_rc_wdt() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// hooks for the different external boards
	virtual void display_shift(int bit);        // Q1 falling edge (rising at the connector)
	virtual void display_strobe();              // Q2 rising edge
	virtual void update_outputs();
	virtual u8 bus_inputs_r();                  // bus value when nothing else drives it

	void common(machine_config &config) ATTR_COLD;

	required_device<pic16c5x_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_ioport m_inputs;
	required_ioport m_dsw;

	u8 m_oki_cs = 0;        // port A bit driving the M6295 /CS
	u8 m_latch_clk = 2;     // port A bit clocking the 74HCT273
	u8 m_latch = 0x00;
	u32 m_display_shift = 0;

private:
	output_finder<2> m_lamps;
	output_finder<> m_motor;
	output_finder<> m_aux;
	output_finder<4> m_digits;

	u8 m_porta = 0x0f;
	u8 m_portb = 0xff;
	u8 m_portb_driven = 0x00;

	void porta_w(offs_t offset, u8 data, u8 mem_mask);
	u8 portb_r();
	void portb_w(offs_t offset, u8 data, u8 mem_mask);

	u8 bus_r(u8 porta);
	void latch_w(u8 data);
	void update_display();
};


void gaelcof3_state::machine_start()
{
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_portb_driven));
	save_item(NAME(m_latch));
	save_item(NAME(m_display_shift));
}

void gaelcof3_state::machine_reset()
{
	// the PIC pins are high impedance after reset (assumed to be pulled up)
	m_porta = 0x0f;
	m_portb_driven = 0x00;

	// assume the 74HCT273 is cleared by the reset circuit
	m_latch = 0x00;
	update_outputs();
}

void gaelcof3_state::init_rc_wdt()
{
	// RC oscillator, watchdog enabled
	m_maincpu->set_config(0x0fff);
}


u8 gaelcof3_state::bus_r(u8 porta)
{
	// 74LS365: both enables (active low) must be asserted. The programs only do it with the M6295 /CS low too once
	// at power on, without reading the bus; the M6295 is then written on the /WR rising edge, see porta_w
	if (!BIT(porta, 1) && !BIT(porta, 3))
		return 0xc0 | (m_dsw->read() & 0x3f);

	// M6295 status read
	if (!BIT(porta, m_oki_cs) && !BIT(porta, 3))
		return m_oki->read();

	// nothing drives the bus: pull-ups and inputs
	return bus_inputs_r();
}

u8 gaelcof3_state::bus_inputs_r()
{
	return m_inputs->read();
}

void gaelcof3_state::porta_w(offs_t offset, u8 data, u8 mem_mask)
{
	// pins configured as inputs float high (assumed)
	data = (data & mem_mask) | (~mem_mask & 0x0f);

	u8 const old = m_porta;
	m_porta = data;

	// what the M6295 and the 74HCT273 see: the bits the PIC drives, the 74LS365 or the pull-ups for the rest.
	// the programs raise /WR once at power on with port B still an input, so the M6295 takes 0xc0 | dips as the
	// first byte of a command and plays phrase 64-127, which is empty in all the ROMs
	u8 const bus = (m_portb & m_portb_driven) | (bus_r(old) & ~m_portb_driven);

	// the M6295 latches a command on the /WR rising edge, with /CS asserted
	if (!BIT(old, 1) && BIT(data, 1) && !BIT(old, m_oki_cs))
	{
		LOGMASKED(LOG_OKI, "M6295 write %02x\n", bus);
		m_oki->write(bus);
	}

	// 74HCT273 clock
	if (!BIT(old, m_latch_clk) && BIT(data, m_latch_clk))
		latch_w(bus);
}

u8 gaelcof3_state::portb_r()
{
	return bus_r(m_porta);
}

void gaelcof3_state::portb_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_portb = data;
	m_portb_driven = mem_mask;
}


void gaelcof3_state::latch_w(u8 data)
{
	u8 const old = m_latch;
	m_latch = data;
	update_outputs();

	// on simultaneous edges the external board latches what was in its shift register before shifting
	if (!BIT(old, 2) && BIT(data, 2))
		display_strobe();

	if (BIT(old, 1) && !BIT(data, 1))
		display_shift(BIT(data, 0));
}

void gaelcof3_state::display_shift(int bit)
{
	m_display_shift = (m_display_shift << 1) | bit;
}

void gaelcof3_state::display_strobe()
{
	update_display();
}

void gaelcof3_state::update_outputs()
{
	machine().bookkeeping().coin_counter_w(0, BIT(m_latch, 3));
	m_aux = BIT(m_latch, 4);
	m_lamps[0] = BIT(m_latch, 5);
	m_lamps[1] = BIT(m_latch, 6);
	m_motor = BIT(m_latch, 7);
}

void gaelcof3_state::update_display()
{
	// 16-bit frame, the first bit shifted in ends at bit 15 (1 = active):
	// bit 15: credits display enable, bits 14-8: units digit segments (a, f, e, d, c, g, b)
	// bit 7:  time display enable,    bits 6-0:  tens digit segments  (f, g, c, d, e, b, a)
	u8 const units = bitswap<7>(m_display_shift >> 8, 1, 5, 4, 3, 2, 0, 6);
	u8 const tens = bitswap<7>(m_display_shift, 5, 6, 2, 3, 4, 1, 0);

	LOGMASKED(LOG_DISPLAY, "display frame %04x\n", BIT(m_display_shift, 0, 16));

	if (BIT(m_display_shift, 15))
	{
		m_digits[0] = tens;
		m_digits[1] = units;
	}

	if (BIT(m_display_shift, 7))
	{
		m_digits[2] = tens;
		m_digits[3] = units;
	}
}


// Pinball: the external board drives five 2-digit displays and returns the inputs through a shift register

class futbol_state : public gaelcof3_state
{
public:
	futbol_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelcof3_state(mconfig, type, tag),
		m_serial(*this, "SERIAL"),
		m_score_home(*this, "score_home%u", 0U),
		m_knock_home(*this, "knock_home%u", 0U),
		m_score_away(*this, "score_away%u", 0U),
		m_knock_away(*this, "knock_away%u", 0U),
		m_center(*this, "center%u", 0U),
		m_led_credit(*this, "led_credit"),
		m_led_time(*this, "led_time"),
		m_led_home(*this, "led_home"),
		m_led_away(*this, "led_away"),
		m_outputs(*this, "out%u", 3U)
	{ }


protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void display_shift(int bit) override;
	virtual void display_strobe() override;
	virtual void update_outputs() override;
	virtual u8 bus_inputs_r() override;

private:
	void load_inputs();

	required_ioport m_serial;
	output_finder<2> m_score_home;
	output_finder<2> m_knock_home;
	output_finder<2> m_score_away;
	output_finder<2> m_knock_away;
	output_finder<2> m_center;
	output_finder<> m_led_credit;
	output_finder<> m_led_time;
	output_finder<> m_led_home;
	output_finder<> m_led_away;
	output_finder<5> m_outputs;

	u16 m_input_shift = 0xffff;
};


void futbol_state::machine_start()
{
	gaelcof3_state::machine_start();
	save_item(NAME(m_input_shift));
}

void futbol_state::machine_reset()
{
	gaelcof3_state::machine_reset();
	m_input_shift = 0xffff;
}

void futbol_state::display_shift(int bit)
{
	gaelcof3_state::display_shift(bit);

	// the input shift register runs on the same clock, but ignores it while Q2 keeps it in parallel load
	if (BIT(m_latch, 2))
		load_inputs();
	else
		m_input_shift = (m_input_shift << 1) | 1;
}

void futbol_state::load_inputs()
{
	// nine bits are read out of bus line D0, target 8 first and target 9 last
	u16 const serial = m_serial->read();
	m_input_shift = (serial << 8) | (BIT(serial, 8) << 7) | 0x7f;
}

void futbol_state::display_strobe()
{
	// three bytes: display select and indicators, then the segments of the units and the tens digit.
	// the board keeps the last 24 bits, the program also clocks it while reading the inputs
	// the bits are captured before the ULN2803A inverters, so the frame comes out complemented
	auto const frame = ~m_display_shift;
	u8 const select = BIT(frame, 16, 8);
	u8 const units = BIT(frame, 8, 8);    // sent first
	u8 const tens = BIT(frame, 0, 8);

	LOGMASKED(LOG_DISPLAY, "display frame: select %02x digits %02x %02x\n", select, tens, units);

	if (BIT(select, 0))
	{
		m_score_home[0] = tens & 0x7f;
		m_score_home[1] = units & 0x7f;
	}
	if (BIT(select, 1))
	{
		m_score_away[0] = tens & 0x7f;
		m_score_away[1] = units & 0x7f;
	}
	if (BIT(select, 2))
	{
		m_knock_home[0] = tens & 0x7f;
		m_knock_home[1] = units & 0x7f;
	}
	if (BIT(select, 3))
	{
		m_knock_away[0] = tens & 0x7f;
		m_knock_away[1] = units & 0x7f;
	}
	if (BIT(select, 4))
	{
		m_center[0] = tens & 0x7f;
		m_center[1] = units & 0x7f;

		// the decimal points of the central display tell what it shows: credits when idle, time in play
		m_led_credit = BIT(units, 7);
		m_led_time = BIT(tens, 7);
	}

	// the game lights one of these while a team plays
	m_led_home = BIT(select, 5);
	m_led_away = BIT(select, 6);

	// the parallel-in shift register of the external board is loaded while Q2 is high
	load_inputs();
}

void futbol_state::update_outputs()
{
	// Q3 to Q7 drive the playfield, what each one does is not known
	for (int i = 0; i < 5; i++)
		m_outputs[i] = BIT(m_latch, i + 3);
}

u8 futbol_state::bus_inputs_r()
{
	// D0 comes from the shift register of the external board, D1 is a direct input
	return (gaelcof3_state::bus_inputs_r() & 0xfe) | BIT(m_input_shift, 15);
}


// Crane: no external board, all the latch outputs drive the machine

class grua_state : public gaelcof3_state
{
public:
	grua_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelcof3_state(mconfig, type, tag),
		m_outputs(*this, "out%u", 0U)
	{ }

protected:
	virtual void display_shift(int bit) override { }
	virtual void display_strobe() override { }
	virtual void update_outputs() override;

private:
	output_finder<8> m_outputs;
};

void grua_state::update_outputs()
{
	machine().bookkeeping().coin_counter_w(0, BIT(m_latch, 3));
	for (int i = 0; i < 8; i++)
		m_outputs[i] = BIT(m_latch, i);
}


// Later kiddie ride PCB: RA0 and RA2 swapped, lamps instead of the external display board

class kiddie_i_state : public gaelcof3_state
{
public:
	kiddie_i_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelcof3_state(mconfig, type, tag),
		m_lamp(*this, "lamp%u", 0U),
		m_gift_motor(*this, "gift_motor"),
		m_motor_out(*this, "motor")
	{
		m_oki_cs = 2;
		m_latch_clk = 0;
	}

protected:
	virtual void display_shift(int bit) override { }
	virtual void display_strobe() override { }
	virtual void update_outputs() override;

private:
	output_finder<4> m_lamp;
	output_finder<> m_gift_motor;
	output_finder<> m_motor_out;
};

void kiddie_i_state::update_outputs()
{
	for (int i = 0; i < 4; i++)
		m_lamp[i] = BIT(m_latch, i);
	machine().bookkeeping().coin_counter_w(0, BIT(m_latch, 4));
	m_gift_motor = BIT(m_latch, 5);
	m_motor_out = BIT(m_latch, 6);
}

static INPUT_PORTS_START( futbol )
	PORT_START("IN0") // direct bus lines
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // serial data from the external board
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 10")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bus D2 (plays a sound)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Goal")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bus D5 (hold to play)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bus D7 (playfield sequence)")

	PORT_START("SERIAL") // shifted in through the external board, target 8 first
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 1")
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 2")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 3")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 4")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 5")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 6")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 7")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 8")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Target 9")

	PORT_START("DSW1") // order not verified, read only at power on
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2") // credits or coins, see SW1:3
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Coinage Mode" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Coins per credit" )
	PORT_DIPSETTING(    0x00, "Credits per coin" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW1:5,6") // tick divider, longest first
	PORT_DIPSETTING(    0x30, "1 (longest)" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4 (shortest)" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // not connected, pulled up
INPUT_PORTS_END

static INPUT_PORTS_START( gruacarr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Home Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2 (Q1 while held)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Grab")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Sensor D4 (waited for with Q7 on)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Return Home")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1") // order not verified, read only at power on
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, "Free Game After No Grab" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Button 2 Time" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "0.5 seconds" )
	PORT_DIPSETTING(    0x30, "1 second" )
	PORT_DIPSETTING(    0x28, "1.5 seconds" )
	PORT_DIPSETTING(    0x20, "2 seconds" )
	PORT_DIPSETTING(    0x18, "2.5 seconds" )
	PORT_DIPSETTING(    0x10, "3 seconds" )
	PORT_DIPSETTING(    0x08, "3.5 seconds" )
	PORT_DIPSETTING(    0x00, "4 seconds" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // not connected, pulled up
INPUT_PORTS_END

static INPUT_PORTS_START( gruacarra )
	PORT_INCLUDE( gruacarr )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( irn ) // 'IRN' kiddie ride program
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // only read at the end of a ride, when there are credits left
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1") // order not verified, read only at power on
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "140 seconds" )
	PORT_DIPSETTING(    0x01, "126 seconds" )
	PORT_DIPSETTING(    0x02, "112 seconds" )
	PORT_DIPSETTING(    0x03, "98 seconds" )
	PORT_DIPSETTING(    0x04, "84 seconds" )
	PORT_DIPSETTING(    0x05, "70 seconds" )
	PORT_DIPSETTING(    0x06, "56 seconds" )
	PORT_DIPSETTING(    0x07, "42 seconds" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // not connected, pulled up
INPUT_PORTS_END

static INPUT_PORTS_START( i3 ) // 'I3' kiddie ride program
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Gift Delivered") // push aux, only read while the gift motor runs
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1") // read only at power on
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Ride Time per Credit" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1 minute" )
	PORT_DIPSETTING(    0x04, "2 minutes" )
	PORT_DIPSETTING(    0x08, "2.5 minutes" )
	PORT_DIPSETTING(    0x0c, "3 minutes" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Gift" ) PORT_DIPLOCATION("SW1:6") // also phrase 6 after the attract one
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // not connected, pulled up
INPUT_PORTS_END

static INPUT_PORTS_START( i4 ) // 'I4' kiddie ride program
	PORT_INCLUDE( i3 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop") // only read after a ride, stops the motor

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, "Extra Attract Phrase" ) PORT_DIPLOCATION("SW1:6") // phrase 6 after the attract one
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( m2003_1 ) // no buttons for the child
	PORT_INCLUDE( i4 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( m2003_a )
	PORT_INCLUDE( i4 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Phone")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Horn")
INPUT_PORTS_END

static INPUT_PORTS_START( pokemgkr ) // one button
	PORT_INCLUDE( i4 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void gaelcof3_state::common(machine_config &config)
{
	m_maincpu->write_a().set(FUNC(gaelcof3_state::porta_w));
	m_maincpu->read_b().set(FUNC(gaelcof3_state::portb_r));
	m_maincpu->write_b().set(FUNC(gaelcof3_state::portb_w));

	SPEAKER(config, "mono").front_center();

	// presumably clocked from the PIC CLKOUT pin, so trimmer C11 also sets the pitch; SS pin not verified
	OKIM6295(config, m_oki, PIC_CLOCK / 4, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gaelcof3_state::gaelcof3(machine_config &config)
{
	PIC16C56(config, m_maincpu, PIC_CLOCK);
	common(config);
}

void gaelcof3_state::gaelcof3_c54(machine_config &config)
{
	PIC16C54(config, m_maincpu, PIC_CLOCK);
	common(config);
}


// Pinballs

ROM_START( futbol )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "p4n_pic16c56.bin", 0x0000, 0x2000, CRC(da3530a2) SHA1(d1a99f733901bf66a2025cfb6cf1ffeb30911bb0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pinball_futbol_p4_97e7_p4n_26-6-98_27c020.bin", 0x00000, 0x40000, CRC(448d244b) SHA1(51c3d6309b487d17085aac161016190249e2900b) )
ROM_END

ROM_START( futbola )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "p4n_pic16c56.bin", 0x0000, 0x2000, CRC(da3530a2) SHA1(d1a99f733901bf66a2025cfb6cf1ffeb30911bb0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pinball_futbol_p3_20f6_p4n_21-10-97_27c020.bin", 0x00000, 0x40000, CRC(05a3595d) SHA1(226fd63ea23d06022bbad9eb5a60fe04707a8fca) )
ROM_END

ROM_START( futbolt )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "test_pic16c54a.bin", 0x0000, 0x2000, CRC(ad819aaa) SHA1(f10500e9147c703e24a26b4d48305c16996b7c0a) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "test_futbol_27c010a.bin", 0x00000, 0x20000, CRC(57cf1ca4) SHA1(8d7f027bf7809194035c5b4671919d3b3dce2f1b) )
ROM_END


// Kiddie rides

// Based on the song "El auto feo", composed by Enrique Fischer 'Pipo Pescador'.
ROM_START( autopapa )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "autopapa.u1", 0x00000, 0x40000, CRC(a3e5607e) SHA1(24a9c79edec7b2f7f64b622240f2ad8f3ffa29ca) ) // NEC D27C2001D
ROM_END

// Based on the song "Hola Don Pepito", composed by Ramón del Rivero.
ROM_START( donpepito )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "don_pepito.u1", 0x00000, 0x40000, CRC(574fcd14) SHA1(a23f1eb6d2cef5aa07df3a553fe1d33803648f43) )
ROM_END

// Based on the Spanish cover version of the song "I Like To Move It" by Reel 2 Real, named "Te Gusta el Mueve Mueve".
ROM_START( mueve )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mueve_reclam_ea76_pic_irn_27c020.u1", 0x00000, 0x40000, CRC(f3cc6936) SHA1(35334aeb85f3524f2afdf20f49005d7573ec5494) )
ROM_END

// Based on the song by the Beatles.
ROM_START( obladi )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "obladi_reclam_5a5c_pic_irn_27c020.u1", 0x00000, 0x40000, CRC(a156f749) SHA1(f2bcbe5857e8ea6d96c2abe3051a5d02308dc963) )
ROM_END

// Based on the song composed by Rafael Pérez Botija.
ROM_START( susanita )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "susanita.u1", 0x00000, 0x40000, CRC(766868cb) SHA1(eb42dc46b865bc448052d9d67c840e51c49ce49a) ) // Am27C020
ROM_END


// These Italian sets need different PIC programs (M3 or M4, both undumped)

ROM_START( cochegkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m4_pic16c54c.bin", 0x0000, 0x2000, NO_DUMP ) // Protected

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "coche_freno_italia_-r-_69f0_pic_m4.bin", 0x00000, 0x20000, CRC(c2f26fdd) SHA1(92eed91d5e491b3563dd48933be570fc8a96d78f) ) // sum 6f90, the label says 69f0, probably a typo
ROM_END

ROM_START( kwairi )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m3_pic16c54c.bin", 0x0000, 0x2000, NO_DUMP ) // Protected

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "k_wai_regalo_italia_11-03_m3_a669_27c020.bin", 0x00000, 0x40000, CRC(fad3f35c) SHA1(ecc2b9764bcdddee0f6b479eceb4be9395c4b99a) )
ROM_END

ROM_START( motoxgkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m4_pic16c54c.bin", 0x0000, 0x2000, NO_DUMP ) // Protected

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "freno_moto_italia_cros_7bce_m4_27c010a.bin", 0x00000, 0x20000, CRC(c0988321) SHA1(557839d5f6ee5e3325c5ce943a75138406224eaa) )
ROM_END

ROM_START( motogkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m4_pic16c54c.bin", 0x0000, 0x2000, NO_DUMP ) // Protected

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "motoit-99-e216_m4_27c020.bin", 0x00000, 0x40000, CRC(97f7a2f4) SHA1(002310887b400b98130d2912dd5dbe4a9d63781c) )
ROM_END

ROM_START( motvespa )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m4_pic16c54c.bin", 0x0000, 0x2000, NO_DUMP ) // Protected

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "moto_freno_italia_vespa_cd9b_pic_m4_27c010.bin", 0x00000, 0x20000, CRC(937de303) SHA1(ac47aada63eab433b33a07b50b07689558698968) )
ROM_END


// Italian kiddie rides, different PCB

ROM_START( aladigkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "aladino_27c020.bin", 0x00000, 0x40000, CRC(8714993a) SHA1(097f71d7a7cc238add1d439363df89632f0932e8) ) // phrase 9, the second song, ends at 0x47400, out of the M6295 range
ROM_END

ROM_START( m2003_1 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i4_pic16c54.bin", 0x0000, 0x2000, CRC(a241a001) SHA1(0ea534fa4e8c7740cd1dce79ad0cf13f0b9444f9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "m_2003_1_b5b7_27c2001.bin", 0x00000, 0x40000, CRC(73d0357b) SHA1(e33dc3fe2778983372145b20df7a0857748ff684) )
ROM_END

ROM_START( m2003_a )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i4_pic16c54.bin", 0x0000, 0x2000, CRC(a241a001) SHA1(0ea534fa4e8c7740cd1dce79ad0cf13f0b9444f9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "m_2003_a_5e54.bin", 0x00000, 0x40000, CRC(b7e28e08) SHA1(590bfc716d8d56c0899ff1232cb9314fe44598f3) )
ROM_END

ROM_START( memo0102 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_0102_ef03_27c2001.bin", 0x00000, 0x40000, CRC(1a78b49c) SHA1(6e94b3f84fa7e4fd65cbcbf236f08fc01bf55cff) ) // sum 0xe703, the label says ef03
ROM_END

ROM_START( memo0304 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_0304_a9c7_27c2001.bin", 0x00000, 0x40000, CRC(f8af458c) SHA1(c2e9d36c9cfeeb85b670034d9174a94b97a4afa0) )
ROM_END

ROM_START( memo0506 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_0506_a7f5_27c2001.bin", 0x00000, 0x40000, CRC(cab7e50c) SHA1(04f75ca693ee55dd50fcba531edddea77f466774) )
ROM_END

ROM_START( memo0708 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_0708_af5b_27c2001.bin", 0x00000, 0x40000, CRC(a20c71ba) SHA1(1b239e7eff767bf95660c6ce51791277a7aeeac9) )
ROM_END

ROM_START( memo1011 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_1011_4ec3_27c2001.bin", 0x00000, 0x40000, CRC(86869ecc) SHA1(e5f51ef8018b8a0b19991b2a1721f95001e50cf9) )
ROM_END

ROM_START( memo1213 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_1213_2b64_27c020.bin", 0x00000, 0x40000, CRC(219926d7) SHA1(05c12a3f6858e4e0f00bb44c1d8522e9de8ba5cf) )
ROM_END

ROM_START( memo1415 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_1415_0767_27c020.bin", 0x00000, 0x40000, CRC(50d324ef) SHA1(911937a91979e9bff151ed401e886d1dde51f298) )
ROM_END

ROM_START( memo1617 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i3_pic16c54c.bin", 0x0000, 0x2000, CRC(c1f74d05) SHA1(6c09d4854141ee7731f246db9eb916a1ebecfd2e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "memo_1617_29ed_27c020.bin", 0x00000, 0x40000, CRC(c7a6a65d) SHA1(283296db184724cef879b13513155f0682c3bf3c) )
ROM_END

ROM_START( pokemgkr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i4_pic16c54.bin", 0x0000, 0x2000, CRC(a241a001) SHA1(0ea534fa4e8c7740cd1dce79ad0cf13f0b9444f9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pokemon_puls_d5e1_27c020.bin", 0x00000, 0x40000, CRC(e4af0060) SHA1(4d26c92d4034afdd095d159a0a25b9b1f486d26a) )
ROM_END

ROM_START( pokemgkra )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i4_pic16c54.bin", 0x0000, 0x2000, CRC(a241a001) SHA1(0ea534fa4e8c7740cd1dce79ad0cf13f0b9444f9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pokemon_reclamo_d7ed_am27c020.bin", 0x00000, 0x40000, CRC(f2b07781) SHA1(8e8fb589ad15cd93ac9655be1a05726253f5d9d0) )
ROM_END

ROM_START( pokemgkrb )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.i4_pic16c54.bin", 0x0000, 0x2000, CRC(a241a001) SHA1(0ea534fa4e8c7740cd1dce79ad0cf13f0b9444f9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pokemon_sense_polsador_4225_27c2001.bin", 0x00000, 0x40000, CRC(cb9dc64a) SHA1(28239a674c5eecc0a6737cb36d5e8254a6a81fb2) )
ROM_END


// Cranes

ROM_START( gruacarr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.gr2_pic16c54a.u3", 0x0000, 0x2000, CRC(cfa6f8c0) SHA1(bc72c54ac7e5b9df2e9dfb3581114c76de1b338c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "grua_carrus_7bfd_pic_gr_27c020.u1", 0x00000, 0x40000, CRC(a1322e89) SHA1(4c1995c6cf54acf174de7d9497ef30a69a007964) )
ROM_END

ROM_START( gruacarra )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.gr_pic16c54a.u3", 0x0000, 0x2000, CRC(8ba92d8a) SHA1(a4cb34cbebe49b6a381fd1032aab348a99e6376d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "grua_carrus_7bfd_pic_gr_27c020.u1", 0x00000, 0x40000, CRC(a1322e89) SHA1(4c1995c6cf54acf174de7d9497ef30a69a007964) )
ROM_END


// Foosballs

ROM_START( futbolin )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "futbolin.u3", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "futbolin_4107_pic_27c020.u1", 0x00000, 0x40000, CRC(0b555366) SHA1(ef6e67ada1db0579f9ddd841928fe43165de9638) )
ROM_END


} // anonymous namespace

GAMEL( 1998, futbol,       0, gaelcof3,     futbol, futbol_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Futbol (set 1)",    MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_fut )
GAMEL( 1997, futbola, futbol, gaelcof3,     futbol, futbol_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Futbol (set 2)",    MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_fut )
GAMEL( 1997, futbolt, futbol, gaelcof3_c54, futbol, futbol_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Futbol (test ROM)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_fut )

GAMEL( 199?, autopapa,  0, gaelcof3,     irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", u8"El auto de papá", MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 199?, donpepito, 0, gaelcof3,     irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Don Pepito",        MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 199?, mueve,     0, gaelcof3,     irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Mueve",             MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 199?, obladi,    0, gaelcof3,     irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Ob-La-Di",          MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 199?, susanita,  0, gaelcof3,     irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Cresmatic", "Susanita",          MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )

GAMEL( 200?, cochegkr, 0, gaelcof3_c54, irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco",           "Coche (sudden braking, Italy)",                      MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 2003, kwairi,   0, gaelcof3_c54, irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco / Rumatic", "K Wai Regalo (Italy)",                               MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 200?, motoxgkr, 0, gaelcof3_c54, irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco",           "Moto Cross (Gaelco kiddie Ride, with brake, Italy)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 1999, motogkr,  0, gaelcof3_c54, irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco",           "Moto (Gaelco kiddie Ride, Italy)",                   MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )
GAMEL( 200?, motvespa, 0, gaelcof3_c54, irn, gaelcof3_state, init_rc_wdt, ROT0, "Gaelco",           "Moto Vespa (with brake, Italy)",                     MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid )

GAMEL( 200?, aladigkr, 0, gaelcof3_c54, i3, kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Aladino (Gaelco kiddie ride, Italy)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i ) // Not working because an unreachable sound that causes silent rides

GAMEL( 2003, m2003_1, 0, gaelcof3_c54, m2003_1, kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Març 2003 1 (Nella Vecchia Fattoria / Popoff)", MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 2003, m2003_a, 0, gaelcof3_c54, m2003_a, kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Març 2003 A (Dagli Una Spinta)",                MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )

GAMEL( 200?, memo0102, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Memo 0102 (44 Gatti / Torero Camomillo)",                      MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo0304, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Memo 0304 (Volevo Un Gatto Nero / Il Valzer Del Moscerino)",   MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo0506, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Memo 0506 (Il Caffè Della Peppina / Dagli Una Spinta)",      MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo0708, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Memo 0708 (Il Corsaro Nero / Popoff)",                         MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
// "Memo 0910" missing (Il lungo, il corto e il pacioccone / ?)
GAMEL( 200?, memo1011, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Memo 1011 (Mi Scappa La Pipì, Papà / La Casetta In Canadà)", MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo1213, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Memo 1213 (Nella Vecchia Fattoria / Sandokan)",                MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo1415, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", "Memo 1415 (Attenti Al Lupo / Viva La Pappa Col Pomodoro)",     MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, memo1617, 0, gaelcof3_c54, i3,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Memo 1617 (Dolce Remì / Anna Dei Capelli Rossi)",            MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )

GAMEL( 200?, pokemgkr,  0,        gaelcof3_c54, pokemgkr, kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Pokémon (Gaelco kiddie ride, with button)",        MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, pokemgkra, pokemgkr, gaelcof3_c54, m2003_1,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Pokémon (Gaelco kiddie ride, with attract sound)", MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )
GAMEL( 200?, pokemgkrb, pokemgkr, gaelcof3_c54, m2003_1,  kiddie_i_state, init_rc_wdt, ROT0, "Gaelco", u8"Pokémon (Gaelco kiddie ride, without button)",     MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE, layout_futbol3_kid_i )

GAME( 199?, gruacarr,  0,        gaelcof3_c54, gruacarr,  grua_state, init_rc_wdt, ROT0, "Gaelco", u8"Grúa Carrus (set 1)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME( 199?, gruacarra, gruacarr, gaelcof3_c54, gruacarra, grua_state, init_rc_wdt, ROT0, "Gaelco", u8"Grúa Carrus (set 2)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )

GAME( 1992, futbolin, 0, gaelcof3, futbol, futbol_state, init_rc_wdt, ROT0, "Gaelco / Rumatic", u8"Futbolín Electrónico", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
