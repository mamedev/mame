// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  This driver is a collection of simple dedicated handheld and tabletop
  toys based around the TMS1000 MCU series. Anything more complex or clearly
  part of a series is (or will be) in its own driver.
  
  Let's use this driver for a list of known devices and their serials,
  excluding TI's own products (see for example ticalc1x.c, tispeak.c)

  serial   device   etc.
--------------------------------------------------------------------
 @CP0904A  TMS0970  1977, Milton Bradley Comp IV
 @MP0905B  TMS0970  1977, Parker Brothers Codename Sector
 @MP0914   TMS1000  1978, Entex Baseball 1
 @MP1030   TMS1100  1980, APF Mathemagician
 @MP3226   TMS1000  1978, Milton Bradley Simon
 @MP3403   TMS1100  1978, Marx Electronic Bowling
 @MP3404   TMS1100  1978, Parker Brothers Merlin
 @MP3405   TMS1100  1979, Coleco Amaze-A-Tron
 @MP3438A  TMS1100  1979, Kenner Star Wars Electronic Battle Command
  MP3450A  TMS1100  1979, MicroVision cartridge: Blockbuster
  MP3454   TMS1100  1979, MicroVision cartridge: Star Trek Phaser Strike
  MP3455   TMS1100  1980, MicroVision cartridge: Pinball
  MP3457   TMS1100  1979, MicroVision cartridge: Mindbuster
  MP3474   TMS1100  1979, MicroVision cartridge: Vegas Slots
  MP3475   TMS1100  1979, MicroVision cartridge: Bowling
  MP3479   TMS1100  1980, MicroVision cartridge: Baseball
  MP3481   TMS1100  1979, MicroVision cartridge: Connect Four
  MP3496   TMS1100  1980, MicroVision cartridge: Sea Duel
 @MP6100A  TMS0980  1979, Ideal Electronic Detective
 @MP6101B  TMS0980  1979, Parker Brothers Stop Thief
 @MP7313   TMS1400  1980, Parker Brothers Bank Shot
 @MP7314   TMS1400  1980, Parker Brothers Split Second
 *MP7332   TMS1400  1981, Milton Bradley Dark Tower
 @MP7334   TMS1400  1981, Coleco Total Control 4

  inconsistent:
  
  M34009   TMS1100  1981, MicroVision cartridge: Alien Raiders
  M34017   TMS1100  1981, MicroVision cartridge: Cosmic Hunter
  M34047   TMS1100  1982, MicroVision cartridge: Super Blockbuster

  CD7282SL TMS1100  1981, Tandy-12 (serial is similar to TI Speak & Spell series?)

  (* denotes not yet emulated by MESS, @ denotes it's in this driver)


  TODO:
  - verify output PLA and microinstructions PLA for MCUs that have been dumped
    electronically (mpla is usually the default, opla is often custom)
  - unknown MCU clocks for some: TMS1000 and TMS1100 RC curve is documented in
    the data manual, but for TMS1400 it's unknown. TMS0970/0980 osc. is on-die.
  - some of the games rely on the fact that faster(longer) strobed leds appear
    brighter: tc4(offensive players), bankshot(cue ball)
  - add softwarelist for tc4 cartridges?
  - stopthiep: unable to start a game (may be intentional?)

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

// internal artwork
#include "amaztron.lh"
#include "bankshot.lh"
#include "cnsector.lh"
#include "ebball.lh"
#include "elecbowl.lh"
#include "elecdet.lh"
#include "comp4.lh"
#include "mathmagi.lh"
#include "merlin.lh" // clickable
#include "simon.lh" // clickable
#include "splitsec.lh"
#include "starwbc.lh"
#include "stopthie.lh"
#include "tandy12.lh" // clickable
#include "tc4.lh"


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;
	
	// misc common
	UINT16 m_r;
	UINT16 m_o;
	UINT16 m_inp_mux;
	bool m_power_on;

	UINT8 read_inputs(int columns);
	DECLARE_INPUT_CHANGED_MEMBER(tms0980_power_button);
	DECLARE_WRITE_LINE_MEMBER(tms0980_auto_power_off);

	virtual void machine_start();
	virtual void machine_reset();

	// display common
	int m_display_wait;
	int m_display_maxy;
	int m_display_maxx;
	
	UINT32 m_display_state[0x20];
	UINT32 m_display_cache[0x20];
	UINT8 m_display_decay[0x20][0x20];
	UINT16 m_7seg_mask[0x20];

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety);
	
	// game-specific handlers
	void mathmagi_display();
	DECLARE_READ8_MEMBER(mathmagi_read_k);
	DECLARE_WRITE16_MEMBER(mathmagi_write_r);
	DECLARE_WRITE16_MEMBER(mathmagi_write_o);

	void amaztron_display();
	DECLARE_READ8_MEMBER(amaztron_read_k);
	DECLARE_WRITE16_MEMBER(amaztron_write_r);
	DECLARE_WRITE16_MEMBER(amaztron_write_o);

	void tc4_display();
	DECLARE_READ8_MEMBER(tc4_read_k);
	DECLARE_WRITE16_MEMBER(tc4_write_r);
	DECLARE_WRITE16_MEMBER(tc4_write_o);

	void ebball_display();
	DECLARE_READ8_MEMBER(ebball_read_k);
	DECLARE_WRITE16_MEMBER(ebball_write_r);
	DECLARE_WRITE16_MEMBER(ebball_write_o);

	DECLARE_READ8_MEMBER(elecdet_read_k);
	DECLARE_WRITE16_MEMBER(elecdet_write_r);
	DECLARE_WRITE16_MEMBER(elecdet_write_o);

	void starwbc_display();
	DECLARE_READ8_MEMBER(starwbc_read_k);
	DECLARE_WRITE16_MEMBER(starwbc_write_r);
	DECLARE_WRITE16_MEMBER(starwbc_write_o);

	DECLARE_READ8_MEMBER(elecbowl_read_k);
	DECLARE_WRITE16_MEMBER(elecbowl_write_r);
	DECLARE_WRITE16_MEMBER(elecbowl_write_o);

	DECLARE_READ8_MEMBER(comp4_read_k);
	DECLARE_WRITE16_MEMBER(comp4_write_r);
	DECLARE_WRITE16_MEMBER(comp4_write_o);

	DECLARE_READ8_MEMBER(simon_read_k);
	DECLARE_WRITE16_MEMBER(simon_write_r);
	DECLARE_WRITE16_MEMBER(simon_write_o);

	DECLARE_READ8_MEMBER(cnsector_read_k);
	DECLARE_WRITE16_MEMBER(cnsector_write_r);
	DECLARE_WRITE16_MEMBER(cnsector_write_o);

	DECLARE_READ8_MEMBER(merlin_read_k);
	DECLARE_WRITE16_MEMBER(merlin_write_r);
	DECLARE_WRITE16_MEMBER(merlin_write_o);

	DECLARE_READ8_MEMBER(stopthief_read_k);
	DECLARE_WRITE16_MEMBER(stopthief_write_r);
	DECLARE_WRITE16_MEMBER(stopthief_write_o);

	DECLARE_READ8_MEMBER(bankshot_read_k);
	DECLARE_WRITE16_MEMBER(bankshot_write_r);
	DECLARE_WRITE16_MEMBER(bankshot_write_o);

	DECLARE_READ8_MEMBER(splitsec_read_k);
	DECLARE_WRITE16_MEMBER(splitsec_write_r);
	DECLARE_WRITE16_MEMBER(splitsec_write_o);

	void tandy12_display();
	DECLARE_READ8_MEMBER(tandy12_read_k);
	DECLARE_WRITE16_MEMBER(tandy12_write_r);
	DECLARE_WRITE16_MEMBER(tandy12_write_o);
};


// machine_start/reset

void hh_tms1k_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, 0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_7seg_mask, 0, sizeof(m_7seg_mask));
	
	m_o = 0;
	m_r = 0;
	m_inp_mux = 0;
	m_power_on = false;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_cache));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_7seg_mask));

	save_item(NAME(m_o));
	save_item(NAME(m_r));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_power_on));
}

void hh_tms1k_state::machine_reset()
{
	m_power_on = true;
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// LED segments
enum
{
	lA = 0x01,
	lB = 0x02,
	lC = 0x04,
	lD = 0x08,
	lE = 0x10,
	lF = 0x20,
	lG = 0x40,
	lDP = 0x80
};

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_tms1k_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x < m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_power_on && m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			int ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_7seg_mask[y] != 0)
				output_set_digit_value(y, active_state[y] & m_7seg_mask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x < m_display_maxx; x++)
				output_set_lamp_value(y * mul + x, active_state[y] >> x & 1);
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_tms1k_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x < m_display_maxx; x++)
			if (!(m_display_state[y] >> x & 1) && m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;
	
	display_update();
}

void hh_tms1k_state::display_matrix(int maxx, int maxy, UINT32 setx, UINT32 sety)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;

	// update current state
	UINT32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? (setx & mask) : 0;
	
	display_update();
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

WRITE_LINE_MEMBER(hh_tms1k_state::tms0980_auto_power_off)
{
	if (state)
	{
		m_power_on = false;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

INPUT_CHANGED_MEMBER(hh_tms1k_state::tms0980_power_button)
{
	m_power_on = (bool)(FPTR)param;
	m_maincpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}



/***************************************************************************

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  APF Mathemagician
  * TMS1100 MCU, labeled MP1030
  * 2 x DS8870N - Hex LED Digit Driver
  * 2 x DS8861N - MOS-to-LED 5-Segment Driver

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

void hh_tms1k_state::mathmagi_display()
{
	m_display_maxx = 8;
	m_display_maxy = 11;

	// R0-R7: 7seg leds
	for (int y = 0; y < 8; y++)
	{
		m_7seg_mask[y] = 0x7f;
		m_display_state[y] = (m_r >> y & 1) ? (m_o >> 1) : 0;
	}

	// R8: custom math symbols digit
	// R9: custom equals digit
	// R10: misc lamps
	for (int y = 8; y < 11; y++)
		m_display_state[y] = (m_r >> y & 1) ? m_o : 0;

	display_update();
}

READ8_MEMBER(hh_tms1k_state::mathmagi_read_k)
{
	return read_inputs(6);
}

WRITE16_MEMBER(hh_tms1k_state::mathmagi_write_r)
{
	// R3,R5-R7,R9,R10: input mux
	m_inp_mux = (data >> 3 & 1) | (data >> 4 & 0xe) | (data >> 5 & 0x30);
	
	// +others:
	m_r = data;
	mathmagi_display();
}

WRITE16_MEMBER(hh_tms1k_state::mathmagi_write_o)
{
	// O1-O7: led segments A-G
	// O0: N/C
	data = (data << 1 & 0xfe) | (data >> 7 & 1); // because opla is unknown
	m_o = data;
}


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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
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

static MACHINE_CONFIG_START( mathmagi, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 175000) // RC osc. R=68K, C=82pf -> ~175kHz
	MCFG_TMS1XXX_OUTPUT_PLA(mathmagi_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, mathmagi_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, mathmagi_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, mathmagi_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mathmagi)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Amaze-A-Tron, by Ralph Baer
  * TMS1100 MCU, labeled MP3405(die label too)

  This is an electronic board game with a selection of 8 maze games,
  most of them for 2 players. A 5x5 playing grid and four markers are
  required to play. Refer to the official manual for more information.

***************************************************************************/

void hh_tms1k_state::amaztron_display()
{
	m_display_maxx = 8;
	m_display_maxy = 3;

	// R8,R9: select digit
	for (int y = 0; y < 2; y++)
	{
		m_7seg_mask[y] = 0x7f;
		m_display_state[y] = (m_r >> (y + 8) & 1) ? m_o : 0;
	}
	
	// R6,R7: lamps (-> lamp20,21)
	m_display_state[2] = m_r >> 6 & 3;
	
	display_update();
}

READ8_MEMBER(hh_tms1k_state::amaztron_read_k)
{
	UINT8 k = read_inputs(6);

	// the 5th column is tied to K4+K8
	if (k & 0x10) k |= 0xc;
	return k & 0xf;
}

WRITE16_MEMBER(hh_tms1k_state::amaztron_write_r)
{
	// R0-R5: input mux
	m_inp_mux = data & 0x3f;
	
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// other bits:
	m_r = data;
	amaztron_display();
}

WRITE16_MEMBER(hh_tms1k_state::amaztron_write_o)
{
	// O0-O6: digit segments
	// O7: N/C
	m_o = data & 0x7f;
	amaztron_display();
}


static INPUT_PORTS_START( amaztron )
	PORT_START("IN.0") // R0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Button 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Button 16")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 21")

	PORT_START("IN.1") // R1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Button 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Button 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Button 17")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Button 22")

	PORT_START("IN.2") // R2
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Button 8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Button 13")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Button 18")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Button 23")

	PORT_START("IN.3") // R3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Button 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Button 14")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Button 19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Button 24")

	PORT_START("IN.4") // R4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Button 5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Button 10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Button 15")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Button 20")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Button 25")

	PORT_START("IN.5") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Game Select")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Game Start")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static MACHINE_CONFIG_START( amaztron, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K?, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, amaztron_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, amaztron_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, amaztron_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_amaztron)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Total Control 4
  * TMS1400NLL MP7334-N2 (die labeled MP7334)

  This is a head to head electronic tabletop LED-display sports console.
  One cartridge(Football) was included with the console, the other three were
  sold separately. Gameplay has emphasis on strategy, read the official manual
  on how to play. Remember that you can rotate the view in MESS: rotate left
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

void hh_tms1k_state::tc4_display()
{
	m_display_wait = 50;
	
	// R5,7,8,9 are 7segs
	for (int y = 0; y < 10; y++)
		if (y >= 5 && y <= 9 && y != 6)
			m_7seg_mask[y] = 0x7f;
	
	// update current state (note: R6 as extra column!)
	display_matrix(9, 10, (m_o | (m_r << 2 & 0x100)), m_r);
}

READ8_MEMBER(hh_tms1k_state::tc4_read_k)
{
	UINT8 k = read_inputs(6);

	// read from cartridge
	if (m_inp_mux & 0x200)
		k |= m_inp_matrix[6]->read();

	return k;
}

WRITE16_MEMBER(hh_tms1k_state::tc4_write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R9: to cartridge slot
	m_inp_mux = data & 0x23f;
	
	// R6: led column 8
	// +other columns
	m_r = data;
	tc4_display();
}

WRITE16_MEMBER(hh_tms1k_state::tc4_write_o)
{
	// O0-O7: led row
	m_o = data;
	tc4_display();
}


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


static MACHINE_CONFIG_START( tc4, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 450000) // approximation - RC osc. R=27.3K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, tc4_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, tc4_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, tc4_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tc4)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Electronic Baseball (1)
  * TMS1000NLP MP0914 (die labeled MP0914A)
  
  This is a handheld LED baseball game. One player controls the batter, the CPU
  or other player controls the pitcher. Pitcher throw buttons are on a 'joypad'
  obtained from a compartment in the back. Player scores are supposed to be
  written down manually, the game doesn't save scores or innings (this annoyance
  was resolved in the sequel). For more information, refer to the official manual.
  
  The overlay graphic is known to have 2 versions: one where the field players
  are denoted by words ("left", "center", "short", etc), and an alternate one
  with little guys drawn next to the LEDs.


  lamp translation table: led LDzz from game PCB = MESS lampyx:

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

void hh_tms1k_state::ebball_display()
{
	// R8 is a 7seg
	m_7seg_mask[8] = 0x7f;
	
	display_matrix(7, 9, ~m_o, m_r);
}

READ8_MEMBER(hh_tms1k_state::ebball_read_k)
{
	// note: K8(Vss row) is always on
	return m_inp_matrix[5]->read() | read_inputs(5);
}

WRITE16_MEMBER(hh_tms1k_state::ebball_write_r)
{
	// R1-R5: input mux
	m_inp_mux = data >> 1 & 0x1f;
	
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);
	
	// R0-R8: led columns
	m_r = data;
	ebball_display();
}

WRITE16_MEMBER(hh_tms1k_state::ebball_write_o)
{
	// O0-O6: led row
	// O7: N/C
	m_o = data;
	ebball_display();
}


static INPUT_PORTS_START( ebball )
	PORT_START("IN.0") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Change Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Change Sides")
	PORT_CONFNAME( 0x04, 0x04, "Pitcher" )
	PORT_CONFSETTING(    0x04, "Auto" )
	PORT_CONFSETTING(    0x00, "Manual" )
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

static MACHINE_CONFIG_START( ebball, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 375000) // RC osc. R=43K, C=47pf -> ~375kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, ebball_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, ebball_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, ebball_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ebball)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Ideal Electronic Detective
  * TMS0980NLL MP6100A (die labeled 0980B-00)
  hardware (and concept) is very similar to Parker Bros Stop Thief

  This is an electronic board game. It requires game cards with suspect info,
  and good old pen and paper to record game progress. To start the game, enter
  difficulty(1-3), then number of players(1-4), then [ENTER]. Refer to the
  manual for more information.

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::elecdet_read_k)
{
	// note: the Vss row is always on
	return m_inp_matrix[4]->read() | read_inputs(4);
}

WRITE16_MEMBER(hh_tms1k_state::elecdet_write_r)
{
	// R7,R8: speaker on
	m_speaker->level_w((data & 0x180 && m_o & 0x80) ? 1 : 0);

	// R0-R6: select digit
	for (int y = 0; y < 7; y++)
		m_7seg_mask[y] = 0x7f;

	display_matrix(7, 7, BITSWAP8(m_o,7,5,2,1,4,0,6,3), data);
}

WRITE16_MEMBER(hh_tms1k_state::elecdet_write_o)
{
	// O0,O1,O4,O6: input mux
	m_inp_mux = (data & 3) | (data >> 2 & 4) | (data >> 3 & 8);
	
	// O0-O6: led segments A-G
	// O7: speaker out
	m_o = data;
}


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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, tms0980_power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("End Turn")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, tms0980_power_button, (void *)false)
INPUT_PORTS_END


static MACHINE_CONFIG_START( elecdet, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, 425000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, elecdet_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, elecdet_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, elecdet_write_o))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(hh_tms1k_state, tms0980_auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_elecdet)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Kenner Star Wars - Electronic Battle Command
  * TMS1100 MCU, labeled MP3438A

  This is a small tabletop space-dogfighting game. To start the game,
  press BASIC/INTER/ADV and enter P#(number of players), then
  START TURN. Refer to the official manual for more information.

***************************************************************************/

void hh_tms1k_state::starwbc_display()
{
	// R6,R8 are 7segs
	m_7seg_mask[6] = m_7seg_mask[8] = 0x7f;
	
	display_matrix(8, 10, m_o, m_r);
}

READ8_MEMBER(hh_tms1k_state::starwbc_read_k)
{
	return read_inputs(5);
}

WRITE16_MEMBER(hh_tms1k_state::starwbc_write_r)
{
	// R0,R1,R3,R5,R7: input mux
	m_inp_mux = (data & 3) | (data >> 1 & 4) | (data >> 2 & 8) | (data >> 3 & 0x10);

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0,R2,R4,R6,R8: led columns
	m_r = data & 0x155;
	starwbc_display();
}

WRITE16_MEMBER(hh_tms1k_state::starwbc_write_o)
{
	// O0-O7: led row
	m_o = (data << 4 & 0xf0) | (data >> 4 & 0x0f);
	starwbc_display();
}


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

static MACHINE_CONFIG_START( starwbc, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 325000) // RC osc. R=51K, C=47pf -> ~325kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, starwbc_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, starwbc_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, starwbc_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_starwbc)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Marx Series 300 Electronic Bowling Game
  * TMS1100NLL MP3403 DBS 7836 SINGAPORE

  10 lamps for bowling pins + 3 more bulbs, and 7segs for frame number and
  scores. Board size is 10-12" by 6-8".

  some clues:
  - it's from 1978
  - Merlin is MP3404, Amaze-A-Tron is MP3405, this one is MP3403
  - it plays some short jingles (you need to be lucky with button mashing)

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::elecbowl_read_k)
{
	return read_inputs(4);
}

WRITE16_MEMBER(hh_tms1k_state::elecbowl_write_r)
{
	// R4-R7: input mux
	m_inp_mux = data >> 4 & 0xf;

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R10: maybe a switch or other button row?
	// others: ?
}

WRITE16_MEMBER(hh_tms1k_state::elecbowl_write_o)
{
	// ?
}

static INPUT_PORTS_START( elecbowl )
	PORT_START("IN.0") // R4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)

	PORT_START("IN.2") // R6
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) // reset/newgame?

	PORT_START("IN.3") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
INPUT_PORTS_END


static const UINT16 elecbowl_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static MACHINE_CONFIG_START( elecbowl, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 300000) // approximation - unknown freq
	MCFG_TMS1XXX_OUTPUT_PLA(elecbowl_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, elecbowl_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, elecbowl_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, elecbowl_write_o))

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die labeled 4A0970D-04A)

  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  It is known as Logic 5 in Europe, and as Pythaligoras in Japan.

  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.


***************************************************************************/

READ8_MEMBER(hh_tms1k_state::comp4_read_k)
{
	return read_inputs(3);
}

WRITE16_MEMBER(hh_tms1k_state::comp4_write_r)
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

WRITE16_MEMBER(hh_tms1k_state::comp4_write_o)
{
	// O1-O3: input mux
	m_inp_mux = data >> 1 & 7;

	// O0: leds common
	// other bits: N/C
	m_o = data;
	display_matrix(11, 1, m_r, m_o);
}


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

static MACHINE_CONFIG_START( comp4, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, comp4_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, comp4_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, comp4_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_comp4)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Simon, created by Ralph Baer

  Revision A hardware:
  * TMS1000 (die labeled MP3226), DS75494 lamp driver

  Newer revisions (also Pocket Simon) have a smaller 16-pin MB4850 chip
  instead of the TMS1000. This one has been decapped too, but we couldn't
  find an internal ROM. It is possibly a cost-reduced custom ASIC specifically
  for Simon. The semi-sequel Super Simon uses a TMS1100.

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::simon_read_k)
{
	return read_inputs(4);
}

WRITE16_MEMBER(hh_tms1k_state::simon_write_r)
{
	// R4-R8 go through an 75494 IC first:
	// R4 -> 75494 IN6 -> green lamp
	// R5 -> 75494 IN3 -> red lamp
	// R6 -> 75494 IN5 -> yellow lamp
	// R7 -> 75494 IN2 -> blue lamp
	display_matrix(4, 1, data >> 4, 1);

	// R8 -> 75494 IN0 -> speaker
	m_speaker->level_w(data >> 8 & 1);

	// R0,R1,R2,R9: input mux
	// R3: GND
	// other bits: N/C
	m_inp_mux = (data & 7) | (data >> 6 & 8);
}

WRITE16_MEMBER(hh_tms1k_state::simon_write_o)
{
	// N/C
}


static INPUT_PORTS_START( simon )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x07, 0x02, "Game Select")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Green Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Blue Button")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Longest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R9
	PORT_CONFNAME( 0x0f, 0x01, "Skill Level")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )
INPUT_PORTS_END


static MACHINE_CONFIG_START( simon, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, simon_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, simon_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, simon_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_simon)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Code Name: Sector, by Bob Doyle
  * MP0905BNL ZA0379 (die labeled 0970F-05B)

  This is a tabletop submarine pursuit game. A grid board and small toy
  boats are used to remember your locations (a Paint app should be ok too).
  Refer to the official manual for more information, it is not a simple game.

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::cnsector_read_k)
{
	return read_inputs(5);
}

WRITE16_MEMBER(hh_tms1k_state::cnsector_write_r)
{
	m_display_maxx = 8;
	m_display_maxy = 7;

	// R0-R5: select digit (right-to-left)
	for (int y = 0; y < 6; y++)
	{
		m_7seg_mask[y] = 0xff;
		m_display_state[y] = (data >> y & 1) ? m_o : 0;
	}

	// R6-R9: direction leds (-> lamp60-63)
	m_display_state[6] = data >> 6 & 0xf;
	
	display_update();
}

WRITE16_MEMBER(hh_tms1k_state::cnsector_write_o)
{
	// O0-O4: input mux
	m_inp_mux = data & 0x1f;
	
	// O0-O7: digit segments
	m_o = data;
}


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


static MACHINE_CONFIG_START( cnsector, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, cnsector_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, cnsector_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, cnsector_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_cnsector)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Parker Bros Merlin handheld game, by Bob Doyle
  * TMS1100NLL MP3404A-N2
  * red LEDs and 1-bit sound
  
  Also published in Japan by Tomy as "Dr. Smith", white case instead of red.
  The one with dark-blue case is the rare sequel Master Merlin. More sequels
  followed too, but on other hardware.

  To start a game, press NEW GAME, followed by a number:
  1: Tic-Tac-Toe
  2: Music Machine
  3: Echo
  4: Blackjack 13
  5: Magic Square
  6: Mindbender

  Refer to the official manual for more information on the games.

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::merlin_read_k)
{
	return read_inputs(4);
}

WRITE16_MEMBER(hh_tms1k_state::merlin_write_r)
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

WRITE16_MEMBER(hh_tms1k_state::merlin_write_o)
{
	// O4-O6: speaker out (paralleled for increased current driving capability)
	static const int count[8] = { 0, 1, 1, 2, 1, 2, 2, 3 };
	m_speaker->level_w(count[data >> 4 & 7]);

	// O0-O3: input mux
	// O7: N/C
	m_inp_mux = data & 0xf;
}


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
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Same Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Button 10")

	PORT_START("IN.3") // O3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Comp Turn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hit Me")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("New Game")
INPUT_PORTS_END


static const INT16 merlin_speaker_levels[] = { 0, 10922, 21845, 32767 };

static MACHINE_CONFIG_START( merlin, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, merlin_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, merlin_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, merlin_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_merlin)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, merlin_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Stop Thief, by Bob Doyle
  * TMS0980NLL MP6101B (die labeled 0980B-01A)

  Stop Thief is actually a board game, the electronic device emulated here
  (called Electronic Crime Scanner) is an accessory. To start a game, press
  the ON button. Otherwise, it is in test-mode where you can hear all sounds.

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::stopthief_read_k)
{
	// note: the Vss row is always on
	return m_inp_matrix[2]->read() | read_inputs(2);
}

WRITE16_MEMBER(hh_tms1k_state::stopthief_write_r)
{
	m_display_maxx = 7;
	m_display_maxy = 3;

	// R0-R2: select digit
	UINT8 o = BITSWAP8(m_o,3,5,2,1,4,0,6,7) & 0x7f;
	for (int y = 0; y < m_display_maxy; y++)
	{
		m_7seg_mask[y] = 0x7f;
		m_display_state[y] = (data >> y & 1) ? o : 0;
	}

	display_update();

	// R3-R8: speaker on
	m_speaker->level_w((data & 0x1f8 && m_o & 8) ? 1 : 0);
}

WRITE16_MEMBER(hh_tms1k_state::stopthief_write_o)
{
	// O0,O6: input mux
	m_inp_mux = (data & 1) | (data >> 5 & 2);
	
	// O3: speaker out
	// O0-O2,O4-O7: led segments A-G
	m_o = data;
}


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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, tms0980_power_button, (void *)true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Tip")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Arrest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Clue")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, tms0980_power_button, (void *)false)
INPUT_PORTS_END

static MACHINE_CONFIG_START( stopthief, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0980, 425000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, stopthief_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, stopthief_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, stopthief_write_o))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(hh_tms1k_state, tms0980_auto_power_off))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_stopthie)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Bank Shot (known as Cue Ball in the UK), by Garry Kitchen
  * TMS1400NLL MP7313-N2 (die labeled MP7313)

  Bank Shot is an electronic pool game. To select a game, repeatedly press
  the [SELECT] button, then press [CUE UP] to start. Refer to the official
  manual for more information. The game selections are:
  1: Straight Pool (1 player)
  2: Straight Pool (2 players)
  3: Poison Pool
  4: Trick Shots

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::bankshot_read_k)
{
	return read_inputs(2);
}

WRITE16_MEMBER(hh_tms1k_state::bankshot_write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R2,R3: input mux
	m_inp_mux = data >> 2 & 3;

	// R2-R10: led columns
	m_r = data & ~3;
	display_matrix(7, 11, m_o, m_r);
}

WRITE16_MEMBER(hh_tms1k_state::bankshot_write_o)
{
	// O0-O6: led row
	// O7: N/C
	m_o = data;
	display_matrix(7, 11, m_o, m_r);
}


/* physical button layout and labels is like this:
  (note: remember that you can rotate the display in MESS)

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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Cue Up / Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Select / Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ball Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Ball Over")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bankshot, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 475000) // approximation - RC osc. R=24K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, bankshot_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, bankshot_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, bankshot_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_bankshot)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Parker Brothers Split Second
  * TMS1400NLL MP7314-N2 (die labeled MP7314)

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

READ8_MEMBER(hh_tms1k_state::splitsec_read_k)
{
	return read_inputs(2);
}

WRITE16_MEMBER(hh_tms1k_state::splitsec_write_r)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0-R7: led columns
	m_r = data;
	display_matrix(7, 8, m_o, m_r);
}

WRITE16_MEMBER(hh_tms1k_state::splitsec_write_o)
{
	// O0-O6: led row
	// O7: N/C
	m_o = data;
	display_matrix(7, 8, m_o, m_r);
}

static INPUT_PORTS_START( splitsec )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 4 separate directional buttons, hence 16way
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( splitsec, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 475000) // approximation - RC osc. R=24K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, splitsec_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, splitsec_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, splitsec_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_splitsec)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tandy Radio Shack Computerized Arcade (1981, 1982, 1995)
  * TMS1100 CD7282SL

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

void hh_tms1k_state::tandy12_display()
{
	// O0-O7: button lamps 1-8, R0-R3: button lamps 9-12
	display_matrix(13, 1, (m_o << 1 & 0x1fe) | (m_r << 9 & 0x1e00), 1);
}

READ8_MEMBER(hh_tms1k_state::tandy12_read_k)
{
	return read_inputs(5);
}

WRITE16_MEMBER(hh_tms1k_state::tandy12_write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R9: input mux
	m_inp_mux = data >> 5 & 0x1f;

	// other bits:
	m_r = data;
	tandy12_display();
}

WRITE16_MEMBER(hh_tms1k_state::tandy12_write_o)
{
	m_o = data;
	tandy12_display();
}


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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("12")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
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

static MACHINE_CONFIG_START( tandy12, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 400000) // RC osc. R=39K, C=47pf -> ~400kHz
	MCFG_TMS1XXX_OUTPUT_PLA(tandy12_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, tandy12_read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, tandy12_write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, tandy12_write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_tandy12)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mathmagi )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "mp1030", 0x0000, 0x800, CRC(a81d7ccb) SHA1(4756ce42f1ea28ce5fe6498312f8306f10370969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mathmagi_opla.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( amaztron )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3405", 0x0000, 0x0800, CRC(9cbc0009) SHA1(17772681271b59280687492f37fa0859998f041d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_amaztron_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_amaztron_opla.pla", 0, 365, CRC(f3875384) SHA1(3c256a3db4f0aa9d93cf78124db39f4cbdc57e4a) )
ROM_END


ROM_START( tc4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7334", 0x0000, 0x1000, CRC(923f3821) SHA1(a9ae342d7ff8dae1dedcd1e4984bcfae68586581) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_tc4_opla.pla", 0, 557, CRC(3b908725) SHA1(f83bf5faa5b3cb51f87adc1639b00d6f9a71ad19) )
ROM_END


ROM_START( ebball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0914", 0x0000, 0x0400, CRC(3c6fb05b) SHA1(b2fe4b3ca72d6b4c9bfa84d67f64afdc215e7178) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ebball_mpla.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball_opla.pla", 0, 365, CRC(062bf5bb) SHA1(8d73ee35444299595961225528b153e3a5fe66bf) )
ROM_END


ROM_START( elecdet )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6100a", 0x0000, 0x1000, CRC(6f396bb8) SHA1(1f104d4ca9bee0d4572be4779b7551dfe20c4f04) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_elecdet_opla.pla", 0, 352, CRC(652d19c3) SHA1(75550c2b293453b6b9efed88c8cc77195a53161f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_elecdet_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


ROM_START( starwbc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3438a", 0x0000, 0x0800, CRC(c12b7069) SHA1(d1f39c69a543c128023ba11cc6228bacdfab04de) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_starwbc_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_opla.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END

ROM_START( starwbcp )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "us4270755", 0x0000, 0x0800, BAD_DUMP CRC(fb3332f2) SHA1(a79ac81e239983cd699b7cfcc55f89b203b2c9ec) ) // from patent US4270755, may have errors

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_starwbc_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_starwbc_opla.pla", 0, 365, CRC(d358a76d) SHA1(06b60b207540e9b726439141acadea9aba718013) )
ROM_END


ROM_START( elecbowl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3403", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_elecbowl_opla.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( comp4 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0904nl_cp0904a", 0x0000, 0x0400, CRC(6233ee1b) SHA1(738e109b38c97804b4ec52bed80b00a8634ad453) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_comp4_mpla.pla", 0, 860, CRC(ee9d7d9e) SHA1(25484e18f6a07f7cdb21a07220e2f2a82fadfe7b) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_comp4_opla.pla", 0, 352, CRC(a0f887d1) SHA1(3c666663d484d5bed81e1014f8715aab8a3d489f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_comp4_spla.pla", 0, 157, CRC(e5bddd90) SHA1(4b1c6512c70e5bcd23c2dbf0c88cd8aa2c632a10) )
ROM_END


ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_simon_mpla.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_simon_opla.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END


ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(201036e9) SHA1(b37fef86bb2bceaf0ac8bb3745b4702d17366914) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_mpla.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_cnsector_opla.pla", 0, 352, CRC(7c0bdcd6) SHA1(dade774097e8095dca5deac7b2367d0c701aca51) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_cnsector_spla.pla", 0, 157, CRC(56c37a4f) SHA1(18ecc20d2666e89673739056483aed5a261ae927) )
ROM_END


ROM_START( merlin )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "mp3404", 0x0000, 0x800, CRC(7515a75d) SHA1(76ca3605d3fde1df62f79b9bb1f534c2a2ae0229) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_merlin_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_merlin_opla.pla", 0, 365, CRC(3921b074) SHA1(12bd58e4d6676eb8c7059ef53598279e4f1a32ea) )
ROM_END


ROM_START( stopthie )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6101b", 0x0000, 0x1000, CRC(8bde5bb4) SHA1(8c318fcce67acc24c7ae361f575f28ec6f94665a) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_opla.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_stopthie_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( stopthiep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "us4341385", 0x0000, 0x1000, CRC(07aec38a) SHA1(0a3d0956495c0d6d9ea771feae6c14a473a800dc) ) // from patent US4341385, data should be correct (it included checksums)

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_default_mpla.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthie_opla.pla", 0, 352, CRC(50337a48) SHA1(4a9ea62ed797a9ac5190eec3bb6ebebb7814628c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_stopthie_spla.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END


ROM_START( bankshot )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7313", 0x0000, 0x1000, CRC(7a5016a9) SHA1(a8730dc8a282ffaa3d89e675f371d43eb39f39b4) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_bankshot_opla.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


ROM_START( splitsec )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7314", 0x0000, 0x1000, CRC(e94b2098) SHA1(f0fc1f56a829252185592a2508740354c50bedf8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_splitsec_opla.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END


ROM_START( tandy12 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tandy12_opla.pla", 0, 365, NO_DUMP )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1980, mathmagi,  0,        0, mathmagi,  mathmagi,  driver_device, 0, "APF Electronics Inc.", "Mathemagician", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )

CONS( 1979, amaztron,  0,        0, amaztron,  amaztron,  driver_device, 0, "Coleco", "Amaze-A-Tron", GAME_SUPPORTS_SAVE )
CONS( 1981, tc4,       0,        0, tc4,       tc4,       driver_device, 0, "Coleco", "Total Control 4", GAME_SUPPORTS_SAVE )

CONS( 1978, ebball,    0,        0, ebball,    ebball,    driver_device, 0, "Entex", "Electronic Baseball (Entex)", GAME_SUPPORTS_SAVE ) // or 1979?

CONS( 1979, elecdet,   0,        0, elecdet,   elecdet,   driver_device, 0, "Ideal", "Electronic Detective", GAME_SUPPORTS_SAVE ) // unplayable without game cards

CONS( 1979, starwbc,   0,        0, starwbc,   starwbc,   driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command", GAME_SUPPORTS_SAVE )
CONS( 1979, starwbcp,  starwbc,  0, starwbc,   starwbc,   driver_device, 0, "Kenner", "Star Wars - Electronic Battle Command (prototype)", GAME_SUPPORTS_SAVE )

CONS( 1978, elecbowl,  0,        0, elecbowl,  elecbowl,  driver_device, 0, "Marx", "Electronic Bowling", GAME_SUPPORTS_SAVE | GAME_MECHANICAL | GAME_NOT_WORKING )

CONS( 1977, comp4,     0,        0, comp4,     comp4,     driver_device, 0, "Milton Bradley", "Comp IV", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
CONS( 1978, simon,     0,        0, simon,     simon,     driver_device, 0, "Milton Bradley", "Simon (Rev. A)", GAME_SUPPORTS_SAVE )

CONS( 1977, cnsector,  0,        0, cnsector,  cnsector,  driver_device, 0, "Parker Brothers", "Code Name: Sector", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW ) // unplayable without writing board
CONS( 1978, merlin,    0,        0, merlin,    merlin,    driver_device, 0, "Parker Brothers", "Merlin", GAME_SUPPORTS_SAVE )
CONS( 1979, stopthie,  0,        0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner)", GAME_SUPPORTS_SAVE ) // unplayable without game board
CONS( 1979, stopthiep, stopthie, 0, stopthief, stopthief, driver_device, 0, "Parker Brothers", "Stop Thief (Electronic Crime Scanner) (prototype)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
CONS( 1980, bankshot,  0,        0, bankshot,  bankshot,  driver_device, 0, "Parker Brothers", "Bank Shot - Electronic Pool", GAME_SUPPORTS_SAVE )
CONS( 1980, splitsec,  0,        0, splitsec,  splitsec,  driver_device, 0, "Parker Brothers", "Split Second", GAME_SUPPORTS_SAVE )

CONS( 1981, tandy12,   0,        0, tandy12,   tandy12,   driver_device, 0, "Tandy Radio Shack", "Tandy-12: Computerized Arcade", GAME_SUPPORTS_SAVE ) // partially unplayable without cards/dice/..
