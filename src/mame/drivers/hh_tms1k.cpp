// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle, Kevin Horton
/***************************************************************************

  This driver is a collection of simple dedicated handheld and tabletop
  toys based around the TMS1000 MCU series. Anything more complex or clearly
  part of a series is (or will be) in its own driver, see:
  - eva: Chrysler EVA-11 (and EVA-24)
  - microvsn: Milton Bradley MicroVision

  (contd.) hh_tms1k child drivers:
  - ticalc1x: TI TMS1K-based calculators
  - tispellb: TI Spelling B series gen. 1
  - tispeak: TI Speak & Spell series gen. 1

  Let's use this driver for a list of known devices and their serials,
  excluding most of TI's own products(they normally didn't use "MP" codes).

  serial   device    etc.
--------------------------------------------------------------------
 @CP0904A  TMS0970   1977, Milton Bradley Comp IV
 @MP0905B  TMS0970   1977, Parker Brothers Codename Sector
 *MP0057   TMS1000   1978, APH Student Speech+ (same ROM contents as TSI Speech+?)
 @MP0154   TMS1000   1979, Fonas 2 Player Baseball
 @MP0158   TMS1000   1979, Entex Soccer (6003)
 @MP0163   TMS1000   1979, A-One LSI Match Number/LJN Electronic Concentration
 @MP0166   TMS1000   1980, A-One Arrange Ball/LJN Computer Impulse/Tandy Zingo (model 60-2123)
 @MP0168   TMS1000   1979, Conic Multisport/Tandy Sports Arena (model 60-2158)
 @MP0170   TMS1000   1979, Conic Football
 *MP0230   TMS1000   1980, Entex Blast It (6015)
 @MP0271   TMS1000   1982, Tandy Radio Shack Monkey See
 @MP0907   TMS1000   1979, Conic Basketball (101-006)
 @MP0908   TMS1000   1979, Conic Electronic I.Q.
 *MP0910   TMS1000   1979, Conic Basketball (101-003)
 @MP0914   TMS1000   1979, Entex Baseball 1
 @MP0915   TMS1000   1979, Bandai System Control Car: Cheetah/The Incredible Brain Buggy
 @MP0919   TMS1000   1979, Tiger Copy Cat (model 7-520)
 @MP0920   TMS1000   1979, Entex Space Battle (6004)
 @MP0923   TMS1000   1979, Entex Baseball 2 (6002)
 *MP1022   TMS1100   1979, Texas Instruments unknown thermostat
 @MP1030   TMS1100   1980, APF Mathemagician
 *MP1072   TMS1100   198?, unknown device, Germany (have decap)
 @MP1133   TMS1470   1979, Kosmos Astro
 @MP1180   TMS1100   1980, Tomy Power House Pinball
 @MP1181   TMS1100   1979, Conic Football 2
 @MP1183   TMS1100   1980, E.R.S. Superbowl XV Football/Tandy Championship Football (model 60-2151)
 @MP1185   TMS1100   1979, Fonas 3-in-1: Football, Basketball, Soccer
 @MP1193   TMS1100   1980, Tandy Championship Football (model 60-2150)
 @MP1204   TMS1100   1980, Entex Baseball 3 (6007)
 *MP1209   TMS1100   1980, U.S. Games Space Cruiser/Strategy Football
 @MP1211   TMS1100   1980, Entex Space Invader (6012)
 @MP1218   TMS1100   1980, Entex Basketball 2 (6010)
 @MP1219   TMS1100   1980, U.S. Games Super Sports-4
 @MP1221   TMS1100   1980, Entex Raise The Devil (6011)
 *MP1296   TMS1100?  1982, Entex Black Knight
 @MP1312   TMS1100   1983, Gakken FX-Micom R-165/Tandy Radio Shack Science Fair Microcomputer Trainer
 *MP1359   TMS1100?  1985, Capsela CRC2000
 @MP1525   TMS1170   1980, Coleco Head to Head: Electronic Baseball
 @MP1604   TMS1370   1982, Gakken Invader 2000/Tandy Cosmic Fire Away 3000
 @MP1801   TMS1700   1981, Tiger Ditto/Tandy Pocket Repeat (model 60-2152)
 @MP2105   TMS1370   1979, Gakken/Entex Poker (6005)
 @MP2139   TMS1370   1981, Gakken Galaxy Invader 1000/Tandy Cosmic 1000 Fire Away
 @MP2726   TMS1040   1979, Tomy Break Up
 *MP2788   TMS1040?  1980, Bandai Flight Time (? note: VFD-capable)
 @MP3005   TMS1730   1989, Tiger Copy Cat (model 7-522)
 @MP3201   TMS1000   1977, Milton Bradley Electronic Battleship (1977, model 4750A)
 @MP3208   TMS1000   1977, Milton Bradley Electronic Battleship (1977, model 4750B)
 @MP3226   TMS1000   1978, Milton Bradley Simon (Rev A)
 *MP3232   TMS1000   1979, Fonas 2-Player Baseball (no "MP" on chip label)
 @MP3300   TMS1000   1979, Milton Bradley Simon (Rev F)
 @MP3301A  TMS1000   1979, Milton Bradley Big Trak
 @MP3320A  TMS1000   1979, Coleco Head to Head: Electronic Basketball
 @MP3321A  TMS1000   1979, Coleco Head to Head: Electronic Hockey
 *MP3352   TMS1200   1979, Tiger Sub Wars (model 7-490)
 @M32001   TMS1000   1981, Coleco Quiz Wiz Challenger (note: MP3398, MP3399, M3200x?)
 *M32018   TMS1000   1990, unknown device (have decap/dump)
  M32045B  TMS1000   1983, Chrysler Electronic Voice Alert (11-function) -> eva.cpp
 @MP3403   TMS1100   1978, Marx Electronic Bowling
 @MP3404   TMS1100   1978, Parker Brothers Merlin
 @MP3405   TMS1100   1979, Coleco Amaze-A-Tron
 @MP3415   TMS1100   1978, Coleco Electronic Quarterback
 @MP3435   TMS1100   1979, Coleco Zodiac
 @MP3438A  TMS1100   1979, Kenner Star Wars Electronic Battle Command
  MP3450A  TMS1100   1979, MicroVision cartridge: Blockbuster
  MP3454   TMS1100   1979, MicroVision cartridge: Star Trek Phaser Strike
  MP3455   TMS1100   1980, MicroVision cartridge: Pinball
  MP3457   TMS1100   1979, MicroVision cartridge: Mindbuster
 @MP3460   TMS1100   1979, Coleco Head to Head: Electronic Football
  MP3474   TMS1100   1979, MicroVision cartridge: Vegas Slots
  MP3475   TMS1100   1979, MicroVision cartridge: Bowling
 @MP3476   TMS1100   1979, Milton Bradley Super Simon
  MP3479   TMS1100   1980, MicroVision cartridge: Baseball
  MP3481   TMS1100   1979, MicroVision cartridge: Connect Four
 *MP3489   TMS1100   1980, Kenner Live Action Football
 @MP3491   TMS1100   1979, Mattel Thoroughbred Horse Race Analyzer
 *MP3493   TMS1100   1980, Milton Bradley OMNI Entertainment System (1/2)
 *MP3494   TMS1100   1980, Milton Bradley OMNI Entertainment System (2/2)
  MP3496   TMS1100   1980, MicroVision cartridge: Sea Duel
  M34009   TMS1100   1981, MicroVision cartridge: Alien Raiders (note: MP3498, MP3499, M3400x..)
 @M34012   TMS1100   1980, Mattel Dungeons & Dragons - Computer Labyrinth Game
 *M34014   TMS1100   1981, Coleco Bowlatronic
  M34017   TMS1100   1981, MicroVision cartridge: Cosmic Hunter
 @M34018   TMS1100   1981, Coleco Head to Head: Electronic Boxing
 @M34038   TMS1100   1982, Parker Brothers Lost Treasure
  M34047   TMS1100   1982, MicroVision cartridge: Super Blockbuster
 @M34078A  TMS1100   1983, Milton Bradley Electronic Arcade Mania
 @MP4486A  TMS1000C  1983, Vulcan XL 25
 *MP6061   TMS0970   1979, Texas Instruments Electronic Digital Thermostat
 @MP6100A  TMS0980   1979, Ideal Electronic Detective
 @MP6101B  TMS0980   1979, Parker Brothers Stop Thief
 *MP6361   ?         1983, Defender Strikes (? note: VFD-capable)
 @MP7304   TMS1400   1982, Tiger 7 in 1 Sports Stadium (model 7-555)
 @MP7313   TMS1400   1980, Parker Brothers Bank Shot
 @MP7314   TMS1400   1980, Parker Brothers Split Second
  MP7324   TMS1400   1985, Tiger K28/Coleco Talking Teacher -> tispeak.cpp
 @MP7332   TMS1400   1981, Milton Bradley Dark Tower
 @MP7334   TMS1400   1981, Coleco Total Control 4
 @MP7351   TMS1400   1982, Parker Brothers Master Merlin
 @MP7551   TMS1670   1980, Entex Color Football 4 (6009)
 @MPF553   TMS1670   1980, Gakken/Entex Jackpot: Gin Rummy & Black Jack (6008) (note: assume F to be a misprint)
 *MP7573   TMS1670?  1981, Entex Select-a-Game cartridge: Football 4 (? note: 40-pin, VFD-capable)
 *M95041   ?         1983, Tsukuda Game Pachinko (? note: 40-pin, VFD-capable)

  inconsistent:

 @TMS1007  TMS1000   1976, TSI Speech+ (S14002-A)
 @CD7282SL TMS1100   1981, Tandy Radio Shack Tandy-12 (serial is similar to TI Speak & Spell series?)

  (* means undumped unless noted, @ denotes it's in this driver)


  TODO:
  - verify output PLA and microinstructions PLA for MCUs that have been dumped
    electronically (mpla is usually the default, opla is often custom)
  - unknown MCU clocks for some: TMS1000 RC curve is documented in the data manual,
    but not for newer ones (rev. E or TMS1400 MCUs). TMS0970/0980 osc. is on-die.
  - some of the games rely on the fact that faster/longer strobed leds appear brighter,
    eg. tc4/h2hfootb(offense), bankshot(cue ball), f3in1(ball), ...
  - 7in1ss: in 2-player mode, game select and skill select can be configured after selecting a game?
  - arrball: shot button is unresponsive sometimes, maybe BTANB? no video of game on Youtube
    ROM is good, PLAs are good, input mux is good
  - bship discrete sound, netlist is documented
  - finish bshipb SN76477 sound
  - improve elecbowl driver
  - is alphie(patent) the same as the final version?

***************************************************************************/

#include "emu.h"
#include "includes/hh_tms1k.h"

#include "machine/tms1024.h"
#include "sound/beep.h"
#include "sound/s14001a.h"
#include "sound/sn76477.h"
#include "video/hlcd0515.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "softlist.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "7in1ss.lh"
#include "alphie.lh"
#include "amaztron.lh" // clickable
#include "arcmania.lh" // clickable
#include "arrball.lh"
#include "astro.lh"
#include "bankshot.lh"
#include "bcheetah.lh"
#include "bigtrak.lh"
#include "bship.lh" // clickable
#include "cmsport.lh"
#include "cnbaskb.lh"
#include "cnfball.lh"
#include "cnfball2.lh"
#include "cnsector.lh" // clickable
#include "comp4.lh" // clickable
#include "copycat.lh" // clickable
#include "copycatm2.lh" // clickable
#include "ditto.lh" // clickable
#include "cqback.lh"
#include "ebball.lh"
#include "ebball2.lh"
#include "ebball3.lh"
#include "ebaskb2.lh"
#include "efootb4.lh"
#include "einvader.lh"
#include "elecbowl.lh"
#include "elecdet.lh"
#include "eleciq.lh" // clickable
#include "esbattle.lh"
#include "esoccer.lh"
#include "f2pbball.lh"
#include "f3in1.lh"
#include "fxmcr165.lh" // clickable
#include "gjackpot.lh"
#include "gpoker.lh"
#include "h2hbaseb.lh"
#include "h2hbaskb.lh"
#include "h2hboxing.lh"
#include "h2hfootb.lh"
#include "h2hhockey.lh"
#include "horseran.lh"
#include "lostreas.lh" // clickable
#include "matchnum.lh" // clickable
#include "mathmagi.lh"
#include "mbdtower.lh" // clickable
#include "mdndclab.lh" // clickable
#include "merlin.lh" // clickable
#include "mmerlin.lh" // clickable
#include "monkeysee.lh"
#include "quizwizc.lh"
#include "raisedvl.lh"
#include "simon.lh" // clickable
#include "speechp.lh"
#include "splitsec.lh"
#include "ssimon.lh" // clickable
#include "ssports4.lh"
#include "starwbc.lh" // clickable
#include "stopthief.lh"
#include "tandy12.lh" // clickable
#include "tbreakup.lh"
#include "tc4.lh"
#include "tcfball.lh"
#include "tcfballa.lh"
#include "timaze.lh"
#include "xl25.lh" // clickable
#include "zodiac.lh"

#include "hh_tms1k_test.lh" // common test-layout - use external artwork


// machine_start/reset

void hh_tms1k_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
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
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_power_on && m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state |= (ds << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = ds;
			else
				m_out_a[y] = ds;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = active_state & m_display_segmask[y];
	}

	// output optional power led
	if (m_power_led != m_power_on)
	{
		m_power_led = m_power_on;
		output().set_value("power_led", m_power_led ? 1 : 0);
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

void hh_tms1k_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void hh_tms1k_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

u8 hh_tms1k_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}

u8 hh_tms1k_state::read_rotated_inputs(int columns, u8 rowmask)
{
	u8 ret = 0;
	u16 colmask = (1 << columns) - 1;

	// read selected input columns
	for (int i = 0; i < 8; i++)
		if (1 << i & rowmask && m_inp_matrix[i]->read() & m_inp_mux & colmask)
			ret |= 1 << i;

	return ret;
}

INPUT_CHANGED_MEMBER(hh_tms1k_state::power_button)
{
	m_power_on = (bool)(uintptr_t)param;
	m_maincpu->set_input_line(INPUT_LINE_RESET, m_power_on ? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER(hh_tms1k_state::auto_power_off)
{
	// devices with a TMS0980 can auto power-off
	if (state)
		power_off();
}

void hh_tms1k_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  A-One LSI Match Number
  * PCB label PT-204 "Pair Card"
  * TMS1000NLL MP0163 (die label 1000B, MP0163)
  * 2x2-digit 7seg LED displays + 3 LEDs, 1-bit sound

  A-One was a subsidiary of Bandai? The PCB serial PT-xxx is same, and the font
  used on the boxes for "A-One LSI" is same as "Bandai Electronics" from early-80s.

  known releases:
  - Japan: Match Number (white case, Queen playing card bezel)
  - USA: Electronic Concentration, published by LJN (black case, rainbow pattern bezel)
  - UK: Electronic Concentration, published by Peter Pan Playthings (same as USA version)

***************************************************************************/

class matchnum_state : public hh_tms1k_state
{
public:
	matchnum_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void matchnum(machine_config &config);
};

// handlers

void matchnum_state::prepare_display()
{
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 4, m_o, m_r);
}

WRITE16_MEMBER(matchnum_state::write_r)
{
	// R3-R5,R8-R10: input mux
	m_inp_mux = (data >> 3 & 7) | (data >> 5 & 0x38);

	// R6,R7: speaker out
	m_speaker->level_w(data >> 6 & 3);

	// R0-R3: digit/led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(matchnum_state::write_o)
{
	// O0-O6: digit segments A-G
	// O7: led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(matchnum_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

static INPUT_PORTS_START( matchnum )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 16")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 15")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 14")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 13")

	PORT_START("IN.1") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 19")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 18")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 17")

	PORT_START("IN.2") // R5
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Change")
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 12")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 9")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 5")

	PORT_START("IN.5") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 1")
INPUT_PORTS_END

static const s16 matchnum_speaker_levels[4] = { 0, 0x7fff, -0x8000, 0 };

void matchnum_state::matchnum(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(matchnum_state::read_k));
	m_maincpu->r().set(FUNC(matchnum_state::write_r));
	m_maincpu->o().set(FUNC(matchnum_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_matchnum);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, matchnum_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  A-One LSI Arrange Ball
  * PCB label Kaken, PT-249
  * TMS1000NLL MP0166 (die label 1000B, MP0166)
  * 2-digit 7seg LED display + 22 LEDs, 1-bit sound

  known releases:
  - Japan/World: Arrange Ball (black case)
  - USA(1): Zingo (model 60-2123), published by Tandy (red case)
  - USA(2): Computer Impulse, published by LJN (white case)
  - Germany: Fixball, unknown publisher, same as LJN version

***************************************************************************/

class arrball_state : public hh_tms1k_state
{
public:
	arrball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void arrball(machine_config &config);
};

// handlers

void arrball_state::prepare_display()
{
	set_display_segmask(0x10, 0x7f);
	set_display_segmask(0x20, 0x06); // left digit only segments B and C
	display_matrix(7, 7, m_o, m_r);
}

WRITE16_MEMBER(arrball_state::write_r)
{
	// R8: input mux (always set)
	m_inp_mux = data >> 8 & 1;

	// R9,R10: speaker out
	m_speaker->level_w(data >> 9 & 3);

	// R0-R6: digit/led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(arrball_state::write_o)
{
	// O0-O6: digit segments/led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(arrball_state::read_k)
{
	// K: multiplexed inputs (actually just 1)
	return read_inputs(1);
}

// config

static INPUT_PORTS_START( arrball )
	PORT_START("IN.0") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shot")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Stop")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Speed" )
	PORT_CONFSETTING(    0x00, "Slow" )
	PORT_CONFSETTING(    0x08, "Fast" )
INPUT_PORTS_END

static const s16 arrball_speaker_levels[4] = { 0, 0x7fff, -0x8000, 0 };

void arrball_state::arrball(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(arrball_state::read_k));
	m_maincpu->r().set(FUNC(arrball_state::write_r));
	m_maincpu->o().set(FUNC(arrball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_arrball);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, arrball_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  APF Mathemagician
  * TMS1100 MCU, label MP1030 (no decap)
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
	void mathmagi(machine_config &config);
};

// handlers

void mathmagi_state::prepare_display()
{
	set_display_segmask(0xff, 0x7f);
	display_matrix(7, 11, m_o, m_r);
}

WRITE16_MEMBER(mathmagi_state::write_r)
{
	// R3,R5-R7,R9,R10: input mux
	m_inp_mux = (data >> 3 & 1) | (data >> 4 & 0xe) | (data >> 5 & 0x30);

	// R0-R7: 7seg leds
	// R8: custom math symbols digit
	// R9: custom equals digit
	// R10: misc lamps
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(mathmagi_state::write_o)
{
	// O1-O7: led/digit segment data
	// O0: N/C
	m_o = data;
	prepare_display();
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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// output PLA is not decapped
static const u16 mathmagi_output_pla[0x20] =
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

void mathmagi_state::mathmagi(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 175000); // approximation - RC osc. R=68K, C=82pF
	m_maincpu->set_output_pla(mathmagi_output_pla);
	m_maincpu->k().set(FUNC(mathmagi_state::read_k));
	m_maincpu->r().set(FUNC(mathmagi_state::write_r));
	m_maincpu->o().set(FUNC(mathmagi_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_mathmagi);

	/* no sound! */
}





/***************************************************************************

  Bandai System Control Car: Cheetah 「システムコントロールカー チーター」
  * TMS1000NLL MP0915 (die label 1000B, MP0915)
  * 2 motors (one for back axis, one for steering), no sound

  It's a programmable buggy, like Big Track but much simpler. To add a command
  step in program-mode, press a direction key and one of the time delay number
  keys at the same time. To run the program(max 24 steps), switch to run-mode
  and press the go-key.

  known releases:
  - Japan: System Control Car: Cheetah
  - USA: The Incredible Brain Buggy, published by Fundimensions
  - UK: The Incredible Brain Buggy, published by Palitoy (same as USA version)

***************************************************************************/

class bcheetah_state : public hh_tms1k_state
{
public:
	bcheetah_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void bcheetah(machine_config &config);
};

// handlers

WRITE16_MEMBER(bcheetah_state::write_r)
{
	// R0-R4: input mux
	// R5,R6: tied to K4??
	m_inp_mux = data & 0x1f;
}

WRITE16_MEMBER(bcheetah_state::write_o)
{
	// O1: back motor (drive)
	// O0: front motor steer left
	// O2: front motor steer right
	// O3: GND, other: N/C
	output().set_value("motor1", data >> 1 & 1);
	output().set_value("motor2_left", data & 1);
	output().set_value("motor2_right", data >> 2 & 1);
}

READ8_MEMBER(bcheetah_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( bcheetah )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, "Mode")
	PORT_CONFSETTING(    0x02, "Program" )
	PORT_CONFSETTING(    0x00, "Run" )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Forward")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Go")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Stop")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x05, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x05, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
INPUT_PORTS_END

void bcheetah_state::bcheetah(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 100000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(bcheetah_state::read_k));
	m_maincpu->r().set(FUNC(bcheetah_state::write_r));
	m_maincpu->o().set(FUNC(bcheetah_state::write_o));

	config.set_default_layout(layout_bcheetah);

	/* no sound! */
}





/***************************************************************************

  Coleco Amaze-A-Tron, by Ralph Baer
  * TMS1100 MCU, label MP3405 (die label same)
  * 2-digit 7seg LED display + 2 LEDs(one red, one green), 1-bit sound
  * 5*5 pressure-sensitive playing board(buttons), 4 game pieces

  This is an electronic board game with a selection of 8 maze games,
  most of them for 2 players.

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
	void amaztron(machine_config &config);
};

// handlers

void amaztron_state::prepare_display()
{
	set_display_segmask(0xc, 0x7f);
	display_matrix(7, 4, m_o, m_r);
}

WRITE16_MEMBER(amaztron_state::write_r)
{
	// R0-R5: input mux
	m_inp_mux = data & 0x3f;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R6,R7: leds
	// R8,R9: select digit
	m_r = data >> 6 & 0xf;
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
	u8 k = read_inputs(6);

	// the 5th column is tied to K4+K8
	if (k & 0x10) k |= 0xc;
	return k & 0xf;
}

// config

static INPUT_PORTS_START( amaztron )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 16")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 21")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 17")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 22")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 13")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 18")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 23")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 14")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 19")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 24")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 15")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 25")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Game Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Game Start")
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void amaztron_state::amaztron(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 300000); // approximation - RC osc. R=33K?, C=100pF
	m_maincpu->k().set(FUNC(amaztron_state::read_k));
	m_maincpu->r().set(FUNC(amaztron_state::write_r));
	m_maincpu->o().set(FUNC(amaztron_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_amaztron);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Zodiac - The Astrology Computer (model 2110)
  * TMS1100 MP3435 (no decap)
  * 8-digit 7seg display, 12 other LEDs, 1-bit sound

  As the name suggests, this is an astrologic calculator. Refer to the
  (very extensive) manual on how to use it.

***************************************************************************/

class zodiac_state : public hh_tms1k_state
{
public:
	zodiac_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void zodiac(machine_config &config);
};

// handlers

void zodiac_state::prepare_display()
{
	set_display_segmask(0xff, 0x7f);
	display_matrix(8, 10, m_o, m_r);
}

WRITE16_MEMBER(zodiac_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4,R8: input mux
	m_inp_mux = (data & 0x1f) | (data >> 3 & 0x20);

	// R0-R7: digit select
	// R8,R9: led select
	m_r = data & 0x3ff;
	prepare_display();
}

WRITE16_MEMBER(zodiac_state::write_o)
{
	// O0-O7: digit segment/led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(zodiac_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

/* The physical button layout and labels are like this:

      [P]                                [A]
                [ 1 ] [ 2 ] [ 3 ]
    [L]         [ 4 ] [ 5 ] [ 6 ]          [D]
                [ 7 ] [ 8 ] [ 9 ]
      [J]       [ 0 ] [CLR] [ENT]        [E]

                  [.__.__.__.]
                  OFF H  P  A

Despite that this layout features a typical digital keypad and distances
the letter buttons from it, the 8-character encoding for date input uses
letters and digits in combination. This fact and the use of the P key are
why the digit buttons are mapped here as keyboard inputs rather than as a
keypad.
*/

static INPUT_PORTS_START( zodiac )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Clear") PORT_CHAR(8)

	PORT_START("IN.5") // R8
	PORT_CONFNAME( 0x03, 0x01, "Mode")
	PORT_CONFSETTING(    0x01, "Horoscope" )
	PORT_CONFSETTING(    0x02, "Preview" )
	PORT_CONFSETTING(    0x00, "Answer" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// output PLA is not decapped
static const u16 zodiac_output_pla[0x20] =
{
	0x80,                   // empty/led 1/7
	lC,                     // i/led 2/8
	lE+lG,                  // r/led 3/9
	lC+lE+lG,               // n
	lF,                     // seg F/led 4/10
	0,                      // ?
	0,                      // ?
	lC+lE+lF+lG,            // h
	lB,                     // seg B/led 5/11
	lD,                     // seg D/led 6/12
	0,                      // ?
	0,                      // ?
	0,                      // ?
	0,                      // ?
	lA+lB+lE+lF+lG,         // P
	0,                      // ?
	lA+lB+lC+lD+lE+lF,      // 0
	lB+lC,                  // 1
	lA+lB+lD+lE+lG,         // 2
	lA+lB+lC+lD+lG,         // 3
	lB+lC+lF+lG,            // 4
	lA+lC+lD+lF+lG,         // 5
	lA+lC+lD+lE+lF+lG,      // 6
	lA+lB+lC,               // 7
	lA+lB+lC+lD+lE+lF+lG,   // 8
	lA+lB+lC+lD+lF+lG,      // 9
	lA+lB+lC+lE+lF+lG,      // A
	lB+lC+lD+lE+lG,         // d
	lA+lD+lE+lF+lG,         // E
	lB+lC+lD+lE,            // J
	lD+lE+lF,               // L
	lB+lC+lD+lE+lF          // U
};

void zodiac_state::zodiac(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 500000); // approximation - RC osc. R=18K, C=100pF
	m_maincpu->set_output_pla(zodiac_output_pla);
	m_maincpu->k().set(FUNC(zodiac_state::read_k));
	m_maincpu->r().set(FUNC(zodiac_state::write_r));
	m_maincpu->o().set(FUNC(zodiac_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_zodiac);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Electronic Quarterback (model 2120)
  * TMS1100NLL MP3415 (die label same)
  * 9-digit LED grid, 1-bit sound

  known releases:
  - USA(1): Electronic Quarterback
  - USA(2): Electronic Touchdown, published by Sears

***************************************************************************/

class cqback_state : public hh_tms1k_state
{
public:
	cqback_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void cqback(machine_config &config);
};

// handlers

void cqback_state::prepare_display()
{
	// R9 selects between segments B/C or A'/D'
	u16 seg = m_o;
	if (m_r & 0x200)
		seg = (m_o << 7 & 0x300) | (m_o & 0xf9);

	set_display_segmask(0x1ff, 0xff);
	display_matrix(11, 9, seg, m_r & 0x1ff);
}

WRITE16_MEMBER(cqback_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R0-R9: select digit/segment
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(cqback_state::write_o)
{
	// O0-O7: digit segments
	m_o = data;
	prepare_display();
}

READ8_MEMBER(cqback_state::read_k)
{
	// K: multiplexed inputs, rotated matrix
	return read_rotated_inputs(5);
}

// config

static INPUT_PORTS_START( cqback )
	PORT_START("IN.0") // K1
	PORT_BIT( 0x01, 0x01, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // left/right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Kick/Pass")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Display")

	PORT_START("IN.1") // K2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_TOGGLE PORT_NAME("Play Selector") // pass
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("IN.1", 0x01, EQUALS, 0x00) // run/kick

	PORT_START("IN.2") // K4
	PORT_CONFNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x01, "2" )

	PORT_START("IN.3") // K8
	PORT_CONFNAME( 0x01, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) ) // TP1-TP2

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
INPUT_PORTS_END

void cqback_state::cqback(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 310000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(cqback_state::read_k));
	m_maincpu->r().set(FUNC(cqback_state::write_r));
	m_maincpu->o().set(FUNC(cqback_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cqback);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Head to Head: Electronic Football (model 2140)
  * TMS1100NLLE (rev. E!) MP3460 (die label same)
  * 2*SN75492N LED display drivers, 9-digit LED grid, 1-bit sound

  LED electronic football game. To distinguish between offense and defense,
  offense blips (should) appear brighter. The hardware is similar to cqback.

  known releases:
  - USA(1): Head to Head: Electronic Football
  - USA(2): Team Play Football, published by Sears

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
	void h2hfootb(machine_config &config);
};

// handlers

void h2hfootb_state::prepare_display()
{
	set_display_segmask(0x1ff, 0x7f);
	display_matrix(9, 9, m_o | (m_r >> 1 & 0x100), m_r & 0x1ff);
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
	// K: multiplexed inputs, rotated matrix
	return read_rotated_inputs(9);
}

// config

static INPUT_PORTS_START( h2hfootb )
	PORT_START("IN.0") // K1
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.1") // K2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_TOGGLE PORT_NAME("P1 Play Selector") // pass
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("IN.1", 0x01, EQUALS, 0x00) // run/kick

	PORT_START("IN.2") // K4
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.3") // K8
	PORT_BIT( 0x001, 0x001, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // left/right
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Kick/Pass")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Display")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
INPUT_PORTS_END

void h2hfootb_state::h2hfootb(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 310000); // approximation - RC osc. R=39K, C=100pF
	m_maincpu->k().set(FUNC(h2hfootb_state::read_k));
	m_maincpu->r().set(FUNC(h2hfootb_state::write_r));
	m_maincpu->o().set(FUNC(h2hfootb_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_h2hfootb);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Head to Head: Electronic Basketball (model 2150)
  * TMS1000NLL MP3320A (die label 1000E MP3320A)
  * 2-digit 7seg LED display, LED grid display, 1-bit sound

  Coleco Head to Head: Electronic Hockey (model 2160)
  * TMS1000NLL E MP3321A (die label 1000E MP3321A)
  * same PCB/hardware as above

  Unlike the COP420 version(see hh_cop400.cpp driver), each game has its own MCU.

***************************************************************************/

class h2hbaskb_state : public hh_tms1k_state
{
public:
	h2hbaskb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_cap_empty_timer(*this, "cap_empty")
	{ }

	required_device<timer_device> m_cap_empty_timer;
	TIMER_DEVICE_CALLBACK_MEMBER(cap_empty_callback);
	bool m_cap_state;
	attotime m_cap_charge;

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void h2hbaskb(machine_config &config);
	void h2hhockey(machine_config &config);

protected:
	virtual void machine_start() override;
};

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(h2hbaskb_state::cap_empty_callback)
{
	if (~m_r & 0x200)
		m_cap_state = false;
}

void h2hbaskb_state::prepare_display()
{
	// R6,R7 are commons for R0-R5
	u16 sel = 0;
	if (m_r & 0x40) sel |= (m_r & 0x3f);
	if (m_r & 0x80) sel |= (m_r & 0x3f) << 6;

	set_display_segmask(0xc0, 0x7f);
	display_matrix(7, 6+6, m_o, sel);
}

WRITE16_MEMBER(h2hbaskb_state::write_r)
{
	// R0-R3: input mux
	m_inp_mux = (data & 0xf);

	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9: K8 and 15uF cap to V- (used as timer)
	// rising edge, remember the time
	if (data & ~m_r & 0x200)
	{
		m_cap_state = true;
		m_cap_charge = machine().time();
	}
	// falling edge, determine how long K8 should stay up
	if (~data & m_r & 0x200)
	{
		const attotime full = attotime::from_usec(1300); // approx. charge time
		const int factor = 27; // approx. factor for charge/discharge to logic 0

		attotime charge = machine().time() - m_cap_charge;
		if (charge > full)
			charge = full;

		m_cap_empty_timer->adjust(charge * factor);
	}

	// R0-R7: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(h2hbaskb_state::write_o)
{
	// O1-O7: led data
	m_o = data >> 1 & 0x7f;
	prepare_display();
}

READ8_MEMBER(h2hbaskb_state::read_k)
{
	// K1-K4: multiplexed inputs, K8: R9 and capacitor
	return (read_inputs(4) & 7) | (m_cap_state ? 8 : 0);
}

// config

static INPUT_PORTS_START( h2hbaskb )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Pass CW") // clockwise
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Pass CCW") // counter-clockwise
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start/Display")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Left")
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( h2hhockey )
	PORT_INCLUDE( h2hbaskb )

	PORT_MODIFY("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Left")
INPUT_PORTS_END

void h2hbaskb_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill/register for savestates
	m_cap_state = false;
	m_cap_charge = attotime::zero;

	save_item(NAME(m_cap_state));
	save_item(NAME(m_cap_charge));
}

void h2hbaskb_state::h2hbaskb(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=43K, C=100pF
	m_maincpu->k().set(FUNC(h2hbaskb_state::read_k));
	m_maincpu->r().set(FUNC(h2hbaskb_state::write_r));
	m_maincpu->o().set(FUNC(h2hbaskb_state::write_o));

	TIMER(config, "cap_empty").configure_generic(FUNC(h2hbaskb_state::cap_empty_callback));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_h2hbaskb);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void h2hbaskb_state::h2hhockey(machine_config &config)
{
	h2hbaskb(config);
	config.set_default_layout(layout_h2hhockey);
}





/***************************************************************************

  Coleco Head to Head: Electronic Baseball (model 2180)
  * PCB labels Coleco rev C 73891/2
  * TMS1170NLN MP1525-N2 (die label MP1525)
  * 9-digit cyan VFD display, and other LEDs behind bezel, 1-bit sound

  known releases:
  - USA: Head to Head: Electronic Baseball
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
	void h2hbaseb(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void h2hbaseb_state::prepare_display()
{
	set_display_segmask(0x1ff, 0x7f);
	display_matrix(9, 9, (m_r & 0x100) | m_o, (m_r & 0xff) | (m_r >> 1 & 0x100));
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Fast Pitch") // "
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // Vss!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Swing")

	PORT_START("IN.5") // fake
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, h2hbaseb_state, skill_switch, nullptr)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(h2hbaseb_state::skill_switch)
{
	set_clock();
}

void h2hbaseb_state::set_clock()
{
	// MCU clock is from an RC circuit with C=47pF, and R value is depending on
	// skill switch: R=51K(1) or 43K(2)
	m_maincpu->set_unscaled_clock((m_inp_matrix[5]->read() & 1) ? 400000 : 350000);
}

void h2hbaseb_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void h2hbaseb_state::h2hbaseb(machine_config &config)
{
	/* basic machine hardware */
	TMS1170(config, m_maincpu, 350000); // see set_clock
	m_maincpu->k().set(FUNC(h2hbaseb_state::read_k));
	m_maincpu->r().set(FUNC(h2hbaseb_state::write_r));
	m_maincpu->o().set(FUNC(h2hbaseb_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_h2hbaseb);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Head to Head: Electronic Boxing (model 2190)
  * TMS1100NLL M34018-N2 (die label M34018)
  * 2-digit 7seg LED display, LED grid display, 1-bit sound

  This appears to be the last game of Coleco's Head to Head series.

***************************************************************************/

class h2hboxing_state : public hh_tms1k_state
{
public:
	h2hboxing_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void h2hboxing(machine_config &config);
};

// handlers

void h2hboxing_state::prepare_display()
{
	set_display_segmask(0x600, 0x7f);
	display_matrix(8, 11, m_o, m_r);
}

WRITE16_MEMBER(h2hboxing_state::write_r)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R0-R7: select led
	// R9,R10: select digit
	m_r = data & ~0x100;
	prepare_display();
}

WRITE16_MEMBER(h2hboxing_state::write_o)
{
	// O0-O7: digit segments/led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(h2hboxing_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( h2hboxing )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Punch / Pro")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Block / Amateur")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Punch")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Block")
INPUT_PORTS_END

void h2hboxing_state::h2hboxing(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=39K, C=100pF
	m_maincpu->k().set(FUNC(h2hboxing_state::read_k));
	m_maincpu->r().set(FUNC(h2hboxing_state::write_r));
	m_maincpu->o().set(FUNC(h2hboxing_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_h2hboxing);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Coleco Quiz Wiz Challenger
  * TMS1000NLL M32001-N2 (die label 1000E, M32001)
  * 4 7seg LEDs, 17 other LEDs, 1-bit sound

  This is a 4-player version of Quiz Wiz, a multiple choice quiz game.
  According to the manual, Quiz Wiz cartridges are compatible with it.
  The question books are needed to play, as well as optional game pieces.

  Cartridge pinout:
  1 R4
  2 R6
  3 R7
  4 K1
  5 N/C
  6 R8
  7 R5
  8 R9

  The cartridge connects one or more of the R pins to K1. Together with the
  question numbers, the game generates a pseudo-random answerlist.

***************************************************************************/

class quizwizc_state : public hh_tms1k_state
{
public:
	quizwizc_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_pinout(0)
	{ }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);
	u16 m_pinout; // cartridge R pins

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void quizwizc(machine_config &config);

protected:
	virtual void machine_start() override;
};

// handlers

DEVICE_IMAGE_LOAD_MEMBER(quizwizc_state, cartridge)
{
	if (!image.loaded_through_softlist())
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Can only load through softwarelist");
		return image_init_result::FAIL;
	}

	// get cartridge pinout K1 to R connections
	std::string pinout(image.get_feature("pinout"));
	m_pinout = std::stoul(pinout, nullptr, 2) & 0xe7;
	m_pinout = bitswap<8>(m_pinout,4,3,7,5,2,1,6,0) << 4;

	return image_init_result::PASS;
}

void quizwizc_state::prepare_display()
{
	// R6-R9 are 7segs
	set_display_segmask(0x3c0, 0x7f);

	// note: O7 is on VSS
	display_matrix(8, 11, m_o, m_r | 0x400);
}

WRITE16_MEMBER(quizwizc_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R4-R9: to cartridge slot
	m_inp_mux = data & 0x3f;

	// R0-R3: led select
	// R6-R9: digit select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(quizwizc_state::write_o)
{
	// O0-O7: led/digit segment data
	m_o = bitswap<8>(data,7,0,1,2,3,4,5,6);
	prepare_display();
}

READ8_MEMBER(quizwizc_state::read_k)
{
	// K: multiplexed inputs
	// K1: cartridge pin 4 (pin 5 N/C)
	return read_inputs(6) | ((m_r & m_pinout) ? 1 : 0);
}

// config

static INPUT_PORTS_START( quizwizc )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // A (player 1 at bottom, increment counter-clockwise)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // B
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) // C
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) // D

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3)

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Fast Forward")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Slow Forward")

	PORT_START("IN.5") // R5
	PORT_CONFNAME( 0x02, 0x00, "Game Select" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void quizwizc_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_pinout));
}

void quizwizc_state::quizwizc(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 300000); // approximation - RC osc. R=43K, C=100pF
	m_maincpu->k().set(FUNC(quizwizc_state::read_k));
	m_maincpu->r().set(FUNC(quizwizc_state::write_r));
	m_maincpu->o().set(FUNC(quizwizc_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_quizwizc);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "quizwiz_cart"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(device_image_load_delegate(&quizwizc_state::device_image_load_cartridge, this));
	SOFTWARE_LIST(config, "cart_list").set_original("quizwiz");
}





/***************************************************************************

  Coleco Total Control 4
  * TMS1400NLL MP7334-N2 (die label MP7334)
  * 2x2-digit 7seg LED display + 4 LEDs, LED grid display, 1-bit sound

  This is a head to head electronic tabletop LED-display sports console.
  One cartridge(Football) was included with the console, the other three were
  sold in a pack. Gameplay has emphasis on strategy, read the official manual
  on how to play. MAME external artwork is needed for the switchable overlays.

  Cartridge pinout:
  1 N/C
  2 9V+
  3 power switch
  4 K8
  5 K4
  6 K2
  7 K1
  8 R9

  The cartridge connects pin 8 with one of the K-pins.

***************************************************************************/

class tc4_state : public hh_tms1k_state
{
public:
	tc4_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_pinout(0)
	{ }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);
	u8 m_pinout; // cartridge K pins

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void tc4(machine_config &config);

protected:
	virtual void machine_start() override;
};

// handlers

DEVICE_IMAGE_LOAD_MEMBER(tc4_state, cartridge)
{
	if (!image.loaded_through_softlist())
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Can only load through softwarelist");
		return image_init_result::FAIL;
	}

	// get cartridge pinout R9 to K connections
	std::string pinout(image.get_feature("pinout"));
	m_pinout = std::stoul(pinout, nullptr, 0) & 0xf;

	return image_init_result::PASS;
}

void tc4_state::prepare_display()
{
	// R5,R7-R9 are 7segs
	set_display_segmask(0x3a0, 0x7f);

	// note: R6 is an extra column
	display_matrix(9, 10, (m_o | (m_r << 2 & 0x100)), m_r);
}

WRITE16_MEMBER(tc4_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R9: to cartridge slot
	m_inp_mux = data & 0x3f;

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
	// K: multiplexed inputs, cartridge pins from R9
	return read_inputs(6) | ((m_r & 0x200) ? m_pinout : 0);
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
INPUT_PORTS_END

void tc4_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_pinout));
}

void tc4_state::tc4(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 450000); // approximation - RC osc. R=27.3K, C=100pF
	m_maincpu->k().set(FUNC(tc4_state::read_k));
	m_maincpu->r().set(FUNC(tc4_state::write_r));
	m_maincpu->o().set(FUNC(tc4_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_tc4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "tc4_cart"));
	cartslot.set_must_be_loaded(true); // system won't power on without cartridge
	cartslot.set_device_load(device_image_load_delegate(&tc4_state::device_image_load_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("tc4");
}





/***************************************************************************

  Conic Electronic Basketball
  * PCB label CONIC 101-006
  * TMS1000NLL MP0907 (die label 1000B MP0907)
  * DS8871N, 2 7seg LEDs, 30 other LEDs, 1-bit sound

  There are 3 known versions of Conic Basketball: MP0910(101-003) and
  MP0907(101-006) are nearly identical. MP0168 is found in Conic Multisport.

  known releases:
  - Hong Kong: Electronic Basketball
  - USA: Electronic Basketball, published by Cardinal

***************************************************************************/

class cnbaskb_state : public hh_tms1k_state
{
public:
	cnbaskb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void cnbaskb(machine_config &config);
};

// handlers

void cnbaskb_state::prepare_display()
{
	// R7,R8 are 7segs
	set_display_segmask(0x180, 0x7f);
	display_matrix(7, 9, m_o, m_r & 0x1fc);
}

WRITE16_MEMBER(cnbaskb_state::write_r)
{
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0,R1: input mux
	// R10 is also tied to K1 (locks up at boot if it's not handled)
	m_inp_mux = (data >> 8 & 4) | (data & 3);

	// R2-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(cnbaskb_state::write_o)
{
	// O0-O6: led/digit segment data
	// O7: N/C
	m_o = data;
	prepare_display();
}

READ8_MEMBER(cnbaskb_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( cnbaskb )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // amateur
	PORT_CONFSETTING(    0x01, "2" ) // professional
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.2") // R10
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void cnbaskb_state::cnbaskb(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(cnbaskb_state::read_k));
	m_maincpu->r().set(FUNC(cnbaskb_state::write_r));
	m_maincpu->o().set(FUNC(cnbaskb_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cnbaskb);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Conic Electronic Multisport
  * PCB label CONIC 101-027(1979), or CONIC 101-021 REV A(1980, with DS8871N)
  * TMS1000 MP0168 (die label same)
  * 2 7seg LEDs, 33 other LEDs, 1-bit sound

  This handheld includes 3 games: Basketball, Ice Hockey, Soccer.
  MAME external artwork is needed for the switchable overlays.

  known releases:
  - Hong Kong: Electronic Multisport
  - Hong Kong: Basketball/Ice Hockey/Soccer (3 separate handhelds)
  - USA(1): Electronic Multisport, published by Innocron
  - USA(2): Sports Arena, published by Tandy (model 60-2158)

***************************************************************************/

class cmsport_state : public hh_tms1k_state
{
public:
	cmsport_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void cmsport(machine_config &config);
};

// handlers

void cmsport_state::prepare_display()
{
	// R5,R6 are 7segs
	set_display_segmask(0x60, 0x7f);
	display_matrix(8, 9, m_o, m_r & ~0x80);
}

WRITE16_MEMBER(cmsport_state::write_r)
{
	// R7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// R0,R9,R10: input mux
	m_inp_mux = (data & 1) | (data >> 8 & 6);

	// R0-R6,R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(cmsport_state::write_o)
{
	// O0-O7: led/digit segment data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(cmsport_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( cmsport )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x05, 0x01, "Game Select" )
	PORT_CONFSETTING(    0x01, "Basketball" )
	PORT_CONFSETTING(    0x00, "Soccer" )
	PORT_CONFSETTING(    0x04, "Ice Hockey" )
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.2") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Score")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // amateur
	PORT_CONFSETTING(    0x08, "2" ) // professional
INPUT_PORTS_END

void cmsport_state::cmsport(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(cmsport_state::read_k));
	m_maincpu->r().set(FUNC(cmsport_state::write_r));
	m_maincpu->o().set(FUNC(cmsport_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cmsport);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Conic Electronic Football
  * TMS1000 MP0170 (die label same)
  * DS8874N, 3*9 LED array, 7 7seg LEDs, 1-bit sound

  This is a clone of Mattel Football. Apparently Mattel tried to keep imports
  of infringing games from going through customs. Conic (Hong Kong) answered
  by distributing the game under subsidiary brands - see list below.

  known releases:
  - Hong Kong: Electronic Football, Conic
  - USA(1): Football, E.R.S.(Electronic Readout Systems)
  - USA(2): Football, ELECsonic
  - USA(3): Football, no brand!

  Another hardware revision of this game uses a PIC16 MCU.

***************************************************************************/

class cnfball_state : public hh_tms1k_state
{
public:
	cnfball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void cnfball(machine_config &config);
};

// handlers

void cnfball_state::prepare_display()
{
	// declare 7segs, middle ones have DP
	set_display_segmask(0xc3, 0x7f);
	set_display_segmask(0x38, 0xff);

	display_matrix(8+3, 10, m_o | (m_r << 6 & 0x700), m_grid);
}

WRITE16_MEMBER(cnfball_state::write_r)
{
	// R5,R8: N/C
	// R6,R7: speaker out
	m_speaker->level_w(data >> 6 & 3);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0: DS8874N CP: clock pulse edge-triggered
	// note: DS8874N CP goes back to K8 too, game relies on it
	if ((data ^ m_r) & 1)
		m_grid = m_grid << 1 & 0x1ff;

	// R1: DS8874N _DATA: reset shift register
	if (~data & 2)
		m_grid = 1;

	// R2-R4: led data
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(cnfball_state::write_o)
{
	// O0-O7: digit segments
	m_o = data;
	prepare_display();
}

READ8_MEMBER(cnfball_state::read_k)
{
	// K: multiplexed inputs, K8 also tied to DS8874N CP(R0)
	return read_inputs(2) | (m_r << 3 & 8);
}

// config

static INPUT_PORTS_START( cnfball )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // left/right
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Score")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Status")
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x08, "1" ) // college
	PORT_CONFSETTING(    0x00, "2" ) // professional

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
INPUT_PORTS_END

static const s16 cnfball_speaker_levels[4] = { 0, 0x7fff, -0x8000, 0 };

void cnfball_state::cnfball(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(cnfball_state::read_k));
	m_maincpu->r().set(FUNC(cnfball_state::write_r));
	m_maincpu->o().set(FUNC(cnfball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cnfball);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, cnfball_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Conic Electronic Football II
  * TMS1100 MP1181 (no decap)
  * 9-digit LED grid, 1-bit sound

  This is a clone of Coleco's Quarterback, similar at hardware-level too.

  known releases:
  - Hong Kong: Electronic Football II, Conic
  - USA: Electronic Football II, Tandy Radio Shack

***************************************************************************/

class cnfball2_state : public hh_tms1k_state
{
public:
	cnfball2_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void cnfball2(machine_config &config);
};

// handlers

void cnfball2_state::prepare_display()
{
	// R1 selects between segments B/C or A'/D'
	u16 seg = m_o;
	if (~m_r & 2)
		seg = (m_o << 7 & 0x300) | (m_o & 0xf9);

	set_display_segmask(0x1ff, 0xff);
	display_matrix(11, 9, seg, m_r >> 1 & 0x1ff);
}

WRITE16_MEMBER(cnfball2_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R8-R10: input mux
	m_inp_mux = data >> 8 & 7;

	// R1-R10: select digit/segment
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(cnfball2_state::write_o)
{
	// O0-O7: digit segments
	m_o = data;
	prepare_display();
}

READ8_MEMBER(cnfball2_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( cnfball2 )
	PORT_START("IN.0") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_TOGGLE PORT_NAME("Play Selector") // pass/run
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // college
	PORT_CONFSETTING(    0x08, "2" ) // professional

	PORT_START("IN.1") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.2") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Kick/Pass")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Score")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Status")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// output PLA is not decapped
static const u16 cnfball2_output_pla[0x20] =
{
	// first half was dumped electronically
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40, 0x01, 0x08, 0x02, 0x04, 0x00,

	// rest is unknown
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

void cnfball2_state::cnfball2(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->set_output_pla(cnfball2_output_pla);
	m_maincpu->k().set(FUNC(cnfball2_state::read_k));
	m_maincpu->r().set(FUNC(cnfball2_state::write_r));
	m_maincpu->o().set(FUNC(cnfball2_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cnfball2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Conic Electronic I.Q.
  * PCB labels: main: CONIC 101-037 (other side: HG-15, 11*00198*00), button PCB:
    CONIC 102-001, led PCB: CONIC 100-003 REV A itac
  * TMS1000NLL MP0908 (die label 1000B, MP0908)
  * 2 7seg LEDs, 30 other LEDs, 1-bit sound

  This is a peg solitaire game, with random start position.

  known releases:
  - Hong Kong: Electronic I.Q.
  - UK: Solitaire, published by Grandstand

***************************************************************************/

class eleciq_state : public hh_tms1k_state
{
public:
	eleciq_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	void eleciq(machine_config &config);
};

// handlers

void eleciq_state::prepare_display()
{
	// R7,R8 are 7segs
	set_display_segmask(0x180, 0x7f);
	display_matrix(7, 9, m_o, m_r & ~1);
}

WRITE16_MEMBER(eleciq_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R1-R6,R9: input mux
	m_inp_mux = (data >> 1 & 0x3f) | (data >> 3 & 0x40);

	// R1-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(eleciq_state::write_o)
{
	// O0-O6: led/digit segment data
	// O7: N/C
	m_o = data;
	prepare_display();
}

READ8_MEMBER(eleciq_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(7);
}

// config

static INPUT_PORTS_START( eleciq )
	PORT_START("IN.0") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Button A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Up-Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Up")

	PORT_START("IN.1") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Button B")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Up-Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Down")

	PORT_START("IN.2") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Button C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Down-Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Left")

	PORT_START("IN.3") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Button D")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Down-Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right")

	PORT_START("IN.4") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Button E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Button 5")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Button F")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R9
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // amateur
	PORT_CONFSETTING(    0x01, "2" ) // professional
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, eleciq_state, reset_button, nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(eleciq_state::reset_button)
{
	// reset button is directly wired to TMS1000 INIT pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

void eleciq_state::eleciq(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=50pF
	m_maincpu->k().set(FUNC(eleciq_state::read_k));
	m_maincpu->r().set(FUNC(eleciq_state::write_r));
	m_maincpu->o().set(FUNC(eleciq_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_eleciq);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex (Electronic) Soccer
  * TMS1000NL MP0158 (die label same)
  * 2 7seg LEDs, 30 other LEDs, 1-bit sound

  known releases:
  - USA: Electronic Soccer, 2 versions (leds on green bezel, or leds under bezel)
  - Germany: Fussball, with skill switch

***************************************************************************/

class esoccer_state : public hh_tms1k_state
{
public:
	esoccer_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void esoccer(machine_config &config);
};

// handlers

void esoccer_state::prepare_display()
{
	// R8,R9 are 7segs
	set_display_segmask(0x300, 0x7f);
	display_matrix(7, 10, m_o, m_r);
}

WRITE16_MEMBER(esoccer_state::write_r)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(esoccer_state::write_o)
{
	// O0-O6: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(esoccer_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( esoccer )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // R2
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" ) // Auto
	PORT_CONFSETTING(    0x02, "2" ) // Manual
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
INPUT_PORTS_END

void esoccer_state::esoccer(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=33pF
	m_maincpu->k().set(FUNC(esoccer_state::read_k));
	m_maincpu->r().set(FUNC(esoccer_state::write_r));
	m_maincpu->o().set(FUNC(esoccer_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_esoccer);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex (Electronic) Baseball (1)
  * TMS1000NLP MP0914 (die label MP0914A)
  * 1 7seg LED, and other LEDs behind bezel, 1-bit sound

  This is a handheld LED baseball game. One player controls the batter, the CPU
  or other player controls the pitcher. Pitcher throw buttons are on a 'joypad'
  obtained from a compartment in the back. Player scores are supposed to be
  written down manually, the game doesn't save scores or innings (this annoyance
  was resolved in the sequel). For more information, refer to the official manual.

  The overlay graphic is known to have 2 versions: one where the field players
  are denoted by words ("left", "center", "short", etc), and an alternate one
  with little guys drawn next to the LEDs.

  led translation table: led LDzz from game PCB = MAME y.x:

    0 = -     10 = 1.2   20 = 4.2   30 = 6.0
    1 = 2.3   11 = 0.4   21 = 4.1   31 = 6.1
    2 = 0.0   12 = 1.5   22 = 4.0   32 = 6.2
    3 = 0.1   13 = 2.2   23 = 4.3   33 = 7.0
    4 = 0.2   14 = 3.3   24 = 5.3   34 = 7.1
    5 = 1.0   15 = 3.2   25 = 5.2
    6 = 1.3   16 = 2.1   26 = 5.1
    7 = 1.1   17 = 3.1   27 = 5.0
    8 = 0.3   18 = 3.0   28 = 7.2
    9 = 1.4   19 = 2.0   29 = 7.3

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
	void ebball(machine_config &config);
};

// handlers

void ebball_state::prepare_display()
{
	// R8 is a 7seg
	set_display_segmask(0x100, 0x7f);
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
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Players ) )
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

void ebball_state::ebball(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=43K, C=47pF
	m_maincpu->k().set(FUNC(ebball_state::read_k));
	m_maincpu->r().set(FUNC(ebball_state::write_r));
	m_maincpu->o().set(FUNC(ebball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ebball);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex (Electronic) Baseball 2
  * PCB label ZENY
  * TMS1000 MCU, MP0923 (die label same)
  * 3 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  The Japanese version was published by Gakken, black case instead of white.

  The sequel to Entex Baseball, this version keeps up with score and innings.
  As its predecessor, the pitcher controls are on a separate joypad.

  led translation table: led zz from game PCB = MAME y.x:

    00 = -     10 = 9.4   20 = 7.4   30 = 5.0
    01 = 5.3   11 = 9.3   21 = 7.5   31 = 5.1
    02 = 0.7   12 = 9.2   22 = 8.0   32 = 5.2
    03 = 1.7   13 = 6.2   23 = 8.1   33 = 4.0
    04 = 2.7   14 = 7.0   24 = 8.2   34 = 4.1
    05 = 9.7   15 = 7.1   25 = 8.3   35 = 3.1
    06 = 9.0   16 = 6.1   26 = 8.4   36 = 3.0
    07 = 9.5   17 = 7.2   27 = 8.5   37 = 3.3
    08 = 6.3   18 = 7.3   28 = 4.2   38 = 3.2
    09 = 9.1   19 = 6.0   29 = 4.3

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
	void ebball2(machine_config &config);
};

// handlers

void ebball2_state::prepare_display()
{
	// R0-R2 are 7segs
	set_display_segmask(7, 0x7f);
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
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Players ) )
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

void ebball2_state::ebball2(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(ebball2_state::read_k));
	m_maincpu->r().set(FUNC(ebball2_state::write_r));
	m_maincpu->o().set(FUNC(ebball2_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ebball2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex (Electronic) Baseball 3
  * PCB label ZENY
  * TMS1100NLL 6007 MP1204 (rev. E!) (die label MP1204)
  * 2*SN75492N LED display driver
  * 4 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  This is another improvement over Entex Baseball, where gameplay is a bit more
  varied. Like the others, the pitcher controls are on a separate joypad.

  led translation table: led zz from game PCB = MAME y.x:
  note: unlabeled panel leds are listed here as Sz, Bz, Oz, Iz, z left-to-right

    00 = -     10 = 7.5   20 = 7.2
    01 = 6.0   11 = 6.5   21 = 8.4
    02 = 6.1   12 = 5.5   22 = 8.5
    03 = 6.2   13 = 5.2   23 = 9.0
    04 = 6.3   14 = 8.0   24 = 9.2
    05 = 7.3   15 = 8.1   25 = 9.1
    06 = 5.3   16 = 5.1   26 = 9.3
    07 = 7.4   17 = 8.2   27 = 9.5
    08 = 6.4   18 = 8.3   28 = 9.4
    09 = 5.4   19 = 5.0

    S1,S2: 4.0,4.1
    B1-B3: 3.0-3.2
    O1,O2: 4.2,4.3
    I1-I6: 2.0-2.5, I7-I9: 3.3-3.5

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
	void ebball3(machine_config &config);

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
	set_display_segmask(3, 0x7f);

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
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Bunt")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // fake
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, ebball3_state, skill_switch, nullptr)
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(ebball3_state::skill_switch)
{
	set_clock();
}

void ebball3_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=33pF) oscillating by default at ~340kHz,
	// but on PRO, the difficulty switch adds an extra 150K resistor to Vdd to speed
	// it up to around ~440kHz.
	m_maincpu->set_unscaled_clock((m_inp_matrix[3]->read() & 1) ? 440000 : 340000);
}

void ebball3_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void ebball3_state::ebball3(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 340000); // see set_clock
	m_maincpu->k().set(FUNC(ebball3_state::read_k));
	m_maincpu->r().set(FUNC(ebball3_state::write_r));
	m_maincpu->o().set(FUNC(ebball3_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ebball3);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex Space Battle
  * TMS1000 EN-6004 MP0920 (die label 1000B, MP0920)
  * 2 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  The Japanese version was published by Gakken, same name.

  led translation table: led zz from game PCB = MAME y.x:

    0 = -     10 = 1.1   20 = 2.3   30 = 5.2
    1 = 0.0   11 = 1.2   21 = 3.0   31 = 5.3
    2 = 0.1   12 = 1.3   22 = 3.1   32 = 6.0
    3 = 0.2   13 = 1.4   23 = 3.2   33 = 6.1
    4 = 0.3   14 = 1.5   24 = 3.3   34 = 6.2
    5 = 0.4   15 = 1.6   25 = 4.0   35 = 7.0
    6 = 0.5   16 = 1.7   26 = 4.1   36 = 7.1
    7 = 0.6   17 = 2.0   27 = 4.2
    8 = 0.7   18 = 2.1   28 = 5.0
    9 = 1.0   19 = 2.2   29 = 5.1

***************************************************************************/

class esbattle_state : public hh_tms1k_state
{
public:
	esbattle_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void esbattle(machine_config &config);
};

// handlers

void esbattle_state::prepare_display()
{
	// R8,R9 are 7segs
	set_display_segmask(0x300, 0x7f);
	display_matrix(8, 10, m_o, m_r);
}

WRITE16_MEMBER(esbattle_state::write_r)
{
	// R0,R1: input mux
	m_inp_mux = data & 3;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(esbattle_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(esbattle_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(2);
}

// config

static INPUT_PORTS_START( esbattle )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Fire 1") // F1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Fire 2") // F2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Launch")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Fire 1") // F1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Fire 2") // F2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Launch")
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
INPUT_PORTS_END

void esbattle_state::esbattle(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 425000); // approximation - RC osc. R=47K, C=33pF
	m_maincpu->k().set(FUNC(esbattle_state::read_k));
	m_maincpu->r().set(FUNC(esbattle_state::write_r));
	m_maincpu->o().set(FUNC(esbattle_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_esbattle);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex Space Invader
  * TMS1100 MP1211 (die label same)
  * 3 7seg LEDs, LED matrix and overlay mask, 1-bit sound

  There are two versions of this game: the first release(this one) is on
  TMS1100, the second more widespread release runs on a COP400. There are
  also differences with the overlay mask.

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
	void einvader(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void einvader_state::prepare_display()
{
	// R7-R9 are 7segs
	set_display_segmask(0x380, 0x7f);
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
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, einvader_state, skill_switch, nullptr)
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x08, "Professional" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(einvader_state::skill_switch)
{
	set_clock();
}

void einvader_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=56pF) oscillating by default at ~320kHz,
	// but on PRO, the difficulty switch adds an extra 180K resistor to Vdd to speed
	// it up to around ~400kHz.
	m_maincpu->set_unscaled_clock((m_inp_matrix[0]->read() & 8) ? 400000 : 320000);
}

void einvader_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void einvader_state::einvader(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 320000); // see set_clock
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(einvader_state::write_r));
	m_maincpu->o().set(FUNC(einvader_state::write_o));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(939, 1080);
	screen.set_visarea(0, 939-1, 0, 1080-1);
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_einvader);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex Color Football 4
  * TMS1670 6009 MP7551 (die label MP7551)
  * 9-digit cyan VFD display, 60 red and green LEDs behind bezel, 1-bit sound

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
	void efootb4(machine_config &config);
};

// handlers

void efootb4_state::prepare_display()
{
	// R10-R15 are 7segs
	set_display_segmask(0xfc00, 0x7f);
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
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" ) // Auto
	PORT_CONFSETTING(    0x00, "2" ) // Manual
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Status")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void efootb4_state::efootb4(machine_config &config)
{
	/* basic machine hardware */
	TMS1670(config, m_maincpu, 475000); // approximation - RC osc. R=42K, C=47pF
	m_maincpu->k().set(FUNC(efootb4_state::read_k));
	m_maincpu->r().set(FUNC(efootb4_state::write_r));
	m_maincpu->o().set(FUNC(efootb4_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_efootb4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex (Electronic) Basketball 2
  * TMS1100 6010 MP1218 (die label MP1218)
  * 4 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  led translation table: led zz from game PCB = MAME y.x:

    11 = 9.0   21 = 9.1   31 = 9.2   41 = 9.3   51 = 9.5
    12 = 8.0   22 = 8.1   32 = 8.2   42 = 8.3   52 = 8.5
    13 = 7.0   23 = 7.1   33 = 7.2   43 = 7.3   53 = 8.4
    14 = 6.0   24 = 6.1   34 = 6.2   44 = 6.3   54 = 7.5
    15 = 5.0   25 = 5.1   35 = 5.2   45 = 5.3   55 = 7.4
    16 = 4.0   26 = 4.1   36 = 4.2   46 = 4.3   56 = 6.5

    A  = 9.4
    B  = 6.4

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
	void ebaskb2(machine_config &config);
};

// handlers

void ebaskb2_state::prepare_display()
{
	// R0-R3 are 7segs
	set_display_segmask(0xf, 0x7f);
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
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "Amateur" )
	PORT_CONFSETTING(    0x00, "Professional" )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Players ) )
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

void ebaskb2_state::ebaskb2(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 360000); // approximation - RC osc. R=33K, C=82pF
	m_maincpu->k().set(FUNC(ebaskb2_state::read_k));
	m_maincpu->r().set(FUNC(ebaskb2_state::write_r));
	m_maincpu->o().set(FUNC(ebaskb2_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ebaskb2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Entex Raise The Devil
  * TMS1100 MP1221 (die label same)
  * 4 7seg LEDs(rightmost one unused), and other LEDs behind bezel, 1-bit sound

  led translation table: led zz from game PCB = MAME y.x:

    0 = -     10 = 4.4   20 = 5.3   30 = 9.5   40 = 9.2
    1 = 3.0   11 = 4.5   21 = 5.4   31 = 8.5   41 = 9.3
    2 = 3.1   12 = -     22 = -     32 = 6.5   42 = 9.0
    3 = 3.2   13 = 5.0   23 = 6.0   33 = 7.4   43 = 9.1
    4 = 3.3   14 = 5.1   24 = 6.1   34 = 7.0
    5 = 3.4   15 = 5.2   25 = 6.2   35 = 7.1
    6 = 4.0   16 = -     26 = 6.3   36 = 8.0
    7 = 4.1   17 = 7.2   27 = 6.4   37 = 8.1
    8 = 4.2   18 = 7.3   28 = 8.4   38 = 8.2
    9 = 4.3   19 = -     29 = 9.4   39 = 8.3

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
	void raisedvl(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void raisedvl_state::prepare_display()
{
	// R0-R2 are 7segs
	set_display_segmask(7, 0x7f);
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
	PORT_CONFNAME( 0x31, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, raisedvl_state, skill_switch, nullptr)
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
	// MCU clock is from an RC circuit with C=47pF, R=47K by default. Skills
	// 2 and 3 add a 150K resistor in parallel, and skill 4 adds a 100K one.
	// 0:   R=47K  -> ~350kHz
	// 2,3: R=35K8 -> ~425kHz (combined)
	// 4:   R=32K  -> ~465kHz (combined)
	u8 inp = m_inp_matrix[1]->read();
	m_maincpu->set_unscaled_clock((inp & 0x20) ? 465000 : ((inp & 0x10) ? 425000 : 350000));
}

void raisedvl_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void raisedvl_state::raisedvl(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 350000); // see set_clock
	m_maincpu->k().set(FUNC(raisedvl_state::read_k));
	m_maincpu->r().set(FUNC(raisedvl_state::write_r));
	m_maincpu->o().set(FUNC(raisedvl_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_raisedvl);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Fonas 2 Player Baseball
  * TMS1000NLL MP0154 (die label 1000B, MP0154)
  * 4 7seg LEDs, 37 other LEDs, 1-bit sound

  known releases:
  - World: 2 Player Baseball
  - USA: 2 Player Baseball, published by Sears
  - Canada: 2 Player Baseball, published by Talbot Electronics

  led translation table: led zz from game PCB = MAME y.x:

    0 = -     10 = 2.2   20 = 4.0   30 = 4.4
    1 = 2.3   11 = 3.3   21 = 2.7   31 = 3.7
    2 = 0.4   12 = 1.2   22 = 0.0   32 = 4.3
    3 = 3.2   13 = 2.4   23 = 4.1   33 = 4.6
    4 = 0.5   14 = 1.0   24 = 3.1   34 = 3.5
    5 = 0.3   15 = 2.1   25 = 0.2   35 = 4.5
    6 = 3.4   16 = 1.1   26 = 0.1
    7 = 1.3   17 = 4.7   27 = 4.2
    8 = 1.4   18 = 2.0   28 = 3.0
    9 = 1.7   19 = 0.7   29 = 1.5

***************************************************************************/

class f2pbball_state : public hh_tms1k_state
{
public:
	f2pbball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	void f2pbball(machine_config &config);
};

// handlers

void f2pbball_state::prepare_display()
{
	// R5-R8 are 7segs
	set_display_segmask(0x1e0, 0x7f);
	display_matrix(8, 9, m_o, m_r);
}

WRITE16_MEMBER(f2pbball_state::write_r)
{
	// R4,R9,R10: input mux
	m_inp_mux = (data >> 4 & 1) | (data >> 8 & 6);

	// R9,R10(ANDed together): speaker out
	m_speaker->level_w(data >> 10 & data >> 9 & 1);

	// R0-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(f2pbball_state::write_o)
{
	// O0-O7: led state
	m_o = bitswap<8>(data,0,7,6,5,4,3,2,1);
	prepare_display();
}

READ8_MEMBER(f2pbball_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( f2pbball )
	PORT_START("IN.0") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Pick Off")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x0c, 0x04, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "Practice" ) // middle switch
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.1") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Steal")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pitch")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Swing")

	PORT_START("IN.2") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_COCKTAIL PORT_NAME("P2 Curve Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Slow")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Curve Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Fast")

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, f2pbball_state, reset_button, nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(f2pbball_state::reset_button)
{
	// reset button is directly wired to TMS1000 INIT pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

void f2pbball_state::f2pbball(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=51K, C=39pF
	m_maincpu->k().set(FUNC(f2pbball_state::read_k));
	m_maincpu->r().set(FUNC(f2pbball_state::write_r));
	m_maincpu->o().set(FUNC(f2pbball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_f2pbball);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Fonas 3 in 1: Football, Basketball, Soccer
  * TMS1100NLL MP1185
  * 4 7seg LEDs, 40 other LEDs, 1-bit sound

  It's not known if this game has an official title. The current one is
  taken from the handheld front side.
  MAME external artwork is needed for the switchable overlays.

***************************************************************************/

class f3in1_state : public hh_tms1k_state
{
public:
	f3in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);
	void f3in1(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void f3in1_state::prepare_display()
{
	// R6-R9 are 7segs
	set_display_segmask(0x3c0, 0x7f);
	display_matrix(8, 10, m_o, m_r & ~0x20);
}

WRITE16_MEMBER(f3in1_state::write_r)
{
	// R0-R2,R4: input mux
	m_inp_mux = (data & 7) | (data >> 1 & 8);

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4,R6-R9: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(f3in1_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
}

READ8_MEMBER(f3in1_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( f3in1 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("K Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("D Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x0e, 0x02, "Game Select" )
	PORT_CONFSETTING(    0x02, "Football" )
	PORT_CONFSETTING(    0x04, "Basketball" )
	PORT_CONFSETTING(    0x08, "Soccer" )

	PORT_START("IN.4") // fake
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, f3in1_state, skill_switch, nullptr)
	PORT_CONFSETTING(    0x00, "Regular" ) // REG
	PORT_CONFSETTING(    0x01, "Professional" ) // PROF
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(f3in1_state::skill_switch)
{
	set_clock();
}

void f3in1_state::set_clock()
{
	// MCU clock is from an RC circuit where C=47pF, R=39K(PROF) or 56K(REG)
	m_maincpu->set_unscaled_clock((m_inp_matrix[4]->read() & 1) ? 400000 : 300000);
}

void f3in1_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void f3in1_state::f3in1(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 300000); // see set_clock
	m_maincpu->k().set(FUNC(f3in1_state::read_k));
	m_maincpu->r().set(FUNC(f3in1_state::write_r));
	m_maincpu->o().set(FUNC(f3in1_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_f3in1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken Poker
  * PCB label POKER. gakken
  * TMS1370 MP2105 (die label same)
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
	void gpoker(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void gpoker_state::prepare_display()
{
	set_display_segmask(0x7ff, 0x20ff); // 7seg + bottom-right diagonal
	u16 segs = bitswap<16>(m_o, 15,14,7,12,11,10,9,8,6,6,5,4,3,2,1,0) & 0x20ff;
	display_matrix(14, 11, segs | (m_r >> 3 & 0xf00), m_r & 0x7ff);
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

void gpoker_state::gpoker(machine_config &config)
{
	/* basic machine hardware */
	TMS1370(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(gpoker_state::read_k));
	m_maincpu->r().set(FUNC(gpoker_state::write_r));
	m_maincpu->o().set(FUNC(gpoker_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gpoker);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2405); // astable multivibrator - C1 and C2 are 0.003uF, R1 and R4 are 1K, R2 and R3 are 100K
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken Jackpot: Gin Rummy & Black Jack
  * PCB label gakken
  * TMS1670 MPF553 (die label same)
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
	void gjackpot(machine_config &config);
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

void gjackpot_state::gjackpot(machine_config &config)
{
	/* basic machine hardware */
	TMS1670(config, m_maincpu, 450000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(gpoker_state::read_k));
	m_maincpu->r().set(FUNC(gjackpot_state::write_r));
	m_maincpu->o().set(FUNC(gpoker_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gjackpot);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2405); // see gpoker
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken Galaxy Invader 1000
  * TMS1370 MP2139 (die label 1170 MP2139)
  * cyan/red VFD display Futaba DM-25Z 2D, 1-bit sound

  known releases:
  - World: Galaxy Invader 1000
  - Japan: Invader 1000
  - USA(1): Galaxy Invader 1000, published by CGL
  - USA(2): Cosmic 1000 Fire Away, published by Tandy

***************************************************************************/

class ginv1000_state : public hh_tms1k_state
{
public:
	ginv1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);
	void ginv1000(machine_config &config);
};

// handlers

void ginv1000_state::prepare_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,9,8,7);
	u16 plate = bitswap<16>(m_plate,15,14,13,12,3,4,7,8,9,10,11,2,6,5,1,0);
	display_matrix(12, 10, plate, grid);
}

WRITE16_MEMBER(ginv1000_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R8,R15: input mux
	m_inp_mux = (data >> 8 & 1) | (data >> 14 & 2);

	// R1-R10: VFD grid
	// R11-R14: VFD plate
	m_grid = data >> 1 & 0x3ff;
	m_plate = (m_plate & 0xff) | (data >> 3 & 0xf00);
	prepare_display();
}

WRITE16_MEMBER(ginv1000_state::write_o)
{
	// O0-O7: VFD plate
	m_plate = (m_plate & ~0xff) | data;
	prepare_display();
}

READ8_MEMBER(ginv1000_state::read_k)
{
	// K1,K2: multiplexed inputs (K8 is fire button)
	return m_inp_matrix[2]->read() | read_inputs(2);
}

// config

static INPUT_PORTS_START( ginv1000 )
	PORT_START("IN.0") // R8
	PORT_CONFNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFSETTING(    0x02, "3" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R15
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // K8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
INPUT_PORTS_END

void ginv1000_state::ginv1000(machine_config &config)
{
	/* basic machine hardware */
	TMS1370(config, m_maincpu, 340000); // approximation
	m_maincpu->k().set(FUNC(ginv1000_state::read_k));
	m_maincpu->r().set(FUNC(ginv1000_state::write_r));
	m_maincpu->o().set(FUNC(ginv1000_state::write_o));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(226, 1080);
	screen.set_visarea(0, 226-1, 0, 1080-1);
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken Invader 2000
  * TMS1370(28 pins) MP1604 (die label 1370A MP1604)
  * TMS1024 I/O expander
  * cyan/red/green VFD display, 1-bit sound

  known releases:
  - World: Invader 2000
  - USA(1): Galaxy Invader 10000, published by CGL
  - USA(2): Cosmic 3000 Fire Away, published by Tandy

***************************************************************************/

class ginv2000_state : public hh_tms1k_state
{
public:
	ginv2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_expander(*this, "expander")
	{ }

	required_device<tms1024_device> m_expander;
	DECLARE_WRITE8_MEMBER(expander_w);

	void prepare_display();
	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);
	void ginv2000(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void ginv2000_state::prepare_display()
{
	display_matrix(16, 10, m_plate, m_grid);
}

WRITE8_MEMBER(ginv2000_state::expander_w)
{
	// TMS1024 port 4-7: VFD plate
	int shift = (offset - tms1024_device::PORT4) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	prepare_display();
}

WRITE16_MEMBER(ginv2000_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R11,R12: input mux
	m_inp_mux = data >> 11 & 3;

	// R11,R12: TMS1024 S1,S0 (S2 forced high)
	// R13: TMS1024 STD
	m_expander->write_s(space, 0, (data >> 12 & 1) | (data >> 10 & 2) | 4);
	m_expander->write_std(data >> 13 & 1);

	// R1-R10: VFD grid
	m_grid = data >> 1 & 0x3ff;
	prepare_display();
}

WRITE16_MEMBER(ginv2000_state::write_o)
{
	// O4-O7: TMS1024 H1-H4
	m_expander->write_h(space, 0, data >> 4 & 0xf);
}

READ8_MEMBER(ginv2000_state::read_k)
{
	// K1,K2: multiplexed inputs (K8 is fire button)
	return m_inp_matrix[2]->read() | read_inputs(2);
}

// config

static INPUT_PORTS_START( ginv2000 )
	PORT_START("IN.0") // R11
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R12
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // K8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
INPUT_PORTS_END

void ginv2000_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	m_expander->write_ms(1); // Vss
}

void ginv2000_state::ginv2000(machine_config &config)
{
	/* basic machine hardware */
	TMS1370(config, m_maincpu, 425000); // approximation - RC osc. R=36K, C=47pF
	m_maincpu->k().set(FUNC(ginv2000_state::read_k));
	m_maincpu->r().set(FUNC(ginv2000_state::write_r));
	m_maincpu->o().set(FUNC(ginv2000_state::write_o));

	TMS1024(config, m_expander);
	m_expander->write_port4_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port5_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port6_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port7_callback().set(FUNC(ginv2000_state::expander_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(364, 1080);
	screen.set_visarea(0, 364-1, 0, 1080-1);
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Gakken FX-Micom R-165
  * TMS1100 MCU, label MP1312 (die label MP1312A)
  * 1 7seg led, 6 other leds, 1-bit sound

  This is a simple educational home computer. Refer to the extensive manual
  for more information. It was published later in the USA by Tandy Radio Shack,
  under their Science Fair series. Another 25 years later, Gakken re-released
  the R-165 as GMC-4, obviously on modern hardware, but fully compatible.

  known releases:
  - Japan: FX-Micom R-165
  - USA: Science Fair Microcomputer Trainer, published by Tandy Radio Shack.
    Of note is the complete redesign of the case, adding more adjustable wiring

***************************************************************************/

class fxmcr165_state : public hh_tms1k_state
{
public:
	fxmcr165_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void fxmcr165(machine_config &config);
};

// handlers

void fxmcr165_state::prepare_display()
{
	// 7seg digit from O0-O6
	m_display_segmask[0] = 0x7f;
	m_display_state[0] = bitswap<8>(m_o,7,2,6,5,4,3,1,0) & 0x7f;

	// leds from R4-R10
	m_display_state[1] = m_r >> 4 & 0x7f;
	set_display_size(7, 2);
	display_update();
}

WRITE16_MEMBER(fxmcr165_state::write_r)
{
	// R0-R3: input mux low
	m_inp_mux = (m_inp_mux & 0x10) | (data & 0xf);

	// R7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// R4-R10: led data (direct)
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(fxmcr165_state::write_o)
{
	// O7: input mux high
	m_inp_mux = (m_inp_mux & 0xf) | (data >> 3 & 0x10);

	// O0-O6: digit segments (direct)
	m_o = data;
	prepare_display();
}

READ8_MEMBER(fxmcr165_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

/* physical button layout and labels is like this:

    [C]   [D]   [E]   [F]   [ADR SET]
    [8]   [9]   [A]   [B]   [INCR]
    [4]   [5]   [6]   [7]   [RUN]
    [0]   [1]   [2]   [3]   [RESET]
*/

static INPUT_PORTS_START( fxmcr165 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("C")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("D")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("E")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("F")

	PORT_START("IN.4") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Reset")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Run")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Increment")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Address Set")
INPUT_PORTS_END

void fxmcr165_state::fxmcr165(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->k().set(FUNC(fxmcr165_state::read_k));
	m_maincpu->r().set(FUNC(fxmcr165_state::write_r));
	m_maincpu->o().set(FUNC(fxmcr165_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fxmcr165);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Ideal Electronic Detective
  * TMS0980NLL MP6100A (die label 0980B-00)
  * 10-digit 7seg LED display, 2-level sound

  hardware (and concept) is very similar to Parker Brothers Stop Thief

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
	void elecdet(machine_config &config);
};

// handlers

WRITE16_MEMBER(elecdet_state::write_r)
{
	// R7,R8: speaker out
	m_speaker->level_w((m_o & 0x80) ? (data >> 7 & 3) : 0);

	// R0-R6: select digit
	set_display_segmask(0x7f, 0x7f);
	display_matrix(7, 7, bitswap<8>(m_o,7,5,2,1,4,0,6,3), data);
}

WRITE16_MEMBER(elecdet_state::write_o)
{
	// O0,O1,O4,O6: input mux
	m_inp_mux = (data & 3) | (data >> 2 & 4) | (data >> 3 & 8);

	// O0-O6: digit segments A-G
	// O7: speaker on -> write_r
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("End Turn")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
INPUT_PORTS_END

static const s16 elecdet_speaker_levels[4] = { 0, 0x3fff, 0x3fff, 0x7fff };

void elecdet_state::elecdet(machine_config &config)
{
	/* basic machine hardware */
	TMS0980(config, m_maincpu, 425000); // approximation
	m_maincpu->k().set(FUNC(elecdet_state::read_k));
	m_maincpu->r().set(FUNC(elecdet_state::write_r));
	m_maincpu->o().set(FUNC(elecdet_state::write_o));
	m_maincpu->power_off().set(FUNC(hh_tms1k_state::auto_power_off));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_elecdet);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, elecdet_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Kenner Star Wars - Electronic Battle Command
  * TMS1100 MCU, label MP3438A
  * 4x4 LED grid display + 2 separate LEDs and 2-digit 7segs, 1-bit sound

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
	void starwbc(machine_config &config);
};

// handlers

void starwbc_state::prepare_display()
{
	// R6,R8 are 7segs
	set_display_segmask(0x140, 0x7f);
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Basic Game")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Intermediate Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Advanced Game")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Player Number")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Start Turn")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("End Turn")

	PORT_START("IN.2") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Magna Scan") // only used in adv. game
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Enemy Scan") // only used in adv. game
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Screen Up")

	PORT_START("IN.3") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Evasive Maneuvers")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Fire")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Battle Stations")

	PORT_START("IN.4") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
INPUT_PORTS_END

void starwbc_state::starwbc(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 325000); // approximation - RC osc. R=51K, C=47pF
	m_maincpu->k().set(FUNC(starwbc_state::read_k));
	m_maincpu->r().set(FUNC(starwbc_state::write_r));
	m_maincpu->o().set(FUNC(starwbc_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_starwbc);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Kosmos Astro
  * TMS1470NLHL MP1133 (die label TMS1400 MP1133)
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
	void astro(machine_config &config);
};

// handlers

void astro_state::prepare_display()
{
	set_display_segmask(0x3ff, 0xff);
	display_matrix(8, 10, m_o, m_r);
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

void astro_state::astro(machine_config &config)
{
	/* basic machine hardware */
	TMS1470(config, m_maincpu, 450000); // approximation - RC osc. R=4.7K, C=33pF
	m_maincpu->k().set(FUNC(astro_state::read_k));
	m_maincpu->r().set(FUNC(astro_state::write_r));
	m_maincpu->o().set(FUNC(astro_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_astro);

	/* no sound! */
}





/***************************************************************************

  Marx Series 300 Electronic Bowling Game
  * TMS1100NLL MP3403 DBS 7836 SINGAPORE (no decap)
  * 4*SN75492 quad segment driver, 2*SN74259 8-line demultiplexer,
    2*CD4043 quad r/s input latch
  * 5 7seg LEDs, 15 lamps(10 lamps projected to bowling pins reflection),
    1bit-sound with crude volume control
  * edge connector to sensors(switches trigger when ball rolls over)
    and other inputs

  lamp translation table: SN74259.u5(mux 1) goes to MAME output 5.x,
  SN74259.u6(mux 2) goes to MAME output 6.*. u1-u3 are SN75492 ICs,
  where other: u1 A2 is N/C, u3 A1 is from O2 and goes to digits seg C.

    u5 Q0 -> u1 A4 -> L2 (pin #2)       u6 Q0 -> u3 A4 -> L1 (pin #1)
    u5 Q1 -> u1 A5 -> L4 (pin #4)       u6 Q1 -> u3 A5 -> L5 (pin #5)
    u5 Q2 -> u1 A6 -> L7 (pin #7)       u6 Q2 -> u2 A3 -> L11 (player 1)
    u5 Q3 -> u1 A1 -> L8 (pin #8)       u6 Q3 -> u2 A2 -> L12 (player 2)
    u5 Q4 -> u3 A2 -> L3 (pin #3)       u6 Q4 -> u2 A1 -> L15 (?)
    u5 Q5 -> u2 A6 -> L6 (pin #6)       u6 Q5 -> u3 A6 -> L14 (?)
    u5 Q6 -> u2 A5 -> L10 (pin #10)     u6 Q6 -> u1 A3 -> L13 (spare)
    u5 Q7 -> u2 A4 -> L9 (pin #9)       u6 Q7 -> u3 A3 -> digit 4 B+C

***************************************************************************/

class elecbowl_state : public hh_tms1k_state
{
public:
	elecbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void elecbowl(machine_config &config);
};

// handlers

void elecbowl_state::prepare_display()
{
	// standard 7segs
	for (int y = 0; y < 4; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (m_r >> (y + 4) & 1) ? m_o : 0;
	}

	// lamp muxes
	u8 mask = 1 << (m_o & 7);
	u8 d = (m_r & 2) ? mask : 0;
	if (~m_r & 1)
		m_display_state[5] = (m_display_state[5] & ~mask) | d;
	if (~m_r & 4)
		m_display_state[6] = (m_display_state[6] & ~mask) | d;

	// digit 4 is from mux2 Q7
	m_display_segmask[4] = 6;
	m_display_state[4] = (m_display_state[6] & 0x80) ? 6 : 0;

	set_display_size(8, 7);
	display_update();
}

WRITE16_MEMBER(elecbowl_state::write_r)
{
	// R5-R7,R10: input mux
	m_inp_mux = (data >> 5 & 7) | (data >> 7 & 8);

	// R9: speaker out
	// R3,R8: speaker volume..
	m_speaker->level_w(data >> 9 & 1);

	// R4-R7: select digit
	// R0,R2: lamp mux1,2 _enable
	// R1: lamp muxes state
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(elecbowl_state::write_o)
{
	//if (data & 0x80) printf("%X ",data&0x7f);

	// O0-O2: lamp muxes select
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(elecbowl_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( elecbowl )
	PORT_START("IN.0") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1") // R6
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // reset/test?
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // reset/test?

	PORT_START("IN.2") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.3") // R10
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) // 2 players sw?
INPUT_PORTS_END

// output PLA is not decapped
static const u16 elecbowl_output_pla[0x20] =
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

	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0,1,2,3,4,5,6,7,        // lamp muxes select
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f
};

void elecbowl_state::elecbowl(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->set_output_pla(elecbowl_output_pla);
	m_maincpu->k().set(FUNC(elecbowl_state::read_k));
	m_maincpu->r().set(FUNC(elecbowl_state::write_r));
	m_maincpu->o().set(FUNC(elecbowl_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_elecbowl);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Mattel Thoroughbred Horse Race Analyzer
  * PCB label 1670-4619D
  * TMS1100NLL MP3491-N2 (die label 1100E MP3491)
  * HLCD0569, 67-segment LCD panel, no sound

  This handheld is not a toy, read the manual for more information. In short,
  it is a device for prediciting the winning chance of a gambling horserace.

  known releases:
  - USA: Thoroughbred Horse Race Analyzer
  - China/Canada: Thoroughbred Horse Race Analyzer, published in 1994 by
    Advanced Handicapping Technologies, Inc.

***************************************************************************/

class horseran_state : public hh_tms1k_state
{
public:
	horseran_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_lcd(*this, "lcd")
	{ }

	required_device<hlcd0569_device> m_lcd;

	DECLARE_WRITE32_MEMBER(lcd_output_w);
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_READ8_MEMBER(read_k);
	void horseran(machine_config &config);
};

// handlers

WRITE32_MEMBER(horseran_state::lcd_output_w)
{
	// only 3 rows used
	if (offset > 2)
		return;

	// update lcd segments
	display_matrix(24, 3, data, 1 << offset, false);

	// col5-11 and col13-19 are 7segs
	for (int i = 0; i < 2; i++)
		m_display_state[3 + (offset << 1 | i)] = bitswap<8>(data >> (4+8*i),7,3,5,2,0,1,4,6) & 0x7f;

	set_display_segmask(0x3f<<3, 0x7f);
	set_display_size(24, 3+6);
	display_update();
}

WRITE16_MEMBER(horseran_state::write_r)
{
	// R0: HLCD0569 clock
	// R1: HLCD0569 data in
	// R2: HLCD0569 _CS
	m_lcd->write_cs(data >> 2 & 1);
	m_lcd->write_data(data >> 1 & 1);
	m_lcd->write_clock(data & 1);

	// R3-R10: input mux
	m_inp_mux = data >> 3 & 0xff;
}

READ8_MEMBER(horseran_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(8);
}

// config

/* physical button layout and labels is like this:

    [PURSE]      [DIST.]      [P. POSN.]   [DAYS]       [R.S.L.]     [7]     [8]     [9]
    [RACES]      [WINS]       [PLACES]     [SHOWS]      [EARNINGS]   [4]     [5]     [6]
    [DISTANCE]   [L.E.B. 1st] [L.E.B. 2nd] [L.E.B. 3rd] [FIN. POSN.] [1]     [2]     [3]
    [L.E.B. Fin] [SPD. RTG.]  [SPD. RTG.]  [ANALYZE]    [NEXT]       [C/H]   [C/E]   [0]

    R.S.L. = Races Since (Last) Layoff
    L.E.B. = Last Race / Lengths Back
*/

static INPUT_PORTS_START( horseran )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Next")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Final Position")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Earnings")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("R.S.L.")

	PORT_START("IN.1") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Analyze")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("L.E.B. 3rd")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Shows")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Days")

	PORT_START("IN.2") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Speed Rating (right)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("L.E.B. 2nd")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Places")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Post Position")

	PORT_START("IN.3") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Speed Rating (left)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("L.E.B. 1st")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Wins")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Distance (L.E.B.)")

	PORT_START("IN.4") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("L.E.B. Finish")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Distance")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Races")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Purse")

	PORT_START("IN.5") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.6") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("C/E") // Clear Entry
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.7") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("C/H") // Clear Horse Information
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
INPUT_PORTS_END

void horseran_state::horseran(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 300000); // approximation - RC osc. R=56K, C=47pF
	m_maincpu->k().set(FUNC(horseran_state::read_k));
	m_maincpu->r().set(FUNC(horseran_state::write_r));

	/* video hardware */
	HLCD0569(config, m_lcd, 1100); // C=0.022uF
	m_lcd->write_cols().set(FUNC(horseran_state::lcd_output_w));
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_horseran);

	/* no sound! */
}





/***************************************************************************

  Mattel Dungeons & Dragons - Computer Labyrinth Game
  * TMS1100 M34012-N2LL (die label M34012)
  * 72 buttons, no LEDs, 1-bit sound

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
	void mdndclab(machine_config &config);
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

static INPUT_PORTS_START( mdndclab )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.5") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.6") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.7") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.8") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Wall / Door")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Illegal Move / Warrior Moves")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Warrior 1 / Winner")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Warrior 2 / Treasure")

	PORT_START("IN.9") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.10") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.11") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.12") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.13") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.14") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.15") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.16") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Board Sensor")

	PORT_START("IN.17") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Switch Key")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Next Turn / Level 1/2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Dragon Flying / Defeat Tune")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Dragon Attacks / Dragon Wakes")
INPUT_PORTS_END

void mdndclab_state::mdndclab(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 475000); // approximation - RC osc. R=27K, C=100pF
	m_maincpu->k().set(FUNC(mdndclab_state::read_k));
	m_maincpu->r().set(FUNC(mdndclab_state::write_r));
	m_maincpu->o().set(FUNC(mdndclab_state::write_o));

	/* no visual feedback! */
	config.set_default_layout(layout_mdndclab); // playing board

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die label 4A0970D-04A)
  * 10 LEDs behind bezel, no sound

  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.

  known releases:
  - USA: Comp IV (two versions, different case)
  - Europe: Logic 5
  - Japan: Pythaligoras

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
	void comp4(machine_config &config);
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

void comp4_state::comp4(machine_config &config)
{
	/* basic machine hardware */
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(comp4_state::read_k));
	m_maincpu->r().set(FUNC(comp4_state::write_r));
	m_maincpu->o().set(FUNC(comp4_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_comp4);

	/* no sound! */
}





/***************************************************************************

  Milton Bradley Electronic Battleship (1977 version, model 4750A)
  * PCB label 4750A
  * TMS1000NL MP3201 (die label 1000C, MP3201)
  * LM324N, MC14016CP/TP4016AN, NE555P, discrete sound
  * 4 sliding buttons, light bulb

  This is a 2-player electronic board game. It still needs game pieces like the
  original Battleship board game.

  It went through at least 5 hardware revisions (not counting Talking Battleship):
  1977: model 4750A, TMS1000 discrete sound, see notes above
  1977: model 4750B, TMS1000 SN76477 sound, see notes at bshipb (driver below this)
  1979: model 4750C: cost-reduced single-chip design, lesser quality game board.
        The chip is assumed to be custom, no MCU: 28-pin DIP, label 4750, SCUS 0462
  1982: similar custom single-chip hardware, chip label MB4750 SCUS 0562
  1982: back to MCU, COP420 instead of choosing TI, emulated in hh_cop400.cpp

***************************************************************************/

class bship_state : public hh_tms1k_state
{
public:
	bship_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void bship(machine_config &config);
};

// handlers

WRITE16_MEMBER(bship_state::write_r)
{
	// R0-R10: input mux
	m_inp_mux = data;
}

WRITE16_MEMBER(bship_state::write_o)
{
	// O4: explosion light bulb
	display_matrix(1, 1, data >> 4 & 1, 1);

	// other: sound
}

READ8_MEMBER(bship_state::read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[11]->read() | read_inputs(11);
}

// config

static INPUT_PORTS_START( bship )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P1 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P1 A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 A")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P1 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 B")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P1 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 C")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 C")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P1 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("P1 D")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 D")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P1 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("P1 E")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 E")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P1 6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("P1 F")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 F")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P1 7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("P1 G")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 G")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P1 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("P1 H")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 H")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P1 9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 I")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 I")

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P1 10")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 J")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 J")

	PORT_START("IN.10") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P1 Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Fire")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_TOGGLE PORT_CODE(KEYCODE_F1) PORT_NAME("Load/Go") // switch

	PORT_START("IN.11") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("P1 Clear Memory") // CM
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("P1 Clear Last Entry") // CLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Clear Memory")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Clear Last Entry")
INPUT_PORTS_END

void bship_state::bship(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set(FUNC(bship_state::read_k));
	m_maincpu->r().set(FUNC(bship_state::write_r));
	m_maincpu->o().set(FUNC(bship_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_bship);

	/* sound hardware */
	// TODO
}





/***************************************************************************

  Milton Bradley Electronic Battleship (1977 version, model 4750B)
  * PCB label MB 4750B
  * TMS1000NLL MP3208 (die label 1000C, MP3208)
  * SN75494N (acting as inverters), SN76477 sound
  * 4 sliding buttons, light bulb

  This is the 2nd version. The sound hardware was changed to a SN76477.

***************************************************************************/

class bshipb_state : public hh_tms1k_state
{
public:
	bshipb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_sn(*this, "sn76477")
	{ }

	required_device<sn76477_device> m_sn;

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void bshipb(machine_config &config);
};

// handlers

WRITE16_MEMBER(bshipb_state::write_r)
{
	// R0-R10: input mux
	m_inp_mux = data;

	// R4: 75494 to R12 33K to SN76477 pin 20
	m_sn->slf_res_w((data & 0x10) ? RES_INF : RES_K(33));
}

WRITE16_MEMBER(bshipb_state::write_o)
{
	//printf("%X ", m_maincpu->debug_peek_o_index() & 0xf);

	// O0: SN76477 pin 9
	m_sn->enable_w(data & 1);

	// O1: 75494 to R4 100K to SN76477 pin 18
	// O2: 75494 to R3 150K to SN76477 pin 18
	double o12 = RES_INF;
	switch (~data >> 1 & 3)
	{
		case 0: o12 = RES_INF; break;
		case 1: o12 = RES_K(100); break;
		case 2: o12 = RES_K(150); break;
		case 3: o12 = RES_2_PARALLEL(RES_K(100), RES_K(150)); break;
	}
	m_sn->vco_res_w(o12);

	// O2,O6: (TODO) to SN76477 pin 21
	//m_sn->slf_cap_w(x);

	// O4: SN76477 pin 22
	m_sn->vco_w(data >> 4 & 1);

	// O5: R11 27K to SN76477 pin 23
	m_sn->one_shot_cap_w((data & 0x20) ? RES_K(27) : 0);

	// O6: SN76477 pin 25
	m_sn->mixer_b_w(data >> 6 & 1);

	// O7: 75494 to light bulb
	display_matrix(1, 1, data >> 7 & 1, 1);
}

READ8_MEMBER(bshipb_state::read_k)
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inp_matrix[11]->read() | read_inputs(11);
}

// config

// buttons are same as bship set

void bshipb_state::bshipb(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set(FUNC(bshipb_state::read_k));
	m_maincpu->r().set(FUNC(bshipb_state::write_r));
	m_maincpu->o().set(FUNC(bshipb_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_bship);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(47), RES_K(100), CAP_P(47));   // R18, R17, C8
	m_sn->set_decay_res(RES_M(3.3));                            // R16
	m_sn->set_attack_params(CAP_U(0.47), RES_K(15));            // C7, R20
	m_sn->set_amp_res(RES_K(100));                              // R19
	m_sn->set_feedback_res(RES_K(39));                          // R7
	m_sn->set_vco_params(5.0 * RES_VOLTAGE_DIVIDER(RES_K(47), RES_K(33)), CAP_U(0.01), RES_K(270)); // R15/R14, C5, switchable R5/R3/R4
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(22), RES_K(750));    // switchable C4, switchable R13/R12
	m_sn->set_oneshot_params(0, RES_INF);           // NC, switchable R11
	m_sn->set_vco_mode(0);                          // switchable
	m_sn->set_mixer_params(0, 0, 0);                // switchable, GND, GND
	m_sn->set_envelope_params(1, 0);                // Vreg, GND
	m_sn->set_enable(0);                            // switchable
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.35);
}





/***************************************************************************

  Milton Bradley Simon (model 4850), created by Ralph Baer
  * TMS1000 (die label MP3226), or MP3300 (die label 1000C, MP3300)
  * DS75494 Hex digit LED driver, 4 big lamps, 1-bit sound

  known revisions:
  - 1978: Rev A: TMS1000(no label)
  - 198?: Rev B: MB4850 SCUS0640(16-pin custom ASIC), PCB label REV.B,
    cost-reduced, same hardware as Pocket Simon
  - 1979: Rev F: TMS1000(MP3300), PCB label 4850 Rev F

  The semi-sequel Super Simon uses a TMS1100 (see next minidriver).

***************************************************************************/

class simon_state : public hh_tms1k_state
{
public:
	simon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_READ8_MEMBER(read_k);
	void simon(machine_config &config);
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
	PORT_CONFNAME( 0x0f, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )
INPUT_PORTS_END

void simon_state::simon(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(simon_state::read_k));
	m_maincpu->r().set(FUNC(simon_state::write_r));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_simon);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Milton Bradley Super Simon
  * TMS1100 MP3476NLL (die label MP3476)
  * 8 big lamps(2 turn on at same time), 1-bit sound

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
	void ssimon(machine_config &config);

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
	PORT_CONFNAME( 0x0f, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Head-to-Head" ) // this sets R10 K2, see below
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )

	PORT_START("IN.5") // R10
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("IN.4", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // fake
	PORT_CONFNAME( 0x03, 0x01, "Speed" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssimon_state, speed_switch, nullptr)
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
	// MCU clock is from an RC circuit with C=100pF, R=x depending on speed switch:
	// 0 Simple: R=51K -> ~200kHz
	// 1 Normal: R=37K -> ~275kHz
	// 2 Super:  R=22K -> ~400kHz
	u8 inp = m_inp_matrix[6]->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 400000 : ((inp & 1) ? 275000 : 200000));
}

void ssimon_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

void ssimon_state::ssimon(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 275000); // see set_clock
	m_maincpu->k().set(FUNC(ssimon_state::read_k));
	m_maincpu->r().set(FUNC(ssimon_state::write_r));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ssimon);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Milton Bradley Big Trak
  * TMS1000NLL MP3301A or MP3301ANLL E (rev. E!) (die label 1000E MP3301)
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
	void bigtrak(machine_config &config);

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
	output().set_value("left_motor_forward", data >> 1 & 1);
	output().set_value("left_motor_reverse", data >> 2 & 1);
	output().set_value("right_motor_forward", data >> 3 & 1);
	output().set_value("right_motor_reverse", data >> 4 & 1);
	output().set_value("ext_out", data >> 5 & 1);

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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Input Port")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void bigtrak_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill/register for savestates
	m_gearbox_pos = 0;
	save_item(NAME(m_gearbox_pos));
}

static const s16 bigtrak_speaker_levels[8] = { 0, 0x7fff/3, 0x7fff/3, 0x7fff/3*2, 0x7fff/3, 0x7fff/3*2, 0x7fff/3*2, 0x7fff };

void bigtrak_state::bigtrak(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=83K, C=100pF
	m_maincpu->k().set(FUNC(bigtrak_state::read_k));
	m_maincpu->r().set(FUNC(bigtrak_state::write_r));
	m_maincpu->o().set(FUNC(bigtrak_state::write_o));

	TIMER(config, "gearbox").configure_periodic(FUNC(bigtrak_state::gearbox_sim_tick), attotime::from_msec(1));
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_bigtrak);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(8, bigtrak_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Milton Bradley Dark Tower
  * TMS1400NLL MP7332-N1.U1(Rev. B) or MP7332-N2LL(Rev. C) (die label MP7332)
    (assume same ROM contents between revisions)
  * SN75494N MOS-to-LED digit driver
  * motorized rotating reel + lightsensor, 1bit-sound

  This is a board game, it obviously requires game pieces and the board.
  The emulated part is the centerpiece, a black tower with a rotating card
  panel and LED digits for displaying health, amount of gold, etc. As far
  as MAME is concerned, the game works fine.

  To start up the game, first press [MOVE], the machine now does a self-test.
  Then select level and number of players and the game will start. Read the
  official manual on how to play the game.

***************************************************************************/

class mbdtower_state : public hh_tms1k_state
{
public:
	mbdtower_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	bool sensor_led_on() { return m_display_decay[0][0] != 0; }

	int m_motor_pos;
	int m_motor_pos_prev;
	int m_motor_decay;
	bool m_motor_on;
	bool m_sensor_blind;

	TIMER_DEVICE_CALLBACK_MEMBER(motor_sim_tick);

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void mbdtower(machine_config &config);

protected:
	virtual void machine_start() override;
};

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(mbdtower_state::motor_sim_tick)
{
	// it rotates counter-clockwise (when viewed from above)
	if (m_motor_on)
	{
		m_motor_pos = (m_motor_pos - 1) & 0x7f;

		// give it some time to spin out when it's turned off
		if (m_r & 0x200)
			m_motor_decay += (m_motor_decay < 4);
		else if (m_motor_decay > 0)
			m_motor_decay--;
		else
			m_motor_on = false;
	}

	// 8 evenly spaced holes in the rotation disc for the sensor to 'see' through.
	// The first hole is much bigger, enabling the game to determine the position.
	if ((m_motor_pos & 0xf) < 4 || m_motor_pos < 0xc)
		m_sensor_blind = false;
	else
		m_sensor_blind = true;

	// on change, output info
	if (m_motor_pos != m_motor_pos_prev)
		output().set_value("motor_pos", 100 * (m_motor_pos / (float)0x80));

	/* 3 display cards per hole, like this:

	    (0)                <---- display increments this way <----                   (7)

	    CURSED   VICTORY    WIZARD         DRAGON    GOLD KEY     SCOUT    WARRIOR   (void)
	    LOST     WARRIORS   BAZAAR CLOSED  SWORD     SILVER KEY   HEALER   FOOD      (void)
	    PLAGUE   BRIGANDS   KEY MISSING    PEGASUS   BRASS KEY    GOLD     BEAST     (void)
	*/
	int card_pos = m_motor_pos >> 4 & 7;
	if (card_pos != (m_motor_pos_prev >> 4 & 7))
		output().set_value("card_pos", card_pos);

	m_motor_pos_prev = m_motor_pos;
}


void mbdtower_state::prepare_display()
{
	// declare display matrix size and the 2 7segs
	set_display_size(7, 3);
	set_display_segmask(6, 0x7f);

	// update current state
	if (~m_r & 0x10)
	{
		u8 o = bitswap<8>(m_o,7,0,4,3,2,1,6,5) & 0x7f;
		m_display_state[2] = (m_o & 0x80) ? o : 0;
		m_display_state[1] = (m_o & 0x80) ? 0 : o;
		m_display_state[0] = (m_r >> 8 & 1) | (m_r >> 4 & 0xe);

		display_update();
	}
	else
	{
		// display items turned off
		display_matrix(7, 3, 0, 0);
	}
}

WRITE16_MEMBER(mbdtower_state::write_r)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R9: motor on
	if ((m_r ^ data) & 0x200)
		output().set_value("motor_on", data >> 9 & 1);
	if (data & 0x200)
		m_motor_on = true;

	// R3: N/C
	// R4: 75494 /EN (speaker, lamps, digit select go through that IC)
	// R5-R7: tower lamps
	// R8: rotation sensor led
	m_r = data;
	prepare_display();

	// R10: speaker out
	m_speaker->level_w(~data >> 4 & data >> 10 & 1);
}

WRITE16_MEMBER(mbdtower_state::write_o)
{
	// O0-O6: led segments A-G
	// O7: digit select
	m_o = data;
	prepare_display();
}

READ8_MEMBER(mbdtower_state::read_k)
{
	// K: multiplexed inputs
	// K8: rotation sensor
	return read_inputs(3) | ((!m_sensor_blind && sensor_led_on()) ? 8 : 0);
}

// config

/* physical button layout and labels is like this:

    (green)     (l.blue)    (red)
    [YES/       [REPEAT]    [NO/
     BUY]                    END]

    (yellow)    (blue)      (white)
    [HAGGLE]    [BAZAAR]    [CLEAR]

    (blue)      (blue)      (blue)
    [TOMB/      [MOVE]      [SANCTUARY/
     RUIN]                   CITADEL]

    (orange)    (blue)      (d.yellow)
    [DARK       [FRONTIER]  [INVENTORY]
     TOWER]
*/

static INPUT_PORTS_START( mbdtower )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Inventory")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("No/End")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Clear")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Sanctuary/Citadel")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Frontier")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Repeat")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Bazaar")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Move")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Dark Tower")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Yes/Buy")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Haggle")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Tomb/Ruin")
INPUT_PORTS_END

void mbdtower_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill
	m_motor_pos = 0;
	m_motor_pos_prev = -1;
	m_motor_decay = 0;
	m_motor_on = false;
	m_sensor_blind = false;

	// register for savestates
	save_item(NAME(m_motor_pos));
	/* save_item(NAME(m_motor_pos_prev)); */ // don't save!
	save_item(NAME(m_motor_decay));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_sensor_blind));
}

void mbdtower_state::mbdtower(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 425000); // approximation - RC osc. R=43K, C=56pF
	m_maincpu->k().set(FUNC(mbdtower_state::read_k));
	m_maincpu->r().set(FUNC(mbdtower_state::write_r));
	m_maincpu->o().set(FUNC(mbdtower_state::write_o));

	TIMER(config, "tower_motor").configure_periodic(FUNC(mbdtower_state::motor_sim_tick), attotime::from_msec(3500/0x80)); // ~3.5sec for a full rotation
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_mbdtower);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Milton Bradley Electronic Arcade Mania
  * TMS1100 M34078A-N2LL (die label 1100G, M34078A)
  * 9 LEDs, 3-bit sound

  This is a board game. The mini arcade machine is the emulated part here.
  External artwork is needed for the game overlays.

***************************************************************************/

class arcmania_state : public hh_tms1k_state
{
public:
	arcmania_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void arcmania(machine_config &config);
};

// handlers

WRITE16_MEMBER(arcmania_state::write_r)
{
	// R1-R9: leds
	display_matrix(9, 1, data >> 1, 1);
}

WRITE16_MEMBER(arcmania_state::write_o)
{
	// O0-O2(tied together): speaker out
	m_speaker->level_w(data & 7);

	// O3,O4,O6: input mux
	m_inp_mux = (data >> 3 & 3) | (data >> 4 & 4);

	// O5: power off when low (turn back on with button row 1)
	if (~data & 0x20)
		power_off();
}

READ8_MEMBER(arcmania_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

/* physical button layout and labels is like this:

    (orange)    (orange)    (orange)
    [1]         [2]         [3]

    (red)       (yellow)    (blue)
    [RUN]                   [SNEAK
     AMUK]                   ATTACK]

    (green)     (yellow)    (purple)
    [Alien                  [Rattler]
     Raiders]
*/

static INPUT_PORTS_START( arcmania )
	PORT_START("IN.0") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Alien Raiders")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Yellow Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Rattler")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Run Amuk")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Yellow Button 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Sneak Attack")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // O6 (also O5 to power)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Orange Button 1") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Orange Button 2") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Orange Button 3") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const s16 arcmania_speaker_levels[8] = { 0, 0x7fff/3, 0x7fff/3, 0x7fff/3*2, 0x7fff/3, 0x7fff/3*2, 0x7fff/3*2, 0x7fff };

void arcmania_state::arcmania(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 250000); // approximation - RC osc. R=56K, C=100pF
	m_maincpu->k().set(FUNC(arcmania_state::read_k));
	m_maincpu->r().set(FUNC(arcmania_state::write_r));
	m_maincpu->o().set(FUNC(arcmania_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_arcmania);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(8, arcmania_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Code Name: Sector, by Bob Doyle
  * TMS0970 MCU, MP0905BNL ZA0379 (die label 0970F-05B)
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
	void cnsector(machine_config &config);
};

// handlers

WRITE16_MEMBER(cnsector_state::write_r)
{
	// R0-R5: select digit
	// R6-R9: direction leds
	set_display_segmask(0x3f, 0xff);
	display_matrix(8, 10, m_o, data);
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Aim")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Recall")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Evasive Sub") // expert button
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Fire")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Slower")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Sub Finder") // expert button

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Move Ship")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Faster")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Teach Mode")
INPUT_PORTS_END

void cnsector_state::cnsector(machine_config &config)
{
	/* basic machine hardware */
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(cnsector_state::read_k));
	m_maincpu->r().set(FUNC(cnsector_state::write_r));
	m_maincpu->o().set(FUNC(cnsector_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_cnsector);

	/* no sound! */
}





/***************************************************************************

  Parker Brothers Merlin handheld game, by Bob Doyle
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
	void merlin(machine_config &config);
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Button 0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Button 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Button 2")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Button 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Button 5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Button 7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Button 6")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Button 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Button 9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Same Game")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Button 10")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Comp Turn")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Hit Me")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("New Game")
INPUT_PORTS_END

static const s16 merlin_speaker_levels[8] = { 0, 0x7fff/3, 0x7fff/3, 0x7fff/3*2, 0x7fff/3, 0x7fff/3*2, 0x7fff/3*2, 0x7fff };

void merlin_state::merlin(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(merlin_state::read_k));
	m_maincpu->r().set(FUNC(merlin_state::write_r));
	m_maincpu->o().set(FUNC(merlin_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_merlin);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(8, merlin_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Master Merlin
  * TMS1400 MP7351-N2LL (die label 1400CR MP7351)
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

	void mmerlin(machine_config &config);
};

// handlers: uses the ones in merlin_state

// config

static INPUT_PORTS_START( mmerlin )
	PORT_INCLUDE( merlin )

	PORT_MODIFY("IN.3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Score") // instead of Hit Me
INPUT_PORTS_END

void mmerlin_state::mmerlin(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 425000); // approximation - RC osc. R=30K, C=100pF
	m_maincpu->k().set(FUNC(mmerlin_state::read_k));
	m_maincpu->r().set(FUNC(mmerlin_state::write_r));
	m_maincpu->o().set(FUNC(mmerlin_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_mmerlin);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(8, merlin_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Stop Thief, by Bob Doyle
  * TMS0980NLL MP6101B (die label 0980B-01A)
  * 3-digit 7seg LED display, 6-level sound

  Stop Thief is actually a board game, the electronic device emulated here
  (called Electronic Crime Scanner) is an accessory. To start a game, press
  the ON button. Otherwise, it is in test-mode where you can hear all sounds.
  For the patent romset, it needs to be turned off and on to exit test mode.

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
	void stopthief(machine_config &config);
};

// handlers

WRITE16_MEMBER(stopthief_state::write_r)
{
	// R0-R2: select digit
	set_display_segmask(7, 0x7f);
	display_matrix(7, 3, bitswap<8>(m_o,3,5,2,1,4,0,6,7) & 0x7f, data & 7);

	// R3-R8(tied together): speaker out
	int level = 0;
	for (int i = 0; m_o & 8 && i < 6; i++)
		level += (data >> (i+3) & 1);
	m_speaker->level_w(level);
}

WRITE16_MEMBER(stopthief_state::write_o)
{
	// O0,O6: input mux
	m_inp_mux = (data & 1) | (data >> 5 & 2);

	// O3: speaker on
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Tip")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Arrest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Clue")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
INPUT_PORTS_END

static const s16 stopthief_speaker_levels[7] = { 0, 0x7fff/6, 0x7fff/5, 0x7fff/4, 0x7fff/3, 0x7fff/2, 0x7fff };

void stopthief_state::stopthief(machine_config &config)
{
	/* basic machine hardware */
	TMS0980(config, m_maincpu, 425000); // approximation
	m_maincpu->k().set(FUNC(stopthief_state::read_k));
	m_maincpu->r().set(FUNC(stopthief_state::write_r));
	m_maincpu->o().set(FUNC(stopthief_state::write_o));
	m_maincpu->power_off().set(FUNC(hh_tms1k_state::auto_power_off));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_stopthief);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(7, stopthief_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Bank Shot (known as Cue Ball in the UK), by Garry Kitchen
  * TMS1400NLL MP7313-N2 (die label MP7313)
  * LED grid display, 1-bit sound

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

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void bankshot(machine_config &config);
};

// handlers

void bankshot_state::prepare_display()
{
	display_matrix(8, 11, m_o, m_r & ~3);
}

WRITE16_MEMBER(bankshot_state::write_r)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R2,R3: input mux
	m_inp_mux = data >> 2 & 3;

	// R2-R10: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(bankshot_state::write_o)
{
	// O0-O7: led state
	m_o = data;
	prepare_display();
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

void bankshot_state::bankshot(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 475000); // approximation - RC osc. R=24K, C=100pF
	m_maincpu->k().set(FUNC(bankshot_state::read_k));
	m_maincpu->r().set(FUNC(bankshot_state::write_r));
	m_maincpu->o().set(FUNC(bankshot_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_bankshot);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Split Second
  * TMS1400NLL MP7314-N2 (die label MP7314)
  * LED grid display(default round LEDs, and rectangular shape ones), 1-bit sound

  This is an electronic handheld reflex gaming device, it's straightforward
  to use. The included mini-games are:
  1, 2, 3: Mad Maze*
  4, 5: Space Attack*
  6: Auto Cross
  7: Stomp
  8: Speedball

  *: higher number indicates higher difficulty

  display layout, where number yx is lamp R(y).O(x)

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

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void splitsec(machine_config &config);
};

// handlers

void splitsec_state::prepare_display()
{
	display_matrix(7, 8, m_o, m_r);
}

WRITE16_MEMBER(splitsec_state::write_r)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0-R7: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(splitsec_state::write_o)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
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

void splitsec_state::splitsec(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 475000); // approximation - RC osc. R=24K, C=100pF
	m_maincpu->k().set(FUNC(splitsec_state::read_k));
	m_maincpu->r().set(FUNC(splitsec_state::write_r));
	m_maincpu->o().set(FUNC(splitsec_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_splitsec);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Parker Brothers Lost Treasure - The Electronic Deep-Sea Diving Game,
  Featuring The Electronic Dive-Control Center
  * TMS1100 M34038-NLL (die label 1100E, M34038)
  * 11 LEDs, 4-bit sound

  This is a board game. The electronic accessory is the emulated part here.

***************************************************************************/

class lostreas_state : public hh_tms1k_state
{
public:
	lostreas_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void lostreas(machine_config &config);
};

// handlers

WRITE16_MEMBER(lostreas_state::write_r)
{
	// R0-R10: leds
	display_matrix(11, 1, data, 1);
}

WRITE16_MEMBER(lostreas_state::write_o)
{
	// O0-O3: input mux
	m_inp_mux = data & 0xf;

	// O4: 330 ohm resistor - speaker out
	// O5: 220 ohm resistor - speaker out
	// O6: 180 ohm resistor - speaker out
	// O7:  68 ohm resistor - speaker out
	m_speaker->level_w(data >> 4 & 0xf);
}

READ8_MEMBER(lostreas_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

/* physical button layout and labels is like this:
  (note: Canadian version differs slightly to accomodoate dual-language)

    [N-S(gold)]    [1] [2] [3]    [AIR]
    [E-W(gold)]    [4] [5] [6]    [UP]
    [N-S(silv)]    [7] [8] [9]    [$ VALUE]
    [E-W(silv)]                   [CLEAR]
*/

static INPUT_PORTS_START( lostreas )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // air/up
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("$ Value")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("E-W (silver)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("N-S (silver)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("E-W (gold)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("N-S (gold)")

	PORT_START("FAKE") // Air/Up buttons share the same position on the matrix
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Air")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Up")
INPUT_PORTS_END

static const s16 lostreas_speaker_levels[16] =
{
	0, 0x7fff/15, 0x7fff/14, 0x7fff/13, 0x7fff/12, 0x7fff/11, 0x7fff/10, 0x7fff/9,
	0x7fff/8, 0x7fff/7, 0x7fff/6, 0x7fff/5, 0x7fff/4, 0x7fff/3, 0x7fff/2, 0x7fff/1
};

void lostreas_state::lostreas(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 425000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(lostreas_state::read_k));
	m_maincpu->r().set(FUNC(lostreas_state::write_r));
	m_maincpu->o().set(FUNC(lostreas_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_lostreas);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(16, lostreas_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Playskool Alphie
  * TMS1000 (label not known yet)
  * 5 LEDs, 1-bit sound

  This is an educational toy robot for young kids. It has 2 sliding arms:
  the left arm(Alphie's right arm) is for questions, the other for answers.
  Cardboard inlays are used for arm position labels.

  There are 4 play modes:
  - S symbol: Alphie answers questions. The answers are always the same,
    no matter the inlay: Q1=A3, Q2=A5, Q3=A4, Q4=A1, Q5=A2.
  - * symbol: used with Lunar Landing board game
  - X symbol: used with Robot Land board game
  - music note: play a selection of 5 tunes

***************************************************************************/

class alphie_state : public hh_tms1k_state
{
public:
	alphie_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	TIMER_DEVICE_CALLBACK_MEMBER(show_arm_position);

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void alphie(machine_config &config);
};

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(alphie_state::show_arm_position)
{
	// arm position 1(up) to 5(down)
	output().set_value("q_pos", 32 - count_leading_zeros(m_inp_matrix[1]->read()));
	output().set_value("a_pos", 32 - count_leading_zeros(m_inp_matrix[2]->read()));
}

WRITE16_MEMBER(alphie_state::write_r)
{
	// R1-R5, input mux (using d5 for Vss)
	m_inp_mux = (data >> 1 & 0x1f) | 0x20;

	// R6-R10: leds
	display_matrix(5, 1, data >> 6, 1);

	// R0: power off on falling edge (turn back on with button)
	if (~data & m_r & 1)
		power_off();

	m_r = data;
}

WRITE16_MEMBER(alphie_state::write_o)
{
	// O?: speaker out
	m_speaker->level_w(data & 1);
}

READ8_MEMBER(alphie_state::read_k)
{
	// K: multiplexed inputs, rotated matrix
	return read_rotated_inputs(6);
}

// config

static const ioport_value alphie_armpos_table[5] = { 0x01, 0x02, 0x04, 0x08, 0x10 };

static INPUT_PORTS_START( alphie )
	PORT_START("IN.0") // K1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)

	PORT_START("IN.1") // K2
	PORT_BIT( 0x1f, 0x00, IPT_POSITIONAL_V ) PORT_PLAYER(2) PORT_POSITIONS(5) PORT_REMAP_TABLE(alphie_armpos_table) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_NAME("Question Arm")

	PORT_START("IN.2") // K4
	PORT_BIT( 0x1f, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(5) PORT_REMAP_TABLE(alphie_armpos_table) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_NAME("Answer Arm")

	PORT_START("IN.3") // K8
	PORT_CONFNAME( 0x0f, 0x01, "Activity" )
	PORT_CONFSETTING(    0x01, "Questions" )
	PORT_CONFSETTING(    0x02, "Lunar Landing" )
	PORT_CONFSETTING(    0x04, "Robot Land" )
	PORT_CONFSETTING(    0x08, "Tunes" )
INPUT_PORTS_END

// output PLA is guessed
static const u16 alphie_output_pla[0x20] =
{
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};

void alphie_state::alphie(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 350000); // approximation
	m_maincpu->set_output_pla(alphie_output_pla);
	m_maincpu->k().set(FUNC(alphie_state::read_k));
	m_maincpu->r().set(FUNC(alphie_state::write_r));
	m_maincpu->o().set(FUNC(alphie_state::write_o));

	TIMER(config, "arm_position").configure_periodic(FUNC(alphie_state::show_arm_position), attotime::from_msec(50));
	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_alphie);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tandy Radio Shack Championship Football (model 60-2150)
  * PCB label CYG-316
  * TMS1100NLL MP1193 (die label 1100B, MP1193)
  * 7-digit 7seg LED display + LED grid, 1-bit sound

  Another clone of Mattel Football II. The original manufacturer is unknown, but
  suspected to be Conic/E.R.S.(Electronic Readout Systems).

***************************************************************************/

class tcfball_state : public hh_tms1k_state
{
public:
	tcfball_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);
	void tcfball(machine_config &config);
};

// handlers

void tcfball_state::prepare_display()
{
	// R8 enables leds, R9 enables digits
	u16 mask = ((m_r >> 9 & 1) * 0x7f) | ((m_r >> 8 & 1) * 0x780);
	u16 sel = ((m_r & 0x7f) | (m_r << 7 & 0x780)) & mask;

	set_display_segmask(0x77, 0x7f);
	set_display_segmask(0x08, 0xff); // R3 has DP
	display_matrix(8, 11, m_o, sel);
}

WRITE16_MEMBER(tcfball_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R7: input mux
	m_inp_mux = data >> 5 & 7;

	// R8+R0-R3: select led
	// R9+R0-R6: select digit
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(tcfball_state::write_o)
{
	// O0-O7: digit segments/led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(tcfball_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( tcfball )
	PORT_START("IN.0") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Status")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Kick")

	PORT_START("IN.2") // R7
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // college
	PORT_CONFSETTING(    0x08, "2" ) // professional
INPUT_PORTS_END

void tcfball_state::tcfball(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=56K, C=24pF
	m_maincpu->k().set(FUNC(tcfball_state::read_k));
	m_maincpu->r().set(FUNC(tcfball_state::write_r));
	m_maincpu->o().set(FUNC(tcfball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_tcfball);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tandy Radio Shack Championship Football (model 60-2151)
  * TMS1100NLL MP1183 (no decap)
  * 7-digit 7seg LED display + LED grid, 1-bit sound

  The hardware is almost the same as the MP1193 one, they added an extra row of leds.

  known releases:
  - World(1): Superbowl XV Football, published by E.R.S.(Electronic Readout Systems)
  - World(2): Super-Pro Football, no brand
  - USA: Championship Football (model 60-2151), published by Tandy

***************************************************************************/

class tcfballa_state : public tcfball_state
{
public:
	tcfballa_state(const machine_config &mconfig, device_type type, const char *tag)
		: tcfball_state(mconfig, type, tag)
	{ }

	void tcfballa(machine_config &config);
};

// handlers: uses the ones in tcfball_state

// config

static INPUT_PORTS_START( tcfballa )
	PORT_INCLUDE( tcfball )

	PORT_MODIFY("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Display")
INPUT_PORTS_END

// output PLA is not decapped, dumped electronically
static const u16 tcfballa_output_pla[0x20] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x46, 0x70, 0x00, 0x00, 0x00,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void tcfballa_state::tcfballa(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=50pF
	m_maincpu->set_output_pla(tcfballa_output_pla);
	m_maincpu->k().set(FUNC(tcfballa_state::read_k));
	m_maincpu->r().set(FUNC(tcfballa_state::write_r));
	m_maincpu->o().set(FUNC(tcfballa_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_tcfballa);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tandy Radio Shack Computerized Arcade (1981, 1982, 1995)
  * TMS1100 MCU, label CD7282SL
  * 12 lamps behind buttons, 1-bit sound

  known releases:
  - World: Tandy-12: Computerized Arcade
  - Mexico: Fabuloso Fred, published by Ensueño Toys (also released as
    9-button version, a clone of Mego Fabulous Fred)

  This handheld contains 12 minigames. It looks and plays like Game Robot 9 by
  Takatoku Toys (aka Mego's Fabulous Fred) from 1980, which in turn is a mix of
  Merlin and Simon. Unlike Merlin and Simon, spin-offs were not as successful in
  the USA. There were releases with and without the prefix "Tandy-12", I don't
  know which name was more common. Also not worth noting is that it needed five
  batteries; four C-cells and a 9-volt.

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
	void tandy12(machine_config &config);
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

// output PLA is not decapped
static const u16 tandy12_output_pla[0x20] =
{
	// these are certain
	0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
	0x80, 0x00, 0x00, 0x00, 0x00,

	// rest is unused?
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void tandy12_state::tandy12(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->set_output_pla(tandy12_output_pla);
	m_maincpu->k().set(FUNC(tandy12_state::read_k));
	m_maincpu->r().set(FUNC(tandy12_state::write_r));
	m_maincpu->o().set(FUNC(tandy12_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_tandy12);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  (Tandy) Radio Shack Monkey See (1982 version)
  * TMS1000 MP0271 (die label 1000E, MP0271), only half of ROM space used
  * 2 LEDs(one red, one green), 1-bit sound

  This is the TMS1000 version, the one from 1977 has a MM5780.
  To play, enter an equation followed by the ?-key, and the calculator will
  tell you if it was right(green) or wrong(red). For example 1+2=3?

  known releases:
  - USA(1): Monkey See
  - USA(2): Heathcliff, published by McNaught Syndicate in 1983

***************************************************************************/

class monkeysee_state : public hh_tms1k_state
{
public:
	monkeysee_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void monkeysee(machine_config &config);
};

// handlers

WRITE16_MEMBER(monkeysee_state::write_r)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R5: speaker out
	m_speaker->level_w(data >> 5 & 1);
}

WRITE16_MEMBER(monkeysee_state::write_o)
{
	// O6,O7: leds
	// other: N/C
	display_matrix(2, 1, data >> 6, 1);
}

READ8_MEMBER(monkeysee_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( monkeysee )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
INPUT_PORTS_END

void monkeysee_state::monkeysee(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 250000); // approximation - RC osc. R=68K, C=47pF
	m_maincpu->k().set(FUNC(monkeysee_state::read_k));
	m_maincpu->r().set(FUNC(monkeysee_state::write_r));
	m_maincpu->o().set(FUNC(monkeysee_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_monkeysee);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Telesensory Systems, Inc.(TSI) Speech+
  * TMS1000 MCU, label TMS1007NL (die label 1000B, 1007A)
  * TSI S14001A speech chip, GI S14007-A 2KB maskrom for samples
  * 9-digit 7seg LED display

  This is a speaking calculator for the blind, the instructions that came
  with it were on audio cassette. It was also released in 1978 by APH
  (American Printing House for the Blind).

***************************************************************************/

class speechp_state : public hh_tms1k_state
{
public:
	speechp_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_speech(*this, "speech")
	{ }

	required_device<s14001a_device> m_speech;

	void prepare_display();
	virtual DECLARE_WRITE16_MEMBER(write_r);
	virtual DECLARE_WRITE16_MEMBER(write_o);
	virtual DECLARE_READ8_MEMBER(read_k);
	void speechp(machine_config &config);
};

// handlers

void speechp_state::prepare_display()
{
	set_display_segmask(0x1ff, 0xff);
	display_matrix(8, 9, m_o, m_r);
}

WRITE16_MEMBER(speechp_state::write_r)
{
	// R5-R9: TSI C0-C5
	m_speech->data_w(space, 0, data >> 5 & 0x3f);

	// R10: TSI START line
	m_speech->start_w(data >> 10 & 1);

	// R0-R9: input mux
	m_inp_mux = data & 0x3ff;

	// R0-R8: select digit
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(speechp_state::write_o)
{
	// O0-O7: digit segments
	m_o = data;
	prepare_display();
}

READ8_MEMBER(speechp_state::read_k)
{
	// K: multiplexed inputs
	return m_inp_matrix[10]->read() | (read_inputs(10) & 7);
}

// config

static INPUT_PORTS_START( speechp )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("S") // Swap
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C") // Clear
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speak")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("M") // Memory
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.9") // R9
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, "Verbose" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.10") // K8
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x08, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
INPUT_PORTS_END

void speechp_state::speechp(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(speechp_state::read_k));
	m_maincpu->r().set(FUNC(speechp_state::write_r));
	m_maincpu->o().set(FUNC(speechp_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_speechp);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	S14001A(config, m_speech, 25000); // approximation
	m_speech->add_route(ALL_OUTPUTS, "mono", 0.75);
}





/***************************************************************************

  Texas Instruments maze game (unreleased, from patent GB2040172A)
  * TMS1000 (development version)
  * 1 7seg LED digit, no sound

  The title of this game is unknown, the patent describes it simply as a maze game.
  Several electronic maze game concepts are listed in the patent. The PCB schematic
  and program source code is included for one of them: A predefined 12*8 maze,
  walls close to the player are displayed on a 7seg digit.

  In the end Texas Instruments didn't release any electronic maze game. This version
  is too simple and obviously unfinished, start and goal positions are always the same
  and there is a lot of ROM space left for more levels.

***************************************************************************/

class timaze_state : public hh_tms1k_state
{
public:
	timaze_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void timaze(machine_config &config);
};

// handlers

WRITE16_MEMBER(timaze_state::write_r)
{
	// R0: input mux
	m_inp_mux = data & 1;
}

WRITE16_MEMBER(timaze_state::write_o)
{
	// O3210: 7seg EGCD?
	set_display_segmask(1, 0x5c);
	display_matrix(8, 1, bitswap<8>(data, 7,1,6,0,3,2,5,4), 1);
}

READ8_MEMBER(timaze_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(1);
}

// config

static INPUT_PORTS_START( timaze )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
INPUT_PORTS_END

void timaze_state::timaze(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=80K, C=27pF
	m_maincpu->k().set(FUNC(timaze_state::read_k));
	m_maincpu->r().set(FUNC(timaze_state::write_r));
	m_maincpu->o().set(FUNC(timaze_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_timaze);

	/* no sound! */
}





/***************************************************************************

  Tiger Electronics Copy Cat (model 7-520)
  * PCB label CC REV B
  * TMS1000 MCU, label 69-11513 MP0919 (die label MP0919)
  * 4 LEDs, 1-bit sound

  known releases:
  - World: Copy Cat
  - USA(1): Follow Me, published by Sears
  - USA(2): Electronic Repeat, published by Tandy

***************************************************************************/

class copycat_state : public hh_tms1k_state
{
public:
	copycat_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void copycat(machine_config &config);
};

// handlers

WRITE16_MEMBER(copycat_state::write_r)
{
	// R0-R3: leds
	display_matrix(4, 1, data & 0xf, 1);

	// R4-R7: input mux
	// R8-R10: N/C
	m_inp_mux = data >> 4 & 0xf;
}

WRITE16_MEMBER(copycat_state::write_o)
{
	// O0,O1: speaker out
	// O2,O7: N/C, O3-O6: tied together but unused
	m_speaker->level_w(data & 3);
}

READ8_MEMBER(copycat_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( copycat )
	PORT_START("IN.0") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Green Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Orange Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Yellow Button")

	PORT_START("IN.1") // R5
	PORT_CONFNAME( 0x0f, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x08, "4" )

	PORT_START("IN.2") // R6
	PORT_CONFNAME( 0x07, 0x01, "Game Select")
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Best Play")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Play")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Replay")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const s16 copycat_speaker_levels[4] = { 0, 0x7fff, -0x8000, 0 };

void copycat_state::copycat(machine_config &config)
{
	/* basic machine hardware */
	TMS1000(config, m_maincpu, 320000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(copycat_state::read_k));
	m_maincpu->r().set(FUNC(copycat_state::write_r));
	m_maincpu->o().set(FUNC(copycat_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_copycat);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, copycat_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tiger Electronics Copy Cat (model 7-522)
  * PCB label WS 8107-1
  * TMS1730 MCU, label MP3005N (die label 1700 MP3005)
  * 4 LEDs, 1-bit sound

  This is a simplified rerelease of Copy Cat, 10(!) years later. The gameplay
  is identical to Ditto.

  3 variations exist, each with a different colored case. Let's assume that
  they're on the same hardware.
  - white case, yellow orange green red buttons and leds (same as model 7-520)
  - yellow case, purple orange blue pink buttons, leds are same as older version
  - transparent case, transparent purple orange blue red buttons, leds same as before

***************************************************************************/

class copycatm2_state : public hh_tms1k_state
{
public:
	copycatm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	void copycatm2(machine_config &config);
};

// handlers

WRITE16_MEMBER(copycatm2_state::write_r)
{
	// R1-R4: leds
	display_matrix(4, 1, data >> 1 & 0xf, 1);
}

WRITE16_MEMBER(copycatm2_state::write_o)
{
	// O0,O6: speaker out
	m_speaker->level_w((data & 1) | (data >> 5 & 2));
}

// config

static INPUT_PORTS_START( copycatm2 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Orange Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Green Button")
INPUT_PORTS_END

void copycatm2_state::copycatm2(machine_config &config)
{
	/* basic machine hardware */
	TMS1730(config, m_maincpu, 275000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(copycatm2_state::write_r));
	m_maincpu->o().set(FUNC(copycatm2_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_copycatm2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, copycat_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tiger Ditto (model 7-530)
  * TMS1700 MCU, label MP1801-N2LL (die label 1700 MP1801)
  * 4 LEDs, 1-bit sound

  known releases:
  - World: Ditto
  - USA: Electronic Pocket Repeat (model 60-2152/60-2468A), published by Tandy
    note: 1996 model 60-2482 MCU is a Z8, and is assumed to be a clone of Tiger Copycat Jr.

***************************************************************************/

class ditto_state : public hh_tms1k_state
{
public:
	ditto_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	void ditto(machine_config &config);
};

// handlers

WRITE16_MEMBER(ditto_state::write_r)
{
	// R0-R3: leds
	display_matrix(4, 1, data & 0xf, 1);
}

WRITE16_MEMBER(ditto_state::write_o)
{
	// O5,O6: speaker out
	m_speaker->level_w(data >> 5 & 3);
}

// config

static INPUT_PORTS_START( ditto )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Blue Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Orange Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Red Button")
INPUT_PORTS_END

void ditto_state::ditto(machine_config &config)
{
	/* basic machine hardware */
	TMS1730(config, m_maincpu, 275000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(ditto_state::write_r));
	m_maincpu->o().set(FUNC(ditto_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ditto);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, copycat_speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tiger 7 in 1 Sports Stadium
  * TMS1400 MP7304 (die label TMS1400 MP7304A)
  * 2x2-digit 7seg LED display + 39 other LEDs, 1-bit sound

  This handheld includes 7 games: 1: Basketball, 2: Hockey, 3: Soccer,
  4: Maze, 5: Baseball, 6: Football, 7: Raquetball.
  MAME external artwork is needed for the switchable overlays.

  known releases:
  - World: 7 in 1 Sports Stadium
  - USA: 7 in 1 Sports, published by Sears

***************************************************************************/

class ss7in1_state : public hh_tms1k_state
{
public:
	ss7in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void ss7in1(machine_config &config);
};

// handlers

void ss7in1_state::prepare_display()
{
	// R0-R3 are 7segs
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 9, m_o, m_r);
}

WRITE16_MEMBER(ss7in1_state::write_r)
{
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0-R2,R10: input mux
	m_inp_mux = (data & 7) | (data >> 7 & 8);

	// R0-R9: digit/led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ss7in1_state::write_o)
{
	// O0-O7: led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(ss7in1_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( ss7in1 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // R10
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void ss7in1_state::ss7in1(machine_config &config)
{
	/* basic machine hardware */
	TMS1400(config, m_maincpu, 450000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(ss7in1_state::read_k));
	m_maincpu->r().set(FUNC(ss7in1_state::write_r));
	m_maincpu->o().set(FUNC(ss7in1_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_7in1ss);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tomy(tronics) Break Up (manufactured in Japan)
  * PCB label TOMY B.O.
  * TMS1040 MP2726 TOMY WIPE (die label MP2726A)
  * TMS1025N2LL I/O expander
  * 2-digit 7seg display, 46 other leds, 1-bit sound

  known releases:
  - USA: Break Up
  - Japan: Block Attack
  - UK: Break-In

  led translation table: led zz from game PCB = MAME y.x:

    00 = -     10 = 5.0   20 = 4.2
    01 = 7.0   11 = 5.1   21 = 3.3
    02 = 7.1   12 = 5.2   22 = 2.2
    03 = 7.2   13 = 5.3
    04 = 7.3   14 = 4.3
    05 = 6.0   15 = 3.1
    06 = 6.1   16 = 3.2
    07 = 6.2   17 = 3.0
    08 = 6.3   18 = 4.1
    09 = 4.0   19 = 2.1

  the 7seg panel is 0.* and 1.*(aka digit0/1),
  and the 8(2*4) * 3 rectangular leds panel, where x=0,1,2,3:

    9.*        11.*
    8.*        13.*
    10.*       12.*

***************************************************************************/

class tbreakup_state : public hh_tms1k_state
{
public:
	tbreakup_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag),
		m_expander(*this, "expander")
	{ }

	required_device<tms1025_device> m_expander;
	u8 m_exp_port[7];
	DECLARE_WRITE8_MEMBER(expander_w);

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	void set_clock();
	DECLARE_INPUT_CHANGED_MEMBER(skill_switch);
	void tbreakup(machine_config &config);

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

	// 22 round leds from O2-O7 and expander port 7 (update here)
	for (int y = 2; y < 8; y++)
		m_display_state[y] = (data >> y & 1) ? m_exp_port[6] : 0;

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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, tbreakup_state, skill_switch, nullptr)
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
	m_expander->write_ms(1); // Vss
}

void tbreakup_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill/register for savestates
	memset(m_exp_port, 0, sizeof(m_exp_port));
	save_item(NAME(m_exp_port));
}

void tbreakup_state::tbreakup(machine_config &config)
{
	/* basic machine hardware */
	TMS1040(config, m_maincpu, 325000); // see set_clock
	m_maincpu->k().set(FUNC(tbreakup_state::read_k));
	m_maincpu->r().set(FUNC(tbreakup_state::write_r));
	m_maincpu->o().set(FUNC(tbreakup_state::write_o));

	TMS1025(config, m_expander);
	m_expander->write_port1_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port2_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port3_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port4_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port5_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port6_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port7_callback().set(FUNC(tbreakup_state::expander_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_tbreakup);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Tomy Power House Pinball
  * PCB label TOMY P-B
  * TMS1100 MP1180 TOMY PINB (die label MP1180)
  * 3 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  known releases:
  - USA: Power House Pinball
  - Japan: Pinball
  - Europe: Flipper

  led translation table: led zz from game PCB = MAME y.x:

    0 = -     10 = 5.0   20 = 6.4   A = 3.0
    1 = 3.3   11 = 5.5   21 = 6.5   B = 3.4
    2 = 3.1   12 = 5.1   22 = 7.0   C = 3.5
    3 = 3.2   13 = 5.2   23 = 7.1   D = 8.0
    4 = 4.0   14 = 5.3   24 = 7.2   E = 8.1
    5 = 4.1   15 = 6.0   25 = 7.3   F = 8.2
    6 = 4.2   16 = 5.4   26 = 7.4   G = 8.3
    7 = 4.3   17 = 6.1
    8 = 4.4   18 = 6.2
    9 = 4.5   19 = 6.3

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
	void phpball(machine_config &config);
};

// handlers

void phpball_state::prepare_display()
{
	// rectangular LEDs under LEDs D,F and E,G are directly connected
	// to the left and right flipper buttons - output them to 10.a and 9.a
	u16 in1 = m_inp_matrix[1]->read() << 7 & 0x600;

	set_display_segmask(7, 0x7f);
	display_matrix(7, 11, m_o, (m_r & 0x1ff) | in1);
}

WRITE16_MEMBER(phpball_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R9: input mux
	m_inp_mux = data >> 9 & 1;

	// R0-R2: digit select
	// R0-R8: led select
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(phpball_state::write_o)
{
	// O0-O6: digit segment/led data
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(phpball_state::flipper_button)
{
	prepare_display();
}

void phpball_state::phpball(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(phpball_state::read_k));
	m_maincpu->r().set(FUNC(phpball_state::write_r));
	m_maincpu->o().set(FUNC(phpball_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_hh_tms1k_test);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  U.S. Games Super Sports-4
  * TMS1100 MP1219 (no decap)
  * 4 7seg LEDs, 49 other LEDs, 1-bit sound

  This handheld includes 4 games: Basketball, Football, Soccer, Hockey.
  MAME external artwork is needed for the switchable overlays.

  The later Coleco Total Control 4 is clearly based on this.

***************************************************************************/

class ssports4_state : public hh_tms1k_state
{
public:
	ssports4_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void ssports4(machine_config &config);
};

// handlers

void ssports4_state::prepare_display()
{
	// R0,R1 and R8,R9 are 7segs
	set_display_segmask(0x303, 0x7f);

	// note: R2 is an extra column
	display_matrix(9, 10, m_o | (m_r << 6 & 0x100), m_r);
}

WRITE16_MEMBER(ssports4_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select/data
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(ssports4_state::write_o)
{
	// O0-O7: led data
	m_o = data;
	prepare_display();
}

READ8_MEMBER(ssports4_state::read_k)
{
	// input mux is from R0,1,5,8,9 and O7
	m_inp_mux = (m_r & 3) | (m_r >> 3 & 4) | (m_r >> 5 & 0x18) | (m_o >> 2 & 0x20);
	return read_inputs(6);
}

// config

static INPUT_PORTS_START( ssports4 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // R5
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_16WAY PORT_NAME("P1 Kick") // or diagonal up-left
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_16WAY PORT_NAME("P1 Info") // or diagonal up-right
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_16WAY PORT_NAME("P1 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_16WAY PORT_NAME("P1 O.P.") // offensive player (modifier button)

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Info")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 O.P.")

	PORT_START("IN.5") // O7
	PORT_CONFNAME( 0x03, 0x00, "Game Select" )
	PORT_CONFSETTING(    0x02, "Basketball" )
	PORT_CONFSETTING(    0x00, "Football" )
	PORT_CONFSETTING(    0x01, "Soccer" )
	PORT_CONFSETTING(    0x03, "Hockey" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// output PLA is not decapped, dumped electronically
static const u16 ssports4_output_pla[0x20] =
{
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40
};

void ssports4_state::ssports4(machine_config &config)
{
	/* basic machine hardware */
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->set_output_pla(ssports4_output_pla);
	m_maincpu->k().set(FUNC(ssports4_state::read_k));
	m_maincpu->r().set(FUNC(ssports4_state::write_r));
	m_maincpu->o().set(FUNC(ssports4_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ssports4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Vulcan XL 25
  * TMS1000SLC MP4486A (die label 1000C/, MP4486A)
  * 28 LEDs, 1-bit sound

  This game is the same logic puzzle as Tiger's Lights Out, except that
  all 25 lights need to be turned on instead of off.

***************************************************************************/

class xl25_state : public hh_tms1k_state
{
public:
	xl25_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void update_halt();
	DECLARE_INPUT_CHANGED_MEMBER(k4_button);

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
	void xl25(machine_config &config);

protected:
	virtual void machine_reset() override;
};

// handlers

void xl25_state::update_halt()
{
	// O5+K4 go to HALT pin (used when pressing store/recall button)
	bool halt = !((m_o & 0x20) || (read_k(machine().dummy_space(), 0) & 4));
	m_maincpu->set_input_line(TMS1XXX_INPUT_LINE_HALT, halt ? ASSERT_LINE : CLEAR_LINE);
}

void xl25_state::prepare_display()
{
	display_matrix(3, 10, m_o >> 1, m_r);
}

WRITE16_MEMBER(xl25_state::write_r)
{
	// R0-R9: input mux, led select
	m_inp_mux = data;
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(xl25_state::write_o)
{
	// O1-O3: led data
	m_o = data;
	prepare_display();

	// O6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// O5(+K4): MCU halt
	update_halt();
}

READ8_MEMBER(xl25_state::read_k)
{
	// K: multiplexed inputs
	// K4 also goes to MCU halt
	return read_inputs(10);
}

// config

static INPUT_PORTS_START( xl25 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 11") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 12") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 13") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 14") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 15") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 16")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 21")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Store / Recall") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Cross / Knight / Score") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 18")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 23")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Clear") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 19")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 24")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Random") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 25")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Sound") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(xl25_state::k4_button)
{
	update_halt();
}

void xl25_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	update_halt();
}

void xl25_state::xl25(machine_config &config)
{
	/* basic machine hardware */
	TMS1000C(config, m_maincpu, 300000); // approximation - RC osc. R=5.6K, C=47pF
	m_maincpu->k().set(FUNC(xl25_state::read_k));
	m_maincpu->r().set(FUNC(xl25_state::write_r));
	m_maincpu->o().set(FUNC(xl25_state::write_o));

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_tms1k_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_xl25);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( matchnum )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0163", 0x0000, 0x0400, CRC(37507600) SHA1(b1d4d8ea563e97ef378b42c44cb3ea4eb6abe0d2) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_matchnum_output.pla", 0, 365, CRC(da29670c) SHA1(bcec28bf25dc8c81d08851ad8a3f4e89f413017a) )
ROM_END


ROM_START( arrball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0166", 0x0000, 0x0400, CRC(a78694db) SHA1(362aa6e356288e8df7da610246bd01fe72985d57) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_arrball_output.pla", 0, 365, CRC(ffc206fb) SHA1(339be3f066fb2f075211c554e81260b49cd83d15) )
ROM_END


ROM_START( mathmagi )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1030", 0x0000, 0x0800, CRC(a81d7ccb) SHA1(4756ce42f1ea28ce5fe6498312f8306f10370969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mathmagi_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( bcheetah )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0915", 0x0000, 0x0400, CRC(2968c81e) SHA1(d1e6691952600e88ccf626cb3d683419a1e8468c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bcheetah_output.pla", 0, 365, CRC(cc6d1ecd) SHA1(b0635a841d8850c36c1f414abe0571b81884b972) )
ROM_END


ROM_START( amaztron )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3405", 0x0000, 0x0800, CRC(9cbc0009) SHA1(17772681271b59280687492f37fa0859998f041d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_amaztron_output.pla", 0, 365, CRC(f3875384) SHA1(3c256a3db4f0aa9d93cf78124db39f4cbdc57e4a) )
ROM_END


ROM_START( zodiac )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3435", 0x0000, 0x0800, CRC(ecdc3160) SHA1(a7e82d66314a039fcffeddf99919d9f9ad42d61d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, BAD_DUMP CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_zodiac_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( cqback )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3415.u4", 0x0000, 0x0800, CRC(65ebdabf) SHA1(9b5cf5adaf9132ced87f611ae8c3148b9b62ba89) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cqback_output.pla", 0, 365, CRC(c6dcbfd0) SHA1(593b6b7de981a28d1b4a33336b39df92d02ed4f4) )
ROM_END


ROM_START( h2hfootb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3460.u3", 0x0000, 0x0800, CRC(3a4e53a8) SHA1(5052e706f992c6c4bada1fa7769589eec3df6471) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hfootb_output.pla", 0, 365, CRC(c8d85873) SHA1(16bd6fc8e3cd16d5f8fd32d0c74e67de77f5487e) )
ROM_END


ROM_START( h2hbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3320a", 0x0000, 0x0400, CRC(39a63f43) SHA1(14a765e42a39f8d3a465c990e09dd651e595a1c5) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_h2hbaskb_output.pla", 0, 365, CRC(9d1a91e1) SHA1(96303eb22375129b0dfbfcd823c8ca5b919511bc) )
ROM_END


ROM_START( h2hhockey )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3321a", 0x0000, 0x0400, CRC(e974e604) SHA1(ed740c98ce96ad70ee5237eccae1f54a75ad8100) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_h2hhockey_output.pla", 0, 365, CRC(9d1a91e1) SHA1(96303eb22375129b0dfbfcd823c8ca5b919511bc) )
ROM_END


ROM_START( h2hbaseb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1525", 0x0000, 0x0800, CRC(b5d6bf9b) SHA1(2cc9f35f077c1209c46d16ec853af87e4725c2fd) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hbaseb_output.pla", 0, 365, CRC(cb3d7e38) SHA1(6ab4a7c52e6010b7c7158463cb499973e52ff556) )
ROM_END


ROM_START( h2hboxing )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34018", 0x0000, 0x0800, CRC(e26a11a3) SHA1(aa2735088d709fa8d9188c4fb7982a53e3a8c1bc) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hboxing_output.pla", 0, 365, CRC(ffb0e63d) SHA1(31ee3f779270a23f05f9ad508283d2569ef069f1) )
ROM_END


ROM_START( quizwizc )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "m32001", 0x0000, 0x0400, CRC(053657eb) SHA1(38c84f7416f79aa679f434a3d35df54cd9aa528a) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common3_micro.pla", 0, 867, CRC(80912d0a) SHA1(7ae5293ed4d93f5b7a64d43fe30c3639f39fbe5a) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_quizwizc_output.pla", 0, 365, CRC(475b7053) SHA1(8f61bf736eb41d7029a6b165cc0a184ba0a70a2a) )
ROM_END


ROM_START( tc4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7334", 0x0000, 0x1000, CRC(923f3821) SHA1(a9ae342d7ff8dae1dedcd1e4984bcfae68586581) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_tc4_output.pla", 0, 557, CRC(3b908725) SHA1(f83bf5faa5b3cb51f87adc1639b00d6f9a71ad19) )
ROM_END


ROM_START( cnbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0907", 0x0000, 0x0400, CRC(35f84f0f) SHA1(744ca60bb853a2785184042e747530a9e02488f8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cnbaskb_output.pla", 0, 365, CRC(b4e28956) SHA1(8356112da71b351420a88d7e394e7d03e429368c) )
ROM_END


ROM_START( cmsport )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0168.u1", 0x0000, 0x0400, CRC(0712a268) SHA1(bd4e23e5c17b28c52e7e769e44773cc9c8839bed) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cmsport_output.pla", 0, 365, CRC(7defa140) SHA1(477e3cb55e79938d6acaa911e410f6dcb974c218) )
ROM_END


ROM_START( cnfball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0170", 0x0000, 0x0400, CRC(50e8a44f) SHA1(fea6ae03c4ef329d825f8688e6854df15023d47e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cnfball_output.pla", 0, 365, CRC(0af52f64) SHA1(b4cf450e4d895eddb67448aa69e4f18a5a84e033) )
ROM_END


ROM_START( cnfball2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1181", 0x0000, 0x0800, CRC(4553a840) SHA1(2e1132c9bc51641f77ba7f2430b5a3b2766b3a3d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cnfball2_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( eleciq )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0908", 0x0000, 0x0400, CRC(db59b82c) SHA1(c9a6bcba208969560495ad9f8775f53de16a69c3) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_eleciq_output.pla", 0, 365, CRC(b8e04232) SHA1(22eed6d9b1fb1e5c9974ea3df16cda71a39aad57) )
ROM_END


ROM_START( esoccer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0158.ic1", 0x0000, 0x0400, CRC(ae4581ea) SHA1(5f6881f8247094abf8cffb17f6e6586e94cff38c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_esoccer_output.pla", 0, 365, CRC(c6eeabbd) SHA1(99d07902126b5a1c1abf43340f30d3390da5fa92) )
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


ROM_START( esbattle )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "en-6004_mp0920", 0x0000, 0x0400, CRC(7460c179) SHA1(be855054b4a98b05b34fd931d5c247c5c0f9b036) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_esbattle_output.pla", 0, 365, CRC(861b45a2) SHA1(a5a9dc9bef8adb761845ad548058b55e970517d3) )
ROM_END


ROM_START( einvader )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1211", 0x0000, 0x0800, CRC(b6efbe8e) SHA1(d7d54921dab22bb0c2956c896a5d5b56b6f64969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_einvader_output.pla", 0, 365, CRC(490158e1) SHA1(61cace1eb09244663de98d8fb04d9459b19668fd) )

	ROM_REGION( 44398, "svg", 0)
	ROM_LOAD( "einvader.svg", 0, 44398, CRC(48de88fd) SHA1(56a2b9c997a447277b45902ab542eda54e7d5a2f) )
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


ROM_START( f2pbball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0154", 0x0000, 0x0400, CRC(c5b45ace) SHA1(b2de32e83ab447b22d6828f0081843f364040b01) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_f2pbball_output.pla", 0, 365, CRC(30c2f28f) SHA1(db969b22475f37f083c3594f5e4f5759048377b8) )
ROM_END


ROM_START( f3in1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1185", 0x0000, 0x0800, CRC(53f7b28d) SHA1(2249890e3a259095193b4331ca88c29ccd81eefe) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_f3in1_output.pla", 0, 365, CRC(51d947bc) SHA1(f766397d84f038be96e83d40989195c98ddcb1d9) )
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


ROM_START( ginv1000 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2139", 0x0000, 0x0800, CRC(036eab37) SHA1(0795878ad89296f7a6a0314c6e4db23c1cc3673e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ginv1000_output.pla", 0, 365, CRC(b0a5dc41) SHA1(d94746ec48661998173e7f60ccc7c96e56b3484e) )

	ROM_REGION( 226185, "svg", 0)
	ROM_LOAD( "ginv1000.svg", 0, 226185, CRC(1e1bafd1) SHA1(15868ef0c9dadbf537fed0e2d846451ba99fab7b) )
ROM_END


ROM_START( ginv2000 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1604", 0x0000, 0x0800, CRC(f1646d0b) SHA1(65601931d81e3eef7bf22a08de5a146910ce8137) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ginv2000_output.pla", 0, 365, CRC(520bb003) SHA1(1640ae54f8dcc257e0ad0cbe0281b38fcbd8da35) )

	ROM_REGION( 374443, "svg", 0)
	ROM_LOAD( "ginv2000.svg", 0, 374443, CRC(a4ce1e6d) SHA1(57d9ff05d634a8d495b9d544a2a959790cd10b6b) )
ROM_END


ROM_START( fxmcr165 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1312", 0x0000, 0x0800, CRC(6efc8bcc) SHA1(ced8a02b472a3178073691d3dccc0f19f57428fd) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_fxmcr165_output.pla", 0, 365, CRC(ce656866) SHA1(40e1614f5afcc7572fda596e1be453d54e95af0c) )
ROM_END


ROM_START( elecdet )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6100a", 0x0000, 0x1000, CRC(9522fb2d) SHA1(240bdb44b7d67d3b13ebf75851635ac4b4ca2bfd) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_elecdet_output.pla", 0, 352, CRC(5d12c24a) SHA1(e486802151a704c6273d4a8682c9c374d27d1e6d) )
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


ROM_START( elecbowl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3403.u9", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, BAD_DUMP CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_elecbowl_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( horseran )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3491", 0x0000, 0x0800, CRC(a0081671) SHA1(a5a07b502c69d429e5bcd1d313e86b6ee057cda6) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1100_horseran_output.pla", 0, 365, CRC(0fea09b0) SHA1(27a56fcf2b490e9a7dbbc6ad48cc8aaca4cada94) )
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
	ROM_LOAD( "tms0980_comp4_output.pla", 0, 352, CRC(144ce2d5) SHA1(459b92ad62421932df61b7e3965f1821f9636a2c) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_comp4_segment.pla", 0, 157, CRC(73426b07) SHA1(311be3f95a97936b6d1a4dcfa7746da26318ce54) )
ROM_END


ROM_START( bship )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3201", 0x0000, 0x0400, CRC(bf6104a6) SHA1(8d28b43a2aa39dcbbe71f669cdafc518715812c9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bship_output.pla", 0, 365, CRC(ea0570b0) SHA1(6eb803b40717486d7b24939985f245327ac8a7e9) )
ROM_END

ROM_START( bshipb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3208", 0x0000, 0x0400, CRC(982fa720) SHA1(1c6dbbe7b9e55d62a510225a88cd2de55fe9b181) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bshipb_output.pla", 0, 365, BAD_DUMP CRC(74a9a244) SHA1(479c1f1e37cf8f75352e10226b20322906bee813) ) // part of decap photo was obscured
ROM_END


ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_simon_micro.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1000_simon_output.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END

ROM_START( simonf )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3300", 0x0000, 0x0400, CRC(b9fcf93a) SHA1(45960e4242a08495f2a99fc5d44728eabd93cd9f) )

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
	ROM_LOAD( "tms1000_common3_micro.pla", 0, 867, CRC(80912d0a) SHA1(7ae5293ed4d93f5b7a64d43fe30c3639f39fbe5a) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bigtrak_output.pla", 0, 365, CRC(63be45f6) SHA1(918e38a223152db883c1a6f7acf56e87d7074734) )
ROM_END


ROM_START( mbdtower )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7332", 0x0000, 0x1000, CRC(ebeab91a) SHA1(7edbff437da371390fa8f28b3d183f833eaa9be9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mbdtower_output.pla", 0, 557, CRC(64c84697) SHA1(72ce6d24cedf9c606f1742cd5620f75907246e87) )
ROM_END


ROM_START( arcmania )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34078a", 0x0000, 0x0800, CRC(90ea0087) SHA1(9780c9c1ba89300b1bbe72c47e5fec68d8bb6a77) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_arcmania_output.pla", 0, 365, CRC(a1517b15) SHA1(72eedd7fd41de9c9102219f325fe8668a7c02663) )
ROM_END


ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(201036e9) SHA1(b37fef86bb2bceaf0ac8bb3745b4702d17366914) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common2_instr.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_micro.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_cnsector_output.pla", 0, 352, CRC(c8bfb9d2) SHA1(30c3c73cec194debdcb1dd01b4adfefaeddf9516) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
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


ROM_START( stopthief )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp6101b", 0x0000, 0x1000, CRC(b9c9d64a) SHA1(481f8653064c142fe5d9314b750bcd73797b92b2) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthief_output.pla", 0, 352, CRC(680ca1c1) SHA1(dea6365f2e6b50a52f1a8f1d8417176b905d2bc9) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( stopthiefp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "us4341385", 0x0000, 0x1000, CRC(07aec38a) SHA1(0a3d0956495c0d6d9ea771feae6c14a473a800dc) ) // from patent US4341385, data should be correct (it included checksums)

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_stopthief_output.pla", 0, 352, CRC(680ca1c1) SHA1(dea6365f2e6b50a52f1a8f1d8417176b905d2bc9) )
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


ROM_START( lostreas )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34038", 0x0000, 0x0800, CRC(4c996f63) SHA1(ebbaa8b2f909f4300887aa2dbdb7185eedc75d3f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_lostreas_output.pla", 0, 365, CRC(c62d850f) SHA1(d25974e6901eb10c52cdda12e6d4a13e26745e6f) )
ROM_END


ROM_START( alphie )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "us4280809", 0x0000, 0x0400, CRC(f8f14013) SHA1(bf31b929fcbcb189bbe4623104e1da0a639b5954) ) // from patent US4280809, should be good

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, BAD_DUMP CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) ) // not in patent description
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_alphie_output.pla", 0, 365, NO_DUMP ) // "
ROM_END


ROM_START( tcfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1193", 0x0000, 0x0800, CRC(7d9f446f) SHA1(bb6af47b42d989494f21475a73f072cddf58c99f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tcfball_output.pla", 0, 365, CRC(26b2996e) SHA1(df0e706c552bf74123aa65e71b0c9b4d33cddb2b) )
ROM_END

ROM_START( tcfballa )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1183", 0x0000, 0x0800, CRC(2a4db1d5) SHA1(5df15d1115bb425578ad522d607a582dd478f35c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tcfballa_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( tandy12 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x0800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tandy12_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( monkeysee )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0271", 0x0000, 0x0400, CRC(acab0f05) SHA1(226f7688caf4a94a88241d3b61ddc4254e4a918c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_monkeysee_micro.pla", 0, 867, CRC(368d878f) SHA1(956e700a04f453c1610cfdb974fce898ba4cf01f) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_monkeysee_output.pla", 0, 365, CRC(8a010e89) SHA1(3ffbabc5d6c9b34cc06d290817d15b2be42d8b17) )
ROM_END


ROM_START( speechp )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1007nl", 0x0000, 0x0400, CRC(c2669d5c) SHA1(7943d6f39508a9a82bc21e4fe34a5b9f86e3add2) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_speechp_output.pla", 0, 365, CRC(e1b4197f) SHA1(258f4276a9f15c9bfbfa58df2f7202aed1542fdc) )

	ROM_REGION( 0x0800, "speech", 0 )
	ROM_LOAD("s14007-a", 0x0000, 0x0800, CRC(543b46d4) SHA1(99daf7fe3354c378b4bd883840c9bbd22b22ebe7) )
ROM_END


ROM_START( timaze )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "gb2040172a", 0x0000, 0x0400, CRC(0bab4dc6) SHA1(c9d40649fbb27a8b7cf7460d66c7e217b63376f0) ) // from patent GB2040172A, verified with source code

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, BAD_DUMP CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) ) // not in patent, use default one
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_timaze_output.pla", 0, 365, BAD_DUMP CRC(f0f36970) SHA1(a6ad1f5e804ac98e5e1a1d07466b3db3a8d6c256) ) // described in patent, but unsure about pin order
ROM_END


ROM_START( copycat )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0919", 0x0000, 0x0400, CRC(92a21299) SHA1(16daadb8dbf53aaab8a71833017b4a578d035d6d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_copycat_output.pla", 0, 365, CRC(b1d0c96d) SHA1(ac1a003eab3f69e09e9050cb24ea17211e0523fe) )
ROM_END

ROM_START( copycatm2 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "mp3005n", 0x0000, 0x0200, CRC(a87649cb) SHA1(14ef7967a80578885f0b905772c3bb417b5b3255) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_copycatm2_micro.pla", 0, 867, CRC(2710d8ef) SHA1(cb7a13bfabedad43790de753844707fe829baed0) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_copycatm2_output.pla", 0, 365, CRC(d1999aaf) SHA1(0c27789b349e491d5230f9c75c4741e621f5a14e) )
ROM_END


ROM_START( ditto )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "mp1801", 0x0000, 0x0200, CRC(cee6043b) SHA1(4ec334be6835688413637ff9d9d7a5f0d61eba27) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ditto_micro.pla", 0, 867, CRC(2710d8ef) SHA1(cb7a13bfabedad43790de753844707fe829baed0) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ditto_output.pla", 0, 365, CRC(2b708a27) SHA1(e95415e51ffbe5da3bde1484fcd20467dde9f09a) )
ROM_END


ROM_START( 7in1ss )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7304", 0x0000, 0x1000, CRC(2a1c8390) SHA1(fa10e60686af6828a61f05046abc3854ab49af95) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_7in1ss_output.pla", 0, 557, CRC(6b7660f7) SHA1(bb7d58fa04e7606ccdf5b209e1b089948bdd1e7c) )
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


ROM_START( ssports4 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1219", 0x0000, 0x0800, CRC(865c06d6) SHA1(12a625a13bdb57b82b35c42b175d38756a1e2e04) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ssports4_output.pla", 0, 365, NO_DUMP )
ROM_END


ROM_START( xl25 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp4486a", 0x0000, 0x0400, CRC(bd84b515) SHA1(377fcc68a517260acd51eb9746cd62914a75d739) )

	ROM_REGION( 922, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000c_xl25_micro.pla", 0, 922, CRC(8823e7f2) SHA1(676b0eace9d8730f2caa9087e8c51e540c7fabf8) )
	ROM_REGION( 558, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000c_xl25_output.pla", 0, 558, CRC(06ecc6e0) SHA1(e0fa1b9388948197b4de2edd3cd02fbde1dbabbb) )
ROM_END



//    YEAR  NAME        PARENT    CMP MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, matchnum,   0,         0, matchnum,  matchnum,  matchnum_state,  empty_init, "A-One LSI", "Match Number", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, arrball,    0,         0, arrball,   arrball,   arrball_state,   empty_init, "A-One LSI", "Arrange Ball", MACHINE_SUPPORTS_SAVE )

COMP( 1980, mathmagi,   0,         0, mathmagi,  mathmagi,  mathmagi_state,  empty_init, "APF Electronics Inc.", "Mathemagician", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1979, bcheetah,   0,         0, bcheetah,  bcheetah,  bcheetah_state,  empty_init, "Bandai", "System Control Car: Cheetah", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_MECHANICAL ) // ***

CONS( 1978, amaztron,   0,         0, amaztron,  amaztron,  amaztron_state,  empty_init, "Coleco", "Amaze-A-Tron", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // ***
COMP( 1979, zodiac,     0,         0, zodiac,    zodiac,    zodiac_state,    empty_init, "Coleco", "Zodiac - The Astrology Computer", MACHINE_SUPPORTS_SAVE )
CONS( 1978, cqback,     0,         0, cqback,    cqback,    cqback_state,    empty_init, "Coleco", "Electronic Quarterback", MACHINE_SUPPORTS_SAVE )
CONS( 1979, h2hfootb,   0,         0, h2hfootb,  h2hfootb,  h2hfootb_state,  empty_init, "Coleco", "Head to Head: Electronic Football", MACHINE_SUPPORTS_SAVE )
CONS( 1979, h2hbaskb,   0,         0, h2hbaskb,  h2hbaskb,  h2hbaskb_state,  empty_init, "Coleco", "Head to Head: Electronic Basketball (TMS1000 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, h2hhockey,  0,         0, h2hhockey, h2hhockey, h2hbaskb_state,  empty_init, "Coleco", "Head to Head: Electronic Hockey (TMS1000 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hbaseb,   0,         0, h2hbaseb,  h2hbaseb,  h2hbaseb_state,  empty_init, "Coleco", "Head to Head: Electronic Baseball", MACHINE_SUPPORTS_SAVE )
CONS( 1981, h2hboxing,  0,         0, h2hboxing, h2hboxing, h2hboxing_state, empty_init, "Coleco", "Head to Head: Electronic Boxing", MACHINE_SUPPORTS_SAVE )
CONS( 1981, quizwizc,   0,         0, quizwizc,  quizwizc,  quizwizc_state,  empty_init, "Coleco", "Quiz Wiz Challenger", MACHINE_SUPPORTS_SAVE ) // ***
CONS( 1981, tc4,        0,         0, tc4,       tc4,       tc4_state,       empty_init, "Coleco", "Total Control 4", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, cnbaskb,    0,         0, cnbaskb,   cnbaskb,   cnbaskb_state,   empty_init, "Conic", "Electronic Basketball (Conic)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, cmsport,    0,         0, cmsport,   cmsport,   cmsport_state,   empty_init, "Conic", "Electronic Multisport", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1979, cnfball,    0,         0, cnfball,   cnfball,   cnfball_state,   empty_init, "Conic", "Electronic Football (Conic, TMS1000 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, cnfball2,   0,         0, cnfball2,  cnfball2,  cnfball2_state,  empty_init, "Conic", "Electronic Football II (Conic)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, eleciq,     0,         0, eleciq,    eleciq,    eleciq_state,    empty_init, "Conic", "Electronic I.Q.", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, esoccer,    0,         0, esoccer,   esoccer,   esoccer_state,   empty_init, "Entex", "Electronic Soccer (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ebball,     0,         0, ebball,    ebball,    ebball_state,    empty_init, "Entex", "Electronic Baseball (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ebball2,    0,         0, ebball2,   ebball2,   ebball2_state,   empty_init, "Entex", "Electronic Baseball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebball3,    0,         0, ebball3,   ebball3,   ebball3_state,   empty_init, "Entex", "Electronic Baseball 3 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, esbattle,   0,         0, esbattle,  esbattle,  esbattle_state,  empty_init, "Entex", "Space Battle (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, einvader,   0,         0, einvader,  einvader,  einvader_state,  empty_init, "Entex", "Space Invader (Entex, TMS1100 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, efootb4 ,   0,         0, efootb4,   efootb4,   efootb4_state,   empty_init, "Entex", "Color Football 4 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebaskb2 ,   0,         0, ebaskb2,   ebaskb2,   ebaskb2_state,   empty_init, "Entex", "Electronic Basketball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, raisedvl,   0,         0, raisedvl,  raisedvl,  raisedvl_state,  empty_init, "Entex", "Raise The Devil", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, f2pbball,   0,         0, f2pbball,  f2pbball,  f2pbball_state,  empty_init, "Fonas", "2 Player Baseball (Fonas)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, f3in1,      0,         0, f3in1,     f3in1,     f3in1_state,     empty_init, "Fonas", "3 in 1: Football, Basketball, Soccer", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, gpoker,     0,         0, gpoker,    gpoker,    gpoker_state,    empty_init, "Gakken", "Poker (Gakken, 1979 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gjackpot,   0,         0, gjackpot,  gjackpot,  gjackpot_state,  empty_init, "Gakken", "Jackpot: Gin Rummy & Black Jack", MACHINE_SUPPORTS_SAVE )
CONS( 1981, ginv1000,   0,         0, ginv1000,  ginv1000,  ginv1000_state,  empty_init, "Gakken", "Galaxy Invader 1000", MACHINE_SUPPORTS_SAVE )
CONS( 1982, ginv2000,   0,         0, ginv2000,  ginv2000,  ginv2000_state,  empty_init, "Gakken", "Invader 2000", MACHINE_SUPPORTS_SAVE )
COMP( 1983, fxmcr165,   0,         0, fxmcr165,  fxmcr165,  fxmcr165_state,  empty_init, "Gakken", "FX-Micom R-165", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, elecdet,    0,         0, elecdet,   elecdet,   elecdet_state,   empty_init, "Ideal", "Electronic Detective", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1979, starwbc,    0,         0, starwbc,   starwbc,   starwbc_state,   empty_init, "Kenner", "Star Wars - Electronic Battle Command", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, starwbcp,   starwbc,   0, starwbc,   starwbc,   starwbc_state,   empty_init, "Kenner", "Star Wars - Electronic Battle Command (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

COMP( 1979, astro,      0,         0, astro,     astro,     astro_state,     empty_init, "Kosmos", "Astro", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1978, elecbowl,   0,         0, elecbowl,  elecbowl,  elecbowl_state,  empty_init, "Marx", "Electronic Bowling (Marx)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_CONTROLS | MACHINE_MECHANICAL | MACHINE_NOT_WORKING ) // ***

COMP( 1979, horseran,   0,         0, horseran,  horseran,  horseran_state,  empty_init, "Mattel", "Thoroughbred Horse Race Analyzer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
CONS( 1980, mdndclab,   0,         0, mdndclab,  mdndclab,  mdndclab_state,  empty_init, "Mattel", "Dungeons & Dragons - Computer Labyrinth Game", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // ***

CONS( 1977, comp4,      0,         0, comp4,     comp4,     comp4_state,     empty_init, "Milton Bradley", "Comp IV", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW )
CONS( 1977, bship,      0,         0, bship,     bship,     bship_state,     empty_init, "Milton Bradley", "Electronic Battleship (1977 version, model 4750A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // ***
CONS( 1977, bshipb,     bship,     0, bshipb,    bship,     bshipb_state,    empty_init, "Milton Bradley", "Electronic Battleship (1977 version, model 4750B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // ***
CONS( 1978, simon,      0,         0, simon,     simon,     simon_state,     empty_init, "Milton Bradley", "Simon (Rev A)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, simonf,     simon,     0, simon,     simon,     simon_state,     empty_init, "Milton Bradley", "Simon (Rev F)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, ssimon,     0,         0, ssimon,    ssimon,    ssimon_state,    empty_init, "Milton Bradley", "Super Simon", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, bigtrak,    0,         0, bigtrak,   bigtrak,   bigtrak_state,   empty_init, "Milton Bradley", "Big Trak", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL ) // ***
CONS( 1981, mbdtower,   0,         0, mbdtower,  mbdtower,  mbdtower_state,  empty_init, "Milton Bradley", "Dark Tower (Milton Bradley)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_MECHANICAL ) // ***
CONS( 1983, arcmania,   0,         0, arcmania,  arcmania,  arcmania_state,  empty_init, "Milton Bradley", "Electronic Arcade Mania (Arcade Machine)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_REQUIRES_ARTWORK ) // ***

CONS( 1977, cnsector,   0,         0, cnsector,  cnsector,  cnsector_state,  empty_init, "Parker Brothers", "Code Name: Sector", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW ) // ***
CONS( 1978, merlin,     0,         0, merlin,    merlin,    merlin_state,    empty_init, "Parker Brothers", "Merlin - The Electronic Wizard", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, stopthief,  0,         0, stopthief, stopthief, stopthief_state, empty_init, "Parker Brothers", "Stop Thief (Electronic Crime Scanner)", MACHINE_SUPPORTS_SAVE ) // ***
CONS( 1979, stopthiefp, stopthief, 0, stopthief, stopthief, stopthief_state, empty_init, "Parker Brothers", "Stop Thief (Electronic Crime Scanner) (patent)", MACHINE_SUPPORTS_SAVE ) // ***
CONS( 1980, bankshot,   0,         0, bankshot,  bankshot,  bankshot_state,  empty_init, "Parker Brothers", "Bank Shot - Electronic Pool", MACHINE_SUPPORTS_SAVE )
CONS( 1980, splitsec,   0,         0, splitsec,  splitsec,  splitsec_state,  empty_init, "Parker Brothers", "Split Second", MACHINE_SUPPORTS_SAVE )
CONS( 1982, mmerlin,    0,         0, mmerlin,   mmerlin,   mmerlin_state,   empty_init, "Parker Brothers", "Master Merlin", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, lostreas,   0,         0, lostreas,  lostreas,  lostreas_state,  empty_init, "Parker Brothers", "Lost Treasure - The Electronic Deep-Sea Diving Game (Electronic Dive-Control Center)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***

CONS( 1978, alphie,     0,         0, alphie,    alphie,    alphie_state,    empty_init, "Playskool", "Alphie - The Electronic Robot (patent)", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1980, tcfball,    0,         0, tcfball,   tcfball,   tcfball_state,   empty_init, "Tandy Radio Shack", "Championship Football (model 60-2150)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, tcfballa,   tcfball,   0, tcfballa,  tcfballa,  tcfballa_state,  empty_init, "Tandy Radio Shack", "Championship Football (model 60-2151)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, tandy12,    0,         0, tandy12,   tandy12,   tandy12_state,   empty_init, "Tandy Radio Shack", "Tandy-12: Computerized Arcade", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // some of the minigames: ***
CONS( 1982, monkeysee,  0,         0, monkeysee, monkeysee, monkeysee_state, empty_init, "Tandy Radio Shack", "Monkey See (1982 version)", MACHINE_SUPPORTS_SAVE )

COMP( 1976, speechp,    0,         0, speechp,   speechp,   speechp_state,   empty_init, "Telesensory Systems, Inc.", "Speech+", MACHINE_SUPPORTS_SAVE )

CONS( 1979, timaze,     0,         0, timaze,    timaze,    timaze_state,    empty_init, "Texas Instruments", "unknown electronic maze game (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1979, copycat,    0,         0, copycat,   copycat,   copycat_state,   empty_init, "Tiger Electronics", "Copy Cat (model 7-520)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, copycatm2,  copycat,   0, copycatm2, copycatm2, copycatm2_state, empty_init, "Tiger Electronics", "Copy Cat (model 7-522)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, ditto,      0,         0, ditto,     ditto,     ditto_state,     empty_init, "Tiger Electronics", "Ditto", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, 7in1ss,     0,         0, ss7in1,    ss7in1,    ss7in1_state,    empty_init, "Tiger Electronics", "7 in 1 Sports Stadium", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, tbreakup,   0,         0, tbreakup,  tbreakup,  tbreakup_state,  empty_init, "Tomy", "Break Up (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, phpball,    0,         0, phpball,   phpball,   phpball_state,   empty_init, "Tomy", "Power House Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1980, ssports4,   0,         0, ssports4,  ssports4,  ssports4_state,  empty_init, "U.S. Games", "Super Sports-4", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1983, xl25,       0,         0, xl25,      xl25,      xl25_state,      empty_init, "Vulcan Electronics", "XL 25", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, playing cards, pen & paper, etc.
