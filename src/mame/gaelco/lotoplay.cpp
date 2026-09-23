// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

 Driver for "Loto-Play", a small PCB with a LED roulette installed on
 "First Games" arcade cabs from Covielsa that gives player the option to win a
 free play.

 It's a very simple PCB with this layout:
  ___________________________________
 |   _________               O <- GREEN LED
 |  |_8xDIPS_|            O     O   |
 |  _________________   O         O |
 | | MC68705P3S     |     O     O   |
 | |________________|        O      |
=|=                                 |
=|= <- 8-Pin Connector              |
 |__________________________________|

 Roulette of eight LEDs, seven red and one green (the upper one).
 There are at least two versions, one from 1988 and other from 1990.
 Some units use a M68705P5 or a Z80 instead of a MC68705P3S.

 More info and dip switches:
  - https://www.recreativas.org/loto-play-88-11392-gaelco-sa
  - https://www.recreativas.org/loto-play-90-14731-covielsa

 First Games arcade cab (see the bezel left upper corner with the roulette):
  - https://www.recreativas.org/first-games-954-covielsa

 Version with a PIC16C54 as main CPU:
  _____________________________________________
 |  |_8xDIPS_|   __________            O <- GREEN LED
 |              |SN74LS166N         O     O   |
 |                   Xtal         O         O |
 |                  4 MHz           O     O   |
 |         ___   ___                   O      |
=|=       |  |  |  |<-PIC16C54-XT/P           |
=|= <- 8-Pin Connector     ___                |
 |        |  |<-CNY/74-4  |  |<-TL7702ACP     |
 |        |__|  |__|      |__|                |
 |____________________________________________|



Notes on the MC68705P3S roulette program:

- The LEDs are not wired in numerical order.  The rotation table in the ROM
  (DF BF 7F FE FD FB F7 EF) walks the ring as PB5, PB6, PB7, PB0, PB1, PB2,
  PB3, PB4, so the green LED at the top of the bezel is PB5.  The attract mode
  animations only come out symmetrical about it that way, and a photograph of
  the board agrees: it silkscreens the ring L1 to L8 clockwise from nine
  o'clock with the green one at L3 on top, which comes out as Ln driven by
  PB((n + 2) & 7), a constant offset with nothing shuffled.

- The coin inputs are debounced in software by sampling them once per main
  loop, so a pulse has to stay low for two loops after having been high for
  six.

- PC0 emits one pulse per credit, so the board sits between the coin mechanism
  and the game PCB and repeats credits through it.  Winning the roulette just
  adds more credits to the pending counter.

- A spin is 48 steps through a deceleration ramp.  48 is a multiple of 8, so it
  always ends on the LED it started on, and the starting LED is what decides
  the prize.  Starting on the green one is only allowed once a 16 bit
  accumulator has carried into its high byte, and the per play increment is 5,
  10, 18 or 25 out of 256 depending on SW7 and SW8.

- The Loto-Play 90 manual on recreativas.org documents SW1-SW4 as the coin rate
  table, SW5-SW6 as unused and SW7-SW8 as the lottery percentage.  It describes
  lotoplayp rather than these boards, but the program here splits the switches
  the same way, so the numbering is taken to carry over: SW7 is PA0 and SW8 is
  PA1, since the percentages climb in the same order as the ROM's table, and
  SW1 to SW4 are PA4 to PA7 with SW1 least significant, since the manual
  enumerates its rows as a binary count of SW4/SW3/SW2/SW1.  That leaves PA2
  and PA3 for the two the manual calls unused.  Which of them is SW5 and which
  SW6 cannot be settled from what survives: it is a silkscreen fact, the ROM
  only ever sees bit positions, and the manual has nothing to say about
  switches that do nothing on its own board.  In both groups that are pinned
  down the lower switch number sits on the lower bit, so PA2 is taken as SW5
  and PA3 as SW6.  A straight reversed routing would land on the same three
  groups with each one flipped and swap those two, and nothing here rules it
  out.  All switches on selects entry 0 in both tables, which confirms that a
  closed switch reads back as 0.

- The percentages nearly match: the manual gives 2%, 5%, 7% and 10% against the
  1.95%, 3.91%, 7.03% and 9.77% the program uses, so the settings are labelled
  with what this program pays rather than with the manual's figures.  The coin
  rate table does not match at all, and SW5 and SW6 do have an effect here
  despite the manual calling them unused, because the manual is not for these
  boards: its table is, row for row, the one in lotoplayp.

- None of these boards has a crystal, and a photograph of the MC68705P3S one
  shows none fitted.  Bit 7 of the mask option register is set in all four
  dumps, which picks the RC oscillator, so the frequency comes from an external
  resistor and is not going to be a round crystal value.  lotoplayc gives a way
  to put a number on it: its timer interrupt divides by a hundred and then by
  sixty, which only reads as hundredths, seconds and minutes, and the prescaler
  its own mask option register asks for puts the oscillator at 3.2 MHz for that
  to come out right.  The roulette program has no equivalent anchor, so it is
  given the same figure; at that speed a spin lasts about two seconds, the main
  loop runs at 196 Hz and the click is around 780 Hz.

- lotoplayb is the odd one of the three.  Its timer vector points at the reset
  entry, it writes 0x4c to the timer control register and leaves it there with
  the interrupt masked, and there is not a single RTI or CLI in the program: it
  times itself by spinning on the timer flag at the bottom of the main loop
  instead.  The tone on the other two is made in the timer interrupt, so this
  one cannot make it, and sure enough PC1 carries a second pulse output built
  from the same state machine as PC0, thirteen loops low and thirty-two high
  against PC0's seven and thirty-eight.  Two credit lines out, no speaker.
  Its deceleration ramp is longer as well.  The other two step sixteen times
  every four main loops and work down from there, forty-eight steps in all;
  lotoplayb prepends sixteen steps every two loops, so a spin is sixty-four.
  Still a multiple of eight, so it still lands where it started.  Everything
  else, the coin rate table, the percentages, the LED order and the attract
  animation, is the same in all three.

Notes on the MC68705P3S 7-segments display program:

- lotoplayc runs an unrelated program and is wired differently.  There is no
  roulette on it: PORTB bits 0 to 6 carry a common anode seven segment font and
  what gets displayed is a credit counter clamped to ten.  Four DIP switches
  are read one at a time by driving a mux address on PA0-PA2 and sampling PA3,
  and the whole of PORTC is inputs.  Its two coin rate tables give one coin per
  four, three, two or one pulses on the first input, and three, two, five or
  four credits per pulse on the second.

- The routine that refreshes the digit keeps PB7 with AND #$80 and then uses
  ADD rather than ORA, so bit 7 of a table entry doubles as an instruction to
  pull PB7 low, the carry out of the addition being thrown away.  PB7 is high
  whenever the routine runs, and the entry for nine is the only one with that
  bit set, so reaching nine credits updates the digit and drops PB7 in a single
  lookup.  What it leaves on the display is segments a to e rather than a nine,
  which would make sense if PB7 were part of the display as well, but there is
  no photograph of this board to check that against.

- It is marked not working for two reasons.  PA6, PA7 and PB7 pulse but are
  not wired to anything here, so nothing shows when the board hands something
  out, and the loop that steps the PA counter releases PB7 and waits for it to
  read high before moving on, which with nothing driving the pin happens on the
  first pass, so whatever is on the far end of that handshake is neither
  emulated nor known.  On top of that no photograph of this board has turned
  up: the single digit is what the font and the clamp imply rather than
  something anyone has seen, and the entry for nine hints there is more to the
  display than seven segments.

- Every pin is accounted for and none of them carries sound.  PA0-PA1 and
  PA4-PA5 hold a four bit value strobed out on PA2, PA6 and PA7 emit single
  pulses ten to twenty timer ticks wide, and the timer interrupt only keeps
  time, dividing by a hundred and then by sixty.  PB7 is turned around and
  sampled in a loop that keeps stepping those PA counters until it reads high,
  so whatever they drive answers back on it.  There is nothing anywhere that
  toggles a pin at an audio rate, unlike the roulette program, which swings PC1
  in its interrupt handler.

Notes on the PIC16C54 roulette program:

- lotoplayp reaches the same eight LEDs through a PIC16C54.  TRISB is zero, so
  all of PORTB drives them, and this time the animations are symmetrical about
  RB0, so the green LED is that pin rather than the fifth one.  TRISA leaves
  only RA2 as an input, and it is the serial output of the SN74LS166: the
  program strobes SH/LD on RA0 and the clock on RA1 to read the coin lines and
  the switches through it, and RA3 pulses low once per credit.

- RA0 is the speaker as well.  The routine that advances the ring rotates PORTB
  end around and then clears a flag that lets the main loop swing RA0 four
  times, which is the click on each step, and during a win it flips the green
  LED on RB0 and gates the tone to match so the board beeps in step with the
  flashing.  The rest of the time RA0 sits high.  That is what the save and
  restore of PORTA around the coin poll is protecting, and it agrees with the
  manual, which lists a speaker on this model.

- Sharing the pin costs a four microsecond dip every time the shift register is
  loaded, which the poll does every 201 main loops, so about 78 times a second.
  That dip is on the real speaker wire too, but at a hundredth of a per cent
  duty its energy is spread flat out to a quarter of a megahertz and no
  transducer radiates any of it, whereas a level driven square wave generator
  puts the lot into the audio band and it comes out as a buzz the board does
  not make.  Filtering it out would mean adding a component that is not on the
  board, so it is left alone and written down here instead.

- Of the shift register's eight parallel inputs, P7 and P6 are the coin lines,
  P5 to P2 are four switches, P1 is unused and P0 is a fifth.  The sixth switch
  does not fit and goes to the serial input, which is the only wiring that
  accounts for all four documented percentages: with that pin tied either way
  only two of them could ever be selected.  Photographs of both faces do not
  settle that one, because the switch tracks cross to the component side on the
  way to the shift register, but they do show all eight switches commoned on
  one side and pulled up by RR1 on the other.  The program never clears the
  watchdog, and the config byte that survived in the dump has WDTE clear, so it
  is off.

- Its ring is numbered D1 to D8 clockwise with the green one at twelve o'clock,
  so with the ROM putting the green LED on RB0 the LEDs come out as Dn driven
  by RB(n - 1).

*******************************************************************************/

#include "emu.h"

#include "cpu/m6805/m68705.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/74166.h"
#include "sound/spkrdev.h"

#include "speaker.h"

#include "lotoplay_7s.lh"
#include "lotoplay_ro.lh"


namespace {

class lotoplay_ro_state : public driver_device
{
public:
	lotoplay_ro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void lotoplay_ro(machine_config &config) ATTR_COLD;
	void lotoplay_ro_2cred(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void portb_w(u8 data)
	{
		// PB5 is the LED at the top of the bezel, PB6 the next one clockwise
		for (unsigned i = 0; i < 8; i++)
			m_leds[(i + 3) & 7] = BIT(~data, i);
	}

	// there is nothing on the bezel to show the credits going out
	void credit_w(unsigned n, int state)
	{
		if (m_credit_line[n] && !state)
			++m_credit_count[n];
		m_credit_line[n] = state;
	}

	void portc_w(u8 data)
	{
		u32 const before = m_credit_count[0];
		credit_w(0, BIT(data, 0));
		if (m_credit_count[0] != before)
			popmessage("Credits out: %u", m_credit_count[0]);

		m_speaker->level_w(BIT(data, 1));
	}

	void portc_2cred_w(u8 data)
	{
		u32 const before = m_credit_count[0] + m_credit_count[1];
		credit_w(0, BIT(data, 0));
		credit_w(1, BIT(data, 1));
		if (m_credit_count[0] + m_credit_count[1] != before)
			popmessage("Credits out: %u / %u", m_credit_count[0], m_credit_count[1]);
	}

	required_device<m68705p3_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	output_finder<8> m_leds;

	bool m_credit_line[2] = { false, false };
	u32 m_credit_count[2] = { 0, 0 };
};

void lotoplay_ro_state::machine_start()
{
	save_item(NAME(m_credit_line));
	save_item(NAME(m_credit_count));
}

void lotoplay_ro_state::lotoplay_ro(machine_config &config)
{
	M68705P3(config, m_maincpu, 3'200'000); // MC68705P3S, RC oscillator
	m_maincpu->porta_r().set_ioport("DSW");
	m_maincpu->portb_w().set(FUNC(lotoplay_ro_state::portb_w));
	m_maincpu->portc_r().set_ioport("IN");
	m_maincpu->portc_w().set(FUNC(lotoplay_ro_state::portc_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.35);
}

void lotoplay_ro_state::lotoplay_ro_2cred(machine_config &config)
{
	M68705P3(config, m_maincpu, 3'200'000); // RC oscillator
	m_maincpu->porta_r().set_ioport("DSW");
	m_maincpu->portb_w().set(FUNC(lotoplay_ro_state::portb_w));
	m_maincpu->portc_r().set_ioport("IN");
	m_maincpu->portc_w().set(FUNC(lotoplay_ro_state::portc_2cred_w));
}


class lotoplay_7s_state : public driver_device
{
public:
	lotoplay_7s_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsw(*this, "DSW")
		, m_digit(*this, "digit0")
	{
	}

	void lotoplay_7s(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 porta_r() { return BIT(m_dsw->read(), m_mux) << 3; }
	void porta_w(u8 data) { m_mux = data & 0x07; }
	void portb_w(u8 data) { m_digit = ~data & 0x7f; }

	required_device<m68705p3_device> m_maincpu;
	required_ioport m_dsw;
	output_finder<> m_digit;

	u8 m_mux = 0;
};

void lotoplay_7s_state::machine_start()
{
	save_item(NAME(m_mux));
}

void lotoplay_7s_state::lotoplay_7s(machine_config &config)
{
	M68705P3(config, m_maincpu, 3'200'000); // RC oscillator
	m_maincpu->porta_r().set(FUNC(lotoplay_7s_state::porta_r));
	m_maincpu->porta_w().set(FUNC(lotoplay_7s_state::porta_w));
	m_maincpu->portb_w().set(FUNC(lotoplay_7s_state::portb_w));
	m_maincpu->portc_r().set_ioport("IN");
}


class lotoplay_ro_pic_state : public driver_device
{
public:
	lotoplay_ro_pic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_shifter(*this, "shifter")
		, m_speaker(*this, "speaker")
		, m_dsw(*this, "DSW")
		, m_in(*this, "IN")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void lotoplay_ro_pic(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 shifter_r() { return (m_in->read() & 0xc0) | (m_dsw->read() & 0x3d) | 0x02; }
	void qh_w(int state) { m_qh = state; }

	u8 porta_r() { return m_qh << 2; }

	void porta_w(u8 data)
	{
		m_shifter->serial_w(BIT(m_dsw->read(), 6));
		m_shifter->shift_load_w(BIT(data, 0));
		m_shifter->clock_w(BIT(data, 1));

		if (m_credit_line && !BIT(data, 3))
			popmessage("Credits out: %u", ++m_credit_count);
		m_credit_line = BIT(data, 3);

		m_speaker->level_w(BIT(data, 0));
	}

	void portb_w(u8 data)
	{
		for (unsigned i = 0; i < 8; i++)
			m_leds[i] = BIT(~data, i);
	}

	required_device<pic16c54_device> m_maincpu;
	required_device<ttl166_device> m_shifter;
	required_device<speaker_sound_device> m_speaker;
	required_ioport m_dsw;
	required_ioport m_in;
	output_finder<8> m_leds;

	int m_qh = 0;
	bool m_credit_line = false;
	u32 m_credit_count = 0;
};

void lotoplay_ro_pic_state::machine_start()
{
	save_item(NAME(m_qh));
	save_item(NAME(m_credit_line));
	save_item(NAME(m_credit_count));
}

void lotoplay_ro_pic_state::lotoplay_ro_pic(machine_config &config)
{
	PIC16C54(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->read_a().set(FUNC(lotoplay_ro_pic_state::porta_r));
	m_maincpu->write_a().set(FUNC(lotoplay_ro_pic_state::porta_w));
	m_maincpu->write_b().set(FUNC(lotoplay_ro_pic_state::portb_w));

	TTL166(config, m_shifter);
	m_shifter->data_callback().set(FUNC(lotoplay_ro_pic_state::shifter_r));
	m_shifter->qh_callback().set(FUNC(lotoplay_ro_pic_state::qh_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.35);
}


/*
    The switches short their pin to ground and PORTA has internal pull-ups, so
    a switch that is on reads back as 0.

    One switch bank sets both coin slots from a coupled table, so the settings
    below are combined strings.  SW5 and SW6 modify whatever the table produces
    rather than standing on their own.
*/
INPUT_PORTS_START(lotoplay_ro)
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Lottery Percentage" )     PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2%" )
	PORT_DIPSETTING(    0x01, "4%" )
	PORT_DIPSETTING(    0x02, "7%" )
	PORT_DIPSETTING(    0x03, "10%" )
	PORT_DIPNAME( 0x04, 0x00, "Double Credit Values" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Extra Credit On Coin A" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "Coin A 1C/1C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x10, "Coin A 1C/2C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x20, "Coin A 1C/3C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x30, "Coin A 1C/4C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x40, "Coin A 1C/5C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x50, "Coin A 1C/6C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x60, "Coin A 1C/7C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x70, "Coin A 1C/8C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x80, "Coin A 1C/2C, Coin B 2C/1C" )
	PORT_DIPSETTING(    0x90, "Coin A 1C/1C, Coin B 2C/1C" )
	PORT_DIPSETTING(    0xa0, "Coin A 1C/1C, Coin B 3C/1C" )
	PORT_DIPSETTING(    0xb0, "Coin A 1C/1C, Coin B 4C/1C" )
	PORT_DIPSETTING(    0xc0, "Coin A 1C/1C, Coin B 5C/1C" )
	PORT_DIPSETTING(    0xd0, "Coin A 1C/1C, Coin B 6C/1C" )
	PORT_DIPSETTING(    0xe0, "Coin A 1C/1C, Coin B 7C/1C" )
	PORT_DIPSETTING(    0xf0, "Coin A 1C/1C, Coin B 8C/1C" )

	PORT_START("IN")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(lotoplay_7s)
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Play A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Play B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Coin A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Coin B")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*
    The eight parallel inputs of the shift register carry the two coin lines on
    P7 and P6 and four of the switches on P5 to P2, P1 is not connected, and P0
    carries SW7.  SW8 is the one that does not fit, and goes to the serial
    input: with that pin tied either way only two of the four documented
    percentages would be reachable.
*/
INPUT_PORTS_START(lotoplay_ro_pic)
	PORT_START("DSW")
	PORT_DIPNAME( 0x41, 0x41, "Lottery Percentage" )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2%" )
	PORT_DIPSETTING(    0x01, "4%" )
	PORT_DIPSETTING(    0x40, "7%" )
	PORT_DIPSETTING(    0x41, "10%" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x3c, 0x3c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x00, "Coin A 1C/1C, Coin B 1C/2C" )
	PORT_DIPSETTING(    0x20, "Coin A 1C/1C, Coin B 1C/3C" )
	PORT_DIPSETTING(    0x10, "Coin A 1C/1C, Coin B 1C/4C" )
	PORT_DIPSETTING(    0x30, "Coin A 1C/1C, Coin B 1C/5C" )
	PORT_DIPSETTING(    0x08, "Coin A 1C/2C, Coin B 1C/2C" )
	PORT_DIPSETTING(    0x28, "Coin A 1C/2C, Coin B 1C/3C" )
	PORT_DIPSETTING(    0x18, "Coin A 1C/2C, Coin B 1C/4C" )
	PORT_DIPSETTING(    0x38, "Coin A 1C/2C, Coin B 1C/5C" )
	PORT_DIPSETTING(    0x04, "Coin A 2C/1C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x24, "Coin A 2C/1C, Coin B 1C/2C" )
	PORT_DIPSETTING(    0x14, "Coin A 2C/1C, Coin B 1C/3C" )
	PORT_DIPSETTING(    0x34, "Coin A 2C/1C, Coin B 1C/4C" )
	PORT_DIPSETTING(    0x0c, "Coin A 3C/1C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x2c, "Coin A 3C/1C, Coin B 1C/2C" )
	PORT_DIPSETTING(    0x1c, "Coin A 4C/1C, Coin B 1C/1C" )
	PORT_DIPSETTING(    0x3c, "Coin A 4C/1C, Coin B 1C/2C" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")
INPUT_PORTS_END


// Sets with MC68705.

ROM_START(lotoplay)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_sp_ultima_68705p3s.bin", 0x0000, 0x0800, CRC(112645cd) SHA1(f2ad6b2fbec36d0bfe034d7bfb036ef6bf4ee395))
ROM_END

ROM_START(lotoplaya)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_mostra_s_125d9_68705p3s.bin", 0x0000, 0x0800, CRC(9b77603c) SHA1(6799b930f9805332bf20c6146b044222fe49d243))
ROM_END

ROM_START(lotoplayb)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("lp_vii_sch_mostra_11302_68705p3s.bin", 0x0000, 0x0800, CRC(61b426d3) SHA1(b66dc6c382a04d8cdbaee342f179ce80abfd3c71))
ROM_END

// Different PCB than the previous sets, with MC68705 and a seven segment display instead of a roulette.
ROM_START(lotoplayc)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("multn.bin", 0x0000, 0x0800, CRC(20a0e0d0) SHA1(832ed64dfa5f5f150f0e9918b40e9fb4e8e4260d))
ROM_END


// Sets with PIC16C54.

ROM_START(lotoplayp)
	ROM_REGION(0x1fff, "maincpu", 0)
	ROM_LOAD("loto_play_ff46_pic16c54.bin", 0x0000, 0x1fff, CRC(8840349d) SHA1(e9dcc572c7b577618ddda06be1538be69eb15584))
ROM_END

} // anonymous namespace


//     YEAR   NAME       PARENT    MACHINE            INPUT            CLASS                  INIT        ROT   COMPANY              FULLNAME                                                  FLAGS                                        LAYOUT
GAMEL( 1988?, lotoplay,  0,        lotoplay_ro,       lotoplay_ro,     lotoplay_ro_state,     empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 1, with roulette)",              MACHINE_SUPPORTS_SAVE,                       layout_lotoplay_ro )
GAMEL( 1988?, lotoplaya, lotoplay, lotoplay_ro,       lotoplay_ro,     lotoplay_ro_state,     empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 2, with roulette)",              MACHINE_SUPPORTS_SAVE,                       layout_lotoplay_ro )
GAMEL( 1988?, lotoplayb, lotoplay, lotoplay_ro_2cred, lotoplay_ro,     lotoplay_ro_state,     empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 3, with roulette)",              MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE, layout_lotoplay_ro )
GAMEL( 1988?, lotoplayc, lotoplay, lotoplay_7s,       lotoplay_7s,     lotoplay_7s_state,     empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (MC68705, set 4, with seven-segment display)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING,   layout_lotoplay_7s )
GAMEL( 1990,  lotoplayp, lotoplay, lotoplay_ro_pic,   lotoplay_ro_pic, lotoplay_ro_pic_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play (PIC16C54, with roulette)",                    MACHINE_SUPPORTS_SAVE,                       layout_lotoplay_ro )
