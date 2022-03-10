// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Kevin Horton
/***************************************************************************

This driver is a collection of simple dedicated handheld and tabletop
toys based around the TMS1000 MCU series. Anything more complex or clearly
part of a series is (or will be) in its own driver, see:
- eva: Chrysler EVA-11 (and EVA-24)
- microvsn: Milton Bradley Microvision
- sag: Entex Select-A-Game Machine

(contd.) hh_tms1k child drivers:
- tispellb: TI Spelling B series gen. 1
- tispeak: TI Speak & Spell series gen. 1

About the approximated MCU frequency everywhere: The RC osc. is not that
stable on most of these handhelds. When comparing multiple video recordings
of the same game, it shows(and sounds) that the frequency range can differ
up to 50kHz. This is probably exaggerated due to components getting worn out
after many decades. TMS1000 RC curve is documented in the data manual, but
not for newer ones (rev. E or TMS1400 MCUs). TMS0970/0980 osc. is on-die.

ROM source notes when dumped from another publisher, but confident it's the same:
- arrball: Tandy Zingo
- bcheetah: Fundimensions Incredible Brain Buggy
- cmsport: Conic Basketball
- cnbaskb: Cardinal Electronic Basketball
- cnfball: Elecsonic Football
- copycat: Sears Follow Me
- ditto: Tandy Electronic Pocket Repeat
- fxmcr165: Tandy Science Fair Microcomputer Trainer
- ginv1000: Tandy Cosmic 1000 Fire Away
- gjackpot: Entex Electronic Jackpot: Gin Rummy & Black Jack
- gpoker: Entex Electronic Poker
- matchnum: LJN Electronic Concentration
- palmf31: Toshiba BC-8018B
- ti1250: Texas Instruments TI-1200
- ti25503: Texas Instruments TI-1265
- ti5100: loose 1979 TMS1073NL chip

TODO:
- Verify output PLA and microinstructions PLA for MCUs that have been dumped
  electronically (mpla is usually the default, opla is often custom).
- unknown MCU clocks for some, especially if no YouTube videos are found
- Fake-press ON button when emulation starts for machines that have it on the
  button matrix (doesn't look like any relies on it though).
- t7in1ss: in 2-player mode, game select and skill select can be configured after
  selecting a game? Possibly BTANB, players are expected to quickly press the
  "First Up" button after the alarm sound.
- bship discrete sound, netlist is documented
- finish bshipb SN76477 sound
- improve elecbowl driver
- tithermos temperature sensor comparator (right now just the digital clock works)
- is alphie(patent) the same as the final version?
- is starwbcp the same as MP3438? (starwbc is MP3438A)

============================================================================

Let's use this driver for a list of known devices and their serials,
excluding most of TI's own products(they normally didn't use "MP" codes).
For TI's calculators, a comprehensive list of MCU serials is available
on Joerg Woerner's datamath.org: http://www.datamath.org/IC_List.htm

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
 *MP0220   TMS1000   1980, Tomy Teacher
 @MP0230   TMS1000   1980, Entex Blast It (6015)
 @MP0271   TMS1000   1982, Radio Shack Monkey See
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
 @MP1185   TMS1100   1979, Fonas 3 in 1: Football, Basketball, Soccer
 @MP1193   TMS1100   1980, Tandy Championship Football (model 60-2150)
 @MP1204   TMS1100   1980, Entex Baseball 3 (6007)
 *MP1209   TMS1100   1980, U.S. Games Space Cruiser/Strategy Football
 @MP1211   TMS1100   1980, Entex Space Invader (6012)
 @MP1215   TMS1100   1980, Tiger Playmaker
 @MP1218   TMS1100   1980, Entex Basketball 2 (6010)
 @MP1219   TMS1100   1980, U.S. Games Super Sports-4
 @MP1221   TMS1100   1980, Entex Raise The Devil (6011)
 @MP1231   TMS1100   1984, Tandy 3 in 1 Sports Arena (model 60-2178)
 @MP1296   TMS1100   1982, Entex Black Knight Pinball (6081)
 @MP1311   TMS1100   1981, Bandai TC7: Air Traffic Control
 @MP1312   TMS1100   1983, Gakken FX-Micom R-165/Radio Shack Science Fair Microcomputer Trainer
 *MP1359   TMS1100?  1985, Capsela CRC2000
 @MP1525   TMS1170   1980, Coleco Head to Head: Electronic Baseball
 @MP1604   TMS1370   1982, Gakken Invader 2000/Tandy Cosmic Fire Away 3000
 @MP1801   TMS1700   1981, Tiger Ditto/Tandy Pocket Repeat (model 60-2152)
 @MP2105   TMS1370   1979, Gakken/Entex Poker (6005)
 @MP2110   TMS1370   1980, Gakken Invader/Tandy Fire Away
 @MP2139   TMS1370   1981, Gakken Galaxy Invader 1000/Tandy Cosmic 1000 Fire Away
 @MP2726   TMS1040   1979, Tomy Break Up
 *MP2788   TMS1070?  1980, Bandai Flight Time (? note: VFD-capable)
 @MP3005   TMS1730   1989, Tiger Copy Cat (model 7-522)
 @MP3200   TMS1000   1978, Parker Brothers Electronic Master Mind
 @MP3201   TMS1000   1977, Milton Bradley Electronic Battleship (1977, model 4750A)
 *MP3206   TMS1000   1979, Concept 2000 Mr. Mus-I-Cal
 @MP3208   TMS1000   1977, Milton Bradley Electronic Battleship (1977, model 4750B)
 @MP3226   TMS1000   1978, Milton Bradley Simon (Rev A)
 *MP3232   TMS1000   1979, Fonas 2 Player Baseball (no "MP" on chip label)
 @MP3260   TMS1000   1979, Electroplay Quickfire
 @MP3300   TMS1000   1979, Milton Bradley Simon (Rev F)
 @MP3301A  TMS1000   1979, Milton Bradley Big Trak
 *MP3312   TMS1000   1980, Nathan Mega 10000
 @MP3320A  TMS1000   1979, Coleco Head to Head: Electronic Basketball
 @MP3321A  TMS1000   1979, Coleco Head to Head: Electronic Hockey
 @MP3352   TMS1200   1979, Tiger Sub Wars (model 7-490)
 @M32001   TMS1000   1981, Coleco Quiz Wiz Challenger (note: MP3398, MP3399, M3200x?)
 *M32018   TMS1000   1990, unknown device (have decap/dump)
  M32045B  TMS1000   1983, Chrysler Electronic Voice Alert (11-function) -> eva.cpp
 @MP3403   TMS1100   1978, Marx Electronic Bowling
 @MP3404   TMS1100   1978, Parker Brothers Merlin
 @MP3405   TMS1100   1979, Coleco Amaze-A-Tron
 *MP3407   TMS1100   1979, General Electric The Great Awakening (model 7-4880)
 @MP3415   TMS1100   1978, Coleco Electronic Quarterback
 @MP3435   TMS1100   1979, Coleco Zodiac
 @MP3438A  TMS1100   1979, Kenner Star Wars Electronic Battle Command
  MP3450A  TMS1100   1979, Microvision cartridge: Block Buster
  MP3454   TMS1100   1979, Microvision cartridge: Star Trek Phaser Strike
  MP3455   TMS1100   1980, Microvision cartridge: Pinball
  MP3457   TMS1100   1979, Microvision cartridge: Mindbuster
 @MP3460   TMS1100   1979, Coleco Head to Head: Electronic Football
  MP3474   TMS1100   1979, Microvision cartridge: Vegas Slots
  MP3475   TMS1100   1979, Microvision cartridge: Bowling
 @MP3476   TMS1100   1979, Milton Bradley Super Simon
  MP3479   TMS1100   1980, Microvision cartridge: Baseball
  MP3481   TMS1100   1979, Microvision cartridge: Connect Four
 @MP3489   TMS1100   1980, Kenner Live Action Football
 @MP3491   TMS1100   1979, Mattel Thoroughbred Horse Race Analyzer
 *MP3493   TMS1100   1980, Milton Bradley OMNI Entertainment System (1/2)
 *MP3494   TMS1100   1980, Milton Bradley OMNI Entertainment System (2/2)
  MP3496   TMS1100   1980, Microvision cartridge: Sea Duel
  M34009   TMS1100   1981, Microvision cartridge: Alien Raiders (note: MP3498, MP3499, M3400x..)
 @M34012   TMS1100   1980, Mattel Dungeons & Dragons - Computer Labyrinth Game
 *M34014   TMS1100   1981, Coleco Bowlatronic
  M34017   TMS1100   1981, Microvision cartridge: Cosmic Hunter
 @M34018   TMS1100   1981, Coleco Head to Head: Electronic Boxing
 @M34038   TMS1100   1982, Parker Brothers Lost Treasure
  M34047   TMS1100   1982, Microvision cartridge: Super Blockbuster
 @M34078A  TMS1100   1983, Milton Bradley Electronic Arcade Mania
 *M34137   TMS1100?  1985, Technasonic Weight Talker
 @MP4486A  TMS1000C  1983, Vulcan XL 25
 *MP6061   TMS0970   1979, Texas Instruments Electronic Digital Thermostat (from patent, the one in MAME didn't have a label)
 @MP6100A  TMS0980   1979, Ideal Electronic Detective
 @MP6101B  TMS0980   1979, Parker Brothers Stop Thief
 *MP6354   ?         1982, Tsukuda The Dracula (? note: 40-pin, VFD-capable)
 *MP6361   ?         1983, <unknown> Defender Strikes (? note: VFD-capable)
 @MP7302   TMS1400   1980, Tiger Deluxe Football with Instant Replay
 @MP7304   TMS1400   1982, Tiger 7 in 1 Sports Stadium (model 7-555)
 @MP7313   TMS1400   1980, Parker Brothers Bank Shot
 @MP7314   TMS1400   1980, Parker Brothers Split Second
  MP7324   TMS1400   1985, Tiger K28/Coleco Talking Teacher -> tispeak.cpp
 @MP7332   TMS1400   1981, Milton Bradley Dark Tower
 @MP7334   TMS1400   1981, Coleco Total Control 4
 @MP7351   TMS1400   1982, Parker Brothers Master Merlin
 @MP7551   TMS1670   1980, Entex Color Football 4 (6009)
 @MPF553   TMS1670   1980, Gakken/Entex Jackpot: Gin Rummy & Black Jack (6008) (note: assume F to be a misprint)
  MP7573   TMS1670   1981, Entex Select-A-Game cartridge: Football 4 -> sag.cpp
 *M30026   TMS2370   1983, Yaesu FT-757 Display Unit part
 *M95041   TMS2670   1983, Tsukuda Game Pachinko (have decap, missing MCU emulation)

  inconsistent:

 @TMS1007  TMS1000   1976, TSI Speech+ (S14002-A)
 @CD7282SL TMS1100   1981, Tandy-12 (serial is similar to TI Speak & Spell series?)

  (* means undumped unless noted, @ denotes it's in this driver)

***************************************************************************/

#include "emu.h"
#include "includes/hh_tms1k.h"

#include "machine/clock.h"
#include "machine/ds8874.h"
#include "machine/timer.h"
#include "machine/tmc0999.h"
#include "machine/tms1024.h"
#include "sound/beep.h"
#include "sound/s14001a.h"
#include "sound/sn76477.h"
#include "video/hlcd0515.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "softlist_dev.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "t7in1ss.lh"
#include "alphie.lh"
#include "amaztron.lh" // clickable
#include "arcmania.lh" // clickable
#include "arrball.lh"
#include "astro.lh"
#include "bankshot.lh"
#include "bcheetah.lh"
#include "bigtrak.lh"
#include "blastit.lh"
#include "bship.lh" // clickable
#include "cmsport.lh"
#include "cnbaskb.lh"
#include "cnfball.lh"
#include "cnfball2.lh"
#include "cnsector.lh" // clickable
#include "comp4.lh" // clickable
#include "copycat.lh" // clickable
#include "copycatm2.lh" // clickable
#include "dataman.lh"
#include "ditto.lh" // clickable
#include "dxfootb.lh"
#include "cqback.lh"
#include "ebball.lh"
#include "ebball2.lh"
#include "ebball3.lh"
#include "ebaskb2.lh"
#include "ebknight.lh"
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
#include "liveafb.lh"
#include "lostreas.lh" // clickable
#include "matchnum.lh" // clickable
#include "mathmagi.lh"
#include "mathmarv.lh"
#include "mbdtower.lh" // clickable
#include "mdndclab.lh" // clickable
#include "merlin.lh" // clickable
#include "mmerlin.lh" // clickable
#include "monkeysee.lh"
#include "palmf31.lh"
#include "palmmd8.lh"
#include "pbmastm.lh"
#include "phpball.lh"
#include "playmaker.lh"
#include "qfire.lh" // clickable
#include "quizwizc.lh"
#include "raisedvl.lh"
#include "simon.lh" // clickable
#include "speechp.lh"
#include "splitsec.lh"
#include "ssimon.lh" // clickable
#include "ssports4.lh"
#include "starwbc.lh" // clickable
#include "stopthief.lh" // clickable
#include "subwars.lh"
#include "t3in1sa.lh"
#include "tandy12.lh" // clickable
#include "tbreakup.lh"
#include "tc4.lh"
#include "tc7atc.lh"
#include "tcfball.lh"
#include "tcfballa.lh"
#include "ti1250.lh"
#include "ti1270.lh"
#include "ti1680.lh"
#include "ti25503.lh"
#include "ti30.lh"
#include "ti5100.lh"
#include "timaze.lh"
#include "tisr16.lh"
#include "tithermos.lh"
#include "wizatron.lh"
#include "xl25.lh" // clickable
#include "zodiac.lh" // clickable

//#include "hh_tms1k_test.lh" // common test-layout - use external artwork


// machine_start/reset

void hh_tms1k_state::machine_start()
{
	// resolve handlers
	m_out_power.resolve();

	// zerofill
	m_o = 0;
	m_r = 0;
	m_inp_mux = 0;
	m_power_on = false;
	m_grid = 0;
	m_plate = 0;

	// register for savestates
	save_item(NAME(m_o));
	save_item(NAME(m_r));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_power_on));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_tms1k_state::machine_reset()
{
	set_power(true);
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_tms1k_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}

u8 hh_tms1k_state::read_rotated_inputs(int columns, u8 rowmask)
{
	u8 ret = 0;
	u16 colmask = (1 << columns) - 1;

	// read selected input columns
	for (int i = 0; i < 8; i++)
		if (1 << i & rowmask && m_inputs[i]->read() & m_inp_mux & colmask)
			ret |= 1 << i;

	return ret;
}

void hh_tms1k_state::switch_change(int sel, u32 mask, bool next)
{
	// config switches (for direct control)
	ioport_field *inp = m_inputs[sel]->field(mask);

	if (next && inp->has_next_setting())
		inp->select_next_setting();
	else if (!next && inp->has_previous_setting())
		inp->select_previous_setting();
}

INPUT_CHANGED_MEMBER(hh_tms1k_state::reset_button)
{
	// when an input is directly wired to MCU INIT pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(hh_tms1k_state::power_button)
{
	set_power((bool)param);
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
	set_power(false);
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void hh_tms1k_state::set_power(bool state)
{
	m_power_on = state;
	m_out_power = state ? 1 : 0;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  A-One LSI Match Number
  * PCB label PT-204 "Pair Card"
  * TMS1000NLL MP0163 (die label 1000B, MP0163)
  * 2x2-digit 7seg LED displays + 3 LEDs, 1-bit sound

  A-One was a subsidiary of Bandai? The PCB serial PT-xxx is same, and the font
  used on the boxes for "A-One LSI" is same as "Bandai Electronics" from early-80s.

  known releases:
  - Japan: Match Number, published by A-One (white case, Queen playing card bezel)
  - USA: Electronic Concentration, published by LJN (black case, rainbow pattern bezel)
  - UK: Electronic Concentration, published by Peter Pan Playthings (same as USA version)

***************************************************************************/

class matchnum_state : public hh_tms1k_state
{
public:
	matchnum_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void matchnum(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void matchnum_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void matchnum_state::write_r(u16 data)
{
	// R3-R5,R8-R10: input mux
	m_inp_mux = (data >> 3 & 7) | (data >> 5 & 0x38);

	// R6,R7: speaker out
	m_speaker->level_w(data >> 6 & 3);

	// R0-R3: digit/led select
	m_r = data;
	update_display();
}

void matchnum_state::write_o(u16 data)
{
	// O0-O6: digit segments A-G
	// O7: led data
	m_o = data;
	update_display();
}

u8 matchnum_state::read_k()
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

void matchnum_state::matchnum(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(matchnum_state::read_k));
	m_maincpu->r().set(FUNC(matchnum_state::write_r));
	m_maincpu->o().set(FUNC(matchnum_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_matchnum);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( matchnum )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0163", 0x0000, 0x0400, CRC(37507600) SHA1(b1d4d8ea563e97ef378b42c44cb3ea4eb6abe0d2) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_matchnum_output.pla", 0, 365, CRC(da29670c) SHA1(bcec28bf25dc8c81d08851ad8a3f4e89f413017a) )
ROM_END





/***************************************************************************

  A-One LSI Arrange Ball
  * PCB label Kaken, PT-249
  * TMS1000NLL MP0166 (die label 1000B, MP0166)
  * 2-digit 7seg LED display + 22 LEDs, 1-bit sound

  known releases:
  - Japan/World: Arrange Ball, published by A-One (black case)
  - USA(1): Zingo (model 60-2123), published by Tandy (red case)
  - USA(2): Computer Impulse, published by LJN (white case)
  - Germany: Fixball, unknown publisher, same as LJN version

***************************************************************************/

class arrball_state : public hh_tms1k_state
{
public:
	arrball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void arrball(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void arrball_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void arrball_state::write_r(u16 data)
{
	// R8: input mux (always set)
	m_inp_mux = data >> 8 & 1;

	// R9,R10: speaker out
	m_speaker->level_w(data >> 9 & 3);

	// R0-R6: digit/led select
	m_r = data;
	update_display();
}

void arrball_state::write_o(u16 data)
{
	// O0-O6: digit segments/led data
	m_o = data;
	update_display();
}

u8 arrball_state::read_k()
{
	// K: multiplexed inputs (actually just 1)
	return read_inputs(1);
}

// config

static INPUT_PORTS_START( arrball )
	PORT_START("IN.0") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shot") // pressed when START lights up
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Stop")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Speed" )
	PORT_CONFSETTING(    0x00, "Slow" )
	PORT_CONFSETTING(    0x08, "Fast" )
INPUT_PORTS_END

void arrball_state::arrball(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(arrball_state::read_k));
	m_maincpu->r().set(FUNC(arrball_state::write_r));
	m_maincpu->o().set(FUNC(arrball_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7, 7);
	m_display->set_segmask(0x10, 0x7f);
	m_display->set_segmask(0x20, 0x06); // left digit only segments B and C
	config.set_default_layout(layout_arrball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( arrball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0166", 0x0000, 0x0400, CRC(a78694db) SHA1(362aa6e356288e8df7da610246bd01fe72985d57) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_arrball_output.pla", 0, 365, CRC(ffc206fb) SHA1(339be3f066fb2f075211c554e81260b49cd83d15) )
ROM_END





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
	mathmagi_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void mathmagi(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void mathmagi_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void mathmagi_state::write_r(u16 data)
{
	// R3,R5-R7,R9,R10: input mux
	m_inp_mux = (data >> 3 & 1) | (data >> 4 & 0xe) | (data >> 5 & 0x30);

	// R0-R7: 7seg leds
	// R8: custom math symbols digit
	// R9: custom equals digit
	// R10: misc lamps
	m_r = data;
	update_display();
}

void mathmagi_state::write_o(u16 data)
{
	// O1-O7: led/digit segment data
	// O0: N/C
	m_o = data;
	update_display();
}

u8 mathmagi_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

/* physical button layout and labels are like this:

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

// output PLA is not decapped, this was made by hand
static const u16 mathmagi_output_pla[0x20] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0, 1, 2, 3, 4, 5, 6, 7
	0x7f, 0x6f, 0x53, 0x50, 0x08, 0x79, 0x40, 0x00, // 8, 9, questionmark, r, underscore?, E, -(negative), empty
	0x00, 0x40, 0x08, 0x7c, 0x02, 0x42, 0x06, 0x6e, // empty, led4/-, led3, b, led2, +, ×, y
	0x01, 0x41, 0x09, 0,    0,    0x5c, 0,    0x3d  // led1, ÷, =, ?, ?, o, ?, G
};

void mathmagi_state::mathmagi(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 175000); // approximation - RC osc. R=68K, C=82pF
	m_maincpu->set_output_pla(mathmagi_output_pla);
	m_maincpu->k().set(FUNC(mathmagi_state::read_k));
	m_maincpu->r().set(FUNC(mathmagi_state::write_r));
	m_maincpu->o().set(FUNC(mathmagi_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_mathmagi);

	// no sound!
}

// roms

ROM_START( mathmagi )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1030", 0x0000, 0x0800, CRC(a81d7ccb) SHA1(4756ce42f1ea28ce5fe6498312f8306f10370969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_mathmagi_output.pla", 0, 365, NO_DUMP )
ROM_END





/***************************************************************************

  Bandai System Control Car: Cheetah 「システムコントロールカー チーター」
  * TMS1000NLL MP0915 (die label 1000B, MP0915)
  * 2 motors (one for back axis, one for steering), no sound

  It's a programmable buggy, like Big Track but much simpler. To add a command
  step in program-mode, press a direction key and one of the time delay number
  keys at the same time. To run the program(max 24 steps), switch to run-mode
  and press the go-key.

  known releases:
  - Japan: System Control Car: Cheetah, published by Bandai
  - USA: The Incredible Brain Buggy, published by Fundimensions
  - UK: The Incredible Brain Buggy, published by Palitoy (same as USA version)

***************************************************************************/

class bcheetah_state : public hh_tms1k_state
{
public:
	bcheetah_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_motor1(*this, "motor1"),
		m_motor2_left(*this, "motor2_left"),
		m_motor2_right(*this, "motor2_right")
	{ }

	void bcheetah(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();

	output_finder<> m_motor1;
	output_finder<> m_motor2_left;
	output_finder<> m_motor2_right;
};

void bcheetah_state::machine_start()
{
	hh_tms1k_state::machine_start();
	m_motor1.resolve();
	m_motor2_left.resolve();
	m_motor2_right.resolve();
}

// handlers

void bcheetah_state::write_r(u16 data)
{
	// R0-R4: input mux
	// R5,R6: tied to K4??
	m_inp_mux = data & 0x1f;
}

void bcheetah_state::write_o(u16 data)
{
	// O1: back motor (drive)
	// O0: front motor steer left
	// O2: front motor steer right
	// O3: GND, other: N/C
	m_motor1 = data >> 1 & 1;
	m_motor2_left = data & 1;
	m_motor2_right = data >> 2 & 1;
}

u8 bcheetah_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 100000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(bcheetah_state::read_k));
	m_maincpu->r().set(FUNC(bcheetah_state::write_r));
	m_maincpu->o().set(FUNC(bcheetah_state::write_o));

	config.set_default_layout(layout_bcheetah);

	// no sound!
}

// roms

ROM_START( bcheetah )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0915", 0x0000, 0x0400, CRC(2968c81e) SHA1(d1e6691952600e88ccf626cb3d683419a1e8468c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bcheetah_output.pla", 0, 365, CRC(cc6d1ecd) SHA1(b0635a841d8850c36c1f414abe0571b81884b972) )
ROM_END





/***************************************************************************

  Bandai TC7: Air Traffic Control
  * TMS1100 MCU, label MP1311 (die label 1100E, MP1311)
  * 4-digit 7seg LED display, 40 other LEDs, 1-bit sound

  It is a very complicated game, refer to the manual on how to play.

***************************************************************************/

class tc7atc_state : public hh_tms1k_state
{
public:
	tc7atc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void tc7atc(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void tc7atc_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void tc7atc_state::write_r(u16 data)
{
	// R5: speaker out
	m_speaker->level_w(BIT(data, 5));

	// R0-R4: input mux, led select
	// R6-R9: digit select
	m_inp_mux = m_r = data;
	update_display();
}

void tc7atc_state::write_o(u16 data)
{
	// O0-O7: led data
	m_o = data;
	update_display();
}

u8 tc7atc_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( tc7atc )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("High Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Hazard")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("West")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("East")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Arrive")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Depart")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Flight Path A / Level 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Flight Path B / Level 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Flight Path C / Level 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Flight Path D / Level 4")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Altitude Descend")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Altitude Ascend")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Air Speed Decrease")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Air Speed Increase")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("10%")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("20%")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("40%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("80%")
INPUT_PORTS_END

void tc7atc_state::tc7atc(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=68K, C=47pF
	m_maincpu->k().set(FUNC(tc7atc_state::read_k));
	m_maincpu->r().set(FUNC(tc7atc_state::write_r));
	m_maincpu->o().set(FUNC(tc7atc_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3c0, 0x7f);
	config.set_default_layout(layout_tc7atc);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tc7atc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1311", 0x0000, 0x0800, CRC(704f5e1b) SHA1(765dce31798640480eab7576550c5378d6351b65) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tc7atc_output.pla", 0, 365, CRC(0e6e3096) SHA1(375beb43657af0cc3070e581b42e501878c0eaaa) )
ROM_END





/***************************************************************************

  Canon Palmtronic F-31, Canon Canola L813, Toshiba BC-8111B, Toshiba BC-8018B,
  Triumph-Adler 81 SN, Silver-Reed 8J, more
  * TMS1040 MCU label TMS1045NL (die label 1040A, 1045)
  * 9-digit cyan VFD display (leftmost may be custom)

  TMS1045NL is a versatile calculator chip for 3rd party manufacturers, used
  by Canon, Toshiba, and several smaller companies. It doesn't look like it
  was used in any TI calculator. It was up to the manufacturer to choose which
  functions(keys) to leave out.

***************************************************************************/

class palmf31_state : public hh_tms1k_state
{
public:
	palmf31_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void palmf31(machine_config &config);

private:
	void update_display();
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void palmf31_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void palmf31_state::write_r(u16 data)
{
	// R0-R8: select digit, input mux
	m_r = m_inp_mux = data;
	update_display();
}

void palmf31_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 palmf31_state::read_k()
{
	// K: multiplexed inputs, one of the columns is K2+K8
	u8 data = read_inputs(9);
	if (data & 1)
		data = (data & 0xe) | 0xa;

	// switches are on K1
	if (m_inp_mux & m_inputs[9]->read())
		data |= 1;

	return data;
}

// config

static INPUT_PORTS_START( palmf31 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("M+") // add to memory
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("SC") // sign change

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("M-") // subtract from memory
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("RV") // reverse

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as M+
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as sqrt

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("(")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("1/x")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(")")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT)

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME(UTF8_SMALL_PI)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("RM/CM") // combined function (press twice)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("% +/-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("CM") // clear memory
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("RM") // recall memory

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("C") // clear (all)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME(UTF8_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("CI") // clear indicator
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("CI/C") // combined function (press twice)

	PORT_START("IN.9") // K1
	PORT_CONFNAME( 0x1f, 0x00, "DP" ) // display point
	PORT_CONFSETTING(    0x02, "+" )
	PORT_CONFSETTING(    0x01, "0" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x10, "4" )
	PORT_CONFSETTING(    0x00, "F" )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x100, 0x000, "AM" ) // accumulate memory
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x100, DEF_STR( On ) )
INPUT_PORTS_END

void palmf31_state::palmf31(machine_config &config)
{
	// basic machine hardware
	TMS1040(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(palmf31_state::read_k));
	m_maincpu->o().set(FUNC(palmf31_state::write_o));
	m_maincpu->r().set(FUNC(palmf31_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_palmf31);

	// no sound!
}

// roms

ROM_START( palmf31 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1045nl", 0x0000, 0x0400, CRC(0c42d43e) SHA1(b76d404623e3abfd0b237ee1c0b46e83f96ceb78) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_palmf31_micro.pla", 0, 867, CRC(639cbc13) SHA1(a96152406881bdfc7ddc542cf4b478525c8b0e23) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_palmf31_output.pla", 0, 365, CRC(bc295ea6) SHA1(7e7c9ed0c1e5e37173dd8f297473516c410bca8c) )
ROM_END





/***************************************************************************

  Canon Palmtronic MD-8 (Multi 8) / Canon Canola MD 810
  * TMS1070 MCU label TMC1079 (die label 1070B, 1079A)
  * 2-line cyan VFD display, each 9-digit 7seg + 1 custom (label 20-ST-22)
  * PCB label Canon EHI-0115-03

  The only difference between MD-8 and MD 810 is the form factor. The latter
  is a tabletop calculator.

***************************************************************************/

class palmmd8_state : public hh_tms1k_state
{
public:
	palmmd8_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void palmmd8(machine_config &config);

private:
	void update_display();
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void palmmd8_state::update_display()
{
	// M-digit is on in memory mode, upper row is off in single mode
	u32 m = (m_inputs[10]->read() & 0x10) ? 0x100000 : 0;
	u32 mask = (m_inputs[10]->read() & 0x20) ? 0xfffff : 0xffc00;

	// R10 selects display row
	u32 sel = (m_r & 0x400) ? (m_r & 0x3ff) : (m_r << 10 & 0xffc00);
	m_display->matrix((sel & mask) | m, m_o);
}

void palmmd8_state::write_r(u16 data)
{
	// R0-R10: input mux, select digit
	m_r = m_inp_mux = data;
	update_display();
}

void palmmd8_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = bitswap<8>(data,0,4,5,6,7,1,2,3);
	update_display();
}

u8 palmmd8_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(11);
}

// config

static INPUT_PORTS_START( palmmd8 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("% +/-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("RM") // recall memory
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("SC") // sign change
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("CM") // clear memory
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("M+") // add to memory
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("RV") // reverse
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CI/C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.10") // R10
	PORT_CONFNAME( 0x31, 0x20, "Mode" ) // bit 4 indicates M-digit on/off, bit 5 indicates upper row filament on/off
	PORT_CONFSETTING(    0x31, "Memory" )
	PORT_CONFSETTING(    0x01, "Single" )
	PORT_CONFSETTING(    0x20, "Process" )
	PORT_CONFNAME( 0x02, 0x00, "AM" ) // accumulate memory
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void palmmd8_state::palmmd8(machine_config &config)
{
	// basic machine hardware
	TMS1070(config, m_maincpu, 250000); // approximation - RC osc. R=56K, C=68pf
	m_maincpu->k().set(FUNC(palmmd8_state::read_k));
	m_maincpu->o().set(FUNC(palmmd8_state::write_o));
	m_maincpu->r().set(FUNC(palmmd8_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(21, 8);
	m_display->set_segmask(0xfffff, 0xff);
	config.set_default_layout(layout_palmmd8);

	// no sound!
}

// roms

ROM_START( palmmd8 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc1079nl", 0x0000, 0x0400, CRC(202c5ed8) SHA1(0143975cac20cb4a4e9f659ca0535e8a9056f5bb) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_palmmd8_output.pla", 0, 365, CRC(e999cece) SHA1(c5012877cd030a4dc66228f109fa23eec1867873) )
ROM_END





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
	amaztron_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void amaztron(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void amaztron_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void amaztron_state::write_r(u16 data)
{
	// R0-R5: input mux
	m_inp_mux = data & 0x3f;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R6,R7: leds
	// R8,R9: select digit
	m_r = data >> 6 & 0xf;
	update_display();
}

void amaztron_state::write_o(u16 data)
{
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 amaztron_state::read_k()
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 300000); // approximation - RC osc. R=33K?, C=100pF
	m_maincpu->k().set(FUNC(amaztron_state::read_k));
	m_maincpu->r().set(FUNC(amaztron_state::write_r));
	m_maincpu->o().set(FUNC(amaztron_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 7);
	m_display->set_segmask(0xc, 0x7f);
	config.set_default_layout(layout_amaztron);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( amaztron )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3405", 0x0000, 0x0800, CRC(9cbc0009) SHA1(17772681271b59280687492f37fa0859998f041d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_amaztron_output.pla", 0, 365, CRC(f3875384) SHA1(3c256a3db4f0aa9d93cf78124db39f4cbdc57e4a) )
ROM_END





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
	zodiac_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void zodiac(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void zodiac_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void zodiac_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4,R8: input mux
	m_inp_mux = (data & 0x1f) | (data >> 3 & 0x20);

	// R0-R7: digit select
	// R8,R9: led select
	m_r = data & 0x3ff;
	update_display();
}

void zodiac_state::write_o(u16 data)
{
	// O0-O7: digit segment/led data
	m_o = bitswap<8>(data,0,7,6,5,4,3,2,1);
	update_display();
}

u8 zodiac_state::read_k()
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Clear") PORT_CHAR(8)

	PORT_START("IN.5") // R8
	PORT_CONFNAME( 0x03, 0x01, "Mode")
	PORT_CONFSETTING(    0x01, "Horoscope" )
	PORT_CONFSETTING(    0x02, "Preview" )
	PORT_CONFSETTING(    0x00, "Answer" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void zodiac_state::zodiac(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 500000); // approximation - RC osc. R=18K, C=100pF
	m_maincpu->k().set(FUNC(zodiac_state::read_k));
	m_maincpu->r().set(FUNC(zodiac_state::write_r));
	m_maincpu->o().set(FUNC(zodiac_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_zodiac);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( zodiac )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3435", 0x0000, 0x0800, CRC(ecdc3160) SHA1(a7e82d66314a039fcffeddf99919d9f9ad42d61d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, BAD_DUMP CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_zodiac_output.pla", 0, 365, NO_DUMP )

	ROM_REGION16_LE( 0x40, "maincpu:opla_b", ROMREGION_ERASE00 ) // verified, electronic dump
	ROM_LOAD16_BYTE( "tms1100_zodiac_output.bin", 0, 0x20, CRC(d35d64cb) SHA1(315e7ca9a18a06ff7dc0b95c6f119e093b15a41f) )
ROM_END





/***************************************************************************

  Coleco Electronic Quarterback (model 2120)
  * TMS1100NLL MP3415 (die label same)
  * 9-digit LED grid, 1-bit sound

  known releases:
  - USA(1): Electronic Quarterback, published by Coleco
  - USA(2): Electronic Touchdown, published by Sears

***************************************************************************/

class cqback_state : public hh_tms1k_state
{
public:
	cqback_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void cqback(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cqback_state::update_display()
{
	// R9 selects between segments B/C or A'/D'
	u16 seg = m_o;
	if (m_r & 0x200)
		seg = (m_o << 7 & 0x300) | (m_o & 0xf9);

	m_display->matrix(m_r & 0x1ff, seg);
}

void cqback_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R0-R9: select digit/segment
	m_r = data;
	update_display();
}

void cqback_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 cqback_state::read_k()
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
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.3") // K8
	PORT_CONFNAME( 0x01, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) ) // TP1-TP2

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void cqback_state::cqback(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 310000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(cqback_state::read_k));
	m_maincpu->r().set(FUNC(cqback_state::write_r));
	m_maincpu->o().set(FUNC(cqback_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 11);
	m_display->set_segmask(0x1ff, 0xff);
	m_display->set_bri_levels(0.003, 0.03); // offense leds are brighter
	config.set_default_layout(layout_cqback);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cqback )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3415.u4", 0x0000, 0x0800, CRC(65ebdabf) SHA1(9b5cf5adaf9132ced87f611ae8c3148b9b62ba89) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_cqback_output.pla", 0, 365, CRC(c6dcbfd0) SHA1(593b6b7de981a28d1b4a33336b39df92d02ed4f4) )
ROM_END





/***************************************************************************

  Coleco Head to Head: Electronic Football (model 2140)
  * TMS1100NLLE (rev. E!) MP3460 (die label same)
  * 2*SN75492N LED display drivers, 9-digit LED grid, 1-bit sound

  LED electronic football game. To distinguish between offense and defense,
  offense blips appear brighter. The hardware is similar to cqback.

  known releases:
  - USA(1): Head to Head: Electronic Football, published by Coleco
  - USA(2): Team Play Football, published by Sears

***************************************************************************/

class h2hfootb_state : public hh_tms1k_state
{
public:
	h2hfootb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void h2hfootb(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void h2hfootb_state::update_display()
{
	m_display->matrix(m_r & 0x1ff, m_o | (m_r >> 1 & 0x100));
}

void h2hfootb_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R8: input mux
	m_inp_mux = data & 0x1ff;

	// R0-R8: select led
	// R9: led between digits
	m_r = data;
	update_display();
}

void h2hfootb_state::write_o(u16 data)
{
	// O0-O7: digit segments A-G,A'
	m_o = data;
	update_display();
}

u8 h2hfootb_state::read_k()
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void h2hfootb_state::h2hfootb(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 310000); // approximation - RC osc. R=39K, C=100pF
	m_maincpu->k().set(FUNC(h2hfootb_state::read_k));
	m_maincpu->r().set(FUNC(h2hfootb_state::write_r));
	m_maincpu->o().set(FUNC(h2hfootb_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 9);
	m_display->set_segmask(0x1ff, 0x7f);
	m_display->set_bri_levels(0.003, 0.03); // offense leds are brighter
	config.set_default_layout(layout_h2hfootb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( h2hfootb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3460.u3", 0x0000, 0x0800, CRC(3a4e53a8) SHA1(5052e706f992c6c4bada1fa7769589eec3df6471) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hfootb_output.pla", 0, 365, CRC(c8d85873) SHA1(16bd6fc8e3cd16d5f8fd32d0c74e67de77f5487e) )
ROM_END





/***************************************************************************

  Coleco Head to Head: Electronic Basketball (model 2150)
  * TMS1000NLL MP3320A (die label 1000E MP3320A)
  * 2-digit 7seg LED display, LED grid display, 1-bit sound

  Coleco Head to Head: Electronic Hockey (model 2160)
  * TMS1000NLL E MP3321A (die label 1000E MP3321A)
  * same PCB/hardware as above

  Unlike the COP420 version(see hh_cop400.cpp driver), each game has its own MCU.
  To begin play, press start while holding left/right.

***************************************************************************/

class h2hbaskb_state : public hh_tms1k_state
{
public:
	h2hbaskb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_cap_empty_timer(*this, "cap_empty")
	{ }

	void h2hbaskb(machine_config &config);
	void h2hhockey(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<timer_device> m_cap_empty_timer;
	TIMER_DEVICE_CALLBACK_MEMBER(cap_empty_callback);
	bool m_cap_state = false;
	attotime m_cap_charge = attotime::zero;

	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void h2hbaskb_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_cap_state));
	save_item(NAME(m_cap_charge));
}

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(h2hbaskb_state::cap_empty_callback)
{
	if (~m_r & 0x200)
		m_cap_state = false;
}

void h2hbaskb_state::update_display()
{
	// R6,R7 are commons for R0-R5
	u16 sel = 0;
	if (m_r & 0x40) sel |= (m_r & 0x3f);
	if (m_r & 0x80) sel |= (m_r & 0x3f) << 6;

	m_display->matrix(sel, m_o);
}

void h2hbaskb_state::write_r(u16 data)
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
	update_display();
}

void h2hbaskb_state::write_o(u16 data)
{
	// O1-O7: led data
	m_o = data >> 1 & 0x7f;
	update_display();
}

u8 h2hbaskb_state::read_k()
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

void h2hbaskb_state::h2hbaskb(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=43K, C=100pF
	m_maincpu->k().set(FUNC(h2hbaskb_state::read_k));
	m_maincpu->r().set(FUNC(h2hbaskb_state::write_r));
	m_maincpu->o().set(FUNC(h2hbaskb_state::write_o));

	TIMER(config, "cap_empty").configure_generic(FUNC(h2hbaskb_state::cap_empty_callback));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6+6, 7);
	m_display->set_segmask(0xc0, 0x7f);
	config.set_default_layout(layout_h2hbaskb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void h2hbaskb_state::h2hhockey(machine_config &config)
{
	h2hbaskb(config);
	config.set_default_layout(layout_h2hhockey);
}

// roms

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





/***************************************************************************

  Coleco Head to Head: Electronic Baseball (model 2180)
  * PCB labels Coleco rev C 73891/2
  * TMS1170NLN MP1525-N2 (die label MP1525)
  * 9-digit cyan VFD display, and other LEDs behind bezel, 1-bit sound

  known releases:
  - USA: Head to Head: Electronic Baseball, published by Coleco
  - Japan: Computer Baseball, published by Tsukuda

***************************************************************************/

class h2hbaseb_state : public hh_tms1k_state
{
public:
	h2hbaseb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void h2hbaseb(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void h2hbaseb_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void h2hbaseb_state::set_clock()
{
	// MCU clock is from an RC circuit with C=47pF, and R value is depending on
	// skill switch: R=51K(1) or 43K(2)
	m_maincpu->set_unscaled_clock((m_inputs[5]->read() & 1) ? 400000 : 350000);
}

void h2hbaseb_state::update_display()
{
	m_display->matrix((m_r & 0xff) | (m_r >> 1 & 0x100), (m_r & 0x100) | m_o);
}

void h2hbaseb_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R4-R7: input mux
	m_inp_mux = data >> 4 & 0xf;

	// R0-R7,R9: select vfd digit/led
	// R8: led state
	m_r = data;
	update_display();
}

void h2hbaseb_state::write_o(u16 data)
{
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data;
	update_display();
}

u8 h2hbaseb_state::read_k()
{
	// K: multiplexed inputs (note: K8(Vss row) is always on)
	return m_inputs[4]->read() | read_inputs(4);
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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, h2hbaseb_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
INPUT_PORTS_END

void h2hbaseb_state::h2hbaseb(machine_config &config)
{
	// basic machine hardware
	TMS1170(config, m_maincpu, 350000); // see set_clock
	m_maincpu->k().set(FUNC(h2hbaseb_state::read_k));
	m_maincpu->r().set(FUNC(h2hbaseb_state::write_r));
	m_maincpu->o().set(FUNC(h2hbaseb_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 9);
	m_display->set_segmask(0x1ff, 0x7f);
	config.set_default_layout(layout_h2hbaseb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( h2hbaseb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1525", 0x0000, 0x0800, CRC(b5d6bf9b) SHA1(2cc9f35f077c1209c46d16ec853af87e4725c2fd) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hbaseb_output.pla", 0, 365, CRC(cb3d7e38) SHA1(6ab4a7c52e6010b7c7158463cb499973e52ff556) )
ROM_END





/***************************************************************************

  Coleco Head to Head: Electronic Boxing (model 2190)
  * TMS1100NLL M34018-N2 (die label M34018)
  * 2-digit 7seg LED display, LED grid display, 1-bit sound

  This appears to be the last game of Coleco's Head to Head series.

***************************************************************************/

class h2hboxing_state : public hh_tms1k_state
{
public:
	h2hboxing_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void h2hboxing(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void h2hboxing_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void h2hboxing_state::write_r(u16 data)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R0-R7: select led
	// R9,R10: select digit
	m_r = data & ~0x100;
	update_display();
}

void h2hboxing_state::write_o(u16 data)
{
	// O0-O7: digit segments/led data
	m_o = data;
	update_display();
}

u8 h2hboxing_state::read_k()
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=39K, C=100pF
	m_maincpu->k().set(FUNC(h2hboxing_state::read_k));
	m_maincpu->r().set(FUNC(h2hboxing_state::write_r));
	m_maincpu->o().set(FUNC(h2hboxing_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 8);
	m_display->set_segmask(0x600, 0x7f);
	config.set_default_layout(layout_h2hboxing);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( h2hboxing )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34018", 0x0000, 0x0800, CRC(e26a11a3) SHA1(aa2735088d709fa8d9188c4fb7982a53e3a8c1bc) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_h2hboxing_output.pla", 0, 365, CRC(ffb0e63d) SHA1(31ee3f779270a23f05f9ad508283d2569ef069f1) )
ROM_END





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
	quizwizc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void quizwizc(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	u16 m_pinout = 0x07; // cartridge R pins

	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void quizwizc_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_pinout));
}

// handlers

DEVICE_IMAGE_LOAD_MEMBER(quizwizc_state::cart_load)
{
	if (!image.loaded_through_softlist())
	{
		image.seterror(image_error::UNSUPPORTED, "Can only load through softwarelist");
		return image_init_result::FAIL;
	}

	// get cartridge pinout K1 to R connections
	const char *pinout = image.get_feature("pinout");
	m_pinout = pinout ? strtoul(pinout, nullptr, 2) & 0xe7 : 0;
	m_pinout = bitswap<8>(m_pinout,4,3,7,5,2,1,6,0) << 4;

	if (m_pinout == 0)
	{
		image.seterror(image_error::INVALIDIMAGE, "Invalid cartridge pinout");
		return image_init_result::FAIL;
	}

	return image_init_result::PASS;
}

void quizwizc_state::update_display()
{
	// note: O7 is on VSS
	m_display->matrix(m_r | 0x400, m_o);
}

void quizwizc_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R4-R9: to cartridge slot
	m_inp_mux = data & 0x3f;

	// R0-R3: led select
	// R6-R9: digit select
	m_r = data;
	update_display();
}

void quizwizc_state::write_o(u16 data)
{
	// O0-O7: led/digit segment data
	m_o = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();
}

u8 quizwizc_state::read_k()
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

void quizwizc_state::quizwizc(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 300000); // approximation - RC osc. R=43K, C=100pF
	m_maincpu->k().set(FUNC(quizwizc_state::read_k));
	m_maincpu->r().set(FUNC(quizwizc_state::write_r));
	m_maincpu->o().set(FUNC(quizwizc_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10+1, 8);
	m_display->set_segmask(0x3c0, 0x7f);
	config.set_default_layout(layout_quizwizc);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	// cartridge
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "quizwiz_cart"));
	cartslot.set_must_be_loaded(true);
	cartslot.set_device_load(FUNC(quizwizc_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("quizwiz");
}

// roms

ROM_START( quizwizc )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "m32001", 0x0000, 0x0400, CRC(053657eb) SHA1(38c84f7416f79aa679f434a3d35df54cd9aa528a) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common4_micro.pla", 0, 867, CRC(80912d0a) SHA1(7ae5293ed4d93f5b7a64d43fe30c3639f39fbe5a) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_quizwizc_output.pla", 0, 365, CRC(475b7053) SHA1(8f61bf736eb41d7029a6b165cc0a184ba0a70a2a) )
ROM_END





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
	tc4_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void tc4(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	u8 m_pinout = 0xf; // cartridge K pins

	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void tc4_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_pinout));
}

// handlers

DEVICE_IMAGE_LOAD_MEMBER(tc4_state::cart_load)
{
	if (!image.loaded_through_softlist())
	{
		image.seterror(image_error::UNSUPPORTED, "Can only load through softwarelist");
		return image_init_result::FAIL;
	}

	// get cartridge pinout R9 to K connections
	const char *pinout = image.get_feature("pinout");
	m_pinout = pinout ? strtoul(pinout, nullptr, 0) & 0xf : 0xf;

	return image_init_result::PASS;
}

void tc4_state::update_display()
{
	// note: R6 is an extra column
	m_display->matrix(m_r, (m_o | (m_r << 2 & 0x100)));
}

void tc4_state::write_r(u16 data)
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
	update_display();
}

void tc4_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 tc4_state::read_k()
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

void tc4_state::tc4(machine_config &config)
{
	// basic machine hardware
	TMS1400(config, m_maincpu, 450000); // approximation - RC osc. R=27.3K, C=100pF
	m_maincpu->k().set(FUNC(tc4_state::read_k));
	m_maincpu->r().set(FUNC(tc4_state::write_r));
	m_maincpu->o().set(FUNC(tc4_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 9);
	m_display->set_segmask(0x3a0, 0x7f);
	m_display->set_bri_levels(0.005, 0.05); // offense leds are brighter
	config.set_default_layout(layout_tc4);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	// cartridge
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "tc4_cart"));
	cartslot.set_must_be_loaded(true); // system won't power on without cartridge
	cartslot.set_device_load(FUNC(tc4_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("tc4");
}

// roms

ROM_START( tc4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7334", 0x0000, 0x1000, CRC(923f3821) SHA1(a9ae342d7ff8dae1dedcd1e4984bcfae68586581) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_tc4_output.pla", 0, 557, CRC(3b908725) SHA1(f83bf5faa5b3cb51f87adc1639b00d6f9a71ad19) )
ROM_END





/***************************************************************************

  Conic Electronic Basketball
  * PCB label CONIC 101-006
  * TMS1000NLL MP0907 (die label 1000B MP0907)
  * DS8871N, 2 7seg LEDs, 30 other LEDs, 1-bit sound

  There are 3 known versions of Conic Basketball: MP0910(101-003) and
  MP0907(101-006) are nearly identical. MP0168 is found in Conic Multisport.

  known releases:
  - Hong Kong: Electronic Basketball, published by Conic
  - USA: Electronic Basketball, published by Cardinal

***************************************************************************/

class cnbaskb_state : public hh_tms1k_state
{
public:
	cnbaskb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void cnbaskb(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cnbaskb_state::update_display()
{
	m_display->matrix(m_r & 0x1fc, m_o);
}

void cnbaskb_state::write_r(u16 data)
{
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0,R1: input mux
	// R10 is also tied to K1 (locks up at boot if it's not handled)
	m_inp_mux = (data >> 8 & 4) | (data & 3);

	// R2-R6: led select
	// R7,R8: digit select
	m_r = data;
	update_display();
}

void cnbaskb_state::write_o(u16 data)
{
	// O0-O6: led/digit segment data
	// O7: N/C
	m_o = data;
	update_display();
}

u8 cnbaskb_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(cnbaskb_state::read_k));
	m_maincpu->r().set(FUNC(cnbaskb_state::write_r));
	m_maincpu->o().set(FUNC(cnbaskb_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x180, 0x7f);
	m_display->set_bri_levels(0.01, 0.1); // player led is brighter
	config.set_default_layout(layout_cnbaskb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cnbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0907", 0x0000, 0x0400, CRC(35f84f0f) SHA1(744ca60bb853a2785184042e747530a9e02488f8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cnbaskb_output.pla", 0, 365, CRC(b4e28956) SHA1(8356112da71b351420a88d7e394e7d03e429368c) )
ROM_END





/***************************************************************************

  Conic Electronic Multisport
  * PCB label CONIC 101-027(1979), or CONIC 101-021 REV A(1980, with DS8871N)
  * TMS1000 MP0168 (die label same)
  * 2 7seg LEDs, 33 other LEDs, 1-bit sound

  This handheld includes 3 games: Basketball, Ice Hockey, Soccer.
  MAME external artwork is needed for the switchable overlays.

  known releases:
  - Hong Kong: Electronic Multisport, published by Conic
  - Hong Kong: Basketball/Ice Hockey/Soccer, published by Conic (3 separate handhelds)
  - USA(1): Electronic Multisport, published by Innocron
  - USA(2): Sports Arena, published by Tandy (model 60-2158)

***************************************************************************/

class cmsport_state : public hh_tms1k_state
{
public:
	cmsport_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void cmsport(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cmsport_state::update_display()
{
	m_display->matrix(m_r & ~0x80, m_o);
}

void cmsport_state::write_r(u16 data)
{
	// R7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// R0,R9,R10: input mux
	m_inp_mux = (data & 1) | (data >> 8 & 6);

	// R0-R4,R8: led select
	// R5,R6: digit select
	m_r = data;
	update_display();
}

void cmsport_state::write_o(u16 data)
{
	// O0-O7: led/digit segment data
	m_o = data;
	update_display();
}

u8 cmsport_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(cmsport_state::read_k));
	m_maincpu->r().set(FUNC(cmsport_state::write_r));
	m_maincpu->o().set(FUNC(cmsport_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x60, 0x7f);
	m_display->set_bri_levels(0.01, 0.1); // player led is brighter
	config.set_default_layout(layout_cmsport);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cmsport )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0168.u1", 0x0000, 0x0400, CRC(0712a268) SHA1(bd4e23e5c17b28c52e7e769e44773cc9c8839bed) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cmsport_output.pla", 0, 365, CRC(7defa140) SHA1(477e3cb55e79938d6acaa911e410f6dcb974c218) )
ROM_END





/***************************************************************************

  Conic Electronic Football
  * TMS1000 MP0170 (die label same)
  * DS8874N, 3*9 LED array, 7 7seg LEDs, 1-bit sound

  This is a clone of Mattel Football. Apparently Mattel tried to keep imports
  of infringing games from going through customs. Conic (Hong Kong) answered
  by distributing the game under subsidiary brands - see list below.

  known releases:
  - Hong Kong: Electronic Football, published by Conic
  - USA(1): Football, published by E.R.S.(Electronic Readout Systems)
  - USA(2): Football, published by ELECsonic
  - USA(3): Football, no brand!

  Another hardware revision of this game uses a PIC16 MCU.

***************************************************************************/

class cnfball_state : public hh_tms1k_state
{
public:
	cnfball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_ds8874(*this, "ds8874")
	{ }

	void cnfball(machine_config &config);

private:
	required_device<ds8874_device> m_ds8874;
	void ds8874_output_w(u16 data);

	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cnfball_state::update_display()
{
	m_display->matrix(~m_grid, m_o | (m_r << 6 & 0x700));
}

void cnfball_state::ds8874_output_w(u16 data)
{
	m_grid = data;
	update_display();
}

void cnfball_state::write_r(u16 data)
{
	// R5,R8: N/C
	// R6,R7: speaker out
	m_speaker->level_w(data >> 6 & 3);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0: DS8874N CP (note: it goes back to K8 too, game relies on it)
	// R1: DS8874N _DATA
	m_ds8874->cp_w(BIT(data, 0));
	m_ds8874->data_w(BIT(data, 1));

	// R2-R4: led data
	m_r = data;
	update_display();
}

void cnfball_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 cnfball_state::read_k()
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void cnfball_state::cnfball(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(cnfball_state::read_k));
	m_maincpu->r().set(FUNC(cnfball_state::write_r));
	m_maincpu->o().set(FUNC(cnfball_state::write_o));

	// video hardware
	DS8874(config, m_ds8874).write_output().set(FUNC(cnfball_state::ds8874_output_w));
	PWM_DISPLAY(config, m_display).set_size(9, 8+3);
	m_display->set_segmask(0xc3, 0x7f);
	m_display->set_segmask(0x38, 0xff); // only the middle 3 7segs have DP
	m_display->set_bri_levels(0.01, 0.1); // player led is brighter
	config.set_default_layout(layout_cnfball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cnfball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0170", 0x0000, 0x0400, CRC(50e8a44f) SHA1(fea6ae03c4ef329d825f8688e6854df15023d47e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_cnfball_output.pla", 0, 365, CRC(0af52f64) SHA1(b4cf450e4d895eddb67448aa69e4f18a5a84e033) )
ROM_END





/***************************************************************************

  Conic Electronic Football II
  * TMS1100 MP1181 (no decap)
  * 9-digit LED grid, 1-bit sound

  This is a clone of Coleco's Quarterback, similar at hardware-level too.
  Unlike the other LED Football games, this one looks like it doesn't make
  the offense(player) leds brighter.

  known releases:
  - Hong Kong: Electronic Football II, published by Conic
  - USA: Electronic Football II, published by Tandy

***************************************************************************/

class cnfball2_state : public hh_tms1k_state
{
public:
	cnfball2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void cnfball2(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cnfball2_state::update_display()
{
	// R1 selects between segments B/C or A'/D'
	u16 seg = m_o;
	if (~m_r & 2)
		seg = (m_o << 7 & 0x300) | (m_o & 0xf9);

	m_display->matrix(m_r >> 2 & 0x1ff, seg);
}

void cnfball2_state::write_r(u16 data)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R8-R10: input mux
	m_inp_mux = data >> 8 & 7;

	// R1-R10: select digit/segment
	m_r = data;
	update_display();
}

void cnfball2_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 cnfball2_state::read_k()
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

void cnfball2_state::cnfball2(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(cnfball2_state::read_k));
	m_maincpu->r().set(FUNC(cnfball2_state::write_r));
	m_maincpu->o().set(FUNC(cnfball2_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 11);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_cnfball2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cnfball2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1181", 0x0000, 0x0800, CRC(4553a840) SHA1(2e1132c9bc51641f77ba7f2430b5a3b2766b3a3d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_cnfball2_output.pla", 0, 365, NO_DUMP )

	ROM_REGION16_LE( 0x40, "maincpu:opla_b", ROMREGION_ERASE00 ) // verified, electronic dump, 2nd half unused
	ROM_LOAD16_BYTE( "tms1100_cnfball2_output.bin", 0, 0x20, CRC(c51d7404) SHA1(3a00c69b52b7d0dd12ccd66428130592c7e06240) )
ROM_END





/***************************************************************************

  Conic Electronic I.Q.
  * PCB labels: main: CONIC 101-037 (other side: HG-15, 11*00198*00), button PCB:
    CONIC 102-001, led PCB: CONIC 100-003 REV A itac
  * TMS1000NLL MP0908 (die label 1000B, MP0908)
  * 2 7seg LEDs, 30 other LEDs, 1-bit sound

  This is a peg solitaire game, with random start position.

  known releases:
  - Hong Kong: Electronic I.Q., published by Conic
  - UK: Solitaire, published by Grandstand

***************************************************************************/

class eleciq_state : public hh_tms1k_state
{
public:
	eleciq_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void eleciq(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void eleciq_state::update_display()
{
	m_display->matrix(m_r & ~1, m_o);
}

void eleciq_state::write_r(u16 data)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R1-R6,R9: input mux
	m_inp_mux = (data >> 1 & 0x3f) | (data >> 3 & 0x40);

	// R1-R6: led select
	// R7,R8: digit select
	m_r = data;
	update_display();
}

void eleciq_state::write_o(u16 data)
{
	// O0-O6: led/digit segment data
	// O7: N/C
	m_o = data;
	update_display();
}

u8 eleciq_state::read_k()
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, reset_button, 0)
INPUT_PORTS_END

void eleciq_state::eleciq(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=47K, C=50pF
	m_maincpu->k().set(FUNC(eleciq_state::read_k));
	m_maincpu->r().set(FUNC(eleciq_state::write_r));
	m_maincpu->o().set(FUNC(eleciq_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x180, 0x7f);
	config.set_default_layout(layout_eleciq);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( eleciq )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0908", 0x0000, 0x0400, CRC(db59b82c) SHA1(c9a6bcba208969560495ad9f8775f53de16a69c3) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_eleciq_output.pla", 0, 365, CRC(b8e04232) SHA1(22eed6d9b1fb1e5c9974ea3df16cda71a39aad57) )
ROM_END





/***************************************************************************

  Electroplay Quickfire
  * TMS1000NLL MP3260 (die label same)
  * 2 7seg LEDs, 5 lamps, 3 lightsensors, lightgun

  To play it in MAME, either use the clickable artwork with -mouse, or set
  button 1 to "Z or X or C" and each lightsensor to one of those keys.
  Although the game seems mostly playable without having to use the gun trigger

***************************************************************************/

class qfire_state : public hh_tms1k_state
{
public:
	qfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void qfire(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void qfire_state::write_r(u16 data)
{
	// R1,R2,R5: input mux
	m_inp_mux = (data >> 1 & 3) | (data >> 3 & 4);

	// R3,R4,R6-R8: leds (direct)
	m_display->write_row(2, (data >> 3 & 3) | (data >> 4 & 0x1c));

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);
}

void qfire_state::write_o(u16 data)
{
	// O0: 1st digit "1"
	// O1-O7: 2nd digit segments
	m_display->write_row(0, (data & 1) ? 6 : 0);
	m_display->write_row(1, data >> 1 & 0x7f);
}

u8 qfire_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( qfire )
	PORT_START("IN.0") // R1
	PORT_CONFNAME( 0x0f, 0x00, "Game" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x02, "4" )
	PORT_CONFSETTING(    0x01, "5" )
	PORT_CONFSETTING(    0x06, "6" )

	PORT_START("IN.1") // R2
	PORT_CONFNAME( 0x07, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "A" )
	PORT_CONFSETTING(    0x02, "B" )
	PORT_CONFSETTING(    0x01, "C" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Lightsensor 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Lightsensor 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Lightsensor 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // lightgun trigger, also turns on lightgun lamp
INPUT_PORTS_END

void qfire_state::qfire(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(qfire_state::read_k));
	m_maincpu->r().set(FUNC(qfire_state::write_r));
	m_maincpu->o().set(FUNC(qfire_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 7);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_qfire);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( qfire )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3260", 0x0000, 0x0400, CRC(f6e28376) SHA1(6129584c55a1629b458694cdc97edccb77ab00ba) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common3_micro.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_qfire_output.pla", 0, 365, CRC(8f7668a9) SHA1(c8faeff0f88bfea8f032ce5bc583f049e8930c11) )
ROM_END





/***************************************************************************

  Entex (Electronic) Soccer
  * TMS1000NL MP0158 (die label 1000B, MP0158)
  * 2 7seg LEDs, 30 other LEDs, 1-bit sound

  known releases:
  - USA: Electronic Soccer, 2 versions (leds on green bezel, or leds under bezel)
  - Germany: Fussball, with skill switch

***************************************************************************/

class esoccer_state : public hh_tms1k_state
{
public:
	esoccer_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void esoccer(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void esoccer_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void esoccer_state::write_r(u16 data)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R7: led select
	// R8,R9: digit select
	m_r = data;
	update_display();
}

void esoccer_state::write_o(u16 data)
{
	// O0-O6: led state
	m_o = data;
	update_display();
}

u8 esoccer_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( esoccer )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 475000); // approximation - RC osc. R=47K, C=33pF
	m_maincpu->k().set(FUNC(esoccer_state::read_k));
	m_maincpu->r().set(FUNC(esoccer_state::write_r));
	m_maincpu->o().set(FUNC(esoccer_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0x300, 0x7f);
	m_display->set_bri_levels(0.008, 0.08); // player led is brighter
	config.set_default_layout(layout_esoccer);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( esoccer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0158.ic1", 0x0000, 0x0400, CRC(ae4581ea) SHA1(5f6881f8247094abf8cffb17f6e6586e94cff38c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_esoccer_output.pla", 0, 365, CRC(c6eeabbd) SHA1(99d07902126b5a1c1abf43340f30d3390da5fa92) )
ROM_END





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
	ebball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ebball(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void ebball_state::update_display()
{
	m_display->matrix(m_r, ~m_o);
}

void ebball_state::write_r(u16 data)
{
	// R1-R5: input mux
	m_inp_mux = data >> 1 & 0x1f;

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0-R7: led select
	// R8: digit select
	m_r = data;
	update_display();
}

void ebball_state::write_o(u16 data)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 ebball_state::read_k()
{
	// K: multiplexed inputs (note: K8(Vss row) is always on)
	return m_inputs[5]->read() | read_inputs(5);
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 375000); // approximation - RC osc. R=43K, C=47pF
	m_maincpu->k().set(FUNC(ebball_state::read_k));
	m_maincpu->r().set(FUNC(ebball_state::write_r));
	m_maincpu->o().set(FUNC(ebball_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x100, 0x7f);
	config.set_default_layout(layout_ebball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ebball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0914", 0x0000, 0x0400, CRC(3c6fb05b) SHA1(b2fe4b3ca72d6b4c9bfa84d67f64afdc215e7178) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball_output.pla", 0, 365, CRC(062bf5bb) SHA1(8d73ee35444299595961225528b153e3a5fe66bf) )
ROM_END





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
	ebball2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ebball2(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void ebball2_state::update_display()
{
	m_display->matrix(m_r ^ 0x7f, ~m_o);
}

void ebball2_state::write_r(u16 data)
{
	// R3-R6: input mux
	m_inp_mux = data >> 3 & 0xf;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R2: digit select
	// R3-R9: led select
	m_r = data;
	update_display();
}

void ebball2_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 ebball2_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(ebball2_state::read_k));
	m_maincpu->r().set(FUNC(ebball2_state::write_r));
	m_maincpu->o().set(FUNC(ebball2_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_ebball2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ebball2 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0923", 0x0000, 0x0400, CRC(077acfe2) SHA1(a294ce7614b2cdb01c754a7a50d60d807e3f0939) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball2_output.pla", 0, 365, CRC(adcd73d1) SHA1(d69e590d288ef99293d86716498f3971528e30de) )
ROM_END





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
	ebball3_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ebball3(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void ebball3_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void ebball3_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=33pF) oscillating by default at ~340kHz,
	// but on PRO, the difficulty switch adds an extra 150K resistor to Vdd to speed
	// it up to around ~440kHz.
	m_maincpu->set_unscaled_clock((m_inputs[3]->read() & 1) ? 440000 : 340000);
}

void ebball3_state::update_display()
{
	m_display->matrix_partial(0, 10, m_r, m_o);

	// R0,R1 are normal 7segs
	// R4,R7 contain segments(only F and B) for the two other digits
	m_display->write_row(10, (m_display->read_row(4) & 0x20) | (m_display->read_row(7) & 0x02));
	m_display->write_row(11, ((m_display->read_row(4) & 0x10) | (m_display->read_row(7) & 0x01)) << 1);
}

void ebball3_state::write_r(u16 data)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0,R1, digit select
	// R2-R9: led select
	m_r = data;
	update_display();
}

void ebball3_state::write_o(u16 data)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 ebball3_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

/* physical button layout and labels are like this:

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
	PORT_CONFSETTING(    0x01, "1" ) // AUTO
	PORT_CONFSETTING(    0x00, "2" ) // MAN
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Bunt")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // fake
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, ebball3_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "1" ) // AM
	PORT_CONFSETTING(    0x01, "2" ) // PRO
INPUT_PORTS_END

void ebball3_state::ebball3(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 340000); // see set_clock
	m_maincpu->k().set(FUNC(ebball3_state::read_k));
	m_maincpu->r().set(FUNC(ebball3_state::write_r));
	m_maincpu->o().set(FUNC(ebball3_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10+2, 7);
	m_display->set_segmask(3, 0x7f);
	m_display->set_segmask(0xc00, 0x22);
	config.set_default_layout(layout_ebball3);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ebball3 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "6007_mp1204", 0x0000, 0x0800, CRC(987a29ba) SHA1(9481ae244152187d85349d1a08e439e798182938) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ebball3_output.pla", 0, 365, CRC(00db663b) SHA1(6eae12503364cfb1f863df0e57970d3e766ec165) )
ROM_END





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
	esbattle_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void esbattle(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void esbattle_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void esbattle_state::write_r(u16 data)
{
	// R0,R1: input mux
	m_inp_mux = data & 3;

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R7: led select
	// R8,R9: digit select
	m_r = data;
	update_display();
}

void esbattle_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 esbattle_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 450000); // approximation - RC osc. R=47K, C=33pF
	m_maincpu->k().set(FUNC(esbattle_state::read_k));
	m_maincpu->r().set(FUNC(esbattle_state::write_r));
	m_maincpu->o().set(FUNC(esbattle_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x300, 0x7f);
	config.set_default_layout(layout_esbattle);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( esbattle )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "en-6004_mp0920", 0x0000, 0x0400, CRC(7460c179) SHA1(be855054b4a98b05b34fd931d5c247c5c0f9b036) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_esbattle_output.pla", 0, 365, CRC(861b45a2) SHA1(a5a9dc9bef8adb761845ad548058b55e970517d3) )
ROM_END





/***************************************************************************

  Entex Blast It
  * TMS1000 MP0230 (die label 1000B, MP0230)
  * 3 7seg LEDs, 49 other LEDs (both under an overlay mask), 1-bit sound

***************************************************************************/

class blastit_state : public hh_tms1k_state
{
public:
	blastit_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void blastit(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void blastit_state::update_display()
{
	m_display->matrix(m_r >> 1, m_o);
}

void blastit_state::write_r(u16 data)
{
	// R3: input mux
	m_inp_mux = data >> 3 & 1;

	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R1-R7: led select
	// R8-R10: digit select
	m_r = data;
	update_display();
}

void blastit_state::write_o(u16 data)
{
	// O0-O6: led state
	m_o = data;
	update_display();
}

u8 blastit_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(1);
}

// config

static INPUT_PORTS_START( blastit )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x08, "1" ) // AM
	PORT_CONFSETTING(    0x00, "2" ) // PRO
INPUT_PORTS_END

void blastit_state::blastit(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 425000); // approximation - RC osc. R=47K, C=33pF
	m_maincpu->k().set(FUNC(blastit_state::read_k));
	m_maincpu->r().set(FUNC(blastit_state::write_r));
	m_maincpu->o().set(FUNC(blastit_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0x380, 0x7f);
	m_display->set_bri_levels(0.01, 0.115); // ball/paddle is slightly brighter
	config.set_default_layout(layout_blastit);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( blastit )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0230", 0x0000, 0x0400, CRC(1eb5f473) SHA1(76cd8c0e04368aa2150d428018643e1d0b9adda0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_blastit_output.pla", 0, 365, CRC(fa8081df) SHA1(99706d5ad58a76d47446576fac18964e602171c8) )
ROM_END





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
	einvader_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }
	void einvader(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
};

void einvader_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void einvader_state::set_clock()
{
	// MCU clock is from an RC circuit(R=47K, C=56pF) oscillating by default at ~320kHz,
	// but on PRO, the difficulty switch adds an extra 180K resistor to Vdd to speed
	// it up to around ~400kHz.
	m_maincpu->set_unscaled_clock((m_inputs[0]->read() & 8) ? 400000 : 320000);
}

void einvader_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void einvader_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R6: led select
	// R7-R9: digit select
	m_r = data;
	update_display();
}

void einvader_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

// config

static INPUT_PORTS_START( einvader )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, einvader_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "1" ) // amateur
	PORT_CONFSETTING(    0x08, "2" ) // professional
INPUT_PORTS_END

void einvader_state::einvader(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 320000); // see set_clock
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(einvader_state::write_r));
	m_maincpu->o().set(FUNC(einvader_state::write_o));

	// video hardware
	screen_device &mask(SCREEN(config, "mask", SCREEN_TYPE_SVG));
	mask.set_refresh_hz(60);
	mask.set_size(945, 1080);
	mask.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x380, 0x7f);
	m_display->set_bri_levels(0.01, 0.1); // ufo/player explosion is brighter
	config.set_default_layout(layout_einvader);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( einvader )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1211", 0x0000, 0x0800, CRC(b6efbe8e) SHA1(d7d54921dab22bb0c2956c896a5d5b56b6f64969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_einvader_output.pla", 0, 365, CRC(490158e1) SHA1(61cace1eb09244663de98d8fb04d9459b19668fd) )

	ROM_REGION( 45196, "mask", 0)
	ROM_LOAD( "einvader.svg", 0, 45196, CRC(35a1c744) SHA1(6beb9767454b9bc8f2ccf9fee25e7be209eefd22) )
ROM_END





/***************************************************************************

  Entex Color Football 4
  * TMS1670 6009 MP7551 (die label MP7551)
  * 9-digit cyan VFD display, 60 red and green LEDs behind mask, 1-bit sound

  Another version exist, one with a LED(red) 7seg display.

***************************************************************************/

class efootb4_state : public hh_tms1k_state
{
public:
	efootb4_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void efootb4(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void efootb4_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void efootb4_state::write_r(u16 data)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R0-R9: led select
	// R10-R15: digit select
	m_r = data;
	update_display();
}

void efootb4_state::write_o(u16 data)
{
	// O7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// O0-O6: led state
	m_o = data;
	update_display();
}

u8 efootb4_state::read_k()
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
	PORT_CONFSETTING(    0x00, "1" ) // amateur
	PORT_CONFSETTING(    0x02, "2" ) // professional
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Status")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void efootb4_state::efootb4(machine_config &config)
{
	// basic machine hardware
	TMS1670(config, m_maincpu, 400000); // approximation - RC osc. R=42K, C=47pF
	m_maincpu->k().set(FUNC(efootb4_state::read_k));
	m_maincpu->r().set(FUNC(efootb4_state::write_r));
	m_maincpu->o().set(FUNC(efootb4_state::write_o));

	// video hardware
	screen_device &mask(SCREEN(config, "mask", SCREEN_TYPE_SVG));
	mask.set_refresh_hz(60);
	mask.set_size(1920, 904);
	mask.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(16, 7);
	m_display->set_segmask(0xfc00, 0x7f);
	config.set_default_layout(layout_efootb4);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( efootb4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "6009_mp7551", 0x0000, 0x1000, CRC(54fa7244) SHA1(4d16bd825c4a2db76ca8a263c373ade15c20e270) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_efootb4_output.pla", 0, 557, CRC(5c87c753) SHA1(bde9d4aa1e57a718affd969475c0a1edcf60f444) )

	ROM_REGION( 67472, "mask", 0)
	ROM_LOAD( "efootb4.svg", 0, 67472, CRC(ba0abcda) SHA1(6066e4cbae5404e4db17fae0153808b044adc823) )
ROM_END





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
	ebaskb2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ebaskb2(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void ebaskb2_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void ebaskb2_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R6-R9: input mux
	m_inp_mux = data >> 6 & 0xf;

	// R0-R3: digit select
	// R4-R9: led select
	m_r = data;
	update_display();
}

void ebaskb2_state::write_o(u16 data)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data;
	update_display();
}

u8 ebaskb2_state::read_k()
{
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( ebaskb2 )
	PORT_START("IN.0") // R6
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" ) // amateur
	PORT_CONFSETTING(    0x00, "2" ) // professional
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 360000); // approximation - RC osc. R=33K, C=82pF
	m_maincpu->k().set(FUNC(ebaskb2_state::read_k));
	m_maincpu->r().set(FUNC(ebaskb2_state::write_r));
	m_maincpu->o().set(FUNC(ebaskb2_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0xf, 0x7f);
	m_display->set_bri_levels(0.01, 0.1); // ball carrier led is brighter
	config.set_default_layout(layout_ebaskb2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ebaskb2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "6010_mp1218", 0x0000, 0x0800, CRC(0089ede8) SHA1(c8a79d5aca7e37b637a4d152150acba9f41aad96) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ebaskb2_output.pla", 0, 365, CRC(c18103ae) SHA1(5a9bb8e1d95a9f6919b05ff9471fa0a8014b8b81) )
ROM_END





/***************************************************************************

  Entex Raise The Devil
  * TMS1100 MP1221 (die label same)
  * 4 7seg LEDs(rightmost one unused), and other LEDs behind bezel, 1-bit sound

  Entex Black Knight (licensed handheld version of Williams' pinball game)
  * TMS1100 MP1296 (no decap)
  * same hardware as Raise The Devil

  raisedvl led translation table: led zz from game PCB = MAME y.x:
  (no led labels on ebknight PCB)

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
	raisedvl_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void raisedvl(machine_config &config);
	void ebknight(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void raisedvl_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void raisedvl_state::set_clock()
{
	// MCU clock is from an RC circuit with C=47pF, R=47K by default. Skills
	// 2 and 3 add a 150K resistor in parallel, and skill 4 adds a 100K one.
	// 0:   R=47K  -> ~350kHz
	// 2,3: R=35K8 -> ~425kHz (combined)
	// 4:   R=32K  -> ~465kHz (combined)
	u8 inp = m_inputs[1]->read();
	m_maincpu->set_unscaled_clock((inp & 0x20) ? 465000 : ((inp & 0x10) ? 425000 : 350000));
}

void raisedvl_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void raisedvl_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0,R1: input mux
	m_inp_mux = data & 3;

	// R0-R2: digit select
	// R3-R9: led select
	m_r = data;
	update_display();
}

void raisedvl_state::write_o(u16 data)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 raisedvl_state::read_k()
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
	PORT_CONFNAME( 0x31, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, raisedvl_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x10, "2" )
	PORT_CONFSETTING(    0x11, "3" )
	PORT_CONFSETTING(    0x21, "4" )
INPUT_PORTS_END

void raisedvl_state::raisedvl(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // see set_clock
	m_maincpu->k().set(FUNC(raisedvl_state::read_k));
	m_maincpu->r().set(FUNC(raisedvl_state::write_r));
	m_maincpu->o().set(FUNC(raisedvl_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_raisedvl);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void raisedvl_state::ebknight(machine_config &config)
{
	raisedvl(config);
	config.set_default_layout(layout_ebknight);
}

// roms

ROM_START( raisedvl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1221", 0x0000, 0x0800, CRC(782791cc) SHA1(214249406fcaf44efc6350022bd534e59ec69c88) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_raisedvl_output.pla", 0, 365, CRC(00db663b) SHA1(6eae12503364cfb1f863df0e57970d3e766ec165) )
ROM_END

ROM_START( ebknight )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1296", 0x0000, 0x0800, CRC(bc57a46a) SHA1(f60843779f49a8bd28291df3390086e54b9e3f40) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified, taken from raisedvl
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ebknight_output.pla", 0, 365, BAD_DUMP CRC(00db663b) SHA1(6eae12503364cfb1f863df0e57970d3e766ec165) ) // "
ROM_END





/***************************************************************************

  Fonas 2 Player Baseball
  * PCB label CA-014 (probably Cassia)
  * TMS1000NLL MP0154 (die label 1000B, MP0154)
  * 4 7seg LEDs, 37 other LEDs, 1-bit sound

  known releases:
  - World: 2 Player Baseball, published by Fonas
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
	f2pbball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void f2pbball(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void f2pbball_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void f2pbball_state::write_r(u16 data)
{
	// R4,R9,R10: input mux
	m_inp_mux = (data >> 4 & 1) | (data >> 8 & 6);

	// R9,R10(ANDed together): speaker out
	m_speaker->level_w(data >> 10 & data >> 9 & 1);

	// R0-R4: led select
	// R5-R8: digit select
	m_r = data;
	update_display();
}

void f2pbball_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = bitswap<8>(data,0,7,6,5,4,3,2,1);
	update_display();
}

u8 f2pbball_state::read_k()
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, reset_button, 0)
INPUT_PORTS_END

void f2pbball_state::f2pbball(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 325000); // approximation - RC osc. R=51K, C=39pF
	m_maincpu->k().set(FUNC(f2pbball_state::read_k));
	m_maincpu->r().set(FUNC(f2pbball_state::write_r));
	m_maincpu->o().set(FUNC(f2pbball_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1e0, 0x7f);
	config.set_default_layout(layout_f2pbball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( f2pbball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0154", 0x0000, 0x0400, CRC(c5b45ace) SHA1(b2de32e83ab447b22d6828f0081843f364040b01) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_f2pbball_output.pla", 0, 365, CRC(30c2f28f) SHA1(db969b22475f37f083c3594f5e4f5759048377b8) )
ROM_END





/***************************************************************************

  Fonas 3 in 1: Football, Basketball, Soccer
  * PCB label HP-801
  * TMS1100NLL MP1185
  * 4 7seg LEDs, 40 other LEDs, 1-bit sound

  It's not known if this game has an official title. The current one is
  taken from the handheld front side.
  MAME external artwork is needed for the switchable overlays.

***************************************************************************/

class f3in1_state : public hh_tms1k_state
{
public:
	f3in1_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void f3in1(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void f3in1_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void f3in1_state::set_clock()
{
	// MCU clock is from an RC circuit where C=47pF, R=39K(PROF) or 56K(REG)
	m_maincpu->set_unscaled_clock((m_inputs[4]->read() & 1) ? 400000 : 300000);
}

void f3in1_state::update_display()
{
	m_display->matrix(m_r & ~0x20, m_o);
}

void f3in1_state::write_r(u16 data)
{
	// R0-R2,R4: input mux
	m_inp_mux = (data & 7) | (data >> 1 & 8);

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R4: led select
	// R6-R9: digit select
	m_r = data;
	update_display();
}

void f3in1_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 f3in1_state::read_k()
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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, f3in1_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "1" ) // REG
	PORT_CONFSETTING(    0x01, "2" ) // PROF
INPUT_PORTS_END

void f3in1_state::f3in1(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 300000); // see set_clock
	m_maincpu->k().set(FUNC(f3in1_state::read_k));
	m_maincpu->r().set(FUNC(f3in1_state::write_r));
	m_maincpu->o().set(FUNC(f3in1_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3c0, 0x7f);
	m_display->set_bri_levels(0.003, 0.05); // player led is brighter
	config.set_default_layout(layout_f3in1);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( f3in1 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1185", 0x0000, 0x0800, CRC(53f7b28d) SHA1(2249890e3a259095193b4331ca88c29ccd81eefe) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_f3in1_output.pla", 0, 365, CRC(51d947bc) SHA1(f766397d84f038be96e83d40989195c98ddcb1d9) )
ROM_END





/***************************************************************************

  Gakken Poker
  * PCB label POKER. gakken
  * TMS1370 MP2105 (die label same)
  * 11-digit cyan VFD display Itron FG1114B, oscillator sound

  known releases:
  - Japan: Poker, published by Gakken
  - USA: Electronic Poker, published by Entex

***************************************************************************/

class gpoker_state : public hh_tms1k_state
{
public:
	gpoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_beeper(*this, "beeper")
	{ }

	void gpoker(machine_config &config);

protected:
	required_device<beep_device> m_beeper;

	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void gpoker_state::update_display()
{
	u16 segs = bitswap<16>(m_o, 15,14,7,12,11,10,9,8,6,6,5,4,3,2,1,0) & 0x20ff;
	m_display->matrix(m_r & 0x7ff, segs | (m_r >> 3 & 0xf00));
}

void gpoker_state::write_r(u16 data)
{
	// R15: enable beeper
	m_beeper->set_state(data >> 15 & 1);

	// R0-R6: input mux
	m_inp_mux = data & 0x7f;

	// R0-R10: select digit
	// R11-R14: card symbols
	m_r = data;
	update_display();
}

void gpoker_state::write_o(u16 data)
{
	// O0-O7: digit segments A-G,H
	m_o = data;
	update_display();
}

u8 gpoker_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(7);
}

// config

/* physical button layout and labels are like this:

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
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // 9/DL
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

	PORT_START("FAKE") // 9/DL are electronically the same button
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Deal") // DL
INPUT_PORTS_END

void gpoker_state::gpoker(machine_config &config)
{
	// basic machine hardware
	TMS1370(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(gpoker_state::read_k));
	m_maincpu->r().set(FUNC(gpoker_state::write_r));
	m_maincpu->o().set(FUNC(gpoker_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 14);
	m_display->set_segmask(0x7ff, 0x20ff); // 7seg + bottom-right diagonal
	config.set_default_layout(layout_gpoker);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 2400); // astable multivibrator - C1 and C2 are 0.003uF, R1 and R4 are 1K, R2 and R3 are 100K
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gpoker )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2105", 0x0000, 0x0800, CRC(95a8f5b4) SHA1(d14f00ba9f57e437264d972baa14a14a28ff8719) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_gpoker_output.pla", 0, 365, CRC(f7e2d812) SHA1(cc3abd89afb1d2145dc47636553ccd0ba7de70d9) )
ROM_END





/***************************************************************************

  Gakken Jackpot: Gin Rummy & Black Jack
  * PCB label gakken
  * TMS1670 MPF553 (die label same)
  * 11-digit cyan VFD display Itron FG1114B, oscillator sound

  known releases:
  - Japan: Jackpot(?), published by Gakken
  - USA: Electronic Jackpot: Gin Rummy & Black Jack, published by Entex

***************************************************************************/

class gjackpot_state : public gpoker_state
{
public:
	gjackpot_state(const machine_config &mconfig, device_type type, const char *tag) :
		gpoker_state(mconfig, type, tag)
	{ }

	void gjackpot(machine_config &config);

private:
	virtual void write_r(u16 data) override;
};

// handlers

void gjackpot_state::write_r(u16 data)
{
	// same as gpoker, only input mux msb is R10 instead of R6
	gpoker_state::write_r(data);
	m_inp_mux = (data & 0x3f) | (data >> 4 & 0x40);
}

// config

/* physical button layout and labels are like this:
  (note: on dual-function buttons, upper label=Gin, lower label=Black Jack)

                       BJ --o GIN
                          OFF
    [7]    [8]    [9]    [DL]

    [4]    [5]    [6]    [      [KN
                          DB]    SP]
    [1]    [2]    [3]    [DS]   [DR]
                          BT]    HT]
    [10/0] [T]    [MD    [CH    [AC]
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
	gpoker(config);

	// basic machine hardware
	TMS1670(config.replace(), m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(gjackpot_state::read_k));
	m_maincpu->r().set(FUNC(gjackpot_state::write_r));
	m_maincpu->o().set(FUNC(gjackpot_state::write_o));

	config.set_default_layout(layout_gjackpot);
}

// roms

ROM_START( gjackpot )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mpf553", 0x0000, 0x1000, CRC(f45fd008) SHA1(8d5d6407a8a031a833ceedfb931f5c9d2725ecd0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_gjackpot_output.pla", 0, 557, CRC(50e471a7) SHA1(9d862cb9f51a563882b62662c5bfe61b52e3df00) )
ROM_END





/***************************************************************************

  Gakken Invader
  * PCB label GAKKEN, INVADER, KS-00779
  * TMS1370 MP2110
  * cyan VFD display Itron? CP5008A, 1-bit sound

  known releases:
  - World: Invader, published by Gakken
  - USA(1): Galaxy Invader, published by CGL
  - USA(2): Fire Away, published by Tandy
  - USA(3): Electron Blaster, published by Vanity Fair

  On the real thing, the joystick is "sticky"(it doesn't autocenter when you let go).
  There's also a version with a cyan/red VFD, possibly the same ROM.

***************************************************************************/

class ginv_state : public hh_tms1k_state
{
public:
	ginv_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ginv(machine_config &config);

private:
	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void ginv_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void ginv_state::write_r(u16 data)
{
	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R15: speaker out
	m_speaker->level_w(data >> 15 & 1);

	// R0-R8: VFD grid
	// R11-R14: VFD plate
	m_grid = data & 0x1ff;
	m_plate = (m_plate & 0xff) | (data >> 3 & 0xf00);
	update_display();
}

void ginv_state::write_o(u16 data)
{
	// O0-O7: VFD plate
	m_plate = (m_plate & ~0xff) | data;
	update_display();
}

u8 ginv_state::read_k()
{
	// K1-K4: multiplexed inputs (K8 is fire button)
	return m_inputs[2]->read() | read_inputs(2);
}

// config

static INPUT_PORTS_START( ginv )
	PORT_START("IN.0") // R9
	PORT_CONFNAME( 0x07, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, 0x02, IPT_CUSTOM ) PORT_CONDITION("IN.1", 0x05, EQUALS, 0x00) // joystick centered
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // K8
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
INPUT_PORTS_END

void ginv_state::ginv(machine_config &config)
{
	// basic machine hardware
	TMS1370(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(ginv_state::read_k));
	m_maincpu->r().set(FUNC(ginv_state::write_r));
	m_maincpu->o().set(FUNC(ginv_state::write_o));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(236, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 12);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ginv )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2110", 0x0000, 0x0800, CRC(f09c5588) SHA1(06eb8ed512eaf5367ea30c2b633219e105ddfd14) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ginv_output.pla", 0, 365, CRC(6e33a24e) SHA1(cdf7ecf12ddd3863e6301e20fe80f9737db429e5) )

	ROM_REGION( 142959, "screen", 0)
	ROM_LOAD( "ginv.svg", 0, 142959, CRC(b0dc9bac) SHA1(18f8cc51a432d14f08fdf766275222f3ed184d89) )
ROM_END





/***************************************************************************

  Gakken Invader 1000
  * TMS1370 MP2139 (die label 1170 MP2139)
  * cyan/red VFD display Futaba DM-25Z 2D, 1-bit sound

  known releases:
  - World: Galaxy Invader 1000, published by Gakken
  - Japan: Invader 1000, published by Gakken
  - USA(1): Galaxy Invader 1000, published by CGL
  - USA(2): Cosmic 1000 Fire Away, published by Tandy

***************************************************************************/

class ginv1000_state : public hh_tms1k_state
{
public:
	ginv1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ginv1000(machine_config &config);

private:
	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void ginv1000_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,9,8,7);
	u16 plate = bitswap<16>(m_plate,15,14,13,12,3,4,7,8,9,10,11,2,6,5,1,0);
	m_display->matrix(grid, plate);
}

void ginv1000_state::write_r(u16 data)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R8,R15: input mux
	m_inp_mux = (data >> 8 & 1) | (data >> 14 & 2);

	// R1-R10: VFD grid
	// R11-R14: VFD plate
	m_grid = data >> 1 & 0x3ff;
	m_plate = (m_plate & 0xff) | (data >> 3 & 0xf00);
	update_display();
}

void ginv1000_state::write_o(u16 data)
{
	// O0-O7: VFD plate
	m_plate = (m_plate & ~0xff) | data;
	update_display();
}

u8 ginv1000_state::read_k()
{
	// K1,K2: multiplexed inputs (K8 is fire button)
	return m_inputs[2]->read() | read_inputs(2);
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
	// basic machine hardware
	TMS1370(config, m_maincpu, 350000); // approximation
	m_maincpu->k().set(FUNC(ginv1000_state::read_k));
	m_maincpu->r().set(FUNC(ginv1000_state::write_r));
	m_maincpu->o().set(FUNC(ginv1000_state::write_o));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(226, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 12);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ginv1000 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2139", 0x0000, 0x0800, CRC(036eab37) SHA1(0795878ad89296f7a6a0314c6e4db23c1cc3673e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ginv1000_output.pla", 0, 365, CRC(b0a5dc41) SHA1(d94746ec48661998173e7f60ccc7c96e56b3484e) )

	ROM_REGION( 227224, "screen", 0)
	ROM_LOAD( "ginv1000.svg", 0, 227224, CRC(f220711a) SHA1(729ad85fb9d9853a77c45b5ed072f10ede7649c4) )
ROM_END





/***************************************************************************

  Gakken Invader 2000
  * TMS1370(28 pins) MP1604 (die label 1370A MP1604)
  * TMS1024 I/O expander
  * cyan/red/green VFD display, 1-bit sound

  known releases:
  - World: Invader 2000, published by Gakken
  - USA(1): Galaxy Invader 10000, published by CGL
  - USA(2): Cosmic 3000 Fire Away, published by Tandy

***************************************************************************/

class ginv2000_state : public hh_tms1k_state
{
public:
	ginv2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_expander(*this, "expander")
	{ }

	void ginv2000(machine_config &config);

private:
	required_device<tms1024_device> m_expander;
	void expander_w(offs_t offset, u8 data);

	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void ginv2000_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void ginv2000_state::expander_w(offs_t offset, u8 data)
{
	// TMS1024 port 4-7: VFD plate
	int shift = (offset - tms1024_device::PORT4) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void ginv2000_state::write_r(u16 data)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R11,R12: input mux
	m_inp_mux = data >> 11 & 3;

	// R11,R12: TMS1024 S1,S0 (S2 forced high)
	// R13: TMS1024 STD
	m_expander->write_s((data >> 12 & 1) | (data >> 10 & 2) | 4);
	m_expander->write_std(data >> 13 & 1);

	// R1-R10: VFD grid
	m_grid = data >> 1 & 0x3ff;
	update_display();
}

void ginv2000_state::write_o(u16 data)
{
	// O4-O7: TMS1024 H1-H4
	m_expander->write_h(data >> 4 & 0xf);
}

u8 ginv2000_state::read_k()
{
	// K1,K2: multiplexed inputs (K8 is fire button)
	return m_inputs[2]->read() | read_inputs(2);
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

void ginv2000_state::ginv2000(machine_config &config)
{
	// basic machine hardware
	TMS1370(config, m_maincpu, 425000); // approximation - RC osc. R=36K, C=47pF
	m_maincpu->k().set(FUNC(ginv2000_state::read_k));
	m_maincpu->r().set(FUNC(ginv2000_state::write_r));
	m_maincpu->o().set(FUNC(ginv2000_state::write_o));

	TMS1024(config, m_expander).set_ms(1); // MS tied high
	m_expander->write_port4_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port5_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port6_callback().set(FUNC(ginv2000_state::expander_w));
	m_expander->write_port7_callback().set(FUNC(ginv2000_state::expander_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(364, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ginv2000 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1604", 0x0000, 0x0800, CRC(f1646d0b) SHA1(65601931d81e3eef7bf22a08de5a146910ce8137) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_ginv2000_output.pla", 0, 365, CRC(520bb003) SHA1(1640ae54f8dcc257e0ad0cbe0281b38fcbd8da35) )

	ROM_REGION( 374443, "screen", 0)
	ROM_LOAD( "ginv2000.svg", 0, 374443, CRC(a4ce1e6d) SHA1(57d9ff05d634a8d495b9d544a2a959790cd10b6b) )
ROM_END





/***************************************************************************

  Gakken FX-Micom R-165
  * TMS1100 MCU, label MP1312 (die label MP1312A)
  * 1 7seg led, 6 other leds, 1-bit sound

  This is a simple educational home computer. Refer to the extensive manual
  for more information. It was published later in the USA by Tandy(Radio Shack),
  under their Science Fair series. Another 25 years later, Gakken re-released
  the R-165 as GMC-4, obviously on modern hardware, but fully compatible.

  known releases:
  - Japan: FX-Micom R-165, published by Gakken
  - USA: Science Fair Microcomputer Trainer, published by Tandy. Of note is
    the complete redesign of the case, adding more adjustable wiring

***************************************************************************/

class fxmcr165_state : public hh_tms1k_state
{
public:
	fxmcr165_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void fxmcr165(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void fxmcr165_state::update_display()
{
	// 7seg digit from O0-O6
	m_display->write_row(0, bitswap<8>(m_o,7,2,6,5,4,3,1,0) & 0x7f);

	// leds from R4-R10
	m_display->write_row(1, m_r >> 4 & 0x7f);
}

void fxmcr165_state::write_r(u16 data)
{
	// R0-R3: input mux low
	m_inp_mux = (m_inp_mux & 0x10) | (data & 0xf);

	// R7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// R4-R10: led data (direct)
	m_r = data;
	update_display();
}

void fxmcr165_state::write_o(u16 data)
{
	// O7: input mux high
	m_inp_mux = (m_inp_mux & 0xf) | (data >> 3 & 0x10);

	// O0-O6: digit segments (direct)
	m_o = data;
	update_display();
}

u8 fxmcr165_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

/* physical button layout and labels are like this:

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
	// basic machine hardware
	TMS1100(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->k().set(FUNC(fxmcr165_state::read_k));
	m_maincpu->r().set(FUNC(fxmcr165_state::write_r));
	m_maincpu->o().set(FUNC(fxmcr165_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1+1, 7);
	m_display->set_segmask(1, 0x7f);
	config.set_default_layout(layout_fxmcr165);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( fxmcr165 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1312", 0x0000, 0x0800, CRC(6efc8bcc) SHA1(ced8a02b472a3178073691d3dccc0f19f57428fd) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_fxmcr165_output.pla", 0, 365, CRC(ce656866) SHA1(40e1614f5afcc7572fda596e1be453d54e95af0c) )
ROM_END





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
	elecdet_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void elecdet(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void elecdet_state::write_r(u16 data)
{
	// R7,R8: speaker out
	m_speaker->level_w((m_o & 0x80) ? (data >> 7 & 3) : 0);

	// R0-R6: select digit
	m_display->matrix(data, bitswap<8>(m_o,7,5,2,1,4,0,6,3));
}

void elecdet_state::write_o(u16 data)
{
	// O0,O1,O4,O6: input mux
	m_inp_mux = (data & 3) | (data >> 2 & 4) | (data >> 3 & 8);

	// O0-O6: digit segments A-G
	// O7: speaker on -> write_r
	m_o = data;
}

u8 elecdet_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[4]->read() | read_inputs(4);
}

// config

/* physical button layout and labels are like this:

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

void elecdet_state::elecdet(machine_config &config)
{
	// basic machine hardware
	TMS0980(config, m_maincpu, 425000); // approximation
	m_maincpu->k().set(FUNC(elecdet_state::read_k));
	m_maincpu->r().set(FUNC(elecdet_state::write_r));
	m_maincpu->o().set(FUNC(elecdet_state::write_o));
	m_maincpu->power_off().set(FUNC(elecdet_state::auto_power_off));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7, 7);
	m_display->set_segmask(0x7f, 0x7f);
	config.set_default_layout(layout_elecdet);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 0.5, 0.5, 1.0};
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

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





/***************************************************************************

  Kenner Star Wars - Electronic Battle Command
  * TMS1100 MCU, label MP3438A (die label 1100B, MP3438A)
  * 4x4 LED grid display + 2 separate LEDs and 2-digit 7segs, 1-bit sound

  This is a small tabletop space-dogfighting game. To start the game,
  press BASIC/INTER/ADV and enter P#(number of players), then
  START TURN. Refer to the official manual for more information.

***************************************************************************/

class starwbc_state : public hh_tms1k_state
{
public:
	starwbc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void starwbc(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void starwbc_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void starwbc_state::write_r(u16 data)
{
	// R0,R1,R3,R5,R7: input mux
	m_inp_mux = (data & 3) | (data >> 1 & 4) | (data >> 2 & 8) | (data >> 3 & 0x10);

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0,R2,R4: led select
	// R6,R8: digit select
	m_r = data & 0x155;
	update_display();
}

void starwbc_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = (data << 4 & 0xf0) | (data >> 4 & 0x0f);
	update_display();
}

u8 starwbc_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

/* physical button layout and labels are like this:

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
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=51K, C=47pF
	m_maincpu->k().set(FUNC(starwbc_state::read_k));
	m_maincpu->r().set(FUNC(starwbc_state::write_r));
	m_maincpu->o().set(FUNC(starwbc_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x140, 0x7f);
	config.set_default_layout(layout_starwbc);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

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





/***************************************************************************

  Kenner Live Action Football
  * TMS1100NLL MCU, label MP3489-N2 (die label 1100E, MP3489)
  * 6-digit 7seg LED display, other LEDs under overlay, 1-bit sound

  The LEDs are inside reflective domes, with an overlay mask on top of that.
  It is done with an SVG screen on MAME. In reality, the display is not as
  sharp or as evenly lit as MAME suggests it to be.

  It has a 1-bit roller controller. Half of the axis connects to the input
  (eg. 1 rising edge per full rotation), so there's no difference between
  rotating left or right.

***************************************************************************/

class liveafb_state : public hh_tms1k_state
{
public:
	liveafb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void liveafb(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void liveafb_state::update_display()
{
	u8 d = (~m_r & 0x100) ? (m_r & 0x3f) : 0; // digit select
	u8 l = (m_r & 0x100) ? (m_r & 0xf) : 0; // led select
	m_display->matrix(d | l << 6 | BIT(m_r, 6) << 10 | BIT(m_r, 8, 3) << 11, m_o | (m_r << 4 & 0x300));
}

void liveafb_state::write_r(u16 data)
{
	// R0-R3: input mux
	m_inp_mux = data & 0xf;

	// R7(+R8): speaker out
	m_speaker->level_w(BIT(data, 7) & BIT(data, 8));

	// R8: enable digit or led select
	// R0-R3: led select
	// R0-R5: digit select
	// R4,R5: led data high
	// R6,R8-R10: direct leds
	m_r = data & ~0x80;
	update_display();
}

void liveafb_state::write_o(u16 data)
{
	// O0-O7: led data low
	m_o = data;
	update_display();
}

u8 liveafb_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( liveafb )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("ROLLER", 0x7f, LESSTHAN, 0x40)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Tackle")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Punt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Field Goal")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Skill / Score")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Action")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("ROLLER")
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END

void liveafb_state::liveafb(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(liveafb_state::read_k));
	m_maincpu->r().set(FUNC(liveafb_state::write_r));
	m_maincpu->o().set(FUNC(liveafb_state::write_o));

	// video hardware
	screen_device &mask(SCREEN(config, "mask", SCREEN_TYPE_SVG));
	mask.set_refresh_hz(60);
	mask.set_size(1834, 1080);
	mask.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(6+4+4, 8+2);
	m_display->set_segmask(0x3f, 0x7f);
	m_display->set_segmask(0x20, 0xff); // only one digit has DP
	config.set_default_layout(layout_liveafb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( liveafb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3489", 0x0000, 0x0800, CRC(1fe05ab3) SHA1(a8d7dfed61a6397b7af1d3fcf17b26d5d917b4f0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_liveafb_output.pla", 0, 365, CRC(a7bc9384) SHA1(fab458de394eeddbf5ba0830853a915e51f909c6) )

	ROM_REGION( 162058, "mask", 0)
	ROM_LOAD( "liveafb.svg", 0, 162058, CRC(046078d0) SHA1(68a5775f4f9a1258c06b76839e1cfdab69b61920) )
ROM_END





/***************************************************************************

  Kosmos Astro
  * TMS1470NLHL MP1133 (die label TMS1400 MP1133)
  * 9-digit 7seg VFD display + 8 LEDs(4 green, 4 yellow), no sound

  This is an astrological calculator, and also supports 4-function
  calculations. Refer to the official manual on how to use this device.

***************************************************************************/

class astro_state : public hh_tms1k_state
{
public:
	astro_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void astro(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void astro_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void astro_state::write_r(u16 data)
{
	// R0-R7: input mux
	m_inp_mux = data & 0xff;

	// R0-R9: led select
	m_r = data;
	update_display();
}

void astro_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 astro_state::read_k()
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
	// basic machine hardware
	TMS1470(config, m_maincpu, 450000); // approximation - RC osc. R=4.7K, C=33pF
	m_maincpu->k().set(FUNC(astro_state::read_k));
	m_maincpu->r().set(FUNC(astro_state::write_r));
	m_maincpu->o().set(FUNC(astro_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3ff, 0xff);
	config.set_default_layout(layout_astro);

	// no sound!
}

// roms

ROM_START( astro )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp1133", 0x0000, 0x1000, CRC(bc21109c) SHA1(05a433cce587d5c0c2d28b5fda5f0853ea6726bf) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_astro_output.pla", 0, 557, CRC(eb08957e) SHA1(62ae0d13a1eaafb34f1b27d7df51441b400ccd56) )
ROM_END





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
	elecbowl_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void elecbowl(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void elecbowl_state::update_display()
{
	// standard 7segs
	m_display->matrix_partial(0, 4, m_r >> 4, m_o);

	// lamp muxes
	u8 sel = m_o & 7;
	u8 state = m_r >> 1 & 1;
	if (~m_r & 1)
		m_display->write_element(5, sel, state);
	if (~m_r & 4)
		m_display->write_element(6, sel, state);

	// digit 4 is from mux2 Q7
	m_display->write_row(4, m_display->read_element(6, 7) ? 6 : 0);
}

void elecbowl_state::write_r(u16 data)
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
	update_display();
}

void elecbowl_state::write_o(u16 data)
{
	//if (data & 0x80) printf("%X ",data&0x7f);

	// O0-O2: lamp muxes select
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 elecbowl_state::read_k()
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

// output PLA is not decapped, this was made by hand
static const u16 elecbowl_output_pla[0x20] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, // 0-9
	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, // ?
	0, 1, 2, 3, 4, 5, 6, 7, // lamp muxes select
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f // ?
};

void elecbowl_state::elecbowl(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->set_output_pla(elecbowl_output_pla);
	m_maincpu->k().set(FUNC(elecbowl_state::read_k));
	m_maincpu->r().set(FUNC(elecbowl_state::write_r));
	m_maincpu->o().set(FUNC(elecbowl_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7, 8);
	m_display->set_segmask(0xf, 0x7f);
	m_display->set_segmask(0x10, 0x06); // 1
	config.set_default_layout(layout_elecbowl);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( elecbowl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3403.u9", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, BAD_DUMP CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_elecbowl_output.pla", 0, 365, NO_DUMP )
ROM_END





/***************************************************************************

  Mattel Thoroughbred Horse Race Analyzer
  * PCB label 1670-4619D
  * TMS1100NLL MP3491-N2 (die label 1100E MP3491)
  * HLCD0569, 67-segment LCD panel, no sound

  This handheld is not a toy, read the manual for more information. In short,
  it is a device for predicting the winning chance of a gambling horserace.

  It was rereleased in 1994 in China/Canada by AHTI(Advanced Handicapping
  Technologies, Inc.), on a Hitachi HD613901.

***************************************************************************/

class horseran_state : public hh_tms1k_state
{
public:
	horseran_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_lcd(*this, "lcd")
	{ }

	void horseran(machine_config &config);

private:
	required_device<hlcd0569_device> m_lcd;

	void lcd_output_w(offs_t offset, u32 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void horseran_state::lcd_output_w(offs_t offset, u32 data)
{
	// only 3 rows used
	if (offset > 2)
		return;

	// update lcd segments
	m_display->matrix_partial(0, 3, 1 << offset, data);

	// col5-11 and col13-19 are 7segs
	for (int i = 0; i < 2; i++)
		m_display->write_row(3 + (offset << 1 | i), bitswap<8>(data >> (4+8*i),7,3,5,2,0,1,4,6) & 0x7f);
}

void horseran_state::write_r(u16 data)
{
	// R0: HLCD0569 clock
	// R1: HLCD0569 data in
	// R2: HLCD0569 _CS
	m_lcd->cs_w(data >> 2 & 1);
	m_lcd->data_w(data >> 1 & 1);
	m_lcd->clock_w(data & 1);

	// R3-R10: input mux
	m_inp_mux = data >> 3 & 0xff;
}

u8 horseran_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(8);
}

// config

/* physical button layout and labels are like this:

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
	// basic machine hardware
	TMS1100(config, m_maincpu, 300000); // approximation - RC osc. R=56K, C=47pF
	m_maincpu->k().set(FUNC(horseran_state::read_k));
	m_maincpu->r().set(FUNC(horseran_state::write_r));

	// video hardware
	HLCD0569(config, m_lcd, 1100); // C=0.022uF
	m_lcd->write_cols().set(FUNC(horseran_state::lcd_output_w));

	PWM_DISPLAY(config, m_display).set_size(3+6, 24);
	m_display->set_segmask(0x3f<<3, 0x7f);
	config.set_default_layout(layout_horseran);

	// no sound!
}

// roms

ROM_START( horseran )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3491", 0x0000, 0x0800, CRC(a0081671) SHA1(a5a07b502c69d429e5bcd1d313e86b6ee057cda6) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1100_horseran_output.pla", 0, 365, CRC(0fea09b0) SHA1(27a56fcf2b490e9a7dbbc6ad48cc8aaca4cada94) )
ROM_END





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
	mdndclab_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void mdndclab(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void mdndclab_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: input mux part
	m_inp_mux = (m_inp_mux & 0xff) | (data << 8 & 0x3ff00);
}

void mdndclab_state::write_o(u16 data)
{
	// O0-O7: input mux part
	m_inp_mux = (m_inp_mux & ~0xff) | data;
}

u8 mdndclab_state::read_k()
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 475000); // approximation - RC osc. R=27K, C=100pF
	m_maincpu->k().set(FUNC(mdndclab_state::read_k));
	m_maincpu->r().set(FUNC(mdndclab_state::write_r));
	m_maincpu->o().set(FUNC(mdndclab_state::write_o));

	config.set_default_layout(layout_mdndclab); // playing board

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mdndclab )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34012", 0x0000, 0x0800, CRC(e851fccd) SHA1(158362c2821678a51554e02dbb2f9ef5aaf5f59f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mdndclab_output.pla", 0, 365, CRC(592b40ba) SHA1(63a2531278a665ace54c541101e052eb84413511) )
ROM_END





/***************************************************************************

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die label 4A0970D-04A)
  * 10 LEDs behind bezel, no sound

  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.

  known releases:
  - USA: Comp IV (two versions, different case), published by MB
  - Europe: Logic 5, published by MB
  - Japan: Pythaligoras, published by Takara

***************************************************************************/

class comp4_state : public hh_tms1k_state
{
public:
	comp4_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void comp4(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void comp4_state::write_r(u16 data)
{
	// leds:
	// R4    R9
	// R10!  R8
	// R2    R7
	// R1    R6
	// R0    R5
	m_display->matrix(m_o, data);
}

void comp4_state::write_o(u16 data)
{
	// O1-O3: input mux
	m_inp_mux = data >> 1 & 7;

	// O0: leds common
	// other bits: N/C
	m_o = data;
}

u8 comp4_state::read_k()
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
	// basic machine hardware
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(comp4_state::read_k));
	m_maincpu->r().set(FUNC(comp4_state::write_r));
	m_maincpu->o().set(FUNC(comp4_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 11);
	config.set_default_layout(layout_comp4);

	// no sound!
}

// roms

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
	bship_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void bship(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void bship_state::write_r(u16 data)
{
	// R0-R10: input mux
	m_inp_mux = data;
}

void bship_state::write_o(u16 data)
{
	// O4: explosion light bulb
	m_display->matrix(1, data >> 4 & 1);

	// other: sound
}

u8 bship_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[11]->read() | read_inputs(11);
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set(FUNC(bship_state::read_k));
	m_maincpu->r().set(FUNC(bship_state::write_r));
	m_maincpu->o().set(FUNC(bship_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 1);
	config.set_default_layout(layout_bship);

	// sound hardware
	// TODO
}

// roms

ROM_START( bship )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3201", 0x0000, 0x0400, CRC(bf6104a6) SHA1(8d28b43a2aa39dcbbe71f669cdafc518715812c9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bship_output.pla", 0, 365, CRC(ea0570b0) SHA1(6eb803b40717486d7b24939985f245327ac8a7e9) )
ROM_END





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
	bshipb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_sn(*this, "sn76477")
	{ }

	void bshipb(machine_config &config);

private:
	required_device<sn76477_device> m_sn;

	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void bshipb_state::write_r(u16 data)
{
	// R0-R10: input mux
	m_inp_mux = data;

	// R4: 75494 to R12 33K to SN76477 pin 20
	m_sn->slf_res_w((data & 0x10) ? RES_INF : RES_K(33));
}

void bshipb_state::write_o(u16 data)
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
	m_display->matrix(1, data >> 7 & 1);
}

u8 bshipb_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[11]->read() | read_inputs(11);
}

// config

// buttons are same as bship set

void bshipb_state::bshipb(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set(FUNC(bshipb_state::read_k));
	m_maincpu->r().set(FUNC(bshipb_state::write_r));
	m_maincpu->o().set(FUNC(bshipb_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 1);
	config.set_default_layout(layout_bship);

	// sound hardware
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

// roms

ROM_START( bshipb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3208", 0x0000, 0x0400, CRC(982fa720) SHA1(1c6dbbe7b9e55d62a510225a88cd2de55fe9b181) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bshipb_output.pla", 0, 365, BAD_DUMP CRC(74a9a244) SHA1(479c1f1e37cf8f75352e10226b20322906bee813) ) // part of decap photo was obscured
ROM_END





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
	simon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void simon(machine_config &config);

private:
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void simon_state::write_r(u16 data)
{
	// R4-R8 go through an 75494 IC first:
	// R4 -> 75494 IN6 -> green lamp
	// R5 -> 75494 IN3 -> red lamp
	// R6 -> 75494 IN5 -> yellow lamp
	// R7 -> 75494 IN2 -> blue lamp
	m_display->matrix(1, data >> 4);

	// R8 -> 75494 IN0 -> speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R0-R2,R9: input mux
	// R3: GND
	// other bits: N/C
	m_inp_mux = (data & 7) | (data >> 6 & 8);
}

u8 simon_state::read_k()
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

	PORT_START("SWITCH") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<0>, 0x07)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<0>, 0x07)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<3>, 0x0f)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<3>, 0x0f)
INPUT_PORTS_END

void simon_state::simon(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(simon_state::read_k));
	m_maincpu->r().set(FUNC(simon_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_simon);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common3_micro.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1000_simon_output.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END

ROM_START( simonf )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3300", 0x0000, 0x0400, CRC(b9fcf93a) SHA1(45960e4242a08495f2a99fc5d44728eabd93cd9f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common3_micro.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1000_simon_output.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END





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
	ssimon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ssimon(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(speed_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void write_r(u16 data);
	u8 read_k();
};

void ssimon_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void ssimon_state::set_clock()
{
	// MCU clock is from an RC circuit with C=100pF, R=x depending on speed switch:
	// 0 Simple: R=51K -> ~200kHz
	// 1 Normal: R=37K -> ~275kHz
	// 2 Super:  R=22K -> ~400kHz
	u8 inp = m_inputs[6]->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 400000 : ((inp & 1) ? 275000 : 200000));
}

void ssimon_state::write_r(u16 data)
{
	// R0-R3,R9,R10: input mux
	m_inp_mux = (data & 0xf) | (data >> 5 & 0x30);

	// R4: yellow lamps
	// R5: green lamps
	// R6: blue lamps
	// R7: red lamps
	m_display->matrix(1, data >> 4);

	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);
}

u8 ssimon_state::read_k()
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
	PORT_CONFNAME( 0x03, 0x01, "Speed" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ssimon_state, speed_switch, 0)
	PORT_CONFSETTING(    0x00, "Simple" )
	PORT_CONFSETTING(    0x01, "Normal" )
	PORT_CONFSETTING(    0x02, "Super" )

	PORT_START("SWITCH") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<0>, 0x0f)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<0>, 0x0f)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<4>, 0x0f)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<4>, 0x0f)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<6>, 0x03)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<6>, 0x03)
INPUT_PORTS_END

void ssimon_state::ssimon(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 275000); // see set_clock
	m_maincpu->k().set(FUNC(ssimon_state::read_k));
	m_maincpu->r().set(FUNC(ssimon_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_ssimon);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ssimon )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3476", 0x0000, 0x0800, CRC(98200571) SHA1(cbd0bcfc11a534aa0be5d011584cdcac58ff437a) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 ) // unused
	ROM_LOAD( "tms1100_ssimon_output.pla", 0, 365, CRC(0fea09b0) SHA1(27a56fcf2b490e9a7dbbc6ad48cc8aaca4cada94) )
ROM_END





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
	bigtrak_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_left_motor_forward(*this, "left_motor_forward"),
		m_left_motor_reverse(*this, "left_motor_reverse"),
		m_right_motor_forward(*this, "right_motor_forward"),
		m_right_motor_reverse(*this, "right_motor_reverse"),
		m_ext_out(*this, "ext_out")
	{ }

	void bigtrak(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();

	int m_gearbox_pos = 0;
	bool sensor_state() { return m_gearbox_pos < 0 && m_display->element_on(0, 0); }
	TIMER_DEVICE_CALLBACK_MEMBER(gearbox_sim_tick);

	output_finder<> m_left_motor_forward;
	output_finder<> m_left_motor_reverse;
	output_finder<> m_right_motor_forward;
	output_finder<> m_right_motor_reverse;
	output_finder<> m_ext_out;
};

void bigtrak_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// resolve handlers
	m_left_motor_forward.resolve();
	m_left_motor_reverse.resolve();
	m_right_motor_forward.resolve();
	m_right_motor_reverse.resolve();
	m_ext_out.resolve();

	// register for savestates
	save_item(NAME(m_gearbox_pos));
}

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

void bigtrak_state::write_r(u16 data)
{
	// R0-R5,R8: input mux (keypad, ext in enable)
	m_inp_mux = (data & 0x3f) | (data >> 2 & 0x40);

	// R6: N/C
	// R7: IR led on
	// R9: lamp on
	m_display->matrix(1, (data >> 7 & 1) | (data >> 8 & 2));

	// (O0,O7,)R10(tied together): speaker out
	m_speaker->level_w((m_o & 1) | (m_o >> 6 & 2) | (data >> 8 & 4));
	m_r = data;
}

void bigtrak_state::write_o(u16 data)
{
	// O1: left motor forward
	// O2: left motor reverse
	// O3: right motor reverse
	// O4: right motor reverse
	// O5: ext out
	// O6: N/C
	m_left_motor_forward = data >> 1 & 1;
	m_left_motor_reverse = data >> 2 & 1;
	m_right_motor_forward = data >> 3 & 1;
	m_right_motor_reverse = data >> 4 & 1;
	m_ext_out = data >> 5 & 1;

	// O0,O7(,R10)(tied together): speaker out
	m_speaker->level_w((data & 1) | (data >> 6 & 2) | (m_r >> 8 & 4));
	m_o = data;
}

u8 bigtrak_state::read_k()
{
	// K: multiplexed inputs
	// K8: IR sensor
	return read_inputs(7) | (sensor_state() ? 8 : 0);
}

// config

/* physical button layout and labels are like this:

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

void bigtrak_state::bigtrak(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=83K, C=100pF
	m_maincpu->k().set(FUNC(bigtrak_state::read_k));
	m_maincpu->r().set(FUNC(bigtrak_state::write_r));
	m_maincpu->o().set(FUNC(bigtrak_state::write_o));

	TIMER(config, "gearbox").configure_periodic(FUNC(bigtrak_state::gearbox_sim_tick), attotime::from_msec(1));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 2);
	config.set_default_layout(layout_bigtrak);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[8] = { 0.0, 1.0/3.0, 1.0/3.0, 2.0/3.0, 1.0/3.0, 2.0/3.0, 2.0/3.0, 1.0 };
	m_speaker->set_levels(8, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bigtrak )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3301a", 0x0000, 0x0400, CRC(1351bcdd) SHA1(68865389c25b541c09a742be61f8fb6488134d4e) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common4_micro.pla", 0, 867, CRC(80912d0a) SHA1(7ae5293ed4d93f5b7a64d43fe30c3639f39fbe5a) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_bigtrak_output.pla", 0, 365, CRC(63be45f6) SHA1(918e38a223152db883c1a6f7acf56e87d7074734) )
ROM_END





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
	mbdtower_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_motor_pos_out(*this, "motor_pos"),
		m_card_pos_out(*this, "card_pos"),
		m_motor_on_out(*this, "motor_on")
	{ }

	void mbdtower(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void update_display();
	bool sensor_led_on() { return m_display->element_on(0, 0); }

	output_finder<> m_motor_pos_out;
	output_finder<> m_card_pos_out;
	output_finder<> m_motor_on_out;

	int m_motor_pos = 0;
	int m_motor_pos_prev = -1;
	int m_motor_decay = 0;
	bool m_motor_on = false;
	bool m_sensor_blind = false;

	TIMER_DEVICE_CALLBACK_MEMBER(motor_sim_tick);

	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void mbdtower_state::machine_start()
{
	hh_tms1k_state::machine_start();

	m_motor_pos_out.resolve();
	m_card_pos_out.resolve();
	m_motor_on_out.resolve();

	// register for savestates
	save_item(NAME(m_motor_pos));
	/* save_item(NAME(m_motor_pos_prev)); */ // don't save!
	save_item(NAME(m_motor_decay));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_sensor_blind));
}

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
		m_motor_pos_out = 100 * (m_motor_pos / (float)0x80);

	/* 3 display cards per hole, like this:

	    (0)                <---- display increments this way <----                   (7)

	    CURSED   VICTORY    WIZARD         DRAGON    GOLD KEY     SCOUT    WARRIOR   (void)
	    LOST     WARRIORS   BAZAAR CLOSED  SWORD     SILVER KEY   HEALER   FOOD      (void)
	    PLAGUE   BRIGANDS   KEY MISSING    PEGASUS   BRASS KEY    GOLD     BEAST     (void)
	*/
	int card_pos = m_motor_pos >> 4 & 7;
	if (card_pos != (m_motor_pos_prev >> 4 & 7))
		m_card_pos_out = card_pos;

	m_motor_pos_prev = m_motor_pos;
}

void mbdtower_state::update_display()
{
	// update current state
	if (~m_r & 0x10)
	{
		u8 o = bitswap<8>(m_o,7,0,4,3,2,1,6,5) & 0x7f;
		m_display->write_row(2, (m_o & 0x80) ? o : 0);
		m_display->write_row(1, (m_o & 0x80) ? 0 : o);
		m_display->write_row(0, (m_r >> 8 & 1) | (m_r >> 4 & 0xe));
	}
	else
	{
		// display items turned off
		m_display->clear();
	}
}

void mbdtower_state::write_r(u16 data)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R9: motor on
	if ((m_r ^ data) & 0x200)
		m_motor_on_out = data >> 9 & 1;
	if (data & 0x200)
		m_motor_on = true;

	// R3: N/C
	// R4: 75494 /EN (speaker, lamps, digit select go through that IC)
	// R5-R7: tower lamps
	// R8: rotation sensor led
	m_r = data;
	update_display();

	// R10: speaker out
	m_speaker->level_w(~data >> 4 & data >> 10 & 1);
}

void mbdtower_state::write_o(u16 data)
{
	// O0-O6: led segments A-G
	// O7: digit select
	m_o = data;
	update_display();
}

u8 mbdtower_state::read_k()
{
	// K: multiplexed inputs
	// K8: rotation sensor
	return read_inputs(3) | ((!m_sensor_blind && sensor_led_on()) ? 8 : 0);
}

// config

/* physical button layout and labels are like this:

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

void mbdtower_state::mbdtower(machine_config &config)
{
	// basic machine hardware
	TMS1400(config, m_maincpu, 425000); // approximation - RC osc. R=43K, C=56pF
	m_maincpu->k().set(FUNC(mbdtower_state::read_k));
	m_maincpu->r().set(FUNC(mbdtower_state::write_r));
	m_maincpu->o().set(FUNC(mbdtower_state::write_o));

	TIMER(config, "tower_motor").configure_periodic(FUNC(mbdtower_state::motor_sim_tick), attotime::from_msec(3500/0x80)); // ~3.5sec for a full rotation

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 7);
	m_display->set_segmask(6, 0x7f);
	config.set_default_layout(layout_mbdtower);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mbdtower )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7332", 0x0000, 0x1000, CRC(ebeab91a) SHA1(7edbff437da371390fa8f28b3d183f833eaa9be9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mbdtower_output.pla", 0, 557, CRC(64c84697) SHA1(72ce6d24cedf9c606f1742cd5620f75907246e87) )
ROM_END





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
	arcmania_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void arcmania(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void arcmania_state::write_r(u16 data)
{
	// R1-R9: leds
	m_display->matrix(1, data >> 1);
}

void arcmania_state::write_o(u16 data)
{
	// O0-O2(tied together): speaker out
	m_speaker->level_w(data & 7);

	// O3,O4,O6: input mux
	m_inp_mux = (data >> 3 & 3) | (data >> 4 & 4);

	// O5: power off when low (turn back on with button row 1)
	if (~data & 0x20)
		power_off();
}

u8 arcmania_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// config

/* physical button layout and labels are like this:

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

void arcmania_state::arcmania(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 250000); // approximation - RC osc. R=56K, C=100pF
	m_maincpu->k().set(FUNC(arcmania_state::read_k));
	m_maincpu->r().set(FUNC(arcmania_state::write_r));
	m_maincpu->o().set(FUNC(arcmania_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 9);
	config.set_default_layout(layout_arcmania);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[8] = { 0.0, 1.0/3.0, 1.0/3.0, 2.0/3.0, 1.0/3.0, 2.0/3.0, 2.0/3.0, 1.0 };
	m_speaker->set_levels(8, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( arcmania )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34078a", 0x0000, 0x0800, CRC(90ea0087) SHA1(9780c9c1ba89300b1bbe72c47e5fec68d8bb6a77) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_arcmania_output.pla", 0, 365, CRC(a1517b15) SHA1(72eedd7fd41de9c9102219f325fe8668a7c02663) )
ROM_END





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
	cnsector_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void cnsector(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void cnsector_state::write_r(u16 data)
{
	// R0-R5: select digit
	// R6-R9: direction leds
	m_display->matrix(data, m_o);
}

void cnsector_state::write_o(u16 data)
{
	// O0-O4: input mux
	m_inp_mux = data & 0x1f;

	// O0-O7: digit segments
	m_o = data;
}

u8 cnsector_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

/* physical button layout and labels are like this:

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
	// basic machine hardware
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(cnsector_state::read_k));
	m_maincpu->r().set(FUNC(cnsector_state::write_r));
	m_maincpu->o().set(FUNC(cnsector_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	m_display->set_segmask(0x3f, 0xff);
	config.set_default_layout(layout_cnsector);

	// no sound!
}

// roms

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





/***************************************************************************

  Parker Brothers Merlin handheld game, by Bob Doyle
  * TMS1100NLL MP3404 or MP3404A-N2
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
	merlin_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void merlin(machine_config &config);

protected:
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void merlin_state::write_r(u16 data)
{
	/* leds:

	     R0
	R1   R2   R3
	R4   R5   R6
	R7   R8   R9
	     R10
	*/
	m_display->matrix(1, data);
}

void merlin_state::write_o(u16 data)
{
	// O4-O6(tied together): speaker out
	m_speaker->level_w(data >> 4 & 7);

	// O0-O3: input mux
	// O7: N/C
	m_inp_mux = data & 0xf;
}

u8 merlin_state::read_k()
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

void merlin_state::merlin(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=33K, C=100pF
	m_maincpu->k().set(FUNC(merlin_state::read_k));
	m_maincpu->r().set(FUNC(merlin_state::write_r));
	m_maincpu->o().set(FUNC(merlin_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 11);
	config.set_default_layout(layout_merlin);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[8] = { 0.0, 1.0/3.0, 1.0/3.0, 2.0/3.0, 1.0/3.0, 2.0/3.0, 2.0/3.0, 1.0 };
	m_speaker->set_levels(8, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( merlin )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3404a", 0x0000, 0x0800, CRC(7515a75d) SHA1(76ca3605d3fde1df62f79b9bb1f534c2a2ae0229) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_merlin_output.pla", 0, 365, CRC(3921b074) SHA1(12bd58e4d6676eb8c7059ef53598279e4f1a32ea) )
ROM_END

ROM_START( merlina )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3404", 0x0000, 0x0800, CRC(9362d9f9) SHA1(266d2a4a98cc33944a4fc7ed073ba9321bba8e05) ) // 1 bit different

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common3_micro.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_merlin_output.pla", 0, 365, CRC(3921b074) SHA1(12bd58e4d6676eb8c7059ef53598279e4f1a32ea) )
ROM_END





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
	mmerlin_state(const machine_config &mconfig, device_type type, const char *tag) :
		merlin_state(mconfig, type, tag)
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
	merlin(config);

	// basic machine hardware
	TMS1400(config.replace(), m_maincpu, 425000); // approximation - RC osc. R=30K, C=100pF
	m_maincpu->k().set(FUNC(mmerlin_state::read_k));
	m_maincpu->r().set(FUNC(mmerlin_state::write_r));
	m_maincpu->o().set(FUNC(mmerlin_state::write_o));

	config.set_default_layout(layout_mmerlin);
}

// roms

ROM_START( mmerlin )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7351", 0x0000, 0x1000, CRC(0f7a4c83) SHA1(242c1278ddfe92c28fd7cd87300e48e7a4827831) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mmerlin_output.pla", 0, 557, CRC(fd3dcd93) SHA1(f2afc52df700daa0eb7356c7876af9b2966f971b) )
ROM_END





/***************************************************************************

  Parker Brothers Electronic Master Mind
  * TMS1000NLL MP3200 (die label 1000E, MP3200)
  * 5 red leds, 5 green leds

  This is a board game, it came with 4 plug boards and a lot of colored pegs.
  It's not related to the equally named Electronic Master Mind by Invicta,
  that one is on a Rockwell MM75.

***************************************************************************/

class pbmastm_state : public hh_tms1k_state
{
public:
	pbmastm_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void pbmastm(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void pbmastm_state::write_r(u16 data)
{
	// R1-R10: leds (direct)
	m_display->matrix(1, data >> 1);
}

void pbmastm_state::write_o(u16 data)
{
	// O0-O5: input mux
	m_inp_mux = data & 0x3f;
}

u8 pbmastm_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

static INPUT_PORTS_START( pbmastm )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("D")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Brown")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Black")

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Yellow")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("White")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Pink")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Gray")

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Green")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Orange")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Blank")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Delete")

	PORT_START("IN.5") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Battery Test")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Code")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Check")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pbmastm_state::pbmastm(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 300000); // approximation - RC osc. R=56K, C=47pF
	m_maincpu->k().set(FUNC(pbmastm_state::read_k));
	m_maincpu->r().set(FUNC(pbmastm_state::write_r));
	m_maincpu->o().set(FUNC(pbmastm_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 10);
	config.set_default_layout(layout_pbmastm);

	// no sound!
}

// roms

ROM_START( pbmastm )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3200", 0x0000, 0x0400, CRC(059dbb88) SHA1(20e3f6aeecce167371bda914f263daf53c50c839) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_pbmastm_output.pla", 0, 365, CRC(b355c4d9) SHA1(43cc971a4feacfddfe810c8983c65d9eef3ed57b) )
ROM_END





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
	stopthief_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void stopthief(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void stopthief_state::write_r(u16 data)
{
	// R0-R2: select digit
	m_display->matrix(data & 7, bitswap<8>(m_o,3,5,2,1,4,0,6,7) & 0x7f);

	// R3-R8(tied together): speaker out
	int level = 0;
	for (int i = 0; m_o & 8 && i < 6; i++)
		level += (data >> (i+3) & 1);
	m_speaker->level_w(level);
}

void stopthief_state::write_o(u16 data)
{
	// O0,O6: input mux
	m_inp_mux = (data & 1) | (data >> 5 & 2);

	// O3: speaker on
	// O0-O2,O4-O7: led segments A-G
	m_o = data;
}

u8 stopthief_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[2]->read() | read_inputs(2);
}

// config

/* physical button layout and labels are like this:

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

void stopthief_state::stopthief(machine_config &config)
{
	// basic machine hardware
	TMS0980(config, m_maincpu, 425000); // approximation
	m_maincpu->k().set(FUNC(stopthief_state::read_k));
	m_maincpu->r().set(FUNC(stopthief_state::write_r));
	m_maincpu->o().set(FUNC(stopthief_state::write_o));
	m_maincpu->power_off().set(FUNC(stopthief_state::auto_power_off));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 7);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_stopthief);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[7] = { 1.0/7.0, 1.0/6.0, 1.0/5.0, 1.0/4.0, 1.0/3.0, 1.0/2.0, 1.0 };
	m_speaker->set_levels(7, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

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

  BTANB: Some of the other (not cue) balls temporarily flash brighter sometimes,
  eg. the bottom one when they're placed. This happens on the real device.

***************************************************************************/

class bankshot_state : public hh_tms1k_state
{
public:
	bankshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void bankshot(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void bankshot_state::update_display()
{
	m_display->matrix(m_r & ~3, m_o);
}

void bankshot_state::write_r(u16 data)
{
	// R0: speaker out
	m_speaker->level_w(data & 1);

	// R2,R3: input mux
	m_inp_mux = data >> 2 & 3;

	// R2-R10: led select
	m_r = data;
	update_display();
}

void bankshot_state::write_o(u16 data)
{
	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 bankshot_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(2);
}

// config

/* physical button layout and labels are like this:
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
	// basic machine hardware
	TMS1400(config, m_maincpu, 475000); // approximation - RC osc. R=24K, C=100pF
	m_maincpu->k().set(FUNC(bankshot_state::read_k));
	m_maincpu->r().set(FUNC(bankshot_state::write_r));
	m_maincpu->o().set(FUNC(bankshot_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 8);
	m_display->set_bri_levels(0.01, 0.08); // cue ball is brigher
	config.set_default_layout(layout_bankshot);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bankshot )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7313", 0x0000, 0x1000, CRC(7a5016a9) SHA1(a8730dc8a282ffaa3d89e675f371d43eb39f39b4) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_bankshot_output.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END





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
	splitsec_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void splitsec(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void splitsec_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void splitsec_state::write_r(u16 data)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// R9,R10: input mux
	m_inp_mux = data >> 9 & 3;

	// R0-R7: led select
	m_r = data;
	update_display();
}

void splitsec_state::write_o(u16 data)
{
	// O0-O6: led state
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 splitsec_state::read_k()
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
	// basic machine hardware
	TMS1400(config, m_maincpu, 475000); // approximation - RC osc. R=24K, C=100pF
	m_maincpu->k().set(FUNC(splitsec_state::read_k));
	m_maincpu->r().set(FUNC(splitsec_state::write_r));
	m_maincpu->o().set(FUNC(splitsec_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	config.set_default_layout(layout_splitsec);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( splitsec )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7314", 0x0000, 0x1000, CRC(e94b2098) SHA1(f0fc1f56a829252185592a2508740354c50bedf8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_splitsec_output.pla", 0, 557, CRC(7539283b) SHA1(f791fa98259fc10c393ff1961d4c93040f1a2932) )
ROM_END





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
	lostreas_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void lostreas(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void lostreas_state::write_r(u16 data)
{
	// R0-R10: leds
	m_display->matrix(1, data);
}

void lostreas_state::write_o(u16 data)
{
	// O0-O3: input mux
	m_inp_mux = data & 0xf;

	// O4: 330 ohm resistor - speaker out
	// O5: 220 ohm resistor - speaker out
	// O6: 180 ohm resistor - speaker out
	// O7:  68 ohm resistor - speaker out
	m_speaker->level_w(data >> 4 & 0xf);
}

u8 lostreas_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

/* physical button layout and labels are like this:
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

void lostreas_state::lostreas(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 425000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(lostreas_state::read_k));
	m_maincpu->r().set(FUNC(lostreas_state::write_r));
	m_maincpu->o().set(FUNC(lostreas_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 11);
	config.set_default_layout(layout_lostreas);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	// set volume levels
	static const double speaker_levels[0x10] =
		{ 1.0/16.0, 1.0/15.0, 1.0/14.0, 1.0/13.0, 1.0/12.0, 1.0/11.0, 1.0/10.0, 1.0/9.0, 1.0/8.0, 1.0/7.0, 1.0/6.0, 1.0/5.0, 1.0/4.0, 1.0/3.0, 1.0/2.0, 1.0 };
	m_speaker->set_levels(16, speaker_levels);
}

// roms

ROM_START( lostreas )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "m34038", 0x0000, 0x0800, CRC(4c996f63) SHA1(ebbaa8b2f909f4300887aa2dbdb7185eedc75d3f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_lostreas_output.pla", 0, 365, CRC(c62d850f) SHA1(d25974e6901eb10c52cdda12e6d4a13e26745e6f) )
ROM_END





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
	alphie_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void alphie(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void alphie_state::write_r(u16 data)
{
	// R1-R5, input mux (using d5 for Vss)
	m_inp_mux = (data >> 1 & 0x1f) | 0x20;

	// R6-R10: leds
	m_display->matrix(1, data >> 6);

	// R0: power off on falling edge (turn back on with button)
	if (~data & m_r & 1)
		power_off();

	m_r = data;
}

void alphie_state::write_o(u16 data)
{
	// O?: speaker out
	m_speaker->level_w(data & 1);
}

u8 alphie_state::read_k()
{
	// K: multiplexed inputs, rotated matrix
	return read_rotated_inputs(6);
}

// config

static INPUT_PORTS_START( alphie )
	PORT_START("IN.0") // K1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)

	PORT_START("IN.1") // K2
	PORT_CONFNAME( 0x1f, 0x01, "Question" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x08, "4" )
	PORT_CONFSETTING(    0x10, "5" )

	PORT_START("IN.2") // K4
	PORT_CONFNAME( 0x1f, 0x01, "Answer" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x08, "4" )
	PORT_CONFSETTING(    0x10, "5" )

	PORT_START("IN.3") // K8
	PORT_CONFNAME( 0x0f, 0x01, "Activity" )
	PORT_CONFSETTING(    0x01, "Questions" )
	PORT_CONFSETTING(    0x02, "Lunar Landing" )
	PORT_CONFSETTING(    0x04, "Robot Land" )
	PORT_CONFSETTING(    0x08, "Tunes" )

	PORT_START("SWITCH") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<1>, 0x1f) PORT_NAME("Question Arm Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<1>, 0x1f) PORT_NAME("Question Arm Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<2>, 0x1f) PORT_NAME("Answer Arm Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<2>, 0x1f) PORT_NAME("Answer Arm Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_prev<3>, 0x0f) PORT_NAME("Activity Selector Left")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, switch_next<3>, 0x0f) PORT_NAME("Activity Selector Right")
INPUT_PORTS_END

// output PLA is guessed
static const u16 alphie_output_pla[0x20] =
{
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};

void alphie_state::alphie(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 350000); // approximation
	m_maincpu->set_output_pla(alphie_output_pla);
	m_maincpu->k().set(FUNC(alphie_state::read_k));
	m_maincpu->r().set(FUNC(alphie_state::write_r));
	m_maincpu->o().set(FUNC(alphie_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 5);
	config.set_default_layout(layout_alphie);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( alphie )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "us4280809", 0x0000, 0x0400, CRC(f8f14013) SHA1(bf31b929fcbcb189bbe4623104e1da0a639b5954) ) // from patent US4280809, should be good

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, BAD_DUMP CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) ) // not in patent description
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1000_alphie_output.pla", 0, 365, NO_DUMP ) // "
ROM_END





/***************************************************************************

  Tandy Championship Football (model 60-2150)
  * PCB label CYG-316
  * TMS1100NLL MP1193 (die label 1100B, MP1193)
  * 7-digit 7seg LED display + LED grid, 1-bit sound

  Another clone of Mattel Football II. The original manufacturer is unknown, but
  suspected to be Conic/E.R.S.(Electronic Readout Systems).

***************************************************************************/

class tcfball_state : public hh_tms1k_state
{
public:
	tcfball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void tcfball(machine_config &config);

protected:
	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void tcfball_state::update_display()
{
	// R8 enables leds, R9 enables digits
	u16 mask = ((m_r >> 9 & 1) * 0x7f) | ((m_r >> 8 & 1) * 0x780);
	u16 sel = ((m_r & 0x7f) | (m_r << 7 & 0x780)) & mask;

	m_display->matrix(sel, m_o);
}

void tcfball_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R7: input mux
	m_inp_mux = data >> 5 & 7;

	// R8+R0-R3: select led
	// R9+R0-R6: select digit
	m_r = data;
	update_display();
}

void tcfball_state::write_o(u16 data)
{
	// O0-O7: digit segments/led data
	m_o = data;
	update_display();
}

u8 tcfball_state::read_k()
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=56K, C=24pF
	m_maincpu->k().set(FUNC(tcfball_state::read_k));
	m_maincpu->r().set(FUNC(tcfball_state::write_r));
	m_maincpu->o().set(FUNC(tcfball_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 8);
	m_display->set_segmask(0x77, 0x7f);
	m_display->set_segmask(0x08, 0xff); // R3 has DP
	m_display->set_bri_levels(0.003, 0.03); // offense leds are brighter
	config.set_default_layout(layout_tcfball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tcfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1193", 0x0000, 0x0800, CRC(7d9f446f) SHA1(bb6af47b42d989494f21475a73f072cddf58c99f) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tcfball_output.pla", 0, 365, CRC(26b2996e) SHA1(df0e706c552bf74123aa65e71b0c9b4d33cddb2b) )
ROM_END





/***************************************************************************

  Tandy Championship Football (model 60-2151)
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
	tcfballa_state(const machine_config &mconfig, device_type type, const char *tag) :
		tcfball_state(mconfig, type, tag)
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

void tcfballa_state::tcfballa(machine_config &config)
{
	tcfball(config);

	// basic machine hardware
	m_maincpu->set_clock(375000); // approximation - RC osc. R=47K, C=50pF

	config.set_default_layout(layout_tcfballa);
}

// roms

ROM_START( tcfballa )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1183", 0x0000, 0x0800, CRC(2a4db1d5) SHA1(5df15d1115bb425578ad522d607a582dd478f35c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_tcfballa_output.pla", 0, 365, NO_DUMP )

	ROM_REGION16_LE( 0x40, "maincpu:opla_b", ROMREGION_ERASE00 ) // verified, electronic dump
	ROM_LOAD16_BYTE( "tms1100_tcfballa_output.bin", 0, 0x20, CRC(887ad9a5) SHA1(a2bac48f555df098c1f1f0fa663e5bcb989b8987) )
ROM_END





/***************************************************************************

  Tandy Computerized Arcade (model 60-2159 or 60-2159A)
  * TMS1100 MCU, label CD7282SL
  * 12 lamps behind buttons, 1-bit sound

  known releases:
  - World: Tandy-12: Computerized Arcade, published by Tandy
  - World: Computerized Arcade, published by Tandy, Radio Shack brand.
    This one is model 60-2495 from the mid-90s, perhaps different hardware!
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
	tandy12_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void tandy12(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void tandy12_state::update_display()
{
	m_display->matrix(1, (m_o << 1 & 0x1fe) | (m_r << 9 & 0x1e00));
}

void tandy12_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R9: input mux
	m_inp_mux = data >> 5 & 0x1f;

	// R0-R3: button lamps 9-12
	m_r = data;
	update_display();
}

void tandy12_state::write_o(u16 data)
{
	// O0-O7: button lamps 1-8
	m_o = data;
	update_display();
}

u8 tandy12_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

/* physical button layout and labels are like this:

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

// output PLA is not decapped, this was made by hand
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
	// basic machine hardware
	TMS1100(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->set_output_pla(tandy12_output_pla);
	m_maincpu->k().set(FUNC(tandy12_state::read_k));
	m_maincpu->r().set(FUNC(tandy12_state::write_r));
	m_maincpu->o().set(FUNC(tandy12_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 13);
	config.set_default_layout(layout_tandy12);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tandy12 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x0800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_tandy12_output.pla", 0, 365, NO_DUMP )
ROM_END





/***************************************************************************

  Tandy(Radio Shack division) Monkey See (1982 version)
  * TMS1000 MP0271 (die label 1000E, MP0271), only half of ROM space used
  * 2 LEDs(one red, one green), 1-bit sound

  This is the TMS1000 version, the one from 1977 has a MM5780.
  To play, enter an equation followed by the ?-key, and the calculator will
  tell you if it was right(green) or wrong(red). For example 1+2=3?

  known releases:
  - USA(1): Monkey See, published by Tandy
  - USA(2): Heathcliff, published by McNaught Syndicate in 1983

***************************************************************************/

class monkeysee_state : public hh_tms1k_state
{
public:
	monkeysee_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void monkeysee(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void monkeysee_state::write_r(u16 data)
{
	// R0-R4: input mux
	m_inp_mux = data & 0x1f;

	// R5: speaker out
	m_speaker->level_w(data >> 5 & 1);
}

void monkeysee_state::write_o(u16 data)
{
	// O6,O7: leds
	// other: N/C
	m_display->matrix(1, data >> 6);
}

u8 monkeysee_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 250000); // approximation - RC osc. R=68K, C=47pF
	m_maincpu->k().set(FUNC(monkeysee_state::read_k));
	m_maincpu->r().set(FUNC(monkeysee_state::write_r));
	m_maincpu->o().set(FUNC(monkeysee_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 2);
	config.set_default_layout(layout_monkeysee);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( monkeysee )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0271", 0x0000, 0x0400, CRC(acab0f05) SHA1(226f7688caf4a94a88241d3b61ddc4254e4a918c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_monkeysee_micro.pla", 0, 867, CRC(368d878f) SHA1(956e700a04f453c1610cfdb974fce898ba4cf01f) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_monkeysee_output.pla", 0, 365, CRC(8a010e89) SHA1(3ffbabc5d6c9b34cc06d290817d15b2be42d8b17) )
ROM_END





/***************************************************************************

  Tandy 3 in 1 Sports Arena (model 60-2178)
  * PCB label HP-804
  * TMS1100 (just a datestamp label (8331), die label 1100B MP1231)
  * 2x2-digit 7seg LED display + 47 other LEDs, 1-bit sound

  For Tandy Sports Arena (model 60-2158), see cmsport, this is a different game.
  This version is very similar to ssports4 released a few years earlier.

  3 overlays were included for the games (Tandy calls them graphic sheets),
  MAME external artwork is needed for those.

  It is always in 2-player head-to-head mode, the Player switch is just meant
  for allowing the other player to have full control over 2 defense spots.

***************************************************************************/

class t3in1sa_state : public hh_tms1k_state
{
public:
	t3in1sa_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void t3in1sa(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void t3in1sa_state::update_display()
{
	m_display->matrix(m_r, m_o | (m_r << 6 & 0x100));
}

void t3in1sa_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0,R1,R5,R7-R9: input mux
	m_inp_mux = (data & 3) | (data >> 3 & 4) | (data >> 4 & 0x38);

	// R2: led data high
	// R3-R7: led select
	// R0,R1,R8,R9: digit select
	m_r = data;
	update_display();
}

void t3in1sa_state::write_o(u16 data)
{
	// O0-O7: led data low
	m_o = data;
	update_display();
}

u8 t3in1sa_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

/* physical button layout and labels are like this:

                  ↑
     [ ]         [ ]         [ ]
  TEAM-MATE                  PASS
          ←[ ]         [ ]→  shoot
        ↗                   ↖
     [ ]         [ ]         [ ]
    STATUS        ↓          KICK
                             pass
                             shoot

Game and difficulty switches are under the yellow buttons,
player switch is under the red buttons. P1 is yellow, P2 is red.

*/

static INPUT_PORTS_START( t3in1sa )
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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" ) // AMATEUR
	PORT_CONFSETTING(    0x01, "2" ) // PRO
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x0e, 0x08, "Game Select" )
	PORT_CONFSETTING(    0x08, "Football" ) // F
	PORT_CONFSETTING(    0x04, "Basketball" ) // B
	PORT_CONFSETTING(    0x02, "Soccer" ) // S

	PORT_START("IN.4") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Up-Left / Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Up-Right / Status")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pass / Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Team-Mate")

	PORT_START("IN.5") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Up-Left / Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Up-Right / Status")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Pass / Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Team-Mate")
INPUT_PORTS_END

void t3in1sa_state::t3in1sa(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(t3in1sa_state::read_k));
	m_maincpu->r().set(FUNC(t3in1sa_state::write_r));
	m_maincpu->o().set(FUNC(t3in1sa_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 9);
	m_display->set_segmask(0x303, 0x7f);
	m_display->set_bri_levels(0.005, 0.05); // offense leds are brighter
	config.set_default_layout(layout_t3in1sa);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( t3in1sa )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1231", 0x0000, 0x0800, CRC(1c24e5c2) SHA1(0b6c2edea27eba15e890d82475b91a5e9ef6c4b9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_t3in1sa_output.pla", 0, 365, CRC(de82a294) SHA1(7187666a510919b90798b92b9104ac5d6820d559) )
ROM_END





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
	speechp_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_speech(*this, "speech")
	{ }

	void speechp(machine_config &config);

private:
	required_device<s14001a_device> m_speech;

	void update_display();
	virtual void write_r(u16 data);
	virtual void write_o(u16 data);
	virtual u8 read_k();
};

// handlers

void speechp_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void speechp_state::write_r(u16 data)
{
	// R5-R9: TSI C0-C5
	m_speech->data_w(data >> 5 & 0x3f);

	// R10: TSI START line
	m_speech->start_w(data >> 10 & 1);

	// R0-R9: input mux
	m_inp_mux = data & 0x3ff;

	// R0-R8: select digit
	m_r = data;
	update_display();
}

void speechp_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 speechp_state::read_k()
{
	// K: multiplexed inputs
	return m_inputs[10]->read() | (read_inputs(10) & 7);
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 400000); // approximation - RC osc. R=39K, C=47pF
	m_maincpu->k().set(FUNC(speechp_state::read_k));
	m_maincpu->r().set(FUNC(speechp_state::write_r));
	m_maincpu->o().set(FUNC(speechp_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_speechp);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	S14001A(config, m_speech, 25000); // approximation
	m_speech->add_route(ALL_OUTPUTS, "mono", 0.75);
}

// roms

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





/***************************************************************************

  TI SR-16 (1974, first consumer product with TMS1000 series MCU)
  * TMS1000 MCU label TMS1001NL (die label 1000, 1001A)
  * 12-digit 7seg LED display

  TI SR-16 II (1975 version)
  * TMS1000 MCU label TMS1016NL (die label 1000B, 1016A)
  * notes: cost-reduced 'sequel', [10^x] was removed, and [pi] was added.

***************************************************************************/

class tisr16_state : public hh_tms1k_state
{
public:
	tisr16_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void tisr16(machine_config &config);

private:
	void update_display();
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void tisr16_state::update_display()
{
	m_display->matrix(m_r, m_o);

	// exponent sign is from R10 O1, and R10 itself only uses segment G
	u8 r10 = m_display->read_row(10);
	m_display->write_row(11, r10 << 5 & 0x40);
	m_display->write_row(10, r10 & 0x40);
}

void tisr16_state::write_r(u16 data)
{
	// R0-R10: input mux, select digit
	m_r = m_inp_mux = data;
	update_display();
}

void tisr16_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 tisr16_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(11);
}

// config

static INPUT_PORTS_START( tisr16 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_NAME("EE")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME(UTF8_CAPITAL_SIGMA)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("1/x")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("y" UTF8_POW_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("10" UTF8_POW_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("e" UTF8_POW_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT"x")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.10") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("log")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("ln(x)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tisr16ii )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CD")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("log")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_NAME("EE")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT"x")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("ln(x)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("1/x")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("e" UTF8_POW_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME(UTF8_CAPITAL_SIGMA)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.10") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("y" UTF8_POW_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME(UTF8_SMALL_PI)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
INPUT_PORTS_END

void tisr16_state::tisr16(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 300000); // approximation - RC osc. R=43K, C=68pf (note: tisr16ii MCU RC osc. is different: R=30K, C=100pf, same freq)
	m_maincpu->k().set(FUNC(tisr16_state::read_k));
	m_maincpu->o().set(FUNC(tisr16_state::write_o));
	m_maincpu->r().set(FUNC(tisr16_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(12, 8);
	m_display->set_segmask(0xfff, 0xff);
	config.set_default_layout(layout_tisr16);

	// no sound!
}

// roms

ROM_START( tisr16 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1001nl", 0x0000, 0x0400, CRC(b7ce3c1d) SHA1(95cdb0c6be31043f4fe06314ed41c0ca1337bc46) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_sr16_micro.pla", 0, 867, CRC(5b35019c) SHA1(730d3b9041ed76d57fbedd73b009477fe432b386) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_sr16_output.pla", 0, 365, CRC(29b08739) SHA1(d55f01e40a2d493d45ea422f12e63b01bcde08fb) )
ROM_END

ROM_START( tisr16ii )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1016nl", 0x0000, 0x0400, CRC(c07a7b27) SHA1(34ea4d3b59871e08db74f8c5bfb7ff00d1f0adc7) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_sr16ii_micro.pla", 0, 867, CRC(31b43e95) SHA1(6864e4c20f3affffcd3810dcefbc9484dd781547) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_sr16ii_output.pla", 0, 365, CRC(c45dfbd0) SHA1(5d588c1abc317134b51eb08ac3953f1009d80056) )
ROM_END





/***************************************************************************

  TI-1250/TI-1200 (1975 version), Spirit of '76
  * TMS0950 MCU label TMC0952NL, K0952 (die label 0950A 0952)
  * 9-digit 7seg LED display

  TI-1250/TI-1200 (1976 version), TI-1400, TI-1450, TI-1205, TI-1255, LADY 1200, ABLE
  * TMS0970 MCU label TMS0972NL ZA0348, JP0972A (die label 0970D-72A)
  * 8-digit 7seg LED display, or 9 digits with leftmost unused

  As seen listed above, the basic 4-function TMS0972 calculator MCU was used
  in many calculators. It was licensed to other manufacturers too, one funny
  example being a Mattel Barbie handheld calculator.

  Some cheaper models lacked the memory buttons (the function itself still works).
  The ABLE series was for educational purposes, with each having a small subset of
  available buttons.

  TI-1270
  * TMS0970 MCU label TMC0974NL ZA0355, DP0974A (die label 0970D-74A)
  * 8-digit 7seg LED display
  * notes: almost same hardware as TMS0972 TI-1250, minor scientific functions

***************************************************************************/

class ti1250_state : public hh_tms1k_state
{
public:
	ti1250_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ti1270(machine_config &config);
	void ti1250(machine_config &config);

private:
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void ti1250_state::write_r(u16 data)
{
	// R0-R8: select digit
	m_display->matrix(data, m_o);
}

void ti1250_state::write_o(u16 data)
{
	// O1-O5,O7: input mux
	// O0-O7: digit segments
	m_inp_mux = (data >> 1 & 0x1f) | (data >> 2 & 0x20);
	m_o = data;
}

u8 ti1250_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(6);
}

// config

static INPUT_PORTS_START( ti1250 )
	PORT_START("IN.0") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")

	PORT_START("IN.1") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.2") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.3") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("CS") // named either CS(Change Sign) or +/-
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.5") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("MC")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("MR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT) PORT_NAME("M-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("M+")
INPUT_PORTS_END

static INPUT_PORTS_START( ti1270 )
	PORT_INCLUDE( ti1250 )

	PORT_MODIFY("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CE/C")

	PORT_MODIFY("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME(UTF8_SMALL_PI)

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("1/x")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT"x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
INPUT_PORTS_END

void ti1250_state::ti1250(machine_config &config)
{
	// basic machine hardware
	TMS0950(config, m_maincpu, 200000); // approximation - RC osc. R=68K, C=68pf
	m_maincpu->k().set(FUNC(ti1250_state::read_k));
	m_maincpu->o().set(FUNC(ti1250_state::write_o));
	m_maincpu->r().set(FUNC(ti1250_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0xff, 0xff);
	m_display->set_segmask(0x100, 0x40); // R8 only has segment G connected
	config.set_default_layout(layout_ti1250);

	// no sound!
}

void ti1250_state::ti1270(machine_config &config)
{
	ti1250(config);

	// basic machine hardware
	TMS0970(config.replace(), m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(ti1250_state::read_k));
	m_maincpu->o().set(FUNC(ti1250_state::write_o));
	m_maincpu->r().set(FUNC(ti1250_state::write_r));

	config.set_default_layout(layout_ti1270);
}

// roms

ROM_START( ti1250 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0952nl", 0x0000, 0x0400, CRC(fc0cee65) SHA1(1480e4553181f081281d3b78457721b9ecb20173) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ti1250_micro.pla", 0, 867, CRC(cb3fd2d6) SHA1(82cf36a65dfc3ccb9dd08e48f45ac4d90f693238) )
	ROM_REGION( 195, "maincpu:opla", 0 )
	ROM_LOAD( "tms0950_ti1250_output.pla", 0, 195, CRC(31570eb8) SHA1(c1cb17c31367b65aa777925459515c3d5c565508) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END

ROM_START( ti125076 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms0972nl_za0348", 0x0000, 0x0400, CRC(6e3f8add) SHA1(a249209e2a92f5016e33b7ad2c6c2660df1df959) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common1_micro.pla", 0, 860, CRC(6ff5d51d) SHA1(59d3e5de290ba57694068ddba78d21a0c1edf427) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_ti1270_output.pla", 0, 352, CRC(472f95a0) SHA1(32adb17285f2f3f93a4b027a3dd2156ec48000ec) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END

ROM_START( ti1270 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0974nl_za0355", 0x0000, 0x0400, CRC(48e09b4b) SHA1(17f27167164df223f9f06082ece4c3fc3900eda3) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common1_micro.pla", 0, 860, CRC(6ff5d51d) SHA1(59d3e5de290ba57694068ddba78d21a0c1edf427) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_ti1270_output.pla", 0, 352, CRC(472f95a0) SHA1(32adb17285f2f3f93a4b027a3dd2156ec48000ec) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END





/***************************************************************************

  TI-2550 III, TI-1650/TI-1600, TI-1265 (they have the same chip)
  * TMS1040 MCU label TMS1043NL ZA0352 (die label 1040A, 1043A)
  * 9-digit cyan VFD display

  Only the TI-2550 III has the top button row (RV, SQRT, etc).
  TI-1600 doesn't have the memory buttons.

***************************************************************************/

class ti25503_state : public hh_tms1k_state
{
public:
	ti25503_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ti25503(machine_config &config);

private:
	void update_display();
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void ti25503_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void ti25503_state::write_r(u16 data)
{
	// R0-R6: input mux
	// R0-R8: select digit
	m_r = m_inp_mux = data;
	update_display();
}

void ti25503_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 ti25503_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(7);
}

// config

static INPUT_PORTS_START( ti25503 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("CM")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("MR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_INSERT) PORT_NAME("M-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("M+")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT"x")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("1/x")
INPUT_PORTS_END

void ti25503_state::ti25503(machine_config &config)
{
	// basic machine hardware
	TMS1040(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(ti25503_state::read_k));
	m_maincpu->o().set(FUNC(ti25503_state::write_o));
	m_maincpu->r().set(FUNC(ti25503_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_ti25503);

	// no sound!
}

// roms

ROM_START( ti25503 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1043nl_za0352", 0x0000, 0x0400, CRC(434c2684) SHA1(ff566f1991f63cfe057879674e6bc7ccd580a919) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ti25503_micro.pla", 0, 867, CRC(65d274ae) SHA1(33d77efe38f8b067096c643d71263bb5adde0ca9) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ti25503_output.pla", 0, 365, CRC(ac43b768) SHA1(5eb19b493328c73edab73e44591afda0fbe4965f) )
ROM_END





/***************************************************************************

  TI-5100, more (see below)
  * TMS1070 MCU label TMS1073NL or TMC1073NL (die label 1070B, 1073)
  * 11-digit 7seg VFD (1 custom digit)

  This chip was also used in 3rd-party calculators, such as Toshiba BC-1015,
  Panasonic JE1601U, Radio Shack EC2001, Triumph-Adler D100. The original
  TI version did not have the 3 buttons at R4.

***************************************************************************/

class ti5100_state : public hh_tms1k_state
{
public:
	ti5100_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ti5100(machine_config &config);

private:
	void update_display();
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void ti5100_state::update_display()
{
	// extra segment on R10
	m_display->matrix(m_r, (m_r >> 2 & 0x100) | m_o);
}

void ti5100_state::write_r(u16 data)
{
	// R0-R10: input mux, select digit
	m_r = m_inp_mux = data;
	update_display();
}

void ti5100_state::write_o(u16 data)
{
	// O0-O7: digit segments
	m_o = data;
	update_display();
}

u8 ti5100_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(11);
}

// config

static INPUT_PORTS_START( ti5100 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "K" ) // constant mode
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "DP" ) // display point
	PORT_CONFSETTING(    0x00, "F" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // clear all?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("EX")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("GPM")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME(UTF8_CAPITAL_DELTA"%")

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("C/CE")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("CM")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("N")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // duplicate of R8 0x01
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // duplicate of R9 0x02
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+=")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-=")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.10") // R10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("M+=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_INSERT) PORT_NAME("M-=")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("RM")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
INPUT_PORTS_END

void ti5100_state::ti5100(machine_config &config)
{
	// basic machine hardware
	TMS1070(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(ti5100_state::read_k));
	m_maincpu->o().set(FUNC(ti5100_state::write_o));
	m_maincpu->r().set(FUNC(ti5100_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 9);
	m_display->set_segmask(0x7ff, 0xff);
	config.set_default_layout(layout_ti5100);

	// no sound!
}

// roms

ROM_START( ti5100 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1073nl", 0x0000, 0x0400, CRC(94185933) SHA1(d2d9432f857b8530ac399f5097c6d412f06d8814) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ti5100_micro.pla", 0, 867, CRC(31b43e95) SHA1(6864e4c20f3affffcd3810dcefbc9484dd781547) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ti5100_output.pla", 0, 365, CRC(11f6f0f4) SHA1(f25cf6bd284ab4614746b2e3f98d42d2585e425a) )
ROM_END





/***************************************************************************

  TMC098x series Majestic-line calculators

  TI-30, SR-40, TI-15(less buttons) and several by Koh-I-Noor
  * TMS0980 MCU label TMC0981NL (die label 0980B-81F)
  * 9-digit 7seg LED display

  Of note is a peripheral by Schoenherr, called the Braillotron. It acts as
  a docking station to the TI-30, with an additional display made of magnetic
  refreshable Braille cells. The TI-30 itself is slightly modified to wire
  the original LED display to a 25-pin D-Sub connector.

  TI Business Analyst, TI Business Analyst-I, TI Money Manager, TI-31, TI-41
  * TMS0980 MCU label TMC0982NL (die label 0980B-82F)

  TI Programmer
  * TMS0980 MCU label ZA0675NL, JP0983AT (die label 0980B-83)

***************************************************************************/

class ti30_state : public hh_tms1k_state
{
public:
	ti30_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ti30(machine_config &config);

private:
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void ti30_state::write_r(u16 data)
{
	// R0-R8: select digit
	m_display->matrix(data, m_o);
}

void ti30_state::write_o(u16 data)
{
	// O0-O2,O4-O7: input mux
	// O0-O7: digit segments
	m_inp_mux = (data & 7) | (data >> 1 & 0x78);
	m_o = bitswap<8>(data,7,5,2,1,4,0,6,3);
}

u8 ti30_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[7]->read() | read_inputs(7);
}

// config

static INPUT_PORTS_START( ti30 )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("y" UTF8_POW_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_NAME("K")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("log")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_NAME("EE" UTF8_DOWN)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("ln(x)")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.3") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME(UTF8_SMALL_PI)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("(")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(")")

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("SUM")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.5") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("DRG")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("INV")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("cos")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("sin")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("tan")

	PORT_START("IN.6") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("EXC")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.7") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("On/C") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("1/x")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME(UTF8_SQUAREROOT"x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
INPUT_PORTS_END

static INPUT_PORTS_START( tiprog )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_NAME("K")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("SHF")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("E")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_NAME("d")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_NAME("F")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("OR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("AND")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.3") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_NAME("1'sC")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_NAME("b")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_NAME("A")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("C")

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("XOR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.5") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(")")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("SUM")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("(")

	PORT_START("IN.6") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.7") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("C/ON") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_NAME("DEC")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_NAME("OCT")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_NAME("HEX")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
INPUT_PORTS_END

static INPUT_PORTS_START( tibusan )
	// PORT_NAME lists functions under [2nd] as secondaries.
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("y" UTF8_POW_X"  " UTF8_POW_X"" UTF8_SQUAREROOT"y") // 2nd one implies xth root of y
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%  " UTF8_CAPITAL_DELTA"%")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("SEL")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("CST")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_NAME("MAR")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("STO  m")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("RCL  b")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")

	PORT_START("IN.3") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME(UTF8_CAPITAL_SIGMA"+  " UTF8_CAPITAL_SIGMA"-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("(  AN-CI\"")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("x<>y  L.R.")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME(")  1/x")

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_NAME("SUM  x" UTF8_PRIME)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")

	PORT_START("IN.5") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_NAME("FV")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("N")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME("PMT")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("%i")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_NAME("PV")

	PORT_START("IN.6") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("EXC  x" UTF8_PRIME)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.7") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("On/C") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("2nd")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("x" UTF8_POW_2"  " UTF8_SQUAREROOT"x")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_NAME("ln(x)  e" UTF8_POW_X)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
INPUT_PORTS_END

void ti30_state::ti30(machine_config &config)
{
	// basic machine hardware
	TMS0980(config, m_maincpu, 400000); // guessed
	m_maincpu->k().set(FUNC(ti30_state::read_k));
	m_maincpu->o().set(FUNC(ti30_state::write_o));
	m_maincpu->r().set(FUNC(ti30_state::write_r));
	m_maincpu->power_off().set(FUNC(ti30_state::auto_power_off));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1fe, 0xff);
	m_display->set_segmask(0x001, 0xe2); // 1st digit only has segments B,F,G,DP
	config.set_default_layout(layout_ti30);

	// no sound!
}

// roms

ROM_START( ti30 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "tmc0981nl", 0x0000, 0x1000, CRC(41298a14) SHA1(06f654c70add4044a612d3a38b0c2831c188fd0c) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_ti30_output.pla", 0, 352, CRC(00475f99) SHA1(70e04c1472639bd35d4adaab0b9f1ae4a0e394be) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( tibusan )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "tmc0982nl", 0x0000, 0x1000, CRC(6954560a) SHA1(6c153a0c9239a811e3514a43d809964c06f8f88e) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_common1_micro.pla", 0, 1982, CRC(3709014f) SHA1(d28ee59ded7f3b9dc3f0594a32a98391b6e9c961) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_ti30_output.pla", 0, 352, CRC(00475f99) SHA1(70e04c1472639bd35d4adaab0b9f1ae4a0e394be) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END

ROM_START( tiprog )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "za0675nl", 0x0000, 0x1000, CRC(82355854) SHA1(03fab373bce04df8ea3fe25352525e8539213626) ) // tmc0983

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 1982, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0980_tiprog_micro.pla", 0, 1982, CRC(57043284) SHA1(0fa06d5865830ecdb3d870271cb92ac917bed3ca) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_tiprog_output.pla", 0, 352, BAD_DUMP CRC(125c4ee6) SHA1(b8d865c42fd37c3d9b92c5edbecfc831be558597) ) // corrected by hand
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common1_segment.pla", 0, 157, CRC(399aa481) SHA1(72c56c58fde3fbb657d69647a9543b5f8fc74279) )
ROM_END





/***************************************************************************

  TI-1000 (1977 version)
  * TMS1990 MCU label TMC1991NL (die label 1991-91A)
  * 8-digit 7seg LED display

  TI-1000 (1978 version)
  * TMS1990 MCU label TMC1992-4NL **not dumped yet

***************************************************************************/

class ti1000_state : public hh_tms1k_state
{
public:
	ti1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ti1000(machine_config &config);

private:
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void ti1000_state::write_r(u16 data)
{
	// R0-R7: select digit
	m_display->matrix(data, m_o);
}

void ti1000_state::write_o(u16 data)
{
	// O0-O3,O5(?): input mux
	// O0-O7: digit segments
	m_inp_mux = (data & 0xf) | (data >> 1 & 0x10);
	m_o = bitswap<8>(data,7,4,3,2,1,0,6,5);
}

u8 ti1000_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( ti1000 )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.3") // O3 or O4?
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("On/C") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
INPUT_PORTS_END

void ti1000_state::ti1000(machine_config &config)
{
	// basic machine hardware
	TMS1990(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(ti1000_state::read_k));
	m_maincpu->o().set(FUNC(ti1000_state::write_o));
	m_maincpu->r().set(FUNC(ti1000_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_ti1270);

	// no sound!
}

// roms

ROM_START( ti1000 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc1991nl", 0x0000, 0x0400, CRC(2da5381d) SHA1(b5dc14553db2068ed48e130e5ec9109930d8cda9) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common2_micro.pla", 0, 860, CRC(7f50ab2e) SHA1(bff3be9af0e322986f6e545b567c97d70e135c93) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_ti1000_output.pla", 0, 352, CRC(a936631e) SHA1(1f900b12a41419d6e1ffbddd5cf72be3adaa4435) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common3_segment.pla", 0, 157, CRC(b5b3153f) SHA1(e0145c729695a4f962970aee0571d42cee6f5a9e) )
ROM_END





/***************************************************************************

  TI WIZ-A-TRON
  * TMS0970 MCU label TMC0907NL ZA0379, DP0907BS (die label 0970F-07B)
  * 9-digit 7seg LED display(one custom digit)

***************************************************************************/

class wizatron_state : public hh_tms1k_state
{
public:
	wizatron_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void wizatron(machine_config &config);

protected:
	virtual void write_o(u16 data);
	virtual void write_r(u16 data);
	virtual u8 read_k();
};

// handlers

void wizatron_state::write_r(u16 data)
{
	// R0-R8: select digit

	// 3rd digit only has A and G for =, though some newer hardware revisions
	// (goes for both wizatron and lilprof) use a custom equals-sign digit here

	// 6th digit is custom(not 7seg), for math symbols, like this:
	//   \./    GAB
	//   ---     F
	//   /.\    EDC
	m_display->matrix(data, m_o);
}

void wizatron_state::write_o(u16 data)
{
	// O1-O4: input mux
	// O0-O6: digit segments A-G
	// O7: N/C
	m_inp_mux = data >> 1 & 0xf;
	m_o = data & 0x7f;
}

u8 wizatron_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( wizatron )
	PORT_START("IN.0") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.1") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.2") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.3") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
INPUT_PORTS_END

void wizatron_state::wizatron(machine_config &config)
{
	// basic machine hardware
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(wizatron_state::read_k));
	m_maincpu->o().set(FUNC(wizatron_state::write_o));
	m_maincpu->r().set(FUNC(wizatron_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1ff^8, 0x7f);
	m_display->set_segmask(8, 0x41); // equals sign
	config.set_default_layout(layout_wizatron);

	// no sound!
}

// roms

ROM_START( wizatron )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0907nl_za0379", 0x0000, 0x0400, CRC(5a6af094) SHA1(b1f27e1f13f4db3b052dd50fb08dbf9c4d8db26e) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common2_micro.pla", 0, 860, CRC(7f50ab2e) SHA1(bff3be9af0e322986f6e545b567c97d70e135c93) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_wizatron_output.pla", 0, 352, CRC(c0ee5c04) SHA1(e9fadcef688309adbe6c1c0545aac6883ce4a1ac) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END





/***************************************************************************

  TI Little Professor (1976 version)
  * TMS0970 MCU label TMS0975NL ZA0356, GP0975CS (die label 0970D-75C)
  * 9-digit 7seg LED display(one custom digit)

  The hardware is nearly identical to Wiz-A-Tron (or vice versa, since this
  one is older).

***************************************************************************/

class lilprof_state : public wizatron_state
{
public:
	lilprof_state(const machine_config &mconfig, device_type type, const char *tag) :
		wizatron_state(mconfig, type, tag)
	{ }

	void lilprof(machine_config &config);

private:
	virtual void write_o(u16 data) override;
	virtual u8 read_k() override;
};

// handlers

void lilprof_state::write_o(u16 data)
{
	// O1-O4,O7: input mux
	// O0-O6: digit segments A-G
	m_inp_mux = (data >> 1 & 0xf) | (data >> 3 & 0x10);
	m_o = data;
}

u8 lilprof_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( lilprof )
	PORT_INCLUDE( wizatron )

	PORT_MODIFY("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_DEL) PORT_NAME("Set")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Go")

	PORT_START("IN.4") // O7
	PORT_CONFNAME( 0x0f, 0x01, "Level")
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFSETTING(    0x08, "4" )
INPUT_PORTS_END

void lilprof_state::lilprof(machine_config &config)
{
	wizatron(config);

	// basic machine hardware
	m_maincpu->k().set(FUNC(lilprof_state::read_k));
	m_maincpu->o().set(FUNC(lilprof_state::write_o));
}

// roms

ROM_START( lilprof )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms0975nl_za0356", 0x0000, 0x0400, CRC(fef9dd39) SHA1(5c9614c9c5092d55dabeee2d6e0387d50d6ad4d5) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common1_micro.pla", 0, 860, CRC(6ff5d51d) SHA1(59d3e5de290ba57694068ddba78d21a0c1edf427) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_lilprof_output.pla", 0, 352, CRC(73f9ca93) SHA1(9d6c559e2c1886c62bcd57e3c3aa897e8829b4d2) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END





/***************************************************************************

  TI Little Professor (1978 version)
  * TMS1990 MCU label TMC1993NL (die label 1990C-c3C)
  * 9-digit 7seg LED display(one custom digit)

  1978 re-release, with on/off and level select on buttons instead of
  switches. The casing was slightly revised in 1980 again, but same rom.

***************************************************************************/

class lilprof78_state : public hh_tms1k_state
{
public:
	lilprof78_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void lilprof78(machine_config &config);

private:
	void write_o(u16 data);
	void write_r(u16 data);
	u8 read_k();
};

// handlers

void lilprof78_state::write_r(u16 data)
{
	// update leds state
	u8 seg = bitswap<8>(m_o,7,4,3,2,1,0,6,5) & 0x7f;
	u16 r = (data & 7) | (data << 1 & 0x1f0);
	m_display->matrix(r, seg);

	// 3rd digit A/G(equals sign) is from O7
	m_display->write_row(3, (r != 0 && m_o & 0x80) ? 0x41 : 0);

	// 6th digit is a custom 7seg for math symbols (see wizatron_state write_r)
	m_display->write_row(6, bitswap<8>(m_display->read_row(6),7,6,1,4,2,3,5,0));
}

void lilprof78_state::write_o(u16 data)
{
	// O0-O3,O5(?): input mux
	// O0-O6: digit segments A-G
	// O7: 6th digit
	m_inp_mux = (data & 0xf) | (data >> 1 & 0x10);
	m_o = data;
}

u8 lilprof78_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( lilprof78 )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.3") // O3 or O4?
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Set")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("IN.4") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Go")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
INPUT_PORTS_END

void lilprof78_state::lilprof78(machine_config &config)
{
	// basic machine hardware
	TMS1990(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(lilprof78_state::read_k));
	m_maincpu->o().set(FUNC(lilprof78_state::write_o));
	m_maincpu->r().set(FUNC(lilprof78_state::write_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1ff, 0x7f);
	config.set_default_layout(layout_wizatron);

	// no sound!
}

// roms

ROM_START( lilprof78 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc1993nl", 0x0000, 0x0400, CRC(e941316b) SHA1(7e1542045d1e731cea81a639c9ac9e91bb233b15) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common1_instr.pla", 0, 782, CRC(05306ef8) SHA1(60a0a3c49ce330bce0c27f15f81d61461d0432ce) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_common2_micro.pla", 0, 860, CRC(7f50ab2e) SHA1(bff3be9af0e322986f6e545b567c97d70e135c93) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_lilprof78_output.pla", 0, 352, CRC(b7416cc0) SHA1(57891ffeaf34aafe8a915086c6d2feb78f5113af) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common3_segment.pla", 0, 157, CRC(b5b3153f) SHA1(e0145c729695a4f962970aee0571d42cee6f5a9e) )
ROM_END





/***************************************************************************

  TI-1680, TI-2550-IV
  * TMS1980 MCU label TMC1981NL (die label 1980A 81F)
  * TMC0999NL 256x4 RAM (die label 0999B)
  * 9-digit cyan VFD display(leftmost digit is custom)

  The extra RAM is for scrolling back through calculations. For some reason,
  TI-2550-IV has the same hardware, this makes it very different from II and III.

***************************************************************************/

class ti1680_state : public hh_tms1k_state
{
public:
	ti1680_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_ram(*this, "ram")
	{ }

	void ti1680(machine_config &config);

private:
	required_device<tmc0999_device> m_ram;

	virtual void update_display();
	virtual void write_o(u16 data);
	virtual void write_r(u16 data);
	virtual u8 read_k();
};

// handlers

void ti1680_state::update_display()
{
	// note the extra segments on R9 and R3
	m_display->matrix(m_r & 0x1ff, m_o | (m_r >> 2 & 0x80) | (m_r << 5 & 0x100));
}

void ti1680_state::write_r(u16 data)
{
	// R8,R0,R5,R6: TMC0999 data inputs
	// R2,R4+R1/R7: TMC0999 control pins
	m_ram->di_w(bitswap<4>(data,8,0,5,6));
	m_ram->adr_w(BIT(data, 2));
	m_ram->wr_w(BIT(data, 4) & BIT(data, 7));
	m_ram->rd_w(BIT(data, 4) & BIT(data, 1));

	// R3-R8: input mux
	m_inp_mux = data >> 3 & 0x3f;

	// R0-R8: select digit
	// R9: digit DP segment
	m_r = data;
	update_display();
}

void ti1680_state::write_o(u16 data)
{
	// O0-O6: digit segments A-G
	m_o = bitswap<8>(data,7,1,6,5,4,3,2,0) & 0x7f;
	update_display();
}

u8 ti1680_state::read_k()
{
	// K: multiplexed inputs, RAM data
	return read_inputs(6) | m_ram->do_r();
}

// config

static INPUT_PORTS_START( ti1680 )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("M")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("MR")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("BST")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("RPL")

	PORT_START("IN.1") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_DEL) PORT_NAME("On/C") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.2") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.3") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.4") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.5") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
INPUT_PORTS_END

void ti1680_state::ti1680(machine_config &config)
{
	// basic machine hardware
	TMS1980(config, m_maincpu, 300000); // approximation
	m_maincpu->k().set(FUNC(ti1680_state::read_k));
	m_maincpu->o().set(FUNC(ti1680_state::write_o));
	m_maincpu->r().set(FUNC(ti1680_state::write_r));
	m_maincpu->power_off().set(FUNC(ti1680_state::auto_power_off));

	TMC0999(config, m_ram);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 9);
	m_display->set_segmask(0x1fe, 0xff);
	config.set_default_layout(layout_ti1680);

	// no sound!
}

// roms

ROM_START( ti1680 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "tmc1981nl", 0x0000, 0x1000, CRC(8467c5a1) SHA1(d3a48f6c9dd41e2dc0e0cfafa84dac8d21aacaa3) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "maincpu:opla", 0 )
	ROM_LOAD( "tms1980_ti1680_output.pla", 0, 525, CRC(188af340) SHA1(eaf707266dde0d708870ef5d8f985ce88d35b43e) )
ROM_END





/***************************************************************************

  TI DataMan
  * TMS1980 MCU label TMC1982NL (die label 1980A 82B)
  * 10-digit cyan VFD display(3 digits are custom)

***************************************************************************/

class dataman_state : public hh_tms1k_state
{
public:
	dataman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void dataman(machine_config &config);

protected:
	virtual void update_display();
	virtual void write_o(u16 data);
	virtual void write_r(u16 data);
	virtual u8 read_k();
};

// handlers

void dataman_state::update_display()
{
	// note the extra segment on R9
	m_display->matrix(m_r & 0x1ff, m_o | (m_r >> 2 & 0x80));
}

void dataman_state::write_r(u16 data)
{
	// R0-R4: input mux
	// R0-R8: select digit
	// R9: =(equals sign) segment
	m_r = m_inp_mux = data;
	update_display();
}

void dataman_state::write_o(u16 data)
{
	// O0-O6: digit segments A-G
	m_o = bitswap<8>(data,7,1,6,5,4,3,2,0) & 0x7f;
	update_display();
}

u8 dataman_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[5]->read() | read_inputs(5);
}

// config

static INPUT_PORTS_START( dataman )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Memory Bank")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Go")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Force Out")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Number Guesser")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Wipe Out")

	// note: even though power buttons are on the matrix, they are not CPU-controlled
	PORT_START("IN.5") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_U) PORT_NAME("On/User Entry") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, false)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("?")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Electro Flash")
INPUT_PORTS_END

void dataman_state::dataman(machine_config &config)
{
	// basic machine hardware
	TMS1980(config, m_maincpu, 300000); // patent says 300kHz
	m_maincpu->k().set(FUNC(dataman_state::read_k));
	m_maincpu->o().set(FUNC(dataman_state::write_o));
	m_maincpu->r().set(FUNC(dataman_state::write_r));
	m_maincpu->power_off().set(FUNC(dataman_state::auto_power_off));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0x7f);
	config.set_default_layout(layout_dataman);

	// no sound!
}

// roms

ROM_START( dataman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "tmc1982nl", 0x0000, 0x1000, CRC(3521f53f) SHA1(c46fe7fe20715fdf5aed65833fb867cfd3938062) ) // matches patent US4340374

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "maincpu:opla", 0 )
	ROM_LOAD( "tms1980_dataman_output.pla", 0, 525, CRC(5fc6f451) SHA1(11475c785c34eab5b13c5dc67f413c709cd4bd4d) )
ROM_END





/***************************************************************************

  TI Math Marvel
  * TMS1980 MCU label TMC1986A-NL (die label 1980A 86A)
  * 9-digit cyan VFD display(2 digits are custom), 1-bit sound

  This is the same hardware as DataMan, with R8 connected to a piezo.

***************************************************************************/

class mathmarv_state : public dataman_state
{
public:
	mathmarv_state(const machine_config &mconfig, device_type type, const char *tag) :
		dataman_state(mconfig, type, tag)
	{ }

	void mathmarv(machine_config &config);

private:
	virtual void write_r(u16 data) override;
};

// handlers

void mathmarv_state::write_r(u16 data)
{
	// R8: speaker out
	m_speaker->level_w(data >> 8 & 1);

	// rest is same as dataman
	dataman_state::write_r(data);
}

// config

static INPUT_PORTS_START( mathmarv )
	PORT_INCLUDE( dataman )

	PORT_MODIFY("IN.4") // R4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Quest")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Checker")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Review")

	PORT_MODIFY("IN.5") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_N) PORT_NAME("On/Numberific") PORT_CHANGED_MEMBER(DEVICE_SELF, hh_tms1k_state, power_button, true)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Zap")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Flash")
INPUT_PORTS_END

void mathmarv_state::mathmarv(machine_config &config)
{
	dataman(config);

	// basic machine hardware
	m_maincpu->r().set(FUNC(mathmarv_state::write_r));

	config.set_default_layout(layout_mathmarv);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mathmarv )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD16_WORD( "tmc1986anl", 0x0000, 0x1000, CRC(79fda72d) SHA1(137852b29d9136459f78e29e7810195a956a5903) )

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_common1_instr.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_common2_micro.pla", 0, 2127, CRC(86737ac1) SHA1(4aa0444f3ddf88738ea74aec404c684bf54eddba) )
	ROM_REGION( 525, "maincpu:opla", 0 )
	ROM_LOAD( "tms1980_mathmarv_output.pla", 0, 525, CRC(5fc6f451) SHA1(11475c785c34eab5b13c5dc67f413c709cd4bd4d) )
ROM_END





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
	timaze_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void timaze(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void timaze_state::write_r(u16 data)
{
	// R0: input mux
	m_inp_mux = data & 1;
}

void timaze_state::write_o(u16 data)
{
	// O3210: 7seg EGCD?
	m_display->matrix(1, bitswap<8>(data, 7,1,6,0,3,2,5,4));
}

u8 timaze_state::read_k()
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
	// basic machine hardware
	TMS1000(config, m_maincpu, 200000); // approximation - RC osc. R=80K, C=27pF
	m_maincpu->k().set(FUNC(timaze_state::read_k));
	m_maincpu->r().set(FUNC(timaze_state::write_r));
	m_maincpu->o().set(FUNC(timaze_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 8);
	m_display->set_segmask(1, 0x5c);
	config.set_default_layout(layout_timaze);

	// no sound!
}

// roms

ROM_START( timaze )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "gb2040172a", 0x0000, 0x0400, CRC(0bab4dc6) SHA1(c9d40649fbb27a8b7cf7460d66c7e217b63376f0) ) // from patent GB2040172A, verified with source code

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, BAD_DUMP CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) ) // not in patent, use default one
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_timaze_output.pla", 0, 365, BAD_DUMP CRC(f0f36970) SHA1(a6ad1f5e804ac98e5e1a1d07466b3db3a8d6c256) ) // described in patent, but unsure about pin order
ROM_END





/***************************************************************************

  Texas Instruments Electronic Digital Thermostat
  * TMS0970 MCU, label TMS0970NLL TMC0910B (die label 0970F-10E)
  * 9-digit 7seg LED display, only 4 used
  * temperature sensor, heat/cool/fan outputs

  This is a thermostat and digital clock. It's the 2nd one described in
  patents US4388692 and US4298946.

***************************************************************************/

class tithermos_state : public hh_tms1k_state
{
public:
	tithermos_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_60hz(*this, "ac_line")
	{ }

	void tithermos(machine_config &config);

private:
	required_device<clock_device> m_60hz;

	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void tithermos_state::write_r(u16 data)
{
	// D1-D4: select digit
	m_display->matrix(data, m_o);

	// D6: heat/cool
	// D8: A/D reset
	m_r = data;
}

void tithermos_state::write_o(u16 data)
{
	// SA-SP: input mux
	// SA-SG: digit segments
	m_o = m_inp_mux = data;
}

u8 tithermos_state::read_k()
{
	// K: multiplexed inputs
	u8 data = read_inputs(8);

	// when SB/SD/SP is high:
	if (m_inp_mux & 0x8a)
	{
		// K1: 60hz from AC line
		// K2: battery low?
		// K8: A/D output (TODO)
		data |= (m_60hz->signal_r()) ? 1 : 0;
	}

	return data;
}

// config

static INPUT_PORTS_START( tithermos )
	PORT_START("IN.0") // SA
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Temp Row PM 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Time Row PM 2")

	PORT_START("IN.1") // SB
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // SC
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Temp Row PM 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Time Row PM 1")

	PORT_START("IN.3") // SD
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) // AC line
	PORT_CONFNAME( 0x06, 0x04, "System")
	PORT_CONFSETTING(    0x04, "Heat" )
	PORT_CONFSETTING(    0x00, "Off" )
	PORT_CONFSETTING(    0x02, "Cool" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // A/D output

	PORT_START("IN.4") // SE
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Temp Row AM 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Time Row AM 2")

	PORT_START("IN.5") // SF
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Temp Row AM 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Time Row AM 1")

	PORT_START("IN.6") // SG
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Temp / Actual")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Time / Clock")

	PORT_START("IN.7") // SP
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) // AC line
	PORT_CONFNAME( 0x06, 0x04, "Mode")
	PORT_CONFSETTING(    0x04, "Constant" )
	PORT_CONFSETTING(    0x00, "Day/Night" )
	PORT_CONFSETTING(    0x02, "Night" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // A/D output

	PORT_START("IN.8")
	PORT_CONFNAME( 0x01, 0x00, "Fan")
	PORT_CONFSETTING(    0x00, "On" )
	PORT_CONFSETTING(    0x01, "Auto" ) // same output as heat/cool
INPUT_PORTS_END

void tithermos_state::tithermos(machine_config &config)
{
	// basic machine hardware
	TMS0970(config, m_maincpu, 250000); // approximation
	m_maincpu->k().set(FUNC(tithermos_state::read_k));
	m_maincpu->r().set(FUNC(tithermos_state::write_r));
	m_maincpu->o().set(FUNC(tithermos_state::write_o));

	CLOCK(config, "ac_line", 60);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_tithermos);

	// no sound!
}

// roms

ROM_START( tithermos )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0910b", 0x0000, 0x0400, CRC(232011cf) SHA1(598ae8cecd98226fb5056c99ce9f47fd23785fd7) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_common2_instr.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_tithermos_micro.pla", 0, 860, CRC(a3bb8ca5) SHA1(006ea733440001c37a77d4ffbc4bd2a8fee212ac) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0980_tithermos_output.pla", 0, 352, CRC(eb3598fd) SHA1(2372ae17fb982cae2710bf8c348bf02b767daabd) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0980_common2_segment.pla", 0, 157, CRC(c03cccd8) SHA1(08bc4b597686a7aa8b2c9f43b85b62747ffd455b) )
ROM_END





/***************************************************************************

  Tiger Sub Wars (model 7-490)
  * PCB label CSG201A(main), CSG201B(leds)
  * TMS1200N2LL MP3352 (die label 1000C, MP3352)
  * 4-digit 7seg LED display + 55 other LEDs, 1-bit sound

  Tiger/Yeno also published an LCD handheld called Sub Wars, it's not related.

  This handheld was modified and used as a prop in the 1981 movie Escape from
  New York, Snake Plissken uses it as a homing device.

***************************************************************************/

class subwars_state : public hh_tms1k_state
{
public:
	subwars_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void subwars(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
};

// handlers

void subwars_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void subwars_state::write_r(u16 data)
{
	// R0-R3: digit select
	// R4-R12: led select
	m_r = data;
	update_display();
}

void subwars_state::write_o(u16 data)
{
	// O0-O6: led data
	m_o = data;
	update_display();

	// O7: speaker out
	m_speaker->level_w(BIT(data, 7));
}

// config

static INPUT_PORTS_START( subwars )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void subwars_state::subwars(machine_config &config)
{
	// basic machine hardware
	TMS1200(config, m_maincpu, 550000); // approximation - RC osc. R=24K, C=47pF
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(subwars_state::write_r));
	m_maincpu->o().set(FUNC(subwars_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(13, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_subwars);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( subwars )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp3352", 0x0000, 0x0400, CRC(5dece1e4) SHA1(65ef77b063c94ff4b6c83dace54ea2f75bd3d6a9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common1_micro.pla", 0, 867, CRC(4becec19) SHA1(3c8a9be0f00c88c81f378b76886c39b10304f330) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_subwars_output.pla", 0, 365, CRC(372b9bbc) SHA1(06a875e114b7757c6f4f1727416d1739ebe60931) )
ROM_END





/***************************************************************************

  Tiger Playmaker: Hockey, Soccer, Basketball (model 7-540 or 7-540A)
  * TMS1100 MP1215 (die label 1100B MP1215)
  * 2-digit 7seg LED display + 40 other LEDs, 1-bit sound

  The games are on playcards(Tiger calls them that), the hardware detects which
  game is inserted from a notch at the lower-right. The playcards also function
  as an overlay. MAME external artwork is needed for those.

  Booting the handheld with no playcard inserted will initiate a halftime show.

  "Playmaker" is actually Tiger's trademark for the d-pad controller, this
  controller term was also used in for example Deluxe Football, and 7 in 1 Sports
  Stadium. The d-pad has a ball shape at the bottom that sits on a concave base.
  It is patented under US4256931 (mid-1979, a couple of years before Nintendo's
  Game & Watch d-pad with US4687200).

***************************************************************************/

class playmaker_state : public hh_tms1k_state
{
public:
	playmaker_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void playmaker(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	u8 m_notch = 0; // cartridge K1/K2

	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void playmaker_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// register for savestates
	save_item(NAME(m_notch));
}

// handlers

DEVICE_IMAGE_LOAD_MEMBER(playmaker_state::cart_load)
{
	if (!image.loaded_through_softlist())
	{
		image.seterror(image_error::UNSUPPORTED, "Can only load through softwarelist");
		return image_init_result::FAIL;
	}

	// get cartridge notch
	const char *notch = image.get_feature("notch");
	m_notch = notch ? strtoul(notch, nullptr, 0) & 3 : 0;

	return image_init_result::PASS;
}

void playmaker_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void playmaker_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R3: input mux
	m_inp_mux = data & 0xf;

	// R0-R7: led select
	// R8,R9: digit select
	m_r = data;
	update_display();
}

void playmaker_state::write_o(u16 data)
{
	// O0-O6: led data
	m_o = data;
	update_display();
}

u8 playmaker_state::read_k()
{
	// K: multiplexed inputs, cartridge notch from R3
	return read_inputs(3) | ((m_inp_mux & 8) ? m_notch : 0);
}

// config

static INPUT_PORTS_START( playmaker )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Shoot / P1 Skill")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pass / P2 Skill")
INPUT_PORTS_END

void playmaker_state::playmaker(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=20K, C=250pF
	m_maincpu->k().set(FUNC(playmaker_state::read_k));
	m_maincpu->r().set(FUNC(playmaker_state::write_r));
	m_maincpu->o().set(FUNC(playmaker_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 7);
	m_display->set_segmask(0x300, 0x7f);
	m_display->set_bri_levels(0.004, 0.04); // player 1 leds are brighter
	config.set_default_layout(layout_playmaker);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	// cartridge
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "playmaker_cart"));
	cartslot.set_must_be_loaded(false);
	cartslot.set_device_load(FUNC(playmaker_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("playmaker");
}

// roms

ROM_START( playmaker )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1215", 0x0000, 0x0800, CRC(bfc7b6c8) SHA1(33f6e2b86fae2fd9e4b0a4b8dc842c257ca3047d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_playmaker_output.pla", 0, 365, CRC(0cd484d6) SHA1(4a9af9f3d18af504145690cb0f6444ff1aef26ca) )
ROM_END





/***************************************************************************

  Tiger Deluxe Football with Instant Replay (model 7-550)
  * TMS1400NLL MP7302 (die label TMS1400 MP7302)
  * 4-digit 7seg LED display, 80 red/green LEDs, 1-bit sound

  According to the manual, player 1 is green, player 2 is red. But when
  playing a 1-player game, the CPU controls green, so on MAME, player 1
  is the red side.

  Booting the handheld with the Score and Replay buttons held down will
  initiate a halftime show.

***************************************************************************/

class dxfootb_state : public hh_tms1k_state
{
public:
	dxfootb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void dxfootb(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void dxfootb_state::update_display()
{
	// 2 led groups (double multiplexed)
	u16 g1 = (m_r & 0x100) ? 0x7f : 0;
	u16 g2 = (m_r & 0x80) ? 0x7f << 7 : 0;
	m_display->matrix((m_r & g1) | (m_r << 7 & g2), m_o);
}

void dxfootb_state::write_r(u16 data)
{
	// R9,R10: speaker out
	m_speaker->level_w(data >> 9 & 3);

	// R3-R6: input mux
	m_inp_mux = data >> 3 & 0xf;

	// R0-R6: led select
	// R7,R8: group select
	m_r = data;
	update_display();
}

void dxfootb_state::write_o(u16 data)
{
	// O0-O7: led data
	m_o = data;
	update_display();
}

u8 dxfootb_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( dxfootb )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Replay / Skill (Green)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Score / Skill (Red)")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Kick")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass")

	PORT_START("IN.2") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("IN.3") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
INPUT_PORTS_END

void dxfootb_state::dxfootb(machine_config &config)
{
	// basic machine hardware
	TMS1400(config, m_maincpu, 425000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(dxfootb_state::read_k));
	m_maincpu->r().set(FUNC(dxfootb_state::write_r));
	m_maincpu->o().set(FUNC(dxfootb_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(7+7, 8);
	m_display->set_segmask(0x3c00, 0x7f);
	config.set_default_layout(layout_dxfootb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( dxfootb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7302", 0x0000, 0x1000, CRC(a8077062) SHA1(c1318fe5c8f2db021d7d1264fc70158944045fa3) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_dxfootb_output.pla", 0, 557, CRC(a1b3d2c0) SHA1(8030e6dcd3878b58668c98cff36d93b764e1d67f) )
ROM_END





/***************************************************************************

  Tiger Electronics Copy Cat (model 7-520)
  * PCB label CC REV B
  * TMS1000 MCU, label 69-11513 MP0919 (die label MP0919)
  * 4 LEDs, 1-bit sound

  known releases:
  - World: Copy Cat, published by Tiger
  - USA(1): Follow Me, published by Sears
  - USA(2): Electronic Repeat, published by Tandy

***************************************************************************/

class copycat_state : public hh_tms1k_state
{
public:
	copycat_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void copycat(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void copycat_state::write_r(u16 data)
{
	// R0-R3: leds
	m_display->matrix(1, data & 0xf);

	// R4-R7: input mux
	// R8-R10: N/C
	m_inp_mux = data >> 4 & 0xf;
}

void copycat_state::write_o(u16 data)
{
	// O0,O1: speaker out
	// O2,O7: N/C, O3-O6: tied together but unused
	m_speaker->level_w(data & 3);
}

u8 copycat_state::read_k()
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

void copycat_state::copycat(machine_config &config)
{
	// basic machine hardware
	TMS1000(config, m_maincpu, 320000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(copycat_state::read_k));
	m_maincpu->r().set(FUNC(copycat_state::write_r));
	m_maincpu->o().set(FUNC(copycat_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_copycat);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( copycat )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0919", 0x0000, 0x0400, CRC(92a21299) SHA1(16daadb8dbf53aaab8a71833017b4a578d035d6d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_copycat_output.pla", 0, 365, CRC(b1d0c96d) SHA1(ac1a003eab3f69e09e9050cb24ea17211e0523fe) )
ROM_END





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
	copycatm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void copycatm2(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
};

// handlers

void copycatm2_state::write_r(u16 data)
{
	// R1-R4: leds
	m_display->matrix(1, data >> 1 & 0xf);
}

void copycatm2_state::write_o(u16 data)
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
	// basic machine hardware
	TMS1730(config, m_maincpu, 275000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(copycatm2_state::write_r));
	m_maincpu->o().set(FUNC(copycatm2_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_copycatm2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( copycatm2 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "mp3005n", 0x0000, 0x0200, CRC(a87649cb) SHA1(14ef7967a80578885f0b905772c3bb417b5b3255) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_copycatm2_micro.pla", 0, 867, CRC(2710d8ef) SHA1(cb7a13bfabedad43790de753844707fe829baed0) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_copycatm2_output.pla", 0, 365, CRC(d1999aaf) SHA1(0c27789b349e491d5230f9c75c4741e621f5a14e) )
ROM_END





/***************************************************************************

  Tiger Ditto (model 7-530)
  * TMS1700 MCU, label MP1801-N2LL (die label 1700 MP1801)
  * 4 LEDs, 1-bit sound

  known releases:
  - World: Ditto, published by Tiger
  - USA: Electronic Pocket Repeat (model 60-2152/60-2468A), published by Tandy
    note: 1996 model 60-2482 MCU is a Z8, and is assumed to be a clone of Tiger Copycat Jr.

***************************************************************************/

class ditto_state : public hh_tms1k_state
{
public:
	ditto_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ditto(machine_config &config);

private:
	void write_r(u16 data);
	void write_o(u16 data);
};

// handlers

void ditto_state::write_r(u16 data)
{
	// R0-R3: leds
	m_display->matrix(1, data & 0xf);
}

void ditto_state::write_o(u16 data)
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
	// basic machine hardware
	TMS1700(config, m_maincpu, 275000); // approximation - RC osc. R=100K, C=47pF
	m_maincpu->k().set_ioport("IN.0");
	m_maincpu->r().set(FUNC(ditto_state::write_r));
	m_maincpu->o().set(FUNC(ditto_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 4);
	config.set_default_layout(layout_ditto);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[4] = { 0.0, 1.0, -1.0, 0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ditto )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "mp1801", 0x0000, 0x0200, CRC(cee6043b) SHA1(4ec334be6835688413637ff9d9d7a5f0d61eba27) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ditto_micro.pla", 0, 867, CRC(2710d8ef) SHA1(cb7a13bfabedad43790de753844707fe829baed0) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ditto_output.pla", 0, 365, CRC(2b708a27) SHA1(e95415e51ffbe5da3bde1484fcd20467dde9f09a) )
ROM_END





/***************************************************************************

  Tiger 7 in 1 Sports Stadium (model 7-555)
  * TMS1400 MP7304 (die label TMS1400 MP7304A)
  * 2x2-digit 7seg LED display + 39 other LEDs, 1-bit sound

  This handheld includes 7 games: 1: Basketball, 2: Hockey, 3: Soccer,
  4: Maze, 5: Baseball, 6: Football, 7: Raquetball.
  MAME external artwork is needed for the switchable overlays.

  known releases:
  - World: 7 in 1 Sports Stadium, published by Tiger
  - USA: 7 in 1 Sports, published by Sears

***************************************************************************/

class t7in1ss_state : public hh_tms1k_state
{
public:
	t7in1ss_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void t7in1ss(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void t7in1ss_state::update_display()
{
	m_display->matrix(m_r, m_o);
}

void t7in1ss_state::write_r(u16 data)
{
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R0-R2,R10: input mux
	m_inp_mux = (data & 7) | (data >> 7 & 8);

	// R0-R3: digit select
	// R4-R8: led select
	m_r = data;
	update_display();
}

void t7in1ss_state::write_o(u16 data)
{
	// O0-O7: led data
	m_o = data;
	update_display();
}

u8 t7in1ss_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( t7in1ss )
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

void t7in1ss_state::t7in1ss(machine_config &config)
{
	// basic machine hardware
	TMS1400(config, m_maincpu, 425000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(t7in1ss_state::read_k));
	m_maincpu->r().set(FUNC(t7in1ss_state::write_r));
	m_maincpu->o().set(FUNC(t7in1ss_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0xf, 0x7f);
	m_display->set_bri_levels(0.004, 0.04); // player led is brighter
	config.set_default_layout(layout_t7in1ss);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( t7in1ss )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7304", 0x0000, 0x1000, CRC(2a1c8390) SHA1(fa10e60686af6828a61f05046abc3854ab49af95) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_t7in1ss_output.pla", 0, 557, CRC(6b7660f7) SHA1(bb7d58fa04e7606ccdf5b209e1b089948bdd1e7c) )
ROM_END





/***************************************************************************

  Tomy(tronics) Break Up (manufactured in Japan)
  * PCB label TOMY B.O.
  * TMS1040 MP2726 TOMY WIPE (die label MP2726A)
  * TMS1025N2LL I/O expander
  * 2-digit 7seg display, 46 other leds, 1-bit sound

  known releases:
  - USA: Break Up, published by Tomy
  - Japan: Block Attack, published by Tomy
  - UK: Break-In, published by Tomy

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
	tbreakup_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag),
		m_expander(*this, "expander")
	{ }

	void tbreakup(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(skill_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	required_device<tms1025_device> m_expander;
	u8 m_exp_port[7] = { };
	void expander_w(offs_t offset, u8 data);

	void set_clock();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void tbreakup_state::machine_start()
{
	hh_tms1k_state::machine_start();
	save_item(NAME(m_exp_port));
}

void tbreakup_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	set_clock();
}

// handlers

void tbreakup_state::set_clock()
{
	// MCU clock is from an analog circuit with resistor of 73K, PRO2 adds 100K
	m_maincpu->set_unscaled_clock((m_inputs[3]->read() & 1) ? 500000 : 325000);
}

void tbreakup_state::update_display()
{
	// 7seg leds from R0,R1 and O0-O6
	m_display->matrix_partial(0, 2, m_r, m_o & 0x7f);

	// 22 round leds from O2-O7 and expander port 7
	m_display->matrix_partial(2, 6, m_o >> 2, m_exp_port[6]);

	// 24 rectangular leds from expander ports 1-6 (not strobed)
	for (int y = 0; y < 6; y++)
		m_display->write_row(y+8, m_exp_port[y]);
}

void tbreakup_state::expander_w(offs_t offset, u8 data)
{
	// TMS1025 port 1-7 data
	m_exp_port[offset] = data;
}

void tbreakup_state::write_r(u16 data)
{
	// R6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// R7,R8: input mux
	m_inp_mux = data >> 7 & 3;

	// R3-R5: TMS1025 port S
	// R2: TMS1025 STD pin
	m_expander->write_s(data >> 3 & 7);
	m_expander->write_std(data >> 2 & 1);

	// R0,R1: select digit
	m_r = ~data;
	update_display();
}

void tbreakup_state::write_o(u16 data)
{
	// O0-O3: TMS1025 port H
	m_expander->write_h(data & 0xf);

	// O0-O7: led state
	m_o = data;
	update_display();
}

u8 tbreakup_state::read_k()
{
	// K4: fixed input
	// K8: multiplexed inputs
	return (m_inputs[2]->read() & 4) | (read_inputs(2) & 8);
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
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, tbreakup_state, skill_switch, 0)
	PORT_CONFSETTING(    0x00, "Pro 1" )
	PORT_CONFSETTING(    0x01, "Pro 2" )
INPUT_PORTS_END

void tbreakup_state::tbreakup(machine_config &config)
{
	// basic machine hardware
	TMS1040(config, m_maincpu, 325000); // see set_clock
	m_maincpu->k().set(FUNC(tbreakup_state::read_k));
	m_maincpu->r().set(FUNC(tbreakup_state::write_r));
	m_maincpu->o().set(FUNC(tbreakup_state::write_o));

	TMS1025(config, m_expander).set_ms(1); // MS tied high
	m_expander->write_port1_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port2_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port3_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port4_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port5_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port6_callback().set(FUNC(tbreakup_state::expander_w));
	m_expander->write_port7_callback().set(FUNC(tbreakup_state::expander_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+6+6, 8);
	m_display->set_segmask(3, 0x7f);
	config.set_default_layout(layout_tbreakup);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbreakup )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp2726a", 0x0000, 0x0400, CRC(1f7c28e2) SHA1(164cda4eb3f0b1d20955212a197c9aadf8d18a06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_tbreakup_output.pla", 0, 365, CRC(a1ea035e) SHA1(fcf0b57ed90b41441a8974223a697f530daac0ab) )
ROM_END





/***************************************************************************

  Tomy Power House Pinball
  * PCB label TOMY P-B
  * TMS1100 MP1180 TOMY PINB (die label MP1180)
  * 3 7seg LEDs, and other LEDs behind bezel, 1-bit sound

  known releases:
  - USA: Power House Pinball, published by Tomy
  - Japan: Pinball, published by Tomy
  - Europe: Flipper, published by Tomy

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
	phpball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void phpball(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(flipper_button) { update_display(); }

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void phpball_state::update_display()
{
	// rectangular LEDs under LEDs D,F and E,G are directly connected
	// to the left and right flipper buttons - output them to 10.a and 9.a
	u16 in1 = m_inputs[1]->read() << 7 & 0x600;
	m_display->matrix((m_r & 0x1ff) | in1, m_o);
}

void phpball_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R9: input mux
	m_inp_mux = data >> 9 & 1;

	// R0-R2: digit select
	// R0-R8: led select
	m_r = data;
	update_display();
}

void phpball_state::write_o(u16 data)
{
	// O0-O6: digit segment/led data
	// O7: N/C
	m_o = data & 0x7f;
	update_display();
}

u8 phpball_state::read_k()
{
	// K: multiplexed inputs (note: the Vss row is always on)
	return m_inputs[1]->read() | read_inputs(1);
}

// config

static INPUT_PORTS_START( phpball )
	PORT_START("IN.0") // R9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Plunger")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // Vss!
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper") PORT_CHANGED_MEMBER(DEVICE_SELF, phpball_state, flipper_button, 0)
INPUT_PORTS_END

void phpball_state::phpball(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(phpball_state::read_k));
	m_maincpu->r().set(FUNC(phpball_state::write_r));
	m_maincpu->o().set(FUNC(phpball_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(11, 7);
	m_display->set_segmask(7, 0x7f);
	config.set_default_layout(layout_phpball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( phpball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1180", 0x0000, 0x0800, CRC(2163b92d) SHA1(bc53d1911e88b4e89d951c6f769703105c13389c) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_phpball_output.pla", 0, 365, CRC(87e67aaf) SHA1(ebc7bae1352f39173f1bf0dc10cdc6f635dedab4) )
ROM_END





/***************************************************************************

  U.S. Games Super Sports-4
  * TMS1100 MP1219 (no decap)
  * 4 7seg LEDs, 49 other LEDs, 1-bit sound

  The game is very similar to t3in1sa, even parts of the ROM match. But by
  the time that one was released (in 1983 or 1984), U.S. Games did not exist
  anymore. I suspect t3in1sa was programmed by the same (Hong Kong) company.

  This handheld includes 4 games: Basketball, Football, Soccer, Hockey.
  MAME external artwork is needed for the switchable overlays.

***************************************************************************/

class ssports4_state : public hh_tms1k_state
{
public:
	ssports4_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void ssports4(machine_config &config);

private:
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void ssports4_state::update_display()
{
	// note: R2 is an extra column
	m_display->matrix(m_r, m_o | (m_r << 6 & 0x100));
}

void ssports4_state::write_r(u16 data)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R9: led select/data
	// R0,R1 and R8,R9 are 7segs
	m_r = data;
	update_display();
}

void ssports4_state::write_o(u16 data)
{
	// O0-O7: led data
	m_o = data;
	update_display();
}

u8 ssports4_state::read_k()
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_16WAY PORT_NAME("P1 Up-Left / Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_16WAY PORT_NAME("P1 Up-Right / Info")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_16WAY PORT_NAME("P1 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_16WAY PORT_NAME("P1 O.P.") // offensive player (modifier button)

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Up-Left / Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Up-Right / Info")
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

void ssports4_state::ssports4(machine_config &config)
{
	// basic machine hardware
	TMS1100(config, m_maincpu, 350000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(ssports4_state::read_k));
	m_maincpu->r().set(FUNC(ssports4_state::write_r));
	m_maincpu->o().set(FUNC(ssports4_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 9);
	m_display->set_segmask(0x303, 0x7f);
	m_display->set_bri_levels(0.005, 0.05); // offense leds are brighter
	config.set_default_layout(layout_ssports4);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ssports4 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp1219", 0x0000, 0x0800, CRC(865c06d6) SHA1(12a625a13bdb57b82b35c42b175d38756a1e2e04) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, BAD_DUMP CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) ) // not verified
	ROM_REGION( 365, "maincpu:opla", ROMREGION_ERASE00 )
	ROM_LOAD( "tms1100_ssports4_output.pla", 0, 365, NO_DUMP )

	ROM_REGION16_LE( 0x40, "maincpu:opla_b", ROMREGION_ERASE00 ) // verified, electronic dump
	ROM_LOAD16_BYTE( "tms1100_ssports4_output.bin", 0, 0x20, CRC(cd6f162f) SHA1(3348edb697e996b5ab82be54076b9cd444e0f2b1) )
ROM_END





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
	xl25_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_tms1k_state(mconfig, type, tag)
	{ }

	void xl25(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(k4_button) { update_halt(); }

protected:
	virtual void machine_reset() override;

private:
	void update_halt();
	void update_display();
	void write_r(u16 data);
	void write_o(u16 data);
	u8 read_k();
};

void xl25_state::machine_reset()
{
	hh_tms1k_state::machine_reset();
	update_halt();
}

// handlers

void xl25_state::update_halt()
{
	// O5+K4 go to HALT pin (used when pressing store/recall button)
	bool halt = !((m_o & 0x20) || (read_k() & 4));
	m_maincpu->set_input_line(INPUT_LINE_HALT, halt ? ASSERT_LINE : CLEAR_LINE);
}

void xl25_state::update_display()
{
	m_display->matrix(m_r, m_o >> 1);
}

void xl25_state::write_r(u16 data)
{
	// R0-R9: input mux, led select
	m_inp_mux = data;
	m_r = data;
	update_display();
}

void xl25_state::write_o(u16 data)
{
	// O1-O3: led data
	m_o = data;
	update_display();

	// O6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// O5(+K4): MCU halt
	update_halt();
}

u8 xl25_state::read_k()
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
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 11") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 12") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 13") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 14") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 15") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 16")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 21")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Store / Recall") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Cross / Knight / Score") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 18")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 23")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Clear") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.8") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 19")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 24")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Random") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.9") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Square 25")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Sound") PORT_CHANGED_MEMBER(DEVICE_SELF, xl25_state, k4_button, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xl25_state::xl25(machine_config &config)
{
	// basic machine hardware
	TMS1000C(config, m_maincpu, 300000); // approximation - RC osc. R=5.6K, C=47pF
	m_maincpu->k().set(FUNC(xl25_state::read_k));
	m_maincpu->r().set(FUNC(xl25_state::write_r));
	m_maincpu->o().set(FUNC(xl25_state::write_o));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 3);
	config.set_default_layout(layout_xl25);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( xl25 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp4486a", 0x0000, 0x0400, CRC(bd84b515) SHA1(377fcc68a517260acd51eb9746cd62914a75d739) )

	ROM_REGION( 922, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000c_xl25_micro.pla", 0, 922, CRC(8823e7f2) SHA1(676b0eace9d8730f2caa9087e8c51e540c7fabf8) )
	ROM_REGION( 558, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000c_xl25_output.pla", 0, 558, CRC(06ecc6e0) SHA1(e0fa1b9388948197b4de2edd3cd02fbde1dbabbb) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME        PARENT    CMP MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, matchnum,   0,         0, matchnum,  matchnum,  matchnum_state,  empty_init, "A-One LSI", "Match Number", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, arrball,    0,         0, arrball,   arrball,   arrball_state,   empty_init, "A-One LSI", "Arrange Ball", MACHINE_SUPPORTS_SAVE )

COMP( 1980, mathmagi,   0,         0, mathmagi,  mathmagi,  mathmagi_state,  empty_init, "APF Electronics Inc.", "Mathemagician", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1979, bcheetah,   0,         0, bcheetah,  bcheetah,  bcheetah_state,  empty_init, "Bandai", "System Control Car: Cheetah", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_MECHANICAL ) // ***
CONS( 1981, tc7atc,     0,         0, tc7atc,    tc7atc,    tc7atc_state,    empty_init, "Bandai", "TC7: Air Traffic Control", MACHINE_SUPPORTS_SAVE )

COMP( 1977, palmf31,    0,         0, palmf31,   palmf31,   palmf31_state,   empty_init, "Canon", "Palmtronic F-31", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, palmmd8,    0,         0, palmmd8,   palmmd8,   palmmd8_state,   empty_init, "Canon", "Palmtronic MD-8 (Multi 8)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1978, amaztron,   0,         0, amaztron,  amaztron,  amaztron_state,  empty_init, "Coleco", "Amaze-A-Tron", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // ***
COMP( 1979, zodiac,     0,         0, zodiac,    zodiac,    zodiac_state,    empty_init, "Coleco", "Zodiac - The Astrology Computer", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
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

CONS( 1979, qfire,      0,         0, qfire,     qfire,     qfire_state,     empty_init, "Electroplay", "Quickfire", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, esoccer,    0,         0, esoccer,   esoccer,   esoccer_state,   empty_init, "Entex", "Electronic Soccer (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ebball,     0,         0, ebball,    ebball,    ebball_state,    empty_init, "Entex", "Electronic Baseball (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ebball2,    0,         0, ebball2,   ebball2,   ebball2_state,   empty_init, "Entex", "Electronic Baseball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebball3,    0,         0, ebball3,   ebball3,   ebball3_state,   empty_init, "Entex", "Electronic Baseball 3 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, esbattle,   0,         0, esbattle,  esbattle,  esbattle_state,  empty_init, "Entex", "Space Battle (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, blastit,    0,         0, blastit,   blastit,   blastit_state,   empty_init, "Entex", "Blast It", MACHINE_SUPPORTS_SAVE )
CONS( 1980, einvader,   0,         0, einvader,  einvader,  einvader_state,  empty_init, "Entex", "Space Invader (Entex, TMS1100 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, efootb4 ,   0,         0, efootb4,   efootb4,   efootb4_state,   empty_init, "Entex", "Color Football 4 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ebaskb2 ,   0,         0, ebaskb2,   ebaskb2,   ebaskb2_state,   empty_init, "Entex", "Electronic Basketball 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, raisedvl,   0,         0, raisedvl,  raisedvl,  raisedvl_state,  empty_init, "Entex", "Raise The Devil Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, ebknight,   0,         0, ebknight,  raisedvl,  raisedvl_state,  empty_init, "Entex", "Black Knight Pinball (Entex)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, f2pbball,   0,         0, f2pbball,  f2pbball,  f2pbball_state,  empty_init, "Fonas", "2 Player Baseball (Fonas)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, f3in1,      0,         0, f3in1,     f3in1,     f3in1_state,     empty_init, "Fonas", "3 in 1: Football, Basketball, Soccer", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, gpoker,     0,         0, gpoker,    gpoker,    gpoker_state,    empty_init, "Gakken", "Poker (Gakken, 1979 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gjackpot,   0,         0, gjackpot,  gjackpot,  gjackpot_state,  empty_init, "Gakken", "Jackpot: Gin Rummy & Black Jack", MACHINE_SUPPORTS_SAVE )
CONS( 1980, ginv,       0,         0, ginv,      ginv,      ginv_state,      empty_init, "Gakken", "Invader (Gakken, cyan version)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, ginv1000,   0,         0, ginv1000,  ginv1000,  ginv1000_state,  empty_init, "Gakken", "Galaxy Invader 1000", MACHINE_SUPPORTS_SAVE )
CONS( 1982, ginv2000,   0,         0, ginv2000,  ginv2000,  ginv2000_state,  empty_init, "Gakken", "Invader 2000", MACHINE_SUPPORTS_SAVE )
COMP( 1983, fxmcr165,   0,         0, fxmcr165,  fxmcr165,  fxmcr165_state,  empty_init, "Gakken", "FX-Micom R-165", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1979, elecdet,    0,         0, elecdet,   elecdet,   elecdet_state,   empty_init, "Ideal Toy Corporation", "Electronic Detective", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1979, starwbc,    0,         0, starwbc,   starwbc,   starwbc_state,   empty_init, "Kenner", "Star Wars - Electronic Battle Command", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, starwbcp,   starwbc,   0, starwbc,   starwbc,   starwbc_state,   empty_init, "Kenner", "Star Wars - Electronic Battle Command (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, liveafb,    0,         0, liveafb,   liveafb,   liveafb_state,   empty_init, "Kenner", "Live Action Football", MACHINE_SUPPORTS_SAVE )

COMP( 1979, astro,      0,         0, astro,     astro,     astro_state,     empty_init, "Kosmos", "Astro", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1978, elecbowl,   0,         0, elecbowl,  elecbowl,  elecbowl_state,  empty_init, "Marx", "Electronic Bowling (Marx)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_CONTROLS | MACHINE_MECHANICAL | MACHINE_NOT_WORKING ) // ***

COMP( 1979, horseran,   0,         0, horseran,  horseran,  horseran_state,  empty_init, "Mattel Electronics", "Thoroughbred Horse Race Analyzer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
CONS( 1980, mdndclab,   0,         0, mdndclab,  mdndclab,  mdndclab_state,  empty_init, "Mattel Electronics", "Dungeons & Dragons - Computer Labyrinth Game", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // ***

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
CONS( 1978, merlin,     0,         0, merlin,    merlin,    merlin_state,    empty_init, "Parker Brothers", "Merlin - The Electronic Wizard (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1978, merlina,    merlin,    0, merlin,    merlin,    merlin_state,    empty_init, "Parker Brothers", "Merlin - The Electronic Wizard (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1978, pbmastm,    0,         0, pbmastm,   pbmastm,   pbmastm_state,   empty_init, "Parker Brothers", "Electronic Master Mind (Parker Brothers)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW ) // ***
CONS( 1979, stopthief,  0,         0, stopthief, stopthief, stopthief_state, empty_init, "Parker Brothers", "Stop Thief - Electronic Cops and Robbers (Electronic Crime Scanner)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***
CONS( 1979, stopthiefp, stopthief, 0, stopthief, stopthief, stopthief_state, empty_init, "Parker Brothers", "Stop Thief - Electronic Cops and Robbers (Electronic Crime Scanner) (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***
CONS( 1980, bankshot,   0,         0, bankshot,  bankshot,  bankshot_state,  empty_init, "Parker Brothers", "Bank Shot - Electronic Pool", MACHINE_SUPPORTS_SAVE )
CONS( 1980, splitsec,   0,         0, splitsec,  splitsec,  splitsec_state,  empty_init, "Parker Brothers", "Split Second", MACHINE_SUPPORTS_SAVE )
CONS( 1982, mmerlin,    0,         0, mmerlin,   mmerlin,   mmerlin_state,   empty_init, "Parker Brothers", "Master Merlin", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, lostreas,   0,         0, lostreas,  lostreas,  lostreas_state,  empty_init, "Parker Brothers", "Lost Treasure - The Electronic Deep-Sea Diving Game (Electronic Dive-Control Center)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***

CONS( 1978, alphie,     0,         0, alphie,    alphie,    alphie_state,    empty_init, "Playskool", "Alphie - The Electronic Robot (patent)", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1980, tcfball,    0,         0, tcfball,   tcfball,   tcfball_state,   empty_init, "Tandy Corporation", "Championship Football (model 60-2150)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, tcfballa,   tcfball,   0, tcfballa,  tcfballa,  tcfballa_state,  empty_init, "Tandy Corporation", "Championship Football (model 60-2151)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, tandy12,    0,         0, tandy12,   tandy12,   tandy12_state,   empty_init, "Tandy Corporation", "Tandy-12: Computerized Arcade", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // some of the minigames: ***
CONS( 1982, monkeysee,  0,         0, monkeysee, monkeysee, monkeysee_state, empty_init, "Tandy Corporation", "Monkey See (1982 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1984, t3in1sa,    0,         0, t3in1sa,   t3in1sa,   t3in1sa_state,   empty_init, "Tandy Corporation", "3 in 1 Sports Arena", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

COMP( 1976, speechp,    0,         0, speechp,   speechp,   speechp_state,   empty_init, "Telesensory Systems, Inc.", "Speech+", MACHINE_SUPPORTS_SAVE )

COMP( 1974, tisr16,     0,         0, tisr16,    tisr16,    tisr16_state,    empty_init, "Texas Instruments", "SR-16", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1975, tisr16ii,   0,         0, tisr16,    tisr16ii,  tisr16_state,    empty_init, "Texas Instruments", "SR-16 II", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1975, ti1250,     0,         0, ti1250,    ti1250,    ti1250_state,    empty_init, "Texas Instruments", "TI-1250 (1975 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, ti125076,   ti1250,    0, ti1270,    ti1250,    ti1250_state,    empty_init, "Texas Instruments", "TI-1250 (1976 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, ti1270,     0,         0, ti1270,    ti1270,    ti1250_state,    empty_init, "Texas Instruments", "TI-1270", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, ti25503,    0,         0, ti25503,   ti25503,   ti25503_state,   empty_init, "Texas Instruments", "TI-2550 III", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, ti5100,     0,         0, ti5100,    ti5100,    ti5100_state,    empty_init, "Texas Instruments", "TI-5100", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, ti30,       0,         0, ti30,      ti30,      ti30_state,      empty_init, "Texas Instruments", "TI-30", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, tibusan,    0,         0, ti30,      tibusan,   ti30_state,      empty_init, "Texas Instruments", "TI Business Analyst", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, tiprog,     0,         0, ti30,      tiprog,    ti30_state,      empty_init, "Texas Instruments", "TI Programmer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, ti1000,     0,         0, ti1000,    ti1000,    ti1000_state,    empty_init, "Texas Instruments", "TI-1000 (1977 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, wizatron,   0,         0, wizatron,  wizatron,  wizatron_state,  empty_init, "Texas Instruments", "Wiz-A-Tron", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1976, lilprof,    0,         0, lilprof,   lilprof,   lilprof_state,   empty_init, "Texas Instruments", "Little Professor (1976 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1978, lilprof78,  lilprof,   0, lilprof78, lilprof78, lilprof78_state, empty_init, "Texas Instruments", "Little Professor (1978 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, ti1680,     0,         0, ti1680,    ti1680,    ti1680_state,    empty_init, "Texas Instruments", "TI-1680", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1977, dataman,    0,         0, dataman,   dataman,   dataman_state,   empty_init, "Texas Instruments", "DataMan", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1980, mathmarv,   0,         0, mathmarv,  mathmarv,  mathmarv_state,  empty_init, "Texas Instruments", "Math Marvel", MACHINE_SUPPORTS_SAVE )
CONS( 1979, timaze,     0,         0, timaze,    timaze,    timaze_state,    empty_init, "Texas Instruments", "unknown electronic maze game (patent)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1979, tithermos,  0,         0, tithermos, tithermos, tithermos_state, empty_init, "Texas Instruments", "Electronic Digital Thermostat", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )

CONS( 1979, subwars,    0,         0, subwars,   subwars,   subwars_state,   empty_init, "Tiger Electronics", "Sub Wars (LED version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, playmaker,  0,         0, playmaker, playmaker, playmaker_state, empty_init, "Tiger Electronics", "Playmaker: Hockey, Soccer, Basketball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, dxfootb,    0,         0, dxfootb,   dxfootb,   dxfootb_state,   empty_init, "Tiger Electronics", "Deluxe Football with Instant Replay", MACHINE_SUPPORTS_SAVE )
CONS( 1979, copycat,    0,         0, copycat,   copycat,   copycat_state,   empty_init, "Tiger Electronics", "Copy Cat (model 7-520)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, copycatm2,  copycat,   0, copycatm2, copycatm2, copycatm2_state, empty_init, "Tiger Electronics", "Copy Cat (model 7-522)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1981, ditto,      0,         0, ditto,     ditto,     ditto_state,     empty_init, "Tiger Electronics", "Ditto", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, t7in1ss,    0,         0, t7in1ss,   t7in1ss,   t7in1ss_state,   empty_init, "Tiger Electronics", "7 in 1 Sports Stadium", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1979, tbreakup,   0,         0, tbreakup,  tbreakup,  tbreakup_state,  empty_init, "Tomy", "Break Up (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, phpball,    0,         0, phpball,   phpball,   phpball_state,   empty_init, "Tomy", "Power House Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1980, ssports4,   0,         0, ssports4,  ssports4,  ssports4_state,  empty_init, "U.S. Games", "Super Sports-4", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1983, xl25,       0,         0, xl25,      xl25,      xl25_state,      empty_init, "Vulcan Electronics", "XL 25", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, book, playing cards, pen & paper, etc.
