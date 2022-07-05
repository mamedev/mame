// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Scott Stone, Couriersud, Felipe Sanches
/***************************************************************************

 Atari / Kee Games Driver - Discrete Games made in the 1970's


 Atari / Kee Games List (except for most Pong games) - Data based, in part from:

 - "Andy's collection of Bronzeage Atari Video Arcade PCBs"
 http://www.andysarcade.net/personal/bronzeage/index.htm

 - "Atari's Technical Manual Log"
 http://www.atarigames.com/manuals.txt

 Suspected "same games" are grouped together.  These are usually the exact same game but different cabinet/name.


 Technical Manual #s      Game Name(s)                                                    Atari Part #'s                     Data      PROM/ROM Chip Numbers
 -----------------------+---------------------------------------------------------------+----------------------------------+---------+---------------------------------------
 TM-025                   Anti-Aircraft (1975)                                            A000951                            YES       003127
 TM-048                   Crash 'N Score/Stock Car (1975)                                 A004256                            YES       003186(x2), 003187(x2), 004248, 004247
 TM-030                   Crossfire (1975)                                                A003022                            NO?
 TM-022                   Elimination! (1973)                                             A000845                            NO
 TM-035                   Goal IV (1975)                                                  A000823                            NO
 TM-016,029               Gotcha/Color Gotcha (1973)                                      A000816                            NO
 TM-003,005,011,020,029   Gran Trak 10/Trak 10/Formula K (1974)                           A000872,A000872 K3RT               YES       74186 Racetrack Prom (K5)
 TM-004,021               Gran Trak 20/Trak 20/Twin Racer (1974)                          A001791(RT20),A001793(A20-K4DRTA)  YES       74186 Racetrack prom (K5)
 TM-028                   Hi-Way/Highway (1975)                                           A003211                            NO
 TM-055                   Indy 4 (1976)                                                   A003000,A006268,A006270            YES       003186, 003187, 005502-01, 05503-01
 TM-026                   Indy 800 (1975)                                                 A003000,A003170,A003182            YES       003186-003189 (4)
                                                                                          A003184,A003191,A003198,A003199
 TM-027,052               Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (1975)         A004254,A004255                    YES       004250-004252, 004253-01 to 03 (3)
 TM-077                   Le Mans (1976)                                                  A005844,A005845                    YES       005837-01, 005838-01, 005839-01
 TM-040                   Outlaw (1976)                                                   A003213                            YES       003323 - ROM (8205 @ J4)
 TM-007                   Pin Pong (1974)                                                 A001660                            NO
 TM-019                   Pursuit (1975)                                                  K8P-B 90128                        NO
 TM-012,029,034           Quadrapong (1974)                                               A000845                            NO
 TM-009                   Qwak!/Quack (1974)                                              A000937,A000953                    YES       72074/37-2530N (K9)
 TM-047                   Shark JAWS (1975)                                               A003806                            YES       004182, 004183
 TM-008,029               Space Race (1973)                                               A000803                            NO
 TM-046                   Steeplechase/Astroturf (1975)                                   A003750                            YES       003774 ROM Bugle (C8), 003773-01 "A" Horse (C4), 003773-02 "B" Horse (D4)
 TM-057                   Stunt Cycle (1976)                                              A004128                            YES       004275 ROM Motorcycle/Bus (1F), 004811 ROM Score Translator (D7)
 TM-010,036               Tank/Tank Cocktail (1974)                                       A003111 (K5T-F 90124)              YES       90-2006 004800SD Tank Rom (K10)
 TM-049                   Tank II (1975)                                                  K5T-F 90124                        YES       90-2006
 TM-002                   Touch-Me (1974)                                                 ???????                            NO
 TM-006,017,029           World Cup/World Cup Football/Coupe du Monde/Coup Franc (1974)   A000823                            NO

 - Not Known to be released or produced, but at least announced.

 TM-0??                   Arcade Driver/Driver 1st Person (Not Produced/Released) (1974-75?)
 TM-018                   Dodgeball/Dodgem (Not Produced/Released) (1975)
 TM-024                   Qwakers (Not Produced/Released) (1975) (Kee Games clone of Qwak!?) - A bare PCB has been found marked QWAKERS A000950 ATARI (c)75

 - Information (current as of 27 Mar. 2019) on what logic chips (and some analog parts) are still needed to be emulated in the
   netlist system per-game:

 TM-057 (Stunt Cycle)
    1N751A Zener Diode
    1N752A Zener Diode

 TM-055 (Indy 4)
    7417  Hex Buffers/Drivers
    9301  1-of-10 Decoder
    LM339 Quad Comparator

***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "nl_stuntcyc.h"
#include "nl_gtrak10.h"
#include "nl_tank.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"
#include "screen.h"


namespace {

// copied by Pong, not accurate for this driver!
// start
#define MASTER_CLOCK    7159000
#define V_TOTAL         (0x105+1)       // 262
#define H_TOTAL         (0x1C6+1)       // 454

#define HBSTART         (H_TOTAL)
#define HBEND           (32)
#define VBSTART         (V_TOTAL)
#define VBEND           (16)

#define HRES_MULT       (1)
// end

#define SC_VIDCLOCK     (14318000/2)
#define SC_HTOTAL       (0x1C8+0)       // 456
#define SC_VTOTAL       (0x103+1)       // 259
#define SC_HBSTART      (SC_HTOTAL)
#define SC_HBEND        (32)
#define SC_VBSTART      (SC_VTOTAL)
#define SC_VBEND        (8)

#define TANK_VIDCLOCK   (14318181)
#define TANK_HTOTAL     (952)
#define TANK_VTOTAL     (262)

#define GTRAK10_VIDCLOCK (14318181 / 2)
#define GTRAK10_HTOTAL 451
#define GTRAK10_VTOTAL 521

class atarikee_state : public driver_device
{
public:
	atarikee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;

	void atarikee(machine_config &config);
};

class stuntcyc_state : public driver_device
{
public:
	stuntcyc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	void stuntcyc(machine_config &config);

private:
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;
};

class tank_state : public driver_device
{
public:
	tank_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "fixfreq")
	{
	}

	void tank(machine_config &config);

private:
	required_device<netlist_mame_device> m_maincpu;
	required_device<fixedfreq_device> m_video;
};

class gtrak10_state : public driver_device
{
public:
	gtrak10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video(*this, "fixfreq")
	{
	}

	void gtrak10(machine_config &config);

private:
	required_device<fixedfreq_device> m_video;
};

static NETLIST_START(atarikee)
{
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, atarikee_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
}


void atarikee_state::atarikee(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(netlist_atarikee);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(MASTER_CLOCK);
	m_video->set_horz_params(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL);
	m_video->set_vert_params(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.30);
}

#define STUNTCYC_NL_CLOCK (SC_HTOTAL*SC_VTOTAL*60*140)

void stuntcyc_state::stuntcyc(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, STUNTCYC_NL_CLOCK).set_source(netlist_stuntcyc);
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("VIDEO_OUT", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));
	NETLIST_LOGIC_INPUT(config, "maincpu:coinsw", "coinsw.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw1", "START1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw2", "START2.POS", 0);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(SC_VIDCLOCK);
	m_video->set_horz_params(SC_HTOTAL-84,SC_HTOTAL-64,SC_HTOTAL-16, SC_HTOTAL);
	m_video->set_vert_params(SC_VTOTAL-21,SC_VTOTAL-17,SC_VTOTAL-12, SC_VTOTAL);
	m_video->set_fieldcount(1);
	m_video->set_threshold(0.89);
	m_video->set_gain(0.2);
	m_video->set_horz_scale(4);
}

void tank_state::tank(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, m_maincpu, netlist::config::DEFAULT_CLOCK()).set_source(NETLIST_NAME(tank));
	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("VIDEO_OUT", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));
	NETLIST_LOGIC_INPUT(config, "maincpu:p1lup",   "P1_LEFT_UP.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1ldown", "P1_LEFT_DOWN.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1rup",   "P1_RIGHT_UP.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1rdown", "P1_RIGHT_DOWN.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p2lup",   "P2_LEFT_UP.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p2ldown", "P2_LEFT_DOWN.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p2rup",   "P2_RIGHT_UP.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p2rdown", "P2_RIGHT_DOWN.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1fire",  "P1_FIRE.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p2fire",  "P2_FIRE.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coin1",   "COIN1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coin2",   "COIN2.POS", 0);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(TANK_VIDCLOCK);
	//                    Length of active video,   end of front-porch,   end of sync signal,  end of back porch
	m_video->set_horz_params(776,                   776,                  808,                 904);
	m_video->set_vert_params(512,                   512,                  520,                 520);
	m_video->set_fieldcount(2);
	m_video->set_threshold(1.0);
	m_video->set_vsync_threshold(0.3);
	m_video->set_gain(0.47);
	m_video->set_horz_scale(3);
}

void gtrak10_state::gtrak10(machine_config &config)
{
	/* basic machine hardware */
	NETLIST_CPU(config, "maincpu", netlist::config::DEFAULT_CLOCK()).set_source(netlist_gtrak10);

	NETLIST_ANALOG_OUTPUT(config, "maincpu:vid0", 0).set_params("VIDEO_OUT", "fixfreq", FUNC(fixedfreq_device::update_composite_monochrome));

	NETLIST_LOGIC_INPUT(config, "maincpu:p1lup",    "P1_LEFT_UP.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1lleft",  "P1_LEFT_LEFT.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:p1lright", "P1_LEFT_RIGHT.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:coin1",    "COIN1.POS", 0);
	NETLIST_LOGIC_INPUT(config, "maincpu:startsw1", "STARTSW1.POS", 0);

	/* video hardware */

	/* Service Manual describes it as
	   "true interlaced raster scan"
	   "composed of 260.5 horizontal lines stacked on top of one another"

	   == PARAMETERS ==

	   Pixel Clock = 14.318MHz

	   Horiz Total       = 451
	   Horiz Front Porch =  ?
	   Horiz Sync        =  32
	   Horiz Back Porch  = ?

	   Vert Total       = 521
	   Vert Front Porch =   ?
	   Vert Sync        =   4
	   Vert Back Porch  =   ?
	*/

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	FIXFREQ(config, m_video).set_screen("screen");
	m_video->set_monitor_clock(GTRAK10_VIDCLOCK);
	//                    Length of active video,   end of front-porch,   end of sync signal,  end of line/frame
	m_video->set_horz_params(GTRAK10_HTOTAL  - 96, GTRAK10_HTOTAL - 64, GTRAK10_HTOTAL - 32, GTRAK10_HTOTAL);
	m_video->set_vert_params( GTRAK10_VTOTAL - 32, GTRAK10_VTOTAL -  8, GTRAK10_VTOTAL - 4,  GTRAK10_VTOTAL);
	m_video->set_fieldcount(2);
	m_video->set_threshold(1.0);
	m_video->set_gain(1.50);
	m_video->set_vsync_threshold(0.1);
	m_video->set_horz_scale(2);
}

static INPUT_PORTS_START( gtrak10 )
	// TODO
	// Temporary Controls to test car movement
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )    PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1lup")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )  PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1lleft")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1lright")

	PORT_START("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) NETLIST_LOGIC_PORT_CHANGED("maincpu", "coin1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1) NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw1")

INPUT_PORTS_END

static INPUT_PORTS_START( stuntcyc )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1)    NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2)    NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw2")
INPUT_PORTS_END

static INPUT_PORTS_START( tank )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )    PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1lup")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )  PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1ldown")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1rup")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )   PORT_2WAY NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1rdown")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )    PORT_2WAY PORT_PLAYER(2) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1lup")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )  PORT_2WAY PORT_PLAYER(2) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1ldown")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_2WAY PORT_PLAYER(2) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1rup")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )   PORT_2WAY PORT_PLAYER(2) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1rdown")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p1fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) NETLIST_LOGIC_PORT_CHANGED("maincpu", "p2fire")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) NETLIST_LOGIC_PORT_CHANGED("maincpu", "coin1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) NETLIST_LOGIC_PORT_CHANGED("maincpu", "coin2")
INPUT_PORTS_END

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( antiairc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003127.k1",     0x0000, 0x0020, CRC(9de772d5) SHA1(2855ba908d8e14a5aca43d4e0594d19f23fe9aae) ) // Anti-Aircraft Target
ROM_END


ROM_START( crashnsc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.f6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video #1
	ROM_LOAD( "003186.p6",     0x0200, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video #2

	ROM_REGION( 0x0040, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion #1
	ROM_LOAD( "003187.p7",     0x0020, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion #2

	ROM_REGION( 0x0200, "location", ROMREGION_ERASE00 )
	ROM_LOAD( "004248.d2",     0x0000, 0x0200, CRC(683b203b) SHA1(97202da5dd4a6cb66714d8e58ecee5c6efa65c1c) ) // Car Location Code

	ROM_REGION( 0x0200, "shape", ROMREGION_ERASE00 )
	ROM_LOAD( "004247.e2",     0x0000, 0x0200, CRC(478afac2) SHA1(fb15af0d2fc9d9ed0e92a3e7610c22dadf91d012) ) // Car Shape Code
ROM_END


ROM_START( gtrak10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "maincpu:gamedata", ROMREGION_ERASE00 )
	ROM_LOAD( "074186.j5",    0x0000, 0x0800, CRC(3bad3280) SHA1(b83fe1a1dc6bf20717dadf576f1d817496340f8c) ) // not actually a SN74186 but an Electronic Arrays, Inc. EA4800 16K (2048 x 8) ROM. TI TMS4800 clone (EA4800). Intentionally mislabeled by Atari.
ROM_END


ROM_START( gtrak10a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "maincpu:gamedata", ROMREGION_ERASE00 )
	ROM_LOAD( "074181.j5",    0x0000, 0x0800, CRC(f564c58a) SHA1(8097419e22bd8b5fd2a9fe4ea89302046c42e583) ) // not actually a SN74181 but an Electronic Arrays, Inc. EA4800 16K (2048 x 8) ROM. TI TMS4800 clone (EA4800). Intentionally mislabeled by Atari.
ROM_END


ROM_START( gtrak20 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, "maincpu:gamedata", ROMREGION_ERASE00 )
	ROM_LOAD( "074187.b3",    0x0000, 0x0800, CRC(d38709ca) SHA1(1ea5d174dbd0faa0c8aba6b8c845c62b18d9e60b) )
	ROM_LOAD( "074187a.d3",   0x0800, 0x0800, CRC(3d30654f) SHA1(119bac8ba8c300c026decf3f59a7da4e5d746648) )
	ROM_LOAD( "074187b.f3",   0x1000, 0x0800, CRC(a811cc11) SHA1(a0eb3f732268e796068d1a6c96cdddd1fd7fba21) )
ROM_END


ROM_START( indy4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.p6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video

	ROM_REGION( 0x0020, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion

	ROM_REGION( 0x0020, "checkpoint", ROMREGION_ERASE00 )
	ROM_LOAD( "005502.e5",     0x0000, 0x0020, CRC(e30ea877) SHA1(86f1f2c2e6e8472f7019f17bac723cb36faf098a) ) // Check Points

	ROM_REGION( 0x0200, "racetrack", ROMREGION_ERASE00 )
	ROM_LOAD( "005503.f4",     0x0000, 0x0200, CRC(1aafbe72) SHA1(c59829eccfe5a6014acad9682c401ca3f32fdfc9) ) // Race Track
ROM_END


ROM_START( indy800 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003186.p6",     0x0000, 0x0200, CRC(b3443354) SHA1(f43b82fd5d02dad2f597f890f5845701e73476a5) ) // Car Video

	ROM_REGION( 0x0020, "motion", ROMREGION_ERASE00 )
	ROM_LOAD( "003187.f7",     0x0000, 0x0020, CRC(01dca5b9) SHA1(0e3fbefc5df993b5a6a724aee258653897954255) ) // Car Motion

	ROM_REGION( 0x0020, "checkpoint", ROMREGION_ERASE00 )
	ROM_LOAD( "003188.e5",     0x0000, 0x0020, NO_DUMP ) // Check Points - Might be same as indy4?

	ROM_REGION( 0x0200, "racetrack", ROMREGION_ERASE00 )
	ROM_LOAD( "003189.f4",     0x0000, 0x0200, NO_DUMP ) // Race Track - Might be same as indy4?
ROM_END


ROM_START( jetfight )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0040, "shell", ROMREGION_ERASE00 )
	ROM_LOAD( "004250.m1",     0x0000, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom
	ROM_LOAD( "004250.j1",     0x0020, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom

	ROM_REGION( 0x0020, "singleplayer", ROMREGION_ERASE00 )
	ROM_LOAD( "004251.r5",     0x0000, 0x0020, CRC(bd95f87e) SHA1(4bd863104f1a7260b95f3fb2c13f40b7337d3dd9) ) // Single Player Rom

	ROM_REGION( 0x0100, "score", ROMREGION_ERASE00 )
	ROM_LOAD( "004252.a4",     0x0000, 0x0100, CRC(08a0b011) SHA1(71998728604a152006550869afe60d405643ccf1) ) // Score Rom

	ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 )
	/* Note:  Use 004253-01 and 004253-02 or use 004253-03 ONLY, not both together.  Presumably, -03 = data from -02 and -01 */
	ROM_LOAD( "004253-02.j5",  0x0000, 0x0200, CRC(c58ee65d) SHA1(785f842897a2ce92ce2f009e9b6d8e96950deb1f) ) // Picture & S.C. Rom A
	ROM_LOAD( "004253-01.k5",  0x0200, 0x0200, CRC(0d5648a9) SHA1(7a79ca587376678d9735f025d59088e6686fd783) ) // Picture & S.C. Rom B
//  ROM_LOAD( "004253-03.f5",  0x0000, 0x0400, NO_DUMP ) // Picture & S.C. Rom C
ROM_END


ROM_START( jetfighta )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0040, "shell", ROMREGION_ERASE00 )
	ROM_LOAD( "004250.m1",     0x0000, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom
	ROM_LOAD( "004250.j1",     0x0020, 0x0020, CRC(bee62d20) SHA1(2ea5fd7b087004c37901d2a56da2d6f6dcce9e29) ) // Shell Rom

	ROM_REGION( 0x0020, "singleplayer", ROMREGION_ERASE00 )
	ROM_LOAD( "004251.r5",     0x0000, 0x0020, CRC(bd95f87e) SHA1(4bd863104f1a7260b95f3fb2c13f40b7337d3dd9) ) // Single Player Rom

	ROM_REGION( 0x0200, "score", ROMREGION_ERASE00 )
	ROM_LOAD( "jet.a4",        0x0000, 0x0200, CRC(9e267e44) SHA1(b1c74ab275e30ed41c60e8490eaaf5211ec14ec5) ) // Score Rom

	ROM_REGION( 0x0800, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "jet.j5",        0x0000, 0x0400, CRC(853d61b3) SHA1(c5e1b09153b813b7b4042246e5634cc83de9654c) ) // Picture & S.C. Rom A
	ROM_LOAD( "jet.k5",        0x0400, 0x0400, CRC(a3fada62) SHA1(2efed600683e35ffa10acc5a301e736989c9f236) ) // Picture & S.C. Rom B
ROM_END


ROM_START( lemans )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "005837.n5",     0x0000, 0x0800, CRC(21a0c26a) SHA1(2bfe5ff415e4f252caf123ec80a32e6b8220c73a) )
	ROM_LOAD( "005838.n4",     0x0800, 0x0800, CRC(9b8fc4fd) SHA1(faf043922f0536e5a93fe6ed99d712503a8c4eb1) )
	ROM_LOAD( "005839.n6",     0x1000, 0x0800, CRC(4b1139bb) SHA1(c6418466f251054cbfe889895ec9bb55272f7575) )
ROM_END


ROM_START( qwakttl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "90-2002.9k",    0x0000, 0x0200, CRC(6d3b6270) SHA1(08e295efebc56ed87f56b93b74f87fc7f1df5213) ) // 37-2530n in manual
ROM_END


ROM_START( outlaw )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "0003323.j4",  0x0000, 0x0200, CRC(3166dad9) SHA1(4fca88b4256d8fb3e0deca54a15ffaafb830831e) ) // Rom (8205)
ROM_END


ROM_START( sharkjaw )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "004182.da1",  0x0000, 0x0100, CRC(05242912) SHA1(d3925cde795f04ac04151165bbbff74b15dce5ca) ) // Shark & Fish P-Rom
	ROM_LOAD( "004183.db1",  0x0100, 0x0100, CRC(b161b889) SHA1(009c6fc93174df15fb6a7993a73cfda56c8edfa2) )// Diver P-Rom
ROM_END


ROM_START( steeplec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0220, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "003773-a.4c",  0x0000, 0x0100, CRC(5ddc49b6) SHA1(58eba996703cbb7b3f66ff97357e191c9a3ab340) ) // Horse Graphics
	ROM_LOAD( "003773-b.4d",  0x0100, 0x0100, CRC(e6994cde) SHA1(504f92dba0c8640d55c7412697868582043f3817) ) // Horse Graphics
	ROM_LOAD( "003774.8c",  0x0200, 0x0020, CRC(f3785f4a) SHA1(98f4015049279de5ba109e6dd87bb94071df5860) ) // Bugle
ROM_END


ROM_START( stuntcyc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0200, "maincpu:004275.f1", ROMREGION_ERASE00 )
	ROM_LOAD( "004275.f1",  0x0000, 0x0200, CRC(4ed5a99d) SHA1(1e5f439bce72e78dfff76fd8f61187c6ef484a64) ) // Motorcycle & Bus

	ROM_REGION( 0x0020, "maincpu:004811.d7", ROMREGION_ERASE00 )
	ROM_LOAD( "004811.d7",  0x0000, 0x0020, CRC(31a09efb) SHA1(fd5d538c9ec1234acf7c74ca0704113d220abbf6) ) // Score Translator
ROM_END


ROM_START( tank )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	/* The "custom" 24-pin ROM used in Atari/Kee Games "Tank" is known as a MOSTEK MK28000P. */
	ROM_REGION( 0x0801, "maincpu:gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "90-2006.k10",  0x0000, 0x0800, CRC(87f5c365) SHA1(bc518a5795ef3ed8a7c0463653d70f60780ddda1) )
ROM_END

ROM_START( tankii )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	/* The "custom" 24-pin ROM used in Atari/Kee Games "Tank" is known as a MOSTEK MK28000P. */
	ROM_REGION( 0x0801, "gfx", ROMREGION_ERASE00 ) // 2049 Byte Size?
	ROM_LOAD( "90-2006.k10" ,0x0000, 0x0801, CRC(c25f6014) SHA1(7bd3fca5f64c928a645ca27c643b736667cef216) )
ROM_END

/*  // NO DUMPED ROMS

ROM_START( astrotrf )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

    ROM_REGION( 0x0400, "gfx", ROMREGION_ERASE00 ) // Region Size unknown, dump size unknown
    ROM_LOAD( "003774.c8",     0x0000, 0x0100, NO_DUMP ) // Bugle
    ROM_LOAD( "003773-02.c4",  0x0100, 0x0100, NO_DUMP ) // Graphics (Astroturf - Rev.A)
ROM_END

*/


/*  // 100% TTL - NO ROMS

// Crossfire (1975)
// Unclear if this is 100% TTL or if it uses a ROM:
// IC description in manual says a rom is used (74186 ROM)
// but the parts list in the same manual mentions no IC 74186!
// Simulated in DICE without ROMs from schematics, so unlikely
// it uses any, and is in fact 100% TTL..
ROM_START ( crossfir )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( eliminat )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( goaliv )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( gotchaat )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( gotchaatc )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( hiway )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pinpong )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( pursuit )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( quadpong )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( spacrace )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( touchme )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( worldcup )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( coupdmnd ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( coupfran ) // dummy to satisfy game entry
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

*/

} // Anonymous namespace


GAME(1975,  antiairc,  0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Anti-Aircraft [TTL]",    MACHINE_IS_SKELETON)
GAME(1975,  crashnsc,  0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Crash 'n Score/Stock Car [TTL]",   MACHINE_IS_SKELETON)
GAME(1974,  gtrak10,   0,        gtrak10,   gtrak10,  gtrak10_state, empty_init, ROT0,  "Atari/Kee",    "Gran Trak 10/Trak 10/Formula K [TTL]",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1974,  gtrak10a,  gtrak10,  atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Gran Trak 10/Trak 10/Formula K (older) [TTL]",     MACHINE_IS_SKELETON)
GAME(1974,  gtrak20,   0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Gran Trak 20/Trak 20/Twin Racer [TTL]",    MACHINE_IS_SKELETON)
GAME(1976,  indy4,     0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Indy 4 [TTL]",           MACHINE_IS_SKELETON)
GAME(1975,  indy800,   0,        atarikee,        0, atarikee_state, empty_init, ROT90, "Atari/Kee",    "Indy 800 [TTL]",         MACHINE_IS_SKELETON)
GAME(1975,  jetfight,  0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (set 1) [TTL]",      MACHINE_IS_SKELETON)
GAME(1975,  jetfighta, jetfight, atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Jet Fighter/Jet Fighter Cocktail/Launch Aircraft (set 2) [TTL]",      MACHINE_IS_SKELETON)
GAME(1976,  lemans,    0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Le Mans [TTL]",          MACHINE_IS_SKELETON)
GAME(1976,  outlaw,    0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Outlaw [TTL]",           MACHINE_IS_SKELETON)
GAME(1974,  qwakttl,   0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Qwak!/Quack [TTL]",      MACHINE_IS_SKELETON)
GAME(1975,  sharkjaw,  0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari/Horror Games",    "Shark JAWS [TTL]",     MACHINE_IS_SKELETON)
GAME(1975,  steeplec,  0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari",        "Steeplechase [TTL]",     MACHINE_IS_SKELETON)
GAME(1976,  stuntcyc,  0,        stuntcyc, stuntcyc, stuntcyc_state, empty_init, ROT0,  "Atari",        "Stunt Cycle [TTL]",      MACHINE_IS_SKELETON)
GAME(1974,  tank,      0,        tank,         tank, tank_state,     empty_init, ROT0,  "Atari/Kee",    "Tank/Tank Cocktail [TTL]",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME(1975,  tankii,    0,        atarikee,        0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Tank II [TTL]",          MACHINE_IS_SKELETON)

// MISSING ROM DUMPS
//GAME(1975,  astrotrf,  steeplec, atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Astroturf [TTL]",        MACHINE_IS_SKELETON)

// 100% TTL
//GAME(1974,  coupfran,  worldcup, atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari Europe", "Coup Franc [TTL]",       MACHINE_IS_SKELETON)
//GAME(1974,  coupdmnd,  worldcup, atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari France", "Coup du Monde [TTL]",    MACHINE_IS_SKELETON)
//GAME(1975,  crossfir,  0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Crossfire [TTL]",        MACHINE_IS_SKELETON)
//GAME(1973,  eliminat,  0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari/Kee",    "Elimination! [TTL]",     MACHINE_IS_SKELETON)
//GAME(1975,  goaliv,    0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Goal IV [TTL]",          MACHINE_IS_SKELETON)
//GAME(1973,  gotchaat,  0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Gotcha [TTL]",           MACHINE_IS_SKELETON) //?
//GAME(1973,  gotchaatc, 0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Gotcha Color [TTL]",     MACHINE_IS_SKELETON) //?
//GAME(1975,  hiway,     0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Hi-Way/Highway [TTL]",   MACHINE_IS_SKELETON)
//GAME(1974,  pinpong,   0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Pin Pong [TTL]",         MACHINE_IS_SKELETON)
//GAME(1975,  pursuit,   0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Pursuit [TTL]",          MACHINE_IS_SKELETON)
//GAME(1974,  quadpong,  eliminat, atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Quadrapong [TTL]",       MACHINE_IS_SKELETON)
//GAME(1973,  spacrace,  0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Space Race [TTL]",       MACHINE_IS_SKELETON)
//GAME(1974,  touchme,   0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "Touch-Me [TTL]",         MACHINE_IS_SKELETON) //?
//GAME(1974,  worldcup,  0,        atarikee, 0, atarikee_state, empty_init, ROT0,  "Atari",        "World Cup/World Cup Football [TTL]",   MACHINE_IS_SKELETON)
