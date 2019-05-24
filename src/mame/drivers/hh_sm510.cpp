// license:BSD-3-Clause
// copyright-holders:hap, Henrik Algestam
// thanks-to:Sean Riddle, Igor
/***************************************************************************

Sharp SM5xx family handhelds.
List of child drivers:
- rzone: Tiger R-Zone

The LCD screen graphics are provided internally with an SVG file.
MAME external artwork is recommended for the backgrounds inlays.

Most of these LCD games are meant to stay powered on 24/7. There is no
RTC or NVRAM. Quitting MAME is akin to removing the handheld's battery.
Use -autosave to at least make them remember the highscores.

TODO:
- improve/redo SVGs of: gnw_mmouse, gnw_egg, exospace
- confirm gnw_mmouse/gnw_egg rom (dumped from Soviet clone, but pretty
  confident that it's same)
- scan and identify lcd segments for gnw_chef
- confirm gnw_chef rom (dumped from Soviet clone but should be the same)
- confirm gnw_climbcs rom (assumed to be the same as gnw_climber)
- Currently there is no accurate way to dump the SM511/SM512 melody ROM
  electronically. For the ones that weren't decapped, they were read by
  playing back all melody data and reconstructing it to ROM. Visual(decap)
  verification is wanted for: gnw_bfight, gnw_bjack, gnw_climber, gnw_zelda
- identify lcd segments for tgaiden

****************************************************************************

Misc Nintendo Game & Watch notes:

Trivia: Most of the Nintendo G&W have built-in cheats, likely kept in by
Nintendo to test the game. These were not accessible to users of course,
but for the sake of fun they're (usually) available on MAME.

BTANB: On some of the earlier G&W games, eg. gnw_fire, gnw_mmouse, gnw_pchute,
gnw_popeye, the controls still work after game over, this happens on the real
thing too.

Game list (* denotes not emulated yet)

Serial  Series MCU     Title
---------------------------------------------
AC-01*    s    SM5A?   Ball (aka Toss-Up)
FL-02*    s    SM5A?   Flagman
MT-03*    s    SM5A?   Vermin (aka The Exterminator)
RC-04*    s    SM5A?   Fire (aka Fireman Fireman)
IP-05*    g    SM5A?   Judge
MN-06*    g    SM5A?   Manhole
CN-07*    g    SM5A?   Helmet (aka Headache)
LN-08*    g    SM5A?   Lion
PR-21     ws   SM5A    Parachute
OC-22     ws   SM5A    Octopus
PP-23     ws   SM5A    Popeye
FP-24*    ws   SM5A    Chef
MC-25     ws   SM5A    Mickey Mouse
EG-26     ws   SM5A    Egg (near-certainly same ROM as MC-25, but LCD differs)
FR-27     ws   SM5A    Fire
TL-28     ws   SM510   Turtle Bridge
ID-29     ws   SM510   Fire Attack
SP-30     ws   SM510   Snoopy Tennis
OP-51     ms   SM510   Oil Panic
DK-52     ms   SM510   Donkey Kong
DM-53     ms   SM510   Mickey & Donald
GH-54     ms   SM510   Green House
JR-55     ms   SM510   Donkey Kong II
MW-56     ms   SM510   Mario
LP-57     ms   SM510   Rain Shower
TC-58     ms   SM510   Life Boat
PB-59*    ms   SM511?  Pinball
BJ-60     ms   SM512   Black Jack
MG-61     ms   SM510   Squish
BD-62*    ms   SM512   Bomb Sweeper
JB-63*    ms   SM511?  Safe Buster
MV-64*    ms   SM511?  Gold Cliff
ZL-65     ms   SM512   Zelda
CJ-71*    tt   SM511?  Donkey Kong Jr.
CM-72*    tt   SM511?  Mario's Cement Factory
SM-73*    tt   SM511?  Snoopy
PG-74*    tt   SM511?  Popeye
SM-91*    p    SM511?  Snoopy (assume same ROM & LCD as tabletop version)
PG-92*    p    SM511?  Popeye          "
CJ-93*    p    SM511?  Donkey Kong Jr. "
PB-94*    p    SM511?  Mario's Bombs Away
DC-95*    p    SM511?  Mickey Mouse
MK-96*    p    SM511?  Donkey Kong Circus (same ROM as DC-95? LCD is different)
DJ-101    nws  SM510   Donkey Kong Jr.
ML-102    nws  SM510   Mario's Cement Factory
NH-103    nws  SM510   Manhole
TF-104    nws  SM510   Tropical Fish
YM-105    nws  SM511   Super Mario Bros.
DR-106    nws  SM511   Climber
BF-107    nws  SM511   Balloon Fight
MJ-108*   nws  SM511?  Mario The Juggler
BU-201*   sc   SM510?  Spitball Sparky
UD-202*   sc   SM510?  Crab Grab
BX-301    mvs  SM511   Boxing (aka Punch Out)
AK-302*   mvs  SM511?  Donkey Kong 3
HK-303*   mvs  SM511?  Donkey Kong Hockey
YM-801*   cs   SM511   Super Mario Bros. (assume same ROM as nws version)
DR-802    cs   SM511   Climber            "
BF-803*   cs   SM511   Balloon Fight      "
YM-901-S* x    SM511   Super Mario Bros.  "

RGW-001 (2010 Ball remake) is on different hardware, ATmega169PV MCU.
The "Mini Classics" keychains are by Nelsonic, not Nintendo.

***************************************************************************/

#include "emu.h"
#include "includes/hh_sm510.h"

#include "cpu/sm510/sm500.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "gnw_dualv.lh"
#include "gnw_dualh.lh"

//#include "hh_sm510_test.lh" // common test-layout - use external artwork
//#include "hh_sm500_test.lh" // "


// machine start/reset

void hh_sm510_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();

	// determine number of input lines (set it in the subclass constructor if different)
	if (m_inp_lines == 0 && m_inp_fixed < 0)
	{
		for (; m_inp_matrix[m_inp_lines] != nullptr; m_inp_lines++) { ; }

		// when last input line is fixed(GND)
		if (m_inp_fixed == -2)
		{
			m_inp_lines--;
			m_inp_fixed = m_inp_lines;
		}
	}

	// zerofill
	m_inp_mux = 0;
	m_speaker_data = 0;
	m_s = 0;
	m_r = 0;
	m_display_x_len = 0;
	m_display_y_len = 0;
	m_display_z_len = 0;
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_inp_lines));
	save_item(NAME(m_inp_fixed));
	save_item(NAME(m_speaker_data));
	save_item(NAME(m_s));
	save_item(NAME(m_r));
	save_item(NAME(m_display_x_len));
	save_item(NAME(m_display_y_len));
	save_item(NAME(m_display_z_len));
	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
}

void hh_sm510_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// lcd panel - on lcd handhelds, usually not a generic x/y screen device
// deflicker here, especially needed for SM500/SM5A with the active shift register

TIMER_DEVICE_CALLBACK_MEMBER(hh_sm510_state::display_decay_tick)
{
	u8 z_mask = (1 << m_display_z_len) - 1;
	u8 zx_len = 1 << (m_display_x_len + m_display_z_len);

	for (int zx = 0; zx < zx_len; zx++)
	{
		for (int y = 0; y < m_display_y_len; y++)
		{
			// delay lcd segment on/off state
			if (m_display_state[zx] >> y & 1)
			{
				if (m_display_decay[y][zx] < (2 * m_display_wait - 1))
					m_display_decay[y][zx]++;
			}
			else if (m_display_decay[y][zx] > 0)
				m_display_decay[y][zx]--;
			u8 active_state = (m_display_decay[y][zx] < m_display_wait) ? 0 : 1;

			// SM510 series: output to x.y.z, where:
			// x = group a/b/bs/c (0/1/2/3)
			// y = segment 1-16 (0-15)
			// z = common H1-H4 (0-3)

			// SM500 series: output to x.y.z, where:
			// x = O group (0-*)
			// y = O segment 1-4 (0-3)
			// z = common H1/H2 (0/1)
			m_out_x[zx >> m_display_z_len][y][zx & z_mask] = active_state;
		}
	}
}

void hh_sm510_state::set_display_size(u8 x, u8 y, u8 z)
{
	// x = groups(in bits)
	// y = number of segments per group
	// z = commons(in bits)
	m_display_x_len = x;
	m_display_y_len = y;
	m_display_z_len = z;
}

WRITE16_MEMBER(hh_sm510_state::sm510_lcd_segment_w)
{
	set_display_size(2, 16, 2);
	m_display_state[offset] = data;
}

WRITE8_MEMBER(hh_sm510_state::sm500_lcd_segment_w)
{
	set_display_size(4, 4, 1);
	m_display_state[offset] = data;
}


// generic input handlers - usually S output is input mux, and K input for buttons

u8 hh_sm510_state::read_inputs(int columns, int fixed)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	if (fixed >= 0)
		ret |= m_inp_matrix[fixed]->read();

	return ret;
}

void hh_sm510_state::update_k_line()
{
	// this is necessary because the MCU can wake up on K input activity
	m_maincpu->set_input_line(SM510_INPUT_LINE_K, input_r(machine().dummy_space(), 0, 0xff) ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::input_changed)
{
	update_k_line();
}

WRITE8_MEMBER(hh_sm510_state::input_w)
{
	m_inp_mux = data;
	update_k_line();
}

READ8_MEMBER(hh_sm510_state::input_r)
{
	return read_inputs(m_inp_lines, m_inp_fixed);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::acl_button)
{
	// ACL button is directly tied to MCU ACL pin
	m_maincpu->set_input_line(SM510_INPUT_LINE_ACL, newval ? ASSERT_LINE : CLEAR_LINE);
}


// other generic output handlers

WRITE8_MEMBER(hh_sm510_state::piezo_r1_w)
{
	// R1 to piezo (SM511 R pin is melody output)
	m_speaker->level_w(data & 1);
}

WRITE8_MEMBER(hh_sm510_state::piezo_r2_w)
{
	// R2 to piezo
	m_speaker->level_w(data >> 1 & 1);
}

WRITE8_MEMBER(hh_sm510_state::piezo_input_w)
{
	// R1 to piezo, other to input mux
	piezo_r1_w(space, 0, data & 1);
	input_w(space, 0, data >> 1);
}

static const s16 piezo2bit_r1_120k_s1_39k[] = { 0, 0x7fff/3*1, 0x7fff/3*2, 0x7fff }; // R via 120K resistor, S1 via 39K resistor (eg. tsonic, tsonic2, tbatmana)

WRITE8_MEMBER(hh_sm510_state::piezo2bit_r1_w)
{
	// R1(+S1) to piezo
	m_speaker_data = (m_speaker_data & ~1) | (data & 1);
	m_speaker->level_w(m_speaker_data);
}

WRITE8_MEMBER(hh_sm510_state::piezo2bit_input_w)
{
	// S1(+R1) to piezo, other to input mux
	m_speaker_data = (m_speaker_data & ~2) | (data << 1 & 2);
	m_speaker->level_w(m_speaker_data);
	input_w(space, 0, data >> 1);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

#define PORT_CHANGED_CB(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, x, nullptr)

/***************************************************************************

  Konami Double Dribble
  * Sharp SM510 under epoxy (die label CMS54C, KMS584)
  * lcd screen with custom segments, 1-bit sound

  BTANB: At the basket, the ball goes missing sometimes for 1 frame, or
  may show 2 balls at the same time. It's the same on the real handheld.
  Another BTANB? If a period is over at the same time a defender on the
  2nd column grabs the ball, his arm won't be erased until it's redrawn.

***************************************************************************/

class kdribble_state : public hh_sm510_state
{
public:
	kdribble_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kdribble(machine_config &config);
};

// config

static INPUT_PORTS_START( kdribble )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Level Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kdribble_state::kdribble(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1524, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kdribble )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "584", 0x0000, 0x1000, CRC(1d9022c8) SHA1(64567f9f161e830a0634d5c89917ab866c26c0f8) )

	ROM_REGION( 450339, "svg", 0)
	ROM_LOAD( "kdribble.svg", 0, 450339, CRC(86c3ecc4) SHA1(8dfaeb0f3b35d4b680daaa9f478a6f3decf6ea0a) )
ROM_END





/***************************************************************************

  Konami Top Gun
  * PCB label BH003
  * Sharp SM510 under epoxy (die label CMS54C, KMS598)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class ktopgun_state : public hh_sm510_state
{
public:
	ktopgun_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void ktopgun(machine_config &config);
};

// config

static INPUT_PORTS_START( ktopgun )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void ktopgun_state::ktopgun(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1515, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ktopgun ) // except for filler/unused bytes, ROM listing in patent US5137277 "BH003 Top Gun" is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "598", 0x0000, 0x1000, CRC(50870b35) SHA1(cda1260c2e1c180995eced04b7d7ff51616dcef5) )

	ROM_REGION( 425832, "svg", 0)
	ROM_LOAD( "ktopgun.svg", 0, 425832, CRC(dc488ac0) SHA1(5a47e5639cb1e61dad3f2169efb99efe3d75896f) )
ROM_END





/***************************************************************************

  Konami Contra
  * PCB label BH002
  * Sharp SM511 under epoxy (die label KMS73B, KMS773)
  * lcd screen with custom segments, 1-bit sound

  Contra handheld is titled simply "C" in the USA.

***************************************************************************/

class kcontra_state : public hh_sm510_state
{
public:
	kcontra_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kcontra(machine_config &config);
};

// config

static INPUT_PORTS_START( kcontra )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kcontra_state::kcontra(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1505, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kcontra ) // except for filler/unused bytes, ROM listing in patent US5120057 "BH002 C (Contra)" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "773.program", 0x0000, 0x1000, CRC(bf834877) SHA1(055dd56ec16d63afba61ab866481fd9c029fb54d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "773.melody", 0x000, 0x100, CRC(23d02b99) SHA1(703938e496db0eeacd14fe7605d4b5c39e0a5bc8) )

	ROM_REGION( 721005, "svg", 0)
	ROM_LOAD( "kcontra.svg", 0, 721005, CRC(b5370d0f) SHA1(2f401222d24fa32a4659ef2b64ddac8ac3973c69) )
ROM_END





/***************************************************************************

  Konami Teenage Mutant Ninja Turtles
  * PCB label BH005
  * Sharp SM511 under epoxy (die label KMS73B, KMS774)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class ktmnt_state : public hh_sm510_state
{
public:
	ktmnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void ktmnt(machine_config &config);
};

// config

static INPUT_PORTS_START( ktmnt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CHANGED_CB(input_changed)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void ktmnt_state::ktmnt(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1505, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ktmnt ) // except for filler/unused bytes, ROM listing in patent US5150899 "BH005 TMNT" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "774.program", 0x0000, 0x1000, CRC(a1064f87) SHA1(92156c35fbbb414007ee6804fe635128a741d5f1) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "774.melody", 0x000, 0x100, CRC(8270d626) SHA1(bd91ca1d5cd7e2a62eef05c0033b19dcdbe441ca) )

	ROM_REGION( 610270, "svg", 0)
	ROM_LOAD( "ktmnt.svg", 0, 610270, CRC(ad9412ed) SHA1(154ee44efcd340dafa1cb84c37a9c3cd42cb42ab) )
ROM_END





/***************************************************************************

  Konami Gradius
  * PCB label BH004
  * Sharp SM511 under epoxy (die label KMS73B, KMS774)
  * lcd screen with custom segments, 1-bit sound

  Known in Japan as Nemesis.

***************************************************************************/

class kgradius_state : public hh_sm510_state
{
public:
	kgradius_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kgradius(machine_config &config);
};

// config

static INPUT_PORTS_START( kgradius )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kgradius_state::kgradius(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1420, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kgradius )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "771.program", 0x0000, 0x1000, CRC(830c2afc) SHA1(bb9ebd4e52831cc02cd92dd4b37675f34cf37b8c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "771.melody", 0x000, 0x100, CRC(4c586b73) SHA1(14c5ab2898013a577f678970a648c374749cc66d) )

	ROM_REGION( 638097, "svg", 0)
	ROM_LOAD( "kgradius.svg", 0, 638097, CRC(3adbc0f1) SHA1(fe426bf2335ce30395ea14ecab6399a93c67816a) )
ROM_END





/***************************************************************************

  Konami Lone Ranger
  * PCB label BH009
  * Sharp SM511 under epoxy (die label KMS73B, KMS781)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kloneran_state : public hh_sm510_state
{
public:
	kloneran_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kloneran(machine_config &config);
};

// config

static INPUT_PORTS_START( kloneran )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kloneran_state::kloneran(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1497, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kloneran )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "781.program", 0x0000, 0x1000, CRC(52b9735f) SHA1(06c5ef6e7e781b1176d4c1f2445f765ccf18b3f7) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "781.melody", 0x000, 0x100, CRC(a393de36) SHA1(55089f04833ccb318524ab2b584c4817505f4019) )

	ROM_REGION( 633120, "svg", 0)
	ROM_LOAD( "kloneran.svg", 0, 633120, CRC(f55e5292) SHA1(d0a91b5cd8a1894e7abc9c505fff4a8e1d3bec7a) )
ROM_END





/***************************************************************************

  Konami Blades of Steel
  * PCB label BH011
  * Sharp SM511 under epoxy (die label KMS73B, KMS782)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kblades_state : public hh_sm510_state
{
public:
	kblades_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kblades(machine_config &config);
};

// config

static INPUT_PORTS_START( kblades )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kblades_state::kblades(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1516, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kblades )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "782.program", 0x0000, 0x1000, CRC(3351a35d) SHA1(84c64b65d3cabfa20c18f4649c9ede2578b82523) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "782.melody", 0x000, 0x100, CRC(e8bf48ba) SHA1(3852c014dc9136566322b4f9e2aab0e3ec3a7387) )

	ROM_REGION( 455113, "svg", 0)
	ROM_LOAD( "kblades.svg", 0, 455113, CRC(e22f44c8) SHA1(ac95a837e20f87f3afc6c234f7407cbfcc438011) )
ROM_END





/***************************************************************************

  Konami NFL Football
  * Sharp SM511 under epoxy (die label KMS73B, KMS786)
  * lcd screen with custom segments, 1-bit sound

  This is the 1989 version. It was rereleased in 1992, assumed to be the same
  game underneath.

***************************************************************************/

class knfl_state : public hh_sm510_state
{
public:
	knfl_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void knfl(machine_config &config);
};

// config

static INPUT_PORTS_START( knfl )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void knfl_state::knfl(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1449, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( knfl )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "786.program", 0x0000, 0x1000, CRC(0535c565) SHA1(44cdcd284713ff0b194b24beff9f1b94c8bc63b2) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "786.melody", 0x000, 0x100, CRC(6c80263b) SHA1(d3c21e2f8491fef101907b8e0871b1e1c1ed58f5) )

	ROM_REGION( 571134, "svg", 0)
	ROM_LOAD( "knfl.svg", 0, 571134, CRC(f2c63235) SHA1(70b9232700f5498d3c63c63dd5904c0e19482cc2) )
ROM_END





/***************************************************************************

  Konami The Adventures of Bayou Billy
  * Sharp SM511 under epoxy (die label KMS73B, KMS788)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kbilly_state : public hh_sm510_state
{
public:
	kbilly_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kbilly(machine_config &config);
};

// config

static INPUT_PORTS_START( kbilly )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kbilly_state::kbilly(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1490, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kbilly )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "788.program", 0x0000, 0x1000, CRC(b8b1f734) SHA1(619dd527187b43276d081cdb1b13e0a9a81f2c6a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "788.melody", 0x000, 0x100, CRC(cd488bea) SHA1(8fc60081f46e392978d6950c74711fb7ebd154de) )

	ROM_REGION( 598276, "svg", 0)
	ROM_LOAD( "kbilly.svg", 0, 598276, CRC(2969319e) SHA1(5cd1b0a6eee3168142c1d24f167b9ef38ad88402) )
ROM_END





/***************************************************************************

  Konami Bucky O'Hare
  * Sharp SM511 under epoxy (die label KMS73B, N58)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kbucky_state : public hh_sm510_state
{
public:
	kbucky_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kbucky(machine_config &config);
};

// config

static INPUT_PORTS_START( kbucky )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kbucky_state::kbucky(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1490, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kbucky )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n58.program", 0x0000, 0x1000, CRC(7c36a0c4) SHA1(1b55ac64a71af746fd0a0f44266fcc92cca77482) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n58.melody", 0x000, 0x100, CRC(7e99e469) SHA1(3e9a3843c6ab392f5989f3366df87a2d26cb8620) )

	ROM_REGION( 727841, "svg", 0)
	ROM_LOAD( "kbucky.svg", 0, 727841, CRC(c1d78488) SHA1(9ba4fdbce977455b8f1ad4bd2b01faa44bd05bc7) )
ROM_END





/***************************************************************************

  Konami Garfield
  * Sharp SM511 under epoxy (die label KMS73B, N62)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kgarfld_state : public hh_sm510_state
{
public:
	kgarfld_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kgarfld(machine_config &config);
};

// config

static INPUT_PORTS_START( kgarfld )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("All Clear")
INPUT_PORTS_END

void kgarfld_state::kgarfld(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1500, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kgarfld )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n62.program", 0x0000, 0x1000, CRC(5a762049) SHA1(26d4d891160d254dfd752734e1047126243f88dd) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n62.melody", 0x000, 0x100, CRC(232b7d55) SHA1(76f6a19e8182ee3f00c9f4ef007b5dde75a9c00d) )

	ROM_REGION( 581107, "svg", 0)
	ROM_LOAD( "kgarfld.svg", 0, 581107, CRC(bf09a170) SHA1(075cb95535873018409eb15675183490c61b29b9) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Parachute (model PR-21)
  * PCB label PR-21Y
  * Sharp SM5A label PR-21 52XC (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_pchute_state : public hh_sm510_state
{
public:
	gnw_pchute_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_pchute(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_pchute )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // * Infinite lives cheat here, but configuring it is weird:
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // * One of the R3 inputs needs to be held down, followed
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // * by pressing ACL, then release.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_pchute_state::gnw_pchute(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1602, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_pchute )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pr-21", 0x0000, 0x0740, CRC(392b545e) SHA1(e71940cd4cee07ba1e62c1c7d9e9b19410e7232d) )

	ROM_REGION( 169486, "svg", 0)
	ROM_LOAD( "gnw_pchute.svg", 0, 169486, CRC(bf86e0f9) SHA1(d2fba49453afc4bd1f16613f833a8748b6a36764) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Octopus (model OC-22)
  * PCB label OC-22Y A
  * Sharp SM5A label OC-22 204A (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_octopus_state : public hh_sm510_state
{
public:
	gnw_octopus_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_octopus(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_octopus )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // "

	PORT_START("IN.1") // R3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_octopus_state::gnw_octopus(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1586, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_octopus )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "oc-22", 0x0000, 0x0740, CRC(bd27781d) SHA1(07b4feb9265c83b159f96c7e8ee1c61a2cc17dc5) )

	ROM_REGION( 119681, "svg", 0)
	ROM_LOAD( "gnw_octopus.svg", 0, 119681, CRC(39900430) SHA1(61b71c475365966257f5479eab992538ec235c11) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Popeye (model PP-23)
  * PCB label PP-23 Y
  * Sharp SM5A label PP-23 52YD (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the wide screen version, there's also tabletop and panorama versions.

***************************************************************************/

class gnw_popeye_state : public hh_sm510_state
{
public:
	gnw_popeye_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_popeye(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_popeye )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)") // when cheat is activated, not all segments are lit on the ACL reset screen
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // reset?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.1") // R3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_popeye_state::gnw_popeye(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1604, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_popeye )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pp-23", 0x0000, 0x0740, CRC(49987769) SHA1(ad90659a3ce7169a4df16367c5307435d9f9d956) )

	ROM_REGION( 218428, "svg", 0)
	ROM_LOAD( "gnw_popeye.svg", 0, 218428, CRC(b2c3fdf2) SHA1(5e782f25f9ff432a292e67efc7f5653cf2a81b60) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Chef (model FP-24)
  * Sharp SM5A label ?
  * lcd screen with custom segments, 1-bit sound

  In 1989, Elektronika(USSR) released a clone: Merry Cook. This game most
  likely shares the same ROM (to be verified) and the graphics is slightly
  different.

***************************************************************************/

class gnw_chef_state : public hh_sm510_state
{
public:
	gnw_chef_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void merrycook(machine_config &config);
	void gnw_chef(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_chef )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND, only works after power-on
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_chef_state::gnw_chef(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // assuming same as merry cook
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1920, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void gnw_chef_state::merrycook(machine_config & config)
{
	gnw_chef(config);

	/* basic machine hardware */
	KB1013VK12(config.replace(), m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1679, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( gnw_chef )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fp-24.bin", 0x0000, 0x0740, BAD_DUMP CRC(2806ab39) SHA1(18261a80eec5bf768bb88b803c598f80e078c71f) ) // dumped from Soviet clone

	ROM_REGION( 100000, "svg", 0)
	ROM_LOAD( "gnw_chef.svg", 0, 100000, NO_DUMP )
ROM_END

ROM_START( merrycook )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "merrycook.bin", 0x0000, 0x0740, CRC(2806ab39) SHA1(18261a80eec5bf768bb88b803c598f80e078c71f) )

	ROM_REGION( 143959, "svg", 0)
	ROM_LOAD( "merrycook.svg", 0, 143959, CRC(5601535e) SHA1(7b8818ce3523cccff4adcade9603b7f719d8ab48) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mickey Mouse (model MC-25), Egg (model EG-26)
  * Sharp SM5A label ?
  * lcd screen with custom segments, 1-bit sound

  MC-25 and EG-26 are the same game, it's assumed that the latter was for
  regions where Nintendo wasn't able to license from Disney.

  In 1984, Elektronika(USSR) released a clone: Nu, pogodi! This was followed
  by several other titles that were the same under the hood, only differing
  in graphics. They also made a slightly modified version, adding a new game
  mode (by pressing A+B) where the player/CPU roles are reversed. This version
  is known as Razvedciki kosmosa (export version: Explorers of Space).

***************************************************************************/

class gnw_mmouse_state : public hh_sm510_state
{
public:
	gnw_mmouse_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void exospace(machine_config &config);
	void nupogodi(machine_config &config);
	void gnw_egg(machine_config &config);
	void gnw_mmouse(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mmouse )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND, only works after power-on
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( exospace )
	PORT_INCLUDE( gnw_mmouse )

	PORT_MODIFY("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void gnw_mmouse_state::gnw_mmouse(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // ?
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1711, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void gnw_mmouse_state::gnw_egg(machine_config &config)
{
	gnw_mmouse(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1694, 1080);
	screen->set_visarea_full();
}

void gnw_mmouse_state::nupogodi(machine_config &config)
{
	gnw_mmouse(config);

	/* basic machine hardware */
	KB1013VK12(config.replace(), m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // ?
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1715, 1080);
	screen->set_visarea_full();
}

void gnw_mmouse_state::exospace(machine_config &config)
{
	nupogodi(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1756, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( gnw_mmouse )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc-25", 0x0000, 0x0740, BAD_DUMP CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) ) // dumped from Soviet clone

	ROM_REGION( 102453, "svg", 0)
	ROM_LOAD( "gnw_mmouse.svg", 0, 102453, BAD_DUMP CRC(88cc7c49) SHA1(c000d51d1b99750116b97f9bafc0314ea506366d) )
ROM_END

ROM_START( gnw_egg )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "eg-26", 0x0000, 0x0740, BAD_DUMP CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) ) // dumped from Soviet clone

	ROM_REGION( 102848, "svg", 0)
	ROM_LOAD( "gnw_egg.svg", 0, 102848, BAD_DUMP CRC(742c2605) SHA1(984d430ad2ff47ad7a3f9b25b7d3f3d51b10cca5) )
ROM_END

ROM_START( nupogodi )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "nupogodi.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 156974, "svg", 0)
	ROM_LOAD( "nupogodi.svg", 0, 156974, CRC(8d522ec6) SHA1(67afeca5eebd16449353ea43070a6b919f7ba408) )
ROM_END

ROM_START( exospace )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "exospace.bin", 0x0000, 0x0740, CRC(553e2b09) SHA1(2b74f8437b881fbb62b61f25435a5bfc66872a9a) )

	ROM_REGION( 66790, "svg", 0)
	ROM_LOAD( "exospace.svg", 0, 66790, BAD_DUMP CRC(df31043a) SHA1(2d8caf42894df699e469652e5f448beaebbcc1ae) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Fire (model FR-27)
  * PCB label FR-27
  * Sharp SM5A label FR-27 523B (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the wide screen version, there's also a silver version.
  Also copied by Elektronika as "Space Bridge", with different LCD.

***************************************************************************/

class gnw_fire_state : public hh_sm510_state
{
public:
	gnw_fire_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_fire(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_fire )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // reset?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.1") // R3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED ) // lcd test?

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_fire_state::gnw_fire(machine_config &config)
{
	/* basic machine hardware */
	SM5A(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1624, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_fire )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fr-27", 0x0000, 0x0740, CRC(f4c53ef0) SHA1(6b57120a0f9d2fd4dcd65ad57a5f32def71d905f) )

	ROM_REGION( 163753, "svg", 0)
	ROM_LOAD( "gnw_fire.svg", 0, 163753, CRC(d546fa42) SHA1(492c785aa0ed33ff1ac8c84066e5b6d7cb7d1566) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Turtle Bridge (model TL-28)
  * PCB label TL-28
  * Sharp SM510 label TL-28 523C (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_tbridge_state : public hh_sm510_state
{
public:
	gnw_tbridge_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_tbridge(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_tbridge )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_tbridge_state::gnw_tbridge(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1587, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_tbridge )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tl-28", 0x0000, 0x1000, CRC(284e7224) SHA1(b50d7f3a527ffe50771ef55fdf8214929bfa2253) )

	ROM_REGION( 242781, "svg", 0)
	ROM_LOAD( "gnw_tbridge.svg", 0, 242781, CRC(c0473e53) SHA1(bb43f12f517a3b657b5b35b50baf176e01ce041d) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Fire Attack (model ID-29)
  * PCB label ID-29
  * Sharp SM510 label ID-29 524B (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_fireatk_state : public hh_sm510_state
{
public:
	gnw_fireatk_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_fireatk(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_fireatk )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_fireatk_state::gnw_fireatk(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1655, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_fireatk )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "id-29", 0x0000, 0x1000, CRC(5f6e8042) SHA1(63afc3acd8a2a996095fa8ba2dfccd48e5214478) )

	ROM_REGION( 267755, "svg", 0)
	ROM_LOAD( "gnw_fireatk.svg", 0, 267755, CRC(b13ee452) SHA1(4d1e7e10fd2352bdd805c25de8c0e16bcd8b2220) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Snoopy Tennis (model SP-30)
  * PCB label SP-30
  * Sharp SM510 label SP-30 525B (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_stennis_state : public hh_sm510_state
{
public:
	gnw_stennis_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_stennis(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_stennis )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Hit

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_stennis_state::gnw_stennis(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1581, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_stennis )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "sp-30", 0x0000, 0x1000, CRC(ba1d9504) SHA1(ff601765d88564b1570a59f5b1a4005c7b0fd66c) )

	ROM_REGION( 227964, "svg", 0)
	ROM_LOAD( "gnw_stennis.svg", 0, 227964, CRC(1bb5f99a) SHA1(2e999c75598448e3502e7bab16e987d80d6a301f) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Oil Panic (model OP-51)
  * PCB label OP-51A
  * Sharp SM510 label OP-51 28ZB (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_opanic_state : public hh_sm510_state
{
public:
	gnw_opanic_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_opanic(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_opanic )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_opanic_state::gnw_opanic(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1292/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1230/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_opanic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "op-51", 0x0000, 0x1000, CRC(31c288c9) SHA1(4bfd0fba94a9927cefc925db8196b063c5dd9b19) )

	ROM_REGION( 79616, "svg_top", 0)
	ROM_LOAD( "gnw_opanic_top.svg", 0, 79616, CRC(208dccc5) SHA1(b3cd3dcc8a00ba3b1b8d93d902f756fe579e4dfc) )

	ROM_REGION( 112809, "svg_bottom", 0)
	ROM_LOAD( "gnw_opanic_bottom.svg", 0, 112809, CRC(919b9649) SHA1(f3d3c8ca3fed81782a1fcb5a7aff07faea86db07) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong (model DK-52)
  * PCB label DK-52C
  * Sharp SM510 label DK-52 52ZD (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_dkong_state : public hh_sm510_state
{
public:
	gnw_dkong_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkong(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkong )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkong_state::gnw_dkong(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1266/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1266/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_dkong )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dk-52", 0x0000, 0x1000, CRC(5180cbf8) SHA1(5174570a8d6a601226f51e972bac6735535fe11d) )

	ROM_REGION( 176706, "svg_top", 0)
	ROM_LOAD( "gnw_dkong_top.svg", 0, 176706, CRC(db041556) SHA1(fb0f979dea3ecd25288d341fa80e35b5fd0a8349) )

	ROM_REGION( 145397, "svg_bottom", 0)
	ROM_LOAD( "gnw_dkong_bottom.svg", 0, 145397, CRC(2c8c9d08) SHA1(658fd0bbccaabb0645b02e5cb81709c4b2a4250e) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mickey & Donald (model DM-53)
  * PCB label DM-53
  * Sharp SM510 label DM-53 52ZC (die label CMS54C, CMS565)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_mickdon_state : public hh_sm510_state
{
public:
	gnw_mickdon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mickdon(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mickdon )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mickdon_state::gnw_mickdon(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r2_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1281/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1236/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_mickdon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dm-53_565", 0x0000, 0x1000, CRC(e21fc0f5) SHA1(3b65ccf9f98813319410414e11a3231b787cdee6) )

	ROM_REGION( 126434, "svg_top", 0)
	ROM_LOAD( "gnw_mickdon_top.svg", 0, 126434, CRC(ff05f489) SHA1(2a533c7b5d7249d79f8d7795a0d57fd3e32d3d32) )

	ROM_REGION( 122870, "svg_bottom", 0)
	ROM_LOAD( "gnw_mickdon_bottom.svg", 0, 122870, CRC(8f06ddf1) SHA1(69d4b785781600abcdfc01b3902df1d0ae3608cf) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Green House (model GH-54)
  * PCB label GH-54
  * Sharp SM510 label GH-54 52ZD (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_ghouse_state : public hh_sm510_state
{
public:
	gnw_ghouse_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_ghouse(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_ghouse )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Spray

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_ghouse_state::gnw_ghouse(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1303/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1274/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_ghouse )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "gh-54", 0x0000, 0x1000, CRC(4df12b4d) SHA1(708be5fef8dbd9337f5ab35baaca5bdf21e1f36c) )

	ROM_REGION( 159098, "svg_top", 0)
	ROM_LOAD( "gnw_ghouse_top.svg", 0, 159098, CRC(96bc58d9) SHA1(eda6a0abde739fb71af3e150751a519e59ef021d) )

	ROM_REGION( 149757, "svg_bottom", 0)
	ROM_LOAD( "gnw_ghouse_bottom.svg", 0, 149757, CRC(d66ee72c) SHA1(dcbe1c81ee0c7ddb9692858749ce6934f4dd7f30) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong II (model JR-55)
  * PCB label JR-55
  * Sharp SM510 label JR-55 53YC (die label CMS54C, KMS560)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_dkong2_state : public hh_sm510_state
{
public:
	gnw_dkong2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkong2(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkong2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkong2_state::gnw_dkong2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1241/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1237/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_dkong2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "jr-55_560", 0x0000, 0x1000, CRC(46aed0ae) SHA1(72f75ccbd84aea094148c872fc7cc1683619a18a) )

	ROM_REGION( 267443, "svg_top", 0)
	ROM_LOAD( "gnw_dkong2_top.svg", 0, 267443, CRC(33b26edb) SHA1(600afdf22ff4ac4a4af2de9159287cc6e53dfe3a) )

	ROM_REGION( 390558, "svg_bottom", 0)
	ROM_LOAD( "gnw_dkong2_bottom.svg", 0, 390558, CRC(92d68958) SHA1(aba829bf89b93bf3a4e425c9a8f6eec9e5869bc4) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario Bros. (model MW-56)
  * PCB label MW-56-M
  * Sharp SM510 label MW-56 533C (no decap)
  * horizontal dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_mario_state : public hh_sm510_state
{
public:
	gnw_mario_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mario(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mario )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mario_state::gnw_mario(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_left(SCREEN(config, "screen_left", SCREEN_TYPE_SVG));
	screen_left.set_svg_region("svg_left");
	screen_left.set_refresh_hz(50);
	screen_left.set_size(2258/2, 1440/2);
	screen_left.set_visarea_full();

	screen_device &screen_right(SCREEN(config, "screen_right", SCREEN_TYPE_SVG));
	screen_right.set_svg_region("svg_right");
	screen_right.set_refresh_hz(50);
	screen_right.set_size(2261/2, 1440/2);
	screen_right.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualh);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_mario )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mw-56", 0x0000, 0x1000, CRC(385e59da) SHA1(2f79281bdf2f2afca2fb5bd7b9a3beeffc9c4eb7) )

	ROM_REGION( 154874, "svg_left", 0)
	ROM_LOAD( "gnw_mario_left.svg", 0, 154874, CRC(73ba4f4a) SHA1(d5df39808a1af8e8ad5e397b4a50313221ab6e3b) )

	ROM_REGION( 202863, "svg_right", 0)
	ROM_LOAD( "gnw_mario_right.svg", 0, 202863, CRC(dd2473c9) SHA1(51aca37abf8e4959b84c441aa2d114e16c7d6010) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Rain Shower (model LP-57)
  * PCB labels: LP-57-M-I (left), LP-57-S (right)
  * Sharp SM510 label LP-57 538A (no decap)
  * horizontal dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_rshower_state : public hh_sm510_state
{
public:
	gnw_rshower_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_rshower(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_rshower )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // L/R
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_rshower_state::gnw_rshower(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_left(SCREEN(config, "screen_left", SCREEN_TYPE_SVG));
	screen_left.set_svg_region("svg_left");
	screen_left.set_refresh_hz(50);
	screen_left.set_size(2126/2, 1440/2);
	screen_left.set_visarea_full();

	screen_device &screen_right(SCREEN(config, "screen_right", SCREEN_TYPE_SVG));
	screen_right.set_svg_region("svg_right");
	screen_right.set_refresh_hz(50);
	screen_right.set_size(2146/2, 1440/2);
	screen_right.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualh);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_rshower )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "lp-57", 0x0000, 0x1000, CRC(51a2c5c4) SHA1(d60542e6785ba7b6a44153a66c739787cf670816) )

	ROM_REGION( 135698, "svg_left", 0)
	ROM_LOAD( "gnw_rshower_left.svg", 0, 135698, CRC(f0b36d70) SHA1(252e5cc110112a874265477be11ab3adf8108726) )

	ROM_REGION( 140280, "svg_right", 0)
	ROM_LOAD( "gnw_rshower_right.svg", 0, 140280, CRC(0ce4d049) SHA1(7e1afa1fdbdf658a12a28192ba2d29e5fca807cb) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Lifeboat (model TC-58)
  * PCB labels: TC-58-M (left), TC-58-S (right)
  * Sharp SM510 label TC-58 281D (no decap)
  * horizontal dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_lboat_state : public hh_sm510_state
{
public:
	gnw_lboat_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_lboat(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_lboat )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_lboat_state::gnw_lboat(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_left(SCREEN(config, "screen_left", SCREEN_TYPE_SVG));
	screen_left.set_svg_region("svg_left");
	screen_left.set_refresh_hz(50);
	screen_left.set_size(2116/2, 1440/2);
	screen_left.set_visarea_full();

	screen_device &screen_right(SCREEN(config, "screen_right", SCREEN_TYPE_SVG));
	screen_right.set_svg_region("svg_right");
	screen_right.set_refresh_hz(50);
	screen_right.set_size(2057/2, 1440/2);
	screen_right.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualh);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_lboat )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tc-58", 0x0000, 0x1000, CRC(1f88f6a2) SHA1(22fd62127dda43a0ada2fe89b0518eec8cbe2a25) )

	ROM_REGION( 156272, "svg_left", 0)
	ROM_LOAD( "gnw_lboat_left.svg", 0, 156272, CRC(1f0c18bd) SHA1(ca11c83b4b4d6a91ecb0300cff392e010064ba25) )

	ROM_REGION( 155093, "svg_right", 0)
	ROM_LOAD( "gnw_lboat_right.svg", 0, 155093, CRC(6f68780a) SHA1(63488693fbb1a8ad4d59da9e4e003eef709926f9) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Black Jack (model BJ-60)
  * PCB label BJ-60
  * Sharp SM512 label BJ-60 564D (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_bjack_state : public hh_sm510_state
{
public:
	gnw_bjack_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_bjack(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_bjack )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Double Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Bet x10 / Hit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Bet x1 / Stand")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Enter")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_bjack_state::gnw_bjack(machine_config &config)
{
	/* basic machine hardware */
	SM512(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1290/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1297/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_bjack )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bj-60.program", 0x0000, 0x1000, CRC(8e74f633) SHA1(54b0f65ee716d2820a9ed9c743755d2a2d99ce4d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bj-60.melody", 0x000, 0x100, BAD_DUMP CRC(2619224e) SHA1(b65dc590b6eb1de793e980af236ccf8360b3cfee) ) // decap needed for verification

	ROM_REGION( 75217, "svg_top", 0)
	ROM_LOAD( "gnw_bjack_top.svg", 0, 75205, CRC(5eb0956e) SHA1(f7acd148e5478d4c2ddf06cff23c5e40faee2c24) )

	ROM_REGION( 112450, "svg_bottom", 0)
	ROM_LOAD( "gnw_bjack_bottom.svg", 0, 112438, CRC(9d985b1d) SHA1(cf8af6ce18994f687a5e6fbdda62af4d07a07cf8) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Squish (model MG-61)
  * PCB label MG-61
  * Sharp SM510 label MG-61 8841B (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_squish_state : public hh_sm510_state
{
public:
	gnw_squish_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_squish(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_squish )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Bonus Life (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_squish_state::gnw_squish(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1285/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1287/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_squish )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg-61", 0x0000, 0x1000, CRC(79cd509c) SHA1(969e5425984ba9e5183c68b38b3588f53d1e8e5d) )

	ROM_REGION( 70300, "svg_top", 0)
	ROM_LOAD( "gnw_squish_top.svg", 0, 70300, CRC(f1358ba9) SHA1(414d29db64b83a50b20f31b857e4c3a77d19d3c8) )

	ROM_REGION( 279606, "svg_bottom", 0)
	ROM_LOAD( "gnw_squish_bottom.svg", 0, 279606, CRC(1d4ac23f) SHA1(d6eb78bae5ca18cc5fe5d8a300902766dd9601aa) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Zelda (model ZL-65)
  * PCB label ZL-65
  * Sharp SM512 label ZL-65 8935 A (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_zelda_state : public hh_sm510_state
{
public:
	gnw_zelda_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_zelda(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_zelda )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) // Water of Life

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Attack
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Continue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // Invincibility when playing on bottom screen only
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_zelda_state::gnw_zelda(machine_config &config)
{
	/* basic machine hardware */
	SM512(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen_top(SCREEN(config, "screen_top", SCREEN_TYPE_SVG));
	screen_top.set_svg_region("svg_top");
	screen_top.set_refresh_hz(50);
	screen_top.set_size(1920/2, 1346/2);
	screen_top.set_visarea_full();

	screen_device &screen_bottom(SCREEN(config, "screen_bottom", SCREEN_TYPE_SVG));
	screen_bottom.set_svg_region("svg_bottom");
	screen_bottom.set_refresh_hz(50);
	screen_bottom.set_size(1920/2, 1291/2);
	screen_bottom.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_gnw_dualv);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_zelda )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "zl-65.program", 0x0000, 0x1000, CRC(b96aa64e) SHA1(d1f0c64104eb3ecbf370674d5078a3a85b2b7227) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "zl-65.melody", 0x000, 0x100, BAD_DUMP CRC(3a281b0f) SHA1(7a236775557939050bbcd6f9d0a598d219a032f2) ) // decap needed for verification

	ROM_REGION( 282866, "svg_top", 0)
	ROM_LOAD( "gnw_zelda_top.svg", 0, 282866, CRC(7bd167a0) SHA1(96955538d9c0ab94b144ff725524b601bdf9f28c) )

	ROM_REGION( 424727, "svg_bottom", 0)
	ROM_LOAD( "gnw_zelda_bottom.svg", 0, 424727, CRC(2f4b3239) SHA1(026a1d43dd298ec05f4067ae1a7181984ec8ff83) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong Jr. (model DJ-101)
  * Sharp SM510 label DJ-101 52ZA (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a tabletop version that
  plays more like the arcade game.

***************************************************************************/

class gnw_dkjr_state : public hh_sm510_state
{
public:
	gnw_dkjr_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkjr(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkjr )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkjr_state::gnw_dkjr(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1647, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_dkjr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dj-101", 0x0000, 0x1000, CRC(8dcfb5d1) SHA1(e0ef578e9362eb9a3cab631376df3cf55978f2de) )

	ROM_REGION( 281161, "svg", 0)
	ROM_LOAD( "gnw_dkjr.svg", 0, 281161, CRC(346b025c) SHA1(dad3f3f73d6c2ff4efb43ffd76e97ba0d5f0da73) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario's Cement Factory (model ML-102)
  * Sharp SM510 label ML-102 298D (die label CMS54C, KMS577)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a tabletop version.

***************************************************************************/

class gnw_mariocm_state : public hh_sm510_state
{
public:
	gnw_mariocm_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mariocm(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mariocm )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Open
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mariocm_state::gnw_mariocm(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1647, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_mariocm )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ml-102_577", 0x0000, 0x1000, CRC(c1128dea) SHA1(8647e36f43a0e37756a3c7b6a3f08d4c8243f1cc) )

	ROM_REGION( 302931, "svg", 0)
	ROM_LOAD( "gnw_mariocm.svg", 0, 302931, CRC(5517ae80) SHA1(1902e36d0470ee5548addeb087ea3e7d2c2520a2) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Manhole (model NH-103)
  * PCB label NH-103
  * Sharp SM510 label NH-103 538A (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a Gold Series version (MH-06)

***************************************************************************/

class gnw_manhole_state : public hh_sm510_state
{
public:
	gnw_manhole_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_manhole(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_manhole )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_manhole_state::gnw_manhole(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1560, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_manhole )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "nh-103", 0x0000, 0x1000, CRC(ec03acf7) SHA1(b74ae672d8f8a155b2ea4ecee9afbaed95ec0ceb) )

	ROM_REGION( 223244, "svg", 0)
	ROM_LOAD( "gnw_manhole.svg", 0, 223244, CRC(41848e77) SHA1(d7238d1a3f95d8d274f5ff767ebf783bb50e64eb) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Tropical Fish (model TF-104)
  * PCB label TF-104
  * Sharp SM510 label TF-104 8739A (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_tfish_state : public hh_sm510_state
{
public:
	gnw_tfish_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_tfish(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_tfish )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_tfish_state::gnw_tfish(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1572, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_tfish )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tf-104", 0x0000, 0x1000, CRC(53cde918) SHA1(bc1e1b8f8b282bb886bb076c1c7ce35d00eca6fc) )

	ROM_REGION( 257278, "svg", 0)
	ROM_LOAD( "gnw_tfish.svg", 0, 257278, CRC(fc970f4a) SHA1(a73f5ee35b60842707f13edc5d58869fb2ec98cf) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Super Mario Bros. (model: see below)
  * PCB label YM-105
  * Sharp SM511 label YM-105 9024B (new wide screen version) (die label ?)
  * lcd screen with custom segments, 1-bit sound

  First released in 1986 on Crystal Screen (model YM-801), rereleased on
  New Wide Screen in 1988 (model YM-105). It was also a prize in a Nintendo
  game contest in 1987 (model YM-901-S). In YM-801, Mario looks like the
  ones in ML-102 and MW-56. In YM-901-S and YM-105 he looks more detailed.
  Until further proof, it's assumed that the ROM is the same for each model.

***************************************************************************/

class gnw_smb_state : public hh_sm510_state
{
public:
	gnw_smb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_smb(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_smb )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_smb_state::gnw_smb(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1677, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_smb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ym-105.program", 0x0000, 0x1000, CRC(0dff3b12) SHA1(3fa83f88e49ea9d7080fe935ec90ce69acbe8850) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "ym-105.melody", 0x000, 0x100, CRC(b48c6d90) SHA1(a1ce1e52627767752974ab0d49bec48ead36663e) )

	ROM_REGION( 648209, "svg", 0)
	ROM_LOAD( "gnw_smb.svg", 0, 648209, CRC(4a6fdb28) SHA1(0a0bc48d82d5b8bf8ef96ef9ce2f87ba6ea850c1) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Climber New Wide Screen (model DR-106),
  Nintendo Game & Watch: Climber Crystal Screen (model DR-802)
  * PCB label DR-106 (New Wide Screen), DR-802 (Crystal Screen)
  * Sharp SM511
     - label DR-106 9038B (new wide screen version) (no decap)
     - label DR-802 8626A (crystal screen) (not dumped yet)
  * lcd screen with custom segments, 1-bit sound

  First released in 1986 on Crystal Screen (model DR-802), rereleased on
  New Wide Screen in 1988 (model DR-106). The graphic LCD elements look the same
  in both versions but the display aspect ratio and the graphical background is
  slightly different.
  Until further proof, it's assumed that the ROM is the same for both models.

***************************************************************************/

class gnw_climber_state : public hh_sm510_state
{
public:
	gnw_climber_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_climber(machine_config &config);
	void gnw_climbcs(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_climber )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_climber_state::gnw_climber(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1677, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void gnw_climber_state::gnw_climbcs(machine_config &config)
{
	gnw_climber(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1756, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( gnw_climber )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dr-106.program", 0x0000, 0x1000, CRC(2adcbd6d) SHA1(110dc08c65120ab2c76ee647e89aa2726e24ac1a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "dr-106.melody", 0x000, 0x100, BAD_DUMP CRC(c99d7998) SHA1(4f8cf35b13f8b7654e7186bfd67d197d9053e949) ) // decap needed for verification

	ROM_REGION( 542332, "svg", 0)
	ROM_LOAD( "gnw_climber.svg", 0, 542332, CRC(d7e84c21) SHA1(a5b5b68c8cdb3a09966bfb91b281791bef311248) )
ROM_END

ROM_START( gnw_climbcs )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dr-106.program", 0x0000, 0x1000, BAD_DUMP CRC(2adcbd6d) SHA1(110dc08c65120ab2c76ee647e89aa2726e24ac1a) ) // dumped from NWS version

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "dr-106.melody", 0x000, 0x100, BAD_DUMP CRC(c99d7998) SHA1(4f8cf35b13f8b7654e7186bfd67d197d9053e949) ) // dumped from NWS version

	ROM_REGION( 564704, "svg", 0)
	ROM_LOAD( "gnw_climbcs.svg", 0, 564704, CRC(60b25cc5) SHA1(1c101539a861257c5b0334ffdf9491c877759fa1) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Balloon Fight (model: see below)
  * PCB label DR-106 (same PCB as in Climber (new wide screen version))
  * Sharp SM511 label BF-107 9031B (new wide screen version) (no decap)
  * lcd screen with custom segments, 1-bit sound

  First released in 1986 on Crystal Screen (model BF-803), rereleased on
  New Wide Screen in 1988 (model BF-107). The graphic LCD elements look the same
  in both versions but the graphical background is slightly different.
  Until further proof, it's assumed that the ROM is the same for both models.

***************************************************************************/

class gnw_bfight_state : public hh_sm510_state
{
public:
	gnw_bfight_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_bfight(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_bfight )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Eject
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_bfight_state::gnw_bfight(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1549, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_bfight )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bf-107.program", 0x0000, 0x1000, CRC(4c8d07ed) SHA1(a8974dff85d5f3bacaadb71b86e9b30994b6d129) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bf-107.melody", 0x000, 0x100, BAD_DUMP CRC(aadc22a1) SHA1(f6e5572232eb9e83f6833073e1e1e99776245c50) ) // decap needed for verification

	ROM_REGION( 558341, "svg", 0)
	ROM_LOAD( "gnw_bfight.svg", 0, 558341, CRC(f0d61fe8) SHA1(b0b56224a967e4b26836c0f7e3015d13b42ae5cc) )
ROM_END





/***************************************************************************

  Nintendo Micro Vs. System: Boxing (model BX-301)
  * Sharp SM511 label BX-301 287C (die label KMS73B, KMS744)
  * wide lcd screen with custom segments, 1-bit sound

  Also known as Punch-Out!! in the USA.

***************************************************************************/

class gnw_boxing_state : public hh_sm510_state
{
public:
	gnw_boxing_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_boxing(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_boxing )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_PLAYER(2)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_PLAYER(2)

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_PLAYER(2)

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // S7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "P2 Decrease Health (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "P1 Infinite Health (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_boxing_state::gnw_boxing(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1920, 524);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gnw_boxing )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bx-301_744.program", 0x0000, 0x1000, CRC(0fdf0303) SHA1(0b791c9d4874e9534d0a9b7a8968ce02fe4bee96) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bx-301_744.melody", 0x000, 0x100, CRC(439d943d) SHA1(52880df15ec7513f96482f455ef3d9778aa24750) )

	ROM_REGION( 265174, "svg", 0)
	ROM_LOAD( "gnw_boxing.svg", 0, 265174, CRC(e8a3ab25) SHA1(53e32542b582dcdf4ddd051f182738eee6c732c9) )
ROM_END





/***************************************************************************

  Tiger Gauntlet (model 7-778), Robin Hood (model 7-861)
  * Sharp SM510 under epoxy (die label CMS54C, KMS583)
  * lcd screen with custom segments, 1-bit sound

  known releases (Gauntlet):
  - World: Gauntlet
  - Japan: Gauntlet (published by Sega)
  - UK: Gauntlet (published by Grandstand)

  Robin Hood is the same MCU/ROM, different LCD.

***************************************************************************/

class tgaunt_state : public hh_sm510_state
{
public:
	tgaunt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void trobhood(machine_config &config);
	void tgaunt(machine_config &config);
};

// config

static INPUT_PORTS_START( tgaunt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Bomb")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Key")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

static INPUT_PORTS_START( trobhood )
	PORT_INCLUDE( tgaunt )

	PORT_MODIFY("IN.3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Money")
INPUT_PORTS_END

void tgaunt_state::tgaunt(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1425, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void tgaunt_state::trobhood(machine_config &config)
{
	tgaunt(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1468, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( tgaunt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "583", 0x0000, 0x1000, CRC(598d8156) SHA1(9f776e8b9b4321e8118481e6b1304f8a38f9932e) )

	ROM_REGION( 713020, "svg", 0)
	ROM_LOAD( "tgaunt.svg", 0, 713020, CRC(1f65ae21) SHA1(57ca33d073d1096a7fc17f2bdac940868d1ae651) )
ROM_END

ROM_START( trobhood )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "583", 0x0000, 0x1000, CRC(598d8156) SHA1(9f776e8b9b4321e8118481e6b1304f8a38f9932e) )

	ROM_REGION( 704892, "svg", 0)
	ROM_LOAD( "trobhood.svg", 0, 704892, CRC(291fd8db) SHA1(1de6bd0e203f16c44f7d661e44863a1a919f3da9) )
ROM_END





/***************************************************************************

  Tiger Double Dragon (model 7-780)
  * Sharp SM510 under epoxy (die label CMS54C, KMS570, 593)
  * lcd screen with custom segments, 1-bit sound

  BTANB: On the baddie in the background throwing dynamite, the sparks
  above his head are the same segment as the body, not the arm.

***************************************************************************/

class tddragon_state : public hh_sm510_state
{
public:
	tddragon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tddragon(machine_config &config);
};

// config

static INPUT_PORTS_START( tddragon )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sway")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch/Pick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Status")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tddragon_state::tddragon(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1467, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tddragon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "593", 0x0000, 0x1000, CRC(2642f778) SHA1(fee77acf93e057a8b4627389dfd481c6d9cbd02b) )

	ROM_REGION( 511434, "svg", 0)
	ROM_LOAD( "tddragon.svg", 0, 511434, CRC(641e7ceb) SHA1(bbfc37cc085e00921422f65d9aac9949f871e7b7) )
ROM_END





/***************************************************************************

  Tiger Karnov (model 7-783)
  * Sharp SM510 under epoxy (die label CMS54C, KMS582)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tkarnov_state : public hh_sm510_state
{
public:
	tkarnov_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tkarnov(machine_config &config);
};

// config

static INPUT_PORTS_START( tkarnov )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire-Ball")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Boomerang")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shield")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Max Score")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tkarnov_state::tkarnov(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1477, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tkarnov )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "582", 0x0000, 0x1000, CRC(cee85bdd) SHA1(143e39524f1dea523e0575f327ed189343cc87f5) )

	ROM_REGION( 527377, "svg", 0)
	ROM_LOAD( "tkarnov.svg", 0, 527377, CRC(971840fc) SHA1(48db7139fa875e60b44340fb475b6d1081ef5c10) )
ROM_END





/***************************************************************************

  Tiger Vindicators (model 7-786)
  * Sharp SM510 under epoxy (die label CMS54C, KMS595)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tvindictr_state : public hh_sm510_state
{
public:
	tvindictr_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tvindictr(machine_config &config);
};

// config

static INPUT_PORTS_START( tvindictr )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Gun Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Gun Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Max Score")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tvindictr_state::tvindictr(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1459, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tvindictr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "595", 0x0000, 0x1000, CRC(b574d16f) SHA1(d2cb0f2e21ca2defe49a4b45f4c8e169ae9979ab) )

	ROM_REGION( 314165, "svg", 0)
	ROM_LOAD( "tvindictr.svg", 0, 314165, CRC(2241992c) SHA1(efd44879d1c0d5befd7ea07089418406fc101315) )
ROM_END





/***************************************************************************

  Tiger Ninja Gaiden (model 7-787)
  * Sharp SM510 under epoxy (die label M82)
  * lcd screen with custom segments, 1 led, 1-bit sound

***************************************************************************/

class tgaiden_state : public hh_sm510_state
{
public:
	tgaiden_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag),
		m_led_out(*this, "led")
	{
		inp_fixed_last();
	}

	output_finder<> m_led_out;

	DECLARE_WRITE8_MEMBER(write_r);
	void tgaiden(machine_config &config);

protected:
	virtual void machine_start() override;
};

void tgaiden_state::machine_start()
{
	hh_sm510_state::machine_start();

	// resolve handlers
	m_led_out.resolve();
}

// handlers

WRITE8_MEMBER(tgaiden_state::write_r)
{
	// R1: speaker out
	piezo_r1_w(space, 0, data & 1);

	// R2: led
	m_led_out = data >> 1 & 1;
}

// config

static INPUT_PORTS_START( tgaiden )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack/Pick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tgaiden_state::tgaiden(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(tgaiden_state::write_r));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1920, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tgaiden )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m82", 0x0000, 0x1000, CRC(278eafb0) SHA1(14396a0010bade0fde705969151200ed432321e7) )

	ROM_REGION( 100000, "svg", 0)
	ROM_LOAD( "tgaiden.svg", 0, 100000, NO_DUMP )
ROM_END





/***************************************************************************

  Tiger Batman (model 7-799)
  * Sharp SM510 under epoxy (die label CMS54C, KMS597)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tbatman_state : public hh_sm510_state
{
public:
	tbatman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tbatman(machine_config &config);
};

// config

static INPUT_PORTS_START( tbatman )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Max Score")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tbatman_state::tbatman(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1442, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbatman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "597", 0x0000, 0x1000, CRC(8b7acc97) SHA1(fe811675dc5c5ef9f6f969685c933926c8b9e868) )

	ROM_REGION( 551890, "svg", 0)
	ROM_LOAD( "tbatman.svg", 0, 551890, CRC(65809ee3) SHA1(5fc38bdb2108d45dc99bce3379253423ea88e0fc) )
ROM_END





/***************************************************************************

  Tiger Space Harrier II (model 7-814)
  * Sharp SM510 under epoxy (die label M91)
  * lcd screen with custom segments, 1-bit sound

  known releases:
  - World: Space Harrier II
  - Japan: Space Harrier (published by Sega)

***************************************************************************/

class tsharr2_state : public hh_sm510_state
{
public:
	tsharr2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsharr2(machine_config &config);
};

// config

static INPUT_PORTS_START( tsharr2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Attack
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Max Score")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsharr2_state::tsharr2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1493, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsharr2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m91", 0x0000, 0x1000, CRC(b207ac79) SHA1(9889dfec26089313ba2bdac845a75a26742d09e1) )

	ROM_REGION( 555126, "svg", 0)
	ROM_LOAD( "tsharr2.svg", 0, 555126, CRC(ff43e29b) SHA1(0af02e65a1dcf95958296a292343430670b67ae5) )
ROM_END





/***************************************************************************

  Tiger Strider (model 7-815)
  * Sharp SM510 under epoxy (die label M92)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tstrider_state : public hh_sm510_state
{
public:
	tstrider_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tstrider(machine_config &config);
};

// config

static INPUT_PORTS_START( tstrider )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack/Pick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Weapon")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tstrider_state::tstrider(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1479, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tstrider )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m92", 0x0000, 0x1000, CRC(4b488e8f) SHA1(b037c220c4a456f0dac67d759736f202a7609ee5) )

	ROM_REGION( 554817, "svg", 0)
	ROM_LOAD( "tstrider.svg", 0, 554817, CRC(be5de6bd) SHA1(cde0a3fe21af24d7d22d2ce0aec9c308f8696c7e) )
ROM_END





/***************************************************************************

  Tiger Golden Axe (model 7-817)
  * Sharp SM510 under epoxy (die label M94)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tgoldnaxe_state : public hh_sm510_state
{
public:
	tgoldnaxe_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tgoldnaxe(machine_config &config);
};

// config

static INPUT_PORTS_START( tgoldnaxe )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Magic")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Right")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tgoldnaxe_state::tgoldnaxe(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1456, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tgoldnaxe )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m94", 0x0000, 0x1000, CRC(af183fbf) SHA1(23716e2a7c4bb4842b2af1a43fe88db44e18dc17) )

	ROM_REGION( 605483, "svg", 0)
	ROM_LOAD( "tgoldnaxe.svg", 0, 605483, CRC(533bea14) SHA1(08d419bd7af5de7216654dc7f978beed95192c2d) )
ROM_END





/***************************************************************************

  Tiger Robocop 2 (model 7-830), The Rocketeer (model 7-864)
  * Sharp SM510 under epoxy (die label M96)
  * lcd screen with custom segments, 1-bit sound

  The Rocketeer is the same MCU/ROM, different LCD.

***************************************************************************/

class trobocop2_state : public hh_sm510_state
{
public:
	trobocop2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void trobocop2(machine_config &config);
	void trockteer(machine_config &config);
};

// config

static INPUT_PORTS_START( trobocop2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Rescue")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("P1 Down/Pick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

static INPUT_PORTS_START( trockteer )
	PORT_INCLUDE( trobocop2 )

	PORT_MODIFY("IN.0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("P1 Up/Rocket Pack")

	PORT_MODIFY("IN.3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire Right")

	PORT_MODIFY("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire Left")
INPUT_PORTS_END

void trobocop2_state::trobocop2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1487, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void trobocop2_state::trockteer(machine_config &config)
{
	trobocop2(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1463, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( trobocop2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m96", 0x0000, 0x1000, CRC(3704b60c) SHA1(04275833e1a79fd33226faf060890b66ae54e1d3) )

	ROM_REGION( 463532, "svg", 0)
	ROM_LOAD( "trobocop2.svg", 0, 463532, CRC(c2b92868) SHA1(87912f02bea967c10ba1d8f7c810e3c44b0e3cff) )
ROM_END

ROM_START( trockteer )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m96", 0x0000, 0x1000, CRC(3704b60c) SHA1(04275833e1a79fd33226faf060890b66ae54e1d3) )

	ROM_REGION( 558086, "svg", 0)
	ROM_LOAD( "trockteer.svg", 0, 558086, CRC(8afe0f88) SHA1(702127a4ff72be492f72b24bd8917ae0e15f247d) )
ROM_END





/***************************************************************************

  Tiger Altered Beast (model 7-831)
  * Sharp SM510 under epoxy (die label M88)
  * lcd screen with custom segments, 1-bit sound

  known releases:
  - World: Altered Beast
  - Japan: Juuouki (published by Sega)

***************************************************************************/

class taltbeast_state : public hh_sm510_state
{
public:
	taltbeast_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void taltbeast(machine_config &config);
};

// config

static INPUT_PORTS_START( taltbeast )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick/Attack")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void taltbeast_state::taltbeast(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1455, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( taltbeast )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m88", 0x0000, 0x1000, CRC(1b3d15e7) SHA1(78371230dff872d6c07eefdbc4856c2a3336eb61) )

	ROM_REGION( 667887, "svg", 0)
	ROM_LOAD( "taltbeast.svg", 0, 667887, CRC(1ca9bbf1) SHA1(be844dddee4a95f70ea2adf875d3ee6cda2a6633) )
ROM_END





/***************************************************************************

  Tiger Street Fighter 2010 - The Final Fight (model 7-837)
  * Sharp SM510 under epoxy (die label MA2)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tsf2010_state : public hh_sm510_state
{
public:
	tsf2010_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsf2010(machine_config &config);
};

// config

static INPUT_PORTS_START( tsf2010 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select Planet")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsf2010_state::tsf2010(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1465, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsf2010 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma2", 0x0000, 0x1000, CRC(764b3757) SHA1(c5f90b860128658576bb837e9cabbb3045ad2756) )

	ROM_REGION( 595149, "svg", 0)
	ROM_LOAD( "tsf2010.svg", 0, 595149, CRC(b873856b) SHA1(1d070d4d9578bbc322d1edead208bbd44340b71a) )
ROM_END





/***************************************************************************

  Tiger Swamp Thing (model 7-851)
  * Sharp SM510 under epoxy (die label MB0)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tswampt_state : public hh_sm510_state
{
public:
	tswampt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tswampt(machine_config &config);
};

// config

static INPUT_PORTS_START( tswampt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch/Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Throw")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tswampt_state::tswampt(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1450, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tswampt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb0", 0x0000, 0x1000, CRC(8433530c) SHA1(60716d3bba92dc8ac3f1ee29c5734c9e894a1aff) )

	ROM_REGION( 578505, "svg", 0)
	ROM_LOAD( "tswampt.svg", 0, 578505, CRC(98ff2fbb) SHA1(a5a4e9934b86f69176549f99246b40f323441945) )
ROM_END





/***************************************************************************

  Tiger Spider-Man (model 7-853)
  * Sharp SM510 under epoxy (die label MA5)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tspidman_state : public hh_sm510_state
{
public:
	tspidman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tspidman(machine_config &config);
};

// config

static INPUT_PORTS_START( tspidman )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Right/Kick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Up/Punch")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Left/Rescue")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Status")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tspidman_state::tspidman(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1440, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tspidman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma5", 0x0000, 0x1000, CRC(2624daed) SHA1(7c10434ae899637264de706045d48e3fce1d30a7) )

	ROM_REGION( 605332, "svg", 0)
	ROM_LOAD( "tspidman.svg", 0, 605332, CRC(6e687727) SHA1(c1a2ee450509e05d1db61e02f6a911207d2830c4) )
ROM_END





/***************************************************************************

  Tiger X-Men (model 7-854)
  * Sharp SM510 under epoxy (die label MA7)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class txmen_state : public hh_sm510_state
{
public:
	txmen_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void txmen(machine_config &config);
};

// config

static INPUT_PORTS_START( txmen )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack/Pick Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch/Claws")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack/Pick Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void txmen_state::txmen(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1467, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( txmen )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma7", 0x0000, 0x1000, CRC(6f3ff34f) SHA1(aa24fbc3a4117ea51ebf951ee343a36c77692b72) )

	ROM_REGION( 543232, "svg", 0)
	ROM_LOAD( "txmen.svg", 0, 543232, CRC(51daf7f9) SHA1(b59ecbd83e05478f4b2654a019291c7e06893112) )
ROM_END





/***************************************************************************

  Tiger Double Dragon 3 - The Rosetta Stone (model 7-858)
  * Sharp SM510 under epoxy (die label MA6)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tddragon3_state : public hh_sm510_state
{
public:
	tddragon3_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tddragon3(machine_config &config);
};

// config

static INPUT_PORTS_START( tddragon3 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("P1 Down/Pick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Throw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tddragon3_state::tddragon3(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1514, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tddragon3 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma6", 0x0000, 0x1000, CRC(8e2da0d9) SHA1(54dd05124b4c605975b0cb1eadd7456ff4a94d68) )

	ROM_REGION( 615684, "svg", 0)
	ROM_LOAD( "tddragon3.svg", 0, 615684, CRC(3f5df090) SHA1(c9248fbf3a4dec0ce3b32b10fb67f133595cc54d) )
ROM_END





/***************************************************************************

  Tiger The Flash (model 7-859)
  * Sharp SM510 under epoxy (die label MB5)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tflash_state : public hh_sm510_state
{
public:
	tflash_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tflash(machine_config &config);
};

// config

static INPUT_PORTS_START( tflash )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch/Brake")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Run")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Tie")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // S7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tflash_state::tflash(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1444, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tflash )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb5", 0x0000, 0x1000, CRC(f7f1d082) SHA1(49a7a931450cf27fe69076c4e15ffb34814e25d4) )

	ROM_REGION( 587820, "svg", 0)
	ROM_LOAD( "tflash.svg", 0, 587820, CRC(aa1ad063) SHA1(aec6b15569d3d58ff9a4f7db779cda4a1c8efc35) )
ROM_END





/***************************************************************************

  Tiger MC Hammer: U Can't Touch This (model 7-863)
  * Sharp SM511 under epoxy (die label N63)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tmchammer_state : public hh_sm510_state
{
public:
	tmchammer_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tmchammer(machine_config &config);
};

// config

static INPUT_PORTS_START( tmchammer )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Leg Footwork Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Arm Up")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Leg Footwork Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Leg Leaps Up/Jump")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Leg Leaps Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Arm Splits Right")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Arm Splits Left")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Arm Down")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Mode")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tmchammer_state::tmchammer(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1471, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmchammer )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n63.program", 0x0000, 0x1000, CRC(303aa6f7) SHA1(296689be1ee05238e52e9882812868b2ea96202c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n63.melody", 0x000, 0x100, CRC(77c1a5a3) SHA1(c00ae3b7c64dd9db96eab520fe674a40571fc15f) )

	ROM_REGION( 456446, "svg", 0)
	ROM_LOAD( "tmchammer.svg", 0, 456446, CRC(79d6d45d) SHA1(bf6b8c6fdccad657377ad9f721dd22408f6ae775) )
ROM_END





/***************************************************************************

  Tiger Battletoads (model 7-868)
  * Sharp SM510 under epoxy (die label MB3)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tbtoads_state : public hh_sm510_state
{
public:
	tbtoads_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tbtoads(machine_config &config);
};

// config

static INPUT_PORTS_START( tbtoads )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Score")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tbtoads_state::tbtoads(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1454, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbtoads )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb3", 0x0000, 0x1000, CRC(8fa4c55a) SHA1(2be97e63dfed51313e180d7388dd431058db5a51) )

	ROM_REGION( 694365, "svg", 0)
	ROM_LOAD( "tbtoads.svg", 0, 694365, CRC(3af488e9) SHA1(d0e9ec61fac23bb22e508da4fa8bf2a7b8f186cf) )
ROM_END





/***************************************************************************

  Tiger Hook (model 7-869)
  * Sharp SM510 under epoxy (die label MB7)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class thook_state : public hh_sm510_state
{
public:
	thook_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void thook(machine_config &config);
};

// config

static INPUT_PORTS_START( thook )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sword Up")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sword Down/Dodge")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Swing Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack/Pick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Swing Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void thook_state::thook(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1489, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( thook )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb7", 0x0000, 0x1000, CRC(7eb1a6e2) SHA1(f4a09ab95c968b0ddbe56cd7bb2667881c145731) )

	ROM_REGION( 680503, "svg", 0)
	ROM_LOAD( "thook.svg", 0, 680503, CRC(28bd6da2) SHA1(e97b1dda219a766ffcca15d1b3279f5cee5e2fed) )
ROM_END





/***************************************************************************

  Tiger Back to the Future (model 7-809)
  * Sharp SM510 under epoxy (die label MC3)
  * lcd screen with custom segments, 1-bit sound

  This game is from 1992, even though the model number suggests otherwise.
  Perhaps Tiger filled unused model numbers before switching to 78-xxx.

***************************************************************************/

class tbttf_state : public hh_sm510_state
{
public:
	tbttf_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tbttf(machine_config &config);
};

// config

static INPUT_PORTS_START( tbttf )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Repair")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Brake")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Accelerate")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Rescue")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tbttf_state::tbttf(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1466, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbttf )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc3", 0x0000, 0x1000, CRC(9c37a23c) SHA1(c09fa5caac8b574f8460265b98c0bea1d5e78c6a) )

	ROM_REGION( 667700, "svg", 0)
	ROM_LOAD( "tbttf.svg", 0, 667700, CRC(d1d19ec5) SHA1(7361943ccf1f4072bba6fd4e6acae3e2f3d7a0ea) )
ROM_END





/***************************************************************************

  Tiger The Addams Family (model 7-829)
  * Sharp SM510 under epoxy (die label MC2)
  * lcd screen with custom segments, 1-bit sound

  Like Back to the Future, this game is newer than the model number suggests.

***************************************************************************/

class taddams_state : public hh_sm510_state
{
public:
	taddams_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void taddams(machine_config &config);
};

// config

static INPUT_PORTS_START( taddams )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Take")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Throw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void taddams_state::taddams(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1464, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( taddams )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc2", 0x0000, 0x1000, CRC(af33d432) SHA1(676ada238c389d1dd02dcb29731d69624f60b342) )

	ROM_REGION( 554649, "svg", 0)
	ROM_LOAD( "taddams.svg", 0, 554649, CRC(0b916c6d) SHA1(5a2456b4a0f31db94a78373baab46f3ff9732b92) )
ROM_END





/***************************************************************************

  Tiger Home Alone (model 78-???)
  * Sharp SM510 under epoxy (die label MC7)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class thalone_state : public hh_sm510_state
{
public:
	thalone_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void thalone(machine_config &config);
};

// config

static INPUT_PORTS_START( thalone )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Trap Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Climb")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Trap Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void thalone_state::thalone(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1448, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( thalone )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc7", 0x0000, 0x1000, CRC(eceda335) SHA1(20c9ffcf914db61aba03716fe146bac42873ac82) )

	ROM_REGION( 494235, "svg", 0)
	ROM_LOAD( "thalone.svg", 0, 494235, CRC(0e32df1d) SHA1(1fff1d37a5fe66d4f59d12af3ce67665c0049800) )
ROM_END





/***************************************************************************

  Tiger X-Men - Project X (model 78-504)
  * Sharp SM510 under epoxy (die label MD3)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class txmenpx_state : public hh_sm510_state
{
public:
	txmenpx_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void txmenpx(machine_config &config);
};

// config

static INPUT_PORTS_START( txmenpx )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Move")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void txmenpx_state::txmenpx(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1464, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( txmenpx )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "md3", 0x0000, 0x1000, CRC(11c2b09a) SHA1(f94b1e3e60f002398b39c98946469dd1a6aa8e77) )

	ROM_REGION( 572538, "svg", 0)
	ROM_LOAD( "txmenpx.svg", 0, 572538, CRC(9a89c753) SHA1(e3828a8c10c77ee5634128d0e9239e8cda19f988) )
ROM_END





/***************************************************************************

  Tiger Home Alone 2 - Lost in New York (model 78-506)
  * Sharp SM510 under epoxy (die label MD7)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class thalone2_state : public hh_sm510_state
{
public:
	thalone2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void thalone2(machine_config &config);
};

// config

static INPUT_PORTS_START( thalone2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Trap/Flash Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Climb/Pick")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Trap/Flash Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void thalone2_state::thalone2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1454, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( thalone2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "md7", 0x0000, 0x1000, CRC(ac8a21e9) SHA1(9024f74e34056f90b7dbf439300797183f74eb00) )

	ROM_REGION( 748886, "svg", 0)
	ROM_LOAD( "thalone2.svg", 0, 748886, CRC(a5d8898e) SHA1(de8fae0169a3797a46b5c81d9b556df636a5674e) )
ROM_END





/***************************************************************************

  Tiger Sonic The Hedgehog (model 78-513)
  * Sharp SM511 under epoxy (die label KMS73B, N71)
  * lcd screen with custom segments, 2-bit sound

***************************************************************************/

class tsonic_state : public hh_sm510_state
{
public:
	tsonic_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsonic(machine_config &config);
};

// config

static INPUT_PORTS_START( tsonic )
	PORT_START("IN.0") // S2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) // Jump Up
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) // Down
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Super Sonic Spin
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsonic_state::tsonic(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::piezo2bit_input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo2bit_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1517, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, piezo2bit_r1_120k_s1_39k);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsonic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n71.program", 0x0000, 0x1000, CRC(44cafd68) SHA1(bf8d0ab88d153fabc688ffec19959209ca79c3db) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n71.melody", 0x000, 0x100, CRC(bae258c8) SHA1(81cb75d73fab4479cd92fcb13d9cb03cec2afdd5) )

	ROM_REGION( 541450, "svg", 0)
	ROM_LOAD( "tsonic.svg", 0, 541450, CRC(f01835e3) SHA1(25f924af55ffadd2aebf50a89f75571d788d5ac1) )
ROM_END





/***************************************************************************

  Tiger Robocop 3 (model 78-514)
  * Sharp SM510 under epoxy (die label MC6)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class trobocop3_state : public hh_sm510_state
{
public:
	trobocop3_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void trobocop3(machine_config &config);
};

// config

static INPUT_PORTS_START( trobocop3 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump/Fly")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Rescue")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("P1 Down/Pick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void trobocop3_state::trobocop3(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1464, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( trobocop3 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc6", 0x0000, 0x1000, CRC(07b44e4c) SHA1(3165c85e16c062d2d9d0c0f1b1f6bd6079b4de15) )

	ROM_REGION( 612103, "svg", 0)
	ROM_LOAD( "trobocop3.svg", 0, 612103, CRC(9a162642) SHA1(b775f64e4616c4fc8d2c139938f148c9666e646a) )
ROM_END





/***************************************************************************

  Tiger The Incredible Crash Dummies (model 78-516)
  * Sharp SM510 under epoxy (die label ME0)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tdummies_state : public hh_sm510_state
{
public:
	tdummies_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tdummies(machine_config &config);
};

// config

static INPUT_PORTS_START( tdummies )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Crash")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Brake")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Accelerate")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tdummies_state::tdummies(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1441, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tdummies )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me0", 0x0000, 0x1000, CRC(29efae4a) SHA1(0b26913a3fd2fde2b39549f0f7cbc3daaa41eb50) )

	ROM_REGION( 525493, "svg", 0)
	ROM_LOAD( "tdummies.svg", 0, 525493, CRC(a18a5216) SHA1(1238e8c489445e715d4fc53e597820845b386233) )
ROM_END





/***************************************************************************

  Tiger Street Fighter II (model 78-522)
  * Sharp SM510 under epoxy (die label ME1)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tsfight2_state : public hh_sm510_state
{
public:
	tsfight2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsfight2(machine_config &config);
};

// config

static INPUT_PORTS_START( tsfight2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) // Down
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch/Special Move")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsfight2_state::tsfight2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1444, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsfight2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me1", 0x0000, 0x1000, CRC(73384e94) SHA1(350417d101ce034b3974b4a1d2e04bcb3bf70605) )

	ROM_REGION( 630403, "svg", 0)
	ROM_LOAD( "tsfight2.svg", 0, 630403, CRC(eadc2c81) SHA1(20b2a797f6b9a008c1994eaee7b87e3fe828e837) )
ROM_END





/***************************************************************************

  Tiger Wayne's World (model 78-523)
  * Sharp SM510 under epoxy (die label ME7)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class twworld_state : public hh_sm510_state
{
public:
	twworld_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void twworld(machine_config &config);
};

// config

static INPUT_PORTS_START( twworld )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("High Five")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Hockey Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot/Cassandra")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Hockey Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void twworld_state::twworld(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1429, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( twworld )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me7", 0x0000, 0x1000, CRC(dcb16d98) SHA1(539989e12bbc4a719818546c5edcfda02b98210e) )

	ROM_REGION( 527859, "svg", 0)
	ROM_LOAD( "twworld.svg", 0, 527859, CRC(0a2cffce) SHA1(d8c3f2fef60357e47ce0b44d588d0bb39112c8b9) )
ROM_END





/***************************************************************************

  Tiger Jurassic Park (model 78-524)
  * Sharp SM510 under epoxy (die label MF4)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tjpark_state : public hh_sm510_state
{
public:
	tjpark_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tjpark(machine_config &config);
};

// config

static INPUT_PORTS_START( tjpark )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump/Climb")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Swing/Power")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Forward")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Call")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tjpark_state::tjpark(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1454, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tjpark )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf4", 0x0000, 0x1000, CRC(f66faf73) SHA1(4cfa743dcd6e44a3c1f56206d5824fddba16df01) )

	ROM_REGION( 812575, "svg", 0)
	ROM_LOAD( "tjpark.svg", 0, 812575, CRC(539c9b9c) SHA1(bf9a95586438df677d753deb17abc97f8837cbe3) )
ROM_END





/***************************************************************************

  Tiger Sonic The Hedgehog 2 (model 78-527)
  * Sharp SM511 under epoxy (die label N86)
  * lcd screen with custom segments, 2-bit sound

***************************************************************************/

class tsonic2_state : public hh_sm510_state
{
public:
	tsonic2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsonic2(machine_config &config);
};

// config

static INPUT_PORTS_START( tsonic2 )
	PORT_START("IN.0") // S2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Action
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsonic2_state::tsonic2(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::piezo2bit_input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo2bit_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1475, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, piezo2bit_r1_120k_s1_39k);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsonic2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n86.program", 0x0000, 0x1000, CRC(782874c5) SHA1(b7eb1f56cbc781ba0b90f6b4b5b51944120733cc) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n86.melody", 0x000, 0x100, CRC(c16fa2b2) SHA1(222772d311fd3b3b05d80cfd539c2c862bed0be5) )

	ROM_REGION( 667887, "svg", 0)
	ROM_LOAD( "tsonic2.svg", 0, 667887, CRC(ef82d40e) SHA1(f22efba565adb32634d8b46c31459ec833b13d98) )
ROM_END





/***************************************************************************

  Tiger Super Double Dragon (model 78-528)
  * Sharp SM510 under epoxy (die label MF5)
  * lcd screen with custom segments, 1-bit sound

  BTANB: The player char right arm muscle is part of the right kick segment.
  They probably meant to use it for the right punch segment, but this is
  how it shows on the LCD.

***************************************************************************/

class tsddragon_state : public hh_sm510_state
{
public:
	tsddragon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsddragon(machine_config &config);
};

// config

static INPUT_PORTS_START( tsddragon )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("P1 Down/Pick")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Special Technique")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsddragon_state::tsddragon(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1503, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsddragon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf5", 0x0000, 0x1000, CRC(264c8e82) SHA1(470eb2f09a58ef05eb0b7c8e11380ad1d8ce4e1a) )

	ROM_REGION( 753533, "svg", 0)
	ROM_LOAD( "tsddragon.svg", 0, 753533, CRC(fb526049) SHA1(552fe005a6e23e083867b7d1c10d20daa8913a14) )
ROM_END





/***************************************************************************

  Tiger Dennis the Menace (model 78-532)
  * Sharp SM510 under epoxy (die label MF9)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tdennis_state : public hh_sm510_state
{
public:
	tdennis_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tdennis(machine_config &config);
};

// config

static INPUT_PORTS_START( tdennis )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Slingshot Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Brake")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick Left")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Slingshot Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick Right")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tdennis_state::tdennis(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1467, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tdennis )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf9", 0x0000, 0x1000, CRC(d95f54d5) SHA1(1b3a170f32deec98e54ad09c04b404f5ae03dcea) )

	ROM_REGION( 754842, "svg", 0)
	ROM_LOAD( "tdennis.svg", 0, 754842, CRC(3b1ed476) SHA1(adc94919daa9a6c42f1acd8ef5113b61859338b7) )
ROM_END





/***************************************************************************

  Tiger Nightmare Before Christmas (model 78-537)
  * Sharp SM510 under epoxy (die label MG0)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tnmarebc_state : public hh_sm510_state
{
public:
	tnmarebc_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	virtual DECLARE_WRITE8_MEMBER(input_w) override;
	void tnmarebc(machine_config &config);
};

// handlers

WRITE8_MEMBER(tnmarebc_state::input_w)
{
	// S5 and S6 tied together
	hh_sm510_state::input_w(space, 0, (data & 0x1f) | (data >> 1 & 0x10));
}

// config

static INPUT_PORTS_START( tnmarebc )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Action
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5/S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tnmarebc_state::tnmarebc(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(tnmarebc_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1456, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tnmarebc )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg0", 0x0000, 0x1000, CRC(5ef21421) SHA1(8fd458575111b89d7c33c969e76703bde5ad2c36) )

	ROM_REGION( 631310, "svg", 0)
	ROM_LOAD( "tnmarebc.svg", 0, 631310, CRC(f9c96205) SHA1(1947d358efd94ae3257ed959172a819798d2c9a1) )
ROM_END





/***************************************************************************

  Tiger Transformers - Generation 2 (model 78-541)
  * Sharp SM510 under epoxy (die label MG2)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class ttransf2_state : public hh_sm510_state
{
public:
	ttransf2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void ttransf2(machine_config &config);
};

// config

static INPUT_PORTS_START( ttransf2 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Call")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Transform")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void ttransf2_state::ttransf2(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1476, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ttransf2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg2", 0x0000, 0x1000, CRC(65c0f456) SHA1(b1bc3887c5088b3fe359585658e5c5236c09af9e) )

	ROM_REGION( 727662, "svg", 0)
	ROM_LOAD( "ttransf2.svg", 0, 727662, CRC(52fd5ea1) SHA1(35ae9fe2cea14ee4c591df0458fed478c9feb044) )
ROM_END





/***************************************************************************

  Tiger Operation: Aliens (model 78-552)
  * Sharp SM510 under epoxy (die label MJ1)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class topaliens_state : public hh_sm510_state
{
public:
	topaliens_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void topaliens(machine_config &config);
};

// config

static INPUT_PORTS_START( topaliens )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void topaliens_state::topaliens(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1450, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( topaliens )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mj1", 0x0000, 0x1000, CRC(ccc196cf) SHA1(f18f7cf842cddecf90d05ab0f90257bb76514f54) )

	ROM_REGION( 1214876, "svg", 0)
	ROM_LOAD( "topaliens.svg", 0, 1214876, CRC(683c70aa) SHA1(0fac5ba8ab5f9b73a3cbbff046be60550fa5f98a) )
ROM_END





/***************************************************************************

  Tiger Mortal Kombat (model 78-553)
  * Sharp SM510 under epoxy (die label MG6)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tmkombat_state : public hh_sm510_state
{
public:
	tmkombat_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tmkombat(machine_config &config);
};

// config

static INPUT_PORTS_START( tmkombat )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch High")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick High")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Low")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick Low")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Select")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tmkombat_state::tmkombat(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1468, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmkombat )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg6", 0x0000, 0x1000, CRC(f6375dc7) SHA1(a711199c2623979f19c11067ebfff9355256c2c3) )

	ROM_REGION( 841829, "svg", 0)
	ROM_LOAD( "tmkombat.svg", 0, 841829, CRC(9dc4f58c) SHA1(9c9b080d7f3b777407445c22195990c55c6352ca) )
ROM_END





/***************************************************************************

  Tiger The Shadow (model 78-559)
  * Sharp SM510 under epoxy (die label MJ5)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tshadow_state : public hh_sm510_state
{
public:
	tshadow_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tshadow(machine_config &config);
};

// config

static INPUT_PORTS_START( tshadow )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shadow")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tshadow_state::tshadow(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1484, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tshadow )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mj5", 0x0000, 0x1000, CRC(09822d73) SHA1(30cae8b783a4f388193aee248fa18c6c1042e0ec) )

	ROM_REGION( 946450, "svg", 0)
	ROM_LOAD( "tshadow.svg", 0, 946450, CRC(5cab680a) SHA1(8f8f660c08fc56287362b11c183655047fbd91ca) )
ROM_END





/***************************************************************************

  Tiger Skeleton Warriors - The Dark Crusade (model 78-569)
  * Sharp SM510 under epoxy (die label MK0)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tskelwarr_state : public hh_sm510_state
{
public:
	tskelwarr_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tskelwarr(machine_config &);
};

// config

static INPUT_PORTS_START( tskelwarr )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Right")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick/Call")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Attack Left")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tskelwarr_state::tskelwarr(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1444, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tskelwarr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk0", 0x0000, 0x1000, CRC(dc7827a1) SHA1(74ff143605684df0c70db604a5f22dbf512044d7) )

	ROM_REGION( 1125002, "svg", 0)
	ROM_LOAD( "tskelwarr.svg", 0, 1125002, CRC(49c6ca24) SHA1(71f4ed98ab558deeb86820b7fbf7534a7b7d6b01) )
ROM_END





/***************************************************************************

  Tiger Batman Forever - Double Dose of Doom (model 78-572)
  * Sharp SM510 under epoxy (die label MK3)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tbatfor_state : public hh_sm510_state
{
public:
	tbatfor_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tbatfor(machine_config &config);
};

// config

static INPUT_PORTS_START( tbatfor )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Grappling Gun")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Help")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Batarang")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Thruster")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tbatfor_state::tbatfor(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1493, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbatfor )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk3", 0x0000, 0x1000, CRC(9993c382) SHA1(0c89e21024315ce7c086af5390c60f5766028c4f) )

	ROM_REGION( 902364, "svg", 0)
	ROM_LOAD( "tbatfor.svg", 0, 902364, CRC(56889c05) SHA1(dda393ca99196de38ad2e989ec6c292adc36ec5e) )
ROM_END





/***************************************************************************

  Tiger Judge Dredd (model 78-581)
  * Sharp SM510 under epoxy (die label MK5)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tjdredd_state : public hh_sm510_state
{
public:
	tjdredd_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tjdredd(machine_config &config);
};

// config

static INPUT_PORTS_START( tjdredd )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick/Call")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tjdredd_state::tjdredd(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1444, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tjdredd )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk5", 0x0000, 0x1000, CRC(7beee5a7) SHA1(9a190197c5751b43a9ab2dc8c536934dc5fc5e83) )

	ROM_REGION( 1051586, "svg", 0)
	ROM_LOAD( "tjdredd.svg", 0, 1051586, CRC(4fcdca0a) SHA1(d4b019fec94890ba6600baf2b2096dbcf3295180) )
ROM_END





/***************************************************************************

  Tiger Apollo 13 (model 78-591)
  * Sharp SM510 under epoxy (die label 10 07)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tapollo13_state : public hh_sm510_state
{
public:
	tapollo13_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tapollo13(machine_config &config);
};

// config

static INPUT_PORTS_START( tapollo13 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Thruster Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Parachute")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Thruster Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Capture")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tapollo13_state::tapollo13(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1467, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tapollo13 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_07", 0x0000, 0x1000, CRC(63d0deaa) SHA1(d5de99d5e0ee08ec2ebeef7189ebac1c008d2e7d) )

	ROM_REGION( 643176, "svg", 0)
	ROM_LOAD( "tapollo13.svg", 0, 643176, CRC(e2dac162) SHA1(4089fa485579d2b87ac49b1cf33d6c2c085ea4c5) )
ROM_END





/***************************************************************************

  Tiger 007: GoldenEye (model 78-594)
  * Sharp SM510 under epoxy (die label 10 06)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tgoldeye_state : public hh_sm510_state
{
public:
	tgoldeye_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tgoldeye(machine_config &config);
};

// config

static INPUT_PORTS_START( tgoldeye )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Kick")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Q")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tgoldeye_state::tgoldeye(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1461, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tgoldeye )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_06", 0x0000, 0x1000, CRC(fe053efb) SHA1(3c90c0fa43e6e5e1f76b306e402f902d19175c96) )

	ROM_REGION( 938916, "svg", 0)
	ROM_LOAD( "tgoldeye.svg", 0, 938916, CRC(6dddf962) SHA1(1ced43b4225b86eca415f9af7db5fb5e80040186) )
ROM_END





/***************************************************************************

  Tiger Space Jam (model 78-621)
  * Sharp SM510 under epoxy (die label KMS10, 23)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tsjam_state : public hh_sm510_state
{
public:
	tsjam_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tsjam(machine_config &config);
};

// config

static INPUT_PORTS_START( tsjam )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shoot/Block")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Tune/Steal")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tsjam_state::tsjam(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu); // no external XTAL
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1421, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tsjam )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_23", 0x0000, 0x1000, CRC(6eaabfbd) SHA1(f0ecbd6f65fe72ce2d8a452685be2e77a63fc9f0) )

	ROM_REGION( 1046147, "svg", 0)
	ROM_LOAD( "tsjam.svg", 0, 1046147, CRC(6d24e1c9) SHA1(ddbfbd85f70ec964c68f982a8ee8070e3786a85e) )
ROM_END





/***************************************************************************

  Tiger Independence Day (model 78-624)
  * Sharp SM510 under epoxy (die label 10 16)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tinday_state : public hh_sm510_state
{
public:
	tinday_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tinday(machine_config &config);
};

// config

static INPUT_PORTS_START( tinday )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Shield")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fire")
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alert")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Velocity")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tinday_state::tinday(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1463, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tinday )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_16", 0x0000, 0x1000, CRC(77c2c2f7) SHA1(06326b26d0f6757180724ba0bdeb4110cc7e29d6) )

	ROM_REGION( 1162672, "svg", 0)
	ROM_LOAD( "tinday.svg", 0, 1162672, CRC(9b9a8047) SHA1(2aeaa71a54cf897d2a5d91133c733613ca229aae) )
ROM_END





/***************************************************************************

  Tiger Batman: The Animated Series (model 72-505)
  * Sharp SM511 under epoxy (die label N81)
  * lcd screen with custom segments, 2-bit sound

***************************************************************************/

class tbatmana_state : public hh_sm510_state
{
public:
	tbatmana_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tbatmana(machine_config &config);
};

// config

static INPUT_PORTS_START( tbatmana )
	PORT_START("IN.0") // S2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_NAME("Jump")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Fast")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed)  PORT_NAME("Throw/Attack")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S6
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Max Score")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // GND!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_CB(input_changed) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void tbatmana_state::tbatmana(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::piezo2bit_input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo2bit_r1_w));
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1478, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->set_levels(4, piezo2bit_r1_120k_s1_39k);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tbatmana )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n81.program", 0x0000, 0x1000, CRC(efb3f122) SHA1(d55c2fb92fb9bd41d6001f42143691b84f3f389a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n81.melody", 0x000, 0x100, CRC(56ba8fe5) SHA1(5c286ae1bfc943bbe8c8f4cdc9c8b73d9b3c186e) )

	ROM_REGION( 618831, "svg", 0)
	ROM_LOAD( "tbatmana.svg", 0, 618831, CRC(fc38cb9d) SHA1(1b6c10dcd33bfcfef43d61f97fa8e530011c1e61) )
ROM_END





/***************************************************************************

  Tronica Shuttle Voyage (MG-8)
  * Sharp SM510 label 0019 238E TRONICA (no decap)
  * lcd screen with custom segments, 1-bit sound

  Even though the serial is MG-8, the back of the game says 1983, newer than MG-9?
  Thief in Garden (model TG-18) is the exact same MCU, but different graphics.

***************************************************************************/

class trshutvoy_state : public hh_sm510_state
{
public:
	trshutvoy_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void trshutvoy(machine_config &config);
	void tigarden(machine_config &config);
};

// config

static INPUT_PORTS_START( trshutvoy )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("3")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("7")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("=")

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHANGED_CB(input_changed) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_CHANGED_CB(input_changed) PORT_NAME("ALM")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_CHANGED_CB(input_changed) PORT_NAME("%")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CHANGED_CB(input_changed) PORT_NAME("C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // S7
	PORT_BIT( 0x01, 0x01, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // Up/Sound
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // F

	PORT_START("IN.7") // S8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_CHANGED_CB(input_changed) PORT_NAME("Mode")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("FAKE") // Up/Sound are electronically the same button
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(input_changed) PORT_NAME("Sound")
INPUT_PORTS_END

void trshutvoy_state::trshutvoy(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1496, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void trshutvoy_state::tigarden(machine_config &config)
{
	trshutvoy(config);

	/* video hardware */
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(1515, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( trshutvoy )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0019_238e", 0x0000, 0x1000, CRC(8bd0eadd) SHA1(7bb5eb30d569901dce52d777bc01c0979e4afa06) )

	ROM_REGION( 221654, "svg", 0)
	ROM_LOAD( "trshutvoy.svg", 0, 221654, CRC(470a7ff5) SHA1(b297601d8a5a9c4aef414605632849e0b1925caa) )
ROM_END

ROM_START( tigarden )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0019_238e", 0x0000, 0x1000, CRC(8bd0eadd) SHA1(7bb5eb30d569901dce52d777bc01c0979e4afa06) )

	ROM_REGION( 409084, "svg", 0)
	ROM_LOAD( "tigarden.svg", 0, 409084, CRC(cfda5138) SHA1(1bc4ed65ae0cdca3e1e9458d68ca4d6e0fc0e901) )
ROM_END





/***************************************************************************

  Tronica Space Rescue (model MG-9)
  * PCB label MG-9 080492
  * Sharp SM510 label 0015 224B TRONICA (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class trsrescue_state : public hh_sm510_state
{
public:
	trsrescue_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void trsrescue(machine_config &config);
};

// config

static INPUT_PORTS_START( trsrescue )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void trsrescue_state::trsrescue(machine_config &config)
{
	/* basic machine hardware */
	SM510(config, m_maincpu);
	m_maincpu->set_r_mask_option(2); // confirmed
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1533, 1080);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( trsrescue )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0015_224b", 0x0000, 0x1000, CRC(f58a3832) SHA1(2d843b3520de66463e628cea9344a04015d1f5f1) )

	ROM_REGION( 178600, "svg", 0)
	ROM_LOAD( "trsrescue.svg", 0, 178600, CRC(2fa7b2d9) SHA1(5d1fc88db3129c9126f0c05ea55fb5f117e02871) )
ROM_END





/***************************************************************************

  VTech Electronic Number Muncher
  * Sharp SM511 under epoxy (die label 772)
  * lcd screen with custom segments(no background), 1-bit sound

***************************************************************************/

class nummunch_state : public hh_sm510_state
{
public:
	nummunch_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void nummunch(machine_config &config);
};

// config

static INPUT_PORTS_START( nummunch )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CHANGED_CB(input_changed)  PORT_NAME("Calc. / Clear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("=")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_CHANGED_CB(input_changed) PORT_NAME("Count")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHANGED_CB(input_changed) PORT_NAME("Quiz")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHANGED_CB(input_changed) PORT_NAME("Choose +")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_CHANGED_CB(input_changed) PORT_NAME("Choose -")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHANGED_CB(input_changed) PORT_NAME("Choose " UTF8_MULTIPLY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHANGED_CB(input_changed) PORT_NAME("Choose " UTF8_DIVIDE)

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_CHANGED_CB(input_changed) PORT_NAME("Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHANGED_CB(input_changed) PORT_NAME("Right")

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("2")

	PORT_START("IN.6") // S7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("6")

	PORT_START("IN.7") // S8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHANGED_CB(input_changed) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHANGED_CB(input_changed) PORT_NAME(UTF8_DIVIDE)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_CB(acl_button)
INPUT_PORTS_END

void nummunch_state::nummunch(machine_config &config)
{
	/* basic machine hardware */
	SM511(config, m_maincpu);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_svg_region("svg");
	screen.set_refresh_hz(50);
	screen.set_size(1920, 875);
	screen.set_visarea_full();

	TIMER(config, "display_decay").configure_periodic(FUNC(hh_sm510_state::display_decay_tick), attotime::from_msec(1));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( nummunch )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "772.program", 0x0000, 0x1000, CRC(2f7ff516) SHA1(132e7c5c4d69170953b2e51731992d6d6ba829f9) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "772.melody", 0x000, 0x100, CRC(96fe463a) SHA1(dcef5eee15a3f6d21e0db1b8ae3fbddc81633fc8) )

	ROM_REGION( 140664, "svg", 0)
	ROM_LOAD( "nummunch.svg", 0, 140664, CRC(879df7e2) SHA1(78d8500a445cbbea0090d4e97b781c1e4ed11dd3) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME         PARENT   COMP  MACHINE      INPUT        CLASS              INIT        COMPANY, FULLNAME, FLAGS
// Konami
CONS( 1989, kdribble,    0,          0, kdribble,    kdribble,    kdribble_state,    empty_init, "Konami", "Double Dribble (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, ktopgun,     0,          0, ktopgun,     ktopgun,     ktopgun_state,     empty_init, "Konami", "Top Gun (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kcontra,     0,          0, kcontra,     kcontra,     kcontra_state,     empty_init, "Konami", "Contra (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, ktmnt,       0,          0, ktmnt,       ktmnt,       ktmnt_state,       empty_init, "Konami", "Teenage Mutant Ninja Turtles (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kgradius,    0,          0, kgradius,    kgradius,    kgradius_state,    empty_init, "Konami", "Gradius (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kloneran,    0,          0, kloneran,    kloneran,    kloneran_state,    empty_init, "Konami", "Lone Ranger (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kblades,     0,          0, kblades,     kblades,     kblades_state,     empty_init, "Konami", "Blades of Steel (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, knfl,        0,          0, knfl,        knfl,        knfl_state,        empty_init, "Konami", "NFL Football (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kbilly,      0,          0, kbilly,      kbilly,      kbilly_state,      empty_init, "Konami", "The Adventures of Bayou Billy (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, kbucky,      0,          0, kbucky,      kbucky,      kbucky_state,      empty_init, "Konami", "Bucky O'Hare (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, kgarfld,     0,          0, kgarfld,     kgarfld,     kgarfld_state,     empty_init, "Konami", "Garfield (handheld)", MACHINE_SUPPORTS_SAVE )

// Nintendo G&W: wide screen
CONS( 1981, gnw_pchute,  0,          0, gnw_pchute,  gnw_pchute,  gnw_pchute_state,  empty_init, "Nintendo", "Game & Watch: Parachute", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_octopus, 0,          0, gnw_octopus, gnw_octopus, gnw_octopus_state, empty_init, "Nintendo", "Game & Watch: Octopus", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_popeye,  0,          0, gnw_popeye,  gnw_popeye,  gnw_popeye_state,  empty_init, "Nintendo", "Game & Watch: Popeye (wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_chef,    0,          0, gnw_chef,    gnw_chef,    gnw_chef_state,    empty_init, "Nintendo", "Game & Watch: Chef", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1989, merrycook,   gnw_chef,   0, merrycook,   gnw_chef,    gnw_chef_state,    empty_init, "Elektronika", "Merry Cook", MACHINE_SUPPORTS_SAVE)
CONS( 1981, gnw_mmouse,  0,          0, gnw_mmouse,  gnw_mmouse,  gnw_mmouse_state,  empty_init, "Nintendo", "Game & Watch: Mickey Mouse", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_egg,     gnw_mmouse, 0, gnw_egg,     gnw_mmouse,  gnw_mmouse_state,  empty_init, "Nintendo", "Game & Watch: Egg", MACHINE_SUPPORTS_SAVE )
CONS( 1984, nupogodi,    gnw_mmouse, 0, nupogodi,    gnw_mmouse,  gnw_mmouse_state,  empty_init, "Elektronika", "Nu, pogodi!", MACHINE_SUPPORTS_SAVE )
CONS( 1989, exospace,    gnw_mmouse, 0, exospace,    exospace,    gnw_mmouse_state,  empty_init, "Elektronika", "Explorers of Space", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_fire,    0,          0, gnw_fire,    gnw_fire,    gnw_fire_state,    empty_init, "Nintendo", "Game & Watch: Fire (wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_tbridge, 0,          0, gnw_tbridge, gnw_tbridge, gnw_tbridge_state, empty_init, "Nintendo", "Game & Watch: Turtle Bridge", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_fireatk, 0,          0, gnw_fireatk, gnw_fireatk, gnw_fireatk_state, empty_init, "Nintendo", "Game & Watch: Fire Attack", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_stennis, 0,          0, gnw_stennis, gnw_stennis, gnw_stennis_state, empty_init, "Nintendo", "Game & Watch: Snoopy Tennis", MACHINE_SUPPORTS_SAVE )

// Nintendo G&W: multi screen
CONS( 1982, gnw_opanic,  0,          0, gnw_opanic,  gnw_opanic,  gnw_opanic_state,  empty_init, "Nintendo", "Game & Watch: Oil Panic", MACHINE_SUPPORTS_SAVE)
CONS( 1982, gnw_dkong,   0,          0, gnw_dkong,   gnw_dkong,   gnw_dkong_state,   empty_init, "Nintendo", "Game & Watch: Donkey Kong", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_mickdon, 0,          0, gnw_mickdon, gnw_mickdon, gnw_mickdon_state, empty_init, "Nintendo", "Game & Watch: Mickey & Donald", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_ghouse,  0,          0, gnw_ghouse,  gnw_ghouse,  gnw_ghouse_state,  empty_init, "Nintendo", "Game & Watch: Green House", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_dkong2,  0,          0, gnw_dkong2,  gnw_dkong2,  gnw_dkong2_state,  empty_init, "Nintendo", "Game & Watch: Donkey Kong II", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_mario,   0,          0, gnw_mario,   gnw_mario,   gnw_mario_state,   empty_init, "Nintendo", "Game & Watch: Mario Bros.", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_rshower, 0,          0, gnw_rshower, gnw_rshower, gnw_rshower_state, empty_init, "Nintendo", "Game & Watch: Rain Shower", MACHINE_SUPPORTS_SAVE)
CONS( 1983, gnw_lboat,   0,          0, gnw_lboat,   gnw_lboat,   gnw_lboat_state,   empty_init, "Nintendo", "Game & Watch: Lifeboat", MACHINE_SUPPORTS_SAVE)
CONS( 1985, gnw_bjack,   0,          0, gnw_bjack,   gnw_bjack,   gnw_bjack_state,   empty_init, "Nintendo", "Game & Watch: Black Jack", MACHINE_SUPPORTS_SAVE)
CONS( 1986, gnw_squish,  0,          0, gnw_squish,  gnw_squish,  gnw_squish_state,  empty_init, "Nintendo", "Game & Watch: Squish", MACHINE_SUPPORTS_SAVE )
CONS( 1989, gnw_zelda,   0,          0, gnw_zelda,   gnw_zelda,   gnw_zelda_state,   empty_init, "Nintendo", "Game & Watch: Zelda", MACHINE_SUPPORTS_SAVE )

// Nintendo G&W: new wide screen
CONS( 1982, gnw_dkjr,    0,          0, gnw_dkjr,    gnw_dkjr,    gnw_dkjr_state,    empty_init, "Nintendo", "Game & Watch: Donkey Kong Jr. (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_mariocm, 0,          0, gnw_mariocm, gnw_mariocm, gnw_mariocm_state, empty_init, "Nintendo", "Game & Watch: Mario's Cement Factory (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_manhole, 0,          0, gnw_manhole, gnw_manhole, gnw_manhole_state, empty_init, "Nintendo", "Game & Watch: Manhole (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1985, gnw_tfish,   0,          0, gnw_tfish,   gnw_tfish,   gnw_tfish_state,   empty_init, "Nintendo", "Game & Watch: Tropical Fish", MACHINE_SUPPORTS_SAVE )
CONS( 1988, gnw_smb,     0,          0, gnw_smb,     gnw_smb,     gnw_smb_state,     empty_init, "Nintendo", "Game & Watch: Super Mario Bros. (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1988, gnw_climber, 0,          0, gnw_climber, gnw_climber, gnw_climber_state, empty_init, "Nintendo", "Game & Watch: Climber (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1988, gnw_bfight,  0,          0, gnw_bfight,  gnw_bfight,  gnw_bfight_state,  empty_init, "Nintendo", "Game & Watch: Balloon Fight (new wide screen)", MACHINE_SUPPORTS_SAVE )

// Nintendo G&W: crystal screen
CONS( 1986, gnw_climbcs, gnw_climber,0, gnw_climbcs, gnw_climber, gnw_climber_state, empty_init, "Nintendo", "Game & Watch: Climber (crystal screen)", MACHINE_SUPPORTS_SAVE )

// Nintendo G&W: micro vs. system (actually, no official Game & Watch logo anywhere)
CONS( 1984, gnw_boxing,  0,          0, gnw_boxing,  gnw_boxing,  gnw_boxing_state,  empty_init, "Nintendo", "Micro Vs. System: Boxing", MACHINE_SUPPORTS_SAVE )

// Tiger 7-xxx/78-xxx models
CONS( 1989, tgaunt,      0,          0, tgaunt,      tgaunt,      tgaunt_state,      empty_init, "Tiger Electronics (licensed from Tengen)", "Gauntlet (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, trobhood,    tgaunt,     0, trobhood,    trobhood,    tgaunt_state,      empty_init, "Tiger Electronics", "Robin Hood (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, tddragon,    0,          0, tddragon,    tddragon,    tddragon_state,    empty_init, "Tiger Electronics (licensed from Technos/Tradewest)", "Double Dragon (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, tkarnov,     0,          0, tkarnov,     tkarnov,     tkarnov_state,     empty_init, "Tiger Electronics (licensed from Data East)", "Karnov (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, tvindictr,   0,          0, tvindictr,   tvindictr,   tvindictr_state,   empty_init, "Tiger Electronics (licensed from Tengen)", "Vindicators (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, tgaiden,     0,          0, tgaiden,     tgaiden,     tgaiden_state,     empty_init, "Tiger Electronics (licensed from Tecmo)", "Ninja Gaiden (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1989, tbatman,     0,          0, tbatman,     tbatman,     tbatman_state,     empty_init, "Tiger Electronics", "Batman (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, tsharr2,     0,          0, tsharr2,     tsharr2,     tsharr2_state,     empty_init, "Tiger Electronics (licensed from Sega)", "Space Harrier II (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, tstrider,    0,          0, tstrider,    tstrider,    tstrider_state,    empty_init, "Tiger Electronics (licensed from Capcom)", "Strider (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, tgoldnaxe,   0,          0, tgoldnaxe,   tgoldnaxe,   tgoldnaxe_state,   empty_init, "Tiger Electronics (licensed from Sega)", "Golden Axe (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, trobocop2,   0,          0, trobocop2,   trobocop2,   trobocop2_state,   empty_init, "Tiger Electronics", "Robocop 2 (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, trockteer,   trobocop2,  0, trockteer,   trockteer,   trobocop2_state,   empty_init, "Tiger Electronics", "The Rocketeer (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, taltbeast,   0,          0, taltbeast,   taltbeast,   taltbeast_state,   empty_init, "Tiger Electronics (licensed from Sega)", "Altered Beast (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1990, tsf2010,     0,          0, tsf2010,     tsf2010,     tsf2010_state,     empty_init, "Tiger Electronics (licensed from Capcom)", "Street Fighter 2010 - The Final Fight (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tswampt,     0,          0, tswampt,     tswampt,     tswampt_state,     empty_init, "Tiger Electronics", "Swamp Thing (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tspidman,    0,          0, tspidman,    tspidman,    tspidman_state,    empty_init, "Tiger Electronics", "Spider-Man (handheld, Tiger 1991 version)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, txmen,       0,          0, txmen,       txmen,       txmen_state,       empty_init, "Tiger Electronics", "X-Men (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tddragon3,   0,          0, tddragon3,   tddragon3,   tddragon3_state,   empty_init, "Tiger Electronics (licensed from Technos)", "Double Dragon 3 - The Rosetta Stone (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tflash,      0,          0, tflash,      tflash,      tflash_state,      empty_init, "Tiger Electronics", "The Flash (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tmchammer,   0,          0, tmchammer,   tmchammer,   tmchammer_state,   empty_init, "Tiger Electronics", "MC Hammer: U Can't Touch This (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tbtoads,     0,          0, tbtoads,     tbtoads,     tbtoads_state,     empty_init, "Tiger Electronics (licensed from Rare/Tradewest)", "Battletoads (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, thook,       0,          0, thook,       thook,       thook_state,       empty_init, "Tiger Electronics", "Hook (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, tbttf,       0,          0, tbttf,       tbttf,       tbttf_state,       empty_init, "Tiger Electronics", "Back to the Future (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, taddams,     0,          0, taddams,     taddams,     taddams_state,     empty_init, "Tiger Electronics", "The Addams Family (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, thalone,     0,          0, thalone,     thalone,     thalone_state,     empty_init, "Tiger Electronics", "Home Alone (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, txmenpx,     0,          0, txmenpx,     txmenpx,     txmenpx_state,     empty_init, "Tiger Electronics", "X-Men - Project X (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, thalone2,    0,          0, thalone2,    thalone2,    thalone2_state,    empty_init, "Tiger Electronics", "Home Alone 2 - Lost in New York (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, tsonic,      0,          0, tsonic,      tsonic,      tsonic_state,      empty_init, "Tiger Electronics (licensed from Sega)", "Sonic The Hedgehog (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, trobocop3,   0,          0, trobocop3,   trobocop3,   trobocop3_state,   empty_init, "Tiger Electronics", "Robocop 3 (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tdummies,    0,          0, tdummies,    tdummies,    tdummies_state,    empty_init, "Tiger Electronics", "The Incredible Crash Dummies (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tsfight2,    0,          0, tsfight2,    tsfight2,    tsfight2_state,    empty_init, "Tiger Electronics (licensed from Capcom)", "Street Fighter II (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1992, twworld,     0,          0, twworld,     twworld,     twworld_state,     empty_init, "Tiger Electronics", "Wayne's World (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tjpark,      0,          0, tjpark,      tjpark,      tjpark_state,      empty_init, "Tiger Electronics", "Jurassic Park (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tsonic2,     0,          0, tsonic2,     tsonic2,     tsonic2_state,     empty_init, "Tiger Electronics (licensed from Sega)", "Sonic The Hedgehog 2 (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tsddragon,   0,          0, tsddragon,   tsddragon,   tsddragon_state,   empty_init, "Tiger Electronics (licensed from Technos)", "Super Double Dragon (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tdennis,     0,          0, tdennis,     tdennis,     tdennis_state,     empty_init, "Tiger Electronics", "Dennis the Menace (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tnmarebc,    0,          0, tnmarebc,    tnmarebc,    tnmarebc_state,    empty_init, "Tiger Electronics", "Nightmare Before Christmas (handheld)", MACHINE_SUPPORTS_SAVE ) // note: title has no "The"
CONS( 1993, ttransf2,    0,          0, ttransf2,    ttransf2,    ttransf2_state,    empty_init, "Tiger Electronics", "Transformers - Generation 2 (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1994, topaliens,   0,          0, topaliens,   topaliens,   topaliens_state,   empty_init, "Tiger Electronics", "Operation: Aliens (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1993, tmkombat,    0,          0, tmkombat,    tmkombat,    tmkombat_state,    empty_init, "Tiger Electronics (licensed from Midway)", "Mortal Kombat (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1994, tshadow,     0,          0, tshadow,     tshadow,     tshadow_state,     empty_init, "Tiger Electronics", "The Shadow (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1994, tskelwarr,   0,          0, tskelwarr,   tskelwarr,   tskelwarr_state,   empty_init, "Tiger Electronics", "Skeleton Warriors - The Dark Crusade (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, tbatfor,     0,          0, tbatfor,     tbatfor,     tbatfor_state,     empty_init, "Tiger Electronics", "Batman Forever - Double Dose of Doom (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, tjdredd,     0,          0, tjdredd,     tjdredd,     tjdredd_state,     empty_init, "Tiger Electronics", "Judge Dredd (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, tapollo13,   0,          0, tapollo13,   tapollo13,   tapollo13_state,   empty_init, "Tiger Electronics", "Apollo 13 (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1995, tgoldeye,    0,          0, tgoldeye,    tgoldeye,    tgoldeye_state,    empty_init, "Tiger Electronics", "007: GoldenEye (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1996, tsjam,       0,          0, tsjam,       tsjam,       tsjam_state,       empty_init, "Tiger Electronics", "Space Jam (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1996, tinday,      0,          0, tinday,      tinday,      tinday_state,      empty_init, "Tiger Electronics", "Independence Day (handheld)", MACHINE_SUPPORTS_SAVE )

// Tiger 72-xxx models
CONS( 1992, tbatmana,    0,          0, tbatmana,    tbatmana,    tbatmana_state,    empty_init, "Tiger Electronics", "Batman: The Animated Series (handheld)", MACHINE_SUPPORTS_SAVE )

// Tronica
CONS( 1983, trshutvoy,   0,          0, trshutvoy,   trshutvoy,   trshutvoy_state,   empty_init, "Tronica", "Shuttle Voyage", MACHINE_SUPPORTS_SAVE )
CONS( 1983, tigarden,    trshutvoy,  0, tigarden,    trshutvoy,   trshutvoy_state,   empty_init, "Tronica", "Thief in Garden", MACHINE_SUPPORTS_SAVE )
CONS( 1982, trsrescue,   0,          0, trsrescue,   trsrescue,   trsrescue_state,   empty_init, "Tronica", "Space Rescue", MACHINE_SUPPORTS_SAVE )

// misc
CONS( 1989, nummunch,    0,          0, nummunch,    nummunch,    nummunch_state,    empty_init, "VTech", "Electronic Number Muncher", MACHINE_SUPPORTS_SAVE )
