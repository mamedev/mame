// license:BSD-3-Clause
// copyright-holders:hap, Henrik Algestam
// thanks-to:Sean Riddle, Igor, Lee Robson, Milan Galcik
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
- improve display decay simulation? but SVG doesn't support setting brightness
  per segment, adding pwm_display_device right now has no added value
- confirm gnw_bfight rom (assumed to be the same as gnw_bfightn)
- confirm gnw_climber rom (assumed to be the same as gnw_climbern)
- confirm gnw_smb rom (assumed to be the same as gnw_smbn)
- Currently there is no accurate way to dump the SM511/SM512 melody ROM
  electronically. For the ones that weren't decapped, they were read by
  playing back all melody data and reconstructing it to ROM. Visual(decap)
  verification is wanted for: bassmate, gnw_bfightn, gnw_bjack, gnw_bsweep,
  gnw_climbern, gnw_dkcirc, gnw_dkhockey, gnw_dkjrp, gnw_dkong3, gnw_gcliff,
  gnw_mariocmt, gnw_mariocmta, gnw_mariotj, gnw_mbaway, gnw_mmousep,
  gnw_pinball, gnw_popeyep, gnw_sbuster, gnw_snoopyp, gnw_zelda

****************************************************************************

Misc Nintendo Game & Watch notes:

Trivia: Most of the Nintendo G&W have built-in cheats, likely kept in by
Nintendo to test the game. These were not accessible to users of course,
but for the sake of fun they're available on MAME.

BTANB: On some of the earlier G&W games, eg. gnw_fire, gnw_mmouse, gnw_pchute,
gnw_popeye, the controls still work after game over, this happens on the real
thing too.

Game list (* denotes not emulated yet)

Serial  Series MCU     Title
---------------------------------------------
AC-01     s    SM5A    Ball (aka Toss-Up)
FL-02     s    SM5A    Flagman (aka Flag Man)
MT-03     s    SM5A    Vermin (aka The Exterminator)
RC-04     s    SM5A    Fire (aka Fireman Fireman)
IP-05     s    SM5A    Judge
MH-06     g    SM5A    Manhole
CN-07     g    SM5A    Helmet (aka Headache)
LN-08     g    SM5A    Lion
PR-21     ws   SM5A    Parachute
OC-22     ws   SM5A    Octopus
PP-23     ws   SM5A    Popeye
FP-24     ws   SM5A    Chef
MC-25     ws   SM5A    Mickey Mouse
EG-26     ws   SM5A    Egg (same ROM as MC-25, but LCD differs)
FR-27     ws   SM5A    Fire
TL-28     ws   SM510   Turtle Bridge
ID-29     ws   SM510   Fire Attack
SP-30     ws   SM510   Snoopy Tennis
OP-51     ms   SM510   Oil Panic
DK-52     ms   SM510   Donkey Kong
DM-53     ms   SM510   Mickey & Donald
GH-54     ms   SM510   Green House
JR-55     ms   SM510   Donkey Kong II
MW-56     ms   SM510   Mario Bros.
LP-57     ms   SM510   Rain Shower
TC-58     ms   SM510   Life Boat
PB-59     ms   SM511   Pinball
BJ-60     ms   SM512   Black Jack
MG-61     ms   SM510   Squish
BD-62     ms   SM512   Bomb Sweeper
JB-63     ms   SM511   Safe Buster
MV-64     ms   SM512   Gold Cliff
ZL-65     ms   SM512   Zelda
CJ-71*    tt   SM511?  Donkey Kong Jr.
CM-72     tt   SM511   Mario's Cement Factory
SM-73*    tt   SM511?  Snoopy
PG-74*    tt   SM511?  Popeye
SM-91     p    SM511   Snoopy (assume same ROM & LCD as tabletop version)
PG-92     p    SM511   Popeye          "
CJ-93     p    SM511   Donkey Kong Jr. "
TB-94     p    SM511   Mario's Bombs Away
DC-95     p    SM511   Mickey Mouse
MK-96     p    SM511   Donkey Kong Circus (same ROM as DC-95, LCD is different)
DJ-101    nws  SM510   Donkey Kong Jr.
ML-102    nws  SM510   Mario's Cement Factory
NH-103    nws  SM510   Manhole
TF-104    nws  SM510   Tropical Fish
YM-105    nws  SM511   Super Mario Bros.
DR-106    nws  SM511   Climber
BF-107    nws  SM511   Balloon Fight
MB-108    nws  SM511   Mario The Juggler
BU-201    sc   SM510   Spitball Sparky
UD-202    sc   SM510   Crab Grab
BX-301    mvs  SM511   Boxing (aka Punch Out)
AK-302    mvs  SM511   Donkey Kong 3
HK-303    mvs  SM511   Donkey Kong Hockey
YM-801    cs   SM511   Super Mario Bros. (assume same ROM as nws version)
DR-802    cs   SM511   Climber            "
BF-803    cs   SM511   Balloon Fight      "
YM-901-S* x    SM511   Super Mario Bros.  "

RGW-001 (2010 Ball remake) is on different hardware, ATmega169PV MCU.
The "Mini Classics" keychains are by Stadlbauer, not Nintendo.

Bassmate Computer (BM-501) is on identical hardware as G&W Multi Screen,
but it's not part of the game series.

****************************************************************************

Regarding Электроника (Elektronika, translated: Electronics): It is not
actually a company. It was a USSR brand name for consumer electronics,
produced by factories belonging to the Ministry of Electronic Industry
(Minelektronprom, МЭП).

The LCD games were produced by: Angstrem, Mikron, Voschod (Russia), Billur
(Azerbaijan), Kamerton, Evistor (Belarus), Severodonetsk Instrument-Making
Plant (Ukraine), PO Proton and more. Their most popular LCD game (Nu, pogodi!),
is known to be initially produced by Evistor.

Most of the games are marked "bootleg" in MAME, because the ROM contents are
a 1:1 copy of Nintendo Game & Watch games. Known G&W cloned by Elektronika:
Fire(FR-27), Octopus, Chef, Egg/Mickey Mouse, Donkey Kong Jr.(CJ-93),
Spitball Sparky.

The MCUs used were not imported from Sharp, but cloned by USSR, renamed to
КБ1013ВК1-2 for SM5A and КБ1013ВК4-2 for SM510.

***************************************************************************/

#include "emu.h"
#include "includes/hh_sm510.h"

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
		for (; m_inputs[m_inp_lines] != nullptr; m_inp_lines++) { ; }

		// when last input line is fixed(GND)
		if (m_inp_fixed == -2)
		{
			m_inp_lines--;
			m_inp_fixed = m_inp_lines;
		}
	}

	// 1kHz display decay ticks
	m_display_decay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hh_sm510_state::display_decay_tick),this));
	m_display_decay_timer->adjust(attotime::from_hz(1024), 0, attotime::from_hz(1024));

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
	save_item(NAME(m_decay_pivot));
	save_item(NAME(m_decay_len));
}

void hh_sm510_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// lcd panel - on lcd handhelds, usually not a generic x/y screen device
// deflicker here, especially needed for SM500/SM5A with the active shift register

TIMER_CALLBACK_MEMBER(hh_sm510_state::display_decay_tick)
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
				if (m_display_decay[y][zx] < (m_decay_pivot + m_decay_len))
					m_display_decay[y][zx]++;
			}
			else if (m_display_decay[y][zx] > 0)
				m_display_decay[y][zx]--;
			u8 active_state = (m_display_decay[y][zx] < m_decay_pivot) ? 0 : 1;

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

void hh_sm510_state::sm510_lcd_segment_w(offs_t offset, u16 data)
{
	set_display_size(2, 16, 2);
	m_display_state[offset] = data;
}

void hh_sm510_state::sm500_lcd_segment_w(offs_t offset, u16 data)
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
			ret |= m_inputs[i]->read();

	if (fixed >= 0)
		ret |= m_inputs[fixed]->read();

	return ret;
}

void hh_sm510_state::update_k_line()
{
	// this is necessary because the MCU can wake up on K input activity
	m_maincpu->set_input_line(0, input_r() ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::input_changed)
{
	update_k_line();
}

void hh_sm510_state::input_w(u8 data)
{
	m_inp_mux = data;
	update_k_line();
}

u8 hh_sm510_state::input_r()
{
	return read_inputs(m_inp_lines, m_inp_fixed);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::acl_button)
{
	// ACL button is directly tied to MCU ACL pin
	m_maincpu->set_input_line(SM510_INPUT_LINE_ACL, newval ? ASSERT_LINE : CLEAR_LINE);
}


// other generic output handlers

void hh_sm510_state::piezo_r1_w(u8 data)
{
	// R1 to piezo (SM511 R pin is melody output)
	m_speaker->level_w(data & 1);
}

void hh_sm510_state::piezo_r2_w(u8 data)
{
	// R2 to piezo
	m_speaker->level_w(data >> 1 & 1);
}

void hh_sm510_state::piezo_input_w(u8 data)
{
	// R1 to piezo, other to input mux
	piezo_r1_w(data & 1);
	input_w(data >> 1);
}

void hh_sm510_state::piezo2bit_r1_w(u8 data)
{
	// R1(+S1) to piezo
	m_speaker_data = (m_speaker_data & ~1) | (data & 1);
	m_speaker->level_w(m_speaker_data);
}

void hh_sm510_state::piezo2bit_input_w(u8 data)
{
	// S1(+R1) to piezo, other to input mux
	m_speaker_data = (m_speaker_data & ~2) | (data << 1 & 2);
	m_speaker->level_w(m_speaker_data);
	input_w(data >> 1);
}



/***************************************************************************

  Common Machine Configurations

***************************************************************************/

// building blocks

void hh_sm510_state::mcfg_cpu_common(machine_config &config)
{
	m_maincpu->read_k().set(FUNC(hh_sm510_state::input_r));
	m_maincpu->read_ba().set([this] () { return m_io_ba.read_safe(1); });
	m_maincpu->read_b().set([this] () { return m_io_b.read_safe(1); });
}

void hh_sm510_state::mcfg_cpu_sm5a(machine_config &config)
{
	SM5A(config, m_maincpu);
	mcfg_cpu_common(config);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
}

void hh_sm510_state::mcfg_cpu_kb1013vk12(machine_config &config)
{
	KB1013VK12(config, m_maincpu);
	mcfg_cpu_common(config);
	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm500_lcd_segment_w));
}

void hh_sm510_state::mcfg_cpu_sm510(machine_config &config)
{
	SM510(config, m_maincpu);
	mcfg_cpu_common(config);
	m_maincpu->set_r_mask_option(2);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
}

void hh_sm510_state::mcfg_cpu_sm511(machine_config &config)
{
	SM511(config, m_maincpu);
	mcfg_cpu_common(config);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
}

void hh_sm510_state::mcfg_cpu_sm512(machine_config &config)
{
	SM512(config, m_maincpu);
	mcfg_cpu_common(config);
	m_maincpu->write_segs().set(FUNC(hh_sm510_state::sm510_lcd_segment_w));
	m_maincpu->write_s().set(FUNC(hh_sm510_state::input_w));
}

void hh_sm510_state::mcfg_svg_screen(machine_config &config, u16 width, u16 height, const char *tag)
{
	screen_device &screen(SCREEN(config, tag, SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(width, height);
	screen.set_visarea_full();
}

void hh_sm510_state::mcfg_sound_r1(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_r1_w));
}


// common presets

void hh_sm510_state::sm5a_common(machine_config &config, u16 width, u16 height)
{
	mcfg_cpu_sm5a(config);
	mcfg_sound_r1(config);
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	mcfg_svg_screen(config, width, height);
}

void hh_sm510_state::kb1013vk12_common(machine_config &config, u16 width, u16 height)
{
	mcfg_cpu_kb1013vk12(config);
	mcfg_sound_r1(config);
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo_input_w));
	mcfg_svg_screen(config, width, height);
}

void hh_sm510_state::sm510_common(machine_config &config, u16 width, u16 height)
{
	mcfg_cpu_sm510(config);
	mcfg_sound_r1(config);
	mcfg_svg_screen(config, width, height);
}

void hh_sm510_state::sm511_common(machine_config &config, u16 width, u16 height)
{
	mcfg_cpu_sm511(config);
	mcfg_sound_r1(config);
	mcfg_svg_screen(config, width, height);
}


// deviations

// multi-screen

void hh_sm510_state::sm510_dualh(machine_config &config, u16 leftwidth, u16 leftheight, u16 rightwidth, u16 rightheight)
{
	mcfg_cpu_sm510(config);
	mcfg_sound_r1(config);
	mcfg_svg_screen(config, leftwidth, leftheight, "screen_left");
	mcfg_svg_screen(config, rightwidth, rightheight, "screen_right");

	config.set_default_layout(layout_gnw_dualh);
}

void hh_sm510_state::dualv_common(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight)
{
	mcfg_sound_r1(config);
	mcfg_svg_screen(config, topwidth, topheight, "screen_top");
	mcfg_svg_screen(config, botwidth, botheight, "screen_bottom");

	config.set_default_layout(layout_gnw_dualv);
}

void hh_sm510_state::sm510_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight)
{
	mcfg_cpu_sm510(config);
	dualv_common(config, topwidth, topheight, botwidth, botheight);
}

void hh_sm510_state::sm511_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight)
{
	mcfg_cpu_sm511(config);
	dualv_common(config, topwidth, topheight, botwidth, botheight);
}

void hh_sm510_state::sm512_dualv(machine_config &config, u16 topwidth, u16 topheight, u16 botwidth, u16 botheight)
{
	mcfg_cpu_sm512(config);
	dualv_common(config, topwidth, topheight, botwidth, botheight);
}


// Tiger (SM510 R mask is direct, BA/B pins always connected)

void hh_sm510_state::sm510_tiger(machine_config &config, u16 width, u16 height)
{
	sm510_common(config, width, height);

	m_maincpu->set_r_mask_option(sm510_base_device::RMASK_DIRECT);
	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");
}

void hh_sm510_state::sm511_tiger1bit(machine_config &config, u16 width, u16 height)
{
	sm511_common(config, width, height);

	m_maincpu->read_ba().set_ioport("BA");
	m_maincpu->read_b().set_ioport("B");
}

void hh_sm510_state::sm511_tiger2bit(machine_config &config, u16 width, u16 height)
{
	sm511_tiger1bit(config, width, height);

	m_maincpu->write_s().set(FUNC(hh_sm510_state::piezo2bit_input_w));
	m_maincpu->write_r().set(FUNC(hh_sm510_state::piezo2bit_r1_w));

	// R via 120K resistor, S1 via 39K resistor (eg. tsonic, tsonic2, tbatmana)
	static const double speaker_levels[] = { 0.0, 1.0/3.0, 2.0/3.0, 1.0 };
	m_speaker->set_levels(4, speaker_levels);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Nintendo Game & Watch: Ball (model AC-01)
  * PCB label AC-01
  * Sharp SM5A label AC-01 5009 (no decap)
  * lcd screen with custom segments, 1-bit sound

  In the USA, it was distributed as Toss-Up by Mego under their Time-Out series.

***************************************************************************/

class gnw_ball_state : public hh_sm510_state
{
public:
	gnw_ball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void gnw_ball(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_ball )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_CONFNAME( 0x08, 0x00, "Invincibility (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_ball_state::gnw_ball(machine_config &config)
{
	sm5a_common(config, 1671, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_ball )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ac-01", 0x0000, 0x0740, CRC(ac94e6e4) SHA1(8270cb61f9fbff252eafec411b4c67f0171f8687) )

	ROM_REGION( 71748, "screen", 0)
	ROM_LOAD( "gnw_ball.svg", 0, 71748, CRC(7c116eaf) SHA1(578882af492b8a9f1eb72e06a547c8b574255fb9) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Flagman (model FL-02)
  * PCB label FL-02
  * Sharp SM5A label FL-02 2005 (no decap)
  * lcd screen with custom segments, 1-bit sound

  In the USA, it was distributed as Flag Man by Mego under their Time-Out series.

***************************************************************************/

class gnw_flagman_state : public hh_sm510_state
{
public:
	gnw_flagman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_flagman(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_flagman )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("4")

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- only works after power-on
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_flagman_state::gnw_flagman(machine_config &config)
{
	sm5a_common(config, 1511, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_flagman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fl-02", 0x0000, 0x0740, CRC(cc7a99e4) SHA1(d03d9a6b278bc11df7839708831241b5fa805f69) )

	ROM_REGION( 56163, "screen", 0)
	ROM_LOAD( "gnw_flagman.svg", 0, 56163, CRC(3aa97c65) SHA1(a363e71d371e5c85835cb3a0679760d0aedc75d5) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Vermin (model MT-03)
  * PCB label MT-03
  * Sharp SM5A label MT-03 5012 (no decap)
  * lcd screen with custom segments, 1-bit sound

  In the USA, it was distributed as The Exterminator by Mego under their Time-Out series.

***************************************************************************/

class gnw_vermin_state : public hh_sm510_state
{
public:
	gnw_vermin_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void gnw_vermin(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_vermin )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_CONFNAME( 0x08, 0x00, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_vermin_state::gnw_vermin(machine_config &config)
{
	sm5a_common(config, 1650, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_vermin )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mt-03", 0x0000, 0x0740, CRC(f8493177) SHA1(d629432ef8e9fbd7bbdc3fbeb45d9bd70d9d571b) )

	ROM_REGION( 105603, "screen", 0)
	ROM_LOAD( "gnw_vermin.svg", 0, 105603, CRC(1bd59ef4) SHA1(099120105e80d4753838ea513ffa784c4690cf5f) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Fire (model RC-04)
  * PCB label RC-04
  * Sharp SM5A label RC-04 5103 (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the silver version, there's also a wide screen version.

  In the USA, it was distributed as Fireman Fireman by Mego under their Time-Out series.

***************************************************************************/

class gnw_fires_state : public hh_sm510_state
{
public:
	gnw_fires_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void gnw_fires(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_fires )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_CONFNAME( 0x08, 0x00, "Invincibility (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_fires_state::gnw_fires(machine_config &config)
{
	sm5a_common(config, 1646, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_fires )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "rc-04", 0x0000, 0x0740, CRC(154ef27d) SHA1(fb65826dfd405ad05fe0f5f947c213214bbd61c0) )

	ROM_REGION( 102678, "screen", 0)
	ROM_LOAD( "gnw_fires.svg", 0, 102678, CRC(4f61f2f8) SHA1(2873629f0e36d3170bc284fa031a9c6181021495) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Judge (model IP-05)
  * PCB label IP-05
  * Sharp SM5A label IP-05 5010, or IP-15 5012 (no decap)
  * lcd screen with custom segments, 1-bit sound

  The first (green) issue of the game contains a bug where the players are
  scored differently when wrongly dodging a win. This issue is fixed in the
  second (purple) issue.

***************************************************************************/

class gnw_judge_state : public hh_sm510_state
{
public:
	gnw_judge_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_judge(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_judge )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("P2 Dodge")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("P2 Hit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("P1 Dodge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("P1 Hit")

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B") // 2-Player
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A") // 1-Player
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Increase Computer's Operation Time (Cheat)") // factory test, unpopulated on PCB -- only works after power-on
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_judge_state::gnw_judge(machine_config &config)
{
	sm5a_common(config, 1647, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_judge )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ip-15", 0x0000, 0x0740, CRC(f6ed6f62) SHA1(97bc1b5c383fb4077d982cfdc5a7d7603a0b5e2f) )

	ROM_REGION( 105108, "screen", 0)
	ROM_LOAD( "gnw_judge.svg", 0, 105108, CRC(7760e82e) SHA1(cfc1f08465ecc8ac3385bcb078268cbbfca9fc41) )
ROM_END

ROM_START( gnw_judgeo )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ip-05", 0x0000, 0x0740, CRC(1b28a834) SHA1(cb8dbbf678ba22c4484d18cc1a6b99c1d34d1951) )

	ROM_REGION( 105108, "screen", 0)
	ROM_LOAD( "gnw_judge.svg", 0, 105108, CRC(7760e82e) SHA1(cfc1f08465ecc8ac3385bcb078268cbbfca9fc41) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Manhole (model MH-06)
  * PCB label MH-06
  * Sharp SM5A label MH-06 5104 (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the Gold Series version, there's also a new wide screen version
  (NH-103)

***************************************************************************/

class gnw_manholeg_state : public hh_sm510_state
{
public:
	gnw_manholeg_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_manholeg(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_manholeg )
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

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // display test?

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_manholeg_state::gnw_manholeg(machine_config &config)
{
	sm5a_common(config, 1667, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_manholeg )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mh-06", 0x0000, 0x0740, CRC(ae52c425) SHA1(8da8a714ecbdde7d0f257b52a5014993675a5f3f) )

	ROM_REGION( 125607, "screen", 0)
	ROM_LOAD( "gnw_manholeg.svg", 0, 125607, CRC(4b281ff2) SHA1(18f212ab5738756e0841d6afa401a03f7aaddf7b) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Helmet (model CN-07)
  * PCB label CN-07
  * Sharp SM5A label CN-07 5102, or CN-17 21ZA (no decap)
  * lcd screen with custom segments, 1-bit sound

  In the UK, it was distributed as Headache by CGL.

  MCU label CN-07 is the first version, CN-17 is a bugfix release.

***************************************************************************/

class gnw_helmet_state : public hh_sm510_state
{
public:
	gnw_helmet_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_helmet(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_helmet )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_CONFNAME( 0x01, 0x00, "Invincibility (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_helmet_state::gnw_helmet(machine_config &config)
{
	sm5a_common(config, 1657, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_helmet )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cn-17", 0x0000, 0x0740, CRC(6d251e2e) SHA1(c61f591514de36fb2270038a6505945564c9f90e) )

	ROM_REGION( 109404, "screen", 0)
	ROM_LOAD( "gnw_helmet.svg", 0, 109404, CRC(0dce1694) SHA1(412e69054b95f17fe08545f3c303c11abbe26304) )
ROM_END

ROM_START( gnw_helmeto )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cn-07", 0x0000, 0x0740, CRC(30c8bc90) SHA1(f5ac0fe7a09ee1ad6f6e4bd096b4be20f65d73db) )

	ROM_REGION( 109404, "screen", 0)
	ROM_LOAD( "gnw_helmet.svg", 0, 109404, CRC(0dce1694) SHA1(412e69054b95f17fe08545f3c303c11abbe26304) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Lion (model LN-08)
  * PCB label LN-08
  * Sharp SM5A label LN-08 519A (no decap)
  * lcd screen with custom segments, 1-bit sound

  BTANB: The game doesn't support simultaneous button presses for the controls,
  it's the same as in eg. gnw_mmouse but in this game it doesn't make much sense
  with the 2 separate guys. More likely a bad game design choice than bug.

***************************************************************************/

class gnw_lion_state : public hh_sm510_state
{
public:
	gnw_lion_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_lion(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_lion )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Increase Speed (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // "
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // "

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

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // display test?

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void gnw_lion_state::gnw_lion(machine_config &config)
{
	sm5a_common(config, 1646, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_lion )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ln-08", 0x0000, 0x0740, CRC(9677681d) SHA1(6f7c960e04b63f1b7d926b598413f4c818b8fe53) )

	ROM_REGION( 155863, "screen", 0)
	ROM_LOAD( "gnw_lion.svg", 0, 155863, CRC(b5a5a4dc) SHA1(49d894d6e1d1fb35cd11f08c7ce30518be89dd0f) )
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
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- disable after boot
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // "
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
	sm5a_common(config, 1602, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_pchute )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pr-21", 0x0000, 0x0740, CRC(392b545e) SHA1(e71940cd4cee07ba1e62c1c7d9e9b19410e7232d) )

	ROM_REGION( 169640, "screen", 0)
	ROM_LOAD( "gnw_pchute.svg", 0, 169640, CRC(f30a0b31) SHA1(676989a418ae0dfe6bb1b097640422219c930453) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Octopus (model OC-22)
  * PCB label OC-22Y A
  * Sharp SM5A label OC-22 204A (no decap)
  * lcd screen with custom segments, 1-bit sound

  Also cloned in 1989 by Elektronika(USSR) as Тайны океана (Tayny okeana, export
  version: Mysteries of the Ocean), ROM is identical, graphics as well except
  for the AM/PM/GAME segments.

***************************************************************************/

class gnw_octopus_state : public hh_sm510_state
{
public:
	gnw_octopus_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_octopus(machine_config &config);
	void taynyoke(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_octopus )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Invincibility (Cheat)") // factory test, unpopulated on PCB
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
	sm5a_common(config, 1586, 1080); // R mask option confirmed
}

void gnw_octopus_state::taynyoke(machine_config &config)
{
	kb1013vk12_common(config, 1647, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_octopus )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "oc-22", 0x0000, 0x0740, CRC(bd27781d) SHA1(07b4feb9265c83b159f96c7e8ee1c61a2cc17dc5) )

	ROM_REGION( 119827, "screen", 0)
	ROM_LOAD( "gnw_octopus.svg", 0, 119827, CRC(efbdaa65) SHA1(42c746bef282176d59f57ddf7328f8d034f4ca02) )
ROM_END

ROM_START( taynyoke )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-03.bin", 0x0000, 0x0740, CRC(bd27781d) SHA1(07b4feb9265c83b159f96c7e8ee1c61a2cc17dc5) )

	ROM_REGION( 93910, "screen", 0)
	ROM_LOAD( "taynyoke.svg", 0, 93910, CRC(da7a835e) SHA1(1fe427a60bbf78fdf29ea401ec86b225098b68bb) )
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
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
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
	sm5a_common(config, 1604, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_popeye )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pp-23", 0x0000, 0x0740, CRC(49987769) SHA1(ad90659a3ce7169a4df16367c5307435d9f9d956) )

	ROM_REGION( 218587, "screen", 0)
	ROM_LOAD( "gnw_popeye.svg", 0, 218587, CRC(4740bcd5) SHA1(a46ab455f2dd41caabd6c85cfa7dfde70805f157) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Chef (model FP-24)
  * PCB label FP-24
  * Sharp SM5A label FP-24 51YB (die label CMS646, ROM ID 74)
  * lcd screen with custom segments, 1-bit sound

  In 1989, Elektronika(USSR) released a clone: Весёлый повар (Vesyolyy povar,
  export version: Merry Cook). This game shares the same ROM, though the graphics
  are slightly different.

***************************************************************************/

class gnw_chef_state : public hh_sm510_state
{
public:
	gnw_chef_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void vespovar(machine_config &config);
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- only works after power-on
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_chef_state::gnw_chef(machine_config &config)
{
	sm5a_common(config, 1666, 1080); // assuming same R mask option as merry cook
}

void gnw_chef_state::vespovar(machine_config & config)
{
	kb1013vk12_common(config, 1679, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_chef )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fp-24", 0x0000, 0x0740, CRC(2806ab39) SHA1(18261a80eec5bf768bb88b803c598f80e078c71f) )

	ROM_REGION( 199518, "screen", 0)
	ROM_LOAD( "gnw_chef.svg", 0, 199518, CRC(ecc18d28) SHA1(1c0b7dfff71faa4d4395c19a84454870e403f927) )
ROM_END

ROM_START( vespovar )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-04.bin", 0x0000, 0x0740, CRC(2806ab39) SHA1(18261a80eec5bf768bb88b803c598f80e078c71f) )

	ROM_REGION( 144128, "screen", 0)
	ROM_LOAD( "vespovar.svg", 0, 144128, CRC(dcd1c073) SHA1(e15bf643f17b7ead37407c985e053e6434683d7c) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mickey Mouse (model MC-25), Egg (model EG-26)
  * PCB label MC-25 EG-26 (yes, both listed)
  * Sharp SM5A label MC-25 51YD (Mickey Mouse), MC-25 519D (Egg) (no decap)
  * lcd screen with custom segments, 1-bit sound

  MC-25 and EG-26 are the same game, it's assumed that the latter was for
  regions where Nintendo wasn't able to license from Disney.

  In 1984, Электроника (Elektronika, USSR) released an unlicensed clone:
  Ну, погоди! (Nu, pogodi!). This was followed by several other titles that
  were the same under the hood, only differing in graphics. They also made a
  slightly modified version, adding a new game mode (by pressing A+B) where the
  player/CPU roles are reversed. This version is known as Разведчики космоса
  (Razvedchiki kosmosa, export version: Explorers of Space).

  The following Mickey Mouse Elektronika clones are emulated in MAME:

  Model  Title               Transliteration      Export version      Note
  ---------------------------------------------------------------------------------
  ИМ-02  Ну, погоди!         Nu, pogodi!          -                   -
  ИМ-10  Хоккей              Hockey (Khokkey)     Ice Hockey          Export version manufactured by PO Proton
  ИМ-13  Разведчики космоса  Razvedchiki kosmosa  Explorers of Space  Modified ROM (see note above)
  ИМ-16  Охота               Okhota               Fowling             -
  ИМ-19  Биатлон             Biathlon (Biatlon)   -                   -
  ИМ-22  Весёлые футболисты  Vesyolye futbolisty  Monkey Goalkeeper   -
  ИМ-32  Кот-рыболов         Kot-rybolov          -                   -
  ИМ-33  Квака-задавака      Kvaka-zadavaka       Frogling            -
  ИМ-49  Ночные воришки      Nochnye vorishki     Night Burglars      -
  ИМ-50  Космический полёт   Kosmicheskiy polyot  Space Flight        The Model ID is the same as Весёлая арифметика (Vesyolaya arithmetika, export version: Amusing Arithmetic) (not emulated in MAME)
  ИМ-51  Морская атака       Morskaya ataka       -                   -
  ИМ-53  Атака астероидов    Ataka asteroidov     -                   Graphics are very similar to ИМ-50

***************************************************************************/

class gnw_mmouse_state : public hh_sm510_state
{
public:
	gnw_mmouse_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mmouse(machine_config &config);
	void gnw_egg(machine_config &config);
	void nupogodi(machine_config &config);
	void ehockey(machine_config &config);
	void rkosmosa(machine_config &config);
	void okhota(machine_config &config);
	void biathlon(machine_config &config);
	void vfutbol(machine_config &config);
	void krybolov(machine_config &config);
	void kvakazad(machine_config &config);
	void nochnyev(machine_config &config);
	void kosmicpt(machine_config &config);
	void morataka(machine_config &config);
	void atakaast(machine_config &config);
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB -- only works after power-on
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rkosmosa )
	PORT_INCLUDE( gnw_mmouse )

	PORT_MODIFY("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void gnw_mmouse_state::gnw_mmouse(machine_config &config)
{
	sm5a_common(config, 1684, 1080); // R mask option confirmed
}

void gnw_mmouse_state::gnw_egg(machine_config &config)
{
	sm5a_common(config, 1690, 1080); // R mask option confirmed
}

void gnw_mmouse_state::nupogodi(machine_config &config)
{
	kb1013vk12_common(config, 1715, 1080); // R mask option ?
}

void gnw_mmouse_state::ehockey(machine_config &config)
{
	kb1013vk12_common(config, 1782, 1080); // R mask option ?
}

void gnw_mmouse_state::rkosmosa(machine_config &config)
{
	kb1013vk12_common(config, 1646, 1080); // R mask option ?
}

void gnw_mmouse_state::okhota(machine_config &config)
{
	kb1013vk12_common(config, 1632, 1080); // R mask option ?
}

void gnw_mmouse_state::biathlon(machine_config &config)
{
	kb1013vk12_common(config, 1633, 1080); // R mask option ?
}

void gnw_mmouse_state::vfutbol(machine_config &config)
{
	kb1013vk12_common(config, 1655, 1080); // R mask option ?
}

void gnw_mmouse_state::krybolov(machine_config &config)
{
	kb1013vk12_common(config, 1638, 1080); // R mask option ?
}

void gnw_mmouse_state::kvakazad(machine_config &config)
{
	kb1013vk12_common(config, 1660, 1080); // R mask option ?
}

void gnw_mmouse_state::nochnyev(machine_config &config)
{
	kb1013vk12_common(config, 1641, 1080); // R mask option ?
}

void gnw_mmouse_state::kosmicpt(machine_config &config)
{
	kb1013vk12_common(config, 1658, 1080); // R mask option ?
}

void gnw_mmouse_state::morataka(machine_config &config)
{
	kb1013vk12_common(config, 1648, 1080); // R mask option ?
}

void gnw_mmouse_state::atakaast(machine_config &config)
{
	kb1013vk12_common(config, 1620, 1080); // R mask option ?
}

// roms

ROM_START( gnw_mmouse )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc-25", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 181706, "screen", 0)
	ROM_LOAD( "gnw_mmouse.svg", 0, 181706, CRC(60cdc76a) SHA1(09755abd16222c1a0fe6c7ebb902706440d3e369) )
ROM_END

ROM_START( gnw_egg )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc-25", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 193119, "screen", 0)
	ROM_LOAD( "gnw_egg.svg", 0, 193119, CRC(1e469fe5) SHA1(bc80114337feefca590e48c823e8488f6b63f896) )
ROM_END

ROM_START( nupogodi )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-02.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 154233, "screen", 0)
	ROM_LOAD( "nupogodi.svg", 0, 154233, CRC(42cfb84a) SHA1(249ca7ec78066b57f9a18e48ada64712c944e461) )
ROM_END

ROM_START( ehockey )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-10.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 94977, "screen", 0)
	ROM_LOAD( "ehockey.svg", 0, 94977, CRC(98cf43b0) SHA1(4353505709612344cd3b597c3b4e9f6b441ddb66) )
ROM_END

ROM_START( rkosmosa )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-13.bin", 0x0000, 0x0740, CRC(553e2b09) SHA1(2b74f8437b881fbb62b61f25435a5bfc66872a9a) )

	ROM_REGION( 81420, "screen", 0)
	ROM_LOAD( "rkosmosa.svg", 0, 81420, CRC(dc6632be) SHA1(0906d933f4cda39ee1e57b502651a821e61e95ef) )
ROM_END

ROM_START( okhota )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-16.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 117838, "screen", 0)
	ROM_LOAD( "okhota.svg", 0, 117838, CRC(7de707c6) SHA1(c876ea16bd8af033086e2e20860d2e1d09296d59) )
ROM_END

ROM_START( biathlon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-19.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 116377, "screen", 0)
	ROM_LOAD( "biathlon.svg", 0, 116377, CRC(fadf729e) SHA1(671f9496e2bfe7b4800ee7bad039485e19958428) )
ROM_END

ROM_START( vfutbol )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-22.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 131901, "screen", 0)
	ROM_LOAD( "vfutbol.svg", 0, 131901, CRC(85811308) SHA1(288aa41bade08c61e0d346b9c1109179564e34ed) )
ROM_END

ROM_START( krybolov )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-32.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 132804, "screen", 0)
	ROM_LOAD( "krybolov.svg", 0, 132804, CRC(4e3e70d3) SHA1(18f1300afa601deb6ac01dcf7dca88187b7940a3) )
ROM_END

ROM_START( kvakazad )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-33.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 131961, "screen", 0)
	ROM_LOAD( "kvakazad.svg", 0, 131961, CRC(37b27420) SHA1(25d9e273f056c10e3a5bc4476ce980bfdb8095e1) )
ROM_END

ROM_START( nochnyev )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-49.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 136498, "screen", 0)
	ROM_LOAD( "nochnyev.svg", 0, 136498, CRC(24a287cd) SHA1(2d14aa9b55b42c634df141fe4037ae286549b17b) )
ROM_END

ROM_START( kosmicpt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-50.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 110214, "screen", 0)
	ROM_LOAD( "kosmicpt.svg", 0, 110214, CRC(ccef6d27) SHA1(71f3cf49a5797ed9296f1e86ec4575ffefab67dd) )
ROM_END

ROM_START( morataka )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-51.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 105057, "screen", 0)
	ROM_LOAD( "morataka.svg", 0, 105057, CRC(c235c56c) SHA1(b6ef74ba7826221683243e23513270d0f0f2cfda) )
ROM_END

ROM_START( atakaast )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-53.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 105570, "screen", 0)
	ROM_LOAD( "atakaast.svg", 0, 105570, CRC(3d79aacc) SHA1(bc25969f4d6fa75b320130c920ac0bdc8fb44cbd) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Fire (model FR-27)
  * PCB label FR-27
  * Sharp SM5A label FR-27 523B (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the wide screen version, there's also a silver version. Doing a
  hex-compare between the two, this one seems to be a complete rewrite.
  FR-27 is the last G&W on SM5A, they were followed with SM51x.

  In 1989 Elektronika(USSR) released a clone: Космический мост (Kosmicheskiy most,
  export version: Space Bridge). This game shares the same ROM, though the
  graphics are different.

***************************************************************************/

class gnw_fire_state : public hh_sm510_state
{
public:
	gnw_fire_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void kosmicmt(machine_config &config);
	void gnw_fire(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_fire )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
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
	sm5a_common(config, 1624, 1080); // R mask option confirmed
}

void gnw_fire_state::kosmicmt(machine_config & config)
{
	kb1013vk12_common(config, 1673, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_fire )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fr-27", 0x0000, 0x0740, CRC(f4c53ef0) SHA1(6b57120a0f9d2fd4dcd65ad57a5f32def71d905f) )

	ROM_REGION( 163920, "screen", 0)
	ROM_LOAD( "gnw_fire.svg", 0, 163920, CRC(be8a9f05) SHA1(644d8bed6228fa7e2f541b60fcfc1a0d97df0df6) )
ROM_END

ROM_START( kosmicmt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-09.bin", 0x0000, 0x0740, CRC(f4c53ef0) SHA1(6b57120a0f9d2fd4dcd65ad57a5f32def71d905f) )

	ROM_REGION( 124578, "screen", 0)
	ROM_LOAD( "kosmicmt.svg", 0, 124578, CRC(913324ef) SHA1(6e72f7f517da754075af11283d71fc8d24ac0529) )
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
	{
		// increase lcd decay: unwanted segments light up
		m_decay_pivot = 25;
		m_decay_len = 25;
	}

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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_tbridge_state::gnw_tbridge(machine_config &config)
{
	sm510_common(config, 1587, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_tbridge )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tl-28", 0x0000, 0x1000, CRC(284e7224) SHA1(b50d7f3a527ffe50771ef55fdf8214929bfa2253) )

	ROM_REGION( 242944, "screen", 0)
	ROM_LOAD( "gnw_tbridge.svg", 0, 242944, CRC(bf66cb38) SHA1(3f19d1e6584062944e56107d47ebe26335d50f42) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_fireatk_state::gnw_fireatk(machine_config &config)
{
	sm510_common(config, 1655, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_fireatk )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "id-29", 0x0000, 0x1000, CRC(5f6e8042) SHA1(63afc3acd8a2a996095fa8ba2dfccd48e5214478) )

	ROM_REGION( 267914, "screen", 0)
	ROM_LOAD( "gnw_fireatk.svg", 0, 267914, CRC(f9eea340) SHA1(1fbc224dac447fe3902920ee3f1afc11150b5962) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_stennis_state::gnw_stennis(machine_config &config)
{
	sm510_common(config, 1581, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_stennis )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "sp-30", 0x0000, 0x1000, CRC(ba1d9504) SHA1(ff601765d88564b1570a59f5b1a4005c7b0fd66c) )

	ROM_REGION( 228125, "screen", 0)
	ROM_LOAD( "gnw_stennis.svg", 0, 228125, CRC(1134ef9a) SHA1(6f35a4d610c952663761f7ccb74c6650752cac77) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_opanic_state::gnw_opanic(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1292/2, 1920/2, 1230/2); // R mask option confirmed
}

// roms

ROM_START( gnw_opanic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "op-51", 0x0000, 0x1000, CRC(31c288c9) SHA1(4bfd0fba94a9927cefc925db8196b063c5dd9b19) )

	ROM_REGION( 79771, "screen_top", 0)
	ROM_LOAD( "gnw_opanic_top.svg", 0, 79771, CRC(0e1e6485) SHA1(15d5ec48cad65759a50ed624e4161a8f2513f704) )

	ROM_REGION( 112962, "screen_bottom", 0)
	ROM_LOAD( "gnw_opanic_bottom.svg", 0, 112962, CRC(ae4f4f1f) SHA1(97907bea3ca92759a0ea889e80d60d25a701027a) )
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkong_state::gnw_dkong(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1266/2, 1920/2, 1266/2); // R mask option confirmed
}

// roms

ROM_START( gnw_dkong )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dk-52", 0x0000, 0x1000, CRC(5180cbf8) SHA1(5174570a8d6a601226f51e972bac6735535fe11d) )

	ROM_REGION( 176843, "screen_top", 0)
	ROM_LOAD( "gnw_dkong_top.svg", 0, 176843, CRC(16c16b84) SHA1(fa2e54c04366a30b51de024296b9f94c1cb76d68) )

	ROM_REGION( 145516, "screen_bottom", 0)
	ROM_LOAD( "gnw_dkong_bottom.svg", 0, 145516, CRC(2b711e9d) SHA1(0e263020cbe0e8b88bb68e3176630639b518935e) )
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mickdon_state::gnw_mickdon(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1281/2, 1920/2, 1236/2); // R mask option confirmed

	m_maincpu->write_r().set(FUNC(gnw_mickdon_state::piezo_r2_w));
}

// roms

ROM_START( gnw_mickdon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dm-53_565", 0x0000, 0x1000, CRC(e21fc0f5) SHA1(3b65ccf9f98813319410414e11a3231b787cdee6) )

	ROM_REGION( 126477, "screen_top", 0)
	ROM_LOAD( "gnw_mickdon_top.svg", 0, 126477, CRC(11e02fce) SHA1(fe2700711c73940a9488a6d223db4c4e92df4188) )

	ROM_REGION( 122915, "screen_bottom", 0)
	ROM_LOAD( "gnw_mickdon_bottom.svg", 0, 122915, CRC(b8cf63c2) SHA1(2406c9826f94a345ca9641e51fb26088f434960c) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Green House (model GH-54)
  * PCB label GH-54
  * Sharp SM510 label GH-54 52ZD (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

  After the 20,000,000th G&W, Nintendo made a special edition of Green House
  (still model GH-54), with the box art showing all the released games so far.

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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_ghouse_state::gnw_ghouse(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1303/2, 1920/2, 1274/2); // R mask option confirmed
}

// roms

ROM_START( gnw_ghouse )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "gh-54", 0x0000, 0x1000, CRC(4df12b4d) SHA1(708be5fef8dbd9337f5ab35baaca5bdf21e1f36c) )

	ROM_REGION( 159258, "screen_top", 0)
	ROM_LOAD( "gnw_ghouse_top.svg", 0, 159258, CRC(308c9c86) SHA1(e83d114e702b6da3cba4e45bd48edfe9882afac1) )

	ROM_REGION( 149922, "screen_bottom", 0)
	ROM_LOAD( "gnw_ghouse_bottom.svg", 0, 149922, CRC(c07c6bb8) SHA1(7a0d6f38ecdbfcd09ab967417fa9d06b5c5c21e4) )
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkong2_state::gnw_dkong2(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1241/2, 1920/2, 1237/2); // R mask option confirmed
}

// roms

ROM_START( gnw_dkong2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "jr-55_560", 0x0000, 0x1000, CRC(46aed0ae) SHA1(72f75ccbd84aea094148c872fc7cc1683619a18a) )

	ROM_REGION( 267462, "screen_top", 0)
	ROM_LOAD( "gnw_dkong2_top.svg", 0, 267462, CRC(41bb5414) SHA1(20c7af7c64e12273320029eecc5a33ec65d15bc5) )

	ROM_REGION( 390601, "screen_bottom", 0)
	ROM_LOAD( "gnw_dkong2_bottom.svg", 0, 390601, CRC(3f85bb01) SHA1(8964f02e8372f5d8dd5e8edfe0b79dae31b59b3a) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario Bros. (model MW-56)
  * PCB label MW-56-M-I (left), MW-56-S (right)
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mario_state::gnw_mario(machine_config &config)
{
	sm510_dualh(config, 2258/2, 1440/2, 2261/2, 1440/2); // R mask option confirmed
}

// roms

ROM_START( gnw_mario )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mw-56", 0x0000, 0x1000, CRC(385e59da) SHA1(2f79281bdf2f2afca2fb5bd7b9a3beeffc9c4eb7) )

	ROM_REGION( 154916, "screen_left", 0)
	ROM_LOAD( "gnw_mario_left.svg", 0, 154916, CRC(8ea82355) SHA1(ad286039a215dfa0f02bb1caf875d55dedb9b71e) )

	ROM_REGION( 202902, "screen_right", 0)
	ROM_LOAD( "gnw_mario_right.svg", 0, 202902, CRC(cfe8c0ba) SHA1(87cd54a8104e9bb4f266b137b043e32a0c1d9772) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_rshower_state::gnw_rshower(machine_config &config)
{
	sm510_dualh(config, 2126/2, 1440/2, 2146/2, 1440/2); // R mask option confirmed
}

// roms

ROM_START( gnw_rshower )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "lp-57", 0x0000, 0x1000, CRC(51a2c5c4) SHA1(d60542e6785ba7b6a44153a66c739787cf670816) )

	ROM_REGION( 135868, "screen_left", 0)
	ROM_LOAD( "gnw_rshower_left.svg", 0, 135868, CRC(806493f1) SHA1(0287fba2c2962aced8156c2ebc4f299c4703acf2) )

	ROM_REGION( 140445, "screen_right", 0)
	ROM_LOAD( "gnw_rshower_right.svg", 0, 140445, CRC(bead097a) SHA1(a3929e0043ff5132fb4cf7a41edece96926f50d2) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Life Boat (model TC-58)
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_lboat_state::gnw_lboat(machine_config &config)
{
	sm510_dualh(config, 2116/2, 1440/2, 2057/2, 1440/2); // R mask option confirmed
}

// roms

ROM_START( gnw_lboat )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tc-58", 0x0000, 0x1000, CRC(1f88f6a2) SHA1(22fd62127dda43a0ada2fe89b0518eec8cbe2a25) )

	ROM_REGION( 156441, "screen_left", 0)
	ROM_LOAD( "gnw_lboat_left.svg", 0, 156441, CRC(a1727890) SHA1(b1dd24f99496d215a3083a138fc3fff923303d34) )

	ROM_REGION( 155258, "screen_right", 0)
	ROM_LOAD( "gnw_lboat_right.svg", 0, 155258, CRC(76619ad3) SHA1(b44d57e2f4a2cecf98e402adf802d16c5934d301) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Pinball (model PB-59)
  * PCB label PB-59
  * Sharp SM511 label PB-59 53ZD (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_pinball_state : public hh_sm510_state
{
public:
	gnw_pinball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_pinball(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_pinball )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB -- this one multiplies scoring factor
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_pinball_state::gnw_pinball(machine_config &config)
{
	sm511_dualv(config, 1920/2, 1271/2, 1920/2, 1286/2);
}

// roms

ROM_START( gnw_pinball )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pb-59.program", 0x0000, 0x1000, CRC(d29dab34) SHA1(69ac9ee63eda67360c21627b7d625093709b5cd9) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "pb-59.melody", 0x000, 0x100, BAD_DUMP CRC(5c9ccb55) SHA1(1bada6caf3609f969421087219e6635f4c135282) ) // decap needed for verification

	ROM_REGION( 83191, "screen_top", 0)
	ROM_LOAD( "gnw_pinball_top.svg", 0, 83191, CRC(abe3edd9) SHA1(b32327b81b788896150e709ab8dc4a2155ae0995) )

	ROM_REGION( 63618, "screen_bottom", 0)
	ROM_LOAD( "gnw_pinball_bottom.svg", 0, 63618, CRC(1db44191) SHA1(73f73d246630d0b9efeb8dc72f37a2b88f735ceb) )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("Double Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("Bet x10 / Hit")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("Bet x1 / Stand")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY PORT_NAME("Enter")

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
	sm512_dualv(config, 1920/2, 1290/2, 1920/2, 1297/2);
}

// roms

ROM_START( gnw_bjack )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bj-60.program", 0x0000, 0x1000, CRC(8e74f633) SHA1(54b0f65ee716d2820a9ed9c743755d2a2d99ce4d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bj-60.melody", 0x000, 0x100, BAD_DUMP CRC(2619224e) SHA1(b65dc590b6eb1de793e980af236ccf8360b3cfee) ) // decap needed for verification

	ROM_REGION( 75366, "screen_top", 0)
	ROM_LOAD( "gnw_bjack_top.svg", 0, 75366, CRC(d36fb4e4) SHA1(7f2a0256d78eb01e757208ead0fd52ee63ce8efa) )

	ROM_REGION( 112599, "screen_bottom", 0)
	ROM_LOAD( "gnw_bjack_bottom.svg", 0, 112599, CRC(04880ae1) SHA1(60f3723f81965fe4891f25a3522351872f338389) )
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
	{
		// increase lcd decay: unwanted segments light up
		m_decay_pivot = 17;
	}

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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Bonus Life (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_squish_state::gnw_squish(machine_config &config)
{
	sm510_dualv(config, 1920/2, 1285/2, 1920/2, 1287/2); // R mask option confirmed
}

// roms

ROM_START( gnw_squish )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg-61", 0x0000, 0x1000, CRC(79cd509c) SHA1(969e5425984ba9e5183c68b38b3588f53d1e8e5d) )

	ROM_REGION( 70456, "screen_top", 0)
	ROM_LOAD( "gnw_squish_top.svg", 0, 70456, CRC(8d10b94e) SHA1(33854e7ea8f02adceb597c9ba259aa553953e698) )

	ROM_REGION( 279739, "screen_bottom", 0)
	ROM_LOAD( "gnw_squish_bottom.svg", 0, 279739, CRC(7f4bd704) SHA1(e625910101896cf3a6d41e28ccda77f902f71c7a) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Bomb Sweeper (model BD-62)
  * PCB label BD-62
  * Sharp SM512 label BD-62 8727 A (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_bsweep_state : public hh_sm510_state
{
public:
	gnw_bsweep_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_bsweep(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_bsweep )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Level Skip (Cheat)") // " -- Controller keys skips level when activated
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_bsweep_state::gnw_bsweep(machine_config &config)
{
	sm512_dualv(config, 1920/2, 1291/2, 1920/2, 1239/2);
}

// roms

ROM_START( gnw_bsweep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bd-62.program", 0x0000, 0x1000, CRC(f3ac66ea) SHA1(3fbf444ade5bc96cf0073ca72f1d583cb0f48fc5) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bd-62.melody", 0x000, 0x100, BAD_DUMP CRC(addc0368) SHA1(fc488bdf1c2ea5ca84cc66762126bb5874659d8f) ) // decap needed for verification

	ROM_REGION( 218174, "screen_top", 0)
	ROM_LOAD( "gnw_bsweep_top.svg", 0, 218174, CRC(b2c8e895) SHA1(9f7d5973a5f920845c83d30f7ebbbec93232c41e) )

	ROM_REGION( 277420, "screen_bottom", 0)
	ROM_LOAD( "gnw_bsweep_bottom.svg", 0, 277420, CRC(8a9786cb) SHA1(48390a77b0e436ec7d7e8835923faef787e163d4) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Safe Buster (model JB-63)
  * PCB label JB-63
  * Sharp SM511 label JB-63 8841 B (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_sbuster_state : public hh_sm510_state
{
public:
	gnw_sbuster_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_sbuster(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_sbuster )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_sbuster_state::gnw_sbuster(machine_config &config)
{
	sm511_dualv(config, 1920/2, 1246/2, 1920/2, 1269/2);
}

// roms

ROM_START( gnw_sbuster )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "jb-63.program", 0x0000, 0x1000, CRC(231d358d) SHA1(c748788da125e77b9d0fe1228f64de71f41af42b) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "jb-63.melody", 0x000, 0x100, BAD_DUMP CRC(28cb2914) SHA1(52d34265611f786b597653193752d16563dd5e82) ) // decap needed for verification

	ROM_REGION( 221903, "screen_top", 0)
	ROM_LOAD( "gnw_sbuster_top.svg", 0, 221903, CRC(adb9b67f) SHA1(902998ead1a13d3c26854393283ab622e1fd3f70) )

	ROM_REGION( 282593, "screen_bottom", 0)
	ROM_LOAD( "gnw_sbuster_bottom.svg", 0, 282593, CRC(12542c5e) SHA1(fb05b8f4a2cbeeb566ae111cd27ff486c1478d7b) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Gold Cliff (model MV-64)
  * PCB label MV-64
  * Sharp SM512 label MV-64 9027 A (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class gnw_gcliff_state : public hh_sm510_state
{
public:
	gnw_gcliff_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_gcliff(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_gcliff )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Continue")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Level Skip (Cheat)") // " -- Left or right skips level when activated
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_gcliff_state::gnw_gcliff(machine_config &config)
{
	sm512_dualv(config, 1920/2, 1257/2, 1920/2, 1239/2);
}

// roms

ROM_START( gnw_gcliff )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mv-64.program", 0x0000, 0x1000, CRC(2448a3bf) SHA1(bfb1a1b500321f8ee0b6f07ef8503e64fe6d37c0) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "mv-64.melody", 0x000, 0x100, BAD_DUMP CRC(cb938709) SHA1(516dcc8a1edffe02f50d349389caac0676de1eba) ) // decap needed for verification

	ROM_REGION( 530731, "screen_top", 0)
	ROM_LOAD( "gnw_gcliff_top.svg", 0, 530731, CRC(3bb60d8f) SHA1(e7dac1fcbe7b682c9d988443c1446e5ad28d3baa) )

	ROM_REGION( 519321, "screen_bottom", 0)
	ROM_LOAD( "gnw_gcliff_bottom.svg", 0, 519321, CRC(1117041e) SHA1(0f87167614c1ba65915fa7205a6bb44778e443a8) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // " -- Invincibility when playing on bottom screen only
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_zelda_state::gnw_zelda(machine_config &config)
{
	sm512_dualv(config, 1920/2, 1346/2, 1920/2, 1291/2);
}

// roms

ROM_START( gnw_zelda )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "zl-65.program", 0x0000, 0x1000, CRC(b96aa64e) SHA1(d1f0c64104eb3ecbf370674d5078a3a85b2b7227) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "zl-65.melody", 0x000, 0x100, BAD_DUMP CRC(3a281b0f) SHA1(7a236775557939050bbcd6f9d0a598d219a032f2) ) // decap needed for verification

	ROM_REGION( 283029, "screen_top", 0)
	ROM_LOAD( "gnw_zelda_top.svg", 0, 283029, CRC(aaab1d7e) SHA1(fe01e8a92e6dcf457da87afe6bf39fcf511da9db) )

	ROM_REGION( 424886, "screen_bottom", 0)
	ROM_LOAD( "gnw_zelda_bottom.svg", 0, 424886, CRC(09f00d09) SHA1(33045028bd7e0df4e976e79dc180028c6886359a) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario's Cement Factory (model CM-72)
  * PCB labels: CM-72 M (main board)
                CM-72 C (joystick controller board)
                CM-72 S (buttons controller board)
  * Sharp SM511 label CM-72 534A, or CM-72A 536C (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

  This is the tabletop version. There's also a new wide screen version which is
  a different game. Unlike the other tabletop games, there is no panorama version.
  There are two known versions, distinguished by the startup jingle. The first
  version sounds like Queen's "Another One Bites the Dust".

***************************************************************************/

class gnw_mariocmt_state : public hh_sm510_state
{
public:
	gnw_mariocmt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mariocmt(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mariocmt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Open
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mariocmt_state::gnw_mariocmt(machine_config &config)
{
	sm511_common(config, 1920, 1046);
}

// roms

ROM_START( gnw_mariocmt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cm-72.program", 0x0000, 0x1000, CRC(b2ae4596) SHA1(f64bf11e18c9fbd4de4134f685bb2d7bda3d7487) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "cm-72.melody", 0x000, 0x100, BAD_DUMP CRC(db4f0fc1) SHA1(e386df3e3e88fa36a73bcd0649feb904180493c8) ) // decap needed for verification

	ROM_REGION( 293317, "screen", 0)
	ROM_LOAD( "gnw_mariocmt.svg", 0, 293317, CRC(4f969dc7) SHA1(fec72c4a8600c0753f81bfb296b53cca6aee14cc) )
ROM_END

ROM_START( gnw_mariocmta )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cm-72a.program", 0x0000, 0x1000, CRC(b2ae4596) SHA1(f64bf11e18c9fbd4de4134f685bb2d7bda3d7487) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "cm-72a.melody", 0x000, 0x100, BAD_DUMP CRC(b6d72560) SHA1(9d7c23f94b7f894ba1b7881f68824949702a37f2) ) // decap needed for verification

	ROM_REGION( 293317, "screen", 0)
	ROM_LOAD( "gnw_mariocmt.svg", 0, 293317, CRC(4f969dc7) SHA1(fec72c4a8600c0753f81bfb296b53cca6aee14cc) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Snoopy (model SM-91)
  * PCB labels: SM-91 M (main board), SM-91C (controller board)
  * Sharp SM511 label SM-91 538A (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

  This is the panorama version. There's also a tabletop version which is
  assumed to use the same ROM/LCD.

***************************************************************************/

class gnw_snoopyp_state : public hh_sm510_state
{
public:
	gnw_snoopyp_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_snoopyp(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_snoopyp )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Hit
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_snoopyp_state::gnw_snoopyp(machine_config &config)
{
	sm511_common(config, 1920, 1020);
}

// roms

ROM_START( gnw_snoopyp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "sm-91.program", 0x0000, 0x1000, CRC(893bd7e3) SHA1(94e218f464b2ec8c81bd4c0f13f3a3049c4effe9) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "sm-91.melody", 0x000, 0x100, BAD_DUMP CRC(09360aaf) SHA1(906eff1d2eaf7ff040d833b4513a995e7026279b) ) // decap needed for verification

	ROM_REGION( 353488, "screen", 0)
	ROM_LOAD( "gnw_snoopyp.svg", 0, 353488, CRC(30cfa42e) SHA1(2abe74299db7241c66f9631b01d0ea336ec411ad) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Popeye (model PG-92)
  * PCB labels: PG-92 M (main board), SM-91C (controller board)
  * Sharp SM511 label PG-92 538A (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

  This is the panorama version. There's also a tabletop version which is
  assumed to use the same ROM/LCD, and a new wide screen version which is
  a different game.

  The PCB design for the controller board is shared with the panorama version
  of Snoopy (SM-91).

***************************************************************************/

class gnw_popeyep_state : public hh_sm510_state
{
public:
	gnw_popeyep_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_popeyep(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_popeyep )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Punch
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives and Stronger Punch (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_popeyep_state::gnw_popeyep(machine_config &config)
{
	sm511_common(config, 1920, 1043);
}

// roms

ROM_START( gnw_popeyep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pg-92.program", 0x0000, 0x1000, CRC(f9a2f181) SHA1(f97969abe63285964ef9585660e82590014bbece) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "pg-92.melody", 0x000, 0x100, BAD_DUMP CRC(ce2a0e03) SHA1(cb7e4c64639579349aa944e4bfff7b05cf49ce0e) ) // decap needed for verification

	ROM_REGION( 541218, "screen", 0)
	ROM_LOAD( "gnw_popeyep.svg", 0, 541218, CRC(ad93aa24) SHA1(b02c1fec1d8388878b5f21887f19aa5007b8ae43) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong Jr. (model CJ-93)
  * PCB labels: CJ-93 M (main board), CJ-93C (controller board)
  * Sharp SM511 label CJ-93 539D (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

  This is the panorama version. There's also a tabletop version which is
  assumed to use the same ROM/LCD, and a new wide screen version which is
  a different game.

  The tabletop version was also licensed to Coleco.

***************************************************************************/

class gnw_dkjrp_state : public hh_sm510_state
{
public:
	gnw_dkjrp_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkjrp(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkjrp )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkjrp_state::gnw_dkjrp(machine_config &config)
{
	sm511_common(config, 1920, 1049);
}

// roms

ROM_START( gnw_dkjrp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cj-93.program", 0x0000, 0x1000, CRC(a2cd5a91) SHA1(33f6fd1530e5522491851f16d7c9f928b2dbdc3b) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "cj-93.melody", 0x000, 0x100, BAD_DUMP CRC(99fbf76a) SHA1(15ba1af51bebc316146eb9a0a3d58d28f644d45f) ) // decap needed for verification

	ROM_REGION( 340751, "screen", 0)
	ROM_LOAD( "gnw_dkjrp.svg", 0, 340751, CRC(eb3cb98b) SHA1(5b148557d3ade2e2050ddde879a6cc05e119b446) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario's Bombs Away (model TB-94)
  * PCB labels: TB-94 M (main board), SM-91C (controller board)
  * Sharp SM511 label TB-94 537C (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_mbaway_state : public hh_sm510_state
{
public:
	gnw_mbaway_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mbaway(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mbaway )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Up/Down
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mbaway_state::gnw_mbaway(machine_config &config)
{
	sm511_common(config, 1920, 1031);
}

// roms

ROM_START( gnw_mbaway )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tb-94.program", 0x0000, 0x1000, CRC(11d18a48) SHA1(afccfa19dace7c4fcc15a84ecfcfb9d7ae3861e4) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "tb-94.melody", 0x000, 0x100, BAD_DUMP CRC(60d98353) SHA1(8789d7cd39111fe01848a89748ab91731de5caef) ) // decap needed for verification

	ROM_REGION( 514643, "screen", 0)
	ROM_LOAD( "gnw_mbaway.svg", 0, 514643, CRC(2ec2f18b) SHA1(8e2fd20615d867aac97e443fb977513ff98138b4) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mickey Mouse (model DC-95),
  Nintendo Game & Watch: Donkey Kong Circus (model MK-96)
  * PCB labels: DC-95M (main board), DC-95C (controller board)
  * Sharp SM511
     - label DC-95 284C (Mickey Mouse) (no decap)
     - label DC-95 541D (Donkey Kong Circus) (no decap)
  * inverted lcd screen with custom segments, 1-bit sound

  This is the panorama version of Mickey Mouse. There's also a wide screen
  version which is a different game.

  DC-95 and MK-96 are the same game, it's assumed that the latter was for
  regions where Nintendo wasn't able to license from Disney.

***************************************************************************/

class gnw_mmousep_state : public hh_sm510_state
{
public:
	gnw_mmousep_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mmousep(machine_config &config);
	void gnw_dkcirc(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mmousep )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mmousep_state::gnw_mmousep(machine_config &config)
{
	sm511_common(config, 1920, 1122);
}

void gnw_mmousep_state::gnw_dkcirc(machine_config &config)
{
	sm511_common(config, 1920, 1107);
}

// roms

ROM_START( gnw_mmousep )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dc-95.program", 0x0000, 0x1000, CRC(39dd864a) SHA1(25c67dac7320fe00990989cd42438461950a68ec) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "dc-95.melody", 0x000, 0x100, BAD_DUMP CRC(6ccde8e3) SHA1(4e704a1d61126465b14e3889b4a0179c5568b90b) ) // decap needed for verification

	ROM_REGION( 275609, "screen", 0)
	ROM_LOAD( "gnw_mmousep.svg", 0, 275609, CRC(bac13689) SHA1(3ddcb4416bc5b8615b2854434ef78acac204a583) )
ROM_END

ROM_START( gnw_dkcirc )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk-96.program", 0x0000, 0x1000, CRC(39dd864a) SHA1(25c67dac7320fe00990989cd42438461950a68ec) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "mk-96.melody", 0x000, 0x100, BAD_DUMP CRC(6ccde8e3) SHA1(4e704a1d61126465b14e3889b4a0179c5568b90b) ) // decap needed for verification

	ROM_REGION( 367718, "screen", 0)
	ROM_LOAD( "gnw_dkcirc.svg", 0, 367718, CRC(f8571437) SHA1(bc000267deab83dfd460aea5c4102a23ac51f169) )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkjr_state::gnw_dkjr(machine_config &config)
{
	sm510_common(config, 1647, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_dkjr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dj-101", 0x0000, 0x1000, CRC(8dcfb5d1) SHA1(e0ef578e9362eb9a3cab631376df3cf55978f2de) )

	ROM_REGION( 281202, "screen", 0)
	ROM_LOAD( "gnw_dkjr.svg", 0, 281202, CRC(f8b18d58) SHA1(fa8321b3d8f81685da763d66fc148d339e6bcd55) )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mariocm_state::gnw_mariocm(machine_config &config)
{
	sm510_common(config, 1647, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_mariocm )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ml-102_577", 0x0000, 0x1000, CRC(c1128dea) SHA1(8647e36f43a0e37756a3c7b6a3f08d4c8243f1cc) )

	ROM_REGION( 302983, "screen", 0)
	ROM_LOAD( "gnw_mariocm.svg", 0, 302983, CRC(32ed7941) SHA1(ce7c5ae7a179ec9bcd17db7d7a27780801f7c1cb) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Manhole (model NH-103)
  * PCB label NH-103
  * Sharp SM510 label NH-103 538A (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a Gold Series version
  (MH-06). The two games are using different MCU types so this version seems
  to be a complete rewrite.

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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_manhole_state::gnw_manhole(machine_config &config)
{
	sm510_common(config, 1560, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_manhole )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "nh-103", 0x0000, 0x1000, CRC(ec03acf7) SHA1(b74ae672d8f8a155b2ea4ecee9afbaed95ec0ceb) )

	ROM_REGION( 223414, "screen", 0)
	ROM_LOAD( "gnw_manhole.svg", 0, 223414, CRC(774d806b) SHA1(acb730d8e397eb29988a353e0a9db8ae69913117) )
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_tfish_state::gnw_tfish(machine_config &config)
{
	sm510_common(config, 1572, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_tfish )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tf-104", 0x0000, 0x1000, CRC(53cde918) SHA1(bc1e1b8f8b282bb886bb076c1c7ce35d00eca6fc) )

	ROM_REGION( 257396, "screen", 0)
	ROM_LOAD( "gnw_tfish.svg", 0, 257396, CRC(6f457a30) SHA1(0b748c9573ff96b99f4fa0adb17d218e89b56d3f) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Super Mario Bros. (model: see below)
  * PCB label YM-801 (Crystal Screen), YM-105 (New Wide Screen)
  * Sharp SM511
     - label YM-801 8034A (crystal screen) (not dumped yet)
     - label YM-105 9024B (new wide screen version) (die label ?)
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
	void gnw_smbn(machine_config & config);
};

// config

static INPUT_PORTS_START( gnw_smb )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_smb_state::gnw_smb(machine_config &config)
{
	sm511_common(config, 1768, 1080);
}

void gnw_smb_state::gnw_smbn(machine_config &config)
{
	sm511_common(config, 1677, 1080);
}

// roms

ROM_START( gnw_smb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ym-801.program", 0x0000, 0x1000, BAD_DUMP CRC(0dff3b12) SHA1(3fa83f88e49ea9d7080fe935ec90ce69acbe8850) ) // dumped from NWS version

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "ym-801.melody", 0x000, 0x100, BAD_DUMP CRC(b48c6d90) SHA1(a1ce1e52627767752974ab0d49bec48ead36663e) ) // dumped from NWS version

	ROM_REGION( 342106, "screen", 0)
	ROM_LOAD( "gnw_smb.svg", 0, 342106, CRC(243224ac) SHA1(9b7f41abe4e340e32893ff1ef6e4d696deadc637) )
ROM_END

ROM_START( gnw_smbn )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ym-105.program", 0x0000, 0x1000, CRC(0dff3b12) SHA1(3fa83f88e49ea9d7080fe935ec90ce69acbe8850) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "ym-105.melody", 0x000, 0x100, CRC(b48c6d90) SHA1(a1ce1e52627767752974ab0d49bec48ead36663e) )

	ROM_REGION( 648313, "screen", 0)
	ROM_LOAD( "gnw_smbn.svg", 0, 648313, CRC(5808c793) SHA1(06b90993eb9db2a1909509f99ebf00e27c20dcad) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Climber Crystal Screen (model DR-802),
  Nintendo Game & Watch: Climber New Wide Screen (model DR-106)
  * PCB label DR-802 (Crystal Screen), DR-106 (New Wide Screen)
  * Sharp SM511
     - label DR-802 8626A (crystal screen) (not dumped yet)
     - label DR-106 9038B (new wide screen version) (no decap)
  * lcd screen with custom segments, 1-bit sound

  First released in 1986 on Crystal Screen (model DR-802), rereleased on
  New Wide Screen in 1988 (model DR-106). The graphic LCD elements look the same
  in both versions but the display aspect ratio and the graphical background is
  slightly different. Until further proof, it's assumed that the ROM is the same
  for both models.

***************************************************************************/

class gnw_climber_state : public hh_sm510_state
{
public:
	gnw_climber_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_climber(machine_config &config);
	void gnw_climbern(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_climber )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Jump
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_climber_state::gnw_climber(machine_config &config)
{
	sm511_common(config, 1756, 1080);
}

void gnw_climber_state::gnw_climbern(machine_config &config)
{
	sm511_common(config, 1677, 1080);
}

// roms

ROM_START( gnw_climber )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dr-802.program", 0x0000, 0x1000, BAD_DUMP CRC(2adcbd6d) SHA1(110dc08c65120ab2c76ee647e89aa2726e24ac1a) ) // dumped from NWS version

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "dr-802.melody", 0x000, 0x100, BAD_DUMP CRC(7c49a3a3) SHA1(fad00d650b4864135c7d50f6fae735b7fffe720f) ) // dumped from NWS version

	ROM_REGION( 564868, "screen", 0)
	ROM_LOAD( "gnw_climber.svg", 0, 564868, CRC(a50ebd1c) SHA1(51047db960c8f110c1b681347cf8efd1d6263b85) )
ROM_END

ROM_START( gnw_climbern )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dr-106.program", 0x0000, 0x1000, CRC(2adcbd6d) SHA1(110dc08c65120ab2c76ee647e89aa2726e24ac1a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "dr-106.melody", 0x000, 0x100, BAD_DUMP CRC(7c49a3a3) SHA1(fad00d650b4864135c7d50f6fae735b7fffe720f) ) // decap needed for verification

	ROM_REGION( 542453, "screen", 0)
	ROM_LOAD( "gnw_climbern.svg", 0, 542453, CRC(2ded966e) SHA1(7e9c99d372b6e547b9b3e789dca9dee60455a427) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Balloon Fight Crystal Screen (model BF-803),
  Nintendo Game & Watch: Balloon Fight New Wide Screen (model BF-107)
  * PCB labels
     - DR-802-2 (Crystal Screen)
     - DR-106 (new wide screen version)
  * Sharp SM511
     - label BF-803 8646A (crystal screen) (not dumped yet)
     - label BF-107 9031B (new wide screen version) (no decap)
  * lcd screen with custom segments, 1-bit sound

  First released in 1986 on Crystal Screen (model BF-803), rereleased on
  New Wide Screen in 1988 (model BF-107). The graphic LCD elements look the same
  in both versions but the graphical background is slightly different.
  Until further proof, it's assumed that the ROM is the same for both models.

  The PCB design for the different editions seems to be shared with the
  corresponding editions of Climber.

***************************************************************************/

class gnw_bfight_state : public hh_sm510_state
{
public:
	gnw_bfight_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_bfight(machine_config &config);
	void gnw_bfightn(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_bfight )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Eject
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_bfight_state::gnw_bfight(machine_config &config)
{
	sm511_common(config, 1771, 1080);
}

void gnw_bfight_state::gnw_bfightn(machine_config &config)
{
	sm511_common(config, 1549, 1080);
}

// roms

ROM_START( gnw_bfight )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bf-803.program", 0x0000, 0x1000, BAD_DUMP CRC(4c8d07ed) SHA1(a8974dff85d5f3bacaadb71b86e9b30994b6d129) ) // dumped from NWS version

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bf-803.melody", 0x000, 0x100, BAD_DUMP CRC(ffddf9ed) SHA1(e9cb3a340924363eeef5ab453c452b9cc69207b9) ) // dumped from NWS version

	ROM_REGION( 586453, "screen", 0)
	ROM_LOAD( "gnw_bfight.svg", 0, 586453, CRC(40d81b65) SHA1(96ed909647229cfde6d733ba10d54ace29e5618a) )
ROM_END

ROM_START( gnw_bfightn )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bf-107.program", 0x0000, 0x1000, CRC(4c8d07ed) SHA1(a8974dff85d5f3bacaadb71b86e9b30994b6d129) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bf-107.melody", 0x000, 0x100, BAD_DUMP CRC(ffddf9ed) SHA1(e9cb3a340924363eeef5ab453c452b9cc69207b9) ) // decap needed for verification

	ROM_REGION( 558496, "screen", 0)
	ROM_LOAD( "gnw_bfightn.svg", 0, 558496, CRC(c488000e) SHA1(f9a042799a1489f83b07a91827b8b421238a67e8) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Mario The Juggler (model MB-108)
  * PCB label MB-108
  * Sharp SM511 label MB-108 9209B (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_mariotj_state : public hh_sm510_state
{
public:
	gnw_mariotj_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_mariotj(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_mariotj )
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_mariotj_state::gnw_mariotj(machine_config &config)
{
	sm511_common(config, 1630, 1080);
}

// roms

ROM_START( gnw_mariotj )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb-108.program", 0x0000, 0x1000, CRC(f7118bb4) SHA1(c3117fd009e4686a149f85fb65786ddffc091eeb) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "mb-108.melody", 0x000, 0x100, BAD_DUMP CRC(d8cc1f74) SHA1(4bbb470ef01777b0c1dbd7b84dc560da6d3b87e7) ) // decap needed for verification

	ROM_REGION( 210391, "screen", 0)
	ROM_LOAD( "gnw_mariotj.svg", 0, 210391, CRC(8f1e6118) SHA1(4ecad443142330470384659af1e8dd59bca519e4) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Spitball Sparky (model BU-201)
  * PCB label BU-201
  * Sharp SM510 label BU-201 542A (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_ssparky_state : public hh_sm510_state
{
public:
	gnw_ssparky_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_ssparky(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_ssparky )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) // Shooter
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_ssparky_state::gnw_ssparky(machine_config &config)
{
	sm510_common(config, 627, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_ssparky )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bu-201", 0x0000, 0x1000, CRC(ae0d28e7) SHA1(1427cca1f3aaf3ef6fc3499171a5220428d9894f) )

	ROM_REGION( 136929, "screen", 0)
	ROM_LOAD( "gnw_ssparky.svg", 0, 136929, CRC(66e5d586) SHA1(b666f675abb8edef65ff402e8bc9a5213b630851) )
ROM_END





/***************************************************************************

  Nintendo Game & Watch: Crab Grab (model UD-202)
  * PCB label UD-202
  * Sharp SM510 label UD-202 542B (no decap)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_cgrab_state : public hh_sm510_state
{
public:
	gnw_cgrab_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_cgrab(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_cgrab )
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

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Do not release crabs (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_cgrab_state::gnw_cgrab(machine_config &config)
{
	sm510_common(config, 609, 1080); // R mask option confirmed
}

// roms

ROM_START( gnw_cgrab )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ud-202", 0x0000, 0x1000, CRC(65e97963) SHA1(f6d589fac337e2c4acdaa8f1281912feabc54198) )

	ROM_REGION( 354770, "screen", 0)
	ROM_LOAD( "gnw_cgrab.svg", 0, 354770, CRC(d61478aa) SHA1(8dd44cfb3720740150defdfbebe0bd52a3b3a377) )
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

static INPUT_PORTS_START( microvs_shared )
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
INPUT_PORTS_END

static INPUT_PORTS_START( gnw_boxing )
	PORT_INCLUDE( microvs_shared )

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "P2 Decrease Health (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "P1 Infinite Health (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_boxing_state::gnw_boxing(machine_config &config)
{
	sm511_common(config, 1920, 524);
}

// roms

ROM_START( gnw_boxing )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bx-301_744.program", 0x0000, 0x1000, CRC(0fdf0303) SHA1(0b791c9d4874e9534d0a9b7a8968ce02fe4bee96) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bx-301_744.melody", 0x000, 0x100, CRC(439d943d) SHA1(52880df15ec7513f96482f455ef3d9778aa24750) )

	ROM_REGION( 265217, "screen", 0)
	ROM_LOAD( "gnw_boxing.svg", 0, 265217, CRC(306c733e) SHA1(8c80df1295ff0889e16ef9a14e45b27a6ebaa9a2) )
ROM_END





/***************************************************************************

  Nintendo Micro Vs. System: Donkey Kong 3 (model AK-302)
  * PCB label AK-302M
  * Sharp SM511 label AK-302 299D (no decap)
  * wide lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_dkong3_state : public hh_sm510_state
{
public:
	gnw_dkong3_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkong3(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkong3 )
	PORT_INCLUDE( microvs_shared )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "P1 Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkong3_state::gnw_dkong3(machine_config &config)
{
	sm511_common(config, 1920, 563);
}

// roms

ROM_START( gnw_dkong3 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ak-302.program", 0x0000, 0x1000, CRC(ed59c15e) SHA1(94f6ce23677d2150c9f86c4b1954f5f531693b21) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "ak-302.melody", 0x000, 0x100, BAD_DUMP CRC(8b8f3d55) SHA1(54ebdeff4dd56a8bc2cd39bca1deada14bb90cce) ) // decap needed for verification

	ROM_REGION( 292480, "screen", 0)
	ROM_LOAD( "gnw_dkong3.svg", 0, 292480, CRC(980d2486) SHA1(8578bdf4a3814401d9a79867252ee09ed7df253c) )
ROM_END





/***************************************************************************

  Nintendo Micro Vs. System: Donkey Kong Hockey (model HK-303)
  * PCB label HK-303M
  * Sharp SM511 label HK-303 57XD (no decap)
  * wide lcd screen with custom segments, 1-bit sound

***************************************************************************/

class gnw_dkhockey_state : public hh_sm510_state
{
public:
	gnw_dkhockey_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void gnw_dkhockey(machine_config &config);
};

// config

static INPUT_PORTS_START( gnw_dkhockey )
	PORT_INCLUDE( microvs_shared )

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Any Goal Scores 10 Points (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "P2 Goals Scores No Points (Cheat)") // "
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void gnw_dkhockey_state::gnw_dkhockey(machine_config &config)
{
	sm511_common(config, 1920, 579);
}

// roms

ROM_START( gnw_dkhockey )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "hk-303.program", 0x0000, 0x1000, CRC(dc73eec7) SHA1(daaca286de326321335fd26d9b435b444787f609) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "hk-303.melody", 0x000, 0x100, BAD_DUMP CRC(3f61032c) SHA1(b0fd9077fb5e59ca2787c828c78c35116c48c245) ) // decap needed for verification

	ROM_REGION( 263239, "screen", 0)
	ROM_LOAD( "gnw_dkhockey.svg", 0, 263239, CRC(3a576c12) SHA1(9a7ca67c35fcfb5858227f3ef2a6027c877c64d3) )
ROM_END





/***************************************************************************

  Telko Bassmate Computer (model BM-501)
  * PCB label BM-501
  * Sharp SM511 label BM-501 556AA (no decap)
  * vertical dual lcd screens with custom segments, 1-bit sound

  The Bassmate Computer was produced for Telko by Nintendo as an OEM product
  and sold under different brands, i.e. Telko, KMV and Probe 2000.

  The hardware is identical as G&W Multi Screen, but it's not part of the game
  series.

***************************************************************************/

class bassmate_state : public hh_sm510_state
{
public:
	bassmate_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void bassmate(machine_config &config);
};

// config

static INPUT_PORTS_START( bassmate )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Compute")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Wind")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Cover")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time of Day")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Water Temp F")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Water Clarity")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Structure")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Water Depth(ft)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Season")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")

	PORT_START("BA")
	PORT_CONFNAME( 0x01, 0x01, "Skip Compute Animation (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void bassmate_state::bassmate(machine_config &config)
{
	sm511_dualv(config, 1920/2, 1253/2, 1920/2, 1273/2);
}

// roms

ROM_START( bassmate )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bm-501.program", 0x0000, 0x1000, CRC(9bdd0501) SHA1(986b3b84184a987ae383c325700df21d8915f0e2) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bm-501.melody", 0x000, 0x100, BAD_DUMP CRC(fbe15600) SHA1(8be64792fffe5b8913a55b9b2624dd57dc238be7) ) // decap needed for verification

	ROM_REGION( 42305, "screen_top", 0)
	ROM_LOAD( "bassmate_top.svg", 0, 42305, CRC(0cc056fe) SHA1(4d0e5b115adf513f5b3148ca7e39e0acbafd925c) )

	ROM_REGION( 19775, "screen_bottom", 0)
	ROM_LOAD( "bassmate_bottom.svg", 0, 19775, CRC(9561d52d) SHA1(903ef3944810c0efdc02f46a619891c1ef17c483) )
ROM_END





/***************************************************************************

  Konami Double Dribble
  * Sharp SM510 under epoxy (die label CMS54C, KMS584)
  * lcd screen with custom segments, 1-bit sound

  BTANB: At the basket, the ball goes missing sometimes for 1 frame, or
  may show 2 balls at the same time. It's the same on the real handheld.
  BTANB: players flicker (increasing LCD delay won't improve it much)
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
	sm510_common(config, 1524, 1080); // R mask option confirmed
}

// roms

ROM_START( kdribble )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "584", 0x0000, 0x1000, CRC(1d9022c8) SHA1(64567f9f161e830a0634d5c89917ab866c26c0f8) )

	ROM_REGION( 450349, "screen", 0)
	ROM_LOAD( "kdribble.svg", 0, 450349, CRC(0ea4153e) SHA1(b5deb398bb9f5e56e5bbcbe477d54528fb989487) )
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
	sm510_common(config, 1515, 1080); // R mask option confirmed
}

// roms

ROM_START( ktopgun ) // except for filler/unused bytes, ROM listing in patent US5137277 "BH003 Top Gun" is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "598", 0x0000, 0x1000, CRC(50870b35) SHA1(cda1260c2e1c180995eced04b7d7ff51616dcef5) )

	ROM_REGION( 425839, "screen", 0)
	ROM_LOAD( "ktopgun.svg", 0, 425839, CRC(f0eb200f) SHA1(cbdc7cfaf1785b393c806dabd1a355d325bddc3f) )
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
	{
		// increase lcd decay: score digit flickers
		m_decay_len = 20;
	}

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
	sm511_common(config, 1505, 1080);
}

// roms

ROM_START( kcontra ) // except for filler/unused bytes, ROM listing in patent US5120057 "BH002 C (Contra)" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "773.program", 0x0000, 0x1000, CRC(bf834877) SHA1(055dd56ec16d63afba61ab866481fd9c029fb54d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "773.melody", 0x000, 0x100, CRC(23d02b99) SHA1(703938e496db0eeacd14fe7605d4b5c39e0a5bc8) )

	ROM_REGION( 721055, "screen", 0)
	ROM_LOAD( "kcontra.svg", 0, 721055, CRC(f1ce8d19) SHA1(7d8f2fac40605a3fd6f1386c945a53412b2f2b15) )
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
	sm511_common(config, 1505, 1080);
}

// roms

ROM_START( ktmnt ) // except for filler/unused bytes, ROM listing in patent US5150899 "BH005 TMNT" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "774.program", 0x0000, 0x1000, CRC(a1064f87) SHA1(92156c35fbbb414007ee6804fe635128a741d5f1) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "774.melody", 0x000, 0x100, CRC(8270d626) SHA1(bd91ca1d5cd7e2a62eef05c0033b19dcdbe441ca) )

	ROM_REGION( 610309, "screen", 0)
	ROM_LOAD( "ktmnt.svg", 0, 610309, CRC(9f48c50d) SHA1(917c0ed8e83d949e5115c897cacda8d60e42d74d) )
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
	sm511_common(config, 1420, 1080);
}

// roms

ROM_START( kgradius )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "771.program", 0x0000, 0x1000, CRC(830c2afc) SHA1(bb9ebd4e52831cc02cd92dd4b37675f34cf37b8c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "771.melody", 0x000, 0x100, CRC(4c586b73) SHA1(14c5ab2898013a577f678970a648c374749cc66d) )

	ROM_REGION( 638136, "screen", 0)
	ROM_LOAD( "kgradius.svg", 0, 638136, CRC(85dd296e) SHA1(bd75d0c08387a69bbcf4fd100252846499a261b3) )
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
	sm511_common(config, 1497, 1080);
}

// roms

ROM_START( kloneran )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "781.program", 0x0000, 0x1000, CRC(52b9735f) SHA1(06c5ef6e7e781b1176d4c1f2445f765ccf18b3f7) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "781.melody", 0x000, 0x100, CRC(a393de36) SHA1(55089f04833ccb318524ab2b584c4817505f4019) )

	ROM_REGION( 633161, "screen", 0)
	ROM_LOAD( "kloneran.svg", 0, 633161, CRC(1fb937ff) SHA1(9e5841dc67e50b789f0161693ebbd75f79915980) )
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
	{
		// increase lcd decay: too much overall flicker
		m_decay_len = 25;
	}

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
	sm511_common(config, 1516, 1080);
}

// roms

ROM_START( kblades )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "782.program", 0x0000, 0x1000, CRC(3351a35d) SHA1(84c64b65d3cabfa20c18f4649c9ede2578b82523) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "782.melody", 0x000, 0x100, CRC(e8bf48ba) SHA1(3852c014dc9136566322b4f9e2aab0e3ec3a7387) )

	ROM_REGION( 455154, "screen", 0)
	ROM_LOAD( "kblades.svg", 0, 455154, CRC(f17ec8ba) SHA1(ed999ef4b3f0ae94c243219ea8ea1eedd7179c26) )
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
	{
		// increase lcd decay: too much overall flicker
		m_decay_len = 35;
	}

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
	sm511_common(config, 1449, 1080);
}

// roms

ROM_START( knfl )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "786.program", 0x0000, 0x1000, CRC(0535c565) SHA1(44cdcd284713ff0b194b24beff9f1b94c8bc63b2) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "786.melody", 0x000, 0x100, CRC(6c80263b) SHA1(d3c21e2f8491fef101907b8e0871b1e1c1ed58f5) )

	ROM_REGION( 571173, "screen", 0)
	ROM_LOAD( "knfl.svg", 0, 571173, CRC(406c5bed) SHA1(1f3a704f091b78c89c06108ba11310f4072cc178) )
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
	sm511_common(config, 1490, 1080);
}

// roms

ROM_START( kbilly )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "788.program", 0x0000, 0x1000, CRC(b8b1f734) SHA1(619dd527187b43276d081cdb1b13e0a9a81f2c6a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "788.melody", 0x000, 0x100, CRC(cd488bea) SHA1(8fc60081f46e392978d6950c74711fb7ebd154de) )

	ROM_REGION( 598317, "screen", 0)
	ROM_LOAD( "kbilly.svg", 0, 598317, CRC(fec67ddf) SHA1(3e5f520733e8b720966028ed6a72062be5381f27) )
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
	sm511_common(config, 1490, 1080);
}

// roms

ROM_START( kbucky )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n58.program", 0x0000, 0x1000, CRC(7c36a0c4) SHA1(1b55ac64a71af746fd0a0f44266fcc92cca77482) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n58.melody", 0x000, 0x100, CRC(7e99e469) SHA1(3e9a3843c6ab392f5989f3366df87a2d26cb8620) )

	ROM_REGION( 727879, "screen", 0)
	ROM_LOAD( "kbucky.svg", 0, 727879, CRC(64cde7e6) SHA1(60c0120c7955a694bb07eb013e42c7a71757ab9f) )
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
	{
		// increase lcd decay: too much overall flicker
		m_decay_len = 30;
	}

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
	sm511_common(config, 1500, 1080);
}

// roms

ROM_START( kgarfld )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n62.program", 0x0000, 0x1000, CRC(5a762049) SHA1(26d4d891160d254dfd752734e1047126243f88dd) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n62.melody", 0x000, 0x100, CRC(232b7d55) SHA1(76f6a19e8182ee3f00c9f4ef007b5dde75a9c00d) )

	ROM_REGION( 581147, "screen", 0)
	ROM_LOAD( "kgarfld.svg", 0, 581147, CRC(ef2e5a61) SHA1(fbf0236cd0d4228403823d2623c6fd2d68349f7a) )
ROM_END





/***************************************************************************

  Tiger Gauntlet (model 7-778), Robin Hood (model 7-861)
  * Sharp SM510 under epoxy (die label CMS54C, KMS583)
  * lcd screen with custom segments, 1-bit sound

  known releases (Gauntlet):
  - World: Gauntlet, published by Tiger
  - Japan: Gauntlet, published by Sega
  - UK: Gauntlet, published by Grandstand

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
	sm510_tiger(config, 1425, 1080);
}

void tgaunt_state::trobhood(machine_config &config)
{
	sm510_tiger(config, 1468, 1080);
}

// roms

ROM_START( tgaunt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "583", 0x0000, 0x1000, CRC(598d8156) SHA1(9f776e8b9b4321e8118481e6b1304f8a38f9932e) )

	ROM_REGION( 713071, "screen", 0)
	ROM_LOAD( "tgaunt.svg", 0, 713071, CRC(b2dfb31b) SHA1(3e57c6aaa665e2874e6e7e051245a81ab7a917b3) )
ROM_END

ROM_START( trobhood )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "583", 0x0000, 0x1000, CRC(598d8156) SHA1(9f776e8b9b4321e8118481e6b1304f8a38f9932e) )

	ROM_REGION( 704816, "screen", 0)
	ROM_LOAD( "trobhood.svg", 0, 704816, CRC(f4b94f32) SHA1(8f68a7f4240489d42934d3875f82456aceabfb48) )
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
	sm510_tiger(config, 1467, 1080); // R mask option confirmed
}

// roms

ROM_START( tddragon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "593", 0x0000, 0x1000, CRC(2642f778) SHA1(fee77acf93e057a8b4627389dfd481c6d9cbd02b) )

	ROM_REGION( 511477, "screen", 0)
	ROM_LOAD( "tddragon.svg", 0, 511477, CRC(d3046671) SHA1(15fd328e28362402eab1094851dddd8e20a0bcec) )
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
	sm510_tiger(config, 1477, 1080);
}

// roms

ROM_START( tkarnov )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "582", 0x0000, 0x1000, CRC(cee85bdd) SHA1(143e39524f1dea523e0575f327ed189343cc87f5) )

	ROM_REGION( 527432, "screen", 0)
	ROM_LOAD( "tkarnov.svg", 0, 527432, CRC(9317066c) SHA1(087cfde97e106c6fd8c52d3a1138e6bde2ad9289) )
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
	sm510_tiger(config, 1459, 1080);
}

// roms

ROM_START( tvindictr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "595", 0x0000, 0x1000, CRC(b574d16f) SHA1(d2cb0f2e21ca2defe49a4b45f4c8e169ae9979ab) )

	ROM_REGION( 314205, "screen", 0)
	ROM_LOAD( "tvindictr.svg", 0, 314205, CRC(fefe9f31) SHA1(3c8e7ab2cd81de72740b2948def07a2fc000a78a) )
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

	void tgaiden(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// R2 connects to a single LED behind the screen
	void led_w(u8 data) { m_led_out = data >> 1 & 1; }
	output_finder<> m_led_out;
};

void tgaiden_state::machine_start()
{
	hh_sm510_state::machine_start();
	m_led_out.resolve();
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
	sm510_tiger(config, 1476, 1080);
	m_maincpu->write_r().append(FUNC(tgaiden_state::led_w));
}

// roms

ROM_START( tgaiden )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m82", 0x0000, 0x1000, CRC(278eafb0) SHA1(14396a0010bade0fde705969151200ed432321e7) )

	ROM_REGION( 588916, "screen", 0)
	ROM_LOAD( "tgaiden.svg", 0, 588916, CRC(5845c630) SHA1(c4b0d4d85e4b58a051920b6b34668847049c57a7) )
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
	sm510_tiger(config, 1442, 1080);
}

// roms

ROM_START( tbatman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "597", 0x0000, 0x1000, CRC(8b7acc97) SHA1(fe811675dc5c5ef9f6f969685c933926c8b9e868) )

	ROM_REGION( 551931, "screen", 0)
	ROM_LOAD( "tbatman.svg", 0, 551931, CRC(95ae104b) SHA1(0508f925f29b2152b41c478447e63c74fce718ad) )
ROM_END





/***************************************************************************

  Tiger Space Harrier II (model 7-814)
  * Sharp SM510 under epoxy (die label M91)
  * lcd screen with custom segments, 1-bit sound

  known releases:
  - World: Space Harrier II, published by Tiger
  - Japan: Space Harrier, published by Sega

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
	sm510_tiger(config, 1493, 1080); // R mask option confirmed
}

// roms

ROM_START( tsharr2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m91", 0x0000, 0x1000, CRC(b207ac79) SHA1(9889dfec26089313ba2bdac845a75a26742d09e1) )

	ROM_REGION( 555177, "screen", 0)
	ROM_LOAD( "tsharr2.svg", 0, 555177, CRC(842f8f7e) SHA1(2c523531059acdfa3a0cebac9d8f84f1971e1086) )
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
	sm510_tiger(config, 1479, 1080);
}

// roms

ROM_START( tstrider )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m92", 0x0000, 0x1000, CRC(4b488e8f) SHA1(b037c220c4a456f0dac67d759736f202a7609ee5) )

	ROM_REGION( 554858, "screen", 0)
	ROM_LOAD( "tstrider.svg", 0, 554858, CRC(12767799) SHA1(9d67a96affee18dacaf3d49d89f60940c33492aa) )
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
	sm510_tiger(config, 1456, 1080);
}

// roms

ROM_START( tgoldnaxe )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m94", 0x0000, 0x1000, CRC(af183fbf) SHA1(23716e2a7c4bb4842b2af1a43fe88db44e18dc17) )

	ROM_REGION( 605525, "screen", 0)
	ROM_LOAD( "tgoldnaxe.svg", 0, 605525, CRC(9c1097c5) SHA1(bf9c2a1f4ae98ebe5eb9d381b0729588a150fa27) )
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
	sm510_tiger(config, 1487, 1080);
}

void trobocop2_state::trockteer(machine_config &config)
{
	sm510_tiger(config, 1463, 1080);
}

// roms

ROM_START( trobocop2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m96", 0x0000, 0x1000, CRC(3704b60c) SHA1(04275833e1a79fd33226faf060890b66ae54e1d3) )

	ROM_REGION( 463572, "screen", 0)
	ROM_LOAD( "trobocop2.svg", 0, 463572, CRC(0218c1d9) SHA1(2932825ca03e008e5c2993882d363ae00df43f26) )
ROM_END

ROM_START( trockteer )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m96", 0x0000, 0x1000, CRC(3704b60c) SHA1(04275833e1a79fd33226faf060890b66ae54e1d3) )

	ROM_REGION( 558128, "screen", 0)
	ROM_LOAD( "trockteer.svg", 0, 558128, CRC(70ff1f46) SHA1(5cd94655654614206ed11844ba31650edb51eb22) )
ROM_END





/***************************************************************************

  Tiger Altered Beast (model 7-831)
  * Sharp SM510 under epoxy (die label M88)
  * lcd screen with custom segments, 1-bit sound

  known releases:
  - World: Altered Beast, published by Tiger
  - Japan: Juuouki, published by Sega

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
	sm510_tiger(config, 1455, 1080); // R mask option confirmed
}

// roms

ROM_START( taltbeast )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m88", 0x0000, 0x1000, CRC(1b3d15e7) SHA1(78371230dff872d6c07eefdbc4856c2a3336eb61) )

	ROM_REGION( 667931, "screen", 0)
	ROM_LOAD( "taltbeast.svg", 0, 667931, CRC(a642d5f7) SHA1(ad005deaa35189e59317a07d860403adeef51aad) )
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
	sm510_tiger(config, 1465, 1080);
}

// roms

ROM_START( tsf2010 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma2", 0x0000, 0x1000, CRC(764b3757) SHA1(c5f90b860128658576bb837e9cabbb3045ad2756) )

	ROM_REGION( 595191, "screen", 0)
	ROM_LOAD( "tsf2010.svg", 0, 595191, CRC(78f96bad) SHA1(4389580e3d47dda0243b01625427013eb7ec4336) )
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
	sm510_tiger(config, 1450, 1080);
}

// roms

ROM_START( tswampt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb0", 0x0000, 0x1000, CRC(8433530c) SHA1(60716d3bba92dc8ac3f1ee29c5734c9e894a1aff) )

	ROM_REGION( 578544, "screen", 0)
	ROM_LOAD( "tswampt.svg", 0, 578544, CRC(921e2b43) SHA1(176426fe9bf4855256e0ff7804ca48e619fa0cf3) )
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
	sm510_tiger(config, 1440, 1080);
}

// roms

ROM_START( tspidman )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma5", 0x0000, 0x1000, CRC(2624daed) SHA1(7c10434ae899637264de706045d48e3fce1d30a7) )

	ROM_REGION( 605375, "screen", 0)
	ROM_LOAD( "tspidman.svg", 0, 605375, CRC(6c032ce4) SHA1(69043096e28709821ddc26ef05f60f353fc3e35d) )
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
	sm510_tiger(config, 1467, 1080);
}

// roms

ROM_START( txmen )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma7", 0x0000, 0x1000, CRC(6f3ff34f) SHA1(aa24fbc3a4117ea51ebf951ee343a36c77692b72) )

	ROM_REGION( 543273, "screen", 0)
	ROM_LOAD( "txmen.svg", 0, 543273, CRC(039b37bb) SHA1(bd57cba8f380185beda2eb5ea7b5d1f25c8d447b) )
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
	sm510_tiger(config, 1514, 1080);
}

// roms

ROM_START( tddragon3 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ma6", 0x0000, 0x1000, CRC(8e2da0d9) SHA1(54dd05124b4c605975b0cb1eadd7456ff4a94d68) )

	ROM_REGION( 615734, "screen", 0)
	ROM_LOAD( "tddragon3.svg", 0, 615734, CRC(4c94d574) SHA1(e2717db6c0279da4813550f0035a23bdaaa8b7bb) )
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
	sm510_tiger(config, 1444, 1080);
}

// roms

ROM_START( tflash )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb5", 0x0000, 0x1000, CRC(f7f1d082) SHA1(49a7a931450cf27fe69076c4e15ffb34814e25d4) )

	ROM_REGION( 587863, "screen", 0)
	ROM_LOAD( "tflash.svg", 0, 587863, CRC(8ddb9391) SHA1(fa8b4610a914de8ae95123c4273a12cdb4353a39) )
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
	sm511_tiger1bit(config, 1471, 1080);
}

// roms

ROM_START( tmchammer )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n63.program", 0x0000, 0x1000, CRC(303aa6f7) SHA1(296689be1ee05238e52e9882812868b2ea96202c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n63.melody", 0x000, 0x100, CRC(77c1a5a3) SHA1(c00ae3b7c64dd9db96eab520fe674a40571fc15f) )

	ROM_REGION( 456487, "screen", 0)
	ROM_LOAD( "tmchammer.svg", 0, 456487, CRC(1cd10ff3) SHA1(092396c56adae7f872c3b5916ef3ecf67ab30161) )
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
	sm510_tiger(config, 1454, 1080);
}

// roms

ROM_START( tbtoads )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb3", 0x0000, 0x1000, CRC(8fa4c55a) SHA1(2be97e63dfed51313e180d7388dd431058db5a51) )

	ROM_REGION( 694417, "screen", 0)
	ROM_LOAD( "tbtoads.svg", 0, 694417, CRC(c0fbc25d) SHA1(def54ab2f10123246f7809fc3caf3e9d26800f87) )
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
	sm510_tiger(config, 1489, 1080);
}

// roms

ROM_START( thook )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mb7", 0x0000, 0x1000, CRC(7eb1a6e2) SHA1(f4a09ab95c968b0ddbe56cd7bb2667881c145731) )

	ROM_REGION( 680544, "screen", 0)
	ROM_LOAD( "thook.svg", 0, 680544, CRC(bddefb9b) SHA1(438b0fe6f4c0196952bd179075498d6bd19c3e48) )
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
	sm510_tiger(config, 1466, 1080);
}

// roms

ROM_START( tbttf )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc3", 0x0000, 0x1000, CRC(9c37a23c) SHA1(c09fa5caac8b574f8460265b98c0bea1d5e78c6a) )

	ROM_REGION( 667741, "screen", 0)
	ROM_LOAD( "tbttf.svg", 0, 667741, CRC(1a57e35a) SHA1(9b622e08cc44e3d48b71a283cd07b89fbcc6faa4) )
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
	sm510_tiger(config, 1464, 1080);
}

// roms

ROM_START( taddams )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc2", 0x0000, 0x1000, CRC(af33d432) SHA1(676ada238c389d1dd02dcb29731d69624f60b342) )

	ROM_REGION( 554703, "screen", 0)
	ROM_LOAD( "taddams.svg", 0, 554703, CRC(85f15123) SHA1(088dbfbe760b782988bbfe6d29d89b8427844992) )
ROM_END





/***************************************************************************

  Tiger Home Alone (model 78-502)
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
	sm510_tiger(config, 1448, 1080);
}

// roms

ROM_START( thalone )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc7", 0x0000, 0x1000, CRC(eceda335) SHA1(20c9ffcf914db61aba03716fe146bac42873ac82) )

	ROM_REGION( 494279, "screen", 0)
	ROM_LOAD( "thalone.svg", 0, 494279, CRC(2479d88d) SHA1(061413d6e0893106b335ca42cf0260f45c7dacc7) )
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
	sm510_tiger(config, 1464, 1080);
}

// roms

ROM_START( txmenpx )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "md3", 0x0000, 0x1000, CRC(11c2b09a) SHA1(f94b1e3e60f002398b39c98946469dd1a6aa8e77) )

	ROM_REGION( 572583, "screen", 0)
	ROM_LOAD( "txmenpx.svg", 0, 572583, CRC(b98f3134) SHA1(78ac7cd32b36f214b5e6eada725378ccbca91987) )
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
	sm510_tiger(config, 1454, 1080);
}

// roms

ROM_START( thalone2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "md7", 0x0000, 0x1000, CRC(ac8a21e9) SHA1(9024f74e34056f90b7dbf439300797183f74eb00) )

	ROM_REGION( 748928, "screen", 0)
	ROM_LOAD( "thalone2.svg", 0, 748928, CRC(d42ec743) SHA1(7df9654b7f700662f29ca7cabe25ac78b2c4b04b) )
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
	sm511_tiger2bit(config, 1517, 1080);
}

// roms

ROM_START( tsonic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n71.program", 0x0000, 0x1000, CRC(44cafd68) SHA1(bf8d0ab88d153fabc688ffec19959209ca79c3db) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n71.melody", 0x000, 0x100, CRC(bae258c8) SHA1(81cb75d73fab4479cd92fcb13d9cb03cec2afdd5) )

	ROM_REGION( 541491, "screen", 0)
	ROM_LOAD( "tsonic.svg", 0, 541491, CRC(ac6bff26) SHA1(fa944958eb64e8283d35951ff105cead28e6ab8a) )
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
	sm510_tiger(config, 1464, 1080);
}

// roms

ROM_START( trobocop3 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc6", 0x0000, 0x1000, CRC(07b44e4c) SHA1(3165c85e16c062d2d9d0c0f1b1f6bd6079b4de15) )

	ROM_REGION( 612142, "screen", 0)
	ROM_LOAD( "trobocop3.svg", 0, 612142, CRC(b55c1440) SHA1(e8f692deecf489be22be570510175b750d65d5c5) )
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
	sm510_tiger(config, 1441, 1080);
}

// roms

ROM_START( tdummies )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me0", 0x0000, 0x1000, CRC(29efae4a) SHA1(0b26913a3fd2fde2b39549f0f7cbc3daaa41eb50) )

	ROM_REGION( 525535, "screen", 0)
	ROM_LOAD( "tdummies.svg", 0, 525535, CRC(b0db8655) SHA1(937b3edd9c0949b9f7d01ef8920ac63b61e64909) )
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
	sm510_tiger(config, 1444, 1080);
}

// roms

ROM_START( tsfight2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me1", 0x0000, 0x1000, CRC(73384e94) SHA1(350417d101ce034b3974b4a1d2e04bcb3bf70605) )

	ROM_REGION( 630444, "screen", 0)
	ROM_LOAD( "tsfight2.svg", 0, 630444, CRC(42b82c9b) SHA1(8e18d0cfd629478973f2c857105daad70eae46d9) )
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
	sm510_tiger(config, 1429, 1080);
}

// roms

ROM_START( twworld )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "me7", 0x0000, 0x1000, CRC(dcb16d98) SHA1(539989e12bbc4a719818546c5edcfda02b98210e) )

	ROM_REGION( 527901, "screen", 0)
	ROM_LOAD( "twworld.svg", 0, 527901, CRC(515fede8) SHA1(e9c5adfb02b860fb97968957f282e685b1a4e3bc) )
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
	sm510_tiger(config, 1454, 1080);
}

// roms

ROM_START( tjpark )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf4", 0x0000, 0x1000, CRC(f66faf73) SHA1(4cfa743dcd6e44a3c1f56206d5824fddba16df01) )

	ROM_REGION( 812619, "screen", 0)
	ROM_LOAD( "tjpark.svg", 0, 812619, CRC(04d85ec6) SHA1(97f67bb496d985b62dc094f6f1d6b5597a4df895) )
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
	sm511_tiger2bit(config, 1475, 1080);
}

// roms

ROM_START( tsonic2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n86.program", 0x0000, 0x1000, CRC(782874c5) SHA1(b7eb1f56cbc781ba0b90f6b4b5b51944120733cc) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n86.melody", 0x000, 0x100, CRC(c16fa2b2) SHA1(222772d311fd3b3b05d80cfd539c2c862bed0be5) )

	ROM_REGION( 667927, "screen", 0)
	ROM_LOAD( "tsonic2.svg", 0, 667927, CRC(d2c52e67) SHA1(24ffbc8fae606dcd2f60a4d95fe1dcfd261c3576) )
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
	sm510_tiger(config, 1503, 1080);
}

// roms

ROM_START( tsddragon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf5", 0x0000, 0x1000, CRC(264c8e82) SHA1(470eb2f09a58ef05eb0b7c8e11380ad1d8ce4e1a) )

	ROM_REGION( 753572, "screen", 0)
	ROM_LOAD( "tsddragon.svg", 0, 753572, CRC(0759388b) SHA1(4e4acb1b97845e529522ba21de846ce1dc74357d) )
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
	sm510_tiger(config, 1467, 1080);
}

// roms

ROM_START( tdennis )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mf9", 0x0000, 0x1000, CRC(d95f54d5) SHA1(1b3a170f32deec98e54ad09c04b404f5ae03dcea) )

	ROM_REGION( 754896, "screen", 0)
	ROM_LOAD( "tdennis.svg", 0, 754896, CRC(6e7512c3) SHA1(b84ef988051a6a883f3435a779ea67e544d50dae) )
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

	void tnmarebc(machine_config &config);

private:
	virtual void input_w(u8 data) override;
};

// handlers

void tnmarebc_state::input_w(u8 data)
{
	// S5 and S6 tied together
	hh_sm510_state::input_w((data & 0x1f) | (data >> 1 & 0x10));
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
	sm510_tiger(config, 1456, 1080);
}

// roms

ROM_START( tnmarebc )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg0", 0x0000, 0x1000, CRC(5ef21421) SHA1(8fd458575111b89d7c33c969e76703bde5ad2c36) )

	ROM_REGION( 631351, "screen", 0)
	ROM_LOAD( "tnmarebc.svg", 0, 631351, CRC(140e278b) SHA1(f0d2b6e6ee2328f54255532ad03d5d505ebbb23b) )
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
	sm510_tiger(config, 1476, 1080);
}

// roms

ROM_START( ttransf2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg2", 0x0000, 0x1000, CRC(65c0f456) SHA1(b1bc3887c5088b3fe359585658e5c5236c09af9e) )

	ROM_REGION( 727708, "screen", 0)
	ROM_LOAD( "ttransf2.svg", 0, 727708, CRC(bd527f23) SHA1(9ce35bbfe1ea61c431eccaf32274faa4181587da) )
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
	sm510_tiger(config, 1450, 1080);
}

// roms

ROM_START( topaliens )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mj1", 0x0000, 0x1000, CRC(ccc196cf) SHA1(f18f7cf842cddecf90d05ab0f90257bb76514f54) )

	ROM_REGION( 1214917, "screen", 0)
	ROM_LOAD( "topaliens.svg", 0, 1214917, CRC(cbd57ab4) SHA1(efac0833f421212a81a8fc9b35c97369375aa1a9) )
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
	sm510_tiger(config, 1468, 1080);
}

// roms

ROM_START( tmkombat )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mg6", 0x0000, 0x1000, CRC(f6375dc7) SHA1(a711199c2623979f19c11067ebfff9355256c2c3) )

	ROM_REGION( 841871, "screen", 0)
	ROM_LOAD( "tmkombat.svg", 0, 841871, CRC(3bd5a963) SHA1(4d093f34c64caf60233e156fe160d8fceb15b6c6) )
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
	sm510_tiger(config, 1484, 1080);
}

// roms

ROM_START( tshadow )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mj5", 0x0000, 0x1000, CRC(09822d73) SHA1(30cae8b783a4f388193aee248fa18c6c1042e0ec) )

	ROM_REGION( 946494, "screen", 0)
	ROM_LOAD( "tshadow.svg", 0, 946494, CRC(e62de82e) SHA1(94fcef33066e97266efcd0d91b60229859088b36) )
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
	sm510_tiger(config, 1444, 1080);
}

// roms

ROM_START( tskelwarr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk0", 0x0000, 0x1000, CRC(dc7827a1) SHA1(74ff143605684df0c70db604a5f22dbf512044d7) )

	ROM_REGION( 1125043, "screen", 0)
	ROM_LOAD( "tskelwarr.svg", 0, 1125043, CRC(ee086073) SHA1(70e06a7fe8a1a8b8fee483c08522996da9917501) )
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
	sm510_tiger(config, 1493, 1080);
}

// roms

ROM_START( tbatfor )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk3", 0x0000, 0x1000, CRC(9993c382) SHA1(0c89e21024315ce7c086af5390c60f5766028c4f) )

	ROM_REGION( 902412, "screen", 0)
	ROM_LOAD( "tbatfor.svg", 0, 902412, CRC(d92d799e) SHA1(fa8b4198099748570a9cc9772c81aa583ac7ea72) )
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
	sm510_tiger(config, 1444, 1080);
}

// roms

ROM_START( tjdredd )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mk5", 0x0000, 0x1000, CRC(7beee5a7) SHA1(9a190197c5751b43a9ab2dc8c536934dc5fc5e83) )

	ROM_REGION( 1051637, "screen", 0)
	ROM_LOAD( "tjdredd.svg", 0, 1051637, CRC(0f3541c6) SHA1(9838e2ae5806a48be595f53e6096b7c150b91651) )
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
	sm510_tiger(config, 1467, 1080);
}

// roms

ROM_START( tapollo13 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_07", 0x0000, 0x1000, CRC(63d0deaa) SHA1(d5de99d5e0ee08ec2ebeef7189ebac1c008d2e7d) )

	ROM_REGION( 643219, "screen", 0)
	ROM_LOAD( "tapollo13.svg", 0, 643219, CRC(f4b94141) SHA1(d67367212f2be1685de0f3acaebaae0dc67734c4) )
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
	sm510_tiger(config, 1461, 1080);
}

// roms

ROM_START( tgoldeye )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_06", 0x0000, 0x1000, CRC(fe053efb) SHA1(3c90c0fa43e6e5e1f76b306e402f902d19175c96) )

	ROM_REGION( 938956, "screen", 0)
	ROM_LOAD( "tgoldeye.svg", 0, 938956, CRC(c4ad9836) SHA1(23555bd5fdaebed190ce02054a8ee681c88a8afb) )
ROM_END





/***************************************************************************

  Tiger Kazaam (model 78-613)
  * Sharp SM510 under epoxy (die label KMS10, 18)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tkazaam_state : public hh_sm510_state
{
public:
	tkazaam_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{
		inp_fixed_last();
	}

	void tkazaam(machine_config &config);
};

// config

static INPUT_PORTS_START( tkazaam )
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Pick/Rescue")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Forward")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Punch Back")
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

void tkazaam_state::tkazaam(machine_config &config)
{
	sm510_tiger(config, 1452, 1080); // no external XTAL
}

// roms

ROM_START( tkazaam )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_18", 0x0000, 0x1000, CRC(aa796372) SHA1(fdaea736e1df46c19fca08ed981e9659e038d15a) )

	ROM_REGION( 929109, "screen", 0)
	ROM_LOAD( "tkazaam.svg", 0, 929109, CRC(5eb178d4) SHA1(bcf64df4f342d16d5fd3d4cb503fceb1e5485591) )
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
	sm510_tiger(config, 1421, 1080); // no external XTAL
}

// roms

ROM_START( tsjam )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_23", 0x0000, 0x1000, CRC(6eaabfbd) SHA1(f0ecbd6f65fe72ce2d8a452685be2e77a63fc9f0) )

	ROM_REGION( 1046158, "screen", 0)
	ROM_LOAD( "tsjam.svg", 0, 1046158, CRC(29187365) SHA1(f277a064e6ecd6219c930736a0bdf56196279b42) )
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
	sm510_tiger(config, 1463, 1080);
}

// roms

ROM_START( tinday )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_16", 0x0000, 0x1000, CRC(77c2c2f7) SHA1(06326b26d0f6757180724ba0bdeb4110cc7e29d6) )

	ROM_REGION( 1162716, "screen", 0)
	ROM_LOAD( "tinday.svg", 0, 1162716, CRC(97056e62) SHA1(0348f0aca5d9b8e24f83c7e73b5e31f419fe60df) )
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Throw/Attack")
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
	sm511_tiger2bit(config, 1478, 1080);
}

// roms

ROM_START( tbatmana )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n81.program", 0x0000, 0x1000, CRC(efb3f122) SHA1(d55c2fb92fb9bd41d6001f42143691b84f3f389a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n81.melody", 0x000, 0x100, CRC(56ba8fe5) SHA1(5c286ae1bfc943bbe8c8f4cdc9c8b73d9b3c186e) )

	ROM_REGION( 618872, "screen", 0)
	ROM_LOAD( "tbatmana.svg", 0, 618872, CRC(8c721273) SHA1(d8c52441466943254bffcd6449af47a9fad6296b) )
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
	sm510_common(config, 1496, 1080); // R mask option confirmed
}

void trshutvoy_state::tigarden(machine_config &config)
{
	sm510_common(config, 1515, 1080);
}

// roms

ROM_START( trshutvoy )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0019_238e", 0x0000, 0x1000, CRC(8bd0eadd) SHA1(7bb5eb30d569901dce52d777bc01c0979e4afa06) )

	ROM_REGION( 221748, "screen", 0)
	ROM_LOAD( "trshutvoy.svg", 0, 221748, CRC(9e630b4e) SHA1(7e2a3a82519f29f7b1b92930604e010b5b9fdb06) )
ROM_END

ROM_START( tigarden )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0019_238e", 0x0000, 0x1000, CRC(8bd0eadd) SHA1(7bb5eb30d569901dce52d777bc01c0979e4afa06) )

	ROM_REGION( 409134, "screen", 0)
	ROM_LOAD( "tigarden.svg", 0, 409134, CRC(628f4aee) SHA1(5fa4843be61b52660a932e0d1efad403cf12de88) )
ROM_END





/***************************************************************************

  Tronica: Space Rescue (model MG-9), Thunder Ball (model FR-23)
  * PCB labels: SPACE RESCUE MG-9 080492 (MG-9)
                SPACE RESCUE MG-9 210982 (FR-23)
  * Sharp SM510 labels (no decap): 0015 224B TRONICA (MG-9)
                                   0015 236D TRONICA (FR-23)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class trsrescue_state : public hh_sm510_state
{
public:
	trsrescue_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void trsrescue(machine_config &config);
	void trthuball(machine_config &config);
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

	PORT_START("B")
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void trsrescue_state::trsrescue(machine_config &config)
{
	sm510_common(config, 1533, 1080); // R mask option confirmed
}

void trsrescue_state::trthuball(machine_config &config)
{
	sm510_common(config, 1599, 1080); // R mask option confirmed
}

// roms

ROM_START( trsrescue )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0015_224b", 0x0000, 0x1000, CRC(f58a3832) SHA1(2d843b3520de66463e628cea9344a04015d1f5f1) )

	ROM_REGION( 178760, "screen", 0)
	ROM_LOAD( "trsrescue.svg", 0, 178760, CRC(40756fd3) SHA1(9762ebbe4753a3194d7f0844c91addb8e1f8930b) )
ROM_END

ROM_START( trthuball )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0015_236d", 0x0000, 0x1000, CRC(f58a3832) SHA1(2d843b3520de66463e628cea9344a04015d1f5f1) )

	ROM_REGION( 175018, "screen", 0)
	ROM_LOAD( "trthuball.svg", 0, 175018, CRC(3404cc1d) SHA1(4cebe3c742c6947c6fddb8a84ae2f7d0cea1b527) )
ROM_END





/***************************************************************************

  Tronica: Space Mission (model SM-11), Spider (model SG-21)
  * PCB labels: SPACE MISSION SM-11 250582 (SM-11)
                SPACE MISSION SM-11 220982 (SG-21)
  * Sharp SM5A labels (no decap): 0126 228B TRONICA (SM-11)
                                  0126 22YC TRONICA (SG-21)
  * lcd screen with custom segments, 1-bit sound

  SM-11 and SG-21 are the exact same MCU, but with different graphics.

  In 1983, Tronica released a second revision of SG-21 with different graphic
  overlays. This version can be distinguished by having the year 1983 labeled
  on the backside of the unit.

***************************************************************************/

class trspacmis_state : public hh_sm510_state
{
public:
	trspacmis_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void trspacmis(machine_config &config);
	void trspider(machine_config & config);
};

// config

static INPUT_PORTS_START( trspacmis )
	PORT_START("IN.0") // R2
	PORT_CONFNAME( 0x01, 0x00, "Invincibility (Cheat)") // factory test, unpopulated on PCB
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // same as 0x01?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // display test?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // alarm test?

	PORT_START("IN.1") // R3
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void trspacmis_state::trspacmis(machine_config &config)
{
	sm5a_common(config, 1601, 1080); // R mask option confirmed
}

void trspacmis_state::trspider(machine_config &config)
{
	sm5a_common(config, 1597, 1080); // R mask option confirmed
}

// roms

ROM_START( trspacmis )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0126_228b", 0x0000, 0x0740, CRC(3d319537) SHA1(007d376cfd29574554caa59ed9163178179ae9c5) )

	ROM_REGION( 106675, "screen", 0)
	ROM_LOAD( "trspacmis.svg", 0, 106675, CRC(45d7b798) SHA1(db08fef21462507a115547ecf8eac38260a0c868) )
ROM_END

ROM_START( trspider )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "0126_22yc", 0x0000, 0x0740, CRC(3d319537) SHA1(007d376cfd29574554caa59ed9163178179ae9c5) )

	ROM_REGION( 130534, "screen", 0)
	ROM_LOAD( "trspider.svg", 0, 130534, CRC(d3f00245) SHA1(348c47dbe87ce8537c9b1c91d0d8bbf0809546e8) )
ROM_END





/***************************************************************************

  Elektronika Автослалом (Autoslalom) (model ИМ-23)
  * KB1013VK1-2 MCU
  * lcd screen with custom segments, 1-bit sound

  This is not an unlicensed clone, but doing a hex compare with MC-25
  still shows around 30% similarity so clearly they used that as a base.

***************************************************************************/

class auslalom_state : public hh_sm510_state
{
public:
	auslalom_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_sm510_state(mconfig, type, tag)
	{ }

	void auslalom(machine_config &config);
};

// config

static INPUT_PORTS_START( auslalom )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_CB(input_changed) PORT_NAME(u8"Запуск (Start)")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_CB(input_changed) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_CB(input_changed) PORT_NAME(u8"Скорость (Speed)")

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_CB(input_changed) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_CB(input_changed) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_CB(acl_button) PORT_NAME("ACL")
INPUT_PORTS_END

void auslalom_state::auslalom(machine_config &config)
{
	kb1013vk12_common(config, 1732, 1080); // R mask option ?
}

// roms

ROM_START( auslalom )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "im-23.bin", 0x0000, 0x0740, CRC(3b6e726f) SHA1(eabd04722811d1cc6519db9386b14a535f5aa865) )

	ROM_REGION( 117520, "screen", 0)
	ROM_LOAD( "auslalom.svg", 0, 117520, CRC(2f90fd4c) SHA1(f0de58b1fe2f7c18fc219f9f9a94c227ca1245e4) )
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CHANGED_CB(input_changed) PORT_NAME("Calc. / Clear")
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
	sm511_common(config, 1920, 875);
}

// roms

ROM_START( nummunch )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "772.program", 0x0000, 0x1000, CRC(2f7ff516) SHA1(132e7c5c4d69170953b2e51731992d6d6ba829f9) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "772.melody", 0x000, 0x100, CRC(96fe463a) SHA1(dcef5eee15a3f6d21e0db1b8ae3fbddc81633fc8) )

	ROM_REGION( 140704, "screen", 0)
	ROM_LOAD( "nummunch.svg", 0, 140704, CRC(050301d7) SHA1(3671d0e1b0cc788d74df0c6adb57a01729f66d7c) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME         PARENT   COMP  MACHINE      INPUT        CLASS              INIT        COMPANY, FULLNAME, FLAGS

// Nintendo G&W: Silver/Gold (initial series is uncategorized, "Silver" was made up later)
CONS( 1980, gnw_ball,     0,           0, gnw_ball,     gnw_ball,     gnw_ball_state,     empty_init, "Nintendo", "Game & Watch: Ball", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gnw_flagman,  0,           0, gnw_flagman,  gnw_flagman,  gnw_flagman_state,  empty_init, "Nintendo", "Game & Watch: Flagman", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gnw_vermin,   0,           0, gnw_vermin,   gnw_vermin,   gnw_vermin_state,   empty_init, "Nintendo", "Game & Watch: Vermin", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gnw_fires,    0,           0, gnw_fires,    gnw_fires,    gnw_fires_state,    empty_init, "Nintendo", "Game & Watch: Fire (Silver)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1980, gnw_judge,    0,           0, gnw_judge,    gnw_judge,    gnw_judge_state,    empty_init, "Nintendo", "Game & Watch: Judge (purple version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, gnw_judgeo,   gnw_judge,   0, gnw_judge,    gnw_judge,    gnw_judge_state,    empty_init, "Nintendo", "Game & Watch: Judge (green version)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_manholeg, 0,           0, gnw_manholeg, gnw_manholeg, gnw_manholeg_state, empty_init, "Nintendo", "Game & Watch: Manhole (Gold)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_helmet,   0,           0, gnw_helmet,   gnw_helmet,   gnw_helmet_state,   empty_init, "Nintendo", "Game & Watch: Helmet (version CN-17)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_helmeto,  gnw_helmet,  0, gnw_helmet,   gnw_helmet,   gnw_helmet_state,   empty_init, "Nintendo", "Game & Watch: Helmet (version CN-07)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_lion,     0,           0, gnw_lion,     gnw_lion,     gnw_lion_state,     empty_init, "Nintendo", "Game & Watch: Lion", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: Wide Screen
CONS( 1981, gnw_pchute,   0,           0, gnw_pchute,   gnw_pchute,   gnw_pchute_state,   empty_init, "Nintendo", "Game & Watch: Parachute", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_octopus,  0,           0, gnw_octopus,  gnw_octopus,  gnw_octopus_state,  empty_init, "Nintendo", "Game & Watch: Octopus", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_popeye,   0,           0, gnw_popeye,   gnw_popeye,   gnw_popeye_state,   empty_init, "Nintendo", "Game & Watch: Popeye (Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_chef,     0,           0, gnw_chef,     gnw_chef,     gnw_chef_state,     empty_init, "Nintendo", "Game & Watch: Chef", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_mmouse,   0,           0, gnw_mmouse,   gnw_mmouse,   gnw_mmouse_state,   empty_init, "Nintendo", "Game & Watch: Mickey Mouse (Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_egg,      gnw_mmouse,  0, gnw_egg,      gnw_mmouse,   gnw_mmouse_state,   empty_init, "Nintendo", "Game & Watch: Egg", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, gnw_fire,     0,           0, gnw_fire,     gnw_fire,     gnw_fire_state,     empty_init, "Nintendo", "Game & Watch: Fire (Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_tbridge,  0,           0, gnw_tbridge,  gnw_tbridge,  gnw_tbridge_state,  empty_init, "Nintendo", "Game & Watch: Turtle Bridge", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_fireatk,  0,           0, gnw_fireatk,  gnw_fireatk,  gnw_fireatk_state,  empty_init, "Nintendo", "Game & Watch: Fire Attack", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_stennis,  0,           0, gnw_stennis,  gnw_stennis,  gnw_stennis_state,  empty_init, "Nintendo", "Game & Watch: Snoopy Tennis", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: Multi Screen
CONS( 1982, gnw_opanic,   0,           0, gnw_opanic,   gnw_opanic,   gnw_opanic_state,   empty_init, "Nintendo", "Game & Watch: Oil Panic", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_dkong,    0,           0, gnw_dkong,    gnw_dkong,    gnw_dkong_state,    empty_init, "Nintendo", "Game & Watch: Donkey Kong", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_mickdon,  0,           0, gnw_mickdon,  gnw_mickdon,  gnw_mickdon_state,  empty_init, "Nintendo", "Game & Watch: Mickey & Donald", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, gnw_ghouse,   0,           0, gnw_ghouse,   gnw_ghouse,   gnw_ghouse_state,   empty_init, "Nintendo", "Game & Watch: Green House", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_dkong2,   0,           0, gnw_dkong2,   gnw_dkong2,   gnw_dkong2_state,   empty_init, "Nintendo", "Game & Watch: Donkey Kong II", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_mario,    0,           0, gnw_mario,    gnw_mario,    gnw_mario_state,    empty_init, "Nintendo", "Game & Watch: Mario Bros.", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_rshower,  0,           0, gnw_rshower,  gnw_rshower,  gnw_rshower_state,  empty_init, "Nintendo", "Game & Watch: Rain Shower", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_lboat,    0,           0, gnw_lboat,    gnw_lboat,    gnw_lboat_state,    empty_init, "Nintendo", "Game & Watch: Life Boat", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_pinball,  0,           0, gnw_pinball,  gnw_pinball,  gnw_pinball_state,  empty_init, "Nintendo", "Game & Watch: Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1985, gnw_bjack,    0,           0, gnw_bjack,    gnw_bjack,    gnw_bjack_state,    empty_init, "Nintendo", "Game & Watch: Black Jack", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1986, gnw_squish,   0,           0, gnw_squish,   gnw_squish,   gnw_squish_state,   empty_init, "Nintendo", "Game & Watch: Squish", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1987, gnw_bsweep,   0,           0, gnw_bsweep,   gnw_bsweep,   gnw_bsweep_state,   empty_init, "Nintendo", "Game & Watch: Bomb Sweeper", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, gnw_sbuster,  0,           0, gnw_sbuster,  gnw_sbuster,  gnw_sbuster_state,  empty_init, "Nintendo", "Game & Watch: Safe Buster", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, gnw_gcliff,   0,           0, gnw_gcliff,   gnw_gcliff,   gnw_gcliff_state,   empty_init, "Nintendo", "Game & Watch: Gold Cliff", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, gnw_zelda,    0,           0, gnw_zelda,    gnw_zelda,    gnw_zelda_state,    empty_init, "Nintendo", "Game & Watch: Zelda", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: New Wide Screen / Crystal Screen
CONS( 1982, gnw_dkjr,     0,           0, gnw_dkjr,     gnw_dkjr,     gnw_dkjr_state,     empty_init, "Nintendo", "Game & Watch: Donkey Kong Jr. (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_mariocm,  0,           0, gnw_mariocm,  gnw_mariocm,  gnw_mariocm_state,  empty_init, "Nintendo", "Game & Watch: Mario's Cement Factory (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_manhole,  0,           0, gnw_manhole,  gnw_manhole,  gnw_manhole_state,  empty_init, "Nintendo", "Game & Watch: Manhole (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1985, gnw_tfish,    0,           0, gnw_tfish,    gnw_tfish,    gnw_tfish_state,    empty_init, "Nintendo", "Game & Watch: Tropical Fish", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1986, gnw_smb,      0,           0, gnw_smb,      gnw_smb,      gnw_smb_state,      empty_init, "Nintendo", "Game & Watch: Super Mario Bros. (Crystal Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, gnw_smbn,     gnw_smb,     0, gnw_smbn,     gnw_smb,      gnw_smb_state,      empty_init, "Nintendo", "Game & Watch: Super Mario Bros. (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1986, gnw_climber,  0,           0, gnw_climber,  gnw_climber,  gnw_climber_state,  empty_init, "Nintendo", "Game & Watch: Climber (Crystal Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, gnw_climbern, gnw_climber, 0, gnw_climbern, gnw_climber,  gnw_climber_state,  empty_init, "Nintendo", "Game & Watch: Climber (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1986, gnw_bfight,   0,           0, gnw_bfight,   gnw_bfight,   gnw_bfight_state,   empty_init, "Nintendo", "Game & Watch: Balloon Fight (Crystal Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, gnw_bfightn,  gnw_bfight,  0, gnw_bfightn,  gnw_bfight,   gnw_bfight_state,   empty_init, "Nintendo", "Game & Watch: Balloon Fight (New Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, gnw_mariotj,  0,           0, gnw_mariotj,  gnw_mariotj,  gnw_mariotj_state,  empty_init, "Nintendo", "Game & Watch: Mario The Juggler", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: Table Top / Panorama Screen (the first Table Top releases in Japan were called "Color Screen")
CONS( 1983, gnw_mariocmt, 0,           0, gnw_mariocmt, gnw_mariocmt, gnw_mariocmt_state, empty_init, "Nintendo", "Game & Watch: Mario's Cement Factory (Table Top, version CM-72)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK ) // "Another one bites the dust" startup jingle
CONS( 1983, gnw_mariocmta,gnw_mariocmt,0, gnw_mariocmt, gnw_mariocmt, gnw_mariocmt_state, empty_init, "Nintendo", "Game & Watch: Mario's Cement Factory (Table Top, version CM-72A)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK ) // Plays an alternate jingle when starting a game
CONS( 1983, gnw_snoopyp,  0,           0, gnw_snoopyp,  gnw_snoopyp,  gnw_snoopyp_state,  empty_init, "Nintendo", "Game & Watch: Snoopy (Panorama Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_popeyep,  0,           0, gnw_popeyep,  gnw_popeyep,  gnw_popeyep_state,  empty_init, "Nintendo", "Game & Watch: Popeye (Panorama Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_dkjrp,    0,           0, gnw_dkjrp,    gnw_dkjrp,    gnw_dkjrp_state,    empty_init, "Nintendo", "Game & Watch: Donkey Kong Jr. (Panorama Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, gnw_mbaway,   0,           0, gnw_mbaway,   gnw_mbaway,   gnw_mbaway_state,   empty_init, "Nintendo", "Game & Watch: Mario's Bombs Away", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, gnw_mmousep,  0,           0, gnw_mmousep,  gnw_mmousep,  gnw_mmousep_state,  empty_init, "Nintendo", "Game & Watch: Mickey Mouse (Panorama Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, gnw_dkcirc,   gnw_mmousep, 0, gnw_dkcirc,   gnw_mmousep,  gnw_mmousep_state,  empty_init, "Nintendo", "Game & Watch: Donkey Kong Circus", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: Super Color
CONS( 1984, gnw_ssparky,  0,           0, gnw_ssparky,  gnw_ssparky,  gnw_ssparky_state,  empty_init, "Nintendo", "Game & Watch: Spitball Sparky", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, gnw_cgrab,    0,           0, gnw_cgrab,    gnw_cgrab,    gnw_cgrab_state,    empty_init, "Nintendo", "Game & Watch: Crab Grab", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W: Micro Vs. System (actually, no official Game & Watch logo anywhere)
CONS( 1984, gnw_boxing,   0,           0, gnw_boxing,   gnw_boxing,   gnw_boxing_state,   empty_init, "Nintendo", "Micro Vs. System: Boxing", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, gnw_dkong3,   0,           0, gnw_dkong3,   gnw_dkong3,   gnw_dkong3_state,   empty_init, "Nintendo", "Micro Vs. System: Donkey Kong 3", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, gnw_dkhockey, 0,           0, gnw_dkhockey, gnw_dkhockey, gnw_dkhockey_state, empty_init, "Nintendo", "Micro Vs. System: Donkey Kong Hockey", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Nintendo G&W hardware licensed to other companies (not part of G&W series)
CONS( 1984, bassmate,     0,           0, bassmate,     bassmate,     bassmate_state,     empty_init, "Telko / Nintendo", "Bassmate Computer", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Elektronika (mostly G&W clones)
CONS( 1988, taynyoke,     gnw_octopus, 0, taynyoke,     gnw_octopus,  gnw_octopus_state,  empty_init, "bootleg (Elektronika)", "Tayny okeana", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, vespovar,     gnw_chef,    0, vespovar,     gnw_chef,     gnw_chef_state,     empty_init, "bootleg (Elektronika)", "Vesyolyy povar", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1984, nupogodi,     gnw_mmouse,  0, nupogodi,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Nu, pogodi!", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1988, ehockey,      gnw_mmouse,  0, ehockey,      gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Hockey (Elektronika)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, rkosmosa,     gnw_mmouse,  0, rkosmosa,     rkosmosa,     gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Razvedchiki kosmosa", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, okhota,       gnw_mmouse,  0, okhota,       gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Okhota", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, biathlon,     gnw_mmouse,  0, biathlon,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Biathlon", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, vfutbol,      gnw_mmouse,  0, vfutbol,      gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Vesyolye futbolisty", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, krybolov,     gnw_mmouse,  0, krybolov,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Kot-rybolov (Elektronika)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kvakazad,     gnw_mmouse,  0, kvakazad,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Kvaka-zadavaka", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 19??, nochnyev,     gnw_mmouse,  0, nochnyev,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Nochnye vorishki", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 19??, kosmicpt,     gnw_mmouse,  0, kosmicpt,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Kosmicheskiy polyot", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 19??, morataka,     gnw_mmouse,  0, morataka,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Morskaja ataka", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, atakaast,     gnw_mmouse,  0, atakaast,     gnw_mmouse,   gnw_mmouse_state,   empty_init, "bootleg (Elektronika)", "Ataka asteroidov", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kosmicmt,     gnw_fire,    0, kosmicmt,     gnw_fire,     gnw_fire_state,     empty_init, "bootleg (Elektronika)", "Kosmicheskiy most", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, auslalom,     0,           0, auslalom,     auslalom,     auslalom_state,     empty_init, "Elektronika", "Autoslalom", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Konami
CONS( 1989, kdribble,     0,           0, kdribble,     kdribble,     kdribble_state,     empty_init, "Konami", "Double Dribble (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, ktopgun,      0,           0, ktopgun,      ktopgun,      ktopgun_state,      empty_init, "Konami", "Top Gun (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kcontra,      0,           0, kcontra,      kcontra,      kcontra_state,      empty_init, "Konami", "Contra (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, ktmnt,        0,           0, ktmnt,        ktmnt,        ktmnt_state,        empty_init, "Konami", "Teenage Mutant Ninja Turtles (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kgradius,     0,           0, kgradius,     kgradius,     kgradius_state,     empty_init, "Konami", "Gradius (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kloneran,     0,           0, kloneran,     kloneran,     kloneran_state,     empty_init, "Konami", "Lone Ranger (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kblades,      0,           0, kblades,      kblades,      kblades_state,      empty_init, "Konami", "Blades of Steel (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, knfl,         0,           0, knfl,         knfl,         knfl_state,         empty_init, "Konami", "NFL Football (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, kbilly,       0,           0, kbilly,       kbilly,       kbilly_state,       empty_init, "Konami", "The Adventures of Bayou Billy (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, kbucky,       0,           0, kbucky,       kbucky,       kbucky_state,       empty_init, "Konami", "Bucky O'Hare (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, kgarfld,      0,           0, kgarfld,      kgarfld,      kgarfld_state,      empty_init, "Konami", "Garfield (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Tiger 7-xxx/78-xxx models
CONS( 1989, tgaunt,       0,           0, tgaunt,       tgaunt,       tgaunt_state,       empty_init, "Tiger Electronics (licensed from Tengen)", "Gauntlet (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, trobhood,     tgaunt,      0, trobhood,     trobhood,     tgaunt_state,       empty_init, "Tiger Electronics", "Robin Hood (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, tddragon,     0,           0, tddragon,     tddragon,     tddragon_state,     empty_init, "Tiger Electronics (licensed from Technos/Tradewest)", "Double Dragon (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, tkarnov,      0,           0, tkarnov,      tkarnov,      tkarnov_state,      empty_init, "Tiger Electronics (licensed from Data East)", "Karnov (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, tvindictr,    0,           0, tvindictr,    tvindictr,    tvindictr_state,    empty_init, "Tiger Electronics (licensed from Tengen)", "Vindicators (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, tgaiden,      0,           0, tgaiden,      tgaiden,      tgaiden_state,      empty_init, "Tiger Electronics (licensed from Tecmo)", "Ninja Gaiden (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1989, tbatman,      0,           0, tbatman,      tbatman,      tbatman_state,      empty_init, "Tiger Electronics", "Batman (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, tsharr2,      0,           0, tsharr2,      tsharr2,      tsharr2_state,      empty_init, "Tiger Electronics (licensed from Sega)", "Space Harrier II (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, tstrider,     0,           0, tstrider,     tstrider,     tstrider_state,     empty_init, "Tiger Electronics (licensed from Capcom)", "Strider (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, tgoldnaxe,    0,           0, tgoldnaxe,    tgoldnaxe,    tgoldnaxe_state,    empty_init, "Tiger Electronics (licensed from Sega)", "Golden Axe (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, trobocop2,    0,           0, trobocop2,    trobocop2,    trobocop2_state,    empty_init, "Tiger Electronics", "Robocop 2 (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, trockteer,    trobocop2,   0, trockteer,    trockteer,    trobocop2_state,    empty_init, "Tiger Electronics", "The Rocketeer (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, taltbeast,    0,           0, taltbeast,    taltbeast,    taltbeast_state,    empty_init, "Tiger Electronics (licensed from Sega)", "Altered Beast (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1990, tsf2010,      0,           0, tsf2010,      tsf2010,      tsf2010_state,      empty_init, "Tiger Electronics (licensed from Capcom)", "Street Fighter 2010 - The Final Fight (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tswampt,      0,           0, tswampt,      tswampt,      tswampt_state,      empty_init, "Tiger Electronics", "Swamp Thing (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tspidman,     0,           0, tspidman,     tspidman,     tspidman_state,     empty_init, "Tiger Electronics", "Spider-Man (handheld, Tiger 1991 version)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, txmen,        0,           0, txmen,        txmen,        txmen_state,        empty_init, "Tiger Electronics", "X-Men (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tddragon3,    0,           0, tddragon3,    tddragon3,    tddragon3_state,    empty_init, "Tiger Electronics (licensed from Technos)", "Double Dragon 3 - The Rosetta Stone (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tflash,       0,           0, tflash,       tflash,       tflash_state,       empty_init, "Tiger Electronics", "The Flash (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tmchammer,    0,           0, tmchammer,    tmchammer,    tmchammer_state,    empty_init, "Tiger Electronics", "MC Hammer: U Can't Touch This (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, tbtoads,      0,           0, tbtoads,      tbtoads,      tbtoads_state,      empty_init, "Tiger Electronics (licensed from Rare/Tradewest)", "Battletoads (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1991, thook,        0,           0, thook,        thook,        thook_state,        empty_init, "Tiger Electronics", "Hook (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, tbttf,        0,           0, tbttf,        tbttf,        tbttf_state,        empty_init, "Tiger Electronics", "Back to the Future (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, taddams,      0,           0, taddams,      taddams,      taddams_state,      empty_init, "Tiger Electronics", "The Addams Family (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, thalone,      0,           0, thalone,      thalone,      thalone_state,      empty_init, "Tiger Electronics", "Home Alone (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, txmenpx,      0,           0, txmenpx,      txmenpx,      txmenpx_state,      empty_init, "Tiger Electronics", "X-Men - Project X (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, thalone2,     0,           0, thalone2,     thalone2,     thalone2_state,     empty_init, "Tiger Electronics", "Home Alone 2 - Lost in New York (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, tsonic,       0,           0, tsonic,       tsonic,       tsonic_state,       empty_init, "Tiger Electronics (licensed from Sega)", "Sonic The Hedgehog (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, trobocop3,    0,           0, trobocop3,    trobocop3,    trobocop3_state,    empty_init, "Tiger Electronics", "Robocop 3 (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tdummies,     0,           0, tdummies,     tdummies,     tdummies_state,     empty_init, "Tiger Electronics", "The Incredible Crash Dummies (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tsfight2,     0,           0, tsfight2,     tsfight2,     tsfight2_state,     empty_init, "Tiger Electronics (licensed from Capcom)", "Street Fighter II (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1992, twworld,      0,           0, twworld,      twworld,      twworld_state,      empty_init, "Tiger Electronics", "Wayne's World (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tjpark,       0,           0, tjpark,       tjpark,       tjpark_state,       empty_init, "Tiger Electronics", "Jurassic Park (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tsonic2,      0,           0, tsonic2,      tsonic2,      tsonic2_state,      empty_init, "Tiger Electronics (licensed from Sega)", "Sonic The Hedgehog 2 (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tsddragon,    0,           0, tsddragon,    tsddragon,    tsddragon_state,    empty_init, "Tiger Electronics (licensed from Technos)", "Super Double Dragon (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tdennis,      0,           0, tdennis,      tdennis,      tdennis_state,      empty_init, "Tiger Electronics", "Dennis the Menace (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tnmarebc,     0,           0, tnmarebc,     tnmarebc,     tnmarebc_state,     empty_init, "Tiger Electronics", "Nightmare Before Christmas (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK ) // note: title has no "The"
CONS( 1993, ttransf2,     0,           0, ttransf2,     ttransf2,     ttransf2_state,     empty_init, "Tiger Electronics", "Transformers - Generation 2 (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1994, topaliens,    0,           0, topaliens,    topaliens,    topaliens_state,    empty_init, "Tiger Electronics", "Operation: Aliens (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1993, tmkombat,     0,           0, tmkombat,     tmkombat,     tmkombat_state,     empty_init, "Tiger Electronics (licensed from Midway)", "Mortal Kombat (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1994, tshadow,      0,           0, tshadow,      tshadow,      tshadow_state,      empty_init, "Tiger Electronics", "The Shadow (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1994, tskelwarr,    0,           0, tskelwarr,    tskelwarr,    tskelwarr_state,    empty_init, "Tiger Electronics", "Skeleton Warriors - The Dark Crusade (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1995, tbatfor,      0,           0, tbatfor,      tbatfor,      tbatfor_state,      empty_init, "Tiger Electronics", "Batman Forever - Double Dose of Doom (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1995, tjdredd,      0,           0, tjdredd,      tjdredd,      tjdredd_state,      empty_init, "Tiger Electronics", "Judge Dredd (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1995, tapollo13,    0,           0, tapollo13,    tapollo13,    tapollo13_state,    empty_init, "Tiger Electronics", "Apollo 13 (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1995, tgoldeye,     0,           0, tgoldeye,     tgoldeye,     tgoldeye_state,     empty_init, "Tiger Electronics", "007: GoldenEye (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1996, tkazaam,      0,           0, tkazaam,      tkazaam,      tkazaam_state,      empty_init, "Tiger Electronics", "Kazaam (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1996, tsjam,        0,           0, tsjam,        tsjam,        tsjam_state,        empty_init, "Tiger Electronics", "Space Jam (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1996, tinday,       0,           0, tinday,       tinday,       tinday_state,       empty_init, "Tiger Electronics", "Independence Day (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Tiger 72-xxx models
CONS( 1992, tbatmana,     0,           0, tbatmana,     tbatmana,     tbatmana_state,     empty_init, "Tiger Electronics", "Batman: The Animated Series (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// Tronica
CONS( 1983, trshutvoy,    0,           0, trshutvoy,    trshutvoy,    trshutvoy_state,    empty_init, "Tronica", "Shuttle Voyage", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, tigarden,     trshutvoy,   0, tigarden,     trshutvoy,    trshutvoy_state,    empty_init, "Tronica", "Thief in Garden", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, trsrescue,    0,           0, trsrescue,    trsrescue,    trsrescue_state,    empty_init, "Tronica", "Space Rescue", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1983, trthuball,    trsrescue,   0, trthuball,    trsrescue,    trsrescue_state,    empty_init, "Tronica", "Thunder Ball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, trspacmis,    0,           0, trspacmis,    trspacmis,    trspacmis_state,    empty_init, "Tronica", "Space Mission (Tronica)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1982, trspider,     trspacmis,   0, trspider,     trspacmis,    trspacmis_state,    empty_init, "Tronica", "Spider (Tronica)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

// misc
CONS( 1989, nummunch,     0,           0, nummunch,     nummunch,     nummunch_state,     empty_init, "VTech", "Electronic Number Muncher", MACHINE_SUPPORTS_SAVE )
