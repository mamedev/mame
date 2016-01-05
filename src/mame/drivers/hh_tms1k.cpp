// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle, Kevin Horton
/***************************************************************************

  This driver is a collection of simple dedicated handheld and tabletop
  toys based around the TMS1000 MCU series. Anything more complex or clearly
  part of a series is (or will be) in its own driver.

  Let's use this driver for a list of known devices and their serials,
  excluding TI's own products (see ticalc1x.cpp, tispeak.cpp, tispellb.cpp)

  serial   device    etc.
--------------------------------------------------------------------
 @CP0904A  TMS0970   1977, Milton Bradley Comp IV
 @MP0905B  TMS0970   1977, Parker Brothers Codename Sector
 *MP0168   TMS1000?  1979, Conic Basketball
 @MP0914   TMS1000   1979, Entex Baseball 1
 @MP0923   TMS1000   1979, Entex Baseball 2
 @MP1030   TMS1100   1980, APF Mathemagician
 @MP1133   TMS1470   1979, Kosmos Astro
 @MP1180   TMS1100   1980, Tomy Power House Pinball
 @MP1204   TMS1100   1980, Entex Baseball 3 (6007)
 @MP1211   TMS1100   1980, Entex Space Invader
 @MP1218   TMS1100   1980, Entex Basketball 2 (6010)
 @MP1221   TMS1100   1980, Entex Raise The Devil
 *MP1296   TMS1100?  1982, Entex Black Knight
 *MP1312   TMS1100   198?, Tandy/RadioShack Science Fair Microcomputer Trainer
 *MP1359   TMS1100?  1985, Capsela CRC2000
 @MP1525   TMS1170   1980, Coleco Head to Head Baseball
 *MP1604   ?         1981, Hanzawa Twinvader III/Tandy Cosmic Fire Away 3000 (? note: VFD-capable)
 @MP2105   TMS1370   1979, Gakken Poker
 *MP2139   TMS1370?  1982, Gakken Galaxy Invader 1000
 @MP2726   TMS1040   1979, Tomy Break Up
 *MP2788   TMS1040?  1980, Bandai Flight Time (? note: VFD-capable)
 *MP3208   TMS1000   1977, Milton Bradley Electronic Battleship (1977, model 4750A or B)
 @MP3226   TMS1000   1978, Milton Bradley Simon (model 4850)
 *MP3232   TMS1000   1979, Fonas 2-Player Baseball (no "MP" on chip label)
 *MP3300   TMS1000   1980, Estrela Genius (from Brazil, looks and plays identical to Simon)
 @MP3301A  TMS1000   1979, Milton Bradley Big Trak
 *MP3320A  TMS1000   1979, Coleco Head to Head Basketball
 *M32001   TMS1000   1981, Coleco Quiz Wiz Challenger (note: MP3398, MP3399, M3200x?)
  MP3403   TMS1100   1978, Marx Electronic Bowling -> elecbowl.c
 @MP3404   TMS1100   1978, Parker Brothers Merlin
 @MP3405   TMS1100   1979, Coleco Amaze-A-Tron
 *MP3415   TMS1100   1978, Coleco Electronic Quarterback
 @MP3438A  TMS1100   1979, Kenner Star Wars Electronic Battle Command
  MP3450A  TMS1100   1979, MicroVision cartridge: Blockbuster
  MP3454   TMS1100   1979, MicroVision cartridge: Star Trek Phaser Strike
  MP3455   TMS1100   1980, MicroVision cartridge: Pinball
  MP3457   TMS1100   1979, MicroVision cartridge: Mindbuster
 @MP3460   TMS1100   1980, Coleco Head to Head Football
  MP3474   TMS1100   1979, MicroVision cartridge: Vegas Slots
  MP3475   TMS1100   1979, MicroVision cartridge: Bowling
 @MP3476   TMS1100   1979, Milton Bradley Super Simon
  MP3479   TMS1100   1980, MicroVision cartridge: Baseball
  MP3481   TMS1100   1979, MicroVision cartridge: Connect Four
 *MP3491   TMS1100   1980, Mattel Horserace Analyzer
  MP3496   TMS1100   1980, MicroVision cartridge: Sea Duel
  M34009   TMS1100   1981, MicroVision cartridge: Alien Raiders (note: MP3498, MP3499, M3400x..)
 @M34012   TMS1100   1980, Mattel Dungeons & Dragons - Computer Labyrinth Game
  M34017   TMS1100   1981, MicroVision cartridge: Cosmic Hunter
  M34047   TMS1100   1982, MicroVision cartridge: Super Blockbuster
 *M34078A  TMS1100   1983, Milton Bradley Arcade Mania
 @MP6100A  TMS0980   1979, Ideal Electronic Detective
 @MP6101B  TMS0980   1979, Parker Brothers Stop Thief
 *MP6361   ?         1983, Defender Strikes (? note: VFD-capable)
 *MP7303   TMS1400?  19??, Tiger 7-in-1 Sports Stadium
 @MP7313   TMS1400   1980, Parker Brothers Bank Shot
 @MP7314   TMS1400   1980, Parker Brothers Split Second
 *MP7324   TMS1400?  1985, Coleco Talking Teacher
  MP7332   TMS1400   1981, Milton Bradley Dark Tower -> mbdtower.c
 @MP7334   TMS1400   1981, Coleco Total Control 4
 @MP7351   TMS1400CR 1982, Parker Brothers Master Merlin
 @MP7551   TMS1670   1980, Entex Color Football 4 (6009)
 @MPF553   TMS1670   1980, Gakken Jackpot: Gin Rummy & Black Jack (note: assume F to be a misprint)
 *MP7573   TMS1670?  1981, Entex Select-a-Game cartridge: Football 4 (? note: 40-pin, VFD-capable)

  inconsistent:

 *M95041   ?         1983, Tsukuda Game Pachinko (? note: 40-pin, VFD-capable)
 @CD7282SL TMS1100   1981, Tandy/RadioShack Tandy-12 (serial is similar to TI Speak & Spell series?)

  (* denotes not yet emulated by MAME, @ denotes it's in this driver)


  TODO:
  - verify output PLA and microinstructions PLA for MCUs that have been dumped
    electronically (mpla is usually the default, opla is often custom)
  - unknown MCU clocks for some: TMS1000 RC curve is documented in the data manual,
    but not for newer ones (rev. E or TMS1400 MCUs). TMS0970/0980 osc. is on-die.
  - some of the games rely on the fact that faster/longer strobed leds appear
    brighter: tc4/h2hfootb(offense), bankshot(cue ball), ...
  - add softwarelist for tc4 cartridges?
  - stopthiep: unable to start a game (may be intentional?)
  - tbreakup: some of the leds flicker (rom and PLAs doublechecked)

***************************************************************************/

#include "includes/hh_tms1k.h"
#include "machine/tms1024.h"
#include "sound/beep.h"

// internal artwork
#include "amaztron.lh"
#include "astro.lh"
#include "bankshot.lh"
#include "bigtrak.lh"
#include "cnsector.lh"
#include "comp4.lh"
#include "ebball.lh"
#include "ebball2.lh"
#include "ebball3.lh"
#include "ebaskb2.lh"
#include "efootb4.lh"
#include "einvader.lh" // test-layout(but still playable)
#include "elecdet.lh"
#include "gjackpot.lh"
#include "gpoker.lh"
#include "h2hbaseb.lh"
#include "h2hfootb.lh"
#include "mathmagi.lh"
#include "merlin.lh" // clickable
#include "mmerlin.lh" // clickable
#include "simon.lh" // clickable
#include "ssimon.lh" // clickable
#include "splitsec.lh"
#include "starwbc.lh"
#include "stopthie.lh"
#include "tandy12.lh" // clickable
#include "tbreakup.lh"
#include "tc4.lh"

#include "hh_tms1k_test.lh" // common test-layout - use external artwork


// machine_start/reset

void hh_tms1k_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, ~0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_o = 0;
	m_r = 0;
	m_inp_mux = 0;
	m_power_led = false;
	m_power_on = false;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	/* save_item(NAME(m_display_cache)); */ // don't save!
	/* save_item(NAME(m_power_led)); */ // don't save!
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_o));
	save_item(NAME(m_r));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_power_on));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_tms1k_state::machine_reset()
{
	m_power_on = true;
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_tms1k_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_power_on && m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			UINT32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_display_segmask[y] != 0)
				output_set_digit_value(y, active_state[y] & m_display_segmask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x <= m_display_maxx; x++)
			{
				int state = active_state[y] >> x & 1;
				char buf1[0x10]; // lampyx
				char buf2[0x10]; // y.x

				if (x == m_display_maxx)
				{
					// always-on if selected
					sprintf(buf1, "lamp%da", y);
					sprintf(buf2, "%d.a", y);
				}
				else
				{
					sprintf(buf1, "lamp%d", y * mul + x);
					sprintf(buf2, "%d.%d", y, x);
				}
				output_set_value(buf1, state);
				output_set_value(buf2, state);
			}
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));

	// output optional power led
	if (m_power_led != m_power_on)
	{
		m_power_led = m_power_on;
		output_set_value("power_led", m_power_led ? 1 : 0);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_tms1k_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_tms1k_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_tms1k_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	set_display_size(maxx, maxy);

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	display_update();
}

void hh_tms1k_state::display_matrix_seg(int maxx, int maxy, UINT32 setx, UINT32 sety, UINT16 segmask)
{
	// expects m_display_segmask to be not-0
	for (int y = 0; y < maxy; y++)
		m_display_segmask[y] &= segmask;

	display_matrix(maxx, maxy, setx, sety);
}


UINT8 hh_tms1k_state::read_inputs(int columns)
{
	UINT8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}


// devices with a TMS0980 can auto power-off

WRITE_LINE_MEMBER(hh_tms1k_state::auto_power_off)
{
	if (state)
	{
		m_power_on = false;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER(hh_tms1k_state::power_button)
{
	m_power_on = (bool)(FPTR)param;
	m_maincpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  APF Mathemagician
  * TMS1100 MCU, labeled MP1030
  * 2 x DS8870N - Hex LED Digit Driver
  * 2 x DS8861N - MOS-to-LED 5-Segment Driver
  * 10-digit 7seg LED display(2 custom ones) + 4 LEDs, no sound

  This is a tabletop educational calculator. It came with plastic overlays
  for playing different kind of games. Refer to the manual on how to use it.
  In short, to start from scratch, press [SEL]. By default the device is in
  calculator teaching mode. If [SEL] is followed with 1-6 and then [NXT],
  one of the games is started.

  1) Number Machine
  2) Countin' On
  3) Walk The Plank
  4) Gooey Gumdrop
  5) Football
  6) Lunar Lander

***************************************************************************/

class mathmagi_state : public hh_tms1k_state
{
public:
	mathmagi_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void mathmagi_state::prepare_display()
{
	// R0-R7: 7seg leds
	for (int y = 0; y < 8; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (m_r >> y & 1) ? (m_o >> 1) : 0;
	}

	// R8: custom math symbols digit
	// R9: custom equals digit
	// R10: misc lamps
	for (int y = 8; y < 11; y++)
		m_display_state[y] = (m_r >> y & 1) ? m_o : 0;

	set_display_size(8, 11);
	display_update();
}

WRITE16_MEMBER(mathmagi_state::write_r)
{
	// R3,R5-R7,R9,R10: input mux
	m_inp_mux = (data >> 3 & 1) | (data >> 4 & 0xe) | (data >> 5 & 0x30);

	// +others:
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(mathmagi_state::write_o)
{
	// O1-O7: digit segments A-G
	// O0: N/C
	data = (data << 1 & 0xfe) | (data >> 7 & 1); // because opla is unknown
	m_o = data;
}

READ8_MEMBER(mathmagi_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(6);
}


// config

/* physical button layout and labels is like this:

    ON     ONE       [SEL] [NXT] [?]   [/]
     |      |        [7]   [8]   [9]   [x]
    OFF    TWO       [4]   [5]   [6]   [-]
         PLAYERS     [1]   [2]   [3]   [+]
                     [0]   [_]   [r]   [=]
*/

static INPUT_PORTS_START( mathmagi )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.1") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("_") // blank
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("r")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.2") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.3") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("SEL")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("NXT")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("?") // check
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.5") // R10
	PORT_CONFNAME( 0x01, 0x00, "Players")
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// output PLA is not dumped
static const UINT16 mathmagi_output_pla[0x20] =
{
	lA+lB+lC+lD+lE+lF,      // 0
	lB+lC,                  // 1
	lA+lB+lG+lE+lD,         // 2
	lA+lB+lG+lC+lD,         // 3
	lF+lB+lG+lC,            // 4
	lA+lF+lG+lC+lD,         // 5
	lA+lF+lG+lC+lD+lE,      // 6
	lA+lB+lC,               // 7
	lA+lB+lC+lD+lE+lF+lG,   // 8
	lA+lB+lG+lF+lC+lD,      // 9
	lA+lB+lG+lE,            // question mark
	lE+lG,                  // r
	lD,                     // underscore?
	lA+lF+lG+lE+lD,         // E
	lG,                     // -
	0,                      // empty
	0,                      // empty
	lG,                     // lamp 4 or MATH -
	lD,                     // lamp 3
	lF+lE+lD+lC+lG,         // b
	lB,                     // lamp 2
	lB+lG,                  // MATH +
	lB+lC,                  // MATH mul
	lF+lG+lB+lC+lD,         // y
	lA,                     // lamp 1
	lA+lG,                  // MATH div
	lA+lD,                  // EQUALS
	0,                      // ?
	0,                      // ?
	lE+lD+lC+lG,            // o
	0,                      // ?
	lA+lF+lE+lD+lC          // G
};

static MACHINE_CONFIG_START( mathmagi, mathmagi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 175000) // RC osc. R=68K, C=82pf -> ~175kHz
	MCFG_TMS1XXX_OUTPUT_PLA(mathmagi_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(mathmagi_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mathmagi_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mathmagi_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mathmagi)

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Amaze-A-Tron, by Ralph Baer
  * TMS1100 MCU, labeled MP3405(die label too)
  * 2-digit 7seg LED display + 2 LEDs(one red, one green), 1bit sound
  * 5x5 pressure-sensitive playing board

  This is an electronic board game with a selection of 8 maze games,
  most of them for 2 players. A 5x5 playing grid and four markers are
  required to play. Refer to the official manual for more information.

***************************************************************************/

class amaztron_state : public hh_tms1k_state
{
public:
	amaztron_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void amaztron_state::prepare_display()
{
	// R8,R9: select digit
	for (int y = 0; y < 2; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (m_r >> (y + 8) & 1) ? m_o : 0;
	}

	// R6,R7: lamps (-> lamp20,21)
	m_display_state[2] = m_r >> 6 & 3;

	set_display_size(8, 3);
	display_update();
}

WRITE16_MEMBER(amaztron_state::write_r)
{
	// R0-R5: input mux
	m_inp_mux = data & 0x3f;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// other bits:
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(amaztron_state::write_o)
{
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(amaztron_state::read_k)
{
	// K: multiplexed inputs
	UINT8 k = read_inputs(6);

	// the 5th column is tied to K4+K8
	if (k & 0x10) k |= 0xc;
	return k & 0xf;
}


// config

static INPUT_PORTS_START( amaztron )
	PORT_START("IN.0") // R0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Grid 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Grid 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Grid 11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Grid 16")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Grid 21")

	PORT_START("IN.1") // R1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Grid 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Grid 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Grid 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Grid 17")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Grid 22")

	PORT_START("IN.2") // R2
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Grid 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Grid 8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Grid 13")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Grid 18")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Grid 23")

	PORT_START("IN.3") // R3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Grid 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Grid 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Grid 14")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Grid 19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Grid 24")

	PORT_START("IN.4") // R4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Grid 5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Grid 10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Grid 15")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Grid 20")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Grid 25")

	PORT_START("IN.5") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Game Select")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Game Start")
	PORT_BIT(0x1c, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static MACHINE_CONFIG_START( amaztron, amaztron_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K?, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(amaztron_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(amaztron_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(amaztron_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_amaztron)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Head to Head Baseball
  * PCB labels Coleco rev C 73891/2
  * TMS1170NLN MP1525-N2 (die labeled MP1525)
  * 9-digit cyan VFD display, and other LEDs behind bezel, 1bit sound

  known releases:
  - USA: Head to Head Baseball
  - Japan: Computer Baseball, published by Tsukuda

***************************************************************************/

class h2hbaseb_state : public hh_tms1k_state
{
public:
	h2hbaseb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

void h2hbaseb_state::prepare_display()
{
	memset(m_display_segmask, ~0, sizeof(m_display_segmask));
	display_matrix_seg(9, 9, (m_r & 0x100) | m_o, (m_r & 0xff) | (m_r >> 1 & 0x100), 0x7f);
}

WRITE16_MEMBER(h2hbaseb_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R4-R7: input mux
	m_inp_mux = data >> 4 & 0xf;

	// R0-R7,R9: select vfd digit/led
	// R8: led state
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(h2hbaseb_state::write_o)
{
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data;
	prepare_display();
}

READ8_MEMBER(h2hbaseb_state::read_k)
{
	// K: multiplexed inputs (note: K8(Vss row) is always on)
	return m_inp_matrix[4]->read() | read_inputs(4);
}


// config

static INPUT_PORTS_START( h2hbaseb )
	PORT_START("IN.0") // R4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("N")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("B")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("S")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Curve") // these two buttons appear twice on the board
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Fast Pitch")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // Vss!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Swing")

	PORT_START("IN.5") // fake
	PORT_CONFNAME( 0x01, 0x00, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, h2hbaseb_state, skill_switch, NULL)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(h2hbaseb_state::skill_switch)
{
	set_clock();
}


void h2hbaseb_state::set_clock()
{
	// MCU clock is from an RC circuit with C=47pf, and R value is depending on
	// skill switch: R=51K(1) or 43K(2)
	m_maincpu->set_unscaled_clock((m_inp_matrix[5]->read() & 1) ? 400000 : 350000);
}

void h2hbaseb_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( h2hbaseb, h2hbaseb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1170, 350000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(READ8(h2hbaseb_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(h2hbaseb_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(h2hbaseb_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_h2hbaseb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Head to Head Football
  * TMS1100NLLE (rev. E!) MP3460 (die labeled MP3460)
  * 2*SN75492N LED display drivers, 9-digit LED grid, 1bit sound

  LED electronic football game. To distinguish between offense and defense,
  offense blips (should) appear brighter.

***************************************************************************/

class h2hfootb_state : public hh_tms1k_state
{
public:
	h2hfootb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void h2hfootb_state::prepare_display()
{
	memset(m_display_segmask, ~0, sizeof(m_display_segmask));
	display_matrix_seg(9, 9, m_o | (m_r >> 1 & 0x100), (m_r & 0x1ff), 0x7f);
}

WRITE16_MEMBER(h2hfootb_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R8: input mux
	m_inp_mux = data & 0x1ff;

	// R0-R8: select led
	// R9: led between digits
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(h2hfootb_state::write_o)
{
	// O0-O7: digit segments A-G,A'
	m_o = data;
	prepare_display();
}

READ8_MEMBER(h2hfootb_state::read_k)
{
	// K: multiplexed inputs
	UINT8 k = 0;

	// compared to the usual setup, the button matrix is rotated
	for (int i = 0; i < 4; i++)
		if (m_inp_matrix[i]->read() & m_inp_mux)
			k |= 1 << i;

	return k;
}


// config

static INPUT_PORTS_START( h2hfootb )
	PORT_START("IN.0") // K1
	PORT_CONFNAME( 0x03, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.1") // K2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_TOGGLE PORT_NAME("P1 Play Selector") // pass
	PORT_BIT( 0x02, 0x02, IPT_SPECIAL ) PORT_CONDITION("IN.1", 0x01, EQUALS, 0x00) // run/kick

	PORT_START("IN.2") // K4
	PORT_CONFNAME( 0x03, 0x01, "Skill Level" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.3") // K8
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Kick/Pass")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Display")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
INPUT_PORTS_END

static MACHINE_CONFIG_START( h2hfootb, h2hfootb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 325000) // approximation - RC osc. R=39K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(h2hfootb_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(h2hfootb_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(h2hfootb_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_h2hfootb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Total Control 4
  * TMS1400NLL MP7334-N2 (die labeled MP7334)
  * 2x2-digit 7seg LED display + 4 LEDs, LED grid display, 1bit sound

  This is a head to head electronic tabletop LED-display sports console.
  One cartridge(Football) was included with the console, the other three were
  sold in a pack. Gameplay has emphasis on strategy, read the official manual
  on how to play. Remember that you can rotate the view in MAME: rotate left
  for Home(P1) orientation, rotate right for Visitor(P2) orientation.

  Cartridge socket:
  1 N/C
  2 9V+
  3 power switch
  4 K8
  5 K4
  6 K2
  7 K1
  8 R9

  The cartridge connects pin 8 with one of the K-pins.

  Available cartridges:
  - Football    (K8, confirmed)
  - Hockey      (K4?)
  - Soccer      (K2?)
  - Basketball  (K1?)

***************************************************************************/

class tc4_state : public hh_tms1k_state
{
public:
	tc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void tc4_state::prepare_display()
{
	// R5,R7-R9 are 7segs
	for (int y = 5; y < 10; y++)
		if (y != 6)
			m_display_segmask[y] = 0x7f;

	// update current state (note: R6 as extra column!)
	display_matrix(9, 10, (m_o | (m_r << 2 & 0x100)), m_r);
}

WRITE16_MEMBER(tc4_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R9: to cartridge slot
	m_inp_mux = (data & 0x3f) | (data >> 3 & 0x40);

	// R0-R4: select led
	// R6: led 8 state
	// R5,R7-R9: select digit
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(tc4_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(tc4_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(7);
}


// config

static INPUT_PORTS_START( tc4 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pass/Shoot Button 3") // right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass/Shoot Button 1") // left
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pass/Shoot Button 2") // middle
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 D/K Button")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 3") // right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 1") // left
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 2") // middle
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D/K Button")

	PORT_START("IN.6") // R9
	PORT_CONFNAME( 0x0f, 0x08, "Cartridge")
	PORT_CONFSETTING(    0x01, "Basketball" )
	PORT_CONFSETTING(    0x02, "Soccer" )
	PORT_CONFSETTING(    0x04, "Hockey" )
	PORT_CONFSETTING(    0x08, "Football" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tc4, tc4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 450000) // approximation - RC osc. R=27.3K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(tc4_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tc4_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tc4_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tc4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex (Electronic) Baseball (1)
  * TMS1000NLP MP0914 (die labeled MP0914A)
  * 1 7seg LED, and other LEDs behind bezel, 1bit sound

  This is a handheld LED baseball game. One player controls the batter, the CPU
  or other player controls the pitcher. Pitcher throw buttons are on a 'joypad'
  obtained from a compartment in the back. Player scores are supposed to be
  written down manually, the game doesn't save scores or innings (this annoyance
  was resolved in the sequel). For more information, refer to the official manual.

  The overlay graphic is known to have 2 versions: one where the field players
  are denoted by words ("left", "center", "short", etc), and an alternate one
  with little guys drawn next to the LEDs.

  lamp translation table: led LDzz from game PCB = MAME lampyx:

    LD0  = -        LD10 = lamp12   LD20 = lamp42   LD30 = lamp60
    LD1  = lamp23   LD11 = lamp4    LD21 = lamp41   LD31 = lamp61
    LD2  = lamp0    LD12 = lamp15   LD22 = lamp40   LD32 = lamp62
    LD3  = lamp1    LD13 = lamp22   LD23 = lamp43   LD33 = lamp70
    LD4  = lamp2    LD14 = lamp33   LD24 = lamp53   LD34 = lamp71
    LD5  = lamp10   LD15 = lamp32   LD25 = lamp52
    LD6  = lamp13   LD16 = lamp21   LD26 = lamp51
    LD7  = lamp11   LD17 = lamp31   LD27 = lamp50
    LD8  = lamp3    LD18 = lamp30   LD28 = lamp72
    LD9  = lamp14   LD19 = lamp20   LD29 = lamp73

***************************************************************************/

class ebball_state : public hh_tms1k_state
{
public:
	ebball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void ebball_state::prepare_display()
{
	// R8 is a 7seg
	m_display_segmask[8] = 0x7f;

	display_matrix(7, 9, ~m_o, m_r);
}

WRITE16_MEMBER(ebball_state::write_r)
{
	// R1-R5: input mux
	m_inp_mux = data >> 1 & 0x1f;

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ebball_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(ebball_state::read_k)
{
	// K: multiplexed inputs (note: K8(Vss row) is always on)
	return m_inp_matrix[5]->read() | read_inputs(5);
}


// config

static INPUT_PORTS_START( ebball )
	PORT_START("IN.0") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Change Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Change Sides")
	PORT_CONFNAME( 0x04, 0x04, "Players" )
	PORT_CONFSETTING(    0x04, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fast Ball")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Knuckler")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Curve")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Slider")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // Vss!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Batter")
INPUT_PORTS_END

static MACHINE_CONFIG_START( ebball, ebball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 375000) // RC osc. R=43K, C=47pf -> ~375kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(ebball_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ebball_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(ebball_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ebball)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex (Electronic) Baseball 2
  * PCBs are labeled: ZENY
  * TMS1000 MCU, MP0923 (die labeled MP0923)
  * 3 7seg LEDs, and other LEDs behind bezel, 1bit sound

  The Japanese version was published by Gakken, black casing instead of white.

  The sequel to Entex Baseball, this version keeps up with score and innings.
  As its predecessor, the pitcher controls are on a separate joypad.

  lamp translation table: led zz from game PCB = MAME lampyx:

    00 = -        10 = lamp94   20 = lamp74   30 = lamp50
    01 = lamp53   11 = lamp93   21 = lamp75   31 = lamp51
    02 = lamp7    12 = lamp92   22 = lamp80   32 = lamp52
    03 = lamp17   13 = lamp62   23 = lamp81   33 = lamp40
    04 = lamp27   14 = lamp70   24 = lamp82   34 = lamp41
    05 = lamp97   15 = lamp71   25 = lamp83   35 = lamp31
    06 = lamp90   16 = lamp61   26 = lamp84   36 = lamp30
    07 = lamp95   17 = lamp72   27 = lamp85   37 = lamp33
    08 = lamp63   18 = lamp73   28 = lamp42   38 = lamp32
    09 = lamp91   19 = lamp60   29 = lamp43

***************************************************************************/

class ebball2_state : public hh_tms1k_state
{
public:
	ebball2_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void ebball2_state::prepare_display()
{
	// R0-R2 are 7segs
	for (int y = 0; y < 3; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(8, 10, ~m_o, m_r ^ 0x7f);
}

WRITE16_MEMBER(ebball2_state::write_r)
{
	// R3-R6: input mux
	m_inp_mux = data >> 3 & 0xf;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ebball2_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(ebball2_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( ebball2 )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, "Players" )
	PORT_CONFSETTING(    0x02, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fast Ball")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Batter")

	PORT_START("IN.1") // R4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Steal")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Change Up")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Slider")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Knuckler")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Curve")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ebball2, ebball2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 350000) // RC osc. R=47K, C=47pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(ebball2_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ebball2_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(ebball2_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ebball2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex (Electronic) Baseball 3
  * PCBs are labeled: ZENY
  * TMS1100NLL 6007 MP1204 (rev. E!) (die labeled MP1204)
  * 2*SN75492N LED display driver
  * 4 7seg LEDs, and other LEDs behind bezel, 1bit sound

  This is another improvement over Entex Baseball, where gameplay is a bit more
  varied. Like the others, the pitcher controls are on a separate joypad.

  lamp translation table: led zz from game PCB = MAME lampyx:
  note: unlabeled panel leds are listed here as Sz, Bz, Oz, Iz, z left-to-right

    00 = -        10 = lamp75   20 = lamp72
    01 = lamp60   11 = lamp65   21 = lamp84
    02 = lamp61   12 = lamp55   22 = lamp85
    03 = lamp62   13 = lamp52   23 = lamp90
    04 = lamp63   14 = lamp80   24 = lamp92
    05 = lamp73   15 = lamp81   25 = lamp91
    06 = lamp53   16 = lamp51   26 = lamp93
    07 = lamp74   17 = lamp82   27 = lamp95
    08 = lamp64   18 = lamp83   28 = lamp94
    09 = lamp54   19 = lamp50

    S1,S2: lamp40,41
    B1-B3: lamp30-32
    O1,O2: lamp42,43
    I1-I6: lamp20-25, I7-I9: lamp33-35

***************************************************************************/

class ebball3_state : public hh_tms1k_state
{
public:
	ebball3_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

void ebball3_state::prepare_display()
{
	// update current state
	for (int y = 0; y < 10; y++)
		m_display_state[y] = (m_r >> y & 1) ? m_o : 0;

	// R0,R1 are normal 7segs
	m_display_segmask[0] = m_display_segmask[1] = 0x7f;

	// R4,R7 contain segments(only F and B) for the two other digits
	m_display_state[10] = (m_display_state[4] & 0x20) | (m_display_state[7] & 0x02);
	m_display_state[11] = ((m_display_state[4] & 0x10) | (m_display_state[7] & 0x01)) << 1;
	m_display_segmask[10] = m_display_segmask[11] = 0x22;

	set_display_size(7, 10+2);
	display_update();
}

WRITE16_MEMBER(ebball3_state::write_r)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ebball3_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(ebball3_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}


// config

/* physical button layout and labels is like this:

    main device (batter side):            remote pitcher:

                          MAN
    PRO                    |              [FAST BALL]  [CHANGE UP]    [CURVE]  [SLIDER]
     |                  OFF|
     o                     o                   [STEAL DEFENSE]           [KNUCKLER]
    AM                    AUTO

    [BUNT]  [BATTER]  [STEAL]
*/

static INPUT_PORTS_START( ebball3 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fast Ball")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Change Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Slider")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Curve")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Knuckler")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Steal")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Batter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Steal Defense")

	PORT_START("IN.2") // R2
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Bunt")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // fake
	PORT_CONFNAME( 0x01, 0x00, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ebball3_state, skill_switch, NULL)
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(ebball3_state::skill_switch)
{
	set_clock();
}


void ebball3_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=33pf) oscillating by default at ~340kHz,
	// but on PRO, the difficulty switch adds an extra 150K resistor to Vdd to speed
	// it up to around ~440kHz.
	m_maincpu->set_unscaled_clock((m_inp_matrix[3]->read() & 1) ? 440000 : 340000);
}

void ebball3_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( ebball3, ebball3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 340000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(READ8(ebball3_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ebball3_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(ebball3_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ebball3)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Space Invader
  * TMS1100 MP1211 (die labeled MP1211)
  * 3 7seg LEDs, LED matrix and overlay mask, 1bit sound

  There are two versions of this game: the first release(this one) is on
  TMS1100, the second more widespread release runs on a COP400. There are
  also differences with the overlay mask.

  NOTE!: MAME external artwork is required

***************************************************************************/

class einvader_state : public hh_tms1k_state
{
public:
	einvader_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

void einvader_state::prepare_display()
{
	// R7-R9 are 7segs
	for (int y = 7; y < 10; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(8, 10, m_o, m_r);
}

WRITE16_MEMBER(einvader_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(einvader_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}


// config

static INPUT_PORTS_START( einvader )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_CONFNAME( 0x08, 0x00, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, einvader_state, skill_switch, NULL)
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x08, "Professional" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(einvader_state::skill_switch)
{
	set_clock();
}


void einvader_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=56pf) oscillating by default at ~320kHz,
	// but on PRO, the difficulty switch adds an extra 180K resistor to Vdd to speed
	// it up to around ~400kHz.
	m_maincpu->set_unscaled_clock((m_inp_matrix[0]->read() & 8) ? 400000 : 320000);
}

void einvader_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( einvader, einvader_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 320000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(IOPORT("IN.0"))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(einvader_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(einvader_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_einvader)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Color Football 4
  * TMS1670 6009 MP7551 (die also labeled MP7551)
  * 9-digit cyan VFD display, 60 red and green LEDs behind bezel, 1bit sound

***************************************************************************/

class efootb4_state : public hh_tms1k_state
{
public:
	efootb4_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void efootb4_state::prepare_display()
{
	// R10-R15 are 7segs
	for (int y = 10; y < 16; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(7, 16, m_o, m_r);
}

WRITE16_MEMBER(efootb4_state::write_r)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(efootb4_state::write_o)
{
	// O7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// O0-O6: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(efootb4_state::read_k)
{
	return read_inputs(5);
}


// config

static INPUT_PORTS_START( efootb4 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY // 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY // 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY // 4

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY // 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY // 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY // 3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY // 4

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pass")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Kick")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Run")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Kick")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_CONFNAME( 0x01, 0x01, "Players" )
	PORT_CONFSETTING(    0x01, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_CONFNAME( 0x02, 0x00, "Skill Level" )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Status")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( efootb4, efootb4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1670, 475000) // approximation - RC osc. R=42K, C=47pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(efootb4_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(efootb4_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(efootb4_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_efootb4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex (Electronic) Basketball 2
  * TMS1100 6010 MP1218 (die also labeled MP1218)
  * 4 7seg LEDs, and other LEDs behind bezel, 1bit sound

  lamp translation table: led zz from game PCB = MAME lampyx:

    11 = lamp90   21 = lamp91   31 = lamp92   41 = lamp93   51 = lamp95
    12 = lamp80   22 = lamp81   32 = lamp82   42 = lamp83   52 = lamp85
    13 = lamp70   23 = lamp71   33 = lamp72   43 = lamp73   53 = lamp84
    14 = lamp60   24 = lamp61   34 = lamp62   44 = lamp63   54 = lamp75
    15 = lamp50   25 = lamp51   35 = lamp52   45 = lamp53   55 = lamp74
    16 = lamp40   26 = lamp41   36 = lamp42   46 = lamp43   56 = lamp65

    A  = lamp94
    B  = lamp64

***************************************************************************/

class ebaskb2_state : public hh_tms1k_state
{
public:
	ebaskb2_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void ebaskb2_state::prepare_display()
{
	// R0-R3 are 7segs
	for (int y = 0; y < 4; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(7, 10, m_o, m_r);
}

WRITE16_MEMBER(ebaskb2_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R6-R9: input mux
	m_inp_mux = data >> 6 & 0xf;

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ebaskb2_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data;
	prepare_display();
}

READ8_MEMBER(ebaskb2_state::read_k)
{
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( ebaskb2 )
	PORT_START("IN.0") // R6
	PORT_CONFNAME( 0x01, 0x01, "Skill Level" )
	PORT_CONFSETTING(    0x01, "Amateur" )
	PORT_CONFSETTING(    0x00, "Professional" )
	PORT_CONFNAME( 0x02, 0x02, "Players" )
	PORT_CONFSETTING(    0x02, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass")

	PORT_START("IN.1") // R7
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Shoot")

	PORT_START("IN.2") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.3") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
INPUT_PORTS_END

static MACHINE_CONFIG_START( ebaskb2, ebaskb2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 360000) // RC osc. R=33K, C=82pf -> ~360kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(ebaskb2_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ebaskb2_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(ebaskb2_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ebaskb2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Raise The Devil
  * TMS1100 MP1221 (die labeled MP1221)
  * 4 7seg LEDs(rightmost one unused), and other LEDs behind bezel, 1bit sound

  lamp translation table: led zz from game PCB = MAME lampyx:

    0 = -          10 = lamp44     20 = lamp53     30 = lamp95     40 = lamp92
    1 = lamp30     11 = lamp45     21 = lamp54     31 = lamp85     41 = lamp93
    2 = lamp31     12 = -          22 = -          32 = lamp65     42 = lamp90
    3 = lamp32     13 = lamp50     23 = lamp60     33 = lamp74     43 = lamp91
    4 = lamp33     14 = lamp51     24 = lamp61     34 = lamp70
    5 = lamp34     15 = lamp52     25 = lamp62     35 = lamp71
    6 = lamp40     16 = -          26 = lamp63     36 = lamp80
    7 = lamp41     17 = lamp72     27 = lamp64     37 = lamp81
    8 = lamp42     18 = lamp73     28 = lamp84     38 = lamp82
    9 = lamp43     19 = -          29 = lamp94     39 = lamp83

  NOTE!: MAME external artwork is required

***************************************************************************/

class raisedvl_state : public hh_tms1k_state
{
public:
	raisedvl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

void raisedvl_state::prepare_display()
{
	// R0-R2 are 7segs
	for (int y = 0; y < 3; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(7, 10, m_o, m_r);
}

WRITE16_MEMBER(raisedvl_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0,R1: input mux
	m_inp_mux = data & 3;

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(raisedvl_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(raisedvl_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(2) & 0xf;
}


// config

static INPUT_PORTS_START( raisedvl )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1 (only bit 0)
	PORT_CONFNAME( 0x31, 0x00, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, raisedvl_state, skill_switch, NULL)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x10, "2" )
	PORT_CONFSETTING(    0x11, "3" )
	PORT_CONFSETTING(    0x21, "4" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(raisedvl_state::skill_switch)
{
	set_clock();
}


void raisedvl_state::set_clock()
{
	// MCU clock is from an RC circuit with C=47pf, R=47K by default. Skills
	// 2 and 3 add a 150K resistor in parallel, and skill 4 adds a 100K one.
	// 0:   R=47K  -> ~350kHz
	// 2,3: R=35K8 -> ~425kHz (combined)
	// 4:   R=32K  -> ~465kHz (combined)
	UINT8 inp = m_inp_matrix[1]->read();
	m_maincpu->set_unscaled_clock((inp & 0x20) ? 465000 : ((inp & 0x10) ? 425000 : 350000));
}

void raisedvl_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( raisedvl, raisedvl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(READ8(raisedvl_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(raisedvl_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(raisedvl_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_tms1k_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gakken Poker
  * PCB label POKER. gakken
  * TMS1370 MP2105 (die labeled MP2105)
  * 11-digit cyan VFD display Itron FG1114B, oscillator sound

  known releases:
  - Japan: Poker
  - USA: Electronic Poker, published by Entex

***************************************************************************/

class gpoker_state : public hh_tms1k_state
{
public:
	gpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_beeper(*this, "beeper")
	{ }

	required_device<beep_device> m_beeper;

	void prepare_display();
	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);

protected:
	virtual void machine_reset() override;
};

// handlers

void gpoker_state::prepare_display()
{
	memset(m_display_segmask, ~0, sizeof(m_display_segmask));
	display_matrix_seg(12, 11, m_o | (m_r >> 3 & 0xf00), m_r & 0x7ff, 0x7f);
}

WRITE16_MEMBER(gpoker_state::write_r)
{
	// R15: enable beeper
	m_beeper->set_state(data >> 15 & 1);

	// R0-R6: input mux
	m_inp_mux = data & 0x7f;

	// R0-R10: select digit
	// R11-R14: card symbols
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(gpoker_state::write_o)
{
	// O0-O7: digit segments A-G,H
	m_o = data;
	prepare_display();
}

READ8_MEMBER(gpoker_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(7);
}


// config

/* physical button layout and labels is like this:

    [7]   [8]   [9]   [DL]   | (on/off switch)
    [4]   [5]   [6]   [BT]
    [1]   [2]   [3]   [CA]  [CE]
    [0]         [GO]  [T]   [AC]
*/

static INPUT_PORTS_START( gpoker )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Go")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("9/Deal") // DL, shares pad with 9
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear Entry") // CE

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear All") // AC

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Call") // CA
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Total") // T
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet") // BT
INPUT_PORTS_END


void gpoker_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	m_beeper->set_state(0);
}

static MACHINE_CONFIG_START( gpoker, gpoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1370, 350000) // RC osc. R=47K, C=47pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(gpoker_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(gpoker_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(gpoker_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_gpoker)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 2405) // astable multivibrator - C1 and C2 are 0.003uF, R1 and R4 are 1K, R2 and R3 are 100K
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gakken Jackpot: Gin Rummy & Black Jack
  * PCB label gakken
  * TMS1670 MPF553 (die labeled MPF553)
  * 11-digit cyan VFD display Itron FG1114B, oscillator sound

  known releases:
  - Japan: Jackpot(?)
  - USA: Electronic Jackpot: Gin Rummy & Black Jack, published by Entex

***************************************************************************/

class gjackpot_state : public gpoker_state
{
public:
	gjackpot_state(const machine_config &mconfig, device_type type, const char *tag)
		: gpoker_state(mconfig, type, tag)
	{ }

	virtual DECLARE_WRITE16_MEMBER(write_r) override;
};

// handlers

WRITE16_MEMBER(gjackpot_state::write_r)
{
	// same as gpoker, only input mux msb is R10 instead of R6
	gpoker_state::write_r(space, offset, data);
	m_inp_mux = (data & 0x3f) | (data >> 4 & 0x40);
}


// config

/* physical button layout and labels is like this:
  (note: on dual-function buttons, upper label=Gin, lower label=Black Jack)

                       BJ --o GIN
                          OFF
    [7]    [8]    [9]    [DL]

    [4]    [5]    [6]    [      [KN
                          DB]    SP]
    [1]    [2]    [3]    [DS]   [DR]
                          BT]    HT]
    [10/1] [T]    [MD    [CH    [AC]
                   GO]    ST]
*/

static INPUT_PORTS_START( gjackpot )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10/0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Draw/Hit") // DR/HT
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Deal") // DL

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Knock/Split") // KN/SP
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Total") // T

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Display/Bet") // DS/BT
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Meld/Go") // MD/GO
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Double") // DB

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Change/Stand") // CH/ST
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear Entry") // CE
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R10
	PORT_CONFNAME( 0x06, 0x04, "Game Select" )
	PORT_CONFSETTING(    0x04, "Gin Rummy" )
	PORT_CONFSETTING(    0x02, "Black Jack" )
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( gjackpot, gjackpot_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1670, 450000) // approximation - RC osc. R=47K, C=47pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(gpoker_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(gjackpot_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(gpoker_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_gjackpot)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 2405) // see gpoker
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Ideal Electronic Detective
  * TMS0980NLL MP6100A (die labeled 0980B-00)
  * 10-digit 7seg LED display, 1bit sound

  hardware (and concept) is very similar to Parker Bros Stop Thief

  This is an electronic board game. It requires game cards with suspect info,
  and good old pen and paper to record game progress. To start the game, enter
  difficulty(1-3), then number of players(1-4), then [ENTER]. Refer to the
  manual for more information.

***************************************************************************/

class elecdet_state : public hh_tms1k_state
{
public:
	elecdet_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(elecdet_state::write_r)
{
	// R7,R8: speaker on
	m_speaker->level_w((data & 0x180 && m_o & 0x80) ? 1 : 0);

	// R0-R6: select digit
	memset(m_display_segmask, ~0, sizeof(m_display_segmask));
	display_matrix_seg(7, 7, BITSWAP8(m_o,7,5,2,1,4,0,6,3), data, 0x7f);
}

WRITE16_MEMBER(elecdet_state::write_o)
{
	// O0,O1,O4,O6: input mux
	m_inp_mux = (data & 3) | (data >> 2 & 4) | (data >> 3 & 8);

	// O0-O6: digit segments A-G
	// O7: speaker out -> write_r
	m_o = data;
}

READ8_MEMBER(elecdet_state::read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[4]->read() | read_inputs(4);
}


// config

/* physical button layout and labels is like this:

    [1]   [2]   [3]   [SUSPECT]
    [4]   [5]   [6]   [PRIVATE QUESTION]
    [7]   [8]   [9]   [I ACCUSE]
                [0]   [ENTER]
    [ON]  [OFF]       [END TURN]
*/

static INPUT_PORTS_START( elecdet )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Private Question")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("I Accuse")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.3") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Suspect")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.4") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("End Turn")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, (void *)false)
INPUT_PORTS_END

static MACHINE_CONFIG_START( elecdet, elecdet_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, 425000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(elecdet_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(elecdet_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(elecdet_state, write_o))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(hh_tms1k_state, auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_elecdet)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Kenner Star Wars - Electronic Battle Command
  * TMS1100 MCU, labeled MP3438A
  * 4x4 LED grid display + 2 separate LEDs and 2-digit 7segs, 1bit sound

  This is a small tabletop space-dogfighting game. To start the game,
  press BASIC/INTER/ADV and enter P#(number of players), then
  START TURN. Refer to the official manual for more information.

***************************************************************************/

class starwbc_state : public hh_tms1k_state
{
public:
	starwbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void starwbc_state::prepare_display()
{
	// R6,R8 are 7segs
	m_display_segmask[6] = m_display_segmask[8] = 0x7f;
	display_matrix(8, 10, m_o, m_r);
}

WRITE16_MEMBER(starwbc_state::write_r)
{
	// R0,R1,R3,R5,R7: input mux
	m_inp_mux = (data & 3) | (data >> 1 & 4) | (data >> 2 & 8) | (data >> 3 & 0x10);

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0,R2,R4,R6,R8: led select
	m_r = data & 0x155;
	prepare_display();
}

WRITE16_MEMBER(starwbc_state::write_o)
{
	// O0-O7: led state
	m_o = (data << 4 & 0xf0) | (data >> 4 & 0x0f);
	prepare_display();
}

READ8_MEMBER(starwbc_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}


// config

/* physical button layout and labels is like this:

    (reconnnaissance=yellow)        (tactical reaction=green)
    [MAGNA] [ENEMY]                 [EM]       [BS]   [SCR]

    [BASIC] [INTER]    [START TURN] [END TURN] [MOVE] [FIRE]
    [ADV]   [P#]       [<]          [^]        [>]    [v]
    (game=blue)        (maneuvers=red)
*/

static INPUT_PORTS_START( starwbc )
	PORT_START("IN.0") // R0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Basic Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Intermediate Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Advanced Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Player Number")

	PORT_START("IN.1") // R1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Turn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("End Turn")

	PORT_START("IN.2") // R3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Magna Scan") // only used in adv. game
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Enemy Scan") // only used in adv. game
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Screen Up")

	PORT_START("IN.3") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Evasive Maneuvers")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Fire")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Battle Stations")

	PORT_START("IN.4") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
INPUT_PORTS_END

static MACHINE_CONFIG_START( starwbc, starwbc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 325000) // RC osc. R=51K, C=47pf -> ~325kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(starwbc_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(starwbc_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(starwbc_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_starwbc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Kosmos Astro
  * TMS1470NLHL MP1133 (die labeled TMS1400 MP1133)
  * 9digit 7seg VFD display + 8 LEDs(4 green, 4 yellow), no sound

  This is an astrological calculator, and also supports 4-function
  calculations. Refer to the official manual on how to use this device.

***************************************************************************/

class astro_state : public hh_tms1k_state
{
public:
	astro_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void astro_state::prepare_display()
{
	memset(m_display_segmask, ~0, sizeof(m_display_segmask));
	display_matrix_seg(8, 10, m_o, m_r, 0xff);
}

WRITE16_MEMBER(astro_state::write_r)
{
	// R0-R7: input mux
	m_inp_mux = data & 0xff;

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(astro_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(astro_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(8);
}


// config

static INPUT_PORTS_START( astro )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE"/Sun")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY"/Mercury")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-/Venus")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+/Mars")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=/Astro")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("B1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("B2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, "Mode" )
	PORT_CONFSETTING(    0x00, "Calculator" )
	PORT_CONFSETTING(    0x08, "Astro" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( astro, astro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1470, 450000) // approximation - RC osc. R=4.7K, C=33pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(astro_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(astro_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(astro_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_astro)

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Dungeons & Dragons - Computer Labyrinth Game
  * TMS1100 M34012-N2LL (die labeled M34012)
  * 72 buttons, no LEDs, 1bit sound

  This is an electronic board game. It requires markers and wall pieces to play.
  Refer to the official manual for more information.

***************************************************************************/

class mdndclab_state : public hh_tms1k_state
{
public:
	mdndclab_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(mdndclab_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: input mux part
	m_inp_mux = (m_inp_mux & 0xff) | (data << 8 & 0x3ff00);
}

WRITE16_MEMBER(mdndclab_state::write_o)
{
	// O0-O7: input mux part
	m_inp_mux = (m_inp_mux & ~0xff) | data;
}

READ8_MEMBER(mdndclab_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(18);
}


// config

/* physical button layout and labels is like this:

    8 buttons on the left, top-to-bottom: (lower 6 are just for sound-preview)
    [Switch Key]  [Next Turn / Level 1/2]  [Dragon Flying / Defeat Tune]  [Dragon Attacks / Dragon Wakes]
    [Wall / Door]  [Illegal Move / Warrior Moves]  [Warrior 1 / Winner]  [Warrior 2 / Treasure]

    8*8 buttons to the right of that, making the gameboard

*/

static INPUT_PORTS_START( mdndclab )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a1")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b1")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c1")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d1")

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e1")

	PORT_START("IN.5") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f1")

	PORT_START("IN.6") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g1")

	PORT_START("IN.7") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h1")

	PORT_START("IN.8") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Wall / Door")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Illegal Move / Warrior Moves")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Warrior 1 / Winner")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Warrior 2 / Treasure")

	PORT_START("IN.9") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid a5")

	PORT_START("IN.10") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid b5")

	PORT_START("IN.11") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid c5")

	PORT_START("IN.12") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid d5")

	PORT_START("IN.13") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid e5")

	PORT_START("IN.14") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid f5")

	PORT_START("IN.15") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid g5")

	PORT_START("IN.16") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Grid h5")

	PORT_START("IN.17") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Switch Key")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Next Turn / Level 1/2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Dragon Flying / Defeat Tune")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Dragon Attacks / Dragon Wakes")
INPUT_PORTS_END


static MACHINE_CONFIG_START( mdndclab, mdndclab_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 500000) // approximation - RC osc. R=27K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(mdndclab_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mdndclab_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mdndclab_state, write_o))

	/* no visual feedback! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die labeled 4A0970D-04A)
  * 10 LEDs behind bezel, no sound

  known releases:
  - USA: Comp IV (two versions, different casing)
  - Europe: Logic 5
  - Japan: Pythaligoras

  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.

***************************************************************************/

class comp4_state : public hh_tms1k_state
{
public:
	comp4_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(comp4_state::write_r)
{
	// leds:
	// R4    R9
	// R10!  R8
	// R2    R7
	// R1    R6
	// R0    R5
	m_r = data;
	display_matrix(11, 1, m_r, m_o);
}

WRITE16_MEMBER(comp4_state::write_o)
{
	// O1-O3: input mux
	m_inp_mux = data >> 1 & 7;

	// O0: leds common
	// other bits: N/C
	m_o = data;
	display_matrix(11, 1, m_r, m_o);
}

READ8_MEMBER(comp4_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}


// config

static INPUT_PORTS_START( comp4 )
	PORT_START("IN.0") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("R")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.1") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.2") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
INPUT_PORTS_END

static MACHINE_CONFIG_START( comp4, comp4_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(comp4_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(comp4_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(comp4_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_comp4)

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Simon, created by Ralph Baer

  Revision A hardware:
  * TMS1000 (die labeled MP3226)
  * DS75494 Hex digit LED driver, 4 big lamps, 1bit sound

  Newer revisions (also Pocket Simon) have a smaller 16-pin MB4850 chip
  instead of the TMS1000. This one has been decapped too, but we couldn't
  find an internal ROM. It is possibly a cost-reduced custom ASIC specifically
  for Simon. The semi-sequel Super Simon uses a TMS1100 (see next minidriver).

***************************************************************************/

class simon_state : public hh_tms1k_state
{
public:
	simon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(simon_state::write_r)
{
	// R4-R8 go through an 75494 IC first:
	// R4 -> 75494 IN6 -> green lamp
	// R5 -> 75494 IN3 -> red lamp
	// R6 -> 75494 IN5 -> yellow lamp
	// R7 -> 75494 IN2 -> blue lamp
	display_matrix(4, 1, data >> 4, 1);

	// R8 -> 75494 IN0 -> speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R0-R2,R9: input mux
	// R3: GND
	// other bits: N/C
	m_inp_mux = (data & 7) | (data >> 6 & 8);
}

READ8_MEMBER(simon_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( simon )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x07, 0x02, "Game Select")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Green Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Blue Button")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Longest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R9
	PORT_CONFNAME( 0x0f, 0x02, "Skill Level")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( simon, simon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(simon_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(simon_state, write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_simon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Super Simon
  * TMS1100 MP3476NLL (die labeled MP3476)
  * 8 big lamps(2 turn on at same time), 1bit sound

  The semi-squel to Simon, not as popular. It includes more game variations
  and a 2-player head-to-head mode.

***************************************************************************/

class ssimon_state : public hh_tms1k_state
{
public:
	ssimon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(speed_switch);

protected:
	virtual void machine_reset() override;
};

// handlers

WRITE16_MEMBER(ssimon_state::write_r)
{
	// R0-R3,R9,R10: input mux
	m_inp_mux = (data & 0xf) | (data >> 5 & 0x30);

	// R4: yellow lamps
	// R5: green lamps
	// R6: blue lamps
	// R7: red lamps
	display_matrix(4, 1, data >> 4, 1);

	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);
}

READ8_MEMBER(ssimon_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(6);
}


// config

static INPUT_PORTS_START( ssimon )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x0f, 0x01, "Game Select")
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x08, "4" )
	PORT_CONFSETTING(    0x00, "5" )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Yellow Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Green Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Blue Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Red Button")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Longest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Decision")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Yellow Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Green Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Blue Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Red Button")

	PORT_START("IN.4") // R9
	PORT_CONFNAME( 0x0f, 0x02, "Skill Level")
	PORT_CONFSETTING(    0x00, "Head-to-Head" ) // this sets R10 K2, see below
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )

	PORT_START("IN.5") // R10
	PORT_BIT( 0x02, 0x02, IPT_SPECIAL ) PORT_CONDITION("IN.4", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // fake
	PORT_CONFNAME( 0x03, 0x01, "Speed" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssimon_state, speed_switch, NULL)
	PORT_CONFSETTING(    0x00, "Simple" )
	PORT_CONFSETTING(    0x01, "Normal" )
	PORT_CONFSETTING(    0x02, "Super" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(ssimon_state::speed_switch)
{
	set_clock();
}


void ssimon_state::set_clock()
{
	// MCU clock is from an RC circuit with C=100pf, R=x depending on speed switch:
	// 0 Simple: R=51K -> ~200kHz
	// 1 Normal: R=37K -> ~275kHz
	// 2 Super:  R=22K -> ~400kHz
	UINT8 inp = m_inp_matrix[6]->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 400000 : ((inp & 1) ? 275000 : 200000));
}

void ssimon_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

static MACHINE_CONFIG_START( ssimon, ssimon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 275000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(READ8(ssimon_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(ssimon_state, write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ssimon)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Big Trak
  * TMS1000NLL MP3301A or MP3301ANLL E (rev. E!) (die labeled 1000E MP3301)
  * SN75494N Hex digit LED driver, 1 lamp, 3-level sound
  * gearbox with magnetic clutch, 1 IR led+sensor, 2 motors(middle wheels)
  * 24-button keypad, ext in/out ports

  Big Trak is a programmable toy car, up to 16 steps. Supported commands include
  driving and turning, firing a photon cannon(hey, have some imagination!),
  and I/O ports. The output port was used for powering the dump truck accessory.

  The In command was canceled midst production, it is basically a nop. Newer
  releases and the European version removed the button completely.

***************************************************************************/

class bigtrak_state : public hh_tms1k_state
{
public:
	bigtrak_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	int m_gearbox_pos;
	bool sensor_state() { return m_gearbox_pos < 0 && m_display_decay[0][0] != 0; }
	TIMER_DEVICE_CALLBACK_MEMBER(gearbox_sim_tick);

protected:
	virtual void machine_start() override;
};

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(bigtrak_state::gearbox_sim_tick)
{
	// the last gear in the gearbox has 12 evenly spaced holes, it is located
	// between an IR emitter and receiver
	static const int speed = 17;
	if (m_gearbox_pos >= speed)
		m_gearbox_pos = -speed;

	if (m_o & 0x1e)
		m_gearbox_pos++;
}

WRITE16_MEMBER(bigtrak_state::write_r)
{
	// R0-R5,R8: input mux (keypad, ext in enable)
	m_inp_mux = (data & 0x3f) | (data >> 2 & 0x40);

	// R6: N/C
	// R7: IR led on
	// R9: lamp on
	display_matrix(2, 1, (data >> 7 & 1) | (data >> 8 & 2), 1);

	// (O0,O7,)R10(tied together): speaker out
	m_speaker->level_w((m_o & 1) | (m_o >> 6 & 2) | (data >> 8 & 4));
	m_r = data;
}

WRITE16_MEMBER(bigtrak_state::write_o)
{
	// O1: left motor forward
	// O2: left motor reverse
	// O3: right motor reverse
	// O4: right motor reverse
	// O5: ext out
	// O6: N/C
	output_set_value("left_motor_forward", data >> 1 & 1);
	output_set_value("left_motor_reverse", data >> 2 & 1);
	output_set_value("right_motor_forward", data >> 3 & 1);
	output_set_value("right_motor_reverse", data >> 4 & 1);
	output_set_value("ext_out", data >> 5 & 1);

	// O0,O7(,R10)(tied together): speaker out
	m_speaker->level_w((data & 1) | (data >> 6 & 2) | (m_r >> 8 & 4));
	m_o = data;
}

READ8_MEMBER(bigtrak_state::read_k)
{
	// K: multiplexed inputs
	// K8: IR sensor
	return read_inputs(7) | (sensor_state() ? 8 : 0);
}


// config

/* physical button layout and labels is like this:

        USA version:                          UK version:

           [^]           [CLR]                   [^]           [CM]
    [<]    [HOLD] [>]    [FIRE]           [<]    [P]    [>]    [\|/]
           [v]           [CLS]                   [v]           [CE]
    [7]    [8]    [9]    [RPT]            [7]    [8]    [9]    [x2]
    [4]    [5]    [6]    [TEST]           [4]    [5]    [6]    [TEST]
    [1]    [2]    [3]    [CK]             [1]    [2]    [3]    [checkmark]
    [IN]   [0]    [OUT]  [GO]                    [0]    [OUT]  [GO]
*/

static INPUT_PORTS_START( bigtrak )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Hold")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Forward")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Fire")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear Memory")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Backward")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear Last Step")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Test")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Check")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("In")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Out")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Go")

	PORT_START("IN.6") // R8
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CODE(KEYCODE_F1) PORT_NAME("Input Port")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void bigtrak_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill/register for savestates
	m_gearbox_pos = 0;
	save_item(NAME(m_gearbox_pos));
}

static const INT16 bigtrak_speaker_levels[] = { 0, 32767/3, 32767/3, 32767/3*2, 32767/3, 32767/3*2, 32767/3*2, 32767 };

static MACHINE_CONFIG_START( bigtrak, bigtrak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 200000) // approximation - RC osc. R=83K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(bigtrak_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(bigtrak_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(bigtrak_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("gearbox", bigtrak_state, gearbox_sim_tick, attotime::from_msec(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_bigtrak)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(8, bigtrak_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Code Name: Sector, by Bob Doyle
  * TMS0970 MCU, MP0905BNL ZA0379 (die labeled 0970F-05B)
  * 6-digit 7seg LED display + 4 LEDs for compass, no sound

  This is a tabletop submarine pursuit game. A grid board and small toy
  boats are used to remember your locations (a Paint app should be ok too).
  Refer to the official manual for more information, it is not a simple game.

***************************************************************************/

class cnsector_state : public hh_tms1k_state
{
public:
	cnsector_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(cnsector_state::write_r)
{
	// R0-R5: select digit (right-to-left)
	for (int y = 0; y < 6; y++)
	{
		m_display_segmask[y] = 0xff;
		m_display_state[y] = (data >> y & 1) ? m_o : 0;
	}

	// R6-R9: direction leds (-> lamp60-63)
	m_display_state[6] = data >> 6 & 0xf;

	set_display_size(8, 7);
	display_update();
}

WRITE16_MEMBER(cnsector_state::write_o)
{
	// O0-O4: input mux
	m_inp_mux = data & 0x1f;

	// O0-O7: digit segments
	m_o = data;
}

READ8_MEMBER(cnsector_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}


// config

/* physical button layout and labels is like this:

             COMBAT INFORMATION CENTER
    [NEXT SHIP]       [RECALL]    [MOVE SHIP]

    [LEFT]   [RIGHT]      o       [SLOWER] [FASTER]
        STEERING     EVASIVE SUB        SPEED            o (on/off switch)
              NAVIGATIONAL PROGRAMMING                   |

    [RANGE]  [AIM]    [FIRE]        o      [TEACH MODE]
      SONAR CONTROL            SUB FINDER
*/

static INPUT_PORTS_START( cnsector )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Next Ship")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Range")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Aim")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Evasive Sub") // expert button
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Recall")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Sub Finder") // expert button
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Slower")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Teach Mode")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Faster")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Move Ship")
INPUT_PORTS_END

static MACHINE_CONFIG_START( cnsector, cnsector_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(cnsector_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(cnsector_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(cnsector_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_cnsector)

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Parker Bros Merlin handheld game, by Bob Doyle
  * TMS1100NLL MP3404A-N2
  * 11 LEDs behind buttons, 3-level sound

  Also published in Japan by Tomy as "Dr. Smith", white case instead of red.
  The one with dark-blue case is the rare sequel Master Merlin, see below.
  More sequels followed too, but on other hardware.

  To start a game, press NEW GAME, followed by a number:
  1: Tic-Tac-Toe
  2: Music Machine
  3: Echo
  4: Blackjack 13
  5: Magic Square
  6: Mindbender

  Refer to the official manual for more information on the games.

***************************************************************************/

class merlin_state : public hh_tms1k_state
{
public:
	merlin_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(merlin_state::write_r)
{
	/* leds:

	     R0
	R1   R2   R3
	R4   R5   R6
	R7   R8   R9
	     R10
	*/
	display_matrix(11, 1, data, 1);
}

WRITE16_MEMBER(merlin_state::write_o)
{
	// O4-O6(tied together): speaker out
	m_speaker->level_w(data >> 4 & 7);

	// O0-O3: input mux
	// O7: N/C
	m_inp_mux = data & 0xf;
}

READ8_MEMBER(merlin_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}


// config

static INPUT_PORTS_START( merlin )
	PORT_START("IN.0") // O0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Button 0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Button 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Button 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Button 2")

	PORT_START("IN.1") // O1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Button 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Button 7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Button 6")

	PORT_START("IN.2") // O2
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Button 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Button 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Same Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Button 10")

	PORT_START("IN.3") // O3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Comp Turn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hit Me")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("New Game")
INPUT_PORTS_END


static const INT16 merlin_speaker_levels[] = { 0, 32767/3, 32767/3, 32767/3*2, 32767/3, 32767/3*2, 32767/3*2, 32767 };

static MACHINE_CONFIG_START( merlin, merlin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(merlin_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(merlin_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(merlin_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_merlin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(8, merlin_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Master Merlin
  * TMS1400 MP7351-N2LL (die labeled 1400CR MP7351)
  * 11 LEDs behind buttons, 3-level sound

  The TMS1400CR MCU has the same pinout as a standard TMS1100. The hardware
  outside of the MCU is exactly the same as Merlin.

  The included minigames are:
  1: Three Shells
  2: Hi/Lo
  3: Match It
  4: Hit or Miss
  5: Pair Off
  6: Tempo
  7: Musical Ladder
  8: Patterns
  9: Hot Potato

***************************************************************************/

class mmerlin_state : public merlin_state
{
public:
	mmerlin_state(const machine_config &mconfig, device_type type, const char *tag)
		: merlin_state(mconfig, type, tag)
	{ }
};

// handlers: uses the ones in merlin_state


// config

static INPUT_PORTS_START( mmerlin )
	PORT_INCLUDE( merlin )

	PORT_MODIFY("IN.3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Score") // instead of Hit Me
INPUT_PORTS_END

static MACHINE_CONFIG_START( mmerlin, mmerlin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 425000) // approximation - RC osc. R=30K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(mmerlin_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mmerlin_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mmerlin_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mmerlin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(8, merlin_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Stop Thief, by Bob Doyle
  * TMS0980NLL MP6101B (die labeled 0980B-01A)
  * 3-digit 7seg LED display, 1bit sound

  Stop Thief is actually a board game, the electronic device emulated here
  (called Electronic Crime Scanner) is an accessory. To start a game, press
  the ON button. Otherwise, it is in test-mode where you can hear all sounds.

***************************************************************************/

class stopthief_state : public hh_tms1k_state
{
public:
	stopthief_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(stopthief_state::write_r)
{
	// R0-R2: select digit
	UINT8 o = BITSWAP8(m_o,3,5,2,1,4,0,6,7) & 0x7f;
	for (int y = 0; y < 3; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (data >> y & 1) ? o : 0;
	}

	set_display_size(7, 3);
	display_update();

	// R3-R8: speaker on
	m_speaker->level_w((data & 0x1f8 && m_o & 8) ? 1 : 0);
}

WRITE16_MEMBER(stopthief_state::write_o)
{
	// O0,O6: input mux
	m_inp_mux = (data & 1) | (data >> 5 & 2);

	// O3: speaker out
	// O0-O2,O4-O7: led segments A-G
	m_o = data;
}

READ8_MEMBER(stopthief_state::read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[2]->read() | read_inputs(2);
}


// config

/* physical button layout and labels is like this:

    [1] [2] [OFF]
    [3] [4] [ON]
    [5] [6] [T, TIP]
    [7] [8] [A, ARREST]
    [9] [0] [C, CLUE]
*/

static INPUT_PORTS_START( stopthief )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.1") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.2") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Tip")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Arrest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Clue")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, (void *)false)
INPUT_PORTS_END

static MACHINE_CONFIG_START( stopthief, stopthief_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, 425000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(stopthief_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(stopthief_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(stopthief_state, write_o))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(hh_tms1k_state, auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_stopthie)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Bank Shot (known as Cue Ball in the UK), by Garry Kitchen
  * TMS1400NLL MP7313-N2 (die labeled MP7313)
  * LED grid display, 1bit sound

  Bank Shot is an electronic pool game. To select a game, repeatedly press
  the [SELECT] button, then press [CUE UP] to start. Refer to the official
  manual for more information. The game selections are:
  1: Straight Pool (1 player)
  2: Straight Pool (2 players)
  3: Poison Pool
  4: Trick Shots

***************************************************************************/

class bankshot_state : public hh_tms1k_state
{
public:
	bankshot_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(bankshot_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R2,R3: input mux
	m_inp_mux = data >> 2 & 3;

	// R2-R10: led select
	m_r = data & ~3;
	display_matrix(7, 11, m_o, m_r);
}

WRITE16_MEMBER(bankshot_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	display_matrix(7, 11, m_o, m_r);
}

READ8_MEMBER(bankshot_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(2);
}


// config

/* physical button layout and labels is like this:
  (note: remember that you can rotate the display in MAME)

    [SELECT  [BALL UP] [BALL OVER]
     SCORE]

    ------  led display  ------

    [ANGLE]  [AIM]     [CUE UP
                        SHOOT]
*/

static INPUT_PORTS_START( bankshot )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Angle")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Aim")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Cue Up/Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Select/Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ball Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Ball Over")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bankshot, bankshot_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 475000) // approximation - RC osc. R=24K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(bankshot_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(bankshot_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(bankshot_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_bankshot)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Split Second
  * TMS1400NLL MP7314-N2 (die labeled MP7314)
  * LED grid display(default round LEDs, and rectangular shape ones), 1bit sound

  This is an electronic handheld reflex gaming device, it's straightforward
  to use. The included mini-games are:
  1, 2, 3: Mad Maze*
  4, 5: Space Attack*
  6: Auto Cross
  7: Stomp
  8: Speedball

  *: higher number indicates higher difficulty

  display layout, where number xy is lamp R(x),O(y)

       00    02    04
    10 01 12 03 14 05 16
       11    13    15
    20 21 22 23 24 25 26
       31    33    35
    30 41 32 43 34 45 36
       51    53    55
    40 61 42 63 44 65 46
       71    73    75
    50 60 52 62 54 64 56
       70    72    74

***************************************************************************/

class splitsec_state : public hh_tms1k_state
{
public:
	splitsec_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

WRITE16_MEMBER(splitsec_state::write_r)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0-R7: led select
	m_r = data;
	display_matrix(7, 8, m_o, m_r);
}

WRITE16_MEMBER(splitsec_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	display_matrix(7, 8, m_o, m_r);
}

READ8_MEMBER(splitsec_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(2);
}


// config

static INPUT_PORTS_START( splitsec )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( splitsec, splitsec_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 475000) // approximation - RC osc. R=24K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(splitsec_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(splitsec_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(splitsec_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_splitsec)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tandy Radio Shack Computerized Arcade (1981, 1982, 1995)
  * TMS1100 MCU, labeled CD7282SL
  * 12 lamps behind buttons, 1bit sound

  This handheld contains 12 minigames. It looks and plays like "Fabulous Fred"
  by the Japanese company Mego Corp. in 1980, which in turn is a mix of Merlin
  and Simon. Unlike Merlin and Simon, spin-offs like these were not successful.
  There were releases with and without the prefix "Tandy-12", I don't know
  which name was more common. Also not worth noting is that it needed five
  batteries; 4 C-cells and a 9-volt.

  Some of the games require accessories included with the toy (eg. the Baseball
  game is played with a board representing the playing field). To start a game,
  hold the [SELECT] button, then press [START] when the game button lights up.
  As always, refer to the official manual for more information.

  See below at the input defs for a list of the games.

***************************************************************************/

class tandy12_state : public hh_tms1k_state
{
public:
	tandy12_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};

// handlers

void tandy12_state::prepare_display()
{
	// O0-O7: button lamps 1-8, R0-R3: button lamps 9-12
	display_matrix(13, 1, (m_o << 1 & 0x1fe) | (m_r << 9 & 0x1e00), 1);
}

WRITE16_MEMBER(tandy12_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R9: input mux
	m_inp_mux = data >> 5 & 0x1f;

	// other bits:
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(tandy12_state::write_o)
{
	m_o = data;
	prepare_display();
}

READ8_MEMBER(tandy12_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}


// config

/* physical button layout and labels is like this:

        REPEAT-2              SPACE-2
          [O]     OFF--ON       [O]

    [purple]1     [blue]5       [l-green]9
    ORGAN         TAG-IT        TREASURE HUNT

    [l-orange]2   [turquoise]6  [red]10
    SONG WRITER   ROULETTE      COMPETE

    [pink]3       [yellow]7     [violet]11
    REPEAT        BASEBALL      FIRE AWAY

    [green]4      [orange]8     [brown]12
    TORPEDO       REPEAT PLUS   HIDE 'N SEEK

          [O]        [O]        [O]
         START      SELECT    PLAY-2/HIT-7
*/

static INPUT_PORTS_START( tandy12 )
	PORT_START("IN.0") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Button 12")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Button 11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Button 10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Button 9")

	PORT_START("IN.1") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Space-2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Play-2/Hit-7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Start")

	PORT_START("IN.2") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat-2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Button 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Button 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Button 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Button 1")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Button 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Button 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Button 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Button 5")
INPUT_PORTS_END


static const UINT16 tandy12_output_pla[0x20] =
{
	// these are certain
	0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
	0x80, 0x00, 0x00, 0x00, 0x00,

	// rest is unused?
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static MACHINE_CONFIG_START( tandy12, tandy12_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 400000) // RC osc. R=39K, C=47pf -> ~400kHz
	MCFG_TMS1XXX_OUTPUT_PLA(tandy12_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(tandy12_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tandy12_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tandy12_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tandy12)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy(tronics) Break Up (manufactured in Japan)
  * PCB label TOMY B.O.
  * TMS1040 MP2726 TOMY WIPE (die labeled MP2726A)
  * TMS1025N2LL I/O expander
  * 2-digit 7seg display, 46 other leds, 1bit sound

  known releases:
  - USA: Break Up
  - Japan: Block Attack
  - UK: Break-In

  lamp translation table: led zz from game PCB = MAME lampyx:

    00 = -         10 = lamp50    20 = lamp42
    01 = lamp70    11 = lamp51    21 = lamp33
    02 = lamp71    12 = lamp52    22 = lamp22
    03 = lamp72    13 = lamp53
    04 = lamp73    14 = lamp43
    05 = lamp60    15 = lamp31
    06 = lamp61    16 = lamp32
    07 = lamp62    17 = lamp30
    08 = lamp63    18 = lamp41
    09 = lamp40    19 = lamp21

  the 7seg panel is lamp0x and lamp1x(aka digit0/1), and the
  8(2*4) * 3 rectangular leds panel, where x=0,1,2,3:

    lamp9x         lamp11x
    lamp8x         lamp13x
    lamp10x        lamp12x

***************************************************************************/

class tbreakup_state : public hh_tms1k_state
{
public:
	tbreakup_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_expander(*this, "expander")
	{ }

	required_device<tms1024_device> m_expander;
	UINT8 m_exp_port[7];
	DECLARE_WRITE8_MEMBER(expander_w);

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;
};

// handlers

void tbreakup_state::prepare_display()
{
	// 7seg leds from R0,R1 and O0-O6
	for (int y = 0; y < 2; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (m_r >> y & 1) ? (m_o & 0x7f) : 0;
	}

	// 22 round leds from O2-O7 and expander port 7
	for (int y = 2; y < 8; y++)
		m_display_state[y] = (m_o >> y & 1) ? m_exp_port[6] : 0;

	// 24 rectangular leds from expander ports 1-6 (not strobed)
	for (int y = 0; y < 6; y++)
		m_display_state[y+8] = m_exp_port[y];

	set_display_size(8, 14);
	display_update();
}

WRITE8_MEMBER(tbreakup_state::expander_w)
{
	// TMS1025 port 1-7 data
	m_exp_port[offset] = data;
}

WRITE16_MEMBER(tbreakup_state::write_r)
{
	// R6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// R7,R8: input mux
	m_inp_mux = data >> 7 & 3;

	// R3-R5: TMS1025 port S
	// R2: TMS1025 STD pin
	m_expander->write_s(space, 0, data >> 3 & 7);
	m_expander->write_std(data >> 2 & 1);

	// R0,R1: select digit
	m_r = ~data;
	prepare_display();
}

WRITE16_MEMBER(tbreakup_state::write_o)
{
	// O0-O3: TMS1025 port H
	m_expander->write_h(space, 0, data & 0xf);

	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(tbreakup_state::read_k)
{
	// K4: fixed input
	// K8: multiplexed inputs
	return (m_inp_matrix[2]->read() & 4) | (read_inputs(2) & 8);
}


// config

static INPUT_PORTS_START( tbreakup )
	PORT_START("IN.0") // R7 K8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Ball")

	PORT_START("IN.1") // R8 K8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hit")

	PORT_START("IN.2") // K4
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_CONFSETTING(    0x00, "3" )
	PORT_CONFSETTING(    0x04, "5" )

	PORT_START("IN.3") // fake
	PORT_CONFNAME( 0x01, 0x00, "Skill Level" ) PORT_CHANGED_MEMBER(DEVICE_SELF, tbreakup_state, skill_switch, NULL)
	PORT_CONFSETTING(    0x00, "Pro 1" )
	PORT_CONFSETTING(    0x01, "Pro 2" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tbreakup_state::skill_switch)
{
	set_clock();
}


void tbreakup_state::set_clock()
{
	// MCU clock is from an analog circuit with resistor of 73K, PRO2 adds 100K
	m_maincpu->set_unscaled_clock((m_inp_matrix[3]->read() & 1) ? 500000 : 325000);
}

void tbreakup_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void tbreakup_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill/register for savestates
	memset(m_exp_port, 0, sizeof(m_exp_port));
	save_item(NAME(m_exp_port));
}

static MACHINE_CONFIG_START( tbreakup, tbreakup_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1040, 325000) // see set_clock
	MCFG_TMS1XXX_READ_K_CB(READ8(tbreakup_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tbreakup_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tbreakup_state, write_o))

	MCFG_DEVICE_ADD("expander", TMS1025, 0)
	MCFG_TMS1024_WRITE_PORT_CB(1, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(2, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(3, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(4, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(5, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(6, WRITE8(tbreakup_state, expander_w))
	MCFG_TMS1024_WRITE_PORT_CB(7, WRITE8(tbreakup_state, expander_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tbreakup)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tomy Power House Pinball
  * PCB label TOMY P-B
  * TMS1100 MP1180 TOMY PINB (die labeled MP1180)
  * 3 7seg LEDs, and other LEDs behind bezel, 1bit sound

  known releases:
  - USA: Power House Pinball
  - Japan: Pinball
  - Europe: Flipper

  lamp translation table: led zz from game PCB = MAME lampyx:

    0 = -          10 = lamp50     20 = lamp64     A = lamp30
    1 = lamp33     11 = lamp55     21 = lamp65     B = lamp34
    2 = lamp31     12 = lamp51     22 = lamp70     C = lamp35
    3 = lamp32     13 = lamp52     23 = lamp71     D = lamp80
    4 = lamp40     14 = lamp53     24 = lamp72     E = lamp81
    5 = lamp41     15 = lamp60     25 = lamp73     F = lamp82
    6 = lamp42     16 = lamp54     26 = lamp74     G = lamp83
    7 = lamp43     17 = lamp61
    8 = lamp44     18 = lamp62
    9 = lamp45     19 = lamp63

  NOTE!: MAME external artwork is required

***************************************************************************/

class phpball_state : public hh_tms1k_state
{
public:
	phpball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	DECLARE_INPUT_CHANGED_MEMBER(flipper_button);
};

// handlers

void phpball_state::prepare_display()
{
	// R0-R2 are 7segs
	for (int y = 0; y < 3; y++)
		m_display_segmask[y] = 0x7f;

	display_matrix(7, 9, m_o, m_r);
}

WRITE16_MEMBER(phpball_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R9: input mux
	m_inp_mux = data >> 9 & 1;

	// R0-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(phpball_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(phpball_state::read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[1]->read() | read_inputs(1);
}


// config

static INPUT_PORTS_START( phpball )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Plunger")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // Vss!
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, (void *)1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, (void *)0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(phpball_state::flipper_button)
{
	// rectangular LEDs under LEDs D,F and E,G are directly connected
	// to the left and right flipper buttons - output them to lamp90 and 91
	output_set_lamp_value(90 + (int)(FPTR)param, newval);
}


static MACHINE_CONFIG_START( phpball, phpball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 375000) // RC osc. R=47K, C=47pf -> ~375kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(phpball_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(phpball_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(phpball_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_hh_tms1k_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mathmagi )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1030", 0x0000, 0x0800, CRC(a81d7ccb) SHA1(4756ce42f1ea28ce5fe6498312f8306f10370969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mathmagi_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( amaztron )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3405", 0x0000, 0x0800, CRC(9cbc0009) SHA1(17772681271b59280687492f37fa0859998f041d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_amaztron_output.pla", 0, 365, CRC(f3875384) SHA1(3c256a3db4f0aa9d93cf78124db39f4cbdc57e4a) )
ROM_END


ROM_START( h2hbaseb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1525", 0x0000, 0x0800, CRC(b5d6bf9b) SHA1(2cc9f35f077c1209c46d16ec853af87e4725c2fd) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hbaseb_output.pla", 0, 365, CRC(cb3d7e38) SHA1(6ab4a7c52e6010b7c7158463cb499973e52ff556) )
ROM_END


ROM_START( h2hfootb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3460.u3", 0x0000, 0x0800, CRC(3a4e53a8) SHA1(5052e706f992c6c4bada1fa7769589eec3df6471) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hfootb_output.pla", 0, 365, CRC(c8d85873) SHA1(16bd6fc8e3cd16d5f8fd32d0c74e67de77f5487e) )
ROM_END


ROM_START( tc4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7334", 0x0000, 0x1000, CRC(923f3821) SHA1(a9ae342d7ff8dae1dedcd1e4984bcfae68586581) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_tc4_output.pla", 0, 557, CRC(3b908725) SHA1(f83bf5faa5b3cb51f87adc1639b00d6f9a71ad19) )
ROM_END


ROM_START( ebball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0914", 0x0000, 0x0400, CRC(3c6fb05b) SHA1(b2fe4b3ca72d6b4c9bfa84d67f64afdc215e7178) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball_output.pla", 0, 365, CRC(062bf5bb) SHA1(8d73ee35444299595961225528b153e3a5fe66bf) )
ROM_END


ROM_START( ebball2 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0923", 0x0000, 0x0400, CRC(077acfe2) SHA1(a294ce7614b2cdb01c754a7a50d60d807e3f0939) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball2_output.pla", 0, 365, CRC(adcd73d1) SHA1(d69e590d288ef99293d86716498f3971528e30de) )
ROM_END


ROM_START( ebball3 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "6007_mp1204", 0x0000, 0x0800, CRC(987a29ba) SHA1(9481ae244152187d85349d1a08e439e798182938) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ebball3_output.pla", 0, 365, CRC(00db663b) SHA1(6eae12503364cfb1f863df0e57970d3e766ec165) )
ROM_END


ROM_START( einvader )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1211", 0x0000, 0x0800, CRC(b6efbe8e) SHA1(d7d54921dab22bb0c2956c896a5d5b56b6f64969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_einvader_output.pla", 0, 365, CRC(490158e1) SHA1(61cace1eb09244663de98d8fb04d9459b19668fd) )
ROM_END


ROM_START( efootb4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "6009_mp7551", 0x0000, 0x1000, CRC(54fa7244) SHA1(4d16bd825c4a2db76ca8a263c373ade15c20e270) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_efootb4_output.pla", 0, 557, CRC(5c87c753) SHA1(bde9d4aa1e57a718affd969475c0a1edcf60f444) )
ROM_END


ROM_START( ebaskb2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "6010_mp1218", 0x0000, 0x0800, CRC(0089ede8) SHA1(c8a79d5aca7e37b637a4d152150acba9f41aad96) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ebaskb2_output.pla", 0, 365, CRC(c18103ae) SHA1(5a9bb8e1d95a9f6919b05ff9471fa0a8014b8b81) )
ROM_END


ROM_START( raisedvl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1221", 0x0000, 0x0800, CRC(782791cc) SHA1(214249406fcaf44efc6350022bd534e59ec69c88) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_raisedvl_output.pla", 0, 365, CRC(00db663b) SHA1(6eae12503364cfb1f863df0e57970d3e766ec165) )
ROM_END


ROM_START( gpoker )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2105", 0x0000, 0x0800, CRC(95a8f5b4) SHA1(d14f00ba9f57e437264d972baa14a14a28ff8719) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_gpoker_output.pla", 0, 365, CRC(f7e2d812) SHA1(cc3abd89afb1d2145dc47636553ccd0ba7de70d9) )
ROM_END


ROM_START( gjackpot )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mpf553", 0x0000, 0x1000, CRC(f45fd008) SHA1(8d5d6407a8a031a833ceedfb931f5c9d2725ecd0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_gjackpot_output.pla", 0, 557, CRC(50e471a7) SHA1(9d862cb9f51a563882b62662c5bfe61b52e3df00) )
ROM_END


ROM_START( elecdet )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6100a", 0x0000, 0x1000, CRC(6f396bb8) SHA1(1f104d4ca9bee0d4572be4779b7551dfe20c4f04) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_elecdet_output.pla", 0, 352, CRC(652d19c3) SHA1(75550c2b293453b6b9efed88c8cc77195a53161f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


ROM_START( starwbc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3438a", 0x0000, 0x0800, CRC(c12b7069) SHA1(d1f39c69a543c128023ba11cc6228bacdfab04de) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_output.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END

ROM_START( starwbcp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "us4270755", 0x0000, 0x0800, BAD_DUMP CRC(fb3332f2) SHA1(a79ac81e239983cd699b7cfcc55f89b203b2c9ec) ) // from patent US4270755, may have errors

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_output.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END


ROM_START( astro )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp1133", 0x0000, 0x1000, CRC(bc21109c) SHA1(05a433cce587d5c0c2d28b5fda5f0853ea6726bf) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_astro_output.pla", 0, 557, CRC(eb08957e) SHA1(62ae0d13a1eaafb34f1b27d7df51441b400ccd56) )
ROM_END


ROM_START( mdndclab )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34012", 0x0000, 0x0800, CRC(e851fccd) SHA1(158362c2821678a51554e02dbb2f9ef5aaf5f59f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mdndclab_output.pla", 0, 365, CRC(592b40ba) SHA1(63a2531278a665ace54c541101e052eb84413511) )
ROM_END


ROM_START( comp4 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0904nl_cp0904a", 0x0000, 0x0400, CRC(6233ee1b) SHA1(738e109b38c97804b4ec52bed80b00a8634ad453) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common2_instr.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_comp4_micro.pla", 0, 860, CRC(ee9d7d9e) SHA1(25484e18f6a07f7cdb21a07220e2f2a82fadfe7b) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_comp4_output.pla", 0, 352, CRC(a0f887d1) SHA1(3c666663d484d5bed81e1014f8715aab8a3d489f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_comp4_segment.pla", 0, 157, CRC(e5bddd90) SHA1(4b1c6512c70e5bcd23c2dbf0c88cd8aa2c632a10) )
ROM_END


ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_simon_micro.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1000_simon_output.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END


ROM_START( ssimon )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3476", 0x0000, 0x0800, CRC(98200571) SHA1(cbd0bcfc11a534aa0be5d011584cdcac58ff437a) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1100_ssimon_output.pla", 0, 365, CRC(0fea09b0) SHA1(27a56fcf2b490e9a7dbbc6ad48cc8aaca4cada94) )
ROM_END


ROM_START( bigtrak )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3301a", 0x0000, 0x0400, CRC(1351bcdd) SHA1(68865389c25b541c09a742be61f8fb6488134d4e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_bigtrak_micro.pla", 0, 867, CRC(80912d0a) SHA1(7ae5293ed4d93f5b7a64d43fe30c3639f39fbe5a) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bigtrak_output.pla", 0, 365, CRC(63be45f6) SHA1(918e38a223152db883c1a6f7acf56e87d7074734) )
ROM_END


ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(201036e9) SHA1(b37fef86bb2bceaf0ac8bb3745b4702d17366914) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common2_instr.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_micro.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_cnsector_output.pla", 0, 352, CRC(7c0bdcd6) SHA1(dade774097e8095dca5deac7b2367d0c701aca51) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_common2_segment.pla", 0, 157, CRC(56c37a4f) SHA1(18ecc20d2666e89673739056483aed5a261ae927) )
ROM_END


ROM_START( merlin )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3404", 0x0000, 0x0800, CRC(7515a75d) SHA1(76ca3605d3fde1df62f79b9bb1f534c2a2ae0229) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_merlin_output.pla", 0, 365, CRC(3921b074) SHA1(12bd58e4d6676eb8c7059ef53598279e4f1a32ea) )
ROM_END


ROM_START( mmerlin )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7351", 0x0000, 0x1000, CRC(0f7a4c83) SHA1(242c1278ddfe92c28fd7cd87300e48e7a4827831) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mmerlin_output.pla", 0, 557, CRC(fd3dcd93) SHA1(f2afc52df700daa0eb7356c7876af9b2966f971b) )
ROM_END


ROM_START( stopthie )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6101b", 0x0000, 0x1000, CRC(8bde5bb4) SHA1(8c318fcce67acc24c7ae361f575f28ec6f94665a) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_output.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( stopthiep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "us4341385", 0x0000, 0x1000, CRC(07aec38a) SHA1(0a3d0956495c0d6d9ea771feae6c14a473a800dc) ) // from patent US4341385, data should be correct (it included checksums)

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_output.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


ROM_START( bankshot )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7313", 0x0000, 0x1000, CRC(7a5016a9) SHA1(a8730dc8a282ffaa3d89e675f371d43eb39f39b4) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_bankshot_output.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


ROM_START( splitsec )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7314", 0x0000, 0x1000, CRC(e94b2098) SHA1(f0fc1f56a829252185592a2508740354c50bedf8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_splitsec_output.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


ROM_START( tandy12 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x0800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tandy12_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( tbreakup )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp2726a", 0x0000, 0x0400, CRC(1f7c28e2) SHA1(164cda4eb3f0b1d20955212a197c9aadf8d18a06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_tbreakup_output.pla", 0, 365, CRC(a1ea035e) SHA1(fcf0b57ed90b41441a8974223a697f530daac0ab) )
ROM_END


ROM_START( phpball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1180", 0x0000, 0x0800, CRC(2163b92d) SHA1(bc53d1911e88b4e89d951c6f769703105c13389c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_phpball_output.pla", 0, 365, CRC(87e67aaf) SHA1(ebc7bae1352f39173f1bf0dc10cdc6f635dedab4) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1980, mathmagi,  0,        0, mathmagi,  mathmagi,  driver_device, 0, "APF Electronics Inc.", "Mathemagician", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1979, amaztron,  0,        0, amaztron,  amaztron,  driver_device, 0, "Coleco", "Amaze-A-Tron", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hbaseb,  0,        0, h2hbaseb,  h2hbaseb,  driver_device, 0, "Coleco", "Head to Head Baseball", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hfootb,  0,        0, h2hfootb,  h2hfootb,  driver_device, 0, "Coleco", "Head to Head Football", MACHINE_SUPPORTS_SAVE )
CONS( 1981, tc4,       0,        0, tc4,       tc4,       driver_device, 0, "Coleco", "Total Control 4", MACHINE_SUPPORTS_SAVE )

CONS( 1979, ebball,    0,        0, ebball,    ebball,    driver_device, 0, "Entex", "Electronic Baseball (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ebball2,   0,        0, ebball2,   ebball2,   driver_device, 0, "Entex", "Electronic Baseball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebball3,   0,        0, ebball3,   ebball3,   driver_device, 0, "Entex", "Electronic Baseball 3 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, einvader,  0,        0, einvader,  einvader,  driver_device, 0, "Entex", "Space Invader (Entex, TMS1100)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, efootb4 ,  0,        0, efootb4,   efootb4,   driver_device, 0, "Entex", "Color Football 4 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebaskb2 ,  0,        0, ebaskb2,   ebaskb2,   driver_device, 0, "Entex", "Electronic Basketball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, raisedvl,  0,        0, raisedvl,  raisedvl,  driver_device, 0, "Entex", "Raise The Devil", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, gpoker,    0,        0, gpoker,    gpoker,    driver_device, 0, "Gakken", "Poker (Gakken, 1979 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gjackpot,  0,        0, gjackpot,  gjackpot,  driver_device, 0, "Gakken", "Jackpot: Gin Rummy & Black Jack", MACHINE_SUPPORTS_SAVE )

CONS( 1979, elecdet,   0,        0, elecdet,   elecdet,   driver_device, 0, "Ideal", "Electronic Detective", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1979, starwbc,   0,        0, starwbc,   starwbc,   driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command", MACHINE_SUPPORTS_SAVE )
CONS( 1979, starwbcp,  starwbc,  0, starwbc,   starwbc,   driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command (patent)", MACHINE_SUPPORTS_SAVE )

COMP( 1979, astro,     0,        0, astro,     astro,     driver_device, 0, "Kosmos", "Astro", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1980, mdndclab,  0,        0, mdndclab,  mdndclab,  driver_device, 0, "Mattel", "Dungeons & Dragons - Computer Labyrinth Game", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1977, comp4,     0,        0, comp4,     comp4,     driver_device, 0, "Milton Bradley", "Comp IV", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
CONS( 1978, simon,     0,        0, simon,     simon,     driver_device, 0, "Milton Bradley", "Simon (Rev. A)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ssimon,    0,        0, ssimon,    ssimon,    driver_device, 0, "Milton Bradley", "Super Simon", MACHINE_SUPPORTS_SAVE )
CONS( 1979, bigtrak,   0,        0, bigtrak,   bigtrak,   driver_device, 0, "Milton Bradley", "Big Trak", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL ) // ***

CONS( 1977, cnsector,  0,        0, cnsector,  cnsector,  driver_device, 0, "Parker Brothers", "Code Name: Sector", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW ) // ***
CONS( 1978, merlin,    0,        0, merlin,    merlin,    driver_device, 0, "Parker Brothers", "Merlin - The Electronic Wizard", MACHINE_SUPPORTS_SAVE )
CONS( 1979, stopthie,  0,        0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner)", MACHINE_SUPPORTS_SAVE ) // ***
CONS( 1979, stopthiep, stopthie, 0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner) (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1980, bankshot,  0,        0, bankshot,  bankshot,  driver_device, 0, "Parker Brothers", "Bank Shot - Electronic Pool", MACHINE_SUPPORTS_SAVE )
CONS( 1980, splitsec,  0,        0, splitsec,  splitsec,  driver_device, 0, "Parker Brothers", "Split Second", MACHINE_SUPPORTS_SAVE )
CONS( 1982, mmerlin,   0,        0, mmerlin,   mmerlin,   driver_device, 0, "Parker Brothers", "Master Merlin", MACHINE_SUPPORTS_SAVE )

CONS( 1981, tandy12,   0,        0, tandy12,   tandy12,   driver_device, 0, "Tandy Radio Shack", "Tandy-12: Computerized Arcade", MACHINE_SUPPORTS_SAVE ) // some of the minigames: ***

CONS( 1979, tbreakup,  0,        0, tbreakup,  tbreakup,  driver_device, 0, "Tomy", "Break Up (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, phpball,   0,        0, phpball,   phpball,   driver_device, 0, "Tomy", "Power House Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, playing cards, pen & paper, etc.
