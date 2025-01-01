// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

PINBALL
Williams System 8
These are not true pinballs in the normal sense, but are unusual novelty machines.
  Unfortunately they were mostly cancelled before production could begin.

Differences to system 7:
- PIA at 0x2200 removed
- Speech circuits removed
- Dip switches removed
- Crystals changed from 3.58 to 4MHz

Games:
- Pennant Fever (#526)
- Rat Race (#527) (10 produced)
- Rat Race II (#533)
- Gridiron (#538)
- Still Crazy (#543)
- Break Street
- Arena

The first time run, the display will show the model number. Press F3 to clear this.


PENNANT FEVER
=============
This is a baseball game where you aim for targets at the top of the playfield, and the
  players advance towards a home run. There are no bumpers or other 'usual' pinball
  items. 1 or 2 players.
  Buttons: Pitch (Fast, Curve, Change Up), Bat.
  How to play:
  - Insert coin (credits shows in innings)
  - Start game
  - Player 1 is 'Visitors'; optional Player 2 is 'Home'
  - To hit the ball and move to another base: Hit Y or U or I or O
  - To get a home run: Hit B or N or M
  - To miss (get a strike): Hit Alt or Space or LShift; then hit Z
  - To get out immediately: Hit R or T.
  - After that, wait for score to start flashing
  - Press another key, etc
  - When you have 3 Strikes, you are Out
  - When you have 3 Outs, your Innings ends (other player gets a turn)
  - After 3 Innings, it's game over.
  - Match digit appears in Outs digit.


GRIDIRON
========
A conversion kit for Pennant Fever. Didn't get past the prototype stage. The display shows
  Player 1 and 2 (3 digits each), Possessions and Downs (2 digits each).


STILL CRAZY
===========
IMDB shows the number as 534, however both the game and the manual say 543. The
  undumped System 10 game 4-in-1 (Pigskin/Poker/Willy at the Bat/Willy's Cup) also
  was assigned number 543.

A novelty game where the playfield is completely vertical. It has 4 flippers and the
  idea is to get the ball up to the alcohol 'still' before the 'revenuers' do. The
  idea didn't catch on, and the game was not officially released. 1 player.
  The display shows Score and Batch. There is no credit display.
  If the number of batches exceeds 9, the 'hidden digit' will show the tens.
  You cannot get more than 99 batches.
  The score only has 5 digits, but the game stores the 100,000 digit internally.

How to play: Press 5, press 1, press Z. Add points by pressing F,G,H,J. Press J
  a number of times to increment the batches. To end the game, hit X until the siren
  is heard. If you scored > 99999 points, the high score will show 99999.


BREAK STREET
============
Another failed novelty, not much is known about it. Seems it features a break-dancing
  toy and a spinning disk.


ARENA
=====
A one-off prototype table model, similar idea to the system-7 Defender.


RAT RACE
========
Rat Race is played in a cocktail cabinet, the player uses a joystick to tilt the
  board, to coax the ball into following lit passages in a maze. After a successful
  navigation, the maze changes to something else faster and harder. It's almost an
  arcade game done mechanically. Obviously there is no way to emulate it in its intended
  form. Probably would have been a nice game, but it never passed the prototype stage.
  Bad byte at "maincpu" D7FF, although it doesn't seem to cause an issue.

How to play: Press 5, press 1. If you want 2 players, also press 2. The display flashes
  the ball number. Press Z to get the ball into play. The display alternates between the
  score and the seconds remaining to accomplish the task. After the series of passages
  has been followed, a bonus flashes for a few seconds, which you must also run over.
  While one player is playing, the other player's score shows what seem to be random
  numbers (but probably are not). The real machine does this too.
  If time runs out, the ball is over. After 3 balls, it's game over.

Status:
- Playable

ToDo:
- Nothing
- Rat Race: need a manual, playboard switches are unknown.

************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "williamssound.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "speaker.h"

#include "s8pfevr.lh"
#include "s8ratrc.lh"
#include "s8scrzy.lh"


namespace {

class s8_state : public genpin_class
{
public:
	s8_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pias(*this, "pias")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_s9sound(*this, "s9sound")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s8(machine_config &config);
	void pfevr(machine_config &config);
	void ratrc(machine_config &config);
	void scrzy(machine_config &config);
	void psound(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer);

private:
	u8 sound_r();
	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void ratrc_dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol2_w(u8 data);
	void sol3_w(u8 data); // solenoids 0-7
	void scrzy_sol3_w(u8 data); // solenoids 0-7
	void sound_w(u8 data);
	u8 switch_r();
	void switch_w(u8 data);
	void pia21_ca2_w(int state);
	void pia21_cb2_w(int state) { } // enable solenoids
	void pia24_cb2_w(int state) { m_io_outputs[16] = state; } // dummy to stop error log filling up
	void pia28_ca2_w(int state) { m_comma34 = state; } // comma3&4
	void pia28_cb2_w(int state) { m_comma12 = state; } // comma1&2
	void pia_irq(int state);

	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void scrzy_map(address_map &map) ATTR_COLD;

	u8 m_sound_data = 0U;
	u8 m_strobe = 0U;
	u8 m_row = 0U;
	bool m_comma12 = false;
	bool m_comma34 = false;
	bool m_data_ok = false;
	u8 m_lamp_data = 0U;
	emu_timer* m_irq_timer = nullptr;

	required_device<m6802_cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	optional_device<williams_s9_sound_device> m_s9sound;
	required_ioport_array<8> m_io_keyboard;
	output_finder<61> m_digits;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps
};

void s8_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s8_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x4000, 0x7fff).rom().region("maincpu", 0);
}

void s8_state::scrzy_map(address_map &map)
{
	main_map(map);
	map.global_mask(0x7fff);
	map(0x2200, 0x2200).w(FUNC(s8_state::scrzy_sol3_w)); // solenoids
}

void s8_state::audio_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x4000, 0x4003).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xffff).rom().region("audiocpu", 0);
}

static INPUT_PORTS_START( pfevr )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Plumb Tilt") // 3 touches before it tilts
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Strike Drain") // INP09
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bat Swing") // INP10
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Pitch Fast") // INP11
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pitch Curve") // INP12
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pitch Change-up") // INP13
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outfield Drain") // INP15

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Left Single") // INP17
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Left Out") // INP18
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Left Double") // INP19
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Center Triple") // INP20
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Right Double") // INP21
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Right Out") // INP22
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Right Single") // INP23
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Left Home Run") // INP24

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Center Home Run") // INP25
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Right Home Run") // INP26
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Cam Inside") // INP28
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Cam Outside") // INP29

	PORT_START("X4")
	PORT_START("X5")
	PORT_START("X6")
	PORT_START("X7")

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s8_state::audio_nmi), 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s8_state::main_nmi), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( scrzy )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Top L Flip") // INP04
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Bot L Flip") // INP05
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Top R Flip") // INP06
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Bot R Flip") // INP07
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Low L Drain") // INP08

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Lower Ramp Limit") // INP09 game mechanics are ready to start
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Top Drain") // INP10
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Middle R Drain") //INP11
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Level 1") // INP12
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Level 2") // INP13
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Level 3") // INP14
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Level 4") // INP15
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Upper Ramp Limit") // INP16 revenuers have reached the still

	PORT_START("X2")
	PORT_START("X3")
	PORT_START("X4")
	PORT_START("X5")
	PORT_START("X6")
	PORT_START("X7")

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s8_state::main_nmi), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( ratrc )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) // 2nd player start
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("High Score Reset")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP52")

	PORT_START("X7")
	PORT_BIT( 0x11, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP57") // Ball entering play (bit 0 = P1, bit 4 = P2)

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s8_state::main_nmi), 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s8_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s8_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s8_state::sol3_w(u8 data)
{
	if (BIT(data, 1))
		m_samples->start(0, 6); // knocker

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void s8_state::scrzy_sol3_w(u8 data)
{
	if (data==0x0a)
		m_samples->start(0, 7); // mechanical drum when you have 2 or more batches

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void s8_state::sound_w(u8 data)
{
	m_sound_data = data;
}

void s8_state::pia21_ca2_w(int state)
{
// sound ns
	if (m_pias)
		m_pias->ca1_w(state);
}

void s8_state::sol2_w(u8 data)
{
	m_comma12 = BIT(data, 7);
	m_comma34 = BIT(data, 6);

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);
}

void s8_state::lamp0_w(u8 data)
{
	m_lamp_data = data ^ 0xff;
}

void s8_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);
}

void s8_state::dig0_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

void s8_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

void s8_state::ratrc_dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0xdb,0xcf,0xe6,0xed,0xfd,0x07,0xff,0xef,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data & 15] | (m_comma34 ? 0xc000 : 0);
		m_digits[m_strobe] = patterns[data >> 4] | (m_comma12 ? 0xc000 : 0);
	}
	m_data_ok = false;
}

u8 s8_state::switch_r()
{
	u8 data = 0;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s8_state::switch_w(u8 data)
{
	m_row = data;
}

u8 s8_state::sound_r()
{
	return m_sound_data;
}

void s8_state::pia_irq(int state)
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(s8_state::irq_timer)
{
	if(param == 1)
	{
		m_maincpu->set_input_line(M6802_IRQ_LINE, ASSERT_LINE);
		m_irq_timer->adjust(attotime::from_ticks(32,1e6),0);
		m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
		m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
	}
	else
	{
		m_maincpu->set_input_line(M6802_IRQ_LINE, CLEAR_LINE);
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
		m_pia28->ca1_w(1);
		m_pia28->cb1_w(1);
	}
}

void s8_state::machine_start()
{
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));

	if (m_pias)
		save_item(NAME(m_sound_data));

	save_item(NAME(m_comma12));
	save_item(NAME(m_comma34));

	m_irq_timer = timer_alloc(FUNC(s8_state::irq_timer), this);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s8_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

// Pennant Fever sound card has PIA at a custom address
void s8_state::psound(machine_config &config)
{
	M6808(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &s8_state::audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	PIA6821(config, m_pias);
	m_pias->readpa_handler().set(FUNC(s8_state::sound_r));
	m_pias->set_port_a_input_overrides_output_mask(0xff);
	m_pias->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
	m_pias->irqb_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
}

void s8_state::s8(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &s8_state::main_map);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21);
	m_pia21->readpa_handler().set(FUNC(s8_state::sound_r));
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->ca1_w(1); // sound busy
	m_pia21->writepa_handler().set(FUNC(s8_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s8_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s8_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s8_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia24);
	m_pia24->writepa_handler().set(FUNC(s8_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s8_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s8_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia28);
	m_pia28->writepa_handler().set(FUNC(s8_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s8_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s8_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s8_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia30);
	m_pia30->readpa_handler().set(FUNC(s8_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s8_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s8_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void s8_state::pfevr(machine_config &config)
{
	s8(config);

	/* Video */
	config.set_default_layout(layout_s8pfevr);

	/* Add the soundcard */
	psound(config);
}

void s8_state::scrzy(machine_config &config)
{
	s8(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &s8_state::scrzy_map);

	/* Video */
	config.set_default_layout(layout_s8scrzy);

	m_pia21->writepa_handler().set("s9sound", FUNC(williams_s9_sound_device::write));
	m_pia21->ca2_handler().set("s9sound", FUNC(williams_s9_sound_device::strobe));

	/* Add the soundcard */
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S9_SOUND(config, m_s9sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void s8_state::ratrc(machine_config &config)
{
	scrzy(config);
	m_pia28->writepb_handler().set(FUNC(s8_state::ratrc_dig1_w));

	/* Video */
	config.set_default_layout(layout_s8ratrc);
}


/*------------------------------
/ Pennant Fever (#526) 05/1984
/-------------------------------*/
ROM_START(pfevr_l2)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("pf-rom1.u19", 0x1000, 0x1000, CRC(00be42bd) SHA1(72ca21c96e3ffa3c43499165f3339b669c8e94a5))
	ROM_LOAD("pf-rom2.u20", 0x2000, 0x2000, CRC(7b101534) SHA1(21e886d5872104d71bb528b9affb12230268597a))

	ROM_REGION(0x4000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u49.128", 0x0000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END

ROM_START(pfevr_p3)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u19.732", 0x1000, 0x1000, CRC(03796c6d) SHA1(38c95fcce9d0f357a74f041f0df006b9c6f6efc7))
	ROM_LOAD("cpu_u20.764", 0x2000, 0x2000, CRC(3a3acb39) SHA1(7844cc30a9486f718a556850fc9cef3be82f26b7))

	ROM_REGION(0x4000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("cpu_u49.128", 0x0000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END

/*-----------------------------------------------------------------------------
/ Rat Race : (Game #527)- Prototype (displays as #500L1)
/-----------------------------------------------------------------------------*/
ROM_START(ratrc_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.532", 0x1000, 0x1000, CRC(0c5c7c09) SHA1(c93b39ba1460feee5850fcd3ca7cacb72c4c8ff3))
	ROM_LOAD("ic14.532", 0x2000, 0x1000, CRC(c6f4bcf4) SHA1(d71c86299139abe3dd376a324315a039be82875c))
	ROM_LOAD("ic17.532", 0x3000, 0x1000, CRC(0800c214) SHA1(3343c07fd550bb0759032628e01bb750135dab15))

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("b486.bin", 0x6000, 0x2000, CRC(c54b9402) SHA1(c56fc5f105fc2c1166e3b22bb09b72af79e0aec1))
ROM_END

/*-----------------------------
/ Rat Race II (Game #533)
/ - never produced
/-----------------------------*/

/*----------------------------
/ Still Crazy (#543) 06/1984
/-----------------------------*/
ROM_START(scrzy_l1)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.bin", 0x2000, 0x2000, CRC(b0df42e6) SHA1(bb10268d7b820d1de0c20e1b79aba558badd072b) )

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	// 1st and 2nd halves are identical
	ROM_LOAD("ic49.bin", 0x4000, 0x4000, CRC(bcc8ccc4) SHA1(2312f9cc4f5a2dadfbfa61d13c31bb5838adf152) )
ROM_END

} // Anonymous namespace


GAME(1984, pfevr_l2, 0,        pfevr, pfevr, s8_state, empty_init, ROT0, "Williams", "Pennant Fever (L-2)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984, pfevr_p3, pfevr_l2, pfevr, pfevr, s8_state, empty_init, ROT0, "Williams", "Pennant Fever (P-3)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1983, ratrc_l1, 0,        ratrc, ratrc, s8_state, empty_init, ROT0, "Williams", "Rat Race (L-1)",      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1984, scrzy_l1, 0,        scrzy, scrzy, s8_state, empty_init, ROT0, "Williams", "Still Crazy",         MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
